/// @file smoke_links.c
/// @brief Direct links between cache lines (known-target JSR/BSR/JMP/BRA/Bcc): they must run the right code, and
///        when a miss evicts a line another address owns, every line holding a link must be invalidated so a stale
///        link never runs the wrong code. F1 and F2 are exactly one cache span apart, so they share a line: calling
///        F1, F2, F1 again makes the second call evict F1's line, and the third call must still find F1.
#include <stdint.h>
#include "pjit.h"
#include "mem_be.h"

extern void emit_opcode_table(void);

static uint8_t  ram_pool[12 * 1024 * 1024] __attribute__((aligned(4096)));
static uint8_t  big[0x100000 + 0x8000] __attribute__((aligned(0x80000)));
static uint8_t  stk[512] __attribute__((aligned(16)));
static int pass, fail;

static uint8_t *at(uint32_t off) { return big + off; }
static void w16(uint8_t *p, uint16_t v) { p[0] = v >> 8; p[1] = (uint8_t)v; }
static void w32(uint8_t *p, uint32_t v) { w16(p, v >> 16); w16(p + 2, v); }
static void check(const char *what, uint32_t got, uint32_t want)
{
    if (got == want) pass++;
    else { printf("[FAIL] %-34s got 0x%08X want 0x%08X\n", what, got, want); fail++; }
}

int main(void)
{
    printf("=== PJIT smoke test: direct cache-line links and eviction ===\n");
    pjit_cache_init((uint32_t)(ram_pool + sizeof(ram_pool)));
    emit_opcode_table();
    uint32_t F1 = (uint32_t)at(0x100), F2 = (uint32_t)at(0x80100);      // one cache span apart: the same line
    uint32_t M = (uint32_t)at(0x40000), T = M + 0x1000, S = M + 0x2000, J = M + 0x3000;   // other lines, same tag
    w16(at(0x100), 0x7001);   w16(at(0x102), 0x4E75);                   // F1: MOVEQ #1,D0 ; RTS
    w16(at(0x80100), 0x7002); w16(at(0x80102), 0x4E75);                 // F2: MOVEQ #2,D0 ; RTS
    // M is 16-byte aligned: the first eight words are one translation unit, so the JSR F1 below is linked
    // *before* the call to F2 evicts F1's line, and the second time round the loop it runs that old link.
    uint8_t *p = (uint8_t *)M;
    w16(p, 0x4EB9); w32(p + 2, F1); p += 6;  w16(p, 0x2600); p += 2;    // loop: JSR F1 ; MOVE.L D0,D3
    w16(p, 0x207C); w32(p + 2, F2); p += 6;  w16(p, 0x4E90); p += 2;    //       MOVEA.L #F2,A0 ; JSR (A0)  (the router: evicts F1's line)
    w16(p, 0x2800); p += 2;                                             //       MOVE.L D0,D4
    w16(p, 0x51C9); w16(p + 2, (uint16_t)(M - (uint32_t)(p + 2))); p += 4;   //   DBRA D1,loop (D1 = 1: twice round)
    w16(p, 0x6100); w16(p + 2, (uint16_t)(S - (uint32_t)(p + 2))); p += 4;   // BSR.W S (another line): MOVEQ #6,D6 ; RTS
    w16(p, 0x7000); p += 2;                                             // MOVEQ #0,D0       (Z set)
    w16(p, 0x6700); w16(p + 2, (uint16_t)(T - (uint32_t)(p + 2))); p += 4;   // BEQ.W T (another line, taken)
    w16(p, 0x7E63); p += 2;                                             // MOVEQ #99,D7: skipped
    w16(p, 0x4EF9); w32(p + 2, J);                                      // (not reached) JMP J
    w16((uint8_t *)T, 0x7C07); w16((uint8_t *)T + 2, 0x4EF9); w32((uint8_t *)T + 4, J);   // T: MOVEQ #7,D6 ; JMP J
    w16((uint8_t *)S, 0x7C06); w16((uint8_t *)S + 2, 0x4E75);           // S: MOVEQ #6,D6 ; RTS
    w16((uint8_t *)J, 0x7A09); w16((uint8_t *)J + 2, 0x4E4F);           // J: MOVEQ #9,D5 ; TRAP #15

    for (int pass_no = 0; pass_no < 3; pass_no++) {                     // 1: cold, 2 and 3: with whatever the cache kept
        for (int i = 0; i < 8; i++) { pjit_set_dn(i, 0); pjit_set_an(i, 0); }
        pjit_set_dn(1, 1);
        pjit_set_an(7, (uint32_t)stk + 480);
        cpu->ssp = cpu->usp = __builtin_bswap32((uint32_t)stk + 480);
        pjit_set_sr(0x2700);
        if (pass_no == 0) pjit_cache_flush();
        pjit_run((uint16_t *)M);
        check("F1 on the second time round, after F2 evicted its line (D3)", pjit_get_dn(3), 1);
        check("F2 call (D4)", pjit_get_dn(4), 2);
        check("BSR.W then BEQ.W then JMP: reached J (D5)", pjit_get_dn(5), 9);
        check("BEQ.W taken, ran T (D6)", pjit_get_dn(6), 7);
        check("the skipped MOVEQ did not run (D7)", pjit_get_dn(7), 0);
        check("DBRA ran out (D1)", pjit_get_dn(1) & 0xFFFF, 0xFFFF);
    }
    printf("[LINKS] %u links made, %u evictions, %u lines invalidated\n", pjit_arena.links.made, pjit_arena.links.evictions, pjit_arena.links.invalidated);
    check("links were made", pjit_arena.links.made > 0, 1);
    check("F2 evicted F1's line", pjit_arena.links.evictions > 0, 1);
    check("... and the linked lines were invalidated", pjit_arena.links.invalidated > 0, 1);
    printf("[TEST] %d passed, %d failed\n", pass, fail);
    printf(fail ? "FAIL\n" : "PASS\n");
    return fail ? 1 : 0;
}
