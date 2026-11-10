/// @file buffee_blitdur.c
/// @brief Does the CPU's polling slow the blitter down? A 64 x 64 word blit (D only, copy, 4 channels; normal and nasty) with bitplane DMA on is timed from the BLTSIZE write until BBUSY reads clear:
///        (idle) the CPU stays off the bus for a delay and reads DMACONR once (smallest delay that finds it clear, 10 us steps), (poll) the CPU reads DMACONR back to back, (chip) the CPU reads
///        chip RAM back to back and looks at DMACONR every 16 reads. The theoretical time is words x channels x 0.28 us (no bitplane DMA). "BLITDUR" lines; make bundle-blitdur
#include <stdint.h>
#include "pjit.h"
#include "bbb.h"
#include "buffee.h"
#include "mem_be.h"

#define CW(off, v) write_BE_word(0xDFF000 + (off), (v))
#define SRCA 0x80000u
#define SRCB 0x84000u
#define SRCC 0x88000u
#define DST  0x8C000u
#define NWB  4096
#ifndef ROUNDS
#define ROUNDS 5
#endif

static void start(uint16_t con0, int nasty)
{
    CW(0x96, nasty ? 0x8400 : 0x0400);
    CW(0x44, 0xFFFF); CW(0x46, 0xFFFF); CW(0x42, 0); CW(0x60, 0); CW(0x62, 0); CW(0x64, 0); CW(0x66, 0);
    CW(0x48, SRCC >> 16); CW(0x4A, SRCC & 0xFFFF); CW(0x4C, SRCB >> 16); CW(0x4E, SRCB & 0xFFFF);
    CW(0x50, SRCA >> 16); CW(0x52, SRCA & 0xFFFF); CW(0x54, DST >> 16); CW(0x56, DST & 0xFFFF);
    CW(0x40, con0);
}
static void spin_ns(uint32_t ns) { uint32_t c0 = bbb_pmu_cycles(); while (bbb_pmu_cycles() - c0 < ns) ; }

static uint32_t t_idle(uint16_t con0, int nasty)
{
    for (uint32_t d = 0; d < 20000; d += 10) {
        start(con0, nasty);
        uint32_t c0 = bbb_pmu_cycles();
        CW(0x58, 64 << 6 | 64);
        spin_ns(d * 1000);
        int busy = read_BE_word(0xDFF002) & 0x4000;
        if (!busy) return d;
        while (read_BE_word(0xDFF002) & 0x4000) ;                                              /* let it finish before the next try */
        (void)c0;
    }
    return 0xFFFFFFFF;
}
static uint32_t t_poll(uint16_t con0, int nasty, int chip)
{
    start(con0, nasty);
    uint32_t c0 = bbb_pmu_cycles(), n = 0;
    CW(0x58, 64 << 6 | 64);
    for (;;) {
        if (chip) { for (int i = 0; i < 16; i++) (void)read_BE_word(0x90000 + 2 * i); }
        if (!(read_BE_word(0xDFF002) & 0x4000)) break;
        if (++n > 5000000u) break;
    }
    return (bbb_pmu_cycles() - c0) / 1000;
}

static uint32_t t_sp(uint16_t con0, int nasty, uint32_t gap_ns)
{
    start(con0, nasty);
    uint32_t c0 = bbb_pmu_cycles();
    CW(0x58, 64 << 6 | 64);
    while (read_BE_word(0xDFF002) & 0x4000) spin_ns(gap_ns);
    return (bbb_pmu_cycles() - c0) / 1000;
}
static void sweep(const char *name, uint16_t con0)
{
    printf("BLITSWEEP %-10s normal priority, one DMACONR read every N us (0 = back to back):", name);
    static const uint32_t g[] = { 0, 500, 1000, 2000, 4000, 8000, 16000, 32000 };
    for (unsigned i = 0; i < sizeof g / sizeof g[0]; i++) { uint32_t m = ~0u; for (int r = 0; r < 4; r++) { uint32_t t = t_sp(con0, 0, g[i]); if (t < m) m = t; } printf(" %u:%u", (unsigned)(g[i] / 1000), (unsigned)m); }
    printf(" (us)\n");
}
static void one(const char *name, uint16_t con0, int nch, int nasty)
{
    uint32_t idle = t_idle(con0, nasty), poll = 0, chip = 0, pmin = ~0u, cmin = ~0u;
    for (int r = 0; r < ROUNDS; r++) {
        uint32_t p = t_poll(con0, nasty, 0), c = t_poll(con0, nasty, 1);
        poll += p; chip += c; if (p < pmin) pmin = p; if (c < cmin) cmin = c;
    }
    printf("BLITDUR %-22s theory %5u us (no display DMA): idle-then-read %5u us, DMACONR polling %5u us (min %u), chip reads + polling %5u us (min %u)\n", name,
           (unsigned)(NWB * nch * 28 / 100), (unsigned)idle, (unsigned)(poll / ROUNDS), (unsigned)pmin, (unsigned)(chip / ROUNDS), (unsigned)cmin);
}

int main(void)
{
    printf("=== blit duration vs CPU polling ===\n");
    *(volatile uint8_t *)0xBFE201 |= 3; *(volatile uint8_t *)0xBFE001 &= 0xFC;
    CW(0x96, 0x7FFF); CW(0x9A, 0x7FFF);
    for (uint32_t a = 0x70000; a < 0x70000 + 4 * 0x2000; a += 2) write_BE_word(a, 0xAAAA);
    CW(0x100, 0x4200); CW(0x102, 0); CW(0x104, 0); CW(0x108, 0); CW(0x10A, 0); CW(0x8E, 0x2C81); CW(0x90, 0x2CC1); CW(0x92, 0x38); CW(0x94, 0xD0);
    for (int p = 0; p < 4; p++) { uint32_t a = 0x70000 + p * 0x2000; CW(0xE0 + 4 * p, a >> 16); CW(0xE2 + 4 * p, a & 0xFFFF); }
    CW(0x96, 0x8000 | 0x0200 | 0x0100 | 0x0040);
    sweep("D only", 0x01F0); sweep("ABCD", 0x0FCA); sweep("D only", 0x01F0); sweep("ABCD", 0x0FCA);
    for (int r = 0; r < 0; r++) {
        one("D only, normal", 0x01F0, 1, 0);
        one("D only, nasty", 0x01F0, 1, 1);
        one("A->D copy, normal", 0x09F0, 2, 0);
        one("A->D copy, nasty", 0x09F0, 2, 1);
        one("ABCD, normal", 0x0FCA, 4, 0);
        one("ABCD, nasty", 0x0FCA, 4, 1);
    }
    CW(0x96, 0x7FFF);
    *(volatile uint8_t *)0xBFE001 |= 1;
    return 0;
}
