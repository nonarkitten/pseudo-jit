/// @file smoke_user_trap.c
/// @brief A real trap taken from genuine 68K user-mode execution, all the
///        way through pjit_svc_entry -> the guest's handler -> RTE ->
///        resumed user-mode execution.
///
/// Every other exception test (smoke_exception_exec.c,
/// smoke_supervisor_exec.c) enters exec_all() already in supervisor mode,
/// so none of them exercises the exception entry's switch from the user
/// stack to the supervisor stack. The entry parks A7 in cpu->usp with a
/// plain str under ambient E=BE, the convention guest code uses for it too
/// (MOVE to/from USP, the SR instructions' re-bank) and the one
/// pjit_get_dn()/pjit_set_dn() exist to bridge for D0-D7. This test does
/// the real thing: run 68K code in user mode on its own stack, trap via
/// TRAP #0, read cpu->usp back with a genuine MOVE USP,A0 inside the
/// handler, RTE, and check that ordinary user-mode execution resumes.
///
/// Safe regardless of outcome: run_case()'s escape trampoline is a
/// PC-relative `ldr pc,[pc,#-4]` that never dereferences SP, so even if
/// RTE re-banks the live ARM SP to a byte-swapped garbage address, nothing
/// here ever touches it before control returns to C and this function's
/// own epilogue reloads the real C stack pointer.
#include <stdint.h>
#include "pjit.h"
#include "pjit_compat.h"
#include "mem_be.h"

extern void qemu_exit(void);
extern void emit_opcode_table(void);

static uint8_t  ram_pool[12 * 1024 * 1024] __attribute__((aligned(4096)));
static uint16_t progmem[32768u/2] __attribute__((aligned(32768u)));

static uint32_t vectors[64]    __attribute__((aligned(4)));
static uint32_t sup_stack[256] __attribute__((aligned(8)));
static uint32_t usr_stack[256] __attribute__((aligned(8)));

// A7 *is* the ARM SP while JIT'd code runs -- see smoke_exception_exec.c's
// run_case() for why this has to be a global rather than a local.
uint32_t saved_sp;

#define HANDLER_BASE 4096
#define HANDLER_SIZE 8
#define STOP_SLOT    1   // slots after the case's own program before the
                          // hand-built ARM escape trampoline

static uint32_t off, base, np;
static int pass, fail;

static void wbe(uint16_t *a, uint16_t v) { ((uint8_t*)a)[0] = v >> 8; ((uint8_t*)a)[1] = v; }
static void w(uint16_t v) { wbe(&progmem[base + np++], v); }
static void start(void)   { base = off; np = 0; off += 32; }

/// @brief TRAP #0 handler: records the pushed SR/PC, reads cpu->usp back
///        through a genuine JIT'd "MOVE USP,A0" (the read side of the
///        byte-order question), then RTEs.
static void install_handler(void)
{
    uint16_t *h = &progmem[HANDLER_BASE];
    int i = 0;
    wbe(&h[i++], 0x3217);   // MOVE.W (A7),D1    -- pushed SR
    wbe(&h[i++], 0x242F);   // MOVE.L 2(A7),D2   -- pushed PC
    wbe(&h[i++], 0x0002);
    wbe(&h[i++], 0x4E68);   // MOVE USP,A0       -- the read side under test
    wbe(&h[i++], 0x2608);   // MOVE.L A0,D3      -- report it where C can check
    wbe(&h[i++], 0x4E73);   // RTE
    write_BE_long((uint32_t)&vectors[TRAP0], (uint32_t)h);
}

static struct {
    const char *name;
    uint32_t base, trap_pc_off; // trap_pc_off: slots between base and the
                                 // instruction right after TRAP #0
    uint32_t entry_sp;
} cases[8];
static int ncases;

static void def(const char *name, uint32_t entry_sp)
{
    cases[ncases] = (typeof(cases[0])){ name, base, 1, entry_sp };
    ncases++;
}

#define POISON 0xDEADBEEFu

/// @brief Patches the stop slot, then runs the case on its own real 68K
///        user stack. See smoke_exception_exec.c's run_case() for the
///        full rationale (noinline, global saved_sp).
static void run_case(uint32_t e, uint32_t np_total, uint32_t stk)
{
    uint8_t *stop = (uint8_t *)(e + np_total * 2);   // TRAP #15 after the case: where the handler's RTE lands
    stop[0] = 0x4E; stop[1] = 0x4F;
    pjit_set_an(7, stk);
    (void)pjit_compat_run((uint16_t *)e);
}

static void exec_all(void)
{
    for (int i = 0; i < ncases; i++) {
        printf("[RUN] %s\n", cases[i].name);
        uint32_t e = pjit_cache_dispatch((uint32_t)&progmem[cases[i].base]);
        if (!e) { printf("[FAIL] %-14s dispatch returned 0\n", cases[i].name); fail++; continue; }

        uint32_t entry_sp = cases[i].entry_sp;

        pjit_set_sr(0x0000);              // genuine user mode: S clear
        cpu->ssp = __builtin_bswap32((uint32_t)&sup_stack[256]); // supervisor stack, separate (big-endian shadow)
                                               // from the user stack the
                                               // trap must actually save
        cpu->usp = POISON;                // must be overwritten by the trap
        pjit_set_dn(1, POISON);
        pjit_set_dn(2, POISON);
        pjit_set_dn(3, POISON);
        pjit_set_dn(4, POISON);

        uint32_t want_trap_pc = (uint32_t)&progmem[cases[i].base + cases[i].trap_pc_off];
        // Total program slots: TRAP #0, then the MOVEQ #0x55,D4 resume
        // marker -- see the case builder below.
        uint32_t total_np = cases[i].trap_pc_off + 1;

        // Entry SP is the live ARM SP the moment TRAP #0 executes -- with
        // no stack-touching instruction before it, this *is* the USP
        // pjit_svc_entry must capture.
        run_case(e, total_np, entry_sp);

        // D1 (pushed SR) is captured by the handler but not checked here:
        // it's a word-sized MOVE into a register pjit_get_dn() only ever
        // reads back as a full swapped longword, and POISON's untouched
        // upper half makes that comparison meaningless -- not what this
        // test is about. See smoke_exception_exec.c's handler(), which
        // has the same capture and the same non-check.
        uint32_t d2 = pjit_get_dn(2);
        uint32_t d3 = pjit_get_dn(3); // cpu->usp, read back via a real
                                       // JIT'd MOVE USP,A0 inside the
                                       // handler -- the actual quantity
                                       // under test
        uint32_t d4 = pjit_get_dn(4); // proves RTE resumed genuine
                                       // user-mode execution at the right
                                       // PC, not just that *a* PC was
                                       // reached

        int ok = 1;
        if (d2 != want_trap_pc)    { printf("[FAIL] %-14s pushed PC=%08X want %08X\n", cases[i].name, d2, want_trap_pc); ok = 0; }
        if (d3 != entry_sp)        { printf("[FAIL] %-14s cpu->usp read back=%08X want %08X (byte-swapped would read %08X)\n",
                                             cases[i].name, d3, entry_sp, __builtin_bswap32(entry_sp)); ok = 0; }
        if (d4 != 0x55)            { printf("[FAIL] %-14s resume marker D4=%08X want 00000055 -- RTE did not resume user-mode execution correctly\n",
                                             cases[i].name, d4); ok = 0; }

        if (ok) { pass++; printf("[ ok ] %-14s cpu->usp round-tripped %08X, resumed cleanly\n", cases[i].name, d3); }
        else fail++;
    }
}

int main(void)
{
    printf("=== PJIT smoke test: real trap from genuine 68K user-mode execution ===\n");

    pjit_cache_init((uint32_t)(ram_pool + sizeof(ram_pool)));
    emit_opcode_table();

    for (uint32_t i = 0; i < sizeof(progmem)/2; i++) wbe(&progmem[i], 0x4E71); // NOP fill

    cpu->vbr = (uint32_t)vectors;

    // Catch-all so a wrong vector reports a hang-free, obviously-wrong D3
    // rather than branching through an empty table entry.
    for (int v = 0; v < 64; v++)
        write_BE_long((uint32_t)&vectors[v], (uint32_t)&progmem[HANDLER_BASE]);
    install_handler();

    // Two distinct user stack pointers -- not just one -- so a false pass
    // from a coincidentally byte-swap-symmetric address can't hide the bug.
    start(); w(0x4E40); w(0x7855);   // TRAP #0 ; MOVEQ #0x55,D4
                                      def("user trap #1", (uint32_t)&usr_stack[128]);
    start(); w(0x4E40); w(0x7855);   // TRAP #0 ; MOVEQ #0x55,D4
                                      def("user trap #2", (uint32_t)&usr_stack[200]);

    exec_all();

    printf("[TEST] %d passed, %d failed\n", pass, fail);
    printf(fail ? "FAIL\n" : "PASS\n");
    qemu_exit();
    return 0;
}
