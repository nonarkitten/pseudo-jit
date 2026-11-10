/// @file buffee_fc.c
/// @brief The 68K function-code GPIOs: FC2 (GPIO2_10, LCD_DATA4) must follow the guest's S bit through every way it changes (ANDI to SR, MOVE to SR, exception entry, RTE that returns to user /
///        to supervisor), FC1 (GPIO2_9, LCD_DATA3) is high only during buffee_iack(). The guest reads the GPIO2 DATAOUT register itself at each point (a byte-swapped long: the guest is big-endian); the
///        test also compares the pad level (DATAIN) with what is driven, which shows a contention on a bodged wire. FC0 is tied high by the GreenPAK: check it on the 68K header.
///        make bundle-fc
#include <stdint.h>
#include "pjit.h"
#include "bbb.h"
#include "buffee.h"
#include "mem_be.h"

extern void emit_opcode_table(void);
#define GUEST_CODE   0x08001000u
#define GUEST_STACK  0x08100000u
#define GUEST_DATA   0x08020000u
#define GUEST_VEC    0x08010000u
#define GPIO2        0x481AC000u
#define DATAOUT      (GPIO2 + 0x13C)
#define DATAIN       (GPIO2 + 0x138)

static uint32_t hw;
static void w(uint16_t v) { write_BE_word(GUEST_CODE + hw * 2, v); hw++; }
static void wl(uint32_t v) { w(v >> 16); w(v & 0xFFFF); }
static void sample(void) { w(0x2239); wl(DATAOUT); w(0x22C1); }      /* MOVE.L DATAOUT,D1 ; MOVE.L D1,(A1)+ */
static int fail;
static void check(const char *what, uint32_t got_swapped, int want_fc2, int want_fc1)
{
    uint32_t v = __builtin_bswap32(got_swapped);
    int fc2 = !!(v & (1u << 10)), fc1 = !!(v & (1u << 9));
    int ok = fc2 == want_fc2 && fc1 == want_fc1;
    printf("  %-34s DATAOUT %08X  FC2 %d FC1 %d  (want %d %d)  %s\n", what, (unsigned)v, fc2, fc1, want_fc2, want_fc1, ok ? "ok" : "FAIL");
    if (!ok) fail++;
}
static void pad(const char *what, int want_fc2, int want_fc1)
{
    uint32_t in = *(volatile uint32_t *)DATAIN;
    int fc2 = !!(in & (1u << 10)), fc1 = !!(in & (1u << 9)); int ok = fc2 == want_fc2 && fc1 == want_fc1;
    printf("  pad %-30s DATAIN  FC2 %d FC1 %d  (want %d %d)  %s\n", what, fc2, fc1, want_fc2, want_fc1, ok ? "ok" : "CONTENTION or open wire");
    if (!ok) fail++;
}

int main(void)
{
    printf("=== function codes: FC2 = S bit, FC1 = IACK ===\n");
    pjit_cache_init(BUFFEE_CACHE_TOP);
    cpu->trap_exit_mask = 0x8000;
    emit_opcode_table();
    pjit_irq_enable(0);
    pad("after init (S, no IACK)", 1, 0);
    for (int pass = 0; pass < 2; pass++) {
    printf("-- %s\n", pass ? "translated" : "interpreted");
    cpu->dr_threshold = pass ? 0x1000u : 0xFFFFFFFFu;
    /* the handlers: vector 33 (TRAP #1) samples and returns; vector 34 (TRAP #2) samples, sets S in the stacked SR and returns */
    cpu->vbr = GUEST_VEC;
    for (int v = 0; v < 64; v++) write_BE_long(GUEST_VEC + 4 * v, 0);
    write_BE_long(GUEST_VEC + 4 * 33, GUEST_CODE + 0x400);
    write_BE_long(GUEST_VEC + 4 * 34, GUEST_CODE + 0x440);
    for (int i = 0; i < 16; i++) write_BE_long(GUEST_DATA + 4 * i, 0xDEADBEEFu);
    hw = 0x400 / 2; sample(); w(0x4E73);                                  /* H1: sample ; RTE */
    hw = 0x440 / 2; sample(); w(0x0057); w(0x2000); w(0x4E73);           /* H2: sample ; ORI.W #$2000,(A7) ; RTE (returns in supervisor state) */
    hw = 0;
    w(0x227C); wl(GUEST_DATA);                                            /* MOVEA.L #out,A1 */
    sample();                                                             /* 0: supervisor at the start */
    w(0x027C); w(0xDFFF);                                                 /* ANDI.W #$DFFF,SR : to user */
    sample();                                                             /* 1: user */
    w(0x4E41);                                                            /* TRAP #1 -> handler samples (2: supervisor), RTE back to user */
    sample();                                                             /* 3: user again */
    w(0x4E42);                                                            /* TRAP #2 -> handler samples (4: supervisor), returns with S set */
    sample();                                                             /* 5: supervisor */
    w(0x46FC); w(0x0000);                                                 /* MOVE #0,SR : to user */
    sample();                                                             /* 6: user */
    w(0x4E41);                                                            /* TRAP #1: handler 7: supervisor */
    w(0x4E42);                                                            /* TRAP #2: handler 8: supervisor, returns to supervisor */
    w(0x46FC); w(0x2700);                                                 /* MOVE #$2700,SR (supervisor to supervisor) */
    sample();                                                             /* 9: supervisor (the first, i.e. 7 lines above, is covered by the handler samples) */
    w(0x4E4F);
    for (int i = 0; i < 8; i++) { pjit_set_dn(i, 0); pjit_set_an(i, 0); }
    pjit_set_an(7, GUEST_STACK);
    cpu->ssp = __builtin_bswap32(GUEST_STACK - 0x1000); cpu->usp = __builtin_bswap32(GUEST_STACK);
    pjit_set_sr(0x2700);
    cpu->irq_level = 0; cpu->irq_flag = 0;
    pjit_cache_flush();
    uint32_t r = pjit_run((uint16_t *)GUEST_CODE);
    printf("guest ran, exit 0x%X\n", (unsigned)r);
    static const struct { const char *n; int fc2; } exp[] = {
        { "start (S)", 1 }, { "after ANDI #$DFFF,SR (U)", 0 }, { "TRAP #1 handler (S)", 1 }, { "after RTE to user (U)", 0 }, { "TRAP #2 handler (S)", 1 },
        { "after RTE with S set (S)", 1 }, { "after MOVE #0,SR (U)", 0 }, { "TRAP #1 handler (S)", 1 }, { "TRAP #2 handler (S)", 1 }, { "after MOVE #$2700,SR (S)", 1 } };
    for (unsigned i = 0; i < 10; i++) check(exp[i].n, read_BE_long(GUEST_DATA + 4 * i) ? read_BE_long(GUEST_DATA + 4 * i) : 0, exp[i].fc2, 0);
    pad("after the run (S)", 1, 0);
    }
    /* IACK: FC1 and FC2 high for the cycle, back after */
    buffee_fc_supervisor(0); pad("user (FC2 low)", 0, 0);
    uint8_t vec = buffee_iack(3, 1);
    printf("  IACK level 3 read %02X (the Amiga's ROM mirror at $FFFFF7 or a vector; not used on an Amiga)\n", vec);
    pad("after IACK (S restored)", 1, 0);
    printf("FC value driven (FC2 FC1 FC0, FC0 = the GreenPAK's): %u\n", buffee_fc_get());
    printf("FCTEST %s\n", fail ? "FAIL" : "PASS");
    return fail != 0;
}
