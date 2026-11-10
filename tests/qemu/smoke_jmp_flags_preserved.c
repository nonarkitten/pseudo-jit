/// @file smoke_jmp_flags_preserved.c
/// @brief The live CCR survives an instruction that the slot cache hands to the
///        interpreter. (d8,PC,Xn) never fits a slot: the interpreter single-steps
///        it and the router re-enters the cache at the next instruction. The ARM
///        flags are the 68K CCR, so the router parks them in cpu->jmp_cpsr and puts
///        them back before jumping in; the next instruction must see the flags the
///        stepped one set.
///
/// Program: BTST.B #0,LABEL(pc,d0.w), where LABEL is the instruction's own
/// address (d0=0) -- the self-referential pattern golden/mcl68k_testsuite.c's
/// op_BTST0 uses -- then MOVE SR,D2 straight after. (MOVEQ marker goes
/// *after* the SR read: MOVEQ sets flags itself.) The D3 marker confirms
/// the re-entry landed in the right place.
#include <stdint.h>
#include "pjit.h"
#include "pjit_compat.h"

extern void qemu_exit(void);
extern void emit_opcode_table(void);

static uint8_t ram_pool[12 * 1024 * 1024] __attribute__((aligned(4096)));

// MOVEQ #0,D0 ; LABEL: BTST.B #0,LABEL(pc,d0.w) ; MOVE SR,D2 ; MOVEQ #0x55,D3 ; TRAP #15
static uint16_t m68k_program[7] __attribute__((aligned(4)));

static void write_be16(uint16_t *addr, uint16_t value)
{
    uint8_t *b = (uint8_t *)addr;
    b[0] = (uint8_t)(value >> 8);
    b[1] = (uint8_t)(value & 0xFF);
}

void pjit_test_exit(uint32_t d0)
{
    uint32_t sr = pjit_get_dn(2) & 0xFFFF;
    uint32_t d3 = pjit_get_dn(3);
    printf("[TEST] D0=0x%08X D3=0x%08X (want 0x55 -- confirms redispatch reached the marker) SR=0x%04X (Z bit = %u, want 1)\n",
           d0, d3, sr, (sr >> 2) & 1);
    printf((sr & 0x4) != 0 && d3 == 0x55 ? "PASS\n" : "FAIL\n");
    qemu_exit();
}

int main(void)
{
    printf("=== PJIT smoke test: self-referential BTST.B #0,LABEL(pc,d0.w) ===\n");

    pjit_cache_init((uint32_t)(ram_pool + sizeof(ram_pool)));
    emit_opcode_table();

    write_be16(&m68k_program[0], 0x7000); // MOVEQ #0,D0
    write_be16(&m68k_program[1], 0x083B); // LABEL: BTST.B #0,LABEL(pc,d0.w)
    write_be16(&m68k_program[2], 0x0000); // bit number = 0
    write_be16(&m68k_program[3], 0x00FC); // (d8,pc,d0.w), disp=-4 -> back to word[1]
    write_be16(&m68k_program[4], 0x40C2); // MOVE SR,D2 (flags must survive the interpreted BTST)
    write_be16(&m68k_program[5], 0x7655); // MOVEQ #0x55,D3 (marker; after SR since MOVEQ sets flags)
    write_be16(&m68k_program[6], 0x4E4F); // TRAP #15

    uint32_t m68k_pc = (uint32_t)m68k_program;
    uint32_t arm_entry = pjit_cache_dispatch(m68k_pc);

    printf("[TEST] pjit_cache_dispatch() returned 0x%08X\n", arm_entry);
    if (!arm_entry) { printf("FAIL\n"); qemu_exit(); return 0; }

    PJIT_TEST_RUN(arm_entry);

    printf("[TEST] unexpectedly returned from JIT'd code\n");
    printf("FAIL\n");
    qemu_exit();
    return 0;
}
