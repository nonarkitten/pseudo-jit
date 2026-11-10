/// @file smoke_irq_interp.c
/// @brief A level-6 interrupt (the generic timer through the GIC, see platform/qemu_virt/irq.S)
///        lands on guest code that never returns to anything else: BRA to itself, a two-
///        instruction loop, a JSR/RTS loop through the router. Each must be interrupted
///        between instructions with the right stacked PC and SR, run the autovector's handler
///        with S set and the mask at 6, and a level the SR masks must stay posted instead.
#include <stdint.h>
#include "pjit.h"
#include "mem_be.h"

extern void emit_opcode_table(void);
extern void qemu_virt_timer_arm(uint32_t ticks);

static uint8_t  ram_pool[12 * 1024 * 1024] __attribute__((aligned(4096)));
static uint16_t prog[64]    __attribute__((aligned(256)));
static uint16_t sub[64]     __attribute__((aligned(256)));
static uint16_t hand[32]    __attribute__((aligned(8)));
static uint32_t vectors[64] __attribute__((aligned(4)));
static uint8_t  stack[1024] __attribute__((aligned(8)));

static int np, pass, fail;
static void wbe(uint16_t *a, uint16_t v) { ((uint8_t *)a)[0] = v >> 8; ((uint8_t *)a)[1] = (uint8_t)v; }
static void w(uint16_t v)   { wbe(&prog[np++], v); }
static void wl(uint32_t v)  { w(v >> 16); w(v & 0xFFFF); }

static void check(const char *what, uint32_t got, uint32_t want)
{
    if (got == want) pass++;
    else { printf("[FAIL] %-40s got 0x%08X want 0x%08X\n", what, got, want); fail++; }
}

/// The level-6 autovector's handler: D2 = the stacked SR, D4 = the stacked PC, D3 = the
/// SR it runs with, D0 = 0x66, then exit.
static void install_handler(void)
{
    int k = 0;
    wbe(&hand[k++], 0x3417);                    // MOVE.W (A7),D2
    wbe(&hand[k++], 0x282F); wbe(&hand[k++], 2);// MOVE.L 2(A7),D4
    wbe(&hand[k++], 0x40C3);                    // MOVE SR,D3
    wbe(&hand[k++], 0x7066);                    // MOVEQ #$66,D0
    wbe(&hand[k++], 0x4E4F);                    // TRAP #15
    write_BE_long((uint32_t)&vectors[30], (uint32_t)hand);
}

static void run(const char *name, uint16_t sr, uint32_t ticks, uint32_t *pc_out)
{
    printf("[RUN] %s\n", name);
    for (int i = 0; i < 8; i++) { pjit_set_dn(i, 0); pjit_set_an(i, 0); }
    pjit_set_an(7, (uint32_t)(stack + sizeof(stack)));
    cpu->ssp = cpu->usp = __builtin_bswap32((uint32_t)(stack + sizeof(stack)));
    pjit_set_sr(sr);
    cpu->irq_level = 0;
    pjit_cache_flush();
    qemu_virt_timer_arm(ticks);
    pjit_run(prog);
    *pc_out = pjit_get_dn(4);
}

int main(void)
{
    printf("=== PJIT smoke test: interrupts ===\n");
    pjit_cache_init((uint32_t)(ram_pool + sizeof(ram_pool)));
#ifndef PJIT_TEST_DR
    cpu->dr_threshold = 0xFFFFFFFFu;
#endif
    emit_opcode_table();
    pjit_irq_enable(1);
    cpu->vbr = (uint32_t)vectors;
    for (int v = 0; v < 64; v++) write_BE_long((uint32_t)&vectors[v], 0);
    install_handler();
    uint32_t base = (uint32_t)prog, pc;

    // ---- BRA.S to itself: the same-line loop an interrupt must be able to leave ----
    np = 0; w(0x60FE);
    run("BRA.S *", 0x2000, 125000, &pc);
    check("  handler ran", pjit_get_dn(0), 0x66);
    check("  stacked PC", pc, base);
    check("  stacked SR", pjit_get_dn(2) & 0xFFFF, 0x2000);
    check("  handler SR (S, mask 6)", pjit_get_dn(3) & 0xFFFF, 0x2600);

    // ---- ADDQ.L #1,D6 ; BRA.S back ----
    np = 0; w(0x5286); w(0x60FC);
    run("ADDQ.L #1,D6 ; BRA.S", 0x2000, 125000, &pc);
    check("  handler ran", pjit_get_dn(0), 0x66);
    check("  stacked PC is an instruction boundary", (pc == base || pc == base + 2), 1);
    check("  the loop made progress", pjit_get_dn(6) > 100, 1);

    // ---- a subroutine call in the loop: the router is the window ----
    wbe(&sub[0], 0x5286);                       // ADDQ.L #1,D6
    wbe(&sub[1], 0x4E75);                       // RTS
    np = 0; w(0x4EB9); wl((uint32_t)sub); w(0x60F8);    // JSR sub ; BRA.S back
    run("JSR sub ; BRA.S", 0x2000, 125000, &pc);
    check("  handler ran", pjit_get_dn(0), 0x66);
    check("  the loop made progress", pjit_get_dn(6) > 100, 1);
    check("  stacked PC is a boundary", (pc == base || pc == base + 6 || pc == (uint32_t)sub || pc == (uint32_t)sub + 2), 1);

    // ---- level 6 against a mask of 6: not delivered, stays posted ----
    np = 0; w(0x7C00); w(0x5286); w(0x0C86); wl(2000000); w(0x66F6); w(0x4E4F); // MOVEQ #0,D6; ADDQ; CMPI.L #2000000,D6; BNE; TRAP #15
    run("masked: level 6 at mask 6", 0x2600, 125000, &pc);
    check("  the handler did not run", pjit_get_dn(0) != 0x66, 1);
    check("  level 6 is still posted", cpu->irq_level, 6);

    // ---- posted while masked, delivered the moment an instruction lowers the mask ----
    np = 0; w(0x7C00); w(0x5286); w(0x0C86); wl(2000000); w(0x66F6);    // spin ~0.2 s at mask 6
    w(0x027C); w(0xF8FF);                                               // ANDI #$F8FF,SR
    w(0x60FE);                                                          // BRA.S *
    run("masked, then ANDI #$F8FF,SR", 0x2600, 125000, &pc);
    check("  handler ran", pjit_get_dn(0), 0x66);
    check("  stacked PC (just after the ANDI)", pc, base + 16);
    check("  stacked SR (mask lowered, Z from the loop)", pjit_get_dn(2) & 0xFFFF, 0x2004);

    np = 0; w(0x7C00); w(0x5286); w(0x0C86); wl(2000000); w(0x66F6);
    w(0x2F3C); wl(base + 28);                                           // MOVE.L #spin,-(A7)
    w(0x3F3C); w(0x2000);                                               // MOVE.W #$2000,-(A7)
    w(0x4E73);                                                          // RTE
    w(0x60FE);                                                          // spin: BRA.S *
    run("masked, then RTE to mask 0", 0x2600, 125000, &pc);
    check("  handler ran", pjit_get_dn(0), 0x66);
    check("  stacked PC (the RTE's target)", pc, base + 28);

    np = 0; w(0x7C00); w(0x5286); w(0x0C86); wl(2000000); w(0x66F6);
    w(0x46FC); w(0x2000);                                               // MOVE #$2000,SR
    w(0x60FE);
    run("masked, then MOVE #$2000,SR", 0x2600, 125000, &pc);
    check("  handler ran", pjit_get_dn(0), 0x66);
    check("  stacked PC", pc, base + 16);

    // ---- STOP waits for it ----
    np = 0; w(0x4E72); w(0x2000);                                       // STOP #$2000
    w(0x60FE);
    run("STOP #$2000 wakes for level 6", 0x2700, 125000, &pc);
    check("  handler ran", pjit_get_dn(0), 0x66);
    check("  stacked PC (after the STOP)", pc, base + 4);
    check("  stacked SR", pjit_get_dn(2) & 0xFFFF, 0x2000);

    printf("[TEST] %d passed, %d failed\n", pass, fail);
    printf(fail ? "FAIL\n" : "PASS\n");
    return fail ? 1 : 0;
}
