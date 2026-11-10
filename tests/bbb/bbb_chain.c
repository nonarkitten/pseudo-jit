/// @file bbb_chain.c
/// @brief First-run latency on the BeagleBone, slot cache and block runtime alike (`make bbb-bbb_chain`, or `make BLK=1 BUILD=... bbb-bbb_chain`). The guest is a chain of
///        units; each starts with TRAP #14 (D0 = the cycle counter), stores it through A0, does some arithmetic and memory work and ends in BRA.W to the next unit. The
///        gaps between consecutive timestamps are the time of "one unit plus the entry into the next", measured by the guest itself, so every translation, line claim,
///        eviction, ring turnover and link shows up in the distribution. Per case and pass: the gaps' minimum, median, 90th / 99th / 99.9th percentile, maximum, and how
///        many cycles lie in gaps above twice the median (the tail). Cases: footprints that fit the cache (cold pass, then warm), and one bigger than either cache
///        (every pass is a miss pass).
#include <stdint.h>
#include "pjit.h"
#include "cp15.h"
#ifdef SAMPLE
#define SAMPLE_SLOTS 8192
#define SAMPLE_N 256
#define SAMPLE_EVENT 0x11
#endif
#include "bbb.h"
#ifdef SAMPLE
#include "sampler.h"
static int sampling_this;
#endif
#if defined(PJIT_BLK) || defined(PJIT_TIER)
#include "blk.h"
#endif

extern void emit_opcode_table(void);
static uint8_t ram_pool[12 * 1024 * 1024] __attribute__((aligned(4096)));
static uint8_t scratch[0x2000] __attribute__((aligned(0x1000)));
static uint8_t code[(1 << 20) + (1 << 19) + 8192] __attribute__((aligned(1 << 19)));
static uint32_t stamp[16384 + 8];
#if defined(PJIT_BLK) || defined(PJIT_TIER)
static uint8_t big_ring[4 << 20] __attribute__((aligned(4096)));
static uint32_t big_tags[4096 * 8] __attribute__((aligned(64)));
static uint32_t big_links[16 * 4096] __attribute__((aligned(64)));      /* 16 lists of 16 KB (segments of 256 KB) */
static uint8_t big_epochs[16];
static uint8_t big_stack[4096] __attribute__((aligned(16)));
#endif

static void put16(uint8_t *b, int hw, uint16_t v) { b[hw * 2] = v >> 8; b[hw * 2 + 1] = (uint8_t)v; }
static const uint16_t body[] = { 0xD481, 0x2611, 0x5284, 0xCA82, 0xE58E, 0x2483, 0xD681, 0x5483, 0x2A11, 0xBA84, 0x4A83, 0x4E71 };

/// n units of `u` bytes (u >= 16, a multiple of 4): TRAP #14 ; MOVE.L D0,(A0)+ ; body... ; BRA.W next; the last one ends in TRAP #15
static void build(uint8_t *at, int n, int u)
{
    for (int k = 0; k < n; k++) {
        uint8_t *p = at + (uint32_t)k * u; int hw = 0;
        put16(p, hw++, 0x4E4E); put16(p, hw++, 0x20C0);
        int nb = (u - 4 - 4) / 2;
        for (int i = 0; i < nb; i++) put16(p, hw++, body[i % (int)(sizeof body / sizeof body[0])]);
        if (k == n - 1) { put16(p, hw++, 0x4E4F); put16(p, hw++, 0x4E71); }
        else { put16(p, hw++, 0x6000); put16(p, hw++, (uint16_t)(u - hw * 2 + 2)); }          // BRA.W: the displacement is from the word after the opcode
    }
}
static void regs(void)
{
    for (int i = 0; i < 8; i++) pjit_set_dn(i, 0x20 + i);
    for (int i = 0; i < 7; i++) pjit_set_an(i, (uint32_t)scratch + 0x800);
    pjit_set_an(0, (uint32_t)stamp); pjit_set_an(7, (uint32_t)scratch + 0xC00);
    pjit_set_sr(0x2000);
}
static void sort(uint32_t *a, int n) { for (int gap = n / 2; gap > 0; gap /= 2) for (int i = gap; i < n; i++) { uint32_t t = a[i]; int j = i; while (j >= gap && a[j - gap] > t) { a[j] = a[j - gap]; j -= gap; } a[j] = t; } }
static uint32_t sw(uint32_t v) { return __builtin_bswap32(v); }

static void pass(const char *name, uint8_t *at, int n, int u)
{
    for (int i = 0; i < n + 2; i++) stamp[i] = 0;
    regs();
#if defined(PJIT_BLK) || defined(PJIT_TIER)
    uint32_t b_tr = pjit_blk.n_translated, b_route = pjit_blk.n_route, b_one = pjit_blk.n_one, b_ex = pjit_blk.n_exits;
#endif
#ifdef SAMPLE
    if (sampling_this) sample_begin();
#endif
    uint32_t ev0[4]; for (int e = 0; e < 4; e++) ev0[e] = bbb_pmu_event(e);
    uint32_t t0 = bbb_pmu_cycles();
    pjit_run((uint16_t *)at);
#ifdef SAMPLE
    if (sampling_this) sample_end();
#endif
    uint32_t total = bbb_pmu_cycles() - t0;
    uint32_t evd[4]; for (int e = 0; e < 4; e++) evd[e] = bbb_pmu_event(e) - ev0[e];
    printf("[CHAIN]   per unit: %u ARM instructions, %u L1I misses, %u L1D misses, %u mispredicted branches, %u cycles\n", evd[0] / (unsigned)n, evd[1] / (unsigned)n, evd[2] / (unsigned)n, evd[3] / (unsigned)n, total / (unsigned)n);
#if defined(PJIT_BLK) || defined(PJIT_TIER)
    printf("[CHAIN]   (blocks translated %u, router entries %u, interpret-one %u, links made %u)\n", (unsigned)(pjit_blk.n_translated - b_tr), (unsigned)(pjit_blk.n_route - b_route), (unsigned)(pjit_blk.n_one - b_one), (unsigned)(pjit_blk.n_exits - b_ex));
#endif
#if defined(PJIT_BLK) || defined(PJIT_TIER)
    { static const char *nm[] = { "reserve", "decode", "liveness", "emit", "schedule", "stubs", "commit", "cache maint", "router", "foldchk", "prefix", "mdrop", "body+fwd", "dispatch" }; printf("[PHASE] cycles per block translated:"); uint32_t nb = pjit_blk.n_translated - b_tr; for (int i = 0; i < 14; i++) { printf(" %s %u", nm[i], nb ? (unsigned)(pjit_blk.prof[i] / nb) : 0u); pjit_blk.prof[i] = 0; } printf(" (%u blocks)\n", (unsigned)nb); }
#endif
    static uint32_t gap[16384 + 8], srt[16384 + 8];
    int m = 0;
    for (int i = 0; i + 1 < n; i++) { gap[m] = sw(stamp[i + 1]) - sw(stamp[i]); srt[m] = gap[m]; m++; }
    sort(srt, m);
    uint32_t med = srt[m / 2]; uint64_t tail = 0, sum = 0; int ntail = 0;
    for (int i = 0; i < m; i++) { sum += gap[i]; if (gap[i] > 2 * med) { tail += gap[i]; ntail++; } }
    printf("[CHAIN] %-26s %5d units of %3d bytes: total %9u  gap min %5u p50 %5u p90 %5u p99 %6u p99.9 %6u max %7u | %d gaps above 2x median hold %d%% of the cycles\n", name, n, u,
           (unsigned)total, srt[0], med, srt[m * 9 / 10], srt[m * 99 / 100], srt[m * 999 / 1000], srt[m - 1], ntail, sum ? (int)(tail * 100 / sum) : 0);
}

int main(void)
{
    printf("=== first-run latency: a chain of units timed by the guest ===\n");
#ifdef SAMPLE
    sampling_this = 0;
#endif
    pjit_cache_init((uint32_t)(ram_pool + sizeof(ram_pool)));
    cpu->trap_exit_mask = 0x8000;
    emit_opcode_table();
    cpu->dr_threshold = DR_THRESHOLD_DEFAULT;
    { static const uint8_t events[4] = { PMU_INSTR, PMU_L1I_MISS, PMU_L1D_MISS, PMU_BR_MISPRED }; bbb_pmu_start(events); }
    uint8_t *P = code + 0x1000;
    static const struct { int n, u; const char *name; } cases[] = {
        { 740, 66, "48 KB, 66-byte units" }, { 250, 262, "64 KB, 262-byte units" }, { 11400, 66, "750 KB (bigger than both)" }, { 2900, 262, "750 KB, 262-byte units" },
    };
    for (int cfg = 0; cfg < (
#if defined(PJIT_BLK) || defined(PJIT_TIER)
        2
#else
        1
#endif
        ); cfg++) {
#if defined(PJIT_BLK) || defined(PJIT_TIER)
        if (cfg == 1) blk_init(big_ring, sizeof big_ring, 256 * 1024, big_tags, 4096, big_links, big_epochs, big_stack, sizeof big_stack, 8 * 1024, 256);
        printf("[CHAIN] ---- block cache: %s ----\n", cfg ? "4 MB ring, 16 segments of 256 KB" : "256 KB ring, 8 segments of 32 KB (the default)");
#endif
        for (unsigned c = 0; c < sizeof cases / sizeof cases[0]; c++) {
            build(P, cases[c].n, cases[c].u);
            CP15DCacheCleanFlushBuff((uint32_t)P, (uint32_t)cases[c].n * cases[c].u); CP15ICacheFlush();
            pjit_cache_flush();
#ifdef SAMPLE
            sampling_this = 1;
#endif
            pass("cold", P, cases[c].n, cases[c].u);
#ifdef SAMPLE
            sampling_this = 0;
            printf("[PROF] %u samples, one per %d cycles\n", sample_total, SAMPLE_N);
            for (int i = 0; i < SAMPLE_SLOTS; i++) if (sample_tab[2 * i + 1]) printf("[S] %08X %u\n", sample_tab[2 * i], sample_tab[2 * i + 1]);
            printf("PASS\n"); return 0;
#endif
            pass("second", P, cases[c].n, cases[c].u);
            pass("third", P, cases[c].n, cases[c].u);
            printf("[CHAIN] ^ %s\n", cases[c].name);
        }
    }
    printf("PASS\n");
    return 0;
}
