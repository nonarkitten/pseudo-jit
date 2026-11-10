/// @file smoke_btst_exec.c
/// @brief BTST Dn,Dn against a set and a clear bit. 68K's Z reflects the
///        tested bit's value *before* modification, not the whole
///        register after it, and BTST must not write back.
#include <stdint.h>
#include "pjit.h"
#include "pjit_compat.h"

extern void qemu_exit(void);
extern void emit_opcode_table(void);

static uint8_t ram_pool[12 * 1024 * 1024] __attribute__((aligned(4096)));

// word0: 0x7400  MOVEQ #0,D2   -- error accumulator, starts clean (poisoning
//                                 cpu->d2 from C alone isn't enough -- it
//                                 has to be reset by a real 68K instruction,
//                                 or a passing run reads back the untouched
//                                 poison instead of 0)
// word1: 0x7202  MOVEQ #2,D1
// word2: 0x0300  BTST D1,D0    -- tests bit 2 of D0 (set) -- Z must be clear
// word3: 0x6602  BNE.S +2      -- taken if Z clear (expected)
// word4: 0x7401  MOVEQ #1,D2   -- ERROR: Z was wrongly set
// word5: 0x7203  MOVEQ #3,D1
// word6: 0x0300  BTST D1,D0    -- tests bit 3 of D0 (clear) -- Z must be set
// word7: 0x6702  BEQ.S +2      -- taken if Z set (expected)
// word8: 0x7402  MOVEQ #2,D2   -- ERROR: Z was wrongly clear
// word9: 0x4E4F  TRAP #15
static uint16_t m68k_program[10] __attribute__((aligned(4)));

static void write_be16(uint16_t *addr, uint16_t value)
{
    uint8_t *b = (uint8_t *)addr;
    b[0] = (uint8_t)(value >> 8);
    b[1] = (uint8_t)(value & 0xFF);
}

/// @brief TRAP #15 handler; D2 needs a byte swap, see smoke_sub_exec.c.
void pjit_test_exit(uint32_t d0)
{
    uint32_t d2 = __builtin_bswap32(cpu->d2);
    printf("[TEST] D0 = 0x%08X (want 0x00000004, BTST must not modify), D2 = 0x%08X (want 0x00000000, nonzero = which case's Z was wrong)\n", d0, d2);
    printf((d0 == 0x00000004u && d2 == 0u) ? "PASS\n" : "FAIL\n");
    qemu_exit();
}

int main(void)
{
    printf("=== PJIT smoke test: BTST -- Z reflects the pre-modification bit, not the whole register ===\n");

    pjit_cache_init((uint32_t)(ram_pool + sizeof(ram_pool)));
    emit_opcode_table();

    write_be16(&m68k_program[0], 0x7400);
    write_be16(&m68k_program[1], 0x7202);
    write_be16(&m68k_program[2], 0x0300);
    write_be16(&m68k_program[3], 0x6602);
    write_be16(&m68k_program[4], 0x7401);
    write_be16(&m68k_program[5], 0x7203);
    write_be16(&m68k_program[6], 0x0300);
    write_be16(&m68k_program[7], 0x6702);
    write_be16(&m68k_program[8], 0x7402);
    write_be16(&m68k_program[9], 0x4E4F);

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
    pjit_set_dn(0, 0x00000004u);
    cpu->d2 = 0xDEADBEEFu; // poisoned -- must end 0 (no error sentinel written)

    PJIT_TEST_RUN(arm_entry);

    printf("[TEST] unexpectedly returned from JIT'd code -- TRAP #15 exit didn't fire\n");
    printf("FAIL\n");
    qemu_exit();
    return 0;
}
