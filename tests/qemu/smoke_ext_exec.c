/// @file smoke_ext_exec.c
/// @brief EXT.L value and flags. Checks N and C, including a second read
///        of the same flags after an intervening redispatch -- which is
///        what catches CPSR being clobbered by the branch trampoline.
#include <stdint.h>
#include "pjit.h"
#include "pjit_compat.h"

extern void qemu_exit(void);
extern void emit_opcode_table(void);

static uint8_t ram_pool[12 * 1024 * 1024] __attribute__((aligned(4096)));

// word0: 0x7400  MOVEQ #0,D2  -- error accumulator, starts clean (poisoning
//                                cpu->d2 from C alone isn't enough -- it
//                                has to be reset by a real 68K instruction,
//                                or a passing run reads back the untouched
//                                poison instead of 0)
// word1: 0x48C0  EXT.L D0     -- D0 = sign_extend32(low word of D0)
// word2: 0x6B02  BMI.S +2     -- taken if N set (expected: result is negative)
// word3: 0x7401  MOVEQ #1,D2  -- ERROR: N flag wrong
// word4: 0x6402  BCC.S +2     -- taken if C clear (expected: EXT always clears C)
// word5: 0x7402  MOVEQ #2,D2  -- ERROR: C flag wrong
// word6: 0x4E4F  TRAP #15
static uint16_t m68k_program[7] __attribute__((aligned(4)));

static void write_be16(uint16_t *addr, uint16_t value)
{
    uint8_t *b = (uint8_t *)addr;
    b[0] = (uint8_t)(value >> 8);
    b[1] = (uint8_t)(value & 0xFF);
}

// D2 lives in cpu_state memory (D1-D7 do; only D0 is held in a register),
// written while JIT'd code ran with ambient E=BE -- this ordinary C
// function reads it back in E=LE, so it needs an explicit byte-swap
// (see tests/qemu/smoke_sub_exec.c's identical note).
void pjit_test_exit(uint32_t d0)
{
    uint32_t d2 = __builtin_bswap32(cpu->d2);
    printf("[TEST] D0 = 0x%08X (want 0xFFFF8000), D2 = 0x%08X (want 0x00000000, nonzero = which flag was wrong)\n", d0, d2);
    printf((d0 == 0xFFFF8000u && d2 == 0u) ? "PASS\n" : "FAIL\n");
    qemu_exit();
}

int main(void)
{
    printf("=== PJIT smoke test: EXT.L -- value (rotate-field bug) and flags (missing CMP) ===\n");

    pjit_cache_init((uint32_t)(ram_pool + sizeof(ram_pool)));
    emit_opcode_table();

    write_be16(&m68k_program[0], 0x7400);
    write_be16(&m68k_program[1], 0x48C0);
    write_be16(&m68k_program[2], 0x6B02);
    write_be16(&m68k_program[3], 0x7401);
    write_be16(&m68k_program[4], 0x6402);
    write_be16(&m68k_program[5], 0x7402);
    write_be16(&m68k_program[6], 0x4E4F);

    uint32_t m68k_pc = (uint32_t)m68k_program;
    uint32_t arm_entry = pjit_cache_dispatch(m68k_pc);

    printf("[TEST] pjit_cache_dispatch() returned 0x%08X\n", arm_entry);

    if (!arm_entry) {
        printf("FAIL\n");
        qemu_exit();
        return 0;
    }

    // Set D0/D2 last -- see the file header's note on why this can't
    // happen before pjit_cache_dispatch() (or any other call).
    pjit_set_dn(0, 0x00128000u);
    cpu->d2 = 0xDEADBEEFu; // poisoned -- must end 0 (no error sentinel written)

    PJIT_TEST_RUN(arm_entry);

    printf("[TEST] unexpectedly returned from JIT'd code -- TRAP #15 exit didn't fire\n");
    printf("FAIL\n");
    qemu_exit();
    return 0;
}
