/// @file buffee_cia4.c
/// @brief What exactly goes wrong with bit 4 of CIAA DDRB ($BFE301; tests/buffee/buffee_shotgun.c: a 1 written there reads back 0 about half the time)? Classifies each write / read-back:
///        - transient: the first read is wrong, a second read right away is right (a read-timing problem on D4)
///        - stored:    the second read is wrong too (the write did not take, or the register changed)
///        and whether it depends on the written value, the previous value, or a gap before the read. Also tries other values (single bits) and other CIA registers that are safe to write.
#include <stdint.h>
#include "pjit.h"
#include "bbb.h"
#include "buffee.h"
#include "mem_be.h"
#ifndef N
#define N 20000
#endif
static uint32_t rnd(void) { static uint32_t x = 31337; x = x * 1664525u + 1013904223u; return x >> 8; }
#define B(a) (*(volatile uint8_t *)(a))
static void report(const char *name, volatile uint8_t *reg, uint32_t (*gen)(uint32_t), int gap)
{
    uint32_t first_bad = 0, trans = 0, stored = 0, xor_all = 0; uint32_t byval[256] = {0};
    for (uint32_t i = 0; i < N; i++) {
        uint8_t v = (uint8_t)gen(i);
        *reg = v;
        for (volatile int g = 0; g < gap; g++) ;
        uint8_t r1 = *reg, r2 = *reg;
        if (r1 != v) { first_bad++; xor_all |= v ^ r1; byval[v]++; if (r2 == v) trans++; else stored++; }
    }
    printf("CIA4 %-26s gap %3d: first read wrong %5u of %u (transient %5u, still wrong on the 2nd read %5u); bits that differ %02X\n", name, gap, (unsigned)first_bad, N, (unsigned)trans, (unsigned)stored, (unsigned)xor_all);
}
static uint32_t g_rand(uint32_t i) { (void)i; return rnd(); }
static uint32_t g_bit4(uint32_t i) { return (i & 1) ? 0x10 : 0x00; }
static uint32_t g_one4(uint32_t i) { (void)i; return 0x10; }
static uint32_t g_ff(uint32_t i) { (void)i; return 0xFF; }
static uint32_t g_walk(uint32_t i) { return 1u << (i & 7); }
int main(void)
{
    printf("=== CIA bit 4 ===\n");
    B(0xBFE201) |= 3; B(0xBFE001) &= 0xFC;
    write_BE_word(0xDFF096, 0x7FFF);
    volatile uint8_t *ddrb = (volatile uint8_t *)0xBFE301;
    report("DDRB random", ddrb, g_rand, 0);
    report("DDRB random", ddrb, g_rand, 200);
    report("DDRB alternate 00/10", ddrb, g_bit4, 0);
    report("DDRB always 10", ddrb, g_one4, 0);
    report("DDRB always FF", ddrb, g_ff, 0);
    report("DDRB walking one", ddrb, g_walk, 0);
    /* the same register read by a plain loop without the write in between */
    *ddrb = 0x10; uint32_t bad = 0; for (uint32_t i = 0; i < N; i++) if (*ddrb != 0x10) bad++;
    printf("CIA4 DDRB=10 then %u reads: %u wrong\n", N, (unsigned)bad);
    *ddrb = 0xFF; bad = 0; for (uint32_t i = 0; i < N; i++) if (*ddrb != 0xFF) bad++;
    printf("CIA4 DDRB=FF then %u reads: %u wrong\n", N, (unsigned)bad);
    *ddrb = 0x00;
    /* CIAA timer A latch low byte ($BFE401) is write-only for the latch, the counter reads back: use the CRA bit 7 safe register instead: the ICR mask */
    /* the other lane: CIAB (even addresses, D15:D8) DDRA $BFD200 bit 4 */
    volatile uint8_t *bddra = (volatile uint8_t *)0xBFD200;
    uint8_t save = *bddra;
    report("CIAB DDRA random", bddra, g_rand, 0);
    *bddra = save;
    *ddrb = 0x00;
    return 0;
}
