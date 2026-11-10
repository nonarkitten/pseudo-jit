/// @file bbb_wcet.c
/// @brief Best and worst case latency of one guest instruction on a BeagleBone (`make bbb-bbb_wcet`). For each case of tests/data/opcost_cases.h:
///        [W]  op warm  fill  claim  evict  icold  interp   (cycles*100 for warm / interp; plain cycles above warm for the rest)
///          warm   the translated instruction, everything resident (slope of 8 and 40 copies)
///          fill   the first execution when its line is claimed but the slot is not translated (pjit_fill: translate the entry)
///          claim  ... when the line is free as well (claim_line: 1024 `bl` words, a cache flush of 4 KB) -- minus the fill
///          evict  ... when another address's code owns the line (+ the shadow stack cleared, the links of that line's owners invalidated) -- minus the fill
///          icold  warm code after the I-cache and branch predictor were emptied (one fetch from DDR)
///          interp the same instruction in the interpreter (code below the cache threshold)
///        then [L] the worst eviction: a chain of linked lines (every line holds a link) and one eviction that has to invalidate all of them.
#include <stdint.h>
#include "pjit.h"
#include "cp15.h"
#include "bbb.h"
#include "opcost_cases.h"

extern void emit_opcode_table(void);
static uint8_t ram_pool[12 * 1024 * 1024] __attribute__((aligned(4096)));
static uint8_t scratch[0x2000] __attribute__((aligned(0x1000)));
static uint8_t code[(1 << 20) + (1 << 19) + 8192] __attribute__((aligned(1 << 19)));   // guest code: two addresses 512 KB apart share a cache line
#define LINE_STRIDE (1u << 19)

static int dbg = 0;
static void put16(uint8_t *base, int hw, uint16_t v) { base[hw * 2] = v >> 8; base[hw * 2 + 1] = (uint8_t)v; }
static void regs(void)
{
    for (uint32_t a = 0x7C0; a < 0x880; a += 4) { uint32_t v = (uint32_t)scratch + 0x800; uint8_t *b = scratch + a; b[0] = v >> 24; b[1] = v >> 16; b[2] = v >> 8; b[3] = (uint8_t)v; }
    for (int i = 0; i < 6; i++) pjit_set_dn(i, 0x20 + 3 * i);
    pjit_set_dn(6, 0x10); pjit_set_dn(7, 0x30);
    for (int i = 0; i < 7; i++) pjit_set_an(i, (uint32_t)scratch + 0x800);
    pjit_set_an(7, (uint32_t)scratch + 0xC00);
    pjit_set_sr(0x2700);
}
static uint32_t run_at(uint8_t *pc) { if (dbg) printf("[RUN] %p\n", pc); regs(); uint32_t t0 = bbb_pmu_cycles(); pjit_run((uint16_t *)pc); return bbb_pmu_cycles() - t0; }
static uint32_t best5(uint8_t *pc) { uint32_t b = 0xFFFFFFFFu; for (int i = 0; i < 5; i++) { uint32_t c = run_at(pc); if (c < b) b = c; } return b; }

/// writes the case `c` k times at `at` (a guest byte address), then TRAP #15
static void write_case(uint8_t *at, const opcost_case_t *c, int k)
{
    int hw = 0; uint32_t abs = (uint32_t)scratch + 0x800;
    for (int i = 0; i < k; i++) for (int j = 0; j < c->nwords; j++) {
        uint16_t w = c->w[j];
        if (c->absl_at != 255 && j == c->absl_at) w = abs >> 16; else if (c->absl_at != 255 && j == c->absl_at + 1) w = abs & 0xFFFF;
        put16(at, hw++, w);
    }
    put16(at, hw, 0x4E4F);
}
static int32_t slope100(uint8_t *at, const opcost_case_t *c)
{
    uint32_t t[2];
    for (int s = 0; s < 2; s++) { int k = s ? 40 : 8; write_case(at, c, k); pjit_cache_flush(); run_at(at); t[s] = best5(at); }
    return (int32_t)(((int64_t)(int32_t)(t[1] - t[0]) * 100) / 32);
}

int main(void)
{
    printf("=== worst-case latency of one instruction on the BeagleBone ===\n");
    pjit_cache_init((uint32_t)(ram_pool + sizeof(ram_pool)));
    cpu->trap_exit_mask = 0x8000;
    emit_opcode_table();
    for (uint32_t i = 0; i < sizeof code; i += 2) { code[i] = 0x4E; code[i + 1] = 0x71; }       // NOPs everywhere: the translator reads ahead, and zeros are ORI.B #imm, which would swallow a case's first word
    uint8_t *P = code + 0x1000;                                     // a cache line's first byte (512-byte lines): the stub TRAP is at P, the case at P + 16 (entry 2)
    uint8_t *Q = P + LINE_STRIDE;
    { static const uint8_t events[4] = { PMU_INSTR, PMU_L1I_MISS, PMU_L1D_MISS, PMU_BR_MISPRED }; bbb_pmu_start(events); }                                   // the same line, another tag
    for (unsigned i = 0; i < sizeof(opcost_cases) / sizeof(opcost_cases[0]); i++) {
        const opcost_case_t *c = &opcost_cases[i];
        cpu->dr_threshold = DR_THRESHOLD_DEFAULT;
        int32_t warm = slope100(P + 16, c);
        write_case(P + 16, c, 1);
        // fill: the line is claimed (TRAP at P translated), the case's entry is not
        write_case(P + 16, c, 1); pjit_cache_flush(); put16(P, 0, 0x4E4F); run_at(P);       // (code is written before the line is claimed: the cache does not follow self-modifying code)
        if (dbg) for (int hw = 0; hw < 12; hw++) { const uint32_t *q = (const uint32_t *)((uint32_t)cpu->cache_data + ((((uint32_t)P + hw * 2) & 0x7FFFFu) << 3)); printf("[DUMP] hw%02d %08X %08X %08X %08X\n", hw, (unsigned)q[0], (unsigned)q[1], (unsigned)q[2], (unsigned)q[3]); }
        uint32_t e0[4]; for (int e = 0; e < 4; e++) e0[e] = bbb_pmu_event(e);
        uint32_t fill = run_at(P + 16);
        if (c->op == 0x1430 || c->op == 0x1030 || c->op == 0x1010) printf("[PMU] %04X fill: %u ARM instructions, L1I misses %u, L1D misses %u, mispredicts %u, cycles %u\n", c->op, (unsigned)(bbb_pmu_event(0) - e0[0]), (unsigned)(bbb_pmu_event(1) - e0[1]), (unsigned)(bbb_pmu_event(2) - e0[2]), (unsigned)(bbb_pmu_event(3) - e0[3]), (unsigned)fill); uint32_t w1 = best5(P + 16);
        pjit_cache_flush(); uint32_t claim = run_at(P + 16);
        // evict: Q's copy of the line owns it
        pjit_cache_flush(); write_case(Q + 16, c, 1); run_at(Q + 16); uint32_t evict = run_at(P + 16);
        write_case(P + 16, c, 1); pjit_cache_flush(); run_at(P + 16); CP15ICacheFlush(); CP15BranchPredictorInvalidate(); uint32_t icold = run_at(P + 16);
        cpu->dr_threshold = 0xFFFFFFFFu; int32_t in = slope100(P + 16, c); cpu->dr_threshold = DR_THRESHOLD_DEFAULT;
        printf("[W] %04X %d %d %d %d %d %d\n", c->op, warm, (int)(fill - w1), (int)(claim - fill), (int)(evict - fill), (int)(icold - w1), in);
    }

    // ---- the worst eviction: N linked lines, then one conflicting address ----
    for (int n = 0; n <= 1000; n += (n < 8 ? 1 : (n == 8 ? 92 : 900))) {
        pjit_cache_flush();
        for (int k = 0; k < n; k++) { uint8_t *ln = P + (uint32_t)k * 512; put16(ln, 0, 0x6000); put16(ln, 1, 510); }          // BRA.W to the next line: a link
        uint8_t *last = P + (uint32_t)n * 512; put16(last, 0, 0x4E4F);
        run_at(P);                                                    // translate and link the whole chain
        uint32_t base = best5(P);
        uint8_t *victim = Q + (uint32_t)(n / 2) * 512;                // conflicts with the chain's middle line
        put16(victim, 0, 0x4E4F);
        uint32_t t = run_at(victim);
        printf("[L] chain of %d linked lines: warm run %u cycles; one conflicting address: %u cycles\n", n, base, t);
    }
    printf("PASS\n");
    return 0;
}
