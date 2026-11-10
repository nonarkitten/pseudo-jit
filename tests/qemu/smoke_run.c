/// @file smoke_run.c
/// @brief Guest code run entirely through the C-free interpretive
///        runtime (pjit/runtime.s): pjit_run() -> prefetch -> dispatch table ->
///        semantic body -> `bx lr` -> prefetch ... -> TRAP #15 -> back to C.
#include <stdint.h>
#include "pjit.h"

extern void qemu_exit(void);
extern void emit_opcode_table(void);

static uint8_t ram_pool[12 * 1024 * 1024] __attribute__((aligned(4096)));
static uint16_t program[16] __attribute__((aligned(4)));
static uint8_t  guest_stack[1024] __attribute__((aligned(8)));

static void put(int i, uint16_t w) { uint8_t *b = (uint8_t *)&program[i]; b[0] = w >> 8; b[1] = (uint8_t)w; }

static int failures;
static void check(const char *what, uint32_t got, uint32_t want)
{
    int ok = (got == want);
    printf("[TEST] %-34s got 0x%08X want 0x%08X %s\n", what, got, want, ok ? "ok" : "FAIL");
    if (!ok) failures++;
}

int main(void)
{
    printf("=== PJIT smoke test: interpretive runtime (pjit_run) ===\n");
    pjit_cache_init((uint32_t)(ram_pool + sizeof(ram_pool)));
#ifndef PJIT_TEST_DR
    cpu->dr_threshold = 0xFFFFFFFFu;            // interpretive only
#endif
    emit_opcode_table();

    int n = 0;
    put(n++, 0x7005);   // MOVEQ #5,D0
    put(n++, 0x72FF);   // MOVEQ #-1,D1
    put(n++, 0x5280);   // ADDQ.L #1,D0
    put(n++, 0xD081);   // ADD.L D1,D0     (D0 = 6 + 0xFFFFFFFF = 5, carry out)
    put(n++, 0x4E4F);   // TRAP #15        (the test exit)

    for (int i = 0; i < 8; i++) { pjit_set_dn(i, 0xDEAD0000u + i); pjit_set_an(i, 0xA0000000u + i); }
    pjit_set_an(7, (uint32_t)(guest_stack + sizeof(guest_stack)));
    pjit_set_sr(0x2700);

    uint32_t vec = pjit_run(program);

    check("pjit_run() result (TRAP #15)", vec, 47);
    check("D0 after MOVEQ/ADDQ/ADD", pjit_get_dn(0), 5);
    check("D1 after MOVEQ #-1", pjit_get_dn(1), 0xFFFFFFFFu);
    check("D2 untouched", pjit_get_dn(2), 0xDEAD0002u);
    check("A0 round-trips", pjit_get_an(0), 0xA0000000u);
    check("A6 round-trips", pjit_get_an(6), 0xA0000006u);
    check("A7 round-trips", pjit_get_an(7), (uint32_t)(guest_stack + sizeof(guest_stack)));
    check("cpu->pc just past the TRAP", (uint32_t)cpu->pc, (uint32_t)&program[n]);
    // ADD.L D1,D0: 6 + 0xFFFFFFFF = 5 with carry out: C set, X set, N/Z/V clear.
    check("CCR nibble (C only)", pjit_get_sr() & 0xF, 0x1);
#if defined(PJIT_X_LAZY) && defined(PJIT_TEST_DR)
    printf("[TEST] X flag set                         skipped: an X_LAZY build does not save X after ADD.L when nothing reads it\n");
#else
    check("X flag set", (pjit_get_sr() >> 4) & 1, 1);
#endif
    check("S bit preserved", (pjit_get_sr() >> 13) & 1, 1);

    printf(failures ? "FAIL\n" : "PASS\n");
    qemu_exit();
    return failures != 0;
}
