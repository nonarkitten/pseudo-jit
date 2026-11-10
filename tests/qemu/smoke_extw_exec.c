/// @file smoke_extw_exec.c
/// @brief EXT.W: sign-extends byte -> word, leaves bits 31:16 alone, and
///        sets N/Z from the 16-bit result.
#include <stdint.h>
#include "pjit.h"
#include "pjit_compat.h"

extern void qemu_exit(void);
extern void emit_opcode_table(void);

static uint8_t ram_pool[12 * 1024 * 1024] __attribute__((aligned(4096)));

// D7 is the error accumulator (0 = all flag checks passed).
//   D0 = 0x12348085 -> 0x1234FF85, N set
//   D1 = 0xABCD1242 -> 0xABCD0042, N clear (bits 15:8 must be cleared)
//   D2 = 0x5555FF00 -> 0x55550000, Z set
//   D3 = 0x00000080 -> 0x0000FF80, N set
static uint16_t m68k_program[14] __attribute__((aligned(4))) ;
static const uint16_t words[14] = {
    0x7E00,                         // MOVEQ #0,D7
    0x4880, 0x6B02, 0x7E01,         // EXT.W D0 ; BMI.S +2 ; MOVEQ #1,D7
    0x4881, 0x6A02, 0x7E02,         // EXT.W D1 ; BPL.S +2 ; MOVEQ #2,D7
    0x4882, 0x6702, 0x7E03,         // EXT.W D2 ; BEQ.S +2 ; MOVEQ #3,D7
    0x4883, 0x6B02, 0x7E04,         // EXT.W D3 ; BMI.S +2 ; MOVEQ #4,D7
};

static void write_be16(uint16_t *addr, uint16_t value)
{
    uint8_t *b = (uint8_t *)addr;
    b[0] = (uint8_t)(value >> 8);
    b[1] = (uint8_t)(value & 0xFF);
}

// Registers live in cpu_state, written under E=BE; byte-swap on read.
void pjit_test_exit(uint32_t unused)
{
    uint32_t d0 = __builtin_bswap32(cpu->d0);
    uint32_t d1 = __builtin_bswap32(cpu->d1);
    uint32_t d2 = __builtin_bswap32(cpu->d2);
    uint32_t d3 = __builtin_bswap32((&cpu->d0)[3]);
    uint32_t d7 = __builtin_bswap32((&cpu->d0)[7]);
    printf("[TEST] D0 = 0x%08X (want 0x1234FF85)\n", d0);
    printf("[TEST] D1 = 0x%08X (want 0xABCD0042)\n", d1);
    printf("[TEST] D2 = 0x%08X (want 0x55550000)\n", d2);
    printf("[TEST] D3 = 0x%08X (want 0x0000FF80)\n", d3);
    printf("[TEST] D7 = 0x%08X (want 0, nonzero = which flag check failed)\n", d7);
    int ok = d0 == 0x1234FF85u && d1 == 0xABCD0042u && d2 == 0x55550000u &&
             d3 == 0x0000FF80u && d7 == 0u;
    printf(ok ? "PASS\n" : "FAIL\n");
    qemu_exit();
}

int main(void)
{
    printf("=== PJIT smoke test: EXT.W -- value, upper half preserved, N/Z ===\n");

    pjit_cache_init((uint32_t)(ram_pool + sizeof(ram_pool)));
    emit_opcode_table();

    for (int i = 0; i < 13; i++) write_be16(&m68k_program[i], words[i]);
    write_be16(&m68k_program[13], 0x4E4F); // TRAP #15

    uint32_t arm_entry = pjit_cache_dispatch((uint32_t)m68k_program);
    printf("[TEST] pjit_cache_dispatch() returned 0x%08X\n", arm_entry);
    if (!arm_entry) {
        printf("FAIL\n");
        qemu_exit();
        return 0;
    }

    // Registers set after the last C call that could disturb them.
    pjit_set_dn(0, 0x12348085u);
    pjit_set_dn(1, 0xABCD1242u);
    pjit_set_dn(2, 0x5555FF00u);
    pjit_set_dn(3, 0x00000080u);
    pjit_set_dn(7, 0xDEADBEEFu);

    PJIT_TEST_RUN(arm_entry);

    printf("[TEST] unexpectedly returned from JIT'd code\n");
    printf("FAIL\n");
    qemu_exit();
    return 0;
}
