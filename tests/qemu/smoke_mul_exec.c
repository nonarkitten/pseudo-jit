/// @file smoke_mul_exec.c
/// @brief MULU/MULS: the full 32-bit product must survive (the operands
///        are narrowed, not the result), and V and C must end clear.
#include <stdint.h>
#include "pjit.h"
#include "pjit_compat.h"

extern void qemu_exit(void);
extern void emit_opcode_table(void);

static uint8_t ram_pool[12 * 1024 * 1024] __attribute__((aligned(4096)));

// word00: 0x7400  MOVEQ #0,D2              -- error accumulator, starts clean
//
// Poison V=1,C=1 (N=0,Z=0): 0x80000000 + 0xFFFFFFFF = 0x7FFFFFFF, which
// is a 33-bit-wide result (C=1, unsigned carry-out) that also flips sign
// from two negative operands to a positive result (V=1, signed overflow).
// word01: 0x2C3C  MOVE.L #0x80000000,D6 (word0)
// word02: 0x8000  ................... (high 16 bits)
// word03: 0x0000  ................... (low 16 bits)
// word04: 0x7EFF  MOVEQ #-1,D7
// word05: 0xDE86  ADD.L D6,D7              -- D7=0x7FFFFFFF; N=0,Z=0,V=1,C=1
//
// Case 1/2: MULU.W D1,D0 = 2*3 = 6 -- V and C must both clear.
// word06: 0x7002  MOVEQ #2,D0
// word07: 0x7203  MOVEQ #3,D1
// word08: 0xC0C1  MULU.W D1,D0             -- D0=6
// word09: 0x6802  BVC.S +2                 -- taken if V clear (expected)
// word10: 0x5282  ADDQ.L #1,D2             -- ERROR: V still set (bit 1)
// word11: 0x6402  BCC.S +2                 -- taken if C clear (expected)
// word12: 0x5482  ADDQ.L #2,D2             -- ERROR: C still set (bit 2)
//
// Case 3: MULU.W 0xFFFF*0xFFFF -- N must be set (real unsigned product's
// bit31), catching a sign-extension-instead-of-zero-extension bug too.
// word13: 0x303C  MOVE.W #0xFFFF,D0 (word0)
// word14: 0xFFFF  ..................
// word15: 0x323C  MOVE.W #0xFFFF,D1 (word0)
// word16: 0xFFFF  ..................
// word17: 0xC0C1  MULU.W D1,D0             -- D0=0xFFFE0001
// word18: 0x6B02  BMI.S +2                 -- taken if N set (expected)
// word19: 0x5882  ADDQ.L #4,D2             -- ERROR: N clear (zero-ext bug)
//
// Case 4: MULS.W (-1)*(-1) -- N must be clear (real product is +1),
// catching a zero-extension-instead-of-sign-extension bug the other way.
// word20: 0x303C  MOVE.W #0xFFFF,D0 (word0)  -- 0xFFFF == -1 as a signed word
// word21: 0xFFFF  ..................
// word22: 0x323C  MOVE.W #0xFFFF,D1 (word0)
// word23: 0xFFFF  ..................
// word24: 0xC1C1  MULS.W D1,D0             -- D0=1 if correct
// word25: 0x6A02  BPL.S +2                 -- taken if N clear (expected)
// word26: 0x5082  ADDQ.L #8,D2             -- ERROR: N set (zero-ext bug)
// word27: 0x4E4F  TRAP #15
static uint16_t m68k_program[28] __attribute__((aligned(4)));

static void write_be16(uint16_t *addr, uint16_t value)
{
    uint8_t *b = (uint8_t *)addr;
    b[0] = (uint8_t)(value >> 8);
    b[1] = (uint8_t)(value & 0xFF);
}

// D2 (unlike D0/D1) isn't register-pinned -- it lives in cpu_state
// memory the whole time (see tests/qemu/smoke_sub_exec.c's identical comment).
void pjit_test_exit(uint32_t d0)
{
    uint32_t d2 = __builtin_bswap32(cpu->d2);
    printf("[TEST] D0 = 0x%08X (want 0x00000001), D2 = 0x%08X "
           "(want 0x00000000 -- bit0=V,bit1=C,bit2=N/MULU,bit3=N/MULS)\n",
           d0, d2);
    printf((d0 == 1u && d2 == 0u) ? "PASS\n" : "FAIL\n");
    qemu_exit();
}

int main(void)
{
    printf("=== PJIT smoke test: MULU.W/MULS.W -- flag and zero/sign-extension correctness ===\n");

    pjit_cache_init((uint32_t)(ram_pool + sizeof(ram_pool)));
    emit_opcode_table();

    write_be16(&m68k_program[0],  0x7400);
    write_be16(&m68k_program[1],  0x2C3C);
    write_be16(&m68k_program[2],  0x8000);
    write_be16(&m68k_program[3],  0x0000);
    write_be16(&m68k_program[4],  0x7EFF);
    write_be16(&m68k_program[5],  0xDE86);
    write_be16(&m68k_program[6],  0x7002);
    write_be16(&m68k_program[7],  0x7203);
    write_be16(&m68k_program[8],  0xC0C1);
    write_be16(&m68k_program[9],  0x6802);
    write_be16(&m68k_program[10], 0x5282);
    write_be16(&m68k_program[11], 0x6402);
    write_be16(&m68k_program[12], 0x5482);
    write_be16(&m68k_program[13], 0x303C);
    write_be16(&m68k_program[14], 0xFFFF);
    write_be16(&m68k_program[15], 0x323C);
    write_be16(&m68k_program[16], 0xFFFF);
    write_be16(&m68k_program[17], 0xC0C1);
    write_be16(&m68k_program[18], 0x6B02);
    write_be16(&m68k_program[19], 0x5882);
    write_be16(&m68k_program[20], 0x303C);
    write_be16(&m68k_program[21], 0xFFFF);
    write_be16(&m68k_program[22], 0x323C);
    write_be16(&m68k_program[23], 0xFFFF);
    write_be16(&m68k_program[24], 0xC1C1);
    write_be16(&m68k_program[25], 0x6A02);
    write_be16(&m68k_program[26], 0x5082);
    write_be16(&m68k_program[27], 0x4E4F);

    uint32_t m68k_pc = (uint32_t)m68k_program;
    uint32_t arm_entry = pjit_cache_dispatch(m68k_pc);

    printf("[TEST] pjit_cache_dispatch() returned 0x%08X\n", arm_entry);

    if (!arm_entry) {
        printf("FAIL\n");
        qemu_exit();
        return 0;
    }

    pjit_set_dn(0, 0xDEADBEEFu);
    cpu->d2 = 0xDEADBEEFu;

    PJIT_TEST_RUN(arm_entry);

    printf("[TEST] unexpectedly returned from JIT'd code -- TRAP #15 exit didn't fire\n");
    printf("FAIL\n");
    qemu_exit();
    return 0;
}
