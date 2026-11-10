/// @file buffee_speed.c
/// @brief Bus throughput of the Amiga window by access type: 16-bit loads, 32-bit LDR, LDRD, LDM bursts (and the same for stores), from chip RAM and ROM, with a checksum of the 16-bit
///        words so a split / reordered access shows up. Output `SPEED <what> <region>: ns per halfword, MB/s, sum ok`. Display DMA off (as buffee_d5loop).
#include <stdint.h>
#include "pjit.h"
#include "bbb.h"
#include "buffee.h"
#include "mem_be.h"
#define R32(a) (*(volatile uint32_t *)(a))
#define N 16384u            // halfwords per pass (32 KB)
#define PASSES 8u
static uint32_t sum_h(const volatile uint16_t *p) { uint32_t s = 0; for (uint32_t i = 0; i < N; i += 8) { s += p[i] + p[i+1] + p[i+2] + p[i+3] + p[i+4] + p[i+5] + p[i+6] + p[i+7]; } return s; }
static uint32_t sum_w(const volatile uint32_t *p) { uint32_t s = 0; for (uint32_t i = 0; i < N / 2; i += 4) { uint32_t a = p[i], b = p[i+1], c = p[i+2], d = p[i+3]; s += (a & 0xFFFF) + (a >> 16) + (b & 0xFFFF) + (b >> 16) + (c & 0xFFFF) + (c >> 16) + (d & 0xFFFF) + (d >> 16); } return s; }
static uint32_t hs(uint32_t a) { return (a & 0xFFFF) + (a >> 16); }
static uint32_t sum_ldm(const uint32_t *p) { uint32_t s = 0, o[8]; for (uint32_t i = 0; i < N / 2; i += 8) { __asm__ volatile("ldm %0, {r4-r11}\n\tstm %1, {r4-r11}" : : "r"(p + i), "r"(o) : "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "memory");
    s += hs(o[0]) + hs(o[1]) + hs(o[2]) + hs(o[3]) + hs(o[4]) + hs(o[5]) + hs(o[6]) + hs(o[7]); } return s; }
static uint32_t sum_ldm4(const uint32_t *p) { uint32_t s = 0, o[4]; for (uint32_t i = 0; i < N / 2; i += 4) { __asm__ volatile("ldm %0, {r4-r7}\n\tstm %1, {r4-r7}" : : "r"(p + i), "r"(o) : "r4", "r5", "r6", "r7", "memory");
    s += hs(o[0]) + hs(o[1]) + hs(o[2]) + hs(o[3]); } return s; }
static uint32_t sum_ldrd(const uint32_t *p) { uint32_t s = 0, o[2]; for (uint32_t i = 0; i < N / 2; i += 2) { __asm__ volatile("ldrd r4, r5, [%0]\n\tstrd r4, r5, [%1]" : : "r"(p + i), "r"(o) : "r4", "r5", "memory"); s += hs(o[0]) + hs(o[1]); } return s; }
static void rep(const char *what, const char *rg, uint32_t c, uint32_t ok, uint32_t bytes_per_pass)
{
    uint32_t ns = c / (PASSES * N);   // ARM cycles at 1 GHz = ns per halfword
    printf("SPEED %-8s %-5s %4u ns per halfword = %u.%02u MB/s  %s\n", what, rg, (unsigned)ns, (unsigned)(2000u / (ns ? ns : 1)), (unsigned)((200000u / (ns ? ns : 1)) % 100), ok ? "sum ok" : "SUM WRONG");
    (void)bytes_per_pass;
}
int main(void)
{
    printf("=== bus throughput by access type ===\n");
    *(volatile uint8_t *)0xBFE201 |= 3; *(volatile uint8_t *)0xBFE001 &= 0xFC;
    write_BE_word(0xDFF096, 0x7FFF);
    for (uint32_t i = 0; i < N; i++) write_BE_word(0x20000 + 2 * i, (uint16_t)(i * 2654435761u >> 11));
    static const struct { const char *n; uint32_t base; } R[] = { { "chip", 0x20000 }, { "rom", 0xF80000 } };
    for (int r = 0; r < 2; r++) {
        uint32_t base = R[r].base, ref = sum_h((volatile uint16_t *)base), t0, s;
        t0 = bbb_pmu_cycles(); s = 0; for (uint32_t p = 0; p < PASSES; p++) s = sum_h((volatile uint16_t *)base); rep("ldrh", R[r].n, bbb_pmu_cycles() - t0, s == ref, 0);
        t0 = bbb_pmu_cycles(); for (uint32_t p = 0; p < PASSES; p++) s = sum_w((volatile uint32_t *)base); rep("ldr32", R[r].n, bbb_pmu_cycles() - t0, s == ref, 0);
        t0 = bbb_pmu_cycles(); for (uint32_t p = 0; p < PASSES; p++) s = sum_ldrd((uint32_t *)base); rep("ldrd", R[r].n, bbb_pmu_cycles() - t0, s == ref, 0);
        t0 = bbb_pmu_cycles(); for (uint32_t p = 0; p < PASSES; p++) s = sum_ldm4((uint32_t *)base); rep("ldm4", R[r].n, bbb_pmu_cycles() - t0, s == ref, 0);
        t0 = bbb_pmu_cycles(); for (uint32_t p = 0; p < PASSES; p++) s = sum_ldm((uint32_t *)base); rep("ldm8", R[r].n, bbb_pmu_cycles() - t0, s == ref, 0);
    }
    // stores to chip RAM at $30000 under several write timings (CS2 CONFIG2 / 5 / 6; CS off while writing the registers)
    static const struct { const char *n; uint32_t c2, c5, c6; } V[] = {
        { "wrcyc14", 0x00131302, 0x000C141C, 0x10000000 },
        { "wrcyc10", 0x00131302, 0x000C101C, 0x10000000 },
    };
    for (unsigned v = 0; v < sizeof V / sizeof V[0]; v++) {
        R32(0x500000C0 + 4 * 6) = 0; for (volatile int w = 0; w < 20000; w++) ;
        R32(0x500000C0 + 4 * 1) = V[v].c2; R32(0x500000C0 + 4 * 4) = V[v].c5; R32(0x500000C0 + 4 * 5) = V[v].c6;
        R32(0x500000C0 + 4 * 6) = 0x00000F42; for (volatile int w = 0; w < 20000; w++) ;
        uint32_t t0, bad, c;
        for (uint32_t i = 0; i < N; i++) write_BE_word(0x30000 + 2 * i, 0);
        t0 = bbb_pmu_cycles(); for (uint32_t p = 0; p < PASSES; p++) for (uint32_t i = 0; i < N; i += 8) { volatile uint16_t *q = (volatile uint16_t *)(0x30000 + 2 * i); q[0] = i; q[1] = i + 1; q[2] = i + 2; q[3] = i + 3; q[4] = i + 4; q[5] = i + 5; q[6] = i + 6; q[7] = i + 7; }
        c = bbb_pmu_cycles() - t0; bad = sum_h((volatile uint16_t *)0x30000); rep(V[v].n, "strh", c, bad == (N - 1) * N / 2, 0);
        t0 = bbb_pmu_cycles(); for (uint32_t p = 0; p < PASSES; p++) for (uint32_t i = 0; i < N / 2; i += 4) { volatile uint32_t *q = (volatile uint32_t *)(0x30000 + 4 * i); uint32_t j = 2 * i; q[0] = ((j + 0) << 16) | (j + 1); q[1] = ((j + 2) << 16) | (j + 3); q[2] = ((j + 4) << 16) | (j + 5); q[3] = ((j + 6) << 16) | (j + 7); }
        c = bbb_pmu_cycles() - t0; bad = sum_h((volatile uint16_t *)0x30000); rep(V[v].n, "str32", c, bad == (N - 1) * N / 2, 0);
    }
    printf("SPEED done\n");
    return 0;
}
