/// @file smoke_rts.c
/// @brief The shadow return stack: cached calls record (guest, host) return addresses and a cached RTS that finds
///        its popped address on top jumps straight back. It must be invisible to the guest, so: plain returns,
///        recursion deeper than the 32-entry ring, a callee that rewrites its own return address, and a callee
///        that evicts its caller's cache line (the stale host address must never be used).
#include <stdint.h>
#include "pjit.h"
#include "mem_be.h"

extern void emit_opcode_table(void);

static uint8_t  ram_pool[12 * 1024 * 1024] __attribute__((aligned(4096)));
static uint8_t  big[0x100000 + 0x8000] __attribute__((aligned(0x80000)));
static uint8_t  stk[1024] __attribute__((aligned(16)));
static int pass, fail;
static uint8_t *at(uint32_t off) { return big + off; }
static void w16(uint8_t *p, uint16_t v) { p[0] = v >> 8; p[1] = (uint8_t)v; }
static void w32(uint8_t *p, uint32_t v) { w16(p, v >> 16); w16(p + 2, v); }
static void check(const char *what, uint32_t got, uint32_t want)
{
    if (got == want) pass++;
    else { printf("[FAIL] %-44s got 0x%08X want 0x%08X\n", what, got, want); fail++; }
}

int main(void)
{
    printf("=== PJIT smoke test: the shadow return stack ===\n");
    pjit_cache_init((uint32_t)(ram_pool + sizeof(ram_pool)));
    emit_opcode_table();
    uint32_t F0 = (uint32_t)at(0x000), G = (uint32_t)at(0x100), REC = (uint32_t)at(0x2000), M = (uint32_t)at(0x1000);
    // F0: MOVEQ #1,D0 ; RTS
    w16(at(0x000), 0x7001); w16(at(0x002), 0x4E75);
    // M: the program (below), ALT is inside it
    uint8_t *p = (uint8_t *)M;
    w16(p, 0x4EB9); w32(p + 2, F0); p += 6;                 // JSR F0
    w16(p, 0x2400); p += 2;                                 // MOVE.L D0,D2          (D2 = 1)
    w16(p, 0x4EB9); w32(p + 2, G); p += 6;                  // JSR G                 (G returns to ALT, not here)
    w16(p, 0x7663); p += 2;                                 // MOVEQ #99,D3          (skipped)
    uint32_t ALT = (uint32_t)p;
    w16(p, 0x7803); p += 2;                                 // ALT: MOVEQ #3,D4
    w16(p, 0x7064); p += 2;                                 // MOVEQ #100,D0
    w16(p, 0x4EB9); w32(p + 2, REC); p += 6;                // JSR REC               (99 nested calls)
    // (d): call a routine that shares this line's cache index at another tag
    uint32_t HADDR = (((uint32_t)p) & ~0x1FFu) + 0x80000 + 0x100;
    w16(p, 0x4EB9); w32(p + 2, HADDR); p += 6;              // JSR H                 (evicts this line while it runs)
    w16(p, 0x7C08); p += 2;                                 // MOVEQ #8,D6           (must still run)
    w16(p, 0x4E4F); p += 2;                                 // TRAP #15
    w16(at(0x100), 0x2EBC); w32(at(0x102), ALT); w16(at(0x106), 0x4E75);    // G: MOVE.L #ALT,(A7) ; RTS
    // REC: SUBQ.L #1,D0 ; BEQ.S out ; JSR REC ; ADDQ.L #1,D5 ; out: RTS
    uint8_t *r = at(0x2000);
    w16(r, 0x5380); r += 2; w16(r, 0x6700 | 8); r += 2;
    w16(r, 0x4EB9); w32(r + 2, REC); r += 6;
    w16(r, 0x5285); r += 2;
    w16(r, 0x4E75);
    // H: MOVEQ #5,D7 ; RTS
    w16((uint8_t *)HADDR, 0x7E05); w16((uint8_t *)HADDR + 2, 0x4E75);

    for (int round = 0; round < 3; round++) {                // cold, then with whatever the cache and shadow stack kept
        for (int i = 0; i < 8; i++) { pjit_set_dn(i, 0); pjit_set_an(i, 0); }
        pjit_set_an(7, (uint32_t)stk + 960);
        cpu->ssp = cpu->usp = __builtin_bswap32((uint32_t)stk + 960);
        pjit_set_sr(0x2700);
        if (round == 0) pjit_cache_flush();
        pjit_run((uint16_t *)M);
        check("plain call and return (D2)", pjit_get_dn(2), 1);
        check("callee changed its return address: skipped (D3)", pjit_get_dn(3), 0);
        check("... and the program resumed at ALT (D4)", pjit_get_dn(4), 3);
        check("recursion deeper than the ring: every level returned (D5)", pjit_get_dn(5), 99);
        check("callee evicted its caller's line (D7)", pjit_get_dn(7), 5);
        check("... and the caller continued correctly (D6)", pjit_get_dn(6), 8);
    }
    printf("[LINKS] %u links made, %u evictions, %u lines invalidated\n", pjit_arena.links.made, pjit_arena.links.evictions, pjit_arena.links.invalidated);
    printf("[TEST] %d passed, %d failed\n", pass, fail);
    printf(fail ? "FAIL\n" : "PASS\n");
    return fail ? 1 : 0;
}
