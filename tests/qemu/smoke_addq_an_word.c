/// @file smoke_addq_an_word.c
/// @brief ADDQ.W #q,An -- 68K operates on all of An (the size is ignored) and
///        leaves the CCR alone. Also covers PEA (An), as in
///        `pea (A0); ...; addq.w #4,A7`.
#include <stdint.h>
#include "pjit.h"
#include "pjit_compat.h"

extern void qemu_exit(void);
extern void emit_opcode_table(void);

static uint8_t ram_pool[12 * 1024 * 1024] __attribute__((aligned(4096)));

// MOVEA.L #stack,A7 ; MOVEA.L #$345678,A0 ; PEA (A0) ; MOVE.L (A7),D0 ; ADDQ.W #4,A7 ; TRAP #15
static uint16_t m68k_program[12] __attribute__((aligned(4)));
static uint32_t stack_area[64] __attribute__((aligned(16)));

static void write_be16(uint16_t *addr, uint16_t value)
{
    uint8_t *b = (uint8_t *)addr;
    b[0] = (uint8_t)(value >> 8);
    b[1] = (uint8_t)(value & 0xFF);
}

void pjit_test_exit(uint32_t d0)
{
    printf("[TEST] D0=0x%08X (want 0x345678)\n", d0);
    printf(d0 == 0x345678 ? "PASS\n" : "FAIL\n");
    qemu_exit();
}

int main(void)
{
    pjit_cache_init((uint32_t)(ram_pool + sizeof(ram_pool)));
    emit_opcode_table();
    uint32_t sp0 = (uint32_t)&stack_area[32];
    write_be16(&m68k_program[0], 0x2E7C);
    write_be16(&m68k_program[1], sp0 >> 16);
    write_be16(&m68k_program[2], sp0 & 0xFFFF);
    write_be16(&m68k_program[3], 0x207C);
    write_be16(&m68k_program[4], 0x0034);
    write_be16(&m68k_program[5], 0x5678);
    write_be16(&m68k_program[6], 0x4850);
    write_be16(&m68k_program[7], 0x2017);
    write_be16(&m68k_program[8], 0x584F);
    write_be16(&m68k_program[9], 0x4E4F);
    PJIT_SET_STACKS((uint32_t)&stack_area[60]);
    pjit_set_sr(0x2700);
    uint32_t arm_entry = pjit_cache_dispatch((uint32_t)m68k_program);
    PJIT_TEST_RUN(arm_entry);
    printf("FAIL (returned)\n");
    qemu_exit();
    return 0;
}
