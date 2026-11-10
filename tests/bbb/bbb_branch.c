/// @file bbb_branch.c
/// @brief Cycles and mispredicted branches per iteration of small 68K loops, one control-flow construct each, through the real runtime
///        (interpretive and cached) on a BeagleBone. Answers: which constructs mispredict, and how often? `make bbb-bbb_branch`.
#include <stdint.h>
#include "pjit.h"
#include "mem_be.h"
#include "bbb.h"

extern void emit_opcode_table(void);

#define N_A 20000u
#define N_B 60000u

static uint8_t ram_pool[12 * 1024 * 1024] __attribute__((aligned(4096)));
static uint8_t code_mem[2048] __attribute__((aligned(256)));
static uint8_t stack_mem[256] __attribute__((aligned(16)));

typedef struct { const char *name; uint16_t w[24]; int len; int jsr_target_hw; int jsr_at_hw; int a2_hw; } prog_t;      // a2_hw: A2 = the address of this halfword (indirect calls)
static uint8_t copy_buf[0x20000] __attribute__((aligned(16)));
// halfword indexes; a cache line is 4 guest halfwords (8 bytes). count in D5 (MOVE.L #n,D5 = idx 0..2, or MOVE.W #n-1,D5 = idx 0..1 for DBF).
static const prog_t progs[] = {
    { "SUBQ.L + BNE.S (loop crosses a line)",        { 0x2A3C, 0, 0, 0x5385, 0x66FC, 0x4E4F }, 6, 0, 0 },
    { "DBF D5 (one slot, loops on itself)",          { 0x3A3C, 0, 0x51CD, 0xFFFE, 0x4E4F }, 5, 0, 0 },
    { "BSR.S + RTS + SUBQ + BNE.S",                  { 0x2A3C, 0, 0, 0x6106, 0x5385, 0x66FA, 0x4E4F, 0x4E75 }, 8, 0, 0 },
    { "BSR.W + RTS (callee 3 lines away)",           { 0x2A3C, 0, 0, 0x6100, 0x0014, 0x5385, 0x66F8, 0x4E4F, 0,0,0,0,0,0, 0x4E75 }, 15, 0, 0 },
    { "JSR abs.L + RTS (callee 4 lines away)",       { 0x2A3C, 0, 0, 0x4EB9, 0, 0, 0x5385, 0x66F6, 0x4E4F, 0,0,0,0,0,0,0, 0x4E75 }, 17, 16, 3 },
    { "JSR (A2) + RTS (callee 4 lines away)",        { 0x2A3C, 0, 0, 0x4E92, 0x5385, 0x66FA, 0x4E4F, 0,0,0,0,0,0,0,0,0, 0x4E75 }, 17, 0, 0, 16 },
    { "JSR 8(A2) + RTS (callee 4 lines away)",       { 0x2A3C, 0, 0, 0x4EAA, 0x0008, 0x5385, 0x66F8, 0x4E4F, 0,0,0,0,0,0,0,0, 0x4E75 }, 17, 0, 0, 12 },
    { "JMP (A2) forward, SUBQ + BNE.S back to it",   { 0x2A3C, 0, 0, 0x4ED2, 0x4E71, 0x4E71, 0x5385, 0x66F6, 0x4E4F }, 9, 0, 0, 6 },
    { "BRA.S forward, SUBQ + BNE.S back to the BRA",  { 0x2A3C, 0, 0, 0x6004, 0x4E71, 0x4E71, 0x5385, 0x66F6, 0x4E4F }, 9, 0, 0 },
    { "strcpy: MOVE.B (A1)+,(A0)+ ; DBF D5",          { 0x3A3C, 0, 0x10D9, 0x51CD, 0xFFFC, 0x4E4F }, 6, 0, 0 },
    { "BRA.S forward over a slot, then SUBQ + BNE.S", { 0x2A3C, 0, 0, 0x6002, 0x4E71, 0x5385, 0x66F8, 0x4E4F }, 8, 0, 0 },
    { "NOP x4 + SUBQ + BNE.S (loop is 6 halfwords)", { 0x2A3C, 0, 0, 0x4E71, 0x4E71, 0x4E71, 0x4E71, 0x5385, 0x66F4, 0x4E4F }, 10, 0, 0 },
};

static void put16(int hw, uint16_t v) { code_mem[hw * 2] = v >> 8; code_mem[hw * 2 + 1] = v; }

static void load(const prog_t *p, uint32_t n)
{
    for (unsigned i = 0; i < sizeof(code_mem); i++) code_mem[i] = 0;
    for (int i = 0; i < p->len; i++) put16(i, p->w[i]);
    if (p->w[0] == 0x2A3C) { put16(1, n >> 16); put16(2, n & 0xFFFF); }
    else put16(1, n - 1);                                   // MOVE.W #n-1,D5 for DBF
    if (p->jsr_target_hw) {                                 // JSR abs.L: the callee's absolute address
        uint32_t a = (uint32_t)code_mem + p->jsr_target_hw * 2;
        put16(p->jsr_at_hw + 1, a >> 16); put16(p->jsr_at_hw + 2, a & 0xFFFF);
    }
    for (int i = 0; i < 8; i++) { pjit_set_dn(i, 0); pjit_set_an(i, 0); }
    if (p->a2_hw) pjit_set_an(2, (uint32_t)code_mem + p->a2_hw * 2);
    pjit_set_an(0, (uint32_t)copy_buf + 0x10000); pjit_set_an(1, (uint32_t)copy_buf);
    uint32_t sp = (uint32_t)stack_mem + sizeof(stack_mem) - 16;
    pjit_set_an(7, sp);
    cpu->ssp = cpu->usp = __builtin_bswap32(sp);
    pjit_set_sr(0x2700);
    pjit_cache_flush();
}

static void timed(const prog_t *p, uint32_t n, uint32_t *cyc, uint32_t *mis, uint32_t *ins)
{
    load(p, n);
    uint32_t m0 = bbb_pmu_event(1), i0 = bbb_pmu_event(0), t0 = bbb_pmu_cycles();
    pjit_run((uint16_t *)code_mem);
    uint32_t t1 = bbb_pmu_cycles(), m1 = bbb_pmu_event(1), i1 = bbb_pmu_event(0);
    if (pjit_get_dn(5) != (p->w[0] == 0x2A3C ? 0u : 0xFFFFu)) printf("[ERR] D5 = %08X at exit\n", pjit_get_dn(5));
    *cyc = t1 - t0; *mis = m1 - m0; *ins = i1 - i0;
}

static void dump_slots(const prog_t *p)
{
    for (int hw = 0; hw < p->len + 2; hw++) {
        const uint32_t *q = (const uint32_t *)((uint32_t)cpu->cache_data + ((((uint32_t)code_mem + hw * 2) & 0x7FFFFu) << 3));
        printf("[DUMP] hw%02d %08X %08X %08X %08X\n", hw, (unsigned)q[0], (unsigned)q[1], (unsigned)q[2], (unsigned)q[3]);
    }
}

static void report(const char *mode)
{
    for (unsigned k = 0; k < sizeof(progs) / sizeof(progs[0]); k++) {
        uint32_t ca, ma, ia, cb, mb, ib;
        timed(&progs[k], N_A, &ca, &ma, &ia); timed(&progs[k], N_B, &cb, &mb, &ib);
        if (mode[0] == 'c') { printf("[DUMP] %s\n", progs[k].name); dump_slots(&progs[k]); }
        int32_t dn = N_B - N_A;
        int32_t c100 = (int32_t)(((int64_t)(int32_t)(cb - ca) * 100) / dn), i100 = (int32_t)(((int64_t)(int32_t)(ib - ia) * 100) / dn);
        int32_t m100 = (int32_t)(((int64_t)(int32_t)(mb - ma) * 100) / dn);
        printf("[COMP] %u blocks made so far, last refusal site %u\n", (unsigned)pjit_arena.comp[127], (unsigned)pjit_arena.comp[118]);
        printf("[BR] %-7s %-46s %3d.%02d cycles %3d.%02d ARM instrs %d.%02d mispredicts per iteration\n", mode, progs[k].name, c100 / 100, c100 % 100,
               i100 / 100, i100 % 100, m100 / 100, (m100 < 0 ? -m100 : m100) % 100);
    }
}

// ---- how the cost scales with the number of distinct instructions in the loop (branch-target-buffer capacity) ----
static void run_scaling(uint16_t body_op, const char *what)
{
    static const int ks[] = { 8, 32, 64, 128, 192, 256, 384 };
    for (unsigned j = 0; j < sizeof(ks) / sizeof(ks[0]); j++) {
        int K = ks[j]; uint32_t cyc[2], mis[2], ins[2];
        for (int t = 0; t < 2; t++) {
            uint32_t n = t ? 6000 : 2000;
            for (unsigned i = 0; i < sizeof(code_mem); i++) code_mem[i] = 0;
            put16(0, 0x2A3C); put16(1, n >> 16); put16(2, n & 0xFFFF);
            for (int i = 0; i < K; i++) put16(3 + i, body_op);
            put16(3 + K, 0x5385); put16(4 + K, 0x6600); put16(5 + K, (uint16_t)(-(K + 2) * 2)); put16(6 + K, 0x4E4F);
            for (int i = 0; i < 8; i++) { pjit_set_dn(i, 0); pjit_set_an(i, 0); }
            uint32_t sp = (uint32_t)stack_mem + sizeof(stack_mem) - 16; pjit_set_an(7, sp); cpu->ssp = cpu->usp = __builtin_bswap32(sp);
            pjit_set_sr(0x2700); pjit_cache_flush();
            uint32_t m0 = bbb_pmu_event(1), i0 = bbb_pmu_event(0), t0 = bbb_pmu_cycles();
            pjit_run((uint16_t *)code_mem);
            cyc[t] = bbb_pmu_cycles() - t0; mis[t] = bbb_pmu_event(1) - m0; ins[t] = bbb_pmu_event(0) - i0;
        }
        int32_t dn = 4000;
        int32_t c = (int32_t)(((int64_t)(int32_t)(cyc[1] - cyc[0]) * 100) / dn), m = (int32_t)(((int64_t)(int32_t)(mis[1] - mis[0]) * 100) / dn);
        int32_t in = (int32_t)(((int64_t)(int32_t)(ins[1] - ins[0]) * 100) / dn);
        printf("[SCALE] %-22s K=%3d: %4d.%02d cycles, %4d.%02d ARM instrs, %3d.%02d mispredicts per iteration = %d.%02d cycles / %d.%02d instrs per 68K instruction\n", what, K,
               c / 100, c % 100, in / 100, in % 100, m / 100, m % 100, (c / (K + 2)) / 100, (c / (K + 2)) % 100, (in / (K + 2)) / 100, (in / (K + 2)) % 100);
    }
}

int main(void)
{
    printf("=== control-flow constructs on the BeagleBone (PMU) ===\n");
    pjit_cache_init((uint32_t)(ram_pool + sizeof(ram_pool)));
    cpu->trap_exit_mask = 0x8000;
    emit_opcode_table();
    cpu->dr_threshold = DR_THRESHOLD_DEFAULT; report("cached");
    run_scaling(0xD081, "ADD.L D1,D0 (bl body)"); run_scaling(0x7401, "MOVEQ #1,D2 (inline)"); run_scaling(0x2200, "MOVE.L D0,D1 (inline?)");
    cpu->dr_threshold = 0xFFFFFFFFu;          report("interp");
    printf("PASS\n");
    return 0;
}
