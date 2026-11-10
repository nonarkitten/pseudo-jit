/// @file smoke_exception_exec.c
/// @brief Real 68K exception delivery: a vector table in RAM, deliberate
///        faults, a handler per vector that records the stack frame, and
///        RTE back to the instruction after the trap.
///
/// Each case checks four things at once. D0 carries a per-vector marker
/// set by the handler, so a wrong D0 means the wrong vector was taken and
/// reaching the stop slot at all means RTE returned to the right place.
/// D1 and D2 carry the SR and PC the exception frame pushed, so a wrong
/// resume address is reported rather than merely hung on.
///
/// Built twice (tests/qemu/smoke_exception_interp.c, smoke_exception_dr.c): through the
/// C-free runtime interpretive-only, and with the direct-mapped DR cache.
#include <stdint.h>
#include "pjit.h"
#include "mem_be.h"

extern void qemu_exit(void);
extern void emit_opcode_table(void);

static uint8_t  ram_pool[12 * 1024 * 1024] __attribute__((aligned(4096)));

static uint16_t progmem[16384] __attribute__((aligned(32768)));   // 32KB of guest memory

static uint32_t vectors[64]    __attribute__((aligned(4)));
static uint32_t sup_stack[256] __attribute__((aligned(8)));

// The 68K code needs a stack of its own. A7 *is* the ARM SP, so leaving it
// pointing at the C stack means the exception frame -- written six bytes
// below A7 -- lands on top of this function's own spilled locals.
uint32_t saved_sp;   // global: named directly from asm below

// Handlers live above every case's program, not below: the first
// dispatch translates from that case's entry forward to the page edge,
// and translated_up_to then reports the whole rest of the page as done.
// A handler at a lower address would be claimed as translated while
// still holding dispatch trampolines.
#define HANDLER_BASE 4096
#define HANDLER_SIZE 8

static uint32_t off, base, np, nh;
static int pass, fail;

static void wbe(uint16_t *a, uint16_t v) { ((uint8_t*)a)[0] = v >> 8; ((uint8_t*)a)[1] = v; }
static void w(uint16_t v)   { wbe(&progmem[base + np++], v); }
static void w32(uint32_t v) { w(v >> 16); w(v); }
static void start(void)     { base = off; np = 0; off += 32; }

/// @brief Installs a handler for `vector` returning `marker` in D0.
/// @param marker must be 0x00-0x7F: MOVEQ sign-extends its byte.
/// @details Records the pushed SR in D1 and the pushed PC in D2 before
///          returning, so a failure says which part went wrong.
static void handler(int vector, uint8_t marker)
{
    uint16_t *h = &progmem[HANDLER_BASE + nh++ * HANDLER_SIZE];
    wbe(&h[0], 0x3217);           // MOVE.W (A7),D1    -- pushed SR
    wbe(&h[1], 0x242F);           // MOVE.L 2(A7),D2   -- pushed PC
    wbe(&h[2], 0x0002);
    wbe(&h[3], 0x7000 | marker);  // MOVEQ #marker,D0
    if (vector == ILLINSTR || vector == LINE_A || vector == LINE_F) {   // the frame holds the address of the instruction itself: step over it (one word)
        wbe(&h[4], 0x54AF);       // ADDQ.L #2,2(A7)
        wbe(&h[5], 0x0002);
        wbe(&h[6], 0x4E73);       // RTE
    } else
        wbe(&h[4], 0x4E73);       // RTE
    write_BE_long((uint32_t)&vectors[vector], (uint32_t)h);
}

static struct {
    const char *name;
    uint32_t base, np, want_d0, want_pc;
} cases[32];
static int ncases;

/// @brief Records the case just built. want_d0 is the handler's marker,
///        or the poison value when the case must *not* trap.
static void def(const char *name, uint32_t want_d0)
{
    cases[ncases].name    = name;
    cases[ncases].base    = base;
    cases[ncases].np      = np;
    cases[ncases].want_d0 = want_d0;
    // 68K pushes the address of the instruction after the trapping one (TRAP, TRAPV, CHK, DIVZ), which is where the stop slot sits;
    // for illegal instruction and line A / F it pushes the address of the instruction itself.
    cases[ncases].want_pc = (uint32_t)&progmem[base + np - ((want_d0 == 0x11 || want_d0 == 0x55 || want_d0 == 0x66) ? 1 : 0)];
    ncases++;
    w(0x4E4F);                  // TRAP #15: where the handler's RTE lands, and the test exit
}

#define POISON 0xDEADBEEFu

static void exec_all(void)
{
    uint32_t stk = (uint32_t)&sup_stack[256];

    for (int i = 0; i < ncases; i++) {
        pjit_set_sr(0x2000);                 // supervisor
        cpu->ssp = cpu->usp = __builtin_bswap32(stk);   // big-endian, like the other state JIT'd code touches
        for (int r = 0; r < 8; r++) pjit_set_an(r, 0);
        pjit_set_an(7, stk);
        pjit_set_dn(0, POISON);
        pjit_set_dn(1, POISON);
        pjit_set_dn(2, POISON);
        pjit_run(&progmem[cases[i].base]);
        {
            uint32_t d0 = pjit_get_dn(0), d1 = pjit_get_dn(1), d2 = pjit_get_dn(2);
            int ok = (d0 == cases[i].want_d0);
            if (ok && cases[i].want_d0 != POISON && d2 != cases[i].want_pc) ok = 0;
            if (ok) { pass++; printf("[ ok ] %-12s D0=%02X SR=%04X\n", cases[i].name, d0, d1 & 0xFFFF); }
            else {
                fail++;
                printf("[FAIL] %-12s D0=%08X want %08X | frame SR=%08X PC=%08X want %08X\n",
                       cases[i].name, d0, cases[i].want_d0, d1, d2, cases[i].want_pc);
            }
        }
    }
}

int main(void)
{
    printf("=== PJIT smoke test: 68K exception vectors and RTE ===\n");

    pjit_cache_init((uint32_t)(ram_pool + sizeof(ram_pool)));
#ifdef PJIT_TEST_NO_DR
    cpu->dr_threshold = 0xFFFFFFFFu;
#endif
    emit_opcode_table();

    for (uint32_t i = 0; i < sizeof(progmem)/2; i++) wbe(&progmem[i], 0x4E71); // NOP fill

    cpu->vbr = (uint32_t)vectors;

    // Catch-all first, so an unexpected vector reports 0xEE instead of
    // branching through an empty table entry and hanging.
    handler(0, 0xEE);
    for (int v = 0; v < 64; v++)
        write_BE_long((uint32_t)&vectors[v], (uint32_t)&progmem[HANDLER_BASE]);

    handler(ILLINSTR, 0x11);
    handler(TRAP0,    0x22);
    handler(TRAP5,    0x33);
    handler(TRAP9,    0x44);
    handler(LINE_A,   0x55);
    handler(LINE_F,   0x66);
    handler(TRAPV,    0x77);
    handler(TRAPCHK,  0x18);
    handler(DIVZ,     0x19);

    start(); w(0x4AFC);                       def("ILLEGAL",  0x11);
    start(); w(0x4E40);                       def("TRAP #0",  0x22);
    start(); w(0x4E45);                       def("TRAP #5",  0x33);
    start(); w(0x4E49);                       def("TRAP #9",  0x44);
    start(); w(0xA000);                       def("LINE A",   0x55);
    start(); w(0xF000);                       def("LINE F",   0x66);

    // TRAPV traps only when V is set. Both cases reach TRAPV through the
    // same ADD.L D3,D3, so V is whatever that add really produced rather
    // than whatever a previous case left behind.
    start(); w(0x263C); w32(0x7FFFFFFFu); w(0xD683); w(0x4E76);
                                              def("TRAPV V=1", 0x77);
    start(); w(0x263C); w32(1u);          w(0xD683); w(0x4E76);
                                              def("TRAPV V=0", POISON);

    // CHK and DIVZ raise their SVC from inside a shared routine (a body the slot `bl`'d into), so
    // they are the cases that make pjit_svc_entry recover the guest PC from the return address.
    // CHK.W D3,D4 traps when D4 is negative or exceeds the bound in D3.
    start(); w(0x263C); w32(10u); w(0x283C); w32(100u); w(0x4983);
                                              def("CHK >bound", 0x18);
    start(); w(0x263C); w32(10u); w(0x283C); w32(0xFFFFFFFFu); w(0x4983);
                                              def("CHK <0",     0x18);
    start(); w(0x263C); w32(100u); w(0x283C); w32(10u); w(0x4983);
                                              def("CHK ok",     POISON);

    // DIVU.W #0,D3 -- the divisor is zero, so the trap fires before the
    // helper is called at all.
    start(); w(0x86FC); w(0x0000);            def("DIVZ",       0x19);

    exec_all();

    printf("[TEST] %d passed, %d failed\n", pass, fail);
    printf(fail ? "FAIL\n" : "PASS\n");
    qemu_exit();
    return 0;
}
