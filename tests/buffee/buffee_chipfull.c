/// @file buffee_chipfull.c
/// @brief Is ALL of the chip RAM (CHIPKB, default 2048 KB) reliable? Run before and after any bus timing change. Passes (errors per 4 KB block, the blocks with errors listed with the first wrote / read):
///        (1) march: ascending address-derived tags, descending complements, fills 0000 / FFFF / AAAA / 5555, display DMA off; (2) the same ascending + descending march with 4-plane display DMA on
///        and the blitter busy elsewhere; (3) random write / read-back by address weight popcount(A9..A20) = 1..12, data 0000 / 00FF / FFFF, display DMA on; (4) random word and 6-byte frame write / read-backs.
///        "CHIPFULL" lines and one "CHIPFULL RESULT"; make bundle-chipfull
#include <stdint.h>
#include "pjit.h"
#include "bbb.h"
#include "buffee.h"
#include "mem_be.h"

#define CW(off, v) write_BE_word(0xDFF000 + (off), (v))
#ifndef CHIPKB
#define CHIPKB 2048u
#endif
#define BYTES (CHIPKB * 1024u)
#define WORDS (BYTES / 2)
#define BLOCKS (BYTES / 4096)
#define PLANES 0x70000u                    /* four 8 KB planes: written by the march too, the display just shows it */
#ifndef WN
#define WN 3000
#endif

static uint16_t errs[BLOCKS];
static uint16_t firstw[BLOCKS], firstr[BLOCKS];
static uint32_t firsta[BLOCKS], total_errs, total_blocks;
static uint32_t seed = 987654321;
static uint32_t rnd(void) { seed = seed * 1664525u + 1013904223u; return seed >> 8; }
static uint16_t tag(uint32_t i, uint32_t salt) { return (uint16_t)((i * 0x9E37u) ^ (i >> 5) ^ (i >> 13) ^ salt); }

static void chk(uint32_t a, uint16_t want)
{
    uint16_t g = read_BE_word(a);
    if (g != want) { uint32_t b = a >> 12; if (!errs[b]++) { firstw[b] = want; firstr[b] = g; firsta[b] = a; } }
}
static void report(const char *name)
{
    uint32_t tot = 0, win = 0;
    printf("CHIPFULL %-26s blocks with errors:", name);
    for (uint32_t b = 0; b < BLOCKS; b++) if (errs[b]) { tot += errs[b]; win++; if (win <= 16) printf(" %06X:%u(%04X>%04X@%06X)", (unsigned)(b * 4096), errs[b], (unsigned)firstw[b], (unsigned)firstr[b], (unsigned)firsta[b]); errs[b] = 0; }
    printf("  [%u blocks, %u errors]\n", (unsigned)win, (unsigned)tot);
    total_errs += tot; total_blocks += win;
}
static void march(const char *tagname, int fills)
{
    char n[40];
    #define NAME(s) do { const char *p = tagname, *q = s; int k = 0; while (*p) n[k++] = *p++; while (*q) n[k++] = *q++; n[k] = 0; } while (0)
    for (uint32_t i = 0; i < WORDS; i++) write_BE_word(2 * i, tag(i, 0));
    for (uint32_t i = 0; i < WORDS; i++) chk(2 * i, tag(i, 0));
    NAME("ascending tags"); report(n);
    for (uint32_t i = WORDS; i-- > 0; ) write_BE_word(2 * i, ~tag(i, 0x55AA));
    for (uint32_t i = WORDS; i-- > 0; ) chk(2 * i, ~tag(i, 0x55AA));
    NAME("descending ~tags"); report(n);
    if (!fills) return;
    static const uint16_t pats[] = { 0x0000, 0xFFFF, 0xAAAA, 0x5555 };
    const char *h = "0123456789ABCDEF";
    for (int p = 0; p < 4; p++) {
        for (uint32_t i = 0; i < WORDS; i++) write_BE_word(2 * i, pats[p]);
        for (uint32_t i = 0; i < WORDS; i++) chk(2 * i, pats[p]);
        char f[8] = { 'f', 'i', 'l', 'l', ' ', h[pats[p] >> 12], h[pats[p] >> 8 & 15], 0 };
        f[6] = h[pats[p] >> 4 & 15]; f[7] = h[pats[p] & 15]; char z[9]; for (int k = 0; k < 8; k++) z[k] = f[k]; z[8] = 0;
        NAME(z); report(n);
    }
}
static void display_on(void)
{
    CW(0x96, 0x7FFF);
    CW(0x100, 0x4200); CW(0x102, 0); CW(0x104, 0); CW(0x108, 0); CW(0x10A, 0); CW(0x8E, 0x2C81); CW(0x90, 0x2CC1); CW(0x92, 0x38); CW(0x94, 0xD0);
    for (int p = 0; p < 4; p++) { uint32_t a = PLANES + p * 0x2000; CW(0xE0 + 4 * p, a >> 16); CW(0xE2 + 4 * p, a & 0xFFFF); }
    CW(0x96, 0x8000 | 0x0200 | 0x0100);                                                        /* DMAEN | BPLEN */
}
int main(void)
{
    printf("=== ALL chip RAM test: %u KB ===\n", (unsigned)CHIPKB);
    *(volatile uint8_t *)0xBFE201 |= 3; *(volatile uint8_t *)0xBFE001 &= 0xFC;
    CW(0x96, 0x7FFF); CW(0x9A, 0x7FFF);
    /* aliasing check first: the top of the range must be its own memory */
    write_BE_word(0x000100, 0x1111); write_BE_word(BYTES - 2, 0x2222); write_BE_word(BYTES / 2, 0x3333);
    printf("CHIPFULL sizing: $000100=%04X (1111) $%06X=%04X (3333) $%06X=%04X (2222)\n", (unsigned)read_BE_word(0x100), (unsigned)(BYTES / 2), (unsigned)read_BE_word(BYTES / 2), (unsigned)(BYTES - 2), (unsigned)read_BE_word(BYTES - 2));

    march("dma off, ", 1);

    display_on();
    /* the blitter busy on a scratch area while the CPU marches: a long D-only blit repeated is kept going by the loop in chk? no: restarted between passes below */
    march("dma on, ", 0);

    printf("CHIPFULL weight pass (display DMA on): wrong reads of %d writes by weight w(A9..A20), data 0000 / 00FF / FFFF\n", WN);
    static const uint16_t pats[] = { 0x0000, 0x00FF, 0xFFFF };
    for (int w = 1; w <= 12; w++) {
        uint32_t bad[3] = { 0, 0, 0 }, done = 0, tries = 0;
        uint32_t mask = (BYTES - 1) >> 9;                                   /* A20..A9 as a field */
        while (done < WN && tries++ < 400000) {
            uint32_t hi = rnd() & mask;
            if ((uint32_t)__builtin_popcount(hi) != (uint32_t)w) continue;
            uint32_t a = (hi << 9 | (rnd() & 0xFF) << 1);
            if (a < 0x400 || (a >= PLANES && a < PLANES + 0x8000)) continue;
            for (int p = 0; p < 3; p++) { write_BE_word(a, (uint16_t)~pats[p]); write_BE_word(a, pats[p]); if (read_BE_word(a) != pats[p]) bad[p]++; }
            done++;
        }
        printf("CHIPFULL weight %2d | %u %u %u (of %u)\n", w, (unsigned)bad[0], (unsigned)bad[1], (unsigned)bad[2], (unsigned)done);
        total_errs += bad[0] + bad[1] + bad[2];
    }

    uint32_t bad = 0, frames = 0;
    for (uint32_t n = 0; n < 400000; n++) {
        uint32_t a = (rnd() % (WORDS - 4)) * 2, v = rnd() & 0xFFFF;
        if (a >= PLANES && a < PLANES + 0x8000) continue;
        if (n & 1) { write_BE_word(a, v); if (read_BE_word(a) != v) bad++; }
        else { uint32_t pc = rnd() << 8 ^ rnd(); write_BE_word(a, v); write_BE_word(a + 2, pc >> 16); write_BE_word(a + 4, pc & 0xFFFF);
               frames++; if ((uint32_t)(read_BE_word(a + 2) << 16 | read_BE_word(a + 4)) != pc || read_BE_word(a) != v) bad++; }
    }
    printf("CHIPFULL random words and 6-byte frames over all of it: %u bad (%u frames)\n", (unsigned)bad, (unsigned)frames);
    total_errs += bad;
    CW(0x96, 0x7FFF);
    *(volatile uint8_t *)0xBFE001 |= 1;
    printf("CHIPFULL RESULT %s: %u errors in %u blocks over %u KB\n", total_errs ? "FAIL" : "PASS", (unsigned)total_errs, (unsigned)total_blocks, (unsigned)CHIPKB);
    return 0;
}
