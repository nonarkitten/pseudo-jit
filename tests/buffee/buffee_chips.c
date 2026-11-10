/// @file buffee_chips.c
/// @brief Custom-chip register reads over the Buffee window: chip IDs (VPOSR Agnus, DENISEID $DFF07C), read stability of registers that must not change (INTENAR, ADKCONR, DENISEID, JOY1DAT, POTGOR), then JOY0DAT (the mouse
///        port) in a hands-off phase and a moving phase (the console prints the phase): a mouse changes the counters in steps of one count per axis, so big jumps between consecutive reads mean bad reads or noise.
///        make bundle-chips
#include <stdint.h>
#include "pjit.h"
#include "bbb.h"
#include "buffee.h"
#include "mem_be.h"

#define CR(off) read_BE_word(0xDFF000 + (off))
static void ms(uint32_t n) { uint32_t t0 = bbb_pmu_cycles(); while ((bbb_pmu_cycles() - t0) < n * 1000000u) ; }

static void stable(const char *name, uint32_t off)
{
    uint16_t v[8]; uint32_t cnt[8] = { 0 }; int nv = 0; uint32_t other = 0;
    for (uint32_t i = 0; i < 300000; i++) {
        uint16_t r = CR(off); int k;
        for (k = 0; k < nv; k++) if (v[k] == r) break;
        if (k == nv) { if (nv < 8) { v[nv] = r; cnt[nv++] = 1; } else other++; } else cnt[k]++;
    }
    printf("  %-9s ($%03X): %d distinct value(s):", name, (unsigned)off, nv + (other != 0));
    for (int k = 0; k < nv; k++) printf(" %04X x%u", v[k], (unsigned)cnt[k]);
    if (other) printf(" (+%u more)", (unsigned)other);
    printf("\n");
}

static void joy_seconds(uint32_t secs)
{
    printf("[JOY] move the mouse continuously; one line per second\n");
    uint16_t last = CR(0x0A);
    for (uint32_t sec = 0; sec < secs; sec++) {
        uint32_t t0 = bbb_pmu_cycles(), n = 0, changes = 0, small = 0, big = 0, maxjump = 0, lmb = 0; int32_t sx = 0, sy = 0;
        while (bbb_pmu_cycles() - t0 < 1000u * 1000000u) {
            uint16_t v = CR(0x0A); n++;
            if (v != last) {
                int dx = (int8_t)((v & 255) - (last & 255)), dy = (int8_t)((v >> 8) - (last >> 8));
                uint32_t m = (uint32_t)((dx < 0 ? -dx : dx) > (dy < 0 ? -dy : dy) ? (dx < 0 ? -dx : dx) : (dy < 0 ? -dy : dy));
                if (m <= 1) small++; else big++;
                if (m > maxjump) maxjump = m;
                sx += dx; sy += dy; changes++; last = v;
            }
            if (!(*(volatile uint8_t *)0xBFE001 & 0x40)) lmb++;
        }
        printf("[JOY] t=%2u: %u reads, %u changes (%u of 1 count, %u bigger, largest jump %u), net dx %d dy %d, left button down in %u reads\n", (unsigned)sec, (unsigned)n, (unsigned)changes, (unsigned)small, (unsigned)big, (unsigned)maxjump, (int)sx, (int)sy, (unsigned)lmb);
    }
}

int main(void)
{
    printf("=== custom chip reads ===\n");
    *(volatile uint8_t *)0xBFE201 |= 3; *(volatile uint8_t *)0xBFE001 &= ~1;       /* overlay off */
    uint16_t vp = CR(0x04), id = CR(0x7C);
    printf("VPOSR %04X (Agnus id %02X: 20 = ECS PAL 1MB, 00 = OCS PAL, 10 = OCS NTSC, 30 = ECS NTSC), DENISEID %04X (low byte FC = ECS Denise 8373, FF = OCS / no answer)\n", vp, (vp >> 8) & 0x7F, id);
    printf("ID probes: $07C word %04X, bytes %02X %02X, long at $07C %08X; reserved: $07E %04X $0FE %04X $1FC %04X $1FE %04X; $0DE %04X $0E0..: %04X; AGNUS $004 %04X $006 %04X\n", CR(0x7C), *(volatile uint8_t *)0xDFF07C, *(volatile uint8_t *)0xDFF07D, (unsigned)(read_BE_word(0xDFF07C) << 16 | read_BE_word(0xDFF07E)), CR(0x7E), CR(0xFE), CR(0x1FC), CR(0x1FE), CR(0xDE), CR(0xE0), CR(0x04), CR(0x06));
    stable("INTENAR", 0x1C); stable("ADKCONR", 0x10); stable("DENISEID", 0x7C); stable("JOY1DAT", 0x0C); stable("POTGOR", 0x16);
    joy_seconds(30);
    return 0;
}
