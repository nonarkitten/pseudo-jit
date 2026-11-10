/// @file buffee_floppy.c
/// @brief Is the floppy path alive without Kickstart? Selects DF0 with the motor on and prints the drive lines (CIAA PRA: /RDY bit 5, /TK0 bit 4, /PROT bit 3, /CHNG bit 2; CIAB FLAG = index) for a few
///        seconds, steps to track 0, then reads a raw MFM track with disk DMA (sync $4489) into chip RAM and counts the sync marks. Kickstart's trackdisk needs all of this.
///        make bundle-floppy
#include <stdint.h>
#include "pjit.h"
#include "bbb.h"
#include "buffee.h"
#include "mem_be.h"

#define CW(off, v) write_BE_word(0xDFF000 + (off), (v))
#define CR(off)    read_BE_word(0xDFF000 + (off))
#define CIAA_PRA   ((volatile uint8_t *)0xBFE001)
#define CIAA_DDRA  ((volatile uint8_t *)0xBFE201)
#define CIAB_PRB   ((volatile uint8_t *)0xBFD100)
#define CIAB_DDRB  ((volatile uint8_t *)0xBFD300)
#define CIAB_ICR   ((volatile uint8_t *)0xBFDD00)
#define BUF        0x40000u

static void ms(uint32_t n) { uint32_t t0 = bbb_pmu_cycles(); while ((bbb_pmu_cycles() - t0) < n * 1000000u) ; }
static void status(const char *tag)
{
    uint8_t a = *CIAA_PRA;
    printf("  %-14s PRA %02X  /RDY %d /TK0 %d /PROT %d /CHNG %d\n", tag, a, !!(a & 0x20), !!(a & 0x10), !!(a & 8), !!(a & 4));
}
static void step(int out, int n)
{
    for (int i = 0; i < n; i++) {
        *CIAB_PRB = (*CIAB_PRB & ~0x02) | (out ? 0x02 : 0);   /* DIR: 1 = outwards (towards track 0), 0 = inwards */
        *CIAB_PRB &= ~0x01; ms(1); *CIAB_PRB |= 0x01; ms(4);
    }
}

int main(void)
{
    printf("=== floppy: DF0 status and a raw track read ===\n");
    CW(0x96, 0x7FFF);
    *CIAA_DDRA |= 3; *CIAA_PRA &= ~1;                          /* ROM overlay off: chip RAM at $0 */
    *CIAB_DDRB = 0xFF;
    *CIAA_DDRA = (*CIAA_DDRA & ~0x3C) | 0x03;                 /* PRA bits 2-5 are the drive's inputs */
    *CIAB_PRB = 0xFF;
    *CIAB_PRB = 0x7F;                                         /* motor on, nothing selected */
    *CIAB_PRB = 0x77;                                         /* /SEL0 low: the motor latches on, DF0 selected */
    for (int i = 0; i < 12; i++) { char t[24]; snprintf(t, sizeof t, "motor +%d ms", i * 250); status(t); ms(250); }
    /* index pulses: CIAB ICR bit 4 (FLAG) is set once per revolution (200 ms) */
    uint32_t idx = 0; (void)*CIAB_ICR;
    for (int i = 0; i < 100; i++) { ms(10); if (*CIAB_ICR & 0x10) idx++; }
    printf("  index pulses in 1 s: %u (expect ~5)\n", (unsigned)idx);
    step(1, 90); status("after 90 outward");
    step(0, 2);  status("2 inwards");
    step(1, 5);  status("back to 0");
    /* raw read: ADKCON MSBSYNC off, WORDSYNC on, FAST on; sync word 4489; DSKLEN twice */
    for (uint32_t i = 0; i < 0x3400; i++) write_BE_word(BUF + 2 * i, 0);
    CW(0x9E, 0x7F00); CW(0x9E, 0x8000 | 0x0400 | 0x0100);
    CW(0x7E, 0x4489);
    CW(0x20, BUF >> 16); CW(0x22, BUF & 0xFFFF);
    CW(0x9C, 0x0002);                                          /* clear DSKBLK */
    CW(0x96, 0x8000 | 0x0200 | 0x0010);                        /* DMAEN + DSKEN */
    CW(0x24, 0x8000 | 0x3400); CW(0x24, 0x8000 | 0x3400);
    uint32_t t0 = bbb_pmu_cycles(); int done = 0;
    while ((bbb_pmu_cycles() - t0) < 900u * 1000000u) { if (CR(0x1E) & 0x0002) { done = 1; break; } }
    CW(0x24, 0); CW(0x96, 0x0010);
    uint32_t syncs = 0, nonzero = 0;
    for (uint32_t i = 0; i < 0x3400; i++) { uint16_t v = read_BE_word(BUF + 2 * i); nonzero += v != 0; syncs += v == 0x4489; }
    printf("  disk DMA: %s after %u ms, DSKBYTR %04X, %u of %u words non-zero, %u sync words $4489\n", done ? "DSKBLK" : "TIMEOUT", (unsigned)((bbb_pmu_cycles() - t0) / 1000000u), (unsigned)CR(0x1A), (unsigned)nonzero, 0x3400u, (unsigned)syncs);
    printf("  first words:"); for (int i = 0; i < 12; i++) printf(" %04X", read_BE_word(BUF + 2 * i)); printf("\n");
    *CIAB_PRB = 0xFF; *CIAB_PRB = 0x7F; *CIAB_PRB = 0xFF;      /* motor off, deselect */
    return 0;
}
