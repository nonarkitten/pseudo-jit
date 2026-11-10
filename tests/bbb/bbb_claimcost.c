/// @file bbb_claimcost.c
/// @brief Where `claim_line`'s ~16,000 cycles go: the same steps on a 4 KB region, timed one by one (1,024 stores; 64 x DCCMVAC; 64 x ICIMVAU; BPIALL + isb; a scan loop
///        over two 512-byte guest lines), cold (the region not in the caches) and warm. `make bbb-bbb_claimcost`.
#include <stdint.h>
#include "cp15.h"
#include "bbb.h"
#include "pjit_libc.h"

static uint8_t region[16 * 4096] __attribute__((aligned(4096)));
static uint8_t thrash[2 * 1024 * 1024] __attribute__((aligned(64)));
static uint8_t table[65536];

static void evict_all(void) { volatile uint8_t *p = thrash; uint32_t s = 0; for (uint32_t i = 0; i < sizeof thrash; i += 32) s += p[i]; (void)s; }
static inline uint32_t now(void) { return bbb_pmu_cycles(); }

static uint32_t t_store(uint8_t *r) { uint32_t t0 = now(); uint32_t *w = (uint32_t *)r; for (int i = 0; i < 1024; i++) w[i] = 0xEB000000u + i; return now() - t0; }
static uint32_t t_clean(uint8_t *r) { uint32_t t0 = now(); for (uint32_t a = (uint32_t)r; a < (uint32_t)r + 4096; a += 64) __asm__ volatile ("mcr p15, 0, %0, c7, c10, 1" :: "r"(a)); __asm__ volatile ("dsb"); return now() - t0; }
static uint32_t t_icinv(uint8_t *r) { uint32_t t0 = now(); for (uint32_t a = (uint32_t)r; a < (uint32_t)r + 4096; a += 64) __asm__ volatile ("mcr p15, 0, %0, c7, c5, 1" :: "r"(a)); __asm__ volatile ("mcr p15, 0, %0, c7, c5, 6\n dsb\n isb" :: "r"(0)); return now() - t0; }
static uint32_t t_scan(uint8_t *g) { uint32_t t0 = now(); uint32_t s = 0; for (uint32_t i = 0; i < 1024; i += 2) { uint32_t op = (g[i] << 8) | g[i + 1]; s += table[op]; } __asm__ volatile ("" :: "r"(s)); return now() - t0; }

int main(void)
{
    printf("=== where claim_line's cycles go (4 KB region, 1,024 words) ===\n");
    for (int i = 0; i < 65536; i++) table[i] = i & 7;
    for (int mode = 0; mode < 2; mode++) {
        for (int rep = 0; rep < 3; rep++) {
            uint8_t *r = region + rep * 4096;
            if (mode == 0) { evict_all(); CP15ICacheFlush(); }
            uint32_t a = t_store(r), b = t_clean(r), c = t_icinv(r), d = t_scan(r + 8192);
            printf("[CLAIM] %s: 1024 stores %u, 64 x clean %u, 64 x icache invalidate + BPIALL + isb %u, scan 512 words %u cycles\n", mode ? "warm" : "cold", a, b, c, d);
        }
    }
    printf("PASS\n");
    return 0;
}
