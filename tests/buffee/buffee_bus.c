/// @file buffee_bus.c
/// @brief Bus-cycle generator for logic-analyser captures (tools/buffee/phase_cck.py): for ~20 s, chip RAM halfword reads and writes and DMACONR reads with pseudo-random gaps between
///        them, display DMA (5 bitplanes) optionally on. Prints "BUSLOOP start" when the loop begins and "BUSLOOP done" at the end.
#ifndef LOOPS
#define LOOPS 120000000u  /* ~1.25 us per cycle: ~2.5 min (16000000u = ~20 s, 480000000u = ~10 min) */
#endif
#include <stdint.h>
#include "pjit.h"
#include "bbb.h"
#include "buffee.h"
#include "mem_be.h"

#ifndef DISPLAY
#define DISPLAY 1
#endif
#define CW(off, v) write_BE_word(0xDFF000 + (off), (v))
#ifdef SEGMENT
/* Phase-check mode: SEGMENTS runs of LOOPS cycles; after each the bootloader recalibrates (exit 0x110: ASNCalibrate scores both clock phases and prints the choice), then calls us again. */
#define SEG ((volatile uint32_t *)0x9F5FFFD0u)                                                    /* [0] segment number, [1] read errors so far */
#endif
int main(void)
{
#ifdef SEGMENT
    uint32_t round = (*BUFFEE_MAILBOX == 1) ? SEG[0] + 1 : 0;
    if (round == 0) SEG[1] = 0;
    SEG[0] = round;
#endif
    printf("=== bus-cycle generator, display DMA %d ===\n", DISPLAY);
    { uint8_t d0 = *(volatile uint8_t *)0xBFE201, p0 = *(volatile uint8_t *)0xBFE001;
      *(volatile uint8_t *)0xBFE201 |= 3; *(volatile uint8_t *)0xBFE001 &= 0xFC;                  /* ROM overlay off */
      printf("CIAA DDRA %02X -> %02X, PRA %02X -> %02X (bit 0 must be 0); $60000 reads %04X %04X\n", d0, *(volatile uint8_t *)0xBFE201, p0, *(volatile uint8_t *)0xBFE001, read_BE_word(0x60000), read_BE_word(0x60002)); }
    CW(0x96, 0x7FFF); CW(0x9A, 0x7FFF);
    for (uint32_t i = 0; i < 0x1000; i++) write_BE_word(0x60000 + 2 * i, (uint16_t)(i * 0x9E37u));
#if DISPLAY
    CW(0x100, 0x5200); CW(0x102, 0); CW(0x104, 0); CW(0x108, 0); CW(0x10A, 0); CW(0x8E, 0x2C81); CW(0x90, 0xF4C1); CW(0x92, 0x38); CW(0x94, 0xD0);
    for (int pl = 0; pl < 5; pl++) { uint32_t pt = 0x80000 + pl * 0x2800; CW(0xE0 + 4 * pl, pt >> 16); CW(0xE2 + 4 * pl, pt & 0xFFFF); }
    for (uint32_t i = 0; i < 0x6800; i++) write_BE_word(0x80000 + 2 * i, 0x5555);
    CW(0x96, 0x8000 | 0x0200 | 0x0100);                                                           /* DMAEN | BPLEN */
#endif
    printf("BUSLOOP start (%u cycles)\n", (unsigned)LOOPS);
    uint32_t seed = 1, errs = 0, n = 0, shown = 0, xor_or = 0, prev = 0, pgap = 0;
    uint32_t trans = 0, pers = 0, other = 0, bit_t[16] = {0}, bit_p[16] = {0}, prev_t[4] = {0}, gap_t[4] = {0}, prev_p[4] = {0}, gap_p[4] = {0}, same_t = 0, ops[4] = {0}, gaps[4] = {0};
    static uint8_t wctx[0x1000];                                                                  /* per chip RAM word: what preceded its last write (previous op << 2 | gap bucket) */
    uint32_t pk = 0xFFFFFFFFu;
    for (; n < LOOPS; n++) {                                                                  /* ~1.25 us per cycle: ~20 s at 16 M (the PMU counter is 32 bits: a time limit of 20e9 cycles overflowed to 2.8 s) */
        seed = seed * 1664525u + 1013904223u;
        uint32_t k = (seed >> 8) & 0xFFF, gap = (seed >> 22) & 63, op = (seed >> 4) & 3, gb = gap == 0 ? 0 : gap < 8 ? 1 : gap < 32 ? 2 : 3;
        for (volatile uint32_t g = 0; g < gap; g++) ;
        ops[op]++; gaps[gb]++;
        switch (op) {
        case 0: { uint32_t a = 0x60000 + 2 * k; uint16_t v = read_BE_word(a), e = (uint16_t)(k * 0x9E37u);
            if (v != e) { errs++; xor_or |= v ^ e;
                uint16_t r2 = read_BE_word(a); uint16_t r3 = read_BE_word(a); const char *cls;
                if (r2 == e && r3 == e) { cls = "transient"; trans++; for (int i = 0; i < 16; i++) if ((v ^ e) >> i & 1) bit_t[i]++; prev_t[prev]++; gap_t[gb]++; if (pk == k) same_t++; }
                else if (r2 == v && r3 == v) { cls = "persistent"; pers++; for (int i = 0; i < 16; i++) if ((v ^ e) >> i & 1) bit_p[i]++; prev_p[wctx[k] >> 2]++; gap_p[wctx[k] & 3]++; write_BE_word(a, e); }
                else { cls = "other"; other++; }
                if (shown++ < 12) printf("BUSLOOP bad read @%05X: got %04X expected %04X (xor %04X), again %04X %04X: %s, prev op %u, gap bucket %u\n", (unsigned)a, v, e, v ^ e, r2, r3, cls, (unsigned)prev, (unsigned)gb); } } break;
        case 1: write_BE_word(0x60000 + 2 * k, (uint16_t)(k * 0x9E37u)); wctx[k] = (uint8_t)(prev << 2 | gb); break;
        case 2: (void)read_BE_word(0xDFF002); break;
        default: (void)read_BE_word(0xDFF006); break;
        }
        prev = op; pk = k; pgap = gb;
        if ((n & 0x7FFFFFFu) == 0x7FFFFFFu) printf("BUSLOOP progress: %u cycles, %u read errors (a clock-phase slip would show as ~5%% of chip reads wrong from then on)\n", (unsigned)(n + 1), (unsigned)errs);
    }
    (void)pgap;
    CW(0x96, 0x7FFF);
    *(volatile uint8_t *)0xBFE001 |= 1;
    printf("BUSLOOP classes: transient (second and third read right) %u, persistent (stored value wrong) %u, other %u\n", (unsigned)trans, (unsigned)pers, (unsigned)other);
    printf("BUSLOOP bits transient:"); for (int i = 0; i < 16; i++) if (bit_t[i]) printf(" b%d=%u", i, (unsigned)bit_t[i]); printf("\n");
    printf("BUSLOOP bits persistent:"); for (int i = 0; i < 16; i++) if (bit_p[i]) printf(" b%d=%u", i, (unsigned)bit_p[i]); printf("\n");
    printf("BUSLOOP transient, op before the bad read (chip read / write / DMACONR / VHPOSR): %u %u %u %u (all cycles %u %u %u %u); same address as before %u\n", (unsigned)prev_t[0], (unsigned)prev_t[1], (unsigned)prev_t[2], (unsigned)prev_t[3], (unsigned)ops[0], (unsigned)ops[1], (unsigned)ops[2], (unsigned)ops[3], (unsigned)same_t);
    printf("BUSLOOP transient, gap before it (0 / 1-7 / 8-31 / 32-63 iterations): %u %u %u %u (all %u %u %u %u)\n", (unsigned)gap_t[0], (unsigned)gap_t[1], (unsigned)gap_t[2], (unsigned)gap_t[3], (unsigned)gaps[0], (unsigned)gaps[1], (unsigned)gaps[2], (unsigned)gaps[3]);
    printf("BUSLOOP persistent, op before the write: %u %u %u %u; its gap: %u %u %u %u\n", (unsigned)prev_p[0], (unsigned)prev_p[1], (unsigned)prev_p[2], (unsigned)prev_p[3], (unsigned)gap_p[0], (unsigned)gap_p[1], (unsigned)gap_p[2], (unsigned)gap_p[3]);
    printf("BUSLOOP done: %u cycles, %u read errors, bits ever wrong %04X\n", (unsigned)n, (unsigned)errs, (unsigned)xor_or);
#ifdef SEGMENT
    SEG[1] += errs;
    printf("BUSPHASE segment %u of %u: %u cycles, %u read errors (total %u)\n", (unsigned)round, (unsigned)SEGMENTS, (unsigned)n, (unsigned)errs, (unsigned)SEG[1]);
    if (round + 1 < SEGMENTS) return BUFFEE_REROLL_EXIT(0);
    *BUFFEE_MAILBOX = 0;
    printf("BUSPHASE done: %u segments, %u read errors\n", (unsigned)SEGMENTS, (unsigned)SEG[1]);
#endif
    return 0;
}
