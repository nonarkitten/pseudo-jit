/// @file smoke_loop_exec.c
/// @brief A real backward-branch loop (SUBQ.L / BNE.S), run to
///        completion.
#include <stdint.h>
#include "pjit.h"
#include "pjit_compat.h"

extern void qemu_exit(void);
extern void emit_opcode_table(void);

static uint8_t ram_pool[12 * 1024 * 1024] __attribute__((aligned(4096)));

// MOVEQ #3,D0 ; LOOP: SUBQ.L #1,D0 ; BNE.S LOOP ; TRAP #15
//   word0: 0x7003  MOVEQ #3,D0
//   word1: 0x5380  SUBQ.L #1,D0      -- LOOP:
//   word2: 0x66FC  BNE.S -4 (target = addr(word2)+2-4 = addr(word1))
//   word3: 0x4E4F  TRAP #15
static uint16_t m68k_program[4] __attribute__((aligned(4)));

static void write_be16(uint16_t *addr, uint16_t value)
{
    uint8_t *b = (uint8_t *)addr;
    b[0] = (uint8_t)(value >> 8);
    b[1] = (uint8_t)(value & 0xFF);
}

// See smoke_branch_exec.c's header for why TRAP #15 is used instead of a
// patched BX LR to end the test.
void pjit_test_exit(uint32_t d0)
{
    printf("[TEST] D0 = 0x%08X (want 0x00000000 -- loop must run exactly 3 times)\n", d0);
    printf(d0 == 0u ? "PASS\n" : "FAIL\n");
    qemu_exit();
}

int main(void)
{
    printf("=== PJIT smoke test: SUBQ/BNE.S backward-branch loop -- real trampoline execution ===\n");

    pjit_cache_init((uint32_t)(ram_pool + sizeof(ram_pool)));
    emit_opcode_table();

    write_be16(&m68k_program[0], 0x7003); // MOVEQ #3,D0
    write_be16(&m68k_program[1], 0x5380); // LOOP: SUBQ.L #1,D0
    write_be16(&m68k_program[2], 0x66FC); // BNE.S -4 (back to word1)
    write_be16(&m68k_program[3], 0x4E4F); // TRAP #15

    uint32_t m68k_pc = (uint32_t)m68k_program;

    pjit_set_dn(0, 0xDEADBEEFu); // poisoned -- MOVEQ #3,D0 must overwrite it
    uint32_t arm_entry = pjit_cache_dispatch(m68k_pc);

    printf("[TEST] pjit_cache_dispatch() returned 0x%08X\n", arm_entry);

    if (!arm_entry) {
        printf("FAIL\n");
        qemu_exit();
        return 0;
    }

    uint32_t *words = (uint32_t *)arm_entry;
    printf("[TEST] slot0 (MOVEQ #3):    0x%08X 0x%08X\n", words[0], words[1]);
    printf("[TEST] slot1 (SUBQ.L #1):   0x%08X 0x%08X\n", words[2], words[3]);
    printf("[TEST] slot2 (BNE.S -4):    0x%08X 0x%08X\n", words[4], words[5]);
    printf("[TEST] slot3 (TRAP #15):    0x%08X 0x%08X\n", words[6], words[7]);

    PJIT_TEST_RUN(arm_entry);

    // Unreachable if pjit_test_exit() correctly halted via qemu_exit().
    printf("[TEST] unexpectedly returned from JIT'd code -- TRAP #15 exit didn't fire\n");
    printf("FAIL\n");
    qemu_exit();
    return 0;
}
