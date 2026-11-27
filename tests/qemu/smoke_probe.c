/// @file smoke_probe.c
/// @brief The CPU probes programs make on an Amiga (SysInfo, MMULib, SetPatch): user-mode code executes an instruction a 68000 does not have, a handler catches the
///        exception, steps over the instruction by adding its length to the stacked PC, and returns with RTE. A two-word line-F (a 68030 PMOVE) caught directly and
///        through an exec-style dispatch (the vector pushes the trap number and jumps to the task's trap code, which pops it), privilege violations (ORI #,SR, MOVE #,SR)
///        and an illegal MOVEC; and a double fault (an address error whose own vector is odd), where pjit_run() returns PJIT_VEC_HALT. Each probe runs 300 times in a DBF loop, so a tiered build runs it interpreted and translated. (An odd vector cannot be tested here:
///        its address-error frame on top of the first frame puts a long at a 2 mod 4 address, which QEMU faults on and the board does not.)
///
/// Each case checks the handler's count in D0 (one per iteration), the stacked PC it saw last in D2 (the address of the probed instruction), that the instruction after
/// the probe ran every time (D5 collects what it would leave behind otherwise), that A7 is back where it started, and that the program ends in user mode.
///
/// Built twice (tests/qemu/smoke_probe_interp.c, smoke_probe_dr.c): interpretive only, and with the direct-mapped DR cache.
#include <stdint.h>
#include "pjit.h"
#include "mem_be.h"

extern void qemu_exit(void);
extern void emit_opcode_table(void);

static uint8_t  ram_pool[12 * 1024 * 1024] __attribute__((aligned(4096)));
static uint16_t progmem[16384] __attribute__((aligned(32768)));
static uint32_t vectors[64]     __attribute__((aligned(4)));
static uint32_t sup_stack[256]  __attribute__((aligned(8)));
static uint32_t user_stack[256] __attribute__((aligned(8)));

#define HANDLER_BASE 8192                       /* (above every case: see smoke_exception_exec.c) */
#define ITER 300
#define POISON 0xDEADBEEFu

static uint32_t off, base, np, hp = HANDLER_BASE;
static int pass, fail;

static void wbe(uint16_t *a, uint16_t v) { ((uint8_t*)a)[0] = v >> 8; ((uint8_t*)a)[1] = v; }
static void w(uint16_t v)   { wbe(&progmem[base + np++], v); }
static uint32_t here(void)  { return (uint32_t)&progmem[base + np]; }
static void hw(uint16_t v)  { wbe(&progmem[hp++], v); }
static void hw32(uint32_t v) { hw(v >> 16); hw(v); }
static uint32_t hhere(void) { return (uint32_t)&progmem[hp]; }
static void vec(int v, uint32_t a) { write_BE_long((uint32_t)&vectors[v], a); }

/// A handler that counts in D0, keeps the stacked PC in D2, steps the PC over `len` bytes and returns.
static uint32_t handler_skip(int len)
{
    uint32_t h = hhere();
    hw(0x242F); hw(0x0002);                     /* MOVE.L 2(A7),D2 */
    hw(0x5280);                                 /* ADDQ.L #1,D0 */
    hw(0x50AF | ((len & 7) << 9)); hw(0x0002);  /* ADDQ.L #len,2(A7) */
    hw(0x4E73);                                 /* RTE */
    return h;
}

/// exec's 68000 trap entry and a task's trap code: the vector pushes the trap number (a long) and jumps to the code, which checks it (D7 = -1 if it was not `num`),
/// pops it and steps over.
static uint32_t handler_dispatch(int num, int len)
{
    uint32_t code = hhere();
    hw(0x0C6F); hw((uint16_t)num); hw(0x0002);  /* CMPI.W #num,2(A7) (the long's low word) */
    hw(0x6702);                                 /* BEQ.S +2 */
    hw(0x7EFF);                                 /* MOVEQ #-1,D7 */
    hw(0x588F);                                 /* ADDQ.L #4,A7: the trap number off */
    hw(0x242F); hw(0x0002);                     /* MOVE.L 2(A7),D2 */
    hw(0x5280);                                 /* ADDQ.L #1,D0 */
    hw(0x50AF | ((len & 7) << 9)); hw(0x0002);  /* ADDQ.L #len,2(A7) */
    hw(0x4E73);                                 /* RTE */
    uint32_t entry = hhere();
    hw(0x598F);                                 /* SUBQ.L #4,A7 ; MOVE.W #0,(A7) ; MOVE.W #num,2(A7): the trap number as a long, as exec pushes it (two word stores: */
    hw(0x3EBC); hw(0x0000);                     /* the frame leaves A7 at 2 mod 4, and QEMU faults on an unaligned long store, which the board does not) */
    hw(0x3F7C); hw((uint16_t)num); hw(0x0002);
    hw(0x4EF9); hw32(code);                     /* JMP code */
    return entry;
}

static struct { const char *name; uint32_t base, probe, want_d0; } cases[16];
static int ncases;

/// The loop around one probe: D6 counts, D3 is set to -1 before the probe and cleared by the instruction after it, D5 collects D3's low byte (stays 0 when the
/// instruction after the probe ran every time); a balanced SUBQ / ADDQ on A7 around it, as SysInfo has. `probe` writes the probed instruction.
static void loop_case(const char *name, void (*probe)(void), uint32_t want_d0)
{
    base = off; np = 0; off += 64;
    w(0x7000);                                  /* MOVEQ #0,D0 */
    w(0x7A00);                                  /* MOVEQ #0,D5 */
    w(0x7E00);                                  /* MOVEQ #0,D7 */
    w(0x3C3C); w(ITER - 1);                     /* MOVE.W #ITER-1,D6 */
    uint32_t top = here();
    w(0x76FF);                                  /* MOVEQ #-1,D3 */
    w(0x598F);                                  /* SUBQ.L #4,A7 */
    cases[ncases].probe = here();
    probe();
    w(0x4203);                                  /* CLR.B D3 */
    w(0x588F);                                  /* ADDQ.L #4,A7 */
    w(0x8A03);                                  /* OR.B D3,D5 */
    w(0x51CE); w((uint16_t)(top - here()));     /* DBF D6,top */
    w(0x4E4F);                                  /* TRAP #15: the end */
    cases[ncases].name = name;
    cases[ncases].base = base;
    cases[ncases].want_d0 = want_d0;
    ncases++;
}

static void p_pmove(void)  { w(0xF017); w(0x4E00); }   /* PMOVE (A7),... (68030): line F */
static void p_ori_sr(void) { w(0x007C); w(0x0000); }   /* ORI.W #0,SR: privileged */
static void p_move_sr(void){ w(0x46FC); w(0x0000); }   /* MOVE.W #0,SR: privileged */
static void p_movec(void)  { w(0x4E7B); w(0x0801); }   /* MOVEC D0,VBR (68010): illegal on a 68000 */
static void p_linef1(void) { w(0xF000); w(0x4E71); }   /* a one-word line F, then a NOP the handler does not skip */

int main(void)
{
    printf("=== PJIT smoke test: CPU probes (line F, privilege, illegal) from user mode ===\n");

    pjit_cache_init((uint32_t)(ram_pool + sizeof(ram_pool)));
#ifdef PJIT_TEST_NO_DR
    cpu->dr_threshold = 0xFFFFFFFFu;
#endif
    emit_opcode_table();
    for (uint32_t i = 0; i < sizeof(progmem) / 2; i++) wbe(&progmem[i], 0x4E71);
    cpu->vbr = (uint32_t)vectors;

    uint32_t catch_all = hhere();               /* anything unexpected: D7 = $EE, end */
    hw(0x7EEE); hw(0x4E4F);
    for (int v = 0; v < 64; v++) vec(v, catch_all);

    struct { int vector; uint32_t handler; void (*probe)(void); const char *name; } plan[] = {
        { LINE_F,   handler_skip(4),        p_pmove,   "line F, PMOVE, +4" },
        { LINE_F,   handler_dispatch(11, 4),p_pmove,   "line F through a trap-code dispatch" },
        { PRIV,     handler_skip(4),        p_ori_sr,  "privilege, ORI #,SR" },
        { PRIV,     handler_skip(4),        p_move_sr, "privilege, MOVE #,SR" },
        { ILLINSTR, handler_skip(4),        p_movec,   "illegal, MOVEC" },
        { LINE_F,   handler_skip(2),        p_linef1,  "line F, one word, +2" },
    };
    for (unsigned k = 0; k < sizeof plan / sizeof plan[0]; k++) loop_case(plan[k].name, plan[k].probe, ITER);

    for (int i = 0; i < ncases; i++) {
        uint32_t ustk = (uint32_t)&user_stack[200], sstk = (uint32_t)&sup_stack[256];
        for (int v = 0; v < 64; v++) vec(v, catch_all);
        vec(plan[i].vector, plan[i].handler);
        pjit_set_sr(0x0000);
        cpu->ssp = __builtin_bswap32(sstk);
        cpu->usp = __builtin_bswap32(ustk);
        for (int r = 0; r < 8; r++) pjit_set_an(r, 0);
        pjit_set_an(7, ustk);
        for (int r = 0; r < 8; r++) pjit_set_dn(r, POISON);
        pjit_run(&progmem[cases[i].base]);
        uint32_t d0 = pjit_get_dn(0), d2 = pjit_get_dn(2), d5 = pjit_get_dn(5), d7 = pjit_get_dn(7), a7 = pjit_get_an(7), sr = pjit_get_sr();
        int ok = d0 == cases[i].want_d0 && d2 == cases[i].probe && d5 == 0 && d7 == 0 && a7 == ustk && !(sr & 0x2000);
        if (ok) { pass++; printf("[ ok ] %s\n", cases[i].name); }
        else {
            fail++;
            printf("[FAIL] %s: D0=%08X (want %08X) D2=%08X (want %08X) D5=%08X D7=%08X A7=%08X (want %08X) SR=%04X\n", cases[i].name, d0, cases[i].want_d0, d2,
                   cases[i].probe, d5, d7, a7, ustk, sr);
        }
    }

    /* a double fault: an address error (a JMP to an odd address, an odd word read) whose own vector is odd; the 68000 halts, pjit_run() returns PJIT_VEC_HALT with
       the handler's address in cpu->r0 and the vector in cpu->r1 */
    static const struct { const char *name; uint16_t op[4]; int n; } dbl[] = {
        { "double fault: JMP to an odd address", { 0x4EF9, 0, 0 }, 3 },       /* (an odd data access becomes an address error on the board only: tests/buffee/buffee_probe.c) */
    };
    uint32_t odd_handler = (uint32_t)&progmem[hp] + 1;
    for (unsigned k = 0; k < sizeof dbl / sizeof dbl[0]; k++) {
        base = off; np = 0; off += 64;
        uint32_t target = (uint32_t)&progmem[base + 32] + 1;
        for (int j = 0; j < dbl[k].n; j++) w(dbl[k].op[j]);
        if (k == 0) { wbe(&progmem[base + 1], target >> 16); wbe(&progmem[base + 2], target); }
        for (int v = 0; v < 64; v++) vec(v, catch_all);
        vec(ADDRESSERR, odd_handler);
        pjit_set_sr(0x2700);
        uint32_t sstk = (uint32_t)&sup_stack[256];
        cpu->ssp = cpu->usp = __builtin_bswap32(sstk);
        for (int r = 0; r < 8; r++) { pjit_set_an(r, 0); pjit_set_dn(r, POISON); }
        pjit_set_an(0, target);
        pjit_set_an(7, sstk);
        cpu->r0 = cpu->r1 = 0;
        uint32_t r = pjit_run(&progmem[base]);
        int ok = r == PJIT_VEC_HALT && cpu->r0 == odd_handler && cpu->r1 == ADDRESSERR && pjit_get_an(7) == sstk;
        if (ok) { pass++; printf("[ ok ] %s\n", dbl[k].name); }
        else { fail++; printf("[FAIL] %s: pjit_run %X (want %X), r0 %08X (want %08X), r1 %u, A7 %08X (want %08X)\n", dbl[k].name, (unsigned)r, PJIT_VEC_HALT,
                              (unsigned)cpu->r0, (unsigned)odd_handler, (unsigned)cpu->r1, (unsigned)pjit_get_an(7), (unsigned)sstk); }
    }

    printf("[TEST] %d passed, %d failed\n", pass, fail);
    printf(fail ? "FAIL\n" : "PASS\n");
    qemu_exit();
    return 0;
}
