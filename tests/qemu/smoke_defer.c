/// @file smoke_defer.c
/// @brief One translation claims at most one cache line for a link (a claim is ~16 k cycles on the board). Two calls in the same entry to two lines that are not claimed yet:
///        the first links, the second is left cold and translated by the next fill. Both calls must run right, cold and warm, and both end up linked.
#include <stdint.h>
#include "pjit.h"

extern void qemu_exit(void);
extern void emit_opcode_table(void);

static uint8_t ram_pool[12 * 1024 * 1024] __attribute__((aligned(4096)));
static uint8_t code[4096] __attribute__((aligned(1024)));
static uint8_t stack_mem[256] __attribute__((aligned(16)));
static int failures;
static void put(int hw, uint16_t v) { code[hw * 2] = v >> 8; code[hw * 2 + 1] = (uint8_t)v; }
static void check(const char *what, uint32_t got, uint32_t want) { if (got != want) { printf("[TEST] %-48s got 0x%X want 0x%X FAIL\n", what, got, want); failures++; } }

int main(void)
{
    printf("=== PJIT smoke test: at most one line claimed per translation ===\n");
    pjit_cache_init((uint32_t)(ram_pool + sizeof(ram_pool)));
    cpu->trap_exit_mask = 0x8000;
    emit_opcode_table();
    for (unsigned i = 0; i < sizeof code; i += 2) { code[i] = 0x4E; code[i + 1] = 0x71; }
    // main at 0: MOVEQ #0,D0 ; BSR.W s1 ; BSR.W s2 ; BSR.W s3 ; TRAP #15      s1 / s2 / s3 in three other 512-byte lines
    int s1 = 256, s2 = 512, s3 = 768;                     // halfword indexes: 512, 1024, 1536 bytes on
    put(0, 0x7000);
    put(1, 0x6100); put(2, (uint16_t)((s1 - 2) * 2));
    put(3, 0x6100); put(4, (uint16_t)((s2 - 4) * 2));
    put(5, 0x6100); put(6, (uint16_t)((s3 - 6) * 2));
    put(7, 0x4E4F);
    put(s1, 0x5280); put(s1 + 1, 0x4E75);                 // ADDQ.L #1,D0 ; RTS
    put(s2, 0x5480); put(s2 + 1, 0x4E75);                 // ADDQ.L #2,D0 ; RTS
    put(s3, 0x5880); put(s3 + 1, 0x4E75);                 // ADDQ.L #4,D0 ; RTS
    pjit_cache_flush();
    for (int round = 0; round < 3; round++) {
        for (int i = 0; i < 8; i++) { pjit_set_dn(i, 0x11110000u + i); pjit_set_an(i, 0); }
        uint32_t sp = (uint32_t)stack_mem + sizeof stack_mem - 16;
        pjit_set_an(7, sp); cpu->ssp = cpu->usp = __builtin_bswap32(sp);
        pjit_set_sr(0x2700);
        pjit_run((uint16_t *)code);
        check("D0 after three calls (1 + 2 + 4)", pjit_get_dn(0), 7);
        check("A7 balanced", pjit_get_an(7), sp);
    }
    check("three links were made (none lost to the deferral)", pjit_arena.links.made >= 3, 1);
    printf(failures ? "FAIL\n" : "PASS\n");
    qemu_exit();
    return failures != 0;
}
