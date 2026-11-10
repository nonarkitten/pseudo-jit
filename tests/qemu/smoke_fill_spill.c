/// @file smoke_fill_spill.c
/// @brief The slot cache's fill reads ahead through the 16-byte group around the entry instruction, past an RTS into padding. `0000` decodes as ORI.B #imm,D0, whose
///        immediate is the next word: at the last slot of the group it must not write over the slot of the real instruction in the next group (a routine that starts there with an
///        abs.L operand, entered by JSR). Only the entered instruction may spill into the next group.
///        jsr $1010 ; move.l d0,$10000 ; rts ; (padding) ; $1010: tst.b $30000 ; beq.s L ; moveq #7,d0 ; rts ; L: moveq #9,d0 ; rts      ($30000 non-zero: want 7)
#include <stdint.h>
#include "pjit_libc.h"
#include "pjit.h"
#include "mem_be.h"
#include "cp15.h"

extern void qemu_exit(void);
extern void emit_opcode_table(void);

static uint8_t ram_pool[12 * 1024 * 1024] __attribute__((aligned(4096)));
static uint8_t low_mem[1024 * 1024] __attribute__((aligned(1024 * 1024)));
static uint32_t mmu_pagetable[4096] __attribute__((aligned(16384)));
static void map_low(void)
{
    for (uint32_t i = 0; i < 4096; i++) mmu_pagetable[i] = ((i << 20) & 0xFFF00000u) | 0xC0Eu;
    mmu_pagetable[0] = ((uint32_t)low_mem & 0xFFF00000u) | 0xC0Eu;
    CP15TtbCtlTtb0Config(); CP15Ttb0Set((uint32_t)mmu_pagetable); CP15DomainAccessClientSet(); CP15TlbInvalidate(); CP15MMUEnable();
}
static uint32_t run(uint32_t threshold)
{
    cpu->dr_threshold = threshold;
    cpu->trap_exit_mask = 0xFFFF;
    for (int i = 0; i < 8; i++) { pjit_set_dn(i, 0); pjit_set_an(i, 0); }
    pjit_set_an(7, 0xF7FFC);
    pjit_set_sr(0x2700);
    return pjit_run((uint16_t *)0x1000);
}
static void load(void)
{
    static const uint8_t code[] = { 0x4E,0xB9,0x00,0x00,0x10,0x10, 0x23,0xC0,0x00,0x01,0x00,0x00, 0x4E,0x75 };
    static const uint8_t callee[] = { 0x4A,0x39,0x00,0x03,0x00,0x00, 0x67,0x04, 0x70,0x07, 0x4E,0x75, 0x70,0x09, 0x4E,0x75 };
    memset(low_mem + 0x1000, 0, 0x100);
    memcpy(low_mem + 0x1000, code, sizeof code);
    memcpy(low_mem + 0x1010, callee, sizeof callee);
    low_mem[0x100] = 0x4E; low_mem[0x101] = 0x4F;               // TRAP #15
    low_mem[0xF7FFC + 2] = 0x01;                                // return address $100
    low_mem[0x30000] = 0x56;
    low_mem[0x10003] = 0;
}
int main(void)
{
    printf("=== PJIT smoke test: fill read-ahead must not overwrite the next group ===\n");
    pjit_cache_init((uint32_t)(ram_pool + sizeof(ram_pool)));
    emit_opcode_table();
    map_low();
    int failures = 0;
    for (int mode = 0; mode < 2; mode++) {
        pjit_cache_flush(); load(); run(mode ? 0u : 0xFFFFFFFFu);
        uint32_t r = low_mem[0x10003];
        printf("[TEST] %s: result %u (want 7) %s\n", mode ? "cached" : "interpreted", (unsigned)r, r == 7 ? "ok" : "FAIL");
        if (r != 7) failures++;
    }
    printf(failures ? "FAIL\n" : "PASS\n");
    qemu_exit();
    return failures;
}
