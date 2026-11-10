/// @file smoke_alias_interp.c
/// @brief Instructions whose register is also their address register (MOVE.L A0,(A0)+, MOVEA.L (A0)+,A0,
///        MOVEM with the addressing register in the list, LINK/UNLK/PEA with A7). On ARM a load/store with
///        Rt == Rn and write-back is UNPREDICTABLE, so none of them may be emitted that way; this checks
///        the 68000 results the two-step forms must give.
#include <stdint.h>
#include "pjit.h"
#include "mem_be.h"

extern void emit_opcode_table(void);

static uint8_t  ram_pool[12 * 1024 * 1024] __attribute__((aligned(4096)));
static uint16_t prog[64]    __attribute__((aligned(256)));
static uint8_t  buf[128]    __attribute__((aligned(16)));

static int np, pass, fail;
static void wbe(uint16_t *a, uint16_t v) { ((uint8_t *)a)[0] = v >> 8; ((uint8_t *)a)[1] = (uint8_t)v; }
static void w(uint16_t v)   { wbe(&prog[np++], v); }
static uint32_t rl(uint32_t off) { return read_BE_long((uint32_t)buf + off); }
static void wl_(uint32_t off, uint32_t v) { write_BE_long((uint32_t)buf + off, v); }

static void check(const char *what, uint32_t got, uint32_t want)
{
    if (got == want) pass++;
    else { printf("[FAIL] %-34s got 0x%08X want 0x%08X\n", what, got, want); fail++; }
}

/// Runs the program written so far, with A0 = a0, A1 = a1, A7 = a7 (0: a private stack), D0 = 1, D1 = 2.
static void run(const char *name, uint32_t a0, uint32_t a1, uint32_t a7)
{
    printf("[RUN] %s\n", name);
    w(0x4E4F);
    for (int i = 0; i < 8; i++) { pjit_set_dn(i, 0); pjit_set_an(i, 0); }
    pjit_set_dn(0, 1); pjit_set_dn(1, 2);
    pjit_set_an(0, a0); pjit_set_an(1, a1);
    pjit_set_an(7, a7 ? a7 : (uint32_t)buf + 120);
    cpu->ssp = cpu->usp = __builtin_bswap32(a7 ? a7 : (uint32_t)buf + 120);
    pjit_set_sr(0x2700);
    pjit_cache_flush();
    pjit_run(prog);
    np = 0;
}

int main(void)
{
    printf("=== PJIT smoke test: Rt == Rn forms ===\n");
    pjit_cache_init((uint32_t)(ram_pool + sizeof(ram_pool)));
#ifndef PJIT_TEST_DR
    cpu->dr_threshold = 0xFFFFFFFFu;
#endif
    emit_opcode_table();
    uint32_t B = (uint32_t)buf;

    w(0x20C8);                                  // MOVE.L A0,(A0)+
    run("MOVE.L A0,(A0)+", B + 16, 0, 0);
    check("  stored the old A0", rl(16), B + 16);
    check("  A0 incremented", pjit_get_an(0), B + 20);

    w(0x2108);                                  // MOVE.L A0,-(A0)
    run("MOVE.L A0,-(A0)", B + 16, 0, 0);
    check("  stored the old A0", rl(12), B + 16);
    check("  A0 decremented", pjit_get_an(0), B + 12);

    wl_(16, 0x12345678);
    w(0x2058);                                  // MOVEA.L (A0)+,A0
    run("MOVEA.L (A0)+,A0", B + 16, 0, 0);
    check("  A0 = the loaded value", pjit_get_an(0), 0x12345678);

    wl_(12, 0xCAFEBABE);
    w(0x2060);                                  // MOVEA.L -(A0),A0
    run("MOVEA.L -(A0),A0", B + 16, 0, 0);
    check("  A0 = the loaded value", pjit_get_an(0), 0xCAFEBABE);

    w(0x48E0); w(0xC080);                       // MOVEM.L D0/D1/A0,-(A0)
    run("MOVEM.L D0/D1/A0,-(A0)", B + 32, 0, 0);
    check("  A0's initial value stored", rl(28), B + 32);
    check("  D1 stored", rl(24), 2);
    check("  D0 stored", rl(20), 1);
    check("  A0 = the lowest address", pjit_get_an(0), B + 20);

    wl_(0, 0x10); wl_(4, 0x20); wl_(8, 0x30); wl_(12, 0x40);
    w(0x4CD8); w(0x0303);                       // MOVEM.L (A0)+,D0/D1/A0/A1
    run("MOVEM.L (A0)+,D0/D1/A0/A1", B, 0, 0);
    check("  D0", pjit_get_dn(0), 0x10);
    check("  D1", pjit_get_dn(1), 0x20);
    check("  A0 = the postincremented address (the loaded 0x30 is ignored)", pjit_get_an(0), B + 16);
    check("  A1 (read through the walking pointer)", pjit_get_an(1), 0x40);

    w(0x4E57); w(0xFFF8);                       // LINK A7,#-8
    run("LINK A7,#-8", 0, 0, B + 100);
    check("  pushed the decremented A7", rl(96), B + 96);
    check("  A7 = that - 8", pjit_get_an(7), B + 88);

    w(0x4857);                                  // PEA (A7)
    run("PEA (A7)", 0, 0, B + 100);
    check("  pushed A7", rl(96), B + 100);
    check("  A7", pjit_get_an(7), B + 96);

    wl_(100, 0x00ABCDE0);
    w(0x4E5F);                                  // UNLK A7
    run("UNLK A7", 0, 0, B + 100);
    check("  A7 = the loaded value", pjit_get_an(7), 0x00ABCDE0);

    printf("[TEST] %d passed, %d failed\n", pass, fail);
    printf(fail ? "FAIL\n" : "PASS\n");
    return fail ? 1 : 0;
}
