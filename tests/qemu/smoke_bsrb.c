/// @file smoke_bsrb.c
/// @brief BSR.B (a one-halfword call) is a linked call in the slot cache: the slot loads the return address, pushes it and `bl`s straight to the target
///        (`movw ; movt ; str ; bl target`). Three calls to a subroutine that itself calls another with BSR.B and returns with RTS; the result, the stack pointer
///        and that the slots hold the linked form are checked, cold and warm. The same program is also built with BSR.W, JSR (A2) and JSR d16(A2).
#include <stdint.h>
#include "pjit.h"

extern void qemu_exit(void);
extern void emit_opcode_table(void);

static uint8_t ram_pool[12 * 1024 * 1024] __attribute__((aligned(4096)));
static uint16_t program[32] __attribute__((aligned(512)));
static uint8_t guest_stack[1024] __attribute__((aligned(8)));
static int failures;
static void check(const char *what, uint32_t got, uint32_t want)
{
    if (got != want) { printf("[TEST] %-50s got 0x%08X want 0x%08X FAIL\n", what, got, want); failures++; }
}
static void put(int i, uint16_t w) { uint8_t *b = (uint8_t *)&program[i]; b[0] = w >> 8; b[1] = (uint8_t)w; }
static const uint32_t *slot(int hw) { return (const uint32_t *)((uint32_t)cpu->cache_data + ((((uint32_t)program + 2 * hw) & 0x7FFFFu) << 3)); }

/// the same program with BSR.B (1 halfword per call) or BSR.W (2): main at 0, sub at 16, sub2 at 24 (halfword indexes)
static int ncalls = 3;
static int build(int word)
{
    int i = 0;
    put(i++, 0x7000);                                       // MOVEQ #0,D0
    for (int c = 0; c < ncalls; c++) {
        if (word == 2) put(i++, 0x4E92);                                            // JSR (A2)
        else if (word == 3) { put(i, 0x4EAA); put(i + 1, 8); i += 2; }              // JSR 8(A2)
        else if (word) { put(i, 0x6100); put(i + 1, (16 - (i + 1)) * 2); i += 2; }       // BSR.W: displacement from the extension word
        else { put(i, 0x6100 | ((16 - (i + 1)) * 2)); i++; }                        // BSR.B: from the end of the opcode
    }
    put(i++, 0x4E4F);                                       // TRAP #15
    int end = i;
    put(16, 0x5280);                                        // sub:  ADDQ.L #1,D0
    if (word == 1) { put(17, 0x6100); put(18, (24 - 18) * 2); put(19, 0x4E75); }         // BSR.W sub2 ; RTS
    else { put(17, 0x6100 | ((24 - 18) * 2)); put(18, 0x4E75); }                    // BSR.B sub2 ; RTS
    put(24, 0x5480);                                        // sub2: ADDQ.L #2,D0
    put(25, 0x4E75);                                        //       RTS
    return end;
}

int main(void)
{
    printf("=== PJIT smoke test: linked BSR.B ===\n");
    pjit_cache_init((uint32_t)(ram_pool + sizeof(ram_pool)));
    cpu->trap_exit_mask = 0x8000;
    emit_opcode_table();
    for (int word = 0; word < 4; word++) {
        for (unsigned j = 0; j < sizeof(program) / 2; j++) put(j, 0x4E71);      // NOPs: the translator reads on past an RTS, so unused halfwords must be whole instructions
        int end = build(word);
        for (int round = 0; round < 3; round++) {                  // cold, then warm
            for (int j = 0; j < 8; j++) { pjit_set_dn(j, 0x11110000u + j); pjit_set_an(j, 0xA0000000u + j); }
            pjit_set_an(2, (uint32_t)program + 32 - (word == 3 ? 8 : 0));
            uint32_t sp = (uint32_t)(guest_stack + sizeof(guest_stack));
            pjit_set_an(7, sp);
            pjit_set_sr(0x2700);
            if (round == 0) pjit_cache_flush();
            pjit_run(program);
            check(word == 3 ? "JSR 8(A2): D0 after three calls (3 x (1 + 2))" : word == 2 ? "JSR (A2): D0 after three calls (3 x (1 + 2))" : word ? "BSR.W: D0 after three calls (3 x (1 + 2))" : "BSR.B: D0 after three calls (3 x (1 + 2))", pjit_get_dn(0), 3 * ncalls);
            check("A7 balanced", pjit_get_an(7), sp);
            check("cpu->pc just past the TRAP", (uint32_t)cpu->pc, (uint32_t)&program[end]);
        }
        const uint32_t *q = slot(1);
        printf("[INFO] %d: first call slot: %08X %08X %08X %08X\n", word, (unsigned)q[0], (unsigned)q[1], (unsigned)q[2], (unsigned)q[3]);
        if (!word && !(q[2] == 0xE52D0004u && (q[3] >> 24) == 0xEB)) { printf("[TEST] the BSR.B slot is not the linked form (push ; bl target) FAIL\n"); failures++; }
        if (word >= 2) {                                    // JSR (An): `movw ; movt ; bl pjit_jsr_ind` ends the span
            const uint32_t *e = slot(word == 2 ? 1 : 2);
            if (!((e[2] >> 20) == 0xE34 && (e[3] >> 24) == 0xEB)) { printf("[TEST] the JSR (An) span does not end in movt ; bl FAIL\n"); failures++; }
        }
    }
    printf(failures ? "FAIL\n" : "PASS\n");
    qemu_exit();
    return failures != 0;
}
