/// @file smoke_branch_exec.c
/// @brief TST.L D0 then a not-taken BEQ.S, executed for real: checks that
///        TST sets flags and that the branch falls through to the next slot.
#include <stdint.h>
#include "pjit.h"
#include "pjit_compat.h"

extern void qemu_exit(void);
extern void emit_opcode_table(void);

static uint8_t ram_pool[12 * 1024 * 1024] __attribute__((aligned(4096)));

// MOVEQ #-1,D0 ; TST.L D0 ; BEQ.S SKIP ; MOVEQ #7,D0 ; SKIP: TRAP #15
//   word0: 0x70FF  MOVEQ #-1,D0
//   word1: 0x4A80  TST.L D0
//   word2: 0x6702  BEQ.S +2 (target = addr(word2)+2+2 = addr(word4)) -- NOT taken (Z=0)
//   word3: 0x7007  MOVEQ #7,D0   -- must run, D0 ends up 7
//   word4: 0x4E4F  TRAP #15     -- test-only stop, see file header
static uint16_t m68k_program[5] __attribute__((aligned(4)));

static void write_be16(uint16_t *addr, uint16_t value)
{
    uint8_t *b = (uint8_t *)addr;
    b[0] = (uint8_t)(value >> 8);
    b[1] = (uint8_t)(value & 0xFF);
}

/// @brief TRAP #15 handler, a strong override of the platform's weak
///        default (platform/qemu_virt/semihost_exit.c). Never returns.
void pjit_test_exit(uint32_t d0)
{
    printf("[TEST] D0 = 0x%08X (want 0x00000007 -- BEQ.S must NOT have taken, MOVEQ #7,D0 must have run)\n", d0);
    printf(d0 == 7u ? "PASS\n" : "FAIL\n");
    qemu_exit();
}

int main(void)
{
    printf("=== PJIT smoke test: TST/BEQ.S not-taken branch -- real trampoline execution ===\n");

    pjit_cache_init((uint32_t)(ram_pool + sizeof(ram_pool)));
    emit_opcode_table();

    write_be16(&m68k_program[0], 0x70FF); // MOVEQ #-1,D0
    write_be16(&m68k_program[1], 0x4A80); // TST.L D0
    write_be16(&m68k_program[2], 0x6702); // BEQ.S +2 (NOT taken, Z=0)
    write_be16(&m68k_program[3], 0x7007); // MOVEQ #7,D0 (must run)
    write_be16(&m68k_program[4], 0x4E4F); // TRAP #15 (test-only stop)

    uint32_t m68k_pc = (uint32_t)m68k_program;

    pjit_set_dn(0, 0xDEADBEEFu); // poisoned -- MOVEQ #-1,D0 must overwrite it
    uint32_t arm_entry = pjit_cache_dispatch(m68k_pc);

    printf("[TEST] pjit_cache_dispatch() returned 0x%08X\n", arm_entry);

    if (!arm_entry) {
        printf("FAIL\n");
        qemu_exit();
        return 0;
    }

    uint32_t *words = (uint32_t *)arm_entry;
    printf("[TEST] slot0 (MOVEQ #-1): 0x%08X 0x%08X\n", words[0], words[1]);
    printf("[TEST] slot1 (TST.L D0):  0x%08X 0x%08X\n", words[2], words[3]);
    printf("[TEST] slot2 (BEQ.S +2):  0x%08X 0x%08X\n", words[4], words[5]);
    printf("[TEST] slot3 (MOVEQ #7):  0x%08X 0x%08X\n", words[6], words[7]);
    printf("[TEST] slot4 (TRAP #15):  0x%08X 0x%08X\n", words[8], words[9]);

    // No BX-LR patch, no expectation of returning here -- see the file
    // header. This BLX just starts real 68K execution; TRAP #15 at the
    // end is what stops it, via pjit_test_exit() above.
    PJIT_TEST_RUN(arm_entry);

    // Unreachable if pjit_test_exit() correctly halted via qemu_exit().
    printf("[TEST] unexpectedly returned from JIT'd code -- TRAP #15 exit didn't fire\n");
    printf("FAIL\n");
    qemu_exit();
    return 0;
}
