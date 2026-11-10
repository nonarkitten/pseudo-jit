/// @file bbb_opcost.c
/// @brief The cached cost of individual opcodes on a BeagleBone: for each case of tests/data/opcost_cases.h (tools/study/opcost_cases.py: one opcode with valid
///        operands, from a profile of a real program) run 8 and 40 back-to-back copies warm and take the cycle difference per copy, so the loop, the
///        entry and the exit cancel. `[COST] OOOO cycles*100 ARM-instructions*100 mispredicts*100` per case; tools/study/profile_cost.py weights them by the
///        profile. `make bbb-bbb_opcost`.
#include <stdint.h>
#include "pjit.h"
#include "bbb.h"
#include "opcost_cases.h"

extern void emit_opcode_table(void);

static uint8_t ram_pool[12 * 1024 * 1024] __attribute__((aligned(4096)));
static uint8_t scratch[0x2000] __attribute__((aligned(0x1000)));
static uint8_t code_mem[1024] __attribute__((aligned(256)));

#define KA 8
#define KB 40
#define REPS 5

static void put16(int off, uint16_t v) { code_mem[off] = v >> 8; code_mem[off + 1] = (uint8_t)v; }

static void load(const opcost_case_t *c, int k)
{
    int off = 0;
    uint32_t abs = (uint32_t)scratch + 0x800;
    for (int i = 0; i < k; i++)
        for (int j = 0; j < c->nwords; j++) {
            uint16_t w = c->w[j];
            if (c->absl_at != 255 && j == c->absl_at) w = abs >> 16;
            else if (c->absl_at != 255 && j == c->absl_at + 1) w = abs & 0xFFFF;
            put16(off, w); off += 2;
        }
    put16(off, 0x4E4F);                                            // TRAP #15
}

static void regs(void)
{
    // every long around the pointers holds the pointer itself, so a case that loads an address register from memory (MOVEA.L (A0),A0 ...) and goes on to
    // use it, copy after copy, still dereferences a valid address
    for (uint32_t a = 0x7C0; a < 0x880; a += 4) { uint32_t v = (uint32_t)scratch + 0x800; uint8_t *b = scratch + a; b[0] = v >> 24; b[1] = v >> 16; b[2] = v >> 8; b[3] = (uint8_t)v; }
    for (int i = 0; i < 6; i++) pjit_set_dn(i, 0x20 + 3 * i);
    pjit_set_dn(6, 0x10); pjit_set_dn(7, 0x30);
    for (int i = 0; i < 7; i++) pjit_set_an(i, (uint32_t)scratch + 0x800);
    pjit_set_an(7, (uint32_t)scratch + 0xC00);
    pjit_set_sr(0x2700);
}

typedef struct { uint32_t cyc, ins, mis; } meas_t;
static meas_t timed(const opcost_case_t *c, int k)
{
    meas_t best = { 0xFFFFFFFFu, 0, 0 };
    load(c, k);
    pjit_cache_flush();
    for (int r = 0; r < REPS + 1; r++) {                           // the first run translates; the others are warm
        regs();
        uint32_t m0 = bbb_pmu_event(1), i0 = bbb_pmu_event(0), t0 = bbb_pmu_cycles();
        pjit_run((uint16_t *)code_mem);
        uint32_t t1 = bbb_pmu_cycles() - t0, i1 = bbb_pmu_event(0) - i0, m1 = bbb_pmu_event(1) - m0;
        if (r > 0 && t1 < best.cyc) { best.cyc = t1; best.ins = i1; best.mis = m1; }
    }
    return best;
}

int main(void)
{
    printf("=== per-opcode cached cost (%u cases) ===\n", (unsigned)(sizeof(opcost_cases) / sizeof(opcost_cases[0])));
    pjit_cache_init((uint32_t)(ram_pool + sizeof(ram_pool)));
    cpu->trap_exit_mask = 0x8000;
    emit_opcode_table();
    for (unsigned i = 0; i < sizeof(opcost_cases) / sizeof(opcost_cases[0]); i++) {
        const opcost_case_t *c = &opcost_cases[i];
        meas_t a = timed(c, KA), b = timed(c, KB);
        int32_t dk = KB - KA;
        int32_t cyc = ((int32_t)(b.cyc - a.cyc) * 100) / dk, ins = ((int32_t)(b.ins - a.ins) * 100) / dk, mis = ((int32_t)(b.mis - a.mis) * 100) / dk;
        printf("[COST] %04X %d %d %d\n", c->op, cyc, ins, mis);
        // the translated code of the first copy (after the warm run of K = KB): one line per slot, 4 ARM words
        for (int hw = 0; hw < c->nwords; hw++) {
            const uint32_t *q = (const uint32_t *)((uint32_t)cpu->cache_data + ((((uint32_t)code_mem + hw * 2) & 0x7FFFFu) << 3));
            printf("[SLOT] %04X %d %08X %08X %08X %08X\n", c->op, hw, (unsigned)q[0], (unsigned)q[1], (unsigned)q[2], (unsigned)q[3]);
        }
    }
    printf("PASS\n");
    return 0;
}
