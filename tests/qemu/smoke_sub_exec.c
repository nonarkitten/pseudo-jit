/// @file smoke_sub_exec.c
/// @brief SUB.L value and carry flag. ARM's carry for a subtract is the
///        inverse of 68K's, so this checks the fixup across the boundary
///        cases including dst == src.
#include <stdint.h>
#include "pjit.h"
#include "pjit_compat.h"

extern void qemu_exit(void);
extern void emit_opcode_table(void);

static uint8_t ram_pool[12 * 1024 * 1024] __attribute__((aligned(4096)));

// word00: 0x7400  MOVEQ #0,D2        -- error accumulator, starts clean
// word01: 0x7005  MOVEQ #5,D0
// word02: 0x7203  MOVEQ #3,D1
// word03: 0x9081  SUB.L D1,D0        -- D0 = 5-3 = 2, C should be clear
// word04: 0x6402  BCC.S +2           -- taken if C clear (expected)
// word05: 0x5282  ADDQ.L #1,D2       -- ERROR: case 1's C was set
// word06: 0x7003  MOVEQ #3,D0
// word07: 0x9081  SUB.L D1,D0        -- D0 = 3-3 = 0, C should be clear (boundary)
// word08: 0x6402  BCC.S +2           -- taken if C clear (expected)
// word09: 0x5482  ADDQ.L #2,D2       -- ERROR: boundary case's C was set
// word10: 0x7001  MOVEQ #1,D0
// word11: 0x9081  SUB.L D1,D0        -- D0 = 1-3 = -2 (0xFFFFFFFE), C should be set
// word12: 0x6502  BCS.S +2           -- taken if C set (expected)
// word13: 0x5882  ADDQ.L #4,D2       -- ERROR: case 3's C was clear
// word14: 0x4E4F  TRAP #15
static uint16_t m68k_program[15] __attribute__((aligned(4)));

static void write_be16(uint16_t *addr, uint16_t value)
{
    uint8_t *b = (uint8_t *)addr;
    b[0] = (uint8_t)(value >> 8);
    b[1] = (uint8_t)(value & 0xFF);
}

/// @brief TRAP #15 handler. D2 is read straight out of cpu_state, and
///        needs a byte swap: it was written with ambient E=BE.
void pjit_test_exit(uint32_t d0)
{
    uint32_t d2 = __builtin_bswap32(cpu->d2);
    printf("[TEST] D0 = 0x%08X (want 0xFFFFFFFE), D2 = 0x%08X (want 0x00000000 -- bit0/1/2 = case 1/2/3's carry flag wrong)\n", d0, d2);
    printf((d0 == 0xFFFFFFFEu && d2 == 0u) ? "PASS\n" : "FAIL\n");
    qemu_exit();
}

int main(void)
{
    printf("=== PJIT smoke test: general SUB.L Dn,Dn -- value and carry-flag correctness ===\n");

    pjit_cache_init((uint32_t)(ram_pool + sizeof(ram_pool)));
    emit_opcode_table();

    write_be16(&m68k_program[0],  0x7400);
    write_be16(&m68k_program[1],  0x7005);
    write_be16(&m68k_program[2],  0x7203);
    write_be16(&m68k_program[3],  0x9081);
    write_be16(&m68k_program[4],  0x6402);
    write_be16(&m68k_program[5],  0x5282);
    write_be16(&m68k_program[6],  0x7003);
    write_be16(&m68k_program[7],  0x9081);
    write_be16(&m68k_program[8],  0x6402);
    write_be16(&m68k_program[9],  0x5482);
    write_be16(&m68k_program[10], 0x7001);
    write_be16(&m68k_program[11], 0x9081);
    write_be16(&m68k_program[12], 0x6502);
    write_be16(&m68k_program[13], 0x5882);
    write_be16(&m68k_program[14], 0x4E4F);

    uint32_t m68k_pc = (uint32_t)m68k_program;
    uint32_t arm_entry = pjit_cache_dispatch(m68k_pc);

    printf("[TEST] pjit_cache_dispatch() returned 0x%08X\n", arm_entry);

    if (!arm_entry) {
        printf("FAIL\n");
        qemu_exit();
        return 0;
    }

    // Set D0/D2 last -- see tests/qemu/smoke_ext_exec.c's file header for
    // why this can't happen before pjit_cache_dispatch() (or any
    // other call). Not strictly load-bearing for *this* program
    // (word0 is MOVEQ #0,D2 and word1 is MOVEQ #5,D0, both
    // unconditional overwrites), but kept consistent with the other
    // new tests rather than relying on that.
    pjit_set_dn(0, 0xDEADBEEFu);
    cpu->d2 = 0xDEADBEEFu;

    PJIT_TEST_RUN(arm_entry);

    printf("[TEST] unexpectedly returned from JIT'd code -- TRAP #15 exit didn't fire\n");
    printf("FAIL\n");
    qemu_exit();
    return 0;
}
