/// @file bbb_cadence.c
/// @brief How deterministic is the timing of translated code? Four measurements on a BeagleBone (`make bbb-bbb_cadence`):
///        A  run-to-run: the same warm program 300 times, the distribution of its cycle count;
///        B  alignment: the same 8-instruction unit at 16 different positions in the cache (slot and line alignment), cycles per unit;
///        C  context: every ordered pair (A ; B) of 24 common opcodes, cycles per pair against cost(A) + cost(B) measured on their own: how far an
///           instruction's cost depends on its neighbour;
///        D  predictor / cache state: the same unit after the instruction cache and branch predictor were emptied, against warm.
#include <stdint.h>
#include "pjit.h"
#include "cp15.h"
#include "bbb.h"

extern void emit_opcode_table(void);
static uint8_t ram_pool[12 * 1024 * 1024] __attribute__((aligned(4096)));
static uint8_t scratch[0x2000] __attribute__((aligned(0x1000)));
static uint8_t code_mem[4096] __attribute__((aligned(256)));

typedef struct { const char *name; uint8_t n; uint16_t w[3]; } op_t;
static const op_t ops[] = {
    { "MOVE.L D2,D1", 1, { 0x2202 } },        { "ADD.L D1,D0", 1, { 0xD081 } },        { "SUBQ.L #1,D3", 1, { 0x5383 } },     { "ADDQ.W #1,D2", 1, { 0x5242 } },
    { "MOVE.L (A1),D0", 1, { 0x2011 } },      { "MOVE.L D0,(A2)", 1, { 0x2480 } },     { "CMP.L D0,D1", 1, { 0xB280 } },      { "TST.L D0", 1, { 0x4A80 } },
    { "LSL.L #2,D1", 1, { 0xE589 } },         { "EXT.L D2", 1, { 0x48C2 } },           { "MOVEQ #5,D3", 1, { 0x7605 } },      { "LEA 4(A1),A1", 2, { 0x43E9, 0x0004 } },
    { "MOVE.B (A0)+,D1", 1, { 0x1218 } },     { "CMP.B D0,D1", 1, { 0xB200 } },        { "AND.L D1,D0", 1, { 0xC081 } },      { "OR.W D0,D1", 1, { 0x8240 } },
    { "MOVE.L D0,-(A7)", 1, { 0x2F00 } },     { "MOVE.L (A7)+,D0", 1, { 0x201F } },    { "NOP", 1, { 0x4E71 } },              { "CLR.L D2", 1, { 0x4282 } },
    { "NEG.L D0", 1, { 0x4480 } },            { "SWAP D1", 1, { 0x4841 } },            { "BTST #3,D0", 2, { 0x0800, 0x0003 } }, { "MULU.W D1,D0", 1, { 0xC0C1 } },
};
#define NOPS ((int)(sizeof(ops) / sizeof(ops[0])))

static void regs(void)
{
    for (uint32_t a = 0x7C0; a < 0x880; a += 4) { uint32_t v = (uint32_t)scratch + 0x800; uint8_t *b = scratch + a; b[0] = v >> 24; b[1] = v >> 16; b[2] = v >> 8; b[3] = (uint8_t)v; }
    for (int i = 0; i < 6; i++) pjit_set_dn(i, 0x20 + 3 * i);
    pjit_set_dn(6, 0x10); pjit_set_dn(7, 0x30);
    for (int i = 0; i < 7; i++) pjit_set_an(i, (uint32_t)scratch + 0x800);
    pjit_set_an(7, (uint32_t)scratch + 0xC00);
    pjit_set_sr(0x2700);
}
static void put16(int hw, uint16_t v) { code_mem[hw * 2] = v >> 8; code_mem[hw * 2 + 1] = (uint8_t)v; }

/// the program: `pad` NOPs, then K copies of the unit (a list of op indexes), then TRAP #15
static int build(const int *unit, int nu, int K, int pad)
{
    int hw = 0;
    for (int i = 0; i < pad; i++) put16(hw++, 0x4E71);
    for (int k = 0; k < K; k++) for (int u = 0; u < nu; u++) for (int j = 0; j < ops[unit[u]].n; j++) put16(hw++, ops[unit[u]].w[j]);
    put16(hw++, 0x4E4F);
    return hw;
}
static uint32_t run_once(void) { regs(); uint32_t t0 = bbb_pmu_cycles(); pjit_run((uint16_t *)code_mem); return bbb_pmu_cycles() - t0; }

/// cycles per copy of the unit: the warm difference between K = 40 and K = 8 (best of 5)
static int32_t per_copy_x100(const int *unit, int nu, int pad, int cold)
{
    uint32_t best[2];
    for (int t = 0; t < 2; t++) {
        int K = t ? 40 : 8;
        build(unit, nu, K, pad); pjit_cache_flush(); run_once();
        best[t] = 0xFFFFFFFFu;
        for (int r = 0; r < 5; r++) {
            if (cold) { CP15ICacheFlush(); CP15BranchPredictorInvalidate(); }
            uint32_t c = run_once(); if (c < best[t]) best[t] = c;
        }
    }
    return (int32_t)(((int64_t)(int32_t)(best[1] - best[0]) * 100) / 32);
}

static void print2(int32_t x) { printf("%d.%02d", x / 100, (x < 0 ? -x : x) % 100); }

int main(void)
{
    printf("=== timing consistency on the BeagleBone ===\n");
    pjit_cache_init((uint32_t)(ram_pool + sizeof(ram_pool)));
    cpu->trap_exit_mask = 0x8000;
    emit_opcode_table();
    cpu->dr_threshold = DR_THRESHOLD_DEFAULT;

    // ---- A: run-to-run ----
    {
        static const int unit[] = { 0, 1, 4, 6, 8, 9, 12, 13, 14, 16, 17, 20 };
        build(unit, 12, 20, 0); pjit_cache_flush(); run_once();
        static uint32_t v[300]; uint32_t mn = 0xFFFFFFFFu, mx = 0; uint64_t sum = 0;
        for (int i = 0; i < 300; i++) { v[i] = run_once(); if (v[i] < mn) mn = v[i]; if (v[i] > mx) mx = v[i]; sum += v[i]; }
        uint32_t mean = (uint32_t)(sum / 300); uint64_t var = 0; int distinct = 0;
        for (int i = 0; i < 300; i++) { int64_t d = (int64_t)v[i] - mean; var += d * d; int seen = 0; for (int j = 0; j < i; j++) if (v[j] == v[i]) { seen = 1; break; } if (!seen) distinct++; }
        uint32_t sd2 = (uint32_t)(var / 300), sd = 0; while ((sd + 1) * (sd + 1) <= sd2) sd++;
        { static uint32_t vals[16]; static int cnts[16]; int nv = 0; for (int i = 0; i < 300; i++) { int j; for (j = 0; j < nv; j++) if (vals[j] == v[i]) break; if (j == nv && nv < 16) { vals[nv] = v[i]; cnts[nv++] = 0; } if (j < 16) cnts[j]++; }
          for (int j = 0; j < nv; j++) printf("[A]   %u cycles: %d runs\n", vals[j], cnts[j]); }
        printf("[A] 300 warm runs of a 240-instruction program: min %u max %u mean %u stddev %u cycles, %d distinct values\n", mn, mx, mean, sd, distinct);
    }

    // ---- B: alignment ----
    {
        static const int unit[] = { 0, 1, 4, 6, 8, 9, 12, 13 };
        int32_t lo = 0x7FFFFFFF, hi = -0x7FFFFFFF;
        printf("[B] 8-instruction unit, cycles per unit, with 0..15 NOPs in front:");
        for (int pad = 0; pad < 16; pad++) { int32_t c = per_copy_x100(unit, 8, pad, 0); printf(" "); print2(c); if (c < lo) lo = c; if (c > hi) hi = c; }
        printf("\n[B] spread: "); print2(lo); printf(" .. "); print2(hi); printf(" cycles (%d%%)\n", (hi - lo) * 100 / lo);
    }

    // ---- D: cold predictor and instruction cache ----
    {
        static const int unit[] = { 0, 1, 4, 6, 8, 9, 12, 13 };
        int32_t w = per_copy_x100(unit, 8, 0, 0), c = per_copy_x100(unit, 8, 0, 1);
        printf("[D] unit warm "); print2(w); printf(" cycles, after emptying the I-cache and predictor: "); print2(c); printf("\n");
    }

    // ---- C: pairs ----
    {
        static int32_t solo[NOPS];
        for (int i = 0; i < NOPS; i++) { int u[1] = { i }; solo[i] = per_copy_x100(u, 1, 0, 0); }
        int cnt = 0; int64_t absdev = 0; int32_t worst = 0, wa = 0, wb = 0; int within5 = 0, within20 = 0;
        for (int a = 0; a < NOPS; a++) for (int b = 0; b < NOPS; b++) {
            int u[2] = { a, b };
            int32_t pair = per_copy_x100(u, 2, 0, 0), sum = solo[a] + solo[b], d = pair - sum;
            int32_t pct = sum ? (int32_t)((int64_t)d * 100 / sum) : 0;
            if (pct < 0) pct = -pct;
            absdev += pct; cnt++; if (pct <= 5) within5++; if (pct <= 20) within20++;
            if (pct > worst) { worst = pct; wa = a; wb = b; }
            printf("[C] %d %d %d %d\n", a, b, pair, sum);
        }
        printf("[C] 576 ordered pairs: mean |pair - (A + B)| = %d%% of the sum; within 5%%: %d, within 20%%: %d; worst %d%% (%s ; %s)\n", (int)(absdev / cnt), within5, within20, worst, ops[wa].name, ops[wb].name);
        for (int i = 0; i < NOPS; i++) { printf("[C] solo %-18s ", ops[i].name); print2(solo[i]); printf("\n"); }
    }
    printf("PASS\n");
    return 0;
}
