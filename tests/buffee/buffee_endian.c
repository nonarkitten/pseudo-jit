/// @file buffee_endian.c
/// @brief Endianness / lane sanity on the board: guest 68K code reads and writes a known byte pattern in chip RAM (the GPMC window, Device memory) and in guest DRAM (0x08200000, cached)
///        with byte, word and long accesses through the common effective-address modes (d16(An), (An)+, -(An), d8(An,Xn), abs.L, ADD.L <ea>,Dn, MOVEA.L <ea>,An) at 2-aligned
///        addresses, a BSR / RTS and a push / pop on a stack that sits at a 2 mod 4 address in each region. Expected values are computed in C from the pattern.
///        make bundle-endian
#include <stdint.h>
#include "pjit.h"
#include "bbb.h"
#include "buffee.h"
#include "mem_be.h"

extern void emit_opcode_table(void);
#define GUEST_CODE   0x08001000u
#define GUEST_STACK  0x08100000u
#define GUEST_DATA   0x08020000u                 /* results the guest leaves (big-endian longs) */
#define CHIP         0x00070000u
#define DDR          0x08200000u

static uint32_t hw;
static void w(uint16_t v) { write_BE_word(GUEST_CODE + hw * 2, v); hw++; }
static void wl(uint32_t v) { w(v >> 16); w(v & 0xFFFF); }

static uint8_t P[32];
static uint32_t expv[400]; static const char *name[400]; static uint32_t nexp;
static void ex(const char *n, uint32_t v) { name[nexp] = n; expv[nexp++] = v; }
static uint32_t pb(uint32_t o) { return P[o]; }
static uint32_t pw(uint32_t o) { return P[o] << 8 | P[o + 1]; }
static uint32_t pl(uint32_t o) { return pw(o) << 16 | pw(o + 2); }

static void store_d1(void) { w(0x22C1); }          /* MOVE.L D1,(A1)+ */

static void build(uint32_t B)
{
    hw = 0; nexp = 0;
    w(0x227C); wl(GUEST_DATA);                      /* MOVEA.L #out,A1 */
    w(0x207C); wl(B);                               /* MOVEA.L #B,A0 */
    for (uint32_t d = 0; d < 8; d++) { w(0x72FF); w(0x1228); w(d); store_d1(); ex("byte d16", 0xFFFFFF00u | pb(d)); }
    for (uint32_t d = 0; d < 8; d += 2) { w(0x72FF); w(0x3228); w(d); store_d1(); ex("word d16", 0xFFFF0000u | pw(d)); }
    for (uint32_t d = 0; d < 8; d += 2) { w(0x72FF); w(0x2228); w(d); store_d1(); ex("long d16", pl(d)); }
    w(0x247C); wl(B + 2);                           /* MOVEA.L #B+2,A2 */
    w(0x72FF); w(0x221A); store_d1(); ex("long (A2)+", pl(2)); w(0x22CA); ex("A2 after (A2)+", B + 6);
    w(0x247C); wl(B + 10);
    w(0x72FF); w(0x2222); store_d1(); ex("long -(A2)", pl(6)); w(0x22CA); ex("A2 after -(A2)", B + 6);
    w(0x7404); w(0x72FF); w(0x2230); w(0x2802); store_d1(); ex("long 2(A0,D2.L)", pl(6));
    w(0x72FF); w(0x2239); wl(B + 10); store_d1(); ex("long abs.L", pl(10));
    w(0x72FF); w(0x2239); wl(B + 12); store_d1(); ex("long abs.L +12", pl(12));
    w(0x2468); w(6); w(0x22CA); ex("MOVEA.L 6(A0),A2", pl(6));
    w(0x2468); w(2); w(0x22CA); ex("MOVEA.L 2(A0),A2", pl(2));
    w(0x223C); wl(0x10000000u); w(0xD2A8); w(6); store_d1(); ex("ADD.L 6(A0),D1", 0x10000000u + pl(6));
    w(0x223C); wl(0x10000000u); w(0xB2A8); w(6); w(0x6700 | 0x04); w(0x7201); w(0x4E71); store_d1();  /* CMP.L 6(A0),D1 ; BEQ.S +4 ; MOVEQ #1,D1 ; NOP ; store: D1 stays $10000000 */
    ex("CMP.L 6(A0),D1 (branch not taken)", 1);
    /* stack at a 2 mod 4 address inside the region: BSR / RTS, push / pop */
    w(0x284F);                                      /* MOVEA.L A7,A4 */
    w(0x2E7C); wl(B + 0x42);                        /* MOVEA.L #B+$42,A7 */
    w(0x2C3C); wl(0x12345678u); w(0x2F06);         /* MOVE.L #$12345678,D6 ; MOVE.L D6,-(A7) */
    w(0x7A00); w(0x6106); w(0x6008); w(0x4E71); w(0x4E71); w(0x7A01); w(0x4E75);   /* MOVEQ #0,D5 ; BSR.B sub ; BRA.B over ; NOP ; NOP ; sub: MOVEQ #1,D5 ; RTS */
    w(0x2E1F);                                      /* MOVE.L (A7)+,D7 */
    w(0x22C5); ex("BSR/RTS ran (D5)", 1); w(0x22C7); ex("pushed long popped (D7)", 0x12345678u); w(0x22CF); ex("A7 after", B + 0x42);
    w(0x2E4C);                                      /* MOVEA.L A4,A7 */
    /* writes into the scratch area B+0x100 */
    w(0x263C); wl(0x01234567u); w(0x267C); wl(B + 0x100);
    w(0x2743); w(2); w(0x3743); w(6); w(0x1743); w(9);
    w(0x267C); wl(B + 0x110); w(0x26C3);            /* MOVEA.L #W+16,A3 ; MOVE.L D3,(A3)+ */
    w(0x267C); wl(B + 0x11A); w(0x2703);            /* MOVEA.L #W+26,A3 ; MOVE.L D3,-(A3) */
    w(0x4E4F);
}

static int run_one(const char *what, uint32_t B)
{
    int bad = 0;
    for (uint32_t i = 0; i < 32; i++) P[i] = (uint8_t)(i * 0x1D + 0x35);
    for (uint32_t i = 0; i < 32; i += 2) write_BE_word(B + i, P[i] << 8 | P[i + 1]);
    for (uint32_t i = 0; i < 32; i += 2) write_BE_word(B + 0x100 + i, 0xEEEE);
    for (uint32_t i = 0; i < 32; i++) { uint8_t g = *(volatile uint8_t *)(B + i); if (g != P[i]) { printf("  C byte read %s+%u: %02X want %02X\n", what, (unsigned)i, g, P[i]); bad++; } }
    for (uint32_t i = 0; i < 32; i += 2) { uint16_t g = read_BE_word(B + i); if (g != (P[i] << 8 | P[i + 1])) { printf("  C word read %s+%u: %04X want %04X\n", what, (unsigned)i, g, P[i] << 8 | P[i + 1]); bad++; } }
    build(B);
    for (int i = 0; i < 8; i++) { pjit_set_dn(i, 0); pjit_set_an(i, 0); }
    pjit_set_an(7, GUEST_STACK);
    cpu->ssp = cpu->usp = __builtin_bswap32(GUEST_STACK);
    pjit_set_sr(0x2700);
    cpu->irq_level = 0; cpu->irq_flag = 0;
    pjit_cache_flush();
    uint32_t r = pjit_run((uint16_t *)GUEST_CODE);
    printf("%s: guest ran, exit 0x%X, %u results\n", what, (unsigned)r, (unsigned)nexp);
    for (uint32_t i = 0; i < nexp; i++) {
        uint32_t got = read_BE_word(GUEST_DATA + 4 * i) << 16 | read_BE_word(GUEST_DATA + 4 * i + 2);
        if (got != expv[i]) { printf("  [%u] %s: got %08X want %08X\n", (unsigned)i, name[i], (unsigned)got, (unsigned)expv[i]); bad++; }
    }
    static const struct { uint32_t off, len; uint32_t v; } wr[] = { { 2, 4, 0x01234567 }, { 6, 2, 0x4567 }, { 9, 1, 0x67 }, { 16, 4, 0x01234567 }, { 22, 4, 0x01234567 } };
    uint8_t E[32]; for (int i = 0; i < 32; i++) E[i] = 0xEE;
    for (unsigned k = 0; k < sizeof wr / sizeof wr[0]; k++) for (uint32_t j = 0; j < wr[k].len; j++) E[wr[k].off + j] = wr[k].v >> (8 * (wr[k].len - 1 - j));
    for (uint32_t i = 0; i < 32; i++) { uint8_t g = *(volatile uint8_t *)(B + 0x100 + i); if (g != E[i]) { printf("  write area +%u: %02X want %02X\n", (unsigned)i, g, E[i]); bad++; } }
    printf("%s: %s (%d mismatches)\n", what, bad ? "FAIL" : "PASS", bad);
    return bad;
}

#define GUEST_VEC 0x08010000u
/* VERTB interrupts with the supervisor stack in chip RAM at A7 = stack: the six-byte frame puts the stacked PC long at 2 mod 4 for stack 4n+2. The handler copies the stacked SR and PC out; the
   interrupted loop must finish with the right count, every stacked PC must lie inside the loop and the stacked SR must be $2000. */
static int irq_phase(const char *what, uint32_t stack)
{
    cpu->vbr = GUEST_VEC;
    for (int v = 0; v < 64; v++) write_BE_long(GUEST_VEC + 4 * v, 0);
    write_BE_long(GUEST_VEC + 4 * 27, GUEST_CODE + 0x400);
    for (int i = 0; i < 8; i++) write_BE_long(GUEST_DATA + 4 * i, 0);
    hw = 0x400 / 2;
    w(0x2F00);                                       /* MOVE.L D0,-(A7) : the handler itself pushes (long at 2-aligned too) */
    w(0x23EF); w(6); wl(GUEST_DATA + 16);            /* MOVE.L 6(A7),out : the stacked PC (the push above moved A7 by 4) */
    w(0x33EF); w(4); wl(GUEST_DATA + 20);            /* MOVE.W 4(A7),out : the stacked SR */
    w(0x33FC); w(0x0020); wl(0xDFF09C);              /* acknowledge VERTB */
    w(0x52B9); wl(GUEST_DATA);                       /* ADDQ.L #1,count */
    w(0x201F);                                       /* MOVE.L (A7)+,D0 */
    w(0x4E73);                                       /* RTE */
    hw = 0;
    w(0x33FC); w(0x7FFF); wl(0xDFF09A); w(0x33FC); w(0x7FFF); wl(0xDFF09C);
    w(0x33FC); w(0xC020); wl(0xDFF09A);
    w(0x46FC); w(0x2000);                            /* MOVE #$2000,SR */
    w(0x243C); wl(20000000);
    uint32_t loop = GUEST_CODE + hw * 2;
    w(0x5382); w(0x66FC);                            /* L: SUBQ.L #1,D2 ; BNE.S L */
    w(0x33FC); w(0x0020); wl(0xDFF09A);
    w(0x4E4F);
    for (int i = 0; i < 8; i++) { pjit_set_dn(i, 0x01010101u * (i + 1) + 0x00700000u); pjit_set_an(i, 0x02020202u * (i + 1) + 0x00004000u); }
    pjit_set_an(7, stack);
    cpu->ssp = cpu->usp = __builtin_bswap32(stack);
    pjit_set_sr(0x2700);
    cpu->irq_level = 0; cpu->irq_flag = 0;
    pjit_cache_flush();
    uint32_t r = pjit_run((uint16_t *)GUEST_CODE);
    uint32_t n = read_BE_long(GUEST_DATA), pc1 = read_BE_long(GUEST_DATA + 12), pc2 = read_BE_long(GUEST_DATA + 16), sr = read_BE_long(GUEST_DATA + 20) >> 16;
    int regs = 0;
    for (int i = 0; i < 8; i++) { if (i != 2 && pjit_get_dn(i) != 0x01010101u * (i + 1) + 0x00700000u) { printf("  D%d = %08X after the interrupts\n", i, (unsigned)pjit_get_dn(i)); regs++; }
                                   if (i != 7 && pjit_get_an(i) != 0x02020202u * (i + 1) + 0x00004000u) { printf("  A%d = %08X after the interrupts\n", i, (unsigned)pjit_get_an(i)); regs++; } }
    int bad = regs || (r != 0x2F) || n == 0 || pc2 < loop || pc2 > loop + 4 || (sr & 0xFFFF) != 0x2000 || pjit_get_an(7) != stack || pjit_get_dn(2) != 0;
    printf("irq %s (A7 %08X): exit 0x%X, %u interrupts, last stacked PC %08X (loop %08X..%08X), SR %04X, D2 %u, A7 after %08X: %s\n", what, (unsigned)stack, (unsigned)r, (unsigned)n, (unsigned)pc2, (unsigned)loop, (unsigned)loop + 4, (unsigned)sr, (unsigned)pjit_get_dn(2), (unsigned)pjit_get_an(7), bad ? "FAIL" : "PASS");
    (void)pc1;
    return bad;
}

int main(void)
{
    printf("=== endian / lane test ===\n");
    pjit_cache_init(BUFFEE_CACHE_TOP);
    cpu->trap_exit_mask = 0x8000;
    emit_opcode_table();
    cpu->dr_threshold = 0xFFFFFFFFu;                       /* the interpreter ... */
    pjit_irq_enable(0);
    *(volatile uint8_t *)0xBFE201 |= 3; *(volatile uint8_t *)0xBFE001 &= 0xFC;          /* ROM overlay off: low memory is chip RAM */
    int bad = 0;
    for (int pass = 0; pass < 2; pass++) {
        cpu->dr_threshold = pass ? 0x1000u : 0xFFFFFFFFu;   /* ... then the slot cache / blocks for the guest code at 0x08001000 */
        printf("-- %s\n", pass ? "translated" : "interpreted");
        bad += run_one("chip RAM", CHIP);
        bad += run_one("guest DRAM", DDR);
    }
    pjit_irq_enable(1);
    for (int pass = 0; pass < 2; pass++) {
        cpu->dr_threshold = pass ? 0x1000u : 0xFFFFFFFFu;
        printf("-- interrupts, %s\n", pass ? "translated" : "interpreted");
        bad += irq_phase("chip, 4n", CHIP + 0x1000);
        bad += irq_phase("chip, 4n+2", CHIP + 0x1002);
        bad += irq_phase("DDR, 4n+2", DDR + 0x1002);
    }
    printf("%s\n", bad ? "FAIL" : "PASS");
    *(volatile uint8_t *)0xBFE001 |= 1;
    return 0;
}
