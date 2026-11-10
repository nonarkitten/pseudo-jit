/// @file smoke_dhrystone.c
/// @brief Runs Weicker's Dhrystone 2.1 (test_progs/dhrystone/, built into tests/data/dhrystone_image.h by tools/data/dhrystone.py) as
///        68K code and reports *instruction counts*, not wall time: QEMU's -icount shift=0 makes
///        the ARM generic timer count executed instructions (1 ns each), so the counter reads
///        ARM instructions. Three passes: (1) interpretive with the opcode histogram on, which gives the
///        number of 68K instructions; (2) interpretive; (3) out of the DR cache. Run it with
///        `qemu-system-arm ... -icount shift=0,sleep=off`.
#include <stdint.h>
#include "pjit.h"
#if defined(PJIT_BLK) || defined(PJIT_TIER)
#include "blk.h"
extern blk_t pjit_blk;
#endif
#include "pjit_libc.h"
#include "mem_be.h"
#include "cp15.h"
#include "dhrystone_image.h"

#ifdef BBB      /* BeagleBone: real cycles from the ARM performance monitor, no QEMU page table (platform/beaglebone/bbb.c owns the MMU) */
#include "bbb.h"
#define UNIT "cycles"
static uint32_t ev_before[4], ev_delta[4];
static uint32_t read_cntpct_lo(void) { return bbb_pmu_cycles(); }
#else
#define UNIT "ARM instructions"
extern uint32_t read_cntpct_lo(void);
#endif
extern void qemu_exit(void);
extern void emit_opcode_table(void);
extern void pjit_prefetch_count(void);
extern void pjit_prefetch(void);

#ifndef HIST_HOOK
#define HIST_HOOK pjit_prefetch_count
#endif
#ifndef TRACE_RUNS_A
#define TRACE_RUNS_A 10
#endif
#ifndef TRACE_RUNS_B
#define TRACE_RUNS_B 30
#endif
#ifdef ONLY_DR
#define ONLY_DR_THRESHOLD 0u            /* (the guest is below the 16 MB the production setting interprets: translate everything) */
#endif
#ifdef ONLY_INTERP
#define ONLY_DR_THRESHOLD 0xFFFFFFFFu
#endif
#ifndef HIST_TOP
#define HIST_TOP 40          /* how many of the most executed opcodes the [HIST] lines list */
#endif
#ifndef RUNS_A
#define RUNS_A 50
#endif
#ifndef RUNS_B
#define RUNS_B 150
#endif
static uint32_t runs_now;

uint32_t pjit_hist[65536];
uint32_t pjit_trace[131072];
uint32_t pjit_trace_pos;
extern void pjit_prefetch_trace(void);
extern int semihost_write_file(const char *path, const void *buf, unsigned len);
static uint32_t hA[65536];

static uint8_t  ram_pool[12 * 1024 * 1024] __attribute__((aligned(4096)));
#ifdef ON_BUFFEE
#define low_mem ((uint8_t *)0x90000000u)      // on Buffee the guest's low megabyte is the DDR megabyte that bbb_map_low() is given (tests/buffee/buffee_fried.c)
#else
static uint8_t  low_mem[1024 * 1024] __attribute__((aligned(1024 * 1024)));      // what guest addresses 0..0xFFFFF are
#endif
static uint32_t vectors[64] __attribute__((aligned(4)));
static uint16_t hand_a[8] __attribute__((aligned(8)));   // MOVE.L 2(A7),D0 ; TRAP #12
static uint16_t hand_b[8] __attribute__((aligned(8)));   // MOVE.L 10(A7),D0 ; TRAP #12

#ifndef BBB
static uint32_t mmu_pagetable[4096] __attribute__((aligned(16384)));
static void map_low(void)       // the identity map, except that the first megabyte is low_mem (QEMU faults unaligned halfword/word accesses while the MMU is off)
{
    for (uint32_t i = 0; i < 4096; i++) mmu_pagetable[i] = ((i << 20) & 0xFFF00000u) | 0xC0Eu;
    mmu_pagetable[0] = ((uint32_t)low_mem & 0xFFF00000u) | 0xC0Eu;
    CP15TtbCtlTtb0Config(); CP15Ttb0Set((uint32_t)mmu_pagetable); CP15DomainAccessClientSet(); CP15TlbInvalidate(); CP15MMUEnable();
}
#else
static void map_low(void) { bbb_map_low((uint32_t)low_mem); }
#endif

static uint32_t be32(const uint8_t *p) { return (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3]; }
static void put32(uint8_t *p, uint32_t v) { p[0] = v >> 24; p[1] = v >> 16; p[2] = v >> 8; p[3] = v; }
static void put16(uint32_t a, uint16_t v) { low_mem[a] = v >> 8; low_mem[a + 1] = (uint8_t)v; }

#define GUEST_STACK 0xF0000u

/// Lays the program out afresh: zeroed data and bss, the image, the console stubs (nothing is ever printed: the cell QUIET makes the guest's printf return at once), the run count.
static void load(void)
{
    memset(low_mem, 0, 0x8000);
    memcpy(low_mem + DHRYSTONE_LOAD, dhrystone_image, dhrystone_image_size);
    put16(DHRYSTONE_BIOS_PUTC, 0x4E75);                                         // putc: RTS
    put16(DHRYSTONE_BIOS_EXIT, 0x4E4F); put16(DHRYSTONE_BIOS_EXIT + 2, 0x4E75); // exit: TRAP #15 ; RTS
    low_mem[DHRYSTONE_QUIET] = 1;
    put32(low_mem + DHRYSTONE_RUNS, runs_now);
    hand_a[0] = 0x202F >> 8 | (0x202F & 0xFF) << 8; hand_a[1] = 2 << 8; hand_a[2] = 0x4E4C >> 8 | (0x4E4C & 0xFF) << 8;
    hand_b[0] = 0x202F >> 8 | (0x202F & 0xFF) << 8; hand_b[1] = 10 << 8; hand_b[2] = 0x4E4C >> 8 | (0x4E4C & 0xFF) << 8;
    for (int v = 2; v < 64; v++) put32((uint8_t *)&vectors[v], (uint32_t)((v == 2 || v == 3) ? hand_b : hand_a));
    cpu->vbr = (uint32_t)vectors;
}

static int keep_cache;                              // 1: the translated blocks stay (a warm run: no translation in the timed window)
static void start(void)
{
    load();
    for (int i = 0; i < 8; i++) { pjit_set_dn(i, 0); pjit_set_an(i, 0); }
    pjit_set_an(7, GUEST_STACK);                    // (the program sets its own; exception frames go below it)
    cpu->ssp = cpu->usp = __builtin_bswap32(GUEST_STACK);
    pjit_set_sr(0x2700);
    if (!keep_cache) pjit_cache_flush();
}

static uint32_t g32(uint32_t a) { return be32(low_mem + a); }
static int failures;
static int same_string(const char *a, const char *b) { while (*a && *a == *b) a++, b++; return *a == *b; }
static void expect(const char *what, uint32_t got, uint32_t want)
{
    if (got != want) { printf("[FAIL] %s is %u, Dhrystone says it should be %u\n", what, got, want); failures++; }
}

/// The checks the program prints "should be" lines for, read from guest memory: the globals and the two records (Rec_Type: Ptr_Comp, Discr, then the variant at +8: Enum_Comp, Int_Comp, Str_Comp at +16)
static void report_results(const char *tag)
{
    uint32_t p = g32(DHRYSTONE_SYM_Ptr_Glob), q = g32(DHRYSTONE_SYM_Next_Ptr_Glob);
    printf("[%s] Int_Glob=%u (5) Bool_Glob=%u (1) Ch_1=%c Ch_2=%c\n", tag, g32(DHRYSTONE_SYM_Int_Glob), g32(DHRYSTONE_SYM_Bool_Glob),
           low_mem[DHRYSTONE_SYM_Ch_1_Glob], low_mem[DHRYSTONE_SYM_Ch_2_Glob]);
    expect("Int_Glob", g32(DHRYSTONE_SYM_Int_Glob), 5);
    expect("Bool_Glob", g32(DHRYSTONE_SYM_Bool_Glob), 1);
    expect("Ch_1_Glob", low_mem[DHRYSTONE_SYM_Ch_1_Glob], 'A');
    expect("Ch_2_Glob", low_mem[DHRYSTONE_SYM_Ch_2_Glob], 'B');
    expect("Arr_1_Glob[8]", g32(DHRYSTONE_SYM_Arr_1_Glob + 8 * 4), 7);
    expect("Arr_2_Glob[8][7]", g32(DHRYSTONE_SYM_Arr_2_Glob_8_7), runs_now + 10);
    if (p == 0 || q == 0 || p >= 0x8000 || q >= 0x8000) { printf("[FAIL] Ptr_Glob %08X Next_Ptr_Glob %08X\n", p, q); failures++; return; }
    expect("Ptr_Glob->Ptr_Comp", g32(p), q);
    expect("Ptr_Glob->Discr", g32(p + 4), 0);
    expect("Ptr_Glob->Enum_Comp", g32(p + 8), 2);
    expect("Ptr_Glob->Int_Comp", g32(p + 12), 17);
    expect("Next_Ptr_Glob->Discr", g32(q + 4), 0);
    expect("Next_Ptr_Glob->Enum_Comp", g32(q + 8), 1);
    expect("Next_Ptr_Glob->Int_Comp", g32(q + 12), 18);
    expect("Ptr_Glob->Str_Comp", same_string((const char *)low_mem + p + 16, "DHRYSTONE PROGRAM, SOME STRING"), 1);
    expect("Next_Ptr_Glob->Str_Comp", same_string((const char *)low_mem + q + 16, "DHRYSTONE PROGRAM, SOME STRING"), 1);
}

static uint32_t run_counted(void (*hook)(void), uint32_t runs)
{
    runs_now = runs;
    cpu->prefetch_hook = (uint32_t)hook;
    start();
#ifdef BBB
    for (int n = 0; n < 4; n++) ev_before[n] = bbb_pmu_event(n);
#endif
#ifdef SAMPLE
    int sampling = cpu->dr_threshold != 0xFFFFFFFFu && runs == RUNS_B;
    if (sampling) sample_begin();
#endif
    uint32_t t0 = read_cntpct_lo();
    uint32_t vec = pjit_run((uint16_t *)DHRYSTONE_LOAD);
#ifdef SAMPLE
    if (sampling) sample_end();
#endif
    if (vec == 32 + 12) { printf("[EXC] vector %u at guest PC %08X (code offset %X)\n", vec, pjit_get_dn(0), pjit_get_dn(0) - DHRYSTONE_LOAD); failures++; }
    uint32_t t1 = read_cntpct_lo();
#ifdef BBB
    for (int n = 0; n < 4; n++) ev_delta[n] = bbb_pmu_event(n) - ev_before[n];
    return t1 - t0;                                     // cycles
#else
    return (t1 - t0) * 16;                              // ticks -> instructions at 1 ns / 62.5 MHz
#endif
}

int main(void)
{
    map_low();
    printf("=== PJIT Dhrystone, instruction counts (%d and %d runs) ===\n", RUNS_A, RUNS_B);
    pjit_cache_init((uint32_t)(ram_pool + sizeof(ram_pool)));
    cpu->trap_exit_mask = 0x8000 | 0x1000;      // TRAP #15 = done, TRAP #12 = an exception handler
    emit_opcode_table();
    uint32_t dr = RUNS_B - RUNS_A;
#ifdef TRACE_ONLY   /* the instruction stream of two run lengths, for tools/data/cycles68k.py and tools/study/pairstat.py */
    cpu->dr_threshold = 0xFFFFFFFFu;
    for (int k = 0; k < 2; k++) {
        pjit_trace_pos = 0;
        run_counted(pjit_prefetch_trace, k ? TRACE_RUNS_B : TRACE_RUNS_A);
        int n = __builtin_bswap32(pjit_trace_pos) / 8;
        printf("[TRACE] %d instructions -> %s\n", n, k ? "trace_dhrystone_b.bin" : "trace_dhrystone_a.bin");
        semihost_write_file(k ? "trace_dhrystone_b.bin" : "trace_dhrystone_a.bin", pjit_trace, n * 8);
    }
    printf("[CODE] guest code at %08X, %u bytes -> trace_dhrystone_code.bin\n", (unsigned)DHRYSTONE_LOAD, (unsigned)dhrystone_image_size);
    semihost_write_file("trace_dhrystone_code.bin", low_mem + DHRYSTONE_LOAD, dhrystone_image_size);
    return 0;
#endif
#if defined(ONLY_DR) || defined(ONLY_INTERP)   /* for tracing (tools/study/cyclerun.py): two run lengths, one mode, nothing else */
    cpu->dr_threshold = ONLY_DR_THRESHOLD;
    run_counted(pjit_prefetch, RUNS_A);
    run_counted(pjit_prefetch, RUNS_B);
    return 0;
#endif

    // ---- the opcode histogram (interpretive): 68K instructions per run ----
    cpu->dr_threshold = 0xFFFFFFFFu;
    uint64_t ta, tb;
    run_counted(pjit_prefetch_count, RUNS_A);
    report_results("count");
    ta = 0; for (int i = 0; i < 65536; i++) { pjit_hist[i] = __builtin_bswap32(pjit_hist[i]); ta += pjit_hist[i]; }
    for (int i = 0; i < 65536; i++) { hA[i] = pjit_hist[i]; pjit_hist[i] = 0; }
    run_counted(pjit_prefetch_count, RUNS_B);
    tb = 0; for (int i = 0; i < 65536; i++) { pjit_hist[i] = __builtin_bswap32(pjit_hist[i]); tb += pjit_hist[i]; }
    uint32_t per_run = (uint32_t)((tb - ta) / dr);
    printf("[HIST] 68K instructions per Dhrystone run: %u\n", per_run);
    for (int k = 0; k < HIST_TOP; k++) {
        int best = -1; uint32_t bc = 0;
        for (int i = 0; i < 65536; i++) { uint32_t c = (pjit_hist[i] - hA[i]) / dr; if (c > bc) { bc = c; best = i; } }
        if (best < 0) break;
        printf("[HIST] %04X %u\n", best, bc);
        pjit_hist[best] = hA[best];
    }

    // ---- interpretive and cached: ARM instructions per run, from the difference of two lengths ----
    uint32_t a1 = run_counted(pjit_prefetch, RUNS_A), a2 = run_counted(pjit_prefetch, RUNS_B);
    printf("[PERF] interpretive: %u " UNIT " per run = %u.%02u per 68K instruction\n", (a2 - a1) / dr,
           (a2 - a1) / dr / per_run, ((a2 - a1) / dr % per_run) * 100 / per_run);
    cpu->dr_threshold = 0;                          // everything cached: the guest is below the 16 MB that the default setting interprets
#ifdef BBB
    uint32_t eA[4], eB[4];
    uint32_t d1 = run_counted(pjit_prefetch, RUNS_A); for (int n = 0; n < 4; n++) eA[n] = ev_delta[n];
#if defined(PJIT_BLK) || defined(PJIT_TIER)
    printf("[SCHED] after the first cached run: %u regions seen, %u reordered, %u blocks translated\n", pjit_blk.n_sched_try, pjit_blk.n_sched_ok, pjit_blk.n_translated);
    printf("[FUSE] %u compare + Bcc pairs translated in natural polarity\n", (unsigned)pjit_blk.n_fused);
#endif
#if defined(PJIT_BLK) || defined(PJIT_TIER)
    uint32_t b_route0 = pjit_blk.n_route, b_one0 = pjit_blk.n_one, b_ins0 = pjit_blk.n_instr, b_tr0 = pjit_blk.n_translated;
#endif
    uint32_t d2 = run_counted(pjit_prefetch, RUNS_B); for (int n = 0; n < 4; n++) eB[n] = ev_delta[n];
#if defined(PJIT_BLK) || defined(PJIT_TIER)
#if (defined(PJIT_BLK) || defined(PJIT_TIER)) && defined(BBB)
    { extern uint32_t blk_schedlog[] __attribute__((weak)); if (blk_schedlog) for (uint32_t i = 0; i < blk_schedlog[0]; i++) printf("[SLOG] %u %u %u %08X\n", i, blk_schedlog[1 + 3 * i], blk_schedlog[2 + 3 * i], blk_schedlog[3 + 3 * i]); }
#endif
    printf("[SCHED] after the second: %u regions seen, %u reordered, %u blocks translated\n", pjit_blk.n_sched_try, pjit_blk.n_sched_ok, pjit_blk.n_translated);
#endif
#if defined(PJIT_BLK) || defined(PJIT_TIER)
    printf("[BLK] per run in steady state: %u router entries (%u of them interpret-one), %u blocks translated, %u instructions translated\n",
           (pjit_blk.n_route - b_route0) / RUNS_B, (pjit_blk.n_one - b_one0) / RUNS_B, (pjit_blk.n_translated - b_tr0) / RUNS_B, (pjit_blk.n_instr - b_ins0) / RUNS_B);
#endif
    printf("[PMU] cached, per run: ARM instructions %u, mispredicted branches %u, L1D misses %u, L1I misses %u\n",
           (eB[0] - eA[0]) / dr, (eB[1] - eA[1]) / dr, (eB[2] - eA[2]) / dr, (eB[3] - eA[3]) / dr);
#else
    uint32_t d1 = run_counted(pjit_prefetch, RUNS_A), d2 = run_counted(pjit_prefetch, RUNS_B);
#endif
    report_results("dr");
#ifdef BBB
    {   // Warm runs: the translated code stays (start() does not flush), so no translation and no first-use misses are in the window; the run length is baked into the code
        // as a constant, so every warm run is RUNS_A runs, the length of the first translation.
        uint32_t w0 = run_counted(pjit_prefetch, RUNS_A);                    // translates (RUNS_A runs)
        keep_cache = 1;
        uint32_t w1 = run_counted(pjit_prefetch, RUNS_A), w2 = run_counted(pjit_prefetch, RUNS_A), w3 = run_counted(pjit_prefetch, RUNS_A);
        keep_cache = 0;
        printf("[WARM] cycles per run, translated code kept: %u %u %u (cold %u)  ARM instructions per run %u, mispredicts %u\n", w1 / RUNS_A, w2 / RUNS_A, w3 / RUNS_A, w0 / RUNS_A,
               ev_delta[0] / RUNS_A, ev_delta[1] / RUNS_A);
    }
#endif
    printf("[PERF] cached: %u " UNIT " per run = %u.%02u per 68K instruction (cold start %u)\n", (d2 - d1) / dr,
           (d2 - d1) / dr / per_run, ((d2 - d1) / dr % per_run) * 100 / per_run, d1);
#ifdef BBB
    for (uint32_t a = DHRYSTONE_LOAD & ~511u; a < DHRYSTONE_LOAD + dhrystone_image_size; a += 512)
        printf("[XEXACT] line at %08X: %u (0 = not looked at, 1 = lazy, 2 = exact)\n", (unsigned)a, pjit_arena.xexact[(a >> 9) & 1023]);
#endif
#if defined(PJIT_BLK) || defined(PJIT_TIER)
    { printf("[BLK] %u blocks translated, %u router entries (%u interpret-one), %u links made, %u 68K instructions translated\n", pjit_blk.n_translated, pjit_blk.n_route, pjit_blk.n_one, pjit_blk.n_exits, pjit_blk.n_instr);
      printf("[FWD] slot loads %u (forwarded: %u moves, %u extends, %u deleted), slot stores %u\n", pjit_blk.n_slot_ld, pjit_blk.n_fwd_mov, pjit_blk.n_fwd_ext, pjit_blk.n_fwd_del, pjit_blk.n_slot_st); }
#endif
#ifdef SAMPLE
    sample_report();
#endif
    printf("[LINKS] %u links made, %u evictions, %u lines invalidated\n", pjit_arena.links.made, pjit_arena.links.evictions, pjit_arena.links.invalidated);
    printf(failures ? "FAIL\n" : "PASS\n");
    return failures != 0;
}
