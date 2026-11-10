/// @file buffee_top.c
/// @brief Is the top of chip RAM (where Kickstart's supervisor stack lives: $FFF00-$FFFFF) as reliable as the rest? Random 16-bit write / read-back at three regions, with the display DMA off and on,
///        plus 32-bit (two-halfword) frames like an exception frame at 4n and 4n+2. One summary line per region.
#include <stdint.h>
#include "pjit.h"
#include "bbb.h"
#include "buffee.h"
#include "mem_be.h"

#ifndef ITER
#define ITER 400000
#endif
static uint32_t seed = 12345;
static uint32_t rnd(void) { seed = seed * 1664525u + 1013904223u; return seed >> 8; }

static void region(const char *what, uint32_t base, uint32_t len)
{
    uint32_t bad = 0, firstbad = 0, badbits = 0;
    for (uint32_t n = 0; n < ITER; n++) {
        uint32_t a = base + ((rnd() % (len / 2)) * 2), v = rnd() & 0xFFFF;
        if (n & 1) { write_BE_word(a, v); uint32_t g = read_BE_word(a); if (g != v) { if (bad < 6) printf("  word @%06X wrote %04X read %04X\n", (unsigned)a, (unsigned)v, (unsigned)g); bad++; badbits |= g ^ v; if (!firstbad) firstbad = n + 1; } }
        else if (a + 4 <= base + len) {                                  /* a 6-byte exception frame: SR word, PC long (2-aligned) */
            uint32_t pc = rnd() << 8 ^ rnd();
            write_BE_word(a, v); write_BE_word(a + 2, pc >> 16); write_BE_word(a + 4 > base + len - 2 ? a : a + 4, pc & 0xFFFF);
            if (a + 4 <= base + len - 2) { uint32_t g = read_BE_word(a + 2) << 16 | read_BE_word(a + 4); if (g != pc) { if (bad < 6) printf("  frame @%06X+2 wrote %08X read %08X\n", (unsigned)a, (unsigned)pc, (unsigned)g); bad++; badbits |= g ^ pc; if (!firstbad) firstbad = n + 1; } }
        }
    }
    printf("TOPSUM %s %06X..%06X: %u bad of %u, first %u, bits %08X\n", what, (unsigned)base, (unsigned)(base + len - 1), (unsigned)bad, ITER, (unsigned)firstbad, (unsigned)badbits);
}

int main(void)
{
    printf("=== chip RAM top test ===\n");
    *(volatile uint8_t *)0xBFE201 |= 3; *(volatile uint8_t *)0xBFE001 &= 0xFC;
    /* scan: random write / read-back per 4 KB block of the whole 1 MB; blocks with errors are listed */
    {
        uint32_t tot = 0;
        printf("SCAN blocks with errors (of 256 x 4 KB, 3000 random words each):");
        for (uint32_t b = 0; b < 0x100000; b += 0x1000) {
            uint32_t bad = 0;
            for (int n = 0; n < 3000; n++) { uint32_t a = b + ((rnd() % 0x800) * 2), v = rnd() & 0xFFFF; write_BE_word(a, v); if (read_BE_word(a) != v) bad++; }
            if (bad) { printf(" %05X:%u", (unsigned)b, (unsigned)bad); tot++; }
        }
        printf("  [%u blocks]\n", (unsigned)tot);
    }
    for (int dma = 0; dma < 1; dma++) {
        write_BE_word(0xDFF096, 0x7FFF);
        if (dma > 0) { write_BE_word(0xDFF100, 0x1200); write_BE_word(0xDFF092, 0x38); write_BE_word(0xDFF094, 0xD0); write_BE_word(0xDFF08E, 0x2C81); write_BE_word(0xDFF090, 0x2CC1);
                   write_BE_word(0xDFF0E0, 0); write_BE_word(0xDFF0E2, 0x8000); write_BE_word(0xDFF096, 0x8100 | 0x0200); }
        printf("-- display DMA %s\n", dma ? "on (1 bitplane)" : "off");
        region("top 1 KB", 0xFFC00, 0x400);
        region("mid     ", 0x80000, 0x400);
        region("low     ", 0x20000, 0x400);
        region("vectors ", 0x00000, 0x400);
        region("top 256B", 0xFFF00, 0x100);
    }
    write_BE_word(0xDFF096, 0x7FFF);
    *(volatile uint8_t *)0xBFE001 |= 1;
    return 0;
}
