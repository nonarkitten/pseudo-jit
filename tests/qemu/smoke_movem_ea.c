/// @file smoke_movem_ea.c
/// @brief MOVEM.L regs,(An) / regs,d16(An) / (An),regs / d16(An),regs through the DR cache (and interpretively), every base register A0-A7, displacements of
///        both signs, a spread of masks (small ones are emitted in the slots, the rest take the generic code), checked against a C model: the registers, the memory
///        image and the flags.
#include <stdint.h>
#include "pjit.h"
#include "mem_be.h"
#include "cp15.h"

extern void emit_opcode_table(void);

static uint8_t  ram_pool[12 * 1024 * 1024] __attribute__((aligned(4096)));
static uint16_t prog[16] __attribute__((aligned(256)));
static uint32_t mmu_pagetable[4096] __attribute__((aligned(16384)));
static uint8_t  buf[512] __attribute__((aligned(16)));
static int pass, fail;
static void wbe(uint16_t *a, uint16_t v) { ((uint8_t *)a)[0] = v >> 8; ((uint8_t *)a)[1] = (uint8_t)v; }
static uint32_t rl(uint32_t a) { return read_BE_long(a); }
static void check(const char *what, uint32_t got, uint32_t want, uint32_t ctx)
{
    if (got == want) pass++;
    else { if (fail < 20) printf("[FAIL] %s (%08X): got 0x%08X want 0x%08X\n", what, ctx, got, want); fail++; }
}
static uint32_t init_val(int i) { return 0x11110000u * (i + 1) + i * 0x101 + 7; }

static void one(int load, int base, int disp, int use_disp, uint16_t mask, uint32_t align_off)
{
    uint32_t ptr = (uint32_t)buf + 256 + align_off;               // the base register's value
    uint32_t ea = ptr + (use_disp ? disp : 0);
    uint16_t op = (load ? 0x4CC0 : 0x48C0) | (use_disp ? 0x28 : 0x10) | base;
    wbe(&prog[0], op); wbe(&prog[1], mask); int n = 2;
    if (use_disp) wbe(&prog[n++], (uint16_t)disp);
    wbe(&prog[n], 0x4E4F);
    uint32_t reg0[16];
    for (int i = 0; i < 16; i++) reg0[i] = init_val(i);
    reg0[8 + base] = ptr;
    for (int i = 0; i < 8; i++) { pjit_set_dn(i, reg0[i]); pjit_set_an(i, reg0[8 + i]); }
    for (int i = 0; i < 100; i++) write_BE_long((uint32_t)buf + i * 4 + 64, 0xAB000000u + i);
    if (base == 7) cpu->ssp = cpu->usp = __builtin_bswap32(ptr);
    pjit_set_sr(0x271F);
    pjit_cache_flush();
    pjit_run(prog);
    uint32_t ctx = (load << 31) | (base << 24) | ((uint32_t)(disp & 0xFF) << 16) | mask;
    uint32_t w = ea, exp[16]; for (int i = 0; i < 16; i++) exp[i] = reg0[i];
    for (int i = 0; i < 16; i++) if (mask & (1u << i)) {
        if (load) exp[i] = rl(w);                                  // (read before the run? the memory is not changed by a load)
        else check("mem", rl(w), reg0[i], ctx);
        w += 4;
    }
    for (int i = 0; i < 8; i++) { check("Dn", pjit_get_dn(i), exp[i], ctx); check("An", pjit_get_an(i), exp[8 + i], ctx); }
    check("CCR", pjit_get_sr() & 0x1F, 0x1F, ctx);
}

int main(void)
{
    printf("=== PJIT smoke test: MOVEM.L (An) / d16(An) ===\n");
    for (uint32_t i = 0; i < 4096; i++) mmu_pagetable[i] = ((i << 20) & 0xFFF00000u) | 0xC0Eu;
    CP15TtbCtlTtb0Config(); CP15Ttb0Set((uint32_t)mmu_pagetable); CP15DomainAccessClientSet();
    CP15TlbInvalidate(); CP15MMUEnable();
    pjit_cache_init((uint32_t)(ram_pool + sizeof(ram_pool)));
#ifndef PJIT_TEST_DR
    cpu->dr_threshold = 0xFFFFFFFFu;
#endif
    emit_opcode_table();
    static const int disps[] = { 8, -12, 100, -100, 0x7C };
    for (int load = 0; load < 2; load++)
        for (int base = 0; base < 8; base++) {
            for (uint32_t m = 1; m <= 0xFFFF; m += 97) {
                one(load, base, 0, 0, (uint16_t)m, 0);
                one(load, base, disps[m % 5], 1, (uint16_t)m, 0);
            }
            for (int k = 0; k < 16; k++) one(load, base, disps[k % 5], k & 1, (uint16_t)(1u << k), 2);            // every single register, half-aligned base
            printf("[RUN] %s base A%d done\n", load ? "load" : "store", base);
        }
    printf("[TEST] %d passed, %d failed\n", pass, fail);
    printf(fail ? "FAIL\n" : "PASS\n");
    return fail ? 1 : 0;
}
