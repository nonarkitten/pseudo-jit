/// @file smoke_tiers.c
/// @brief One small 68000 program (sum four words with ADD.W (A0)+ / DBRA, store the sum, STOP) run in every tier the build has: interpreted, then out of the slot cache
///        (tiered and slot builds) or as blocks (BLK=1, or a tiered build with TIER_T=1), each cold (the translation included) and warm. Prints the ARM instructions each
///        run retired (the PMU's INST_RETIRED: QEMU must run with `-icount shift=0`, as tools/study/tiers.py runs it) and the host code the cache holds afterwards, as words that
///        tools/study/tiers.py disassembles. Then the same for a program that is only the STOP (what entering and leaving pjit_run costs). `tiers_mark(n)` is called between the runs so that an instruction trace can be cut into them.
#include <stdint.h>
#include "pjit.h"
#if defined(PJIT_BLK) || defined(PJIT_TIER)
#include "blk.h"
#endif

extern void qemu_exit(void);
extern void emit_opcode_table(void);

static uint8_t ram_pool[12 * 1024 * 1024] __attribute__((aligned(4096)));
static uint8_t mem[0x3000] __attribute__((aligned(4096)));            /* the program at +$1000, its data at +$2000 (the source's ORGs, relative) */
static uint8_t stack_mem[256] __attribute__((aligned(16)));

static void put16(uint8_t *p, uint16_t v) { p[0] = v >> 8; p[1] = (uint8_t)v; }
static void put32(uint8_t *p, uint32_t v) { put16(p, v >> 16); put16(p + 2, (uint16_t)v); }
static uint16_t get16(const uint8_t *p) { return (uint16_t)(p[0] << 8 | p[1]); }

__attribute__((noinline)) void tiers_mark(int n) { __asm__ volatile ("" :: "r"(n)); }

static void pmu_start(void)
{
    __asm__ volatile ("mcr p15, 0, %0, c9, c12, 0" :: "r"(7));          /* PMCR: enable, reset the counters */
    __asm__ volatile ("mcr p15, 0, %0, c9, c12, 5" :: "r"(0));          /* PMSELR: counter 0 ... */
    __asm__ volatile ("mcr p15, 0, %0, c9, c13, 1" :: "r"(0x08));       /* ... counts INST_RETIRED */
    __asm__ volatile ("mcr p15, 0, %0, c9, c12, 1" :: "r"(1));          /* PMCNTENSET */
}
static uint32_t instr(void) { uint32_t v; __asm__ volatile ("mcr p15, 0, %1, c9, c12, 5\n mrc p15, 0, %0, c9, c13, 2" : "=r"(v) : "r"(0)); return v; }

static uint8_t *const prog = mem + 0x1000, *const numbers = mem + 0x2000, *const result = mem + 0x2008;
static uint8_t *const base = mem + 0x1800;                             /* the baseline: STOP #$2700 alone, in another line (what entering and leaving pjit_run costs) */
enum { PROG_BYTES = 28 };

static void build(void)
{
    uint8_t *p = prog;
    put16(p, 0x41F9); put32(p + 2, (uint32_t)numbers); p += 6;          /* START: LEA NUMBERS,A0 */
    put16(p, 0x7000); p += 2;                                           /*        MOVEQ #0,D0 */
    put16(p, 0x323C); put16(p + 2, 3); p += 4;                          /*        MOVE.W #3,D1 */
    put16(p, 0xD058); p += 2;                                           /* LOOP:  ADD.W (A0)+,D0 */
    put16(p, 0x51C9); put16(p + 2, 0xFFFC); p += 4;                     /*        DBRA D1,LOOP */
    put16(p, 0x33C0); put32(p + 2, (uint32_t)result); p += 6;           /*        MOVE.W D0,RESULT */
    put16(p, 0x4E72); put16(p + 2, 0x2700);                             /*        STOP #$2700 */
    put16(base, 0x4E72); put16(base + 2, 0x2700);
    static const uint16_t n[4] = { 10, 20, 30, 40 };
    for (int i = 0; i < 4; i++) put16(numbers + 2 * i, n[i]);
}

static int failures;
static uint32_t run_at(uint8_t *at, const char *name, uint32_t thr, int mark)
{
    for (int i = 0; i < 8; i++) { pjit_set_dn(i, 0x5A5A0000u + i); pjit_set_an(i, 0); }
    uint32_t sp = (uint32_t)stack_mem + sizeof stack_mem - 16;
    pjit_set_an(7, sp); cpu->ssp = cpu->usp = __builtin_bswap32(sp);
    pjit_set_sr(0x2700);
    put16(result, 0);
    cpu->dr_threshold = thr;
    tiers_mark(mark);
    uint32_t i0 = instr();
    uint32_t v = pjit_run((uint16_t *)at);
    uint32_t n = instr() - i0;
    tiers_mark(mark + 1);
    if (at == base) { int ok = v == PJIT_VEC_STOP; printf("[TIERS] %-22s %6u ARM instructions %s\n", name, (unsigned)n, ok ? "ok" : "FAIL"); failures += !ok; return n; }
    int ok = v == PJIT_VEC_STOP && get16(result) == 100 && (pjit_get_dn(0) & 0xFFFF) == 100 && (pjit_get_dn(1) & 0xFFFF) == 0xFFFF;
    printf("[TIERS] %-22s %6u ARM instructions (pjit_run returned %X, RESULT %u) %s\n", name, (unsigned)n, (unsigned)v, get16(result), ok ? "ok" : "FAIL");
    failures += !ok;
    return n;
}

static uint32_t run(const char *name, uint32_t thr, int mark) { return run_at(prog, name, thr, mark); }

static void dump(const char *tag, const uint32_t *from, const uint32_t *to)
{
    for (const uint32_t *q = from; q < to; q++) printf("[%s] %08X %08X\n", tag, (unsigned)(uint32_t)q, (unsigned)*q);
}

int main(void)
{
    printf("=== the tiers on one small program ===\n");
    pjit_cache_init((uint32_t)(ram_pool + sizeof(ram_pool)));
    cpu->trap_exit_mask = 0x8000;
    emit_opcode_table();
    build();
    pmu_start();
    printf("[TIERS] program at %08X, data at %08X\n", (unsigned)(uint32_t)prog, (unsigned)(uint32_t)numbers);

    run("interpreted, first", 0xFFFFFFFFu, 10);
    run("interpreted, second", 0xFFFFFFFFu, 12);

    pjit_cache_flush();
#if defined(PJIT_BLK) || defined(PJIT_TIER)
    uint32_t c0 = pjit_blk.bc.cursor, t0 = pjit_blk.n_translated;
#endif
    run("cached, first", DR_THRESHOLD_DEFAULT, 20);
#if defined(PJIT_BLK) || defined(PJIT_TIER)
    uint32_t c1 = pjit_blk.bc.cursor, t1 = pjit_blk.n_translated;
#endif
    run("cached, second", DR_THRESHOLD_DEFAULT, 22);
    run("cached, third", DR_THRESHOLD_DEFAULT, 24);
    run_at(base, "STOP alone, interpreted", 0xFFFFFFFFu, 30);
    run_at(base, "STOP alone, cached", DR_THRESHOLD_DEFAULT, 32);
    run_at(base, "STOP alone, cached", DR_THRESHOLD_DEFAULT, 34);

#if !defined(PJIT_BLK) || defined(PJIT_TIER)
    { const uint8_t *h = (const uint8_t *)cpu->cache_data + (((uint32_t)prog & ((1u << DR_TAG_SHIFT) - 1)) << 3);
      printf("[TIERS] slots of the program (16 bytes per guest halfword) at %08X\n", (unsigned)(uint32_t)h);
      dump("SLOT", (const uint32_t *)h, (const uint32_t *)(h + PROG_BYTES * 8)); }
#endif
#if defined(PJIT_BLK) || defined(PJIT_TIER)
    printf("[TIERS] blocks translated by the first cached run: %u (ring bytes %u)\n", (unsigned)(t1 - t0), (unsigned)(c1 - c0));
    if (c1 != c0) { const uint32_t *r = (const uint32_t *)pjit_blk.bc.ring; dump("BLOCK", r + ((c0 & pjit_blk.bc.mask) >> 2), r + ((c1 & pjit_blk.bc.mask) >> 2)); }
#endif
    printf(failures ? "FAIL\n" : "PASS\n");
    qemu_exit();
    return failures != 0;
}
