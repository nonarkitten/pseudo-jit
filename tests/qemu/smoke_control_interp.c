/// @file smoke_control_interp.c
/// @brief The interpretive runtime's control-flow edge cases: an odd guest PC
///        from every instruction that can produce one (BRA/Bcc/BSR/DBcc/JMP/
///        JSR/RTS) raises an address error with the right fault address,
///        function code and frame PC; the same instructions with an odd
///        displacement that is *not* taken, or an even target, do not; STOP
///        ends pjit_run(); a user-mode STOP raises a privilege violation.
#include <stdint.h>
#include "pjit.h"
#include "mem_be.h"

extern void qemu_exit(void);
extern void emit_opcode_table(void);

static uint8_t  ram_pool[12 * 1024 * 1024] __attribute__((aligned(4096)));
static uint16_t prog[64]    __attribute__((aligned(8)));
static uint16_t hand[64]    __attribute__((aligned(8)));
static uint32_t vectors[64] __attribute__((aligned(4)));
static uint8_t  stack[1024] __attribute__((aligned(8)));

static int np, nh, pass, fail;
static void wbe(uint16_t *a, uint16_t v) { ((uint8_t *)a)[0] = v >> 8; ((uint8_t *)a)[1] = (uint8_t)v; }
static void w(uint16_t v)   { wbe(&prog[np++], v); }

/// Handler for `vector`: records the frame words it is interested in, sets D0 to
/// `marker`, and exits. Address-error frames (from SP up): fc, address(L),
/// instruction word, SR, PC(L).
static void handler(int vector, uint8_t marker)
{
    uint16_t *h = &hand[nh]; int k = 0;
    wbe(&h[k++], 0x3417);                       // MOVE.W (A7),D2        the function-code word
    wbe(&h[k++], 0x222F); wbe(&h[k++], 2);      // MOVE.L 2(A7),D1       the access address
    wbe(&h[k++], 0x282F); wbe(&h[k++], 10);     // MOVE.L 10(A7),D4      the stacked PC
    wbe(&h[k++], 0x7000 | marker);              // MOVEQ #marker,D0
    wbe(&h[k++], 0x4E4F);                       // TRAP #15
    write_BE_long((uint32_t)&vectors[vector], (uint32_t)h);
    nh += 8;
}

typedef struct { uint32_t d[8], a[8]; uint16_t sr; } st_t;

static uint32_t cycle(st_t r, uint32_t *d1, uint32_t *d2, uint32_t *d4)
{
    w(0x4E4F);                                  // TRAP #15: a program that falls off the end exits
    for (int i = 0; i < 8; i++) { pjit_set_dn(i, r.d[i]); pjit_set_an(i, r.a[i]); }
    if (!r.a[7]) pjit_set_an(7, (uint32_t)(stack + sizeof(stack)));
    cpu->ssp = cpu->usp = __builtin_bswap32(r.a[7] ? r.a[7] : (uint32_t)(stack + sizeof(stack)));
    pjit_set_sr(r.sr ? r.sr : 0x2700);
    pjit_cache_flush();                         // the same addresses hold a different program each case
    uint32_t vec = pjit_run(prog);
    *d1 = pjit_get_dn(1); *d2 = pjit_get_dn(2); *d4 = pjit_get_dn(4);
    np = 0;
    return vec;
}

static void check(const char *what, uint32_t got, uint32_t want)
{
    if (got == want) pass++;
    else { printf("[FAIL] %-34s got 0x%08X want 0x%08X\n", what, got, want); fail++; }
}

/// Run `prog` (already written) expecting an address error on `badaddr`.
static void expect_addr_error(const char *name, st_t r, uint32_t badaddr)
{
    uint32_t d1, d2, d4;
    printf("[RUN] %s\n", name);
    cycle(r, &d1, &d2, &d4);
    check(name, pjit_get_dn(0), 0x2A);          // the address-error handler ran
    check("  fault address", d1, badaddr);
    check("  function code (supv. program read)", d2 & 0xFFFF, 0x16);
    check("  stacked PC", d4, badaddr);
}

int main(void)
{
    printf("=== PJIT smoke test: interpretive control-flow edge cases ===\n");
    pjit_cache_init((uint32_t)(ram_pool + sizeof(ram_pool)));
#ifndef PJIT_TEST_DR
    cpu->dr_threshold = 0xFFFFFFFFu;            // interpretive only
#endif
    emit_opcode_table();
    cpu->vbr = (uint32_t)vectors;
    for (int v = 0; v < 64; v++) write_BE_long((uint32_t)&vectors[v], 0);
    handler(ADDRESSERR, 0x2A);
    handler(PRIV, 0x3B);

    st_t r;
    #define R() (r = (st_t){ {0}, {0}, 0 })
    uint32_t base = (uint32_t)prog;

    // ---- an odd PC raises an address error ----
    R(); w(0x6001);                                                 // BRA.S +1: target = pc + 2 + 1
    expect_addr_error("BRA.S to an odd address", r, base + 3);
    R(); w(0x6101);
    expect_addr_error("BSR.S to an odd address", r, base + 3);
    R(); r.d[5] = 0; w(0x7A00); w(0x6701);                          // MOVEQ #0,D5 (Z) ; BEQ.S +1, taken
    expect_addr_error("BEQ.S taken, odd", r, base + 5);
    R(); w(0x7C01); w(0x51CE); w(3);                                // MOVEQ #1,D6 ; DBF D6,*+3
    expect_addr_error("DBF to an odd address", r, base + 4 + 3);
    R(); r.a[0] = base + 9; w(0x4ED0);                              // JMP (A0), A0 odd
    expect_addr_error("JMP (A0), odd", r, base + 9);
    R(); r.a[0] = base + 9; w(0x4E90);                              // JSR (A0), A0 odd
    expect_addr_error("JSR (A0), odd", r, base + 9);
    {
        uint32_t sp = (uint32_t)(stack + sizeof(stack)) - 16;
        write_BE_long(sp, base + 5);
        R(); r.a[7] = sp; w(0x4E75);                                // RTS to an odd return address
        expect_addr_error("RTS to an odd address", r, base + 5);
    }

    // ---- no error when the target is even, or the branch is not taken ----
    uint32_t d1, d2, d4;
    R(); w(0x7A01); w(0x6701); w(0x7005);                           // MOVEQ #1,D5 (not Z) ; BEQ.S +1 not taken ; MOVEQ #5,D0
    printf("[RUN] BEQ.S not taken, odd displacement\n");
    cycle(r, &d1, &d2, &d4);
    check("BEQ.S not taken: falls through", pjit_get_dn(0), 5);
    R(); w(0x6002); w(0x7001); w(0x7007);                           // BRA.S +2: skips the first MOVEQ
    printf("[RUN] BRA.S to an even address\n");
    cycle(r, &d1, &d2, &d4);
    check("BRA.S +2: lands on the second MOVEQ", pjit_get_dn(0), 7);

    // ---- STOP ----
    R(); w(0x4E72); w(0x2004);                                      // STOP #$2004 (supervisor)
    printf("[RUN] STOP (supervisor)\n");
    {
        w(0x4E4F); // (never reached: STOP ends the run) -- but cycle() appends its own TRAP too
        np--;      // keep prog tidy
        for (int i = 0; i < 8; i++) { pjit_set_dn(i, 0); pjit_set_an(i, 0); }
        pjit_set_an(7, (uint32_t)(stack + sizeof(stack)));
        cpu->ssp = cpu->usp = __builtin_bswap32((uint32_t)(stack + sizeof(stack)));
        pjit_set_sr(0x2700);
        pjit_cache_flush();
        uint32_t vec = pjit_run(prog);
        check("STOP ends pjit_run()", vec, PJIT_VEC_STOP);
        check("STOP loaded the SR", pjit_get_sr(), 0x2004);
        check("STOP left the PC just past it", (uint32_t)cpu->pc, base + 4);
        np = 0;
    }
    R(); r.sr = 0x0001; w(0x4E72); w(0x2000);                       // user mode (C set): privilege violation
    printf("[RUN] STOP in user mode\n");
    cycle(r, &d1, &d2, &d4);
    check("user STOP -> privilege violation", pjit_get_dn(0), 0x3B);

    printf("[TEST] %d passed, %d failed\n", pass, fail);
    printf(fail ? "FAIL\n" : "PASS\n");
    qemu_exit();
    return fail != 0;
}
