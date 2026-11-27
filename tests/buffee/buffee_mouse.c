/// @file buffee_mouse.c
/// @brief Which game port does the mouse move? For SECONDS (default 15) prints JOY0DAT (port 1), JOY1DAT (port 2), CIA-A PRA (the fire buttons: bit 6 port 1, bit 7 port 2,
///        active low) and POTGOR (the right / middle buttons) whenever one changes, and at the end how often each changed. `MOUSE` lines.
#include <stdint.h>
#include "pjit.h"
#include "bbb.h"
#include "buffee.h"
#include "mem_be.h"

#ifndef SECONDS
#define SECONDS 15
#endif

int main(void)
{
    uint16_t j0 = read_BE_word(0xDFF00A), j1 = read_BE_word(0xDFF00C), pot = read_BE_word(0xDFF016);
    uint8_t pra = *(volatile uint8_t *)0xBFE001;
    uint32_t n0 = 0, n1 = 0, nf = 0, np = 0, lines = 0, t = bbb_pmu_cycles(), s = 0;

    printf("MOUSE move it and click now (%u s): JOY0DAT %04X JOY1DAT %04X PRA %02X POTGOR %04X\n", SECONDS, j0, j1, pra, pot);
    while (s < SECONDS) {
        uint16_t a = read_BE_word(0xDFF00A), b = read_BE_word(0xDFF00C), p = read_BE_word(0xDFF016);
        uint8_t f = *(volatile uint8_t *)0xBFE001 & 0xC0;
        int ch = 0;
        if (a != j0) { n0++; j0 = a; ch = 1; }
        if (b != j1) { n1++; j1 = b; ch = 1; }
        if (f != (pra & 0xC0)) { nf++; pra = f; ch = 1; }
        if (p != pot) { np++; pot = p; ch = 1; }
        if (ch && lines < 40) { lines++; printf("MOUSE %2u s: JOY0DAT %04X JOY1DAT %04X fire %02X POTGOR %04X\n", (unsigned)s, j0, j1, pra, pot); }
        if (bbb_pmu_cycles() - t > 1000000000u) { t += 1000000000u; s++; }
    }
    printf("MOUSE changes: JOY0DAT %u, JOY1DAT %u, fire buttons %u, POTGOR %u\n", (unsigned)n0, (unsigned)n1, (unsigned)nf, (unsigned)np);
    return 0;
}
