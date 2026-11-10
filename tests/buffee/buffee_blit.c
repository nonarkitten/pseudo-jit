/// @file buffee_blit.c
/// @brief Per-boot blitter health check: random H x W D-only fill blits into zeroed chip RAM, the words written are counted against H * W; a stuck BBUSY is counted and cleared with the next BLTSIZE.
///        One summary line, "BLITSUM good G wrong W hung H", for a boot sweep (make bundle-blit).
#include <stdint.h>
#include "pjit.h"
#include "bbb.h"
#include "buffee.h"
#include "mem_be.h"

#define CW(off, v) write_BE_word(0xDFF000 + (off), (v))
/* RESETS=n: after each check pulse the Amiga's RESET + HALT lines (as the bootloader's AssertReset does) and return BUFFEE_RESET_EXIT: the bootloader waits, recalibrates the bus and calls us again, n times in all */
#ifndef RESETS
#define RESETS 0
#endif
/* PASSES=n: n passes of 6000 blits in one boot (a bad boot stays bad) with a "BLITLOOP start" marker, for a logic-analyser capture */
#ifndef PASSES
#define PASSES 1
#endif
/* HUNGDIAG=1: for the first few hung blits, count the words the blit wrote, keep polling BBUSY for up to ~3 s and say whether / when it cleared by itself (DMACONR raw value too) */
#ifndef HUNGDIAG
#define HUNGDIAG 0
#endif
#define COUNT ((volatile uint32_t *)0x9F5FFFD0u)
int main(void)
{
    uint32_t round = 0;
#if RESETS
    if (*BUFFEE_MAILBOX == 1) round = *COUNT + 1; else round = 0;
    *COUNT = round;
#endif
    printf("=== blitter check, round %u ===\n", (unsigned)round);
    *(volatile uint8_t *)0xBFE201 |= 3; *(volatile uint8_t *)0xBFE001 &= 0xFC;                  /* ROM overlay off: low memory is chip RAM */
    CW(0x96, 0x7FFF); CW(0x9A, 0x7FFF); CW(0x96, 0x8000 | 0x0200 | 0x0040);                     /* DMAEN | BLTEN */
    printf("BLITLOOP start (%u passes)\n", (unsigned)PASSES);
    for (uint32_t pass = 0; pass < PASSES; pass++) {
    uint32_t seed = 12345, good = 0, bad = 0, hung = 0, first_bad = 0;
    for (uint32_t n = 0; n < 6000; n++) {
        seed = seed * 1664525u + 1013904223u; uint32_t H = 1 + (seed >> 16) % 8, W = 1 + (seed >> 24) % 8;
        for (int i = 0; i < 70; i++) write_BE_word(0x60000 + 2 * i, 0);
        CW(0x40, 0x01FF); CW(0x42, 0); CW(0x44, 0xFFFF); CW(0x46, 0xFFFF); CW(0x66, 0); CW(0x54, 0x6); CW(0x56, 0);
        CW(0x58, (uint16_t)(H << 6 | W));
        uint32_t w = 0; while ((read_BE_word(0xDFF002) & 0x4000) && w < 20000) w++;
        if (w >= 20000) { hung++; if (!first_bad) first_bad = n + 1;
#if HUNGDIAG
            if (hung <= 6) {
                uint32_t wrote = 0; for (int i = 0; i < 70; i++) if (read_BE_word(0x60000 + 2 * i) == 0xFFFF) wrote++;
                uint16_t raw = read_BE_word(0xDFF002); uint32_t c0 = bbb_pmu_cycles(), cleared = 0, ms = 0;
                while ((ms = (bbb_pmu_cycles() - c0) / 1000000u) < 3000) if (!(read_BE_word(0xDFF002) & 0x4000)) { cleared = 1; break; }
                printf("HUNG blit %u (H %u W %u, expected %u words): %u words written, DMACONR %04X, %s after %u ms more\n", (unsigned)(n + 1), (unsigned)H, (unsigned)W, (unsigned)(H * W), (unsigned)wrote, (unsigned)raw, cleared ? "CLEARED by itself" : "still busy", (unsigned)ms);
            }
#endif
            continue; }
        uint32_t cnt = 0; for (int i = 0; i < 70; i++) if (read_BE_word(0x60000 + 2 * i) == 0xFFFF) cnt++;
        if (cnt == H * W) good++; else { bad++; if (!first_bad) first_bad = n + 1; }
    }
    printf("BLITSUM round %u good %u wrong %u hung %u first_bad %u\n", (unsigned)round, (unsigned)good, (unsigned)bad, (unsigned)hung, (unsigned)first_bad);
    }
    CW(0x96, 0x7FFF);
    *(volatile uint8_t *)0xBFE001 |= 1;
#if RESETS
    if (round + 1 < RESETS) {
        volatile uint32_t *g3oe = (volatile uint32_t *)0x481AE134u, *g3do = (volatile uint32_t *)0x481AE13Cu, *g0oe = (volatile uint32_t *)0x44E07134u, *g0do = (volatile uint32_t *)0x44E0713Cu;
        *g0do &= ~(1u << 13); *g0oe &= ~(1u << 13); *g3do &= ~(1u << 16); *g3oe &= ~(1u << 16);                  /* RESET and HALT low */
        for (uint32_t c0 = bbb_pmu_cycles(); bbb_pmu_cycles() - c0 < 20u * 1000000u; ) ;                              /* ~20 ms at 1 GHz */
        *g0oe |= 1u << 13; *g3oe |= 1u << 16;                                                                         /* released */
        return BUFFEE_RESET_EXIT;
    }
    *BUFFEE_MAILBOX = 0;
#endif
    return 0;
}
