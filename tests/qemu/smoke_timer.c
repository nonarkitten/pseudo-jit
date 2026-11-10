/// @file smoke_timer.c
/// @brief Checks the ARM generic timer reads used by TRAP #13/#14 are
///        sane and monotonic before any timing test relies on them.
#include <stdint.h>
#include "pjit.h"
#include "pjit_compat.h"

extern void qemu_exit(void);
extern void emit_opcode_table(void);

static uint8_t ram_pool[12 * 1024 * 1024] __attribute__((aligned(4096)));

// word00: 0x7400  MOVEQ #0,D2   -- error accumulator
// word01: 0x4E4D  TRAP #13      -- D0 = CNTFRQ
// word02: 0x4A80  TST.L D0
// word03: 0x6602  BNE.S +2      -- taken if CNTFRQ != 0 (expected)
// word04: 0x5282  ADDQ.L #1,D2  -- ERROR: CNTFRQ was 0
// word05: 0x7200  MOVEQ #0,D1
// word06: 0x4E4E  TRAP #14      -- D0 = CNTPCT read 1
// word07: 0xD280  ADD.L D0,D1   -- D1 = read1 (D1 was 0)
// word08: 0x4E4E  TRAP #14      -- D0 = CNTPCT read 2
// word09: 0x9081  SUB.L D1,D0   -- D0 = read2 - read1
// word10: 0x6402  BCC.S +2      -- taken if no borrow, i.e. read2>=read1 (expected)
// word11: 0x5482  ADDQ.L #2,D2  -- ERROR: apparent non-monotonic/wraparound
// word12: 0x4A80  TST.L D0
// word13: 0x6602  BNE.S +2      -- taken if diff != 0 (expected: real time passed)
// word14: 0x5882  ADDQ.L #4,D2  -- ERROR: diff was exactly 0
// word15: 0x4E4F  TRAP #15
static uint16_t m68k_program[16] __attribute__((aligned(4)));

static void write_be16(uint16_t *addr, uint16_t value)
{
    uint8_t *b = (uint8_t *)addr;
    b[0] = (uint8_t)(value >> 8);
    b[1] = (uint8_t)(value & 0xFF);
}

void pjit_test_exit(uint32_t d0)
{
    uint32_t d2 = __builtin_bswap32(cpu->d2);
    printf("[TEST] D0 (last CNTPCT diff) = 0x%08X, D2 = 0x%08X (want 0x00000000 -- bit0=CNTFRQ zero, bit1=non-monotonic, bit2=zero diff)\n", d0, d2);
    printf(d2 == 0u ? "PASS\n" : "FAIL\n");
    qemu_exit();
}

int main(void)
{
    printf("=== PJIT smoke test: TRAP #13/#14 -- CNTFRQ/CNTPCT via real 68K TRAP round trip ===\n");

    pjit_cache_init((uint32_t)(ram_pool + sizeof(ram_pool)));
    emit_opcode_table();

    write_be16(&m68k_program[0],  0x7400);
    write_be16(&m68k_program[1],  0x4E4D);
    write_be16(&m68k_program[2],  0x4A80);
    write_be16(&m68k_program[3],  0x6602);
    write_be16(&m68k_program[4],  0x5282);
    write_be16(&m68k_program[5],  0x7200);
    write_be16(&m68k_program[6],  0x4E4E);
    write_be16(&m68k_program[7],  0xD280);
    write_be16(&m68k_program[8],  0x4E4E);
    write_be16(&m68k_program[9],  0x9081);
    write_be16(&m68k_program[10], 0x6402);
    write_be16(&m68k_program[11], 0x5482);
    write_be16(&m68k_program[12], 0x4A80);
    write_be16(&m68k_program[13], 0x6602);
    write_be16(&m68k_program[14], 0x5882);
    write_be16(&m68k_program[15], 0x4E4F);

    uint32_t m68k_pc = (uint32_t)m68k_program;
    uint32_t arm_entry = pjit_cache_dispatch(m68k_pc);

    printf("[TEST] pjit_cache_dispatch() returned 0x%08X\n", arm_entry);

    if (!arm_entry) {
        printf("FAIL\n");
        qemu_exit();
        return 0;
    }

    cpu->d2 = 0xDEADBEEFu; // poisoned -- must end 0 (no error sentinel written)

    PJIT_TEST_RUN(arm_entry);

    printf("[TEST] unexpectedly returned from JIT'd code -- TRAP #15 exit didn't fire\n");
    printf("FAIL\n");
    qemu_exit();
    return 0;
}
