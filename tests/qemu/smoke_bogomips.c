/// @file smoke_bogomips.c
/// @brief Times a 68K delay loop through the JIT and reports BogoMIPS,
///        using TRAP #13/#14 to read the ARM generic timer.
/// @see tests/qemu/smoke_native_bogomips.c for the native-ARM baseline.
#include <stdint.h>
#include "pjit.h"
#include "pjit_compat.h"

extern void qemu_exit(void);
extern void emit_opcode_table(void);
extern uint32_t read_cntfrq(void);

static uint8_t ram_pool[12 * 1024 * 1024] __attribute__((aligned(4096)));

// word00: 0x7201  MOVEQ #1,D1        -- loops = 1
// word01: 0x4E4D  TRAP #13           -- D0 = CNTFRQ
// word02: 0x7600  MOVEQ #0,D3
// word03: 0xD680  ADD.L D0,D3        -- D3 = threshold = CNTFRQ
// LOOP:
// word04: 0xD281  ADD.L D1,D1        -- loops *= 2
// word05: 0x4E4E  TRAP #14           -- D0 = start tick
// word06: 0x7800  MOVEQ #0,D4
// word07: 0xD880  ADD.L D0,D4        -- D4 = start
// word08: 0x7A00  MOVEQ #0,D5
// word09: 0xDA81  ADD.L D1,D5        -- D5 = loops (inner counter copy)
// DELAY:
// word10: 0x5385  SUBQ.L #1,D5
// word11: 0x66FC  BNE.S DELAY        -- disp -4
// word12: 0x4E4E  TRAP #14           -- D0 = end tick
// word13: 0x9084  SUB.L D4,D0        -- D0 = elapsed = end - start
// word14: 0x7C00  MOVEQ #0,D6
// word15: 0xDC80  ADD.L D0,D6        -- D6 = elapsed (scratch copy)
// word16: 0x9C83  SUB.L D3,D6        -- D6 = elapsed - threshold
// word17: 0x65E4  BCS.S LOOP         -- disp -28: taken if elapsed < threshold (borrow)
// word18: 0x7400  MOVEQ #0,D2
// word19: 0xD481  ADD.L D1,D2        -- D2 = final loops
// word20: 0x4E4F  TRAP #15           -- report D0 (elapsed), D2 (loops)
static uint16_t m68k_program[21] __attribute__((aligned(4)));

static void write_be16(uint16_t *addr, uint16_t value)
{
    uint8_t *b = (uint8_t *)addr;
    b[0] = (uint8_t)(value >> 8);
    b[1] = (uint8_t)(value & 0xFF);
}

void pjit_test_exit(uint32_t d0)
{
    uint32_t elapsed_ticks = d0;
    uint32_t loops = __builtin_bswap32(cpu->d2);
    uint32_t cntfrq = read_cntfrq();

    printf("[TEST] loops = %u, elapsed ticks = %u, CNTFRQ = %u\n", loops, elapsed_ticks, cntfrq);

    if (loops == 0 || elapsed_ticks == 0 || cntfrq == 0) {
        printf("FAIL\n");
        qemu_exit();
        return;
    }

    double duration = (double)elapsed_ticks / (double)cntfrq;
    double bogomips = 2.0 * (double)loops / duration / 1.0e6;

    printf("[TEST] duration = %d ms\n", (int)(duration * 1000.0));
    printf("[TEST] PJIT 68K BogoMIPS %0.2f\n", bogomips);

    // Sanity, not precision: duration must genuinely be >= 1 second
    // (the loop's own exit condition) and BogoMIPS must be a sane,
    // finite positive number -- this is a smoke test for the
    // *mechanism* (real 68K code driving its own calibration loop
    // through the JIT, timed via real hardware), not a benchmark
    // result to assert an exact figure against.
    printf((duration >= 1.0 && bogomips > 0.0 && bogomips < 10000.0) ? "PASS\n" : "FAIL\n");
    qemu_exit();
}

int main(void)
{
    printf("=== PJIT smoke test: real 68K bogomips measure() loop ===\n");

    pjit_cache_init((uint32_t)(ram_pool + sizeof(ram_pool)));
#ifdef BOGO_INTERP
    cpu->dr_threshold = 0xFFFFFFFFu;            // interpretive only
#endif
    emit_opcode_table();

    write_be16(&m68k_program[0],  0x7201);
    write_be16(&m68k_program[1],  0x4E4D);
    write_be16(&m68k_program[2],  0x7600);
    write_be16(&m68k_program[3],  0xD680);
    write_be16(&m68k_program[4],  0xD281);
    write_be16(&m68k_program[5],  0x4E4E);
    write_be16(&m68k_program[6],  0x7800);
    write_be16(&m68k_program[7],  0xD880);
    write_be16(&m68k_program[8],  0x7A00);
    write_be16(&m68k_program[9],  0xDA81);
    write_be16(&m68k_program[10], 0x5385);
    write_be16(&m68k_program[11], 0x66FC);
    write_be16(&m68k_program[12], 0x4E4E);
    write_be16(&m68k_program[13], 0x9084);
    write_be16(&m68k_program[14], 0x7C00);
    write_be16(&m68k_program[15], 0xDC80);
    write_be16(&m68k_program[16], 0x9C83);
    write_be16(&m68k_program[17], 0x65E4);
    write_be16(&m68k_program[18], 0x7400);
    write_be16(&m68k_program[19], 0xD481);
    write_be16(&m68k_program[20], 0x4E4F);

    uint32_t m68k_pc = (uint32_t)m68k_program;
    uint32_t arm_entry = pjit_cache_dispatch(m68k_pc);

    printf("[TEST] pjit_cache_dispatch() returned 0x%08X\n", arm_entry);

    if (!arm_entry) {
        printf("FAIL\n");
        qemu_exit();
        return 0;
    }

    cpu->d2 = 0xDEADBEEFu; // poisoned -- must end 0 (D1's real loop count, not garbage)

    PJIT_TEST_RUN(arm_entry);

    printf("[TEST] unexpectedly returned from JIT'd code -- TRAP #15 exit didn't fire\n");
    printf("FAIL\n");
    qemu_exit();
    return 0;
}
