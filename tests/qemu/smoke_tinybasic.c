/// @file smoke_tinybasic.c
/// @brief A BASIC interpreter (test_progs/tinybasic.c, a 5 KB 68000 image built by tools/data/tinybasic.py) running on PJIT, interpretively and out of the
///        DR cache. Its console is three routines at fixed guest addresses (TINYBASIC_BIOS_*), which this harness fills with `TRAP #n ; RTS` and plays itself:
///        character out <- D0, character in -> D0 (0 when nothing is waiting). A BASIC program goes in as keystrokes, `BYE` ends the run (the image's
///        `TRAP #14`). Guest address 0 is real RAM through a 1 MB section remap (QEMU: a page table of our own; BeagleBone: bbb_map_low()).
///        With BBB defined it times the cached run in cycles (several programs: tests/bbb/bbb_tinybasic.c). golden/tb_oracle.c runs the same image on the oracle.
#include <stdint.h>
#include "pjit_libc.h"
#include "pjit.h"
#include "mem_be.h"
#include "cp15.h"
#include "tinybasic_image.h"
#ifdef BBB
#include "bbb.h"
#endif

extern void qemu_exit(void);
extern void emit_opcode_table(void);

static uint8_t ram_pool[12 * 1024 * 1024] __attribute__((aligned(4096)));
static uint8_t low_mem[1024 * 1024] __attribute__((aligned(1024 * 1024)));      // what guest addresses 0..0xFFFFF are

#ifndef BBB
static uint32_t mmu_pagetable[4096] __attribute__((aligned(16384)));
static void map_low(void)
{
    for (uint32_t i = 0; i < 4096; i++) mmu_pagetable[i] = ((i << 20) & 0xFFF00000u) | 0xC0Eu;
    mmu_pagetable[0] = ((uint32_t)low_mem & 0xFFF00000u) | 0xC0Eu;
    CP15TtbCtlTtb0Config(); CP15Ttb0Set((uint32_t)mmu_pagetable); CP15DomainAccessClientSet(); CP15TlbInvalidate(); CP15MMUEnable();
}
#else
static void map_low(void) { bbb_map_low((uint32_t)low_mem); }
#endif

#ifndef MODE0
#define MODE0 0
#endif
#ifndef PROGRAM
#define PROGRAM "10 FOR I=1 TO 10\r20 LET S=S+I\r30 NEXT I\r40 PRINT S\rRUN\rBYE\r"
#endif
#ifndef EXPECT
#define EXPECT "55"
#endif
#ifdef HIST      /* the interpretive run counts every opcode (as smoke_dhrystone does): `[HIST] oooo count` lines for tools/study/profile_cost.py */
uint32_t pjit_hist[65536];
extern void pjit_prefetch_count(void);
#endif
#ifdef TRACE2    /* the opcode stream of the BASIC program's run (from the moment RUN has been typed): trace_tinybasic.bin, for tools/study/regreuse.py */
#define TRACE2_MAX (8 * 1024 * 1024)
uint32_t pjit_trace2[TRACE2_MAX / 4];
uint32_t pjit_trace2_pos, pjit_trace2_max = __builtin_bswap32(TRACE2_MAX);   /* (read by big-endian code) */
extern void pjit_prefetch_trace2(void);
extern int semihost_write_file(const char *path, const void *buf, unsigned len);
#endif
static const char *keys; static char outbuf[4096]; static unsigned outn;

static void put_be16(uint32_t a, uint16_t v) { low_mem[a] = v >> 8; low_mem[a + 1] = (uint8_t)v; }

static void load_guest(void)
{
    memset(low_mem, 0, sizeof low_mem);
    memcpy(low_mem + TINYBASIC_LOAD, tinybasic_image, tinybasic_image_size);
    put_be16(TINYBASIC_BIOS_PUTC, 0x4E41); put_be16(TINYBASIC_BIOS_PUTC + 2, 0x4E75);      // console out:  TRAP #1 ; RTS
    put_be16(TINYBASIC_BIOS_GETC, 0x4E42); put_be16(TINYBASIC_BIOS_GETC + 2, 0x4E75);      // console in:   TRAP #2 ; RTS
    put_be16(TINYBASIC_BIOS_EXIT, 0x4E4E); put_be16(TINYBASIC_BIOS_EXIT + 2, 0x4E75);      // exit:         TRAP #14 ; RTS
    CP15DCacheCleanFlushBuff((uint32_t)low_mem, 4096);
    CP15ICacheFlush();
}

static int keep_cache;                              // 1: leave the translated blocks in place (a warm run: no translation in the timed window)
/// one run of the program; returns the console output length
static unsigned run_guest(uint32_t threshold, uint32_t *cycles)
{
    load_guest();
    cpu->dr_threshold = threshold;
    cpu->trap_exit_mask = 0xFFFF;
    if (!keep_cache) pjit_cache_flush();
#ifdef HIST
    if (threshold == 0xFFFFFFFFu) cpu->prefetch_hook = (uint32_t)pjit_prefetch_count;
#endif
    for (int i = 0; i < 8; i++) { pjit_set_dn(i, 0); pjit_set_an(i, 0); }
    pjit_set_sr(0x2700);
    keys = PROGRAM; outn = 0;
    uint32_t pc = TINYBASIC_LOAD, t0 = 0; int typing = 0;
#ifdef BBB
    t0 = bbb_pmu_cycles();
    uint32_t in_guest = 0, ev[4] = { 0, 0, 0, 0 };
#endif
    for (;;) {
#ifdef BBB
        uint32_t g0 = bbb_pmu_cycles(), e0[4]; for (int e = 0; e < 4; e++) e0[e] = bbb_pmu_event(e);
#endif
#ifdef TRACE2
        if (threshold == 0xFFFFFFFFu && keys[0] == 'B' && keys[1] == 'Y') cpu->prefetch_hook = (uint32_t)pjit_prefetch_trace2;      // RUN has been typed: record
#endif
#ifdef SAMPLE
        if (sample_enable) sample_begin();
#endif
        uint32_t vec = pjit_run((uint16_t *)pc);
#ifdef SAMPLE
        if (sample_enable) sample_end();
#endif
#ifdef BBB
        in_guest += bbb_pmu_cycles() - g0; for (int e = 0; e < 4; e++) ev[e] += bbb_pmu_event(e) - e0[e];
#endif
        pc = (uint32_t)cpu->pc;
        uint32_t n = vec - 32;
#ifdef TB_TRACE
        { static int k; if (k++ < 60) printf("[T] trap %u pc %05X d0 %02X sr %04X a7 %X\n", (unsigned)n, (unsigned)pc, (unsigned)pjit_get_dn(0) & 0xFF, (unsigned)pjit_get_sr(), (unsigned)pjit_get_an(7)); }
#endif
        if (n == 1) { if (outn < sizeof outbuf - 1) outbuf[outn++] = (char)(pjit_get_dn(0) & 0x7F); }
        // A line is typed only once the prompt ("> ") is up, as at a real terminal
        else if (n == 2 && *keys && outn >= 2 && outbuf[outn - 2] == '>' && outbuf[outn - 1] == ' ') {
            uint32_t c = (uint8_t)*keys++; pjit_set_dn(0, (pjit_get_dn(0) & ~0xFFu) | c);
            if (c == '\r') typing = 0; else typing = 1;
        }
        else if (n == 2 && *keys && typing) { uint32_t c = (uint8_t)*keys++; pjit_set_dn(0, (pjit_get_dn(0) & ~0xFFu) | c); if (c == '\r') typing = 0; }
        else if (n == 2) pjit_set_dn(0, pjit_get_dn(0) & ~0xFFu);   // nothing waiting: D0.B = 0
        else break;                                             // TRAP #14: BYE
    }
#ifdef BBB
    *cycles = bbb_pmu_cycles() - t0;
    printf("[PMU] ARM instructions %u, mispredicted branches %u, L1I misses %u, L1D misses %u\n", (unsigned)ev[0], (unsigned)ev[1], (unsigned)ev[2], (unsigned)ev[3]);
    printf("[CYCLES] in pjit_run (the guest, the translator, the trap entry and exit): %u\n", (unsigned)in_guest);
#else
    *cycles = 0; (void)t0;
#endif
    outbuf[outn] = 0;
    return outn;
}

int main(void)
{
    printf("=== PJIT smoke test: Tiny BASIC (68000) ===\n");
    pjit_cache_init((uint32_t)(ram_pool + sizeof(ram_pool)));
    emit_opcode_table();
    map_low();
#ifdef BBB
    { static const uint8_t events[4] = { PMU_INSTR, PMU_BR_MISPRED, PMU_L1I_MISS, PMU_L1D_MISS }; bbb_pmu_start(events); }
#endif
    int failures = 0;
    for (int mode = MODE0; mode < 2; mode++) {
        uint32_t cyc;
        run_guest(mode ? 0u : 0xFFFFFFFFu, &cyc);
        printf("[%s] %u characters of console output%s\n", mode ? "cached" : "interp", outn, cyc ? "" : "");
#ifdef BBB
        printf("[CYCLES] %s %u\n", mode ? "cached" : "interp", (unsigned)cyc);
#endif
        for (unsigned i = 0; i < outn; i++) { char c = outbuf[i]; if (c == '\r') continue; printf("%c", c); }
        printf("\n");
#ifdef TRACE2
        if (mode == 0) { unsigned n = __builtin_bswap32(pjit_trace2_pos); printf("[TRACE] %u bytes (%u instructions)\n", n, n / 8); for (unsigned i = 0; i < n / 4; i++) pjit_trace2[i] = __builtin_bswap32(pjit_trace2[i]); semihost_write_file("trace_tinybasic.bin", pjit_trace2, n); }
#endif
#ifdef HIST
        if (mode == 0) {
            uint32_t total = 0;
            for (int i = 0; i < 65536; i++) { pjit_hist[i] = __builtin_bswap32(pjit_hist[i]); total += pjit_hist[i]; }
            printf("[HIST] 68K instructions in the whole run: %u\n", total);
            for (int k = 0; k < 400; k++) {
                uint32_t bc = 0; int best = -1;
                for (int i = 0; i < 65536; i++) if (pjit_hist[i] > bc) { bc = pjit_hist[i]; best = i; }
                if (best < 0) break;
                printf("[HIST] %04X %u\n", best, bc); pjit_hist[best] = 0;
            }
        }
#endif
        if (!strstr(outbuf, EXPECT)) { printf("[TEST] expected \"%s\" in the output FAIL\n", EXPECT); failures++; }
    }
#ifdef BBB
    for (int rep = 0; rep < 8; rep++) {
#ifdef SAMPLE
        sample_enable = rep == 7;
#endif
        uint32_t cyc; run_guest(0u, &cyc); printf("[REPEAT] cached run %d: %u cycles in the whole run\n", rep, (unsigned)cyc); }
#ifndef SAMPLE
    keep_cache = 1;                                 // the blocks of the last run stay: only the guest runs now
    for (int rep = 0; rep < 4; rep++) { uint32_t cyc; run_guest(0u, &cyc); printf("[WARM] cached run %d: %u cycles with the blocks already translated\n", rep, (unsigned)cyc); }
#endif
#ifdef SAMPLE
    sample_report();
#endif
#endif
    printf(failures ? "FAIL\n" : "PASS\n");
    qemu_exit();
    return failures != 0;
}
