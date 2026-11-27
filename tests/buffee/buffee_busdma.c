/// @file buffee_busdma.c
/// @brief Does the CPU's bus access hold up when the chipset takes most of the cycles? Random checked accesses to chip RAM, in the forms PJIT's guest code makes them (words, 6-byte
///        exception frames, longs at 4n and at 4n+2, reads of memory written before the phase), first with no DMA, then with 6 low-resolution planes and a copper list that moves a
///        colour register all frame long, then with the blitter copying 64 KB over and over beside them, normal and with BLTPRI ("nasty"), and last with no DMA again.
///        Each round ends with two phases in which every access follows an idle time of 2 to 25 us (as PJIT's accesses do, in bursts between runs of code in the DRAM),
///        with no DMA and with planes, copper and blitter.
///        A wrong value is read a second time: the same wrong value means the write went wrong, the right one a read that went wrong.
///        ROUNDS=n repeats it n times, each round first measuring the Amiga's clock against the ARM's (the beam counter over 4 s) and printing one line: the clock, its drift
///        since the first round, the GPMC cycle's margin over 4 bus clocks (the core PLL was set at boot for at least 20 ppm), and the wrong accesses of each phase.
///        GAPS=1 sweeps the time between two accesses instead (0 to 3 us in 20 ns steps, read after write, read after read and write after read, with no DMA and beside the
///        blitter) and prints the gaps at which accesses went wrong, how many and how many of them read all ones: a bus that latches at the wrong moment fails at some gaps only.
///        MARK=1 leaves a mark on the bus after every wrong access for a logic analyser to trigger on: ASn high for 300 us (no other gap in the test is that long and that short),
///        ended by a read; the wrong access is the third or fourth before it.
///        "BUSDMA" lines and one "BUSDMA RESULT"; make bundle-busdma (ROUNDS: make bundle-busdma EXTRA_CFLAGS=-DROUNDS=400)
#include <stdint.h>
#include "pjit.h"
#include "bbb.h"
#include "buffee.h"
#include "mem_be.h"

#define CW(off, v) write_BE_word(0xDFF000 + (off), (v))
#ifndef CHIPKB
#define CHIPKB 2048u
#endif
#ifndef GAPS
#define GAPS 0
#endif
#ifndef ROUNDS
#define ROUNDS 1
#endif
#ifndef MARK
#define MARK 0
#endif
#ifndef OPS
#define OPS (ROUNDS > 1 ? 100000u : 400000u)
#endif
#define COPPER  0x40000u                   /* the copper list, 52 KB */
#define BLTSRC  0x50000u                   /* the blitter copies 64 KB from here ... */
#define BLTDST  0x60000u                   /* ... to here */
#define PLANES  0x70000u                   /* six 10 KB planes */
#define READS   0x100000u                  /* 256 KB written once per phase, then only read */
/* a blit with BLTPRI ("nasty") leaves the CPU no cycle until it ends: those phases use short blits and start one only every 64 accesses */
#define READSZ  0x40000u
#define LO      0x80000u                   /* the CPU's own accesses: $80000 to the end, except READS */

static uint32_t seed = 13579;
static uint32_t rnd(void) { seed = seed * 1664525u + 1013904223u; return seed >> 8; }
static uint16_t tag(uint32_t a) { return (uint16_t)((a * 0x9E37u) ^ (a >> 7) ^ 0x5A3C); }

enum { OP_WORD, OP_FRAME, OP_LONG0, OP_LONG2, OP_READ, NOPS };
static const char *opname[NOPS] = { "word", "frame", "long 4n", "long 4n+2", "read" };
static uint32_t bad[NOPS], done[NOPS], ffff, rereadok, blits, total;
static struct { uint32_t op, a, want, got, again; } first[8];
static uint32_t nfirst;

static void gap(uint32_t n);

static void wrong(uint32_t op, uint32_t a, uint32_t want, uint32_t got, uint32_t again)
{
    if (MARK) { gap(300u * buffee_cpu_mhz()); (void)read_BE_word(READS); }
    bad[op]++;
    if ((got & 0xFFFF) == 0xFFFF || (got >> 16) == 0xFFFF) ffff++;
    if (again == want) rereadok++;
    if (nfirst < 8) { first[nfirst].op = op; first[nfirst].a = a; first[nfirst].want = want; first[nfirst].got = got; first[nfirst].again = again; nfirst++; }
}

static uint64_t now64(void);
static volatile uint32_t *L(uint32_t a) { return (volatile uint32_t *)a; }       /* a long in the Amiga's space through the ARM's own load / store (emulated, byte order of C) */

static void blit_start(uint32_t lines)
{
    CW(0x40, 0x09F0); CW(0x42, 0); CW(0x44, 0xFFFF); CW(0x46, 0xFFFF); CW(0x64, 0); CW(0x66, 0);
    CW(0x50, BLTSRC >> 16); CW(0x52, BLTSRC & 0xFFFF); CW(0x54, BLTDST >> 16); CW(0x56, BLTDST & 0xFFFF);
    CW(0x58, (uint16_t)((lines & 1023) << 6 | 32));                            /* lines of 32 words (1024: 0) */
    blits++;
}

static uint32_t idle_us;                                     /* > 0: each access of a phase follows an idle time of 2 to idle_us + 1 us */

static uint32_t phase(const char *name, uint32_t blitter)    /* blitter: the lines of each blit, 0 for none; returns the wrong accesses */
{
    uint32_t ops = idle_us ? OPS / 8 : OPS;
    uint32_t c0 = bbb_pmu_cycles(), a, v, w, g, g2;
    for (int k = 0; k < NOPS; k++) bad[k] = done[k] = 0;
    ffff = rereadok = blits = nfirst = 0;
    for (a = READS; a < READS + READSZ; a += 2) write_BE_word(a, tag(a));
    if (blitter) blit_start(blitter);
    for (uint32_t n = 0; n < ops; n++) {
        if (!(n & 255)) (void)now64();                                        /* (the 64-bit time sees every wrap of the cycle counter) */
        if (idle_us) gap((2 + rnd() % idle_us) * buffee_cpu_mhz());
        if (blitter && !(n & (blitter < 1024 ? 63 : 15)) && !(read_BE_word(0xDFF002) & 0x4000)) blit_start(blitter);
        uint32_t op = rnd() % NOPS;
        do a = (LO + (rnd() % ((CHIPKB * 1024u - LO - 8) / 2)) * 2); while (a + 8 > READS && a < READS + READSZ);
        v = rnd() << 8 ^ rnd();
        done[op]++;
        switch (op) {
        case OP_WORD:
            write_BE_word(a, v & 0xFFFF); g = read_BE_word(a);
            if (g != (v & 0xFFFF)) wrong(op, a, v & 0xFFFF, g, read_BE_word(a));
            break;
        case OP_FRAME:
            w = rnd() & 0xFFFF;
            write_BE_word(a, w); write_BE_word(a + 2, v >> 16); write_BE_word(a + 4, v & 0xFFFF);
            g = (uint32_t)read_BE_word(a + 2) << 16 | read_BE_word(a + 4); g2 = read_BE_word(a);
            if (g != v) wrong(op, a + 2, v, g, (uint32_t)read_BE_word(a + 2) << 16 | read_BE_word(a + 4));
            else if (g2 != w) wrong(op, a, w, g2, read_BE_word(a));
            break;
        case OP_LONG0:
        case OP_LONG2:
            a = (a & ~3u) + (op == OP_LONG2 ? 2 : 0);
            *L(a) = v; g = *L(a);
            if (g != v) wrong(op, a, v, g, *L(a));
            break;
        case OP_READ:
            a = READS + (rnd() % (READSZ / 2)) * 2;
            g = read_BE_word(a);
            if (g != tag(a)) wrong(op, a, tag(a), g, read_BE_word(a));
            break;
        }
    }
    while (blitter && (read_BE_word(0xDFF002) & 0x4000)) ;
    uint32_t ms = (bbb_pmu_cycles() - c0) / (buffee_cpu_mhz() * 1000u), sum = 0;
    for (int k = 0; k < NOPS; k++) sum += bad[k];
    total += sum;
    if (ROUNDS > 1 && !sum) return sum;
    printf("BUSDMA %-34s %6u ms, %u blits: %u wrong (", name, (unsigned)ms, (unsigned)blits, (unsigned)sum);
    for (int k = 0; k < NOPS; k++) printf("%s%s %u/%u", k ? ", " : "", opname[k], (unsigned)bad[k], (unsigned)done[k]);
    printf("); %u with a $FFFF half, %u right when read again\n", (unsigned)ffff, (unsigned)rereadok);
    for (uint32_t k = 0; k < nfirst; k++)
        printf("BUSDMA   %-9s at $%06X: wrote %08X read %08X again %08X\n", opname[first[k].op], (unsigned)first[k].a, (unsigned)first[k].want, (unsigned)first[k].got, (unsigned)first[k].again);
    return sum;
}

static void display_on(void)
{
    uint32_t c = COPPER;
    for (int p = 0; p < 6; p++) { uint32_t a = PLANES + p * 10240; for (uint32_t i = 0; i < 10240; i += 2) write_BE_word(a + i, (uint16_t)(rnd() & 0xFFFF)); }
    for (uint32_t line = 0x2C; line < 0x2C + 256; line++) {                    /* every line: wait for it, then 50 colour moves (the copper takes every cycle it can) */
        uint32_t y = line & 0xFF;
        if (line == 0x100) { write_BE_word(c, 0xFFDF); write_BE_word(c + 2, 0xFFFE); c += 4; }
        write_BE_word(c, (uint16_t)(y << 8 | 0x07)); write_BE_word(c + 2, 0xFFFE); c += 4;
        for (int k = 0; k < 50; k++) { write_BE_word(c, 0x180); write_BE_word(c + 2, (uint16_t)(k * 0x111 + line)); c += 4; }
    }
    write_BE_word(c, 0xFFFF); write_BE_word(c + 2, 0xFFFE);
    CW(0x100, 0x6200); CW(0x102, 0); CW(0x104, 0); CW(0x108, 0); CW(0x10A, 0);
    CW(0x8E, 0x2C81); CW(0x90, 0x2CC1); CW(0x92, 0x38); CW(0x94, 0xD0);
    for (int p = 0; p < 6; p++) { uint32_t a = PLANES + p * 10240; CW(0xE0 + 4 * p, a >> 16); CW(0xE2 + 4 * p, a & 0xFFFF); }
    CW(0x80, COPPER >> 16); CW(0x82, COPPER & 0xFFFF); CW(0x88, 0);
    CW(0x96, 0x8000 | 0x0200 | 0x0100 | 0x0080);                                /* DMAEN | BPLEN | COPEN */
}

/* The Amiga's CLK7 in mHz, measured over 4 s of whole frames of the beam counter (VPOSR / VHPOSR) against the ARM's cycle counter; both crystals' ratio is what the GPMC's
   timing rests on. A PAL frame is 312 or 313 lines (LOF) of 227 colour clocks, an NTSC one 262 or 263 of 227.5; CLK7 is two colour clocks. */
static uint64_t now64(void)                                                       /* the cycle counter in 64 bits: called at least every 4 s */
{ static uint32_t last; static uint64_t acc; uint32_t c = bbb_pmu_cycles(); acc += c - last; last = c; return acc; }
static uint32_t beam(void) { uint32_t v = read_BE_word(0xDFF004), h = read_BE_word(0xDFF006); if ((read_BE_word(0xDFF004) & 7) != (v & 7)) return beam(); return (v & 7) << 8 | h >> 8; }
static uint64_t clock_mhz(void)
{
    uint32_t prev = beam(), cur, maxv = 0;
    uint64_t t0 = 0, t1, cpu_hz = (uint64_t)buffee_cpu_mhz() * 1000000u, cck2 = 0;            /* CLK7 cycles (two colour clocks) over the whole frames */
    for (;;) { cur = beam(); if (cur <= 2 && prev >= 260 && prev <= 320) break; if (cur <= 320) prev = cur; }
    t0 = now64(); prev = 0;
    for (;;) {
        cur = beam();
        if (cur > 320) continue;                                                     /* a read that went wrong */
        if (cur < prev && !(cur <= 2 && prev >= 260)) continue;
        if (cur > maxv) maxv = cur;
        if (cur < prev) {
            t1 = now64();
            if (maxv > 300) cck2 += 2u * 227u * (maxv + 1);                       /* PAL: the frame just ended had lines 0..maxv */
            else cck2 += 455u * (maxv + 1);                                          /* NTSC: 227.5 x 2 */
            maxv = 0;
            if (t1 - t0 >= 4 * cpu_hz) { uint64_t q = cck2 * cpu_hz, dt = t1 - t0; return q / dt * 1000u + q % dt * 1000u / dt; }     /* CLK7 = 2 x CCK, in mHz */
        }
        prev = cur;
    }
}
static int32_t gpmc_margin_ppm(uint64_t clk7_mhz)
{
    uint32_t sel = *(volatile uint32_t *)BUFFEE_IO(0x44E00468u), m = sel >> 8 & 0x7FF, n = (sel & 0x7F) + 1, d = *(volatile uint32_t *)BUFFEE_IO(0x44E00480u) & 0x1F;
    uint32_t units = *(volatile uint32_t *)BUFFEE_IO(0x500000D0u) & 0x1F;
    if (!m || !d || !units) return 0x7FFFFFFF;
    uint64_t l3f_mhz = 48000000000ull * m / n / d;                              /* CORE_CLKOUTM4 in mHz (48000 kHz x M / N / D) */
    return (int32_t)((int64_t)(l3f_mhz * 1000000u / ((uint64_t)units * clk7_mhz)) - 1000000);
}

/* Waits n ARM cycles after the last access was issued (a read has finished by then; a store is posted, so the gap after it counts from its start). */
static void gap(uint32_t n) { uint32_t t = bbb_pmu_cycles(); while (bbb_pmu_cycles() - t < n) ; }
#define GAP_STEP_NS 20u
#define GAP_STEPS   150u
#define GAP_TRIES   1000u
static void gap_sweep(const char *name, uint32_t blitter)
{
    uint32_t per_ns = buffee_cpu_mhz(), tot = 0, tot_ones = 0, shown = 0;
    printf("BUSDMA gaps, %s (%u tries a gap; gap ns: wrong read-after-write / read-after-read / write-after-read, all ones):", name, (unsigned)GAP_TRIES);
    if (blitter) blit_start(blitter);
    for (uint32_t k = 0; k < GAP_STEPS; k++) {
        uint32_t g = k * GAP_STEP_NS * per_ns / 1000u, raw = 0, rar = 0, war = 0, ones = 0;
        (void)now64();
        for (uint32_t n = 0; n < GAP_TRIES; n++) {
            if (blitter && !(n & 63) && !(read_BE_word(0xDFF002) & 0x4000)) blit_start(blitter);
            uint32_t a = READS + (rnd() % (READSZ / 2 - 2)) * 2, b = a + 2, v = rnd() & 0xFFFF, r;
            write_BE_word(a, v); gap(g);
            r = read_BE_word(a); gap(g);                                              /* read after write */
            if (r != v) { raw++; ones += r == 0xFFFF; }
            r = read_BE_word(b); gap(g);                                              /* read after read */
            if (r != tag(b)) { rar++; ones += r == 0xFFFF; }
            write_BE_word(b, tag(b)); gap(g);                                         /* write after read: checked by the next read */
            r = read_BE_word(b);
            if (r != tag(b)) { war++; ones += r == 0xFFFF; write_BE_word(b, tag(b)); }
            write_BE_word(a, tag(a));
        }
        if (raw + rar + war) { if (shown++ < 40) printf(" %u: %u/%u/%u,%u", (unsigned)(k * GAP_STEP_NS), (unsigned)raw, (unsigned)rar, (unsigned)war, (unsigned)ones); }
        tot += raw + rar + war; tot_ones += ones;
    }
    while (blitter && (read_BE_word(0xDFF002) & 0x4000)) ;
    printf("%s  [%u wrong, %u all ones]\n", shown ? "" : " none", (unsigned)tot, (unsigned)tot_ones);
    total += tot;
}

int main(void)
{
    printf("=== bus under DMA: %u KB of chip RAM, %u accesses a phase, %u round(s), CPU %u MHz ===\n", (unsigned)CHIPKB, (unsigned)OPS, (unsigned)ROUNDS, (unsigned)buffee_cpu_mhz());
    *(volatile uint8_t *)0xBFE201 |= 3; *(volatile uint8_t *)0xBFE001 &= 0xFC;                  /* ROM overlay off: low memory is chip RAM */
    CW(0x96, 0x7FFF); CW(0x9A, 0x7FFF);
    for (uint32_t a = BLTSRC; a < BLTSRC + 0x10000; a += 2) write_BE_word(a, tag(a));
    display_on();
    uint64_t c0 = now64(), first_clk = 0;
    if (GAPS) {
        for (uint32_t a = READS; a < READS + READSZ; a += 2) write_BE_word(a, tag(a));
        for (uint32_t r = 0; r < ROUNDS; r++) {
            uint32_t s = (uint32_t)((now64() - c0) / ((uint64_t)buffee_cpu_mhz() * 1000000u));
            printf("BUSDMA gap round %u at %u:%02u\n", (unsigned)r + 1, (unsigned)(s / 60), (unsigned)(s % 60));
            CW(0x96, 0x7FFF);
            gap_sweep("no DMA", 0);
            CW(0x96, 0x8000 | 0x0200 | 0x0040);                                  /* DMAEN | BLTEN */
            gap_sweep("blitter", 1024);
            CW(0x96, 0x8000 | 0x0200 | 0x0100 | 0x0080);                        /* planes and copper too */
            gap_sweep("planes + copper + blitter", 1024);
            CW(0x96, 0x7FFF);
            (void)now64();
        }
    } else
    for (uint32_t r = 0; r < ROUNDS; r++) {
        uint32_t e[8];
        CW(0x96, 0x8000 | 0x0200 | 0x0100 | 0x0080);                            /* the display on while the clock is measured (the beam counts either way) */
        uint64_t clk = ROUNDS > 1 || r == 0 ? clock_mhz() : 0;
        if (!first_clk) first_clk = clk;
        CW(0x96, 0x7FFF);
        e[0] = phase("no DMA", 0);
        CW(0x96, 0x8000 | 0x0200 | 0x0100 | 0x0080);                            /* DMAEN | BPLEN | COPEN */
        e[1] = phase("6 planes + copper", 0);
        CW(0x96, 0x8000 | 0x0040);                                              /* BLTEN */
        e[2] = phase("6 planes + copper + blitter", 1024);
        CW(0x96, 0x8000 | 0x0400);                                              /* BLTPRI */
        e[3] = phase("6 planes + copper + nasty 32-line blits", 32);
        CW(0x96, 0x0100 | 0x0080);                                              /* planes and copper off: the blitter alone */
        e[4] = phase("nasty 32-line blits alone", 32);
        CW(0x96, 0x7FFF);
        e[5] = phase("no DMA again", 0);
        idle_us = 24;                                                           /* far below MARK's 300 us, stalls included */
        e[6] = phase("no DMA, idle before each access", 0);
        CW(0x96, 0x8000 | 0x0200 | 0x0100 | 0x0080 | 0x0040);
        e[7] = phase("planes + copper + blitter, idle before each access", 1024);
        CW(0x96, 0x7FFF);
        idle_us = 0;
        uint32_t s = (uint32_t)((now64() - c0) / ((uint64_t)buffee_cpu_mhz() * 1000000u));
        int32_t drift10 = (int32_t)(((int64_t)clk - (int64_t)first_clk) * 10000000 / (int64_t)first_clk);
        int32_t margin = gpmc_margin_ppm(clk);
        printf("BUSDMA round %3u %3u:%02u  CLK7 %u.%06u MHz  drift %s%d.%u ppm  GPMC margin %d ppm  wrong: none %u, planes %u, blitter %u, nasty+planes %u, nasty %u, none %u, idle %u, idle+DMA %u\n",
               (unsigned)r + 1, (unsigned)(s / 60), (unsigned)(s % 60), (unsigned)(clk / 1000000000u), (unsigned)(clk / 1000u % 1000000u),
               drift10 < 0 ? "-" : "+", (int)((drift10 < 0 ? -drift10 : drift10) / 10), (unsigned)((drift10 < 0 ? -drift10 : drift10) % 10), (int)margin,
               (unsigned)e[0], (unsigned)e[1], (unsigned)e[2], (unsigned)e[3], (unsigned)e[4], (unsigned)e[5], (unsigned)e[6], (unsigned)e[7]);
    }

    CW(0x96, 0x7FFF);
    *(volatile uint8_t *)0xBFE001 |= 1;
    printf("BUSDMA RESULT %s: %u wrong\n", total ? "FAIL" : "PASS", (unsigned)total);
    return 0;
}
