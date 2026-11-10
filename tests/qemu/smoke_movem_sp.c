/// @file smoke_movem_sp.c
/// @brief Every packed mask over D2-D7/A2-A6 (2,048), through MOVEM.L regs,-(A7) and MOVEM.L (A7)+,regs, in both
///        modes, checked against a C model: the registers, the memory image, the stack pointer, and that the
///        flags survive. Then the cases the per-mask routines must hand to the generic code: a mask with other
///        registers in it, A7 itself in the list, and a stack pointer that is only 2-byte aligned.
#include <stdint.h>
#include "pjit.h"
#include "mem_be.h"
#include "cp15.h"

extern void emit_opcode_table(void);

static uint8_t  ram_pool[12 * 1024 * 1024] __attribute__((aligned(4096)));
static uint16_t prog[16] __attribute__((aligned(256)));
static uint32_t mmu_pagetable[4096] __attribute__((aligned(16384)));
static uint8_t  stk[256] __attribute__((aligned(16)));
static int pass, fail;
static void wbe(uint16_t *a, uint16_t v) { ((uint8_t *)a)[0] = v >> 8; ((uint8_t *)a)[1] = (uint8_t)v; }
static uint32_t rl(uint32_t a) { return read_BE_long(a); }
static void check(const char *what, uint32_t got, uint32_t want, uint32_t ctx)
{
    if (got == want) pass++;
    else { if (fail < 20) printf("[FAIL] %s (mask %04X): got 0x%08X want 0x%08X\n", what, ctx, got, want); fail++; }
}

/// reg value i of the 16 (D0..D7, A0..A7) before the instruction
static uint32_t init_val(int i) { return 0x11110000u * (i + 1) + i * 0x101 + 7; }

/// Runs `MOVEM.L list,-(A7)` (save) or `MOVEM.L (A7)+,list` (restore) with the registers set to init_val, A7 =
/// `sp` (the stack holds known words for a restore), and returns the registers and the stack afterwards.
static void run(int save, uint16_t mask, uint32_t sp, uint32_t *reg, uint32_t sr_in, uint32_t *sr_out)
{
    wbe(&prog[0], save ? 0x48E7 : 0x4CDF); wbe(&prog[1], mask); wbe(&prog[2], 0x4E4F);
    for (int i = 0; i < 8; i++) { pjit_set_dn(i, init_val(i)); pjit_set_an(i, init_val(8 + i)); }
    pjit_set_an(7, sp);
    for (int i = 0; i < 64; i++) write_BE_long((uint32_t)stk + i * 4, 0xAB000000u + i);   // restore: what is on the stack
    cpu->ssp = cpu->usp = __builtin_bswap32(sp);
    pjit_set_sr(sr_in);
    pjit_cache_flush();
    pjit_run(prog);
    for (int i = 0; i < 8; i++) { reg[i] = pjit_get_dn(i); reg[8 + i] = pjit_get_an(i); }
    *sr_out = pjit_get_sr();
}

static void one(int save, uint16_t mask_normal, uint32_t sp_off, const char *tag)
{
    // the instruction's mask: reversed for -(A7)
    uint16_t m = 0; if (save) { for (int i = 0; i < 16; i++) if (mask_normal & (1u << i)) m |= 1u << (15 - i); } else m = mask_normal;
    uint32_t sp = (uint32_t)stk + 128 + sp_off, reg[16], sr;
    uint32_t sp0 = sp;
    run(save, m, sp, reg, 0x271F, &sr);
    // the model
    uint32_t exp[16]; for (int i = 0; i < 16; i++) exp[i] = init_val(i);
    exp[15] = sp0; uint32_t a7init = sp0;
    int n = 0; for (int i = 0; i < 16; i++) if (mask_normal & (1u << i)) n++;
    if (save) {
        uint32_t a = sp0 - 4 * n, w = a;
        for (int i = 0; i < 16; i++) if (mask_normal & (1u << i)) { check(tag, rl(w), i == 15 ? a7init : init_val(i), mask_normal); w += 4; }
        exp[15] = a;
    } else {
        uint32_t w = sp0;     // what was on the stack before the instruction ran: the pre-filled words
        for (int i = 0; i < 16; i++) if (mask_normal & (1u << i)) { exp[i] = rl(w); w += 4; }
        exp[15] = w;      // A7 ends up at the postincremented address even when it is in the list (the loaded value is ignored)
        // expected values come from the pre-filled words: word k of the stack area
    }
    for (int i = 0; i < 16; i++) check(tag, reg[i], exp[i], mask_normal);
    check("CCR", sr & 0x1F, 0x1F, mask_normal);
}

int main(void)
{
    // QEMU faults unaligned accesses with the MMU off; the 2-mod-4 stack cases need it on
    for (uint32_t i = 0; i < 4096; i++) mmu_pagetable[i] = ((i << 20) & 0xFFF00000u) | 0xC0Eu;
    CP15TtbCtlTtb0Config(); CP15Ttb0Set((uint32_t)mmu_pagetable); CP15DomainAccessClientSet();
    CP15TlbInvalidate(); CP15MMUEnable();
    printf("=== PJIT smoke test: MOVEM.L with A7 as the base ===\n");
    pjit_cache_init((uint32_t)(ram_pool + sizeof(ram_pool)));
#ifndef PJIT_TEST_DR
    cpu->dr_threshold = 0xFFFFFFFFu;
#endif
    emit_opcode_table();
    for (int save = 0; save < 2; save++) {
        for (uint32_t idx = 0; idx < 2048; idx++) {
            uint16_t m = ((idx & 0x3F) << 2) | ((idx >> 6) << 10);
            one(save, m, 0, save ? "save" : "restore");
        }
        printf("[RUN] %s: all 2048 masks done\n", save ? "save" : "restore");
    }
    // generic fallbacks
    static const uint16_t odd_masks[] = { 0x0003, 0x0300, 0x8001, 0x0080, 0x8000, 0xFFFF, 0x7FFF, 0x4001 };
    for (int save = 0; save < 2; save++)
        for (unsigned i = 0; i < sizeof(odd_masks) / 2; i++) one(save, odd_masks[i], 0, "generic mask");
    // masks with D0/D1/A0/A1 in them (the in-slot form takes the small ones, the generic code the rest)
    for (int save = 0; save < 2; save++) for (uint32_t m = 1; m < 0x8000; m += 13) one(save, (uint16_t)m, 0, "any mask without A7");
    for (int save = 0; save < 2; save++) for (uint32_t idx = 1; idx < 2048; idx += 97) {
        uint16_t m = ((idx & 0x3F) << 2) | ((idx >> 6) << 10);
        one(save, m, 2, "sp 2 mod 4");
    }
    printf("[TEST] %d passed, %d failed\n", pass, fail);
    printf(fail ? "FAIL\n" : "PASS\n");
    return fail ? 1 : 0;
}
