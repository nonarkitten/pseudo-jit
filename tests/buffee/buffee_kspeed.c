/// @file buffee_kspeed.c
/// @brief Where does the speed go? Three measurements on the real bus, with the Kipper2k installed (make bundle-kspeed; the region setting comes from the bootloader's RAM
///        config: run it with no region (`R` 0), with 200000/8BF and with 200000/8 and compare):
///        1. RAW  - ns per halfword by ARM access type for chip RAM, ROM and the Kipper's fast RAM at $200000 (after assigning it a base address the way expansion.library does), with checksums;
///        2. LONG - a SysInfo-style loop of `MOVE.L (A0)+,D0` over 4 KB of chip RAM, run as guest code under PJIT from different places (chip RAM, $210000: the Kipper's RAM or host DRAM
///                  depending on the region), cold (translation included) and warm; the result is also given as the speed of an uncontended MC68000 at 7.09 MHz divided by ours;
///        3. REG  - a register-only loop (4 ALU instructions + BNE), MIPS from the same places.
///        Display DMA is off (no contention), the pace governor is not installed.
#include <stdint.h>
#include "pjit.h"
#include "bbb.h"
#include "buffee.h"
#include "mem_be.h"
#include "blk.h"

extern void emit_opcode_table(void);
#define N 16384u            // halfwords per raw pass (32 KB)
#define PASSES 8u
#define CHIP_DATA   0x40000u        // the 4 KB the guest loop reads
#define KIP         0x210000u       // raw tests and guest code
#define GSTACK      0x7F000u
#define LONG_PASSES 20u
#define REG_ITER    20000u

static uint32_t sum_h(const volatile uint16_t *p) { uint32_t s = 0; for (uint32_t i = 0; i < N; i += 8) { s += p[i] + p[i+1] + p[i+2] + p[i+3] + p[i+4] + p[i+5] + p[i+6] + p[i+7]; } return s; }
static uint32_t hs(uint32_t a) { return (a & 0xFFFF) + (a >> 16); }
static uint32_t sum_ldm(const uint32_t *p) { uint32_t s = 0, o[8]; for (uint32_t i = 0; i < N / 2; i += 8) { __asm__ volatile("ldm %0, {r4-r11}\n\tstm %1, {r4-r11}" : : "r"(p + i), "r"(o) : "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "memory");
    s += hs(o[0]) + hs(o[1]) + hs(o[2]) + hs(o[3]) + hs(o[4]) + hs(o[5]) + hs(o[6]) + hs(o[7]); } return s; }

static void rep(const char *what, const char *rg, uint32_t c, uint32_t ok)
{
    uint32_t ns = c / (PASSES * N);   // ARM cycles at 1 GHz = ns per halfword
    uint32_t h = ns * 10000u / 14097u;                // hundredths of a CLK7 period (140.97 ns)
    printf("RAW %-6s %-6s %4u ns per halfword = %u.%02u clk7  %s\n", what, rg, (unsigned)ns, (unsigned)(h / 100u), (unsigned)(h % 100u), ok ? "sum ok" : "SUM WRONG");
}

static void raw(const char *rg, uint32_t base, int writable)
{
    uint32_t ref, s, t0;
    if (writable) for (uint32_t i = 0; i < N; i++) write_BE_word(base + 2 * i, (uint16_t)(i * 2654435761u >> 11));
    ref = sum_h((volatile uint16_t *)base);
    t0 = bbb_pmu_cycles(); s = 0; for (uint32_t p = 0; p < PASSES; p++) s = sum_h((volatile uint16_t *)base); rep("ldrh", rg, bbb_pmu_cycles() - t0, s == ref);
    t0 = bbb_pmu_cycles(); for (uint32_t p = 0; p < PASSES; p++) s = sum_ldm((uint32_t *)base); rep("ldm8", rg, bbb_pmu_cycles() - t0, s == ref);
    if (writable) {
        uint32_t w = base + 0x8000u;
        for (uint32_t i = 0; i < N; i++) write_BE_word(w + 2 * i, 0);
        t0 = bbb_pmu_cycles();
        for (uint32_t p = 0; p < PASSES; p++) for (uint32_t i = 0; i < N; i += 8) { volatile uint16_t *q = (volatile uint16_t *)(w + 2 * i); q[0] = i; q[1] = i + 1; q[2] = i + 2; q[3] = i + 3; q[4] = i + 4; q[5] = i + 5; q[6] = i + 6; q[7] = i + 7; }
        uint32_t c = bbb_pmu_cycles() - t0, bad = 0; for (uint32_t i = 0; i < N; i++) if (((volatile uint16_t *)w)[i] != (uint16_t)i) bad++;
        rep("strh", rg, c, bad == 0);
    }
}

/* ---- guest code ---- */
static uint32_t cbase, hw;
static void w(uint16_t v) { write_BE_word(cbase + hw * 2, v); hw++; }
static void wl(uint32_t v) { w(v >> 16); w(v & 0xFFFF); }

static uint32_t run(uint32_t entry, int cold)
{
    for (int i = 0; i < 8; i++) { pjit_set_dn(i, 0); pjit_set_an(i, 0); }
    pjit_set_an(7, GSTACK);
    cpu->ssp = cpu->usp = __builtin_bswap32(GSTACK);
    pjit_set_sr(0x2700);
    cpu->irq_level = 0; cpu->irq_flag = 0;
    cpu->tags_goto = cpu->tags_be;                       /* writing guest code into a translated page runs pjit_post_flush(), which points tags_goto at the all-miss array until the router restores it; we clear irq_flag ourselves, so restore it here */
    if (cold) pjit_cache_flush();
    uint32_t t0 = bbb_pmu_cycles();
    (void)pjit_run((uint16_t *)entry);
    return bbb_pmu_cycles() - t0;
}

/// MOVE.L (A0)+,D0 x 1024 per pass; 68000 reference per pass: 1024 x 12 + 64 DBF (10) + LEA 12 + MOVEQ 4 + SUBQ 8 + BNE 10
static void prog_long(uint32_t where)
{
    cbase = where; hw = 0;
    w(0x243C); wl(LONG_PASSES);                         /* MOVE.L #n,D2 */
    uint32_t outer = hw;
    w(0x41F9); wl(CHIP_DATA);                           /* LEA $40000.L,A0 */
    w(0x723F);                                          /* MOVEQ #63,D1 */
    uint32_t inner = hw;
    for (int i = 0; i < 16; i++) w(0x2018);             /* MOVE.L (A0)+,D0 */
    uint32_t op = hw; w(0x51C9); w((uint16_t)(2 * ((int)inner - (int)op - 1)));   /* DBF D1,inner */
    w(0x5382);                                          /* SUBQ.L #1,D2 */
    op = hw; w(0x6600); w((uint16_t)(2 * ((int)outer - (int)op - 1)));            /* BNE.W outer */
    w(0x4E4F);                                          /* TRAP #15 */
}
/// ADDQ.L #1,D1 ; ADD.L D1,D3 ; EOR.L D1,D3 ; SUBQ.L #1,D2 ; BNE.W : 5 instructions, 42 MC68000 cycles per iteration
static void prog_reg(uint32_t where)
{
    cbase = where; hw = 0;
    w(0x243C); wl(REG_ITER);                            /* MOVE.L #n,D2 */
    uint32_t l = hw;
    w(0x5281); w(0xD681); w(0xB383); w(0x5382);
    uint32_t op = hw; w(0x6600); w((uint16_t)(2 * ((int)l - (int)op - 1)));
    w(0x4E4F);
}

static void guest(const char *what, uint32_t where)
{
    /* integer math only (no FPU in the payload). CLK7 = 140.97 ns. */
    const uint32_t REF_LONG_X100 = 12962u * 14097u / 1024u;      /* an uncontended 68000: ns x 100 per MOVE.L (A0)+,D0 incl. the DBF and the outer loop */
    const uint32_t REF_MIPS_X1000 = 5u * 1000u * 1000u / 5920u;  /* 5 instructions in 42 clk = 5.92 us */
    prog_long(where);
    for (int k = 0; k < 3; k++) {
        uint32_t c = run(where, k == 0), per = c / (LONG_PASSES * 1024u), r = REF_LONG_X100 / per;
        printf("LONG %-14s %s: %8u ns total, %5u ns per MOVE.L (an uncontended 68000: %u) => %u.%02u x a 68000\n", what, k == 0 ? "cold" : "warm", (unsigned)c, (unsigned)per, (unsigned)(REF_LONG_X100 / 100u),
               (unsigned)(r / 100u), (unsigned)(r % 100u));
    }
    prog_reg(where);
    for (int k = 0; k < 3; k++) {
        uint32_t c = run(where, k == 0), m = REG_ITER * 5u * 1000u / (c / 1000u);
        printf("REG  %-14s %s: %8u ns total, %u.%03u MIPS (an uncontended 68000: %u.%03u)\n", what, k == 0 ? "cold" : "warm", (unsigned)c, (unsigned)(m / 1000u), (unsigned)(m % 1000u),
               (unsigned)(REF_MIPS_X1000 / 1000u), (unsigned)(REF_MIPS_X1000 % 1000u));
    }
}


/// exec'"'"'s resident-scan loop: CMP.W (A4)+,D2 ; DBEQ D0,loop (65536 iterations over `data`, no match). 68000: 8 + 10 = 18 clk per iteration
static void prog_scan(uint32_t where, uint32_t data)
{
    cbase = where; hw = 0;
    w(0x287C); wl(data);                                /* MOVEA.L #data,A4 */
    w(0x343C); w(0x4AFC);                               /* MOVE.W #$4AFC,D2 */
    w(0x203C); wl(0x0000FFFF);                          /* MOVE.L #$FFFF,D0 */
    w(0x2200); w(0x4841);                               /* MOVE.L D0,D1 ; SWAP D1 */
    w(0x6002);                                          /* BRA.S +2 */
    w(0xB45C);                                          /* L1: CMP.W (A4)+,D2 */
    w(0x57C8); w(0xFFFC);                               /* L2: DBEQ D0,L1 */
    w(0x57C9); w(0xFFF8);                               /*     DBEQ D1,L1 */
    w(0x4E4F);                                          /* TRAP #15 */
}
static void scan(const char *what, uint32_t where, uint32_t data)
{
    for (uint32_t i = 0; i < 65536; i++) write_BE_word(data + 2 * i, 0x1234);
    prog_scan(where, data);
    for (int k = 0; k < 3; k++) {
        uint32_t c = run(where, k == 0), per = c / 65536u;
        printf("SCAN %-14s data %06X %s: %9u ns total, %5u ns per CMP.W+DBEQ iteration (an uncontended 68000: 2538)\n", what, (unsigned)data, k == 0 ? "cold" : "warm", (unsigned)c, (unsigned)per);
    }
}


/// the same 65536-iteration scan closed three ways: DBEQ (a real condition), DBF (always false = a plain counter) and SUBQ/BNE
static void prog_scan_v(uint32_t where, uint32_t data, int v)
{
    cbase = where; hw = 0;
    w(0x287C); wl(data);                                /* MOVEA.L #data,A4 */
    w(0x343C); w(0x4AFC);                               /* MOVE.W #$4AFC,D2 */
    w(0x203C); wl(v == 2 ? 0x00010000 : 0x0000FFFF);   /* MOVE.L #count,D0 */
    uint32_t l1 = hw;
    w(0xB45C);                                          /* L1: CMP.W (A4)+,D2 */
    if (v == 0) { uint32_t op = hw; w(0x57C8); w((uint16_t)(2 * ((int)l1 - (int)op - 1))); }          /* DBEQ D0,L1 */
    if (v == 1) { uint32_t op = hw; w(0x51C8); w((uint16_t)(2 * ((int)l1 - (int)op - 1))); }          /* DBF D0,L1 */
    if (v == 2) { w(0x5380); uint32_t op = hw; w(0x6600); w((uint16_t)(2 * ((int)l1 - (int)op - 1))); }  /* SUBQ.L #1,D0 ; BNE.W L1 */
    w(0x4E4F);                                          /* TRAP #15 */
}
static void scan_v(uint32_t where, uint32_t data)
{
    static const char *nm[] = { "DBEQ D0 ", "DBF D0  ", "SUBQ+BNE" };
    for (uint32_t i = 0; i < 65536; i++) write_BE_word(data + 2 * i, 0x1234);
    for (int v = 0; v < 3; v++) {
        prog_scan_v(where, data, v);
        uint32_t b0 = pjit_blk.n_translated, t0 = pjit_blk.tier_builds, r0 = pjit_blk.n_route, o0 = pjit_blk.n_one;
        (void)run(where, 1); uint32_t c = run(where, 0);
        printf("LOOP %s code %06X data %06X: %9u ns total, %4u ns per iteration; blocks translated %u, tier builds %u, router %u, interpret-one %u\n", nm[v], (unsigned)where, (unsigned)data, (unsigned)c, (unsigned)(c / 65536u),
               (unsigned)(pjit_blk.n_translated - b0), (unsigned)(pjit_blk.tier_builds - t0), (unsigned)(pjit_blk.n_route - r0), (unsigned)(pjit_blk.n_one - o0));
    }
}


/// DBF loop shapes: which ones stay slow? (65536 iterations each)
static void shape(const char *name, uint32_t where, const uint16_t *body, int nbody, int creg, uint16_t closer)
{
    cbase = where; hw = 0;
    w(0x287C); wl(0x230000);                            /* MOVEA.L #$230000,A4 */
    w(0x343C); w(0x4AFC);                               /* MOVE.W #$4AFC,D2 */
    w((uint16_t)(0x203C | (creg << 9))); wl(0xFFFF);    /* MOVE.L #$FFFF,Dc */
    uint32_t l1 = hw;
    for (int i = 0; i < nbody; i++) w(body[i]);
    uint32_t op = hw; w(closer); w((uint16_t)(2 * ((int)l1 - (int)op - 1)));
    w(0x4E4F);
    uint32_t b0 = pjit_blk.n_translated, t0 = pjit_blk.tier_builds, r0 = pjit_blk.tier_rebuilds, n0 = pjit_blk.n_route, o0 = pjit_blk.n_one;
    (void)run(where, 1); uint32_t c = run(where, 0);
    printf("SHAPE %-34s: %6u ns per iteration; blocks translated %u, tier builds %u rebuilds %u pause %u, router %u, interpret-one %u (raw counters, deltas)\n", name, (unsigned)(c / 65536u),
           (unsigned)(pjit_blk.n_translated - b0), (unsigned)(pjit_blk.tier_builds - t0), (unsigned)(pjit_blk.tier_rebuilds - r0), (unsigned)pjit_blk.tier_pause, (unsigned)(pjit_blk.n_route - n0), (unsigned)(pjit_blk.n_one - o0));
}
extern uint32_t pjit_tier_cnt[];
static void dump_cnt(uint32_t g)
{
    uint32_t ix = (g >> 1) & 0x3FF;
    printf("TIERCNT entry %u for %06X (raw words: target %08X count %08X; byte-swapped: target %08X count %08X)\n", (unsigned)ix, (unsigned)g, (unsigned)pjit_tier_cnt[2 * ix], (unsigned)pjit_tier_cnt[2 * ix + 1],
           (unsigned)__builtin_bswap32(pjit_tier_cnt[2 * ix]), (unsigned)__builtin_bswap32(pjit_tier_cnt[2 * ix + 1]));
}
static void dump_slots(uint32_t g, int n)
{
    volatile uint32_t *p = (volatile uint32_t *)((uint8_t *)cpu->cache_data + ((g & 0x7FFFFu) << 3));
    for (int k = 0; k < n; k++) { printf("SLOT %06X:", (unsigned)(g + 2 * k)); for (int i = 0; i < 4; i++) printf(" %08X", (unsigned)p[4 * k + i]); printf("\n"); }
}
static void shapes(uint32_t where)
{
    for (uint32_t i = 0; i < 65536; i++) write_BE_word(0x230000 + 2 * i, 0x1234);
    static const uint16_t cmp[] = { 0xB45C }, addq[] = { 0x5283 }, add2[] = { 0x5283, 0x5284 }, nop[] = { 0x4E71 }, mv[] = { 0x361C };
    shape("CMP.W (A4)+,D2 ; DBF D0", where, cmp, 1, 0, 0x51C8);
    shape("CMP.W (A4)+,D2 ; DBF D1", where, cmp, 1, 1, 0x51C9);
    shape("CMP.W (A4)+,D2 ; DBF D5", where, cmp, 1, 5, 0x51CD);
    shape("MOVE.W (A4)+,D3 ; DBF D1", where, mv, 1, 1, 0x51C9);
    shape("ADDQ.L #1,D3 ; DBF D0", where, addq, 1, 0, 0x51C8);
    shape("ADDQ.L #1,D3 ; DBF D1", where, addq, 1, 1, 0x51C9);
    shape("ADDQ.L #1,D3 ; ADDQ.L #1,D4 ; DBF D1", where, add2, 2, 1, 0x51C9);
    shape("NOP ; DBF D1", where, nop, 1, 1, 0x51C9);
    printf("SLOTS of NOP ; DBF D1 (L1 = %06X, DBF at +2, displacement at +4, TRAP at +6; each guest halfword is one 16-byte slot):\n", (unsigned)(where + 0x10));
    dump_slots(where + 0x10, 5);
    dump_cnt(where + 0x10);
    shape("NOP ; DBEQ D1", where, nop, 1, 1, 0x57C9);
}

int main(void)
{
    const volatile uint8_t *bi = (const volatile uint8_t *)0x9F5FFF00u;
    printf("=== kspeed: Kipper RAM, chip RAM, guest code by place ===\n");
    printf("KSPEED regions from the bootloader:");
    if (*(const volatile uint32_t *)bi == 0x544F4F42u) for (int k = 0; k < 4; k++) { unsigned mb = bi[16 + 4 * k] | bi[17 + 4 * k] << 8; if (bi[18 + 4 * k]) printf(" %X/%uMB flags %u;", mb << 20, 1u << (bi[18 + 4 * k] - 20), bi[19 + 4 * k]); }
    printf(" (none = all on the bus)\n");
    *(volatile uint8_t *)0xBFE201 |= 3; *(volatile uint8_t *)0xBFE001 &= 0xFC;       /* ROM overlay off */
    write_BE_word(0xDFF096, 0x7FFF);                                                  /* DMA off */
    uint16_t t0 = read_BE_word(0xE80000);
    int board = (t0 >> 12) == 0xE;
    printf("KSPEED autoconfig $E80000 = %04X (%s)\n", t0, board ? "a board answers" : "no board");
    if (board) {
        *(volatile uint8_t *)0xE8004A = 0x00; *(volatile uint8_t *)0xE80048 = 0x20;            /* assign $200000, as expansion.library does */
        for (volatile int wt = 0; wt < 3000000; wt++) ;
    }
    for (uint32_t i = 0; i < 1024; i++) { write_BE_word(CHIP_DATA + 4 * i, (uint16_t)i); write_BE_word(CHIP_DATA + 4 * i + 2, (uint16_t)~i); }
    raw("chip", 0x20000, 1);
    raw("rom", 0xF80000, 0);
    raw("kipper", KIP, 1);

    pjit_cache_init(BUFFEE_CACHE_TOP);
    cpu->trap_exit_mask = 0x8000;
    emit_opcode_table();
    cpu->dr_threshold = 0x1000;                                                       /* the memory map decides what is translated */
    guest("code chip $40000+", 0x60000);                                              /* chip RAM (the data block is at $40000-$40FFF) */
    guest("code $210000", KIP);                                                       /* Kipper RAM on the bus, or host DRAM if a region covers it */
    scan("code $210000", KIP, 0x230000);                                               /* data in the same zone (host DRAM: no bus at all; Kipper: the bus) */
    scan("code $210000", KIP, 0x80000);                                                /* data in chip RAM (the bus) */
    scan("code chip", 0x60000, 0x80000);
    scan_v(KIP, 0x230000);
    shapes(KIP);
    *(volatile uint8_t *)0xBFE001 |= 1;
    printf("KSPEED done\n");
    return 0;
}
