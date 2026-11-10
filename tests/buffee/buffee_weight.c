/// @file buffee_weight.c
/// @brief Does the chip RAM write corruption depend on how many address lines are HIGH? For each weight w = popcount(A9..A19), `N` pseudo-random word addresses with that weight (A1..A8 random): write 0x0000 then read back, and 0xFFFF then read back; counts
///        wrong reads per weight, and the same for popcount(A12..A19) and the data pattern 0x0000 / 0x00FF / 0xFFFF.
#include <stdint.h>
#include "pjit.h"
#include "bbb.h"
#include "buffee.h"
#include "mem_be.h"
#ifndef N
#define N 3000
#endif
static uint32_t rnd(void) { static uint32_t x = 12345; x = x * 1664525u + 1013904223u; return x >> 8; }
int main(void)
{
    printf("=== chip RAM write errors by address weight ===\n");
    *(volatile uint8_t *)0xBFE201 |= 3; *(volatile uint8_t *)0xBFE001 &= 0xFC;
    write_BE_word(0xDFF096, 0x7FFF);
    static const uint16_t pats[] = { 0x0000, 0x00FF, 0xFFFF };
    printf("WEIGHT w(A9..A19) | wrong reads of %d writes with data 0000 / 00FF / FFFF\n", N);
    for (int w = 1; w <= 11; w++) {
        uint32_t bad[3] = { 0, 0, 0 }, done = 0;
        while (done < N) {
            uint32_t hi = rnd() & 0x7FF;                              /* A19..A9 */
            if (__builtin_popcount(hi) != w) continue;
            uint32_t a = (hi << 9 | (rnd() & 0xFF) << 1);
            if (a < 0x400) continue;
            for (int p = 0; p < 3; p++) {
                write_BE_word(a, (uint16_t)~pats[p]);                  /* the opposite first, so a stuck word shows */
                write_BE_word(a, pats[p]);
                if (read_BE_word(a) != pats[p]) bad[p]++;
            }
            done++;
        }
        printf("WEIGHT %2d | %u %u %u\n", w, (unsigned)bad[0], (unsigned)bad[1], (unsigned)bad[2]);
    }
    *(volatile uint8_t *)0xBFE001 |= 1;
    return 0;
}
