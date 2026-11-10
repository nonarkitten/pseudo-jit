/// @file buffee_abort.c
/// @brief A data abort in guest code becomes a 68K exception: odd-address word / long accesses to the Amiga window are address errors (vector 3), accesses to unmapped addresses are bus
///        errors (vector 2). The guest installs both vectors; each handler records the group-0 frame (status word, access address, SR, PC) and resumes after the faulting instruction.
///        Run interpreted (dr_threshold = ~0) and through the caches. make bundle-abort
#include <stdint.h>
#include "pjit.h"
#include "bbb.h"
#include "buffee.h"
#include "mem_be.h"

#define GUEST_CODE   0x08001000u
#define GUEST_VEC    0x08010000u
#define GUEST_REC    0x08020000u                 /* the handlers' records: vector, access address, status, SR, PC (5 longs each) */
#define GUEST_STACK  0x08100000u

extern uint32_t buffee_abort_bus, buffee_abort_addr;
extern void emit_opcode_table(void);
static uint32_t hw;
static void w(uint16_t v) { write_BE_word(GUEST_CODE + hw * 2, v); hw++; }
static void wl(uint32_t v) { w(v >> 16); w(v & 0xFFFF); }

static struct { const char *name; uint32_t vec, addr, status; uint32_t at, end; } T[8];
static unsigned nt;
/* a case: MOVEA.L #cont,A5 ; the faulting instruction (ins[0..n-1]) ; cont: */
static void test(const char *name, const uint16_t *ins, unsigned n, uint32_t vec, uint32_t addr, uint32_t status)
{
    w(0x2A7C); uint32_t slot = hw; wl(0);                /* MOVEA.L #cont,A5 (patched below) */
    T[nt].at = GUEST_CODE + hw * 2;
    for (unsigned i = 0; i < n; i++) w(ins[i]);
    T[nt].end = GUEST_CODE + hw * 2;
    uint32_t keep = hw; hw = slot; wl(T[nt].end); hw = keep;
    T[nt].name = name; T[nt].vec = vec; T[nt].addr = addr; T[nt].status = status; nt++;
}

static int run(const char *what, int interp)
{
    nt = 0; hw = 0;
    cpu->vbr = GUEST_VEC;
    for (int v = 0; v < 64; v++) write_BE_long(GUEST_VEC + 4 * v, 0);
    write_BE_long(GUEST_VEC + 4 * 2, GUEST_CODE + 0x400);
    write_BE_long(GUEST_VEC + 4 * 3, GUEST_CODE + 0x500);
    for (int i = 0; i < 64; i++) write_BE_long(GUEST_REC + 4 * i, 0xDEADBEEFu);
    /* handlers */
    hw = 0x400 / 2; w(0x7A02);                             /* bus:  MOVEQ #2,D5 ; BRA.W common (written below) */
    hw = 0x500 / 2; w(0x7A03);                             /* addr: MOVEQ #3,D5 ; falls into common */
    uint32_t common = GUEST_CODE + hw * 2;
    w(0x28C5);                                             /* MOVE.L D5,(A4)+ */
    w(0x28EF); w(2);                                       /* MOVE.L 2(A7),(A4)+ : the access address */
    w(0x3817); w(0x28C4);                                  /* MOVE.W (A7),D4 ; MOVE.L D4,(A4)+ : the status word */
    w(0x382F); w(8); w(0x28C4);                            /* MOVE.W 8(A7),D4 ; MOVE.L D4,(A4)+ : the SR */
    w(0x28EF); w(10);                                      /* MOVE.L 10(A7),(A4)+ : the PC */
    w(0xDEFC); w(14);                                      /* ADDA.W #14,A7 */
    w(0x4ED5);                                             /* JMP (A5) */
    hw = 0x402 / 2; { int32_t d = (int32_t)(common - (GUEST_CODE + 0x404)); w(0x6000); w((uint16_t)d); }   /* BRA.W common */
    hw = 0;
    w(0x287C); wl(GUEST_REC);                              /* MOVEA.L #rec,A4 */
    w(0x7000 | 0x55);                                      /* MOVEQ #$55,D0 */
    { const uint16_t i[] = { 0x33C0, 0x0007, 0x0001 };     test("MOVE.W D0,$70001 (odd word write)", i, 3, 3, 0x70001, 0x0005); }
    { const uint16_t i[] = { 0x2239, 0x0007, 0x0001 };     test("MOVE.L $70001,D1 (odd long read)", i, 3, 3, 0x70001, 0x0015); }
    { const uint16_t i[] = { 0x3239, 0x0007, 0x0003 };     test("MOVE.W $70003,D1 (odd word read)", i, 3, 3, 0x70003, 0x0015); }
    { const uint16_t i[] = { 0x3239, 0xA000, 0x0000 };     test("MOVE.W $A0000000,D1 (unmapped read)", i, 3, 2, 0xA0000000u, 0x0015); }
    { const uint16_t i[] = { 0x33C0, 0xB000, 0x0000 };     test("MOVE.W D0,$B0000000 (unmapped write)", i, 3, 2, 0xB0000000u, 0x0005); }
    { const uint16_t i[] = { 0x3239, 0x0007, 0x0000 };     test("MOVE.W $70000,D1 (no fault)", i, 3, 0, 0, 0); }
    w(0x4E4F);                                             /* TRAP #15 ends the run */

    for (int i = 0; i < 8; i++) { pjit_set_dn(i, 0); pjit_set_an(i, 0); }
    pjit_set_an(7, GUEST_STACK);
    cpu->ssp = cpu->usp = __builtin_bswap32(GUEST_STACK);
    pjit_set_sr(0x2700);
    cpu->irq_level = 0; cpu->irq_flag = 0;
    cpu->dr_threshold = interp ? ~0u : 0x01000000u;
    pjit_cache_flush();
    uint32_t r = pjit_run((uint16_t *)GUEST_CODE);
    int bad = 0, k = 0;
    printf("%s: exit 0x%X, A7 %08X\n", what, (unsigned)r, (unsigned)pjit_get_an(7));
    if (r != 0x2F) bad++;
    if (pjit_get_an(7) != GUEST_STACK) { printf("  A7 not restored\n"); bad++; }
    for (unsigned t = 0; t < nt; t++) {
        if (!T[t].vec) { if (read_BE_long(GUEST_REC + 20 * k) != 0xDEADBEEFu && 0) bad++; continue; }
        uint32_t rec[5];
        for (int j = 0; j < 5; j++) rec[j] = read_BE_long(GUEST_REC + 20 * k + 4 * j);
        k++;
        int ok = rec[0] == T[t].vec && rec[1] == T[t].addr && (rec[2] & 0xFFFF) == T[t].status && (rec[3] & 0xFFFF) == 0x2700 && rec[4] >= T[t].at + 2 && rec[4] <= T[t].end + 2;
        printf("  %-40s vec %u addr %08X status %04X SR %04X PC %08X (insn %08X..%08X): %s\n", T[t].name, (unsigned)rec[0], (unsigned)rec[1], (unsigned)rec[2], (unsigned)rec[3], (unsigned)rec[4], (unsigned)T[t].at, (unsigned)T[t].end, ok ? "ok" : "FAIL");
        bad += !ok;
    }
    if (read_BE_long(GUEST_REC + 20 * k) != 0xDEADBEEFu) { printf("  an extra exception was delivered\n"); bad++; }
    printf("%s: %s\n", what, bad ? "FAIL" : "PASS");
    return bad;
}

int main(void)
{
    printf("=== ABORT -> EXCEPTION ===\n");
    pjit_cache_init(BUFFEE_CACHE_TOP);
    cpu->trap_exit_mask = 0x8000;
    emit_opcode_table();
    pjit_irq_enable(0);
    *(volatile uint8_t *)0xBFE201 |= 3; *(volatile uint8_t *)0xBFE001 &= 0xFC;          /* ROM overlay off: low memory is chip RAM */
    int bad = 0;
    bad += run("interpreter", 1);
    bad += run("cached", 0);
    bad += run("interpreter again", 1);
    printf("abort counters: bus %u, address %u\n", (unsigned)buffee_abort_bus, (unsigned)buffee_abort_addr);
    *(volatile uint8_t *)0xBFE001 |= 1;
    printf("ABORT %s\n", bad ? "FAIL" : "PASS");
    return bad;
}
