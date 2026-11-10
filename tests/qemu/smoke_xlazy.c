/// @file smoke_xlazy.c
/// @brief X across an instruction that clears C: ADD.L sets X and C, MOVE.L clears C and leaves X, ADDX.L must still add the carry-in.
///        The ADDX is in the same line (or the next) as the ADD: exact in every build (in an X_LAZY build the line is translated from the
///        exact twin sequences). A second copy has the ADD three lines before the ADDX: exact in an exact build, stale (documented) in a lazy one.
#include <stdint.h>
#include "pjit.h"

extern void qemu_exit(void);
extern void emit_opcode_table(void);

static uint8_t ram_pool[12 * 1024 * 1024] __attribute__((aligned(4096)));
static uint8_t program[4096] __attribute__((aligned(4096)));          // 512-byte lines: 8 of them
static uint8_t guest_stack[1024] __attribute__((aligned(8)));

static int failures;
static void check(const char *what, uint32_t got, uint32_t want)
{
    int ok = (got == want);
    printf("[TEST] %-52s got 0x%08X want 0x%08X %s\n", what, got, want, ok ? "ok" : "FAIL");
    if (!ok) failures++;
}
static void put(int off, uint16_t w) { program[off] = w >> 8; program[off + 1] = (uint8_t)w; }

// ADD.L D1,D0 ; MOVE.L D2,D3 ; ADDX.L D4,D5 ; TRAP #15  with D0 = -1, D1 = 1 (carry out), D2 = 0, D4 = D5 = 0: D5 must become 1
static uint32_t run(int base)
{
    for (int i = 0; i < 8; i++) { pjit_set_dn(i, 0); pjit_set_an(i, 0xA0000000u + i); }
    pjit_set_dn(0, 0xFFFFFFFFu); pjit_set_dn(1, 1);
    pjit_set_an(7, (uint32_t)(guest_stack + sizeof(guest_stack)));
    pjit_set_sr(0x2700);
    pjit_cache_flush();
    pjit_run((uint16_t *)(program + base));
    return pjit_get_dn(5);
}

int main(void)
{
    printf("=== PJIT smoke test: X across a C-clobbering instruction, lazy and exact lines ===\n");
    pjit_cache_init((uint32_t)(ram_pool + sizeof(ram_pool)));
    cpu->trap_exit_mask = 0x8000;
    emit_opcode_table();

    // near: everything in the first line
    put(0, 0xD081);     // ADD.L D1,D0
    put(2, 0x2602);     // MOVE.L D2,D3
    put(4, 0xDB84);     // ADDX.L D4,D5
    put(6, 0x4E4F);     // TRAP #15
    check("ADD / MOVE / ADDX in one line: D5 = carry-in", run(0), 1);

    // adjacent lines: ADD at the end of line 0, MOVE and ADDX at the start of line 1
    for (int i = 0; i < 4096; i++) program[i] = 0;
    put(504, 0xD081);   // ADD.L D1,D0   (last-but-two word of line 0)
    put(506, 0x4E71);   // NOP
    put(508, 0x4E71);   // NOP
    put(510, 0x4E71);   // NOP
    put(512, 0x2602);   // MOVE.L D2,D3  (first word of line 1)
    put(514, 0xDB84);   // ADDX.L D4,D5
    put(516, 0x4E4F);   // TRAP #15
    check("ADD at the end of a line, ADDX in the next: D5", run(504), 1);

    // far: ADD in line 0, ADDX three lines later (BRA.W over the gap)
    for (int i = 0; i < 4096; i++) program[i] = 0;
    put(0, 0xD081);     // ADD.L D1,D0
    put(2, 0x6000); put(4, 1536 - 4);              // BRA.W to 1536  (the displacement is from the extension word at 4)
    put(1536, 0x2602);  // MOVE.L D2,D3
    put(1538, 0xDB84);  // ADDX.L D4,D5
    put(1540, 0x4E4F);  // TRAP #15
    uint32_t far = run(0);
    for (int l = 0; l < 4; l++) printf("[INFO] line %d (guest %08X): xexact = %d\n", l, (unsigned)(program + 512 * l), pjit_arena.xexact[(((uint32_t)program >> 9) + l) & 1023]);
    printf("[INFO] ADD three lines before the ADDX: D5 = %u (%s)\n", far, far == 1 ? "exact" : "stale X: a lazy build, documented");

    printf(failures ? "FAIL\n" : "PASS\n");
    qemu_exit();
    return failures != 0;
}
