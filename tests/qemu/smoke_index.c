/// @file smoke_index.c
/// @brief Every (d8,An,Xn) / (d8,PC,Xn) index register and size, as a source (LEA) and as a destination (MOVE.L Dn,<ea>):
///        D0-D7 and A0-A7, .W (sign-extended) and .L, positive and negative d8. In the DR cache these are built into the
///        slots by pjit_fill (the register adds are inline), so this exercises that code for all 32 index forms.
#include <stdint.h>
#include "pjit.h"
#include "mem_be.h"
#include "cp15.h"

extern void emit_opcode_table(void);

static uint8_t  ram_pool[12 * 1024 * 1024] __attribute__((aligned(4096)));
static uint16_t prog[16] __attribute__((aligned(256)));
static uint32_t mmu_pagetable[4096] __attribute__((aligned(16384)));
static uint8_t  area[0x20000] __attribute__((aligned(16)));
static uint8_t  stk[256] __attribute__((aligned(16)));
static int pass, fail;
static void wbe(uint16_t *a, uint16_t v) { ((uint8_t *)a)[0] = v >> 8; ((uint8_t *)a)[1] = (uint8_t)v; }
static void check(const char *what, uint32_t got, uint32_t want, uint32_t ctx)
{
    if (got == want) pass++;
    else { if (fail < 20) printf("[FAIL] %s (ext %04X): got 0x%08X want 0x%08X\n", what, ctx, got, want); fail++; }
}

int main(void)
{
    // unaligned long stores: QEMU only allows them with the MMU on
    for (uint32_t i = 0; i < 4096; i++) mmu_pagetable[i] = ((i << 20) & 0xFFF00000u) | 0xC0Eu;
    CP15TtbCtlTtb0Config(); CP15Ttb0Set((uint32_t)mmu_pagetable); CP15DomainAccessClientSet();
    CP15TlbInvalidate(); CP15MMUEnable();
    printf("=== PJIT smoke test: every index register in (d8,An,Xn) and (d8,PC,Xn) ===\n");
    pjit_cache_init((uint32_t)(ram_pool + sizeof(ram_pool)));
#ifndef PJIT_TEST_DR
    cpu->dr_threshold = 0xFFFFFFFFu;
#endif
    emit_opcode_table();
    const uint32_t X = 0x0000F678;                      // the low word is negative: .W and .L differ
    const uint32_t base = (uint32_t)area + 0x10000;
    for (int form = 0; form < 3; form++)                // 0: LEA (d8,A0,Xn),A1   1: LEA (d8,PC,Xn),A1   2: MOVE.L D2,(d8,A0,Xn)
    for (int reg = 0; reg < 16; reg++)
    for (int lng = 0; lng < 2; lng++)
    for (int k = 0; k < 2; k++) {
        if (form == 2 && (reg == 2 || reg == 8 || reg == 15)) continue;     // those would be wild store addresses
        int8_t d8 = k ? -6 : 10;
        uint16_t ext = (uint16_t)((reg << 12) | (lng << 11) | (uint8_t)d8);
        wbe(&prog[0], form == 0 ? 0x43F0 : form == 1 ? 0x43FB : 0x2182);
        wbe(&prog[1], ext);
        wbe(&prog[2], 0x4E4F);
        for (int i = 0; i < 8; i++) { pjit_set_dn(i, X); pjit_set_an(i, X); }
        pjit_set_dn(2, 0xCAFEBABE);
        pjit_set_an(0, base);
        pjit_set_an(7, (uint32_t)stk + 224);
        cpu->ssp = cpu->usp = __builtin_bswap32((uint32_t)stk + 224);
        pjit_set_sr(0x2700);
        uint32_t regval = reg < 8 ? (reg == 2 ? 0xCAFEBABE : X) : (reg == 8 ? base : reg == 15 ? (uint32_t)stk + 224 : X);
        pjit_cache_flush();
        pjit_run(prog);
        uint32_t idx = lng ? regval : (uint32_t)(int32_t)(int16_t)regval;
        uint32_t basev = form == 1 ? (uint32_t)&prog[1] : base;
        uint32_t want = basev + (uint32_t)(int32_t)d8 + idx;
        if (form < 2) check(form ? "LEA (d8,PC,Xn)" : "LEA (d8,A0,Xn)", pjit_get_an(1), want, ext);
        else check("MOVE.L D2,(d8,A0,Xn)", read_BE_long(want), 0xCAFEBABE, ext);
    }
    printf("[TEST] %d passed, %d failed\n", pass, fail);
    printf(fail ? "FAIL\n" : "PASS\n");
    return fail ? 1 : 0;
}
