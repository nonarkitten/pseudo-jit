/// @file smoke_call_exec.c
/// @brief Real execution of BSR (8/16-bit) and JSR ((An), d16(An),
///        abs.L), each followed by RTS -- every call form. A seventh case
///        wraps a JMP abs.L in the same BSR/RTS shape, as a tail call:
///        the wrapper never returns itself, but JMP lands on an RTS that
///        returns to the wrapper's *own* caller, so it fits the same
///        accumulate-a-bit-and-return harness as the other six.
///
/// Identical subroutines each OR a distinct bit (set by the caller in D2,
/// via MOVEQ) into D1, then RTS. Each is reached by a different call
/// form. A correct final D1 proves every call landed, pushed a return
/// address on A7 (the guest's own stack, not a C frame), and RTS popped it correctly.
///
/// The subroutines are laid out *after* main, not before: translation
/// only ever walks forward from a dispatch's own entry point, and
/// translated_up_to is a high-water mark. Anything at a lower address
/// than the first dispatch gets silently claimed as "already translated"
/// while still holding the page's untranslated placeholder -- the same
/// trap this file's own handler-layout comment (smoke_exception_exec.c)
/// already warns about.
#include <stdint.h>
#include "pjit.h"
#include "pjit_compat.h"
#include "mem_be.h"

extern void qemu_exit(void);
extern void emit_opcode_table(void);

static uint8_t  ram_pool[12 * 1024 * 1024] __attribute__((aligned(4096)));
static uint16_t progmem[64] __attribute__((aligned(4)));
static uint32_t sup_stack[256] __attribute__((aligned(8)));

// Word indices: main (28 words), then six 2-word subroutines, then a
// JMP-abs.L trampoline (3 words) and the body it tail-calls into (2).
enum {
    MAIN = 0,
    SUB1 = 28, SUB2 = 30, SUB3 = 32, SUB4 = 34, SUB5 = 36, SUB6 = 38,
    SUB7_JMP = 40, SUB7_BODY = 43,
};

static void wbe(uint16_t *a, uint16_t v) { ((uint8_t*)a)[0] = v >> 8; ((uint8_t*)a)[1] = v; }
static void w32(uint16_t *a, uint32_t v) { wbe(a, v >> 16); wbe(a + 1, v); }

/// @brief OR.L D2,D1 ; RTS -- the body shared by all six subroutines.
static void def_sub(int idx)
{
    wbe(&progmem[idx],     0x8282);  // OR.L D2,D1
    wbe(&progmem[idx + 1], 0x4E75);  // RTS
}

/// @brief BSR.S: 1 word, disp8 relative to the word after the opcode.
static void emit_bsr_s(int at, int target)
{
    int32_t disp = target * 2 - (at * 2 + 2);
    wbe(&progmem[at], 0x6100 | ((uint8_t)disp));
}

/// @brief BSR.W: opcode word (disp byte 0x00) + a 16-bit displacement.
static void emit_bsr_w(int at, int target)
{
    int32_t disp = target * 2 - (at * 2 + 2);
    wbe(&progmem[at], 0x6100);
    wbe(&progmem[at + 1], (uint16_t)disp);
}

int main(void)
{
    printf("=== PJIT smoke test: BSR/JSR/RTS -- real trampoline execution ===\n");

    pjit_cache_init((uint32_t)(ram_pool + sizeof(ram_pool)));
    emit_opcode_table();

    int i = MAIN;
    wbe(&progmem[i++], 0x7200);            // MOVEQ #0,D1
    wbe(&progmem[i++], 0x7401);            // MOVEQ #1,D2
    emit_bsr_s(i, SUB1); i += 1;            // BSR.S sub1              -> D1 |= 1
    wbe(&progmem[i++], 0x7402);            // MOVEQ #2,D2
    emit_bsr_w(i, SUB2); i += 2;            // BSR.W sub2              -> D1 |= 2
    wbe(&progmem[i++], 0x7404);            // MOVEQ #4,D2
    emit_bsr_w(i, SUB3); i += 2;            // BSR.W sub3              -> D1 |= 4
    wbe(&progmem[i++], 0x4E71);            // NOP (keeps the layout constants below)
    wbe(&progmem[i++], 0x7408);            // MOVEQ #8,D2
    wbe(&progmem[i], 0x41F9);              // LEA sub4.L,A0
    w32(&progmem[i + 1], (uint32_t)&progmem[SUB4]); i += 3;
    wbe(&progmem[i++], 0x4E90);            // JSR (A0)                -> D1 |= 8, push_extra=0
    wbe(&progmem[i++], 0x7410);            // MOVEQ #16,D2
    wbe(&progmem[i], 0x4EB9);              // JSR sub5.L (abs.L)
    w32(&progmem[i + 1], (uint32_t)&progmem[SUB5]); i += 3;
                                            // -> D1 |= 0x10, push_extra=4
    wbe(&progmem[i++], 0x7420);            // MOVEQ #32,D2
    wbe(&progmem[i], 0x41F9);              // LEA (sub6-100).L,A0
    w32(&progmem[i + 1], (uint32_t)&progmem[SUB6] - 100); i += 3;
    wbe(&progmem[i], 0x4EA8);              // JSR 100(A0)  -- (d16,An)
    wbe(&progmem[i + 1], 100); i += 2;     // -> D1 |= 0x20, push_extra=2
    wbe(&progmem[i++], 0x7440);            // MOVEQ #0x40,D2
    emit_bsr_s(i, SUB7_JMP); i += 1;        // BSR.S sub7 (JMP wrapper) -> D1 |= 0x40
    wbe(&progmem[i++], 0x4E4F);            // TRAP #15 (test-only stop)

    if (i != SUB1) {
        printf("[FAIL] main is %d words, expected %d (fix the SUB* constants)\nFAIL\n", i, SUB1);
        qemu_exit();
        return 0;
    }

    def_sub(SUB1); def_sub(SUB2); def_sub(SUB3);
    def_sub(SUB4); def_sub(SUB5); def_sub(SUB6);

    // SUB7_JMP: JMP sub7body.L -- a tail call. It never returns itself;
    // sub7body's own RTS returns to whoever BSR'd into SUB7_JMP, exactly
    // as if SUB7_JMP's body had been inlined there directly.
    wbe(&progmem[SUB7_JMP], 0x4EF9);
    w32(&progmem[SUB7_JMP + 1], (uint32_t)&progmem[SUB7_BODY]);
    def_sub(SUB7_BODY);

    uint32_t stk = (uint32_t)&sup_stack[256];
    pjit_set_sr(0x2000);
    PJIT_SET_STACKS(stk);
    pjit_set_dn(1, 0xDEADBEEFu);

    uint32_t arm_entry = pjit_cache_dispatch((uint32_t)&progmem[MAIN]);
    if (!arm_entry) {
        printf("[FAIL] dispatch returned 0\nFAIL\n");
        qemu_exit();
        return 0;
    }

    pjit_set_an(7, stk);
    PJIT_TEST_RUN(arm_entry);

    // Unreachable: pjit_test_exit() below halts via qemu_exit().
    printf("[TEST] unexpectedly returned from JIT'd code\nFAIL\n");
    qemu_exit();
    return 0;
}

/// @brief TRAP #15 handler, a strong override of the platform's weak
///        default (platform/qemu_virt/semihost_exit.c). Never returns.
void pjit_test_exit(uint32_t d0)
{
    (void)d0;
    uint32_t d1 = pjit_get_dn(1);
    printf("[TEST] D1 = 0x%08X (want 0x0000007F -- one bit per BSR/JSR/JMP form that landed)\n", d1);
    printf(d1 == 0x7Fu ? "PASS\n" : "FAIL\n");
    qemu_exit();
}
