/// @file smoke_bra_flags.c
/// @brief A *taken* Bcc must not disturb the live flags: two conditional
///        branches back to back (`bcc.w +4 ; bne.w +4`, the first taken,
///        then the second on the same Z) are ordinary 68K code, and the
///        second must see the Z the first one tested.
///
/// Program: MOVEQ #1,D0 (Z=0,C=0) ; BCC.W +4 (taken, skips ADDQ) ;
/// ADDQ.L #1,D0 ; BNE.W +4 (must be taken -- Z is still 0) ;
/// MOVEQ #$55,D1 (skipped) ; TRAP #15.
#include <stdint.h>
#include "pjit.h"
#include "pjit_compat.h"

extern void qemu_exit(void);
extern void emit_opcode_table(void);

static uint8_t ram_pool[12 * 1024 * 1024] __attribute__((aligned(4096)));
static uint16_t m68k_program[10] __attribute__((aligned(4)));

static void write_be16(uint16_t *addr, uint16_t value)
{
    uint8_t *b = (uint8_t *)addr;
    b[0] = (uint8_t)(value >> 8);
    b[1] = (uint8_t)(value & 0xFF);
}

void pjit_test_exit(uint32_t d0)
{
    uint32_t d1 = pjit_get_dn(1);
    printf("[TEST] D0=%u (want 1, ADDQ skipped) D1=0x%X (want 0, MOVEQ skipped)\n", d0, d1);
    printf((d0 == 1 && d1 == 0) ? "PASS\n" : "FAIL\n");
    qemu_exit();
}

int main(void)
{
    pjit_cache_init((uint32_t)(ram_pool + sizeof(ram_pool)));
    emit_opcode_table();

    write_be16(&m68k_program[0], 0x7001);   // moveq #1,d0
    write_be16(&m68k_program[1], 0x6400);   // bcc.w +4
    write_be16(&m68k_program[2], 0x0004);
    write_be16(&m68k_program[3], 0x5280);   // addq.l #1,d0 (skipped)
    write_be16(&m68k_program[4], 0x6600);   // bne.w +4
    write_be16(&m68k_program[5], 0x0004);
    write_be16(&m68k_program[6], 0x7255);   // moveq #$55,d1 (skipped)
    write_be16(&m68k_program[7], 0x4E4F);   // trap #15

    PJIT_SET_STACKS((uint32_t)&ram_pool[1024]);
    pjit_set_sr(0x2700);
    uint32_t arm_entry = pjit_cache_dispatch((uint32_t)m68k_program);
    PJIT_TEST_RUN(arm_entry);
    printf("FAIL (returned)\n");
    qemu_exit();
    return 0;
}
