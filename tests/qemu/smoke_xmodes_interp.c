/// @file smoke_xmodes_interp.c
/// @brief Every extension-word class of pjit/runtime.s's interpretive
///        handlers, checked against expected values: sign extension of d16,
///        .W vs .L index registers (D and A, incl. A7), (d16,PC) and
///        (d8,PC,Xn), 32-bit immediates/absolutes, and the (source,
///        destination) combinations -- including the two-index-mode case that
///        has to borrow r3. Each case ends in TRAP #15, so no control flow.
#include <stdint.h>
#include "pjit.h"

extern void qemu_exit(void);
extern void emit_opcode_table(void);

static uint8_t  ram_pool[12 * 1024 * 1024] __attribute__((aligned(4096)));
static uint16_t prog[64] __attribute__((aligned(8)));
static uint8_t  buf[32]  __attribute__((aligned(8)));       // read-only data: v_i = 0x11223340 + i
static uint8_t  rw[32]   __attribute__((aligned(8)));       // scratch, cleared per case
static uint8_t  stack[1024] __attribute__((aligned(8)));

static int np, pass, fail;
static void w(uint16_t v)   { ((uint8_t *)&prog[np])[0] = v >> 8; ((uint8_t *)&prog[np])[1] = (uint8_t)v; np++; }
static void w32(uint32_t v) { w(v >> 16); w((uint16_t)v); }
static void put32(uint8_t *p, uint32_t v) { p[0] = v >> 24; p[1] = v >> 16; p[2] = v >> 8; p[3] = (uint8_t)v; }

typedef struct { uint32_t d[8], a[8]; } regs_t;

static void run(const char *name, regs_t r, uint32_t want)
{
    for (int i = 0; i < 8; i++) { put32(buf + 4*i, 0x11223340u + i); put32(rw + 4*i, 0); }
    printf("[RUN] %s\n", name);
    w(0x4E4F);                                  // TRAP #15
    for (int i = 0; i < 8; i++) { pjit_set_dn(i, r.d[i]); pjit_set_an(i, r.a[i] ? r.a[i] : 0); }
    if (!r.a[7]) pjit_set_an(7, (uint32_t)(stack + sizeof(stack)));
    pjit_set_sr(0x2700);
    pjit_cache_flush();                         // the same addresses hold a different program each case
    pjit_run(prog);
    uint32_t got = pjit_get_dn(0);
    if (got == want) pass++;
    else { printf("[FAIL] %-34s D0=0x%08X want 0x%08X\n", name, got, want); fail++; }
    np = 0;
}
static uint32_t B(int i) { return (uint32_t)buf + i; }
static uint32_t RW(int i) { return (uint32_t)rw + i; }
#define V(i) (0x11223340u + (i))
#define CASE(name, regs, want, body) do { np = 0; body; run(name, regs, want); } while (0)

int main(void)
{
    printf("=== PJIT smoke test: interpretive extension handlers ===\n");
    pjit_cache_init((uint32_t)(ram_pool + sizeof(ram_pool)));
#ifndef PJIT_TEST_DR
    cpu->dr_threshold = 0xFFFFFFFFu;            // interpretive only
#endif
    emit_opcode_table();

    regs_t r;
    #define R0() (r = (regs_t){ {0}, {0} })

    // ---- sources, into D0 --------------------------------------------------
    R0(); r.a[0] = B(0);
    CASE("MOVE.L (d16,A0),D0  d16=8",   r, V(2), (w(0x2028), w(8)));
    R0(); r.a[0] = B(16);
    CASE("MOVE.L (d16,A0),D0  d16=-4",  r, V(3), (w(0x2028), w(0xFFFC)));
    R0(); r.a[7] = B(0);
    CASE("MOVE.L (d16,A7),D0",          r, V(1), (w(0x202F), w(4)));

    R0(); r.a[0] = B(8); r.d[1] = 0x0000FFFCu;
    CASE("(d8,A0,D1.W) word idx = -4",  r, V(1), (w(0x2030), w(0x1000)));
    R0(); r.a[0] = B(0); r.d[1] = 0xFFFF0004u;
    CASE("(d8,A0,D1.W) ignores high",   r, V(1), (w(0x2030), w(0x1000)));
    R0(); r.a[0] = B(8); r.d[1] = 0xFFFFFFFCu;
    CASE("(d8,A0,D1.L) long idx = -4",  r, V(1), (w(0x2030), w(0x1800)));
    R0(); r.a[0] = B(0); r.d[1] = 4;
    CASE("(d8,A0,D1.W) d8=+4",          r, V(2), (w(0x2030), w(0x1004)));
    R0(); r.a[0] = B(16); r.d[1] = 4;
    CASE("(d8,A0,D1.W) d8=-4",          r, V(4), (w(0x2030), w(0x10FC)));   // 16+4-4 = 16 -> v4
    R0(); r.a[0] = B(0); r.a[1] = 8;
    CASE("(d8,A0,A1.L)",                r, V(2), (w(0x2030), w(0x9800)));
    R0(); r.a[0] = 0; r.a[7] = B(8);
    CASE("(d8,A0,A7.L)  A7 as index",   r, V(2), (w(0x2030), w(0xF800)));
    R0(); r.a[0] = B(0); r.d[7] = 12;
    CASE("(d8,A0,D7.L)",                r, V(3), (w(0x2030), w(0x7800)));

    // PC-relative: the data sits in the instruction stream, after an explicit
    // TRAP #15 so it is never executed. The extension word is word 1, so
    // (d16,PC) = &word1 + d16.
    R0();
    CASE("MOVE.W (d16,PC),D0",          r, 0x1234u, (w(0x303A), w(4), w(0x4E4F), w(0x1234)));
    R0();
    CASE("MOVE.W (d16,PC),D0  +6",      r, 0x4321u, (w(0x303A), w(6), w(0x4E4F), w(0x4E71), w(0x4321)));
    R0(); r.d[1] = 2;
    CASE("MOVE.W (d8,PC,D1.W),D0",      r, 0x5678u, (w(0x303B), w(0x1004), w(0x4E4F), w(0x4E71), w(0x5678)));

    R0();
    np = 0; w(0x41FA); w(6); w(0x2008);
    { uint32_t ext = (uint32_t)&prog[1]; run("LEA (d16,PC),A0 -> D0", r, ext + 6); }
    R0(); r.d[1] = 0x20;
    np = 0; w(0x41FB); w(0x1810); w(0x2008);                     // d8 = 0x10, D1.L
    { uint32_t ext = (uint32_t)&prog[1]; run("LEA (d8,PC,D1.L),A0 -> D0", r, ext + 0x10 + 0x20); }

    // ---- 32-bit immediates and absolutes ---------------------------------------
    R0();
    CASE("MOVE.L #imm32,D0",            r, 0xCAFEF00Du, (w(0x203C), w32(0xCAFEF00Du)));
    R0();
    CASE("MOVE.L (abs.L),D0",           r, V(1), (w(0x2039), w32(B(4))));
    R0();
    CASE("MOVE.W #imm16,D0",            r, 0x8001u, (w(0x303C), w(0x8001)));
    R0(); r.d[0] = 0xFFFFFF00u;
    CASE("MOVE.B #imm8,D0 keeps upper", r, 0xFFFFFF7Fu, (w(0x103C), w(0x007F)));

    // ---- destination extension words: write, then read back into D0 ------------
    R0(); r.d[1] = 0xCAFEBABEu; r.a[0] = RW(0);
    CASE("MOVE.L D1,(d16,A0)",          r, 0xCAFEBABEu, (w(0x2141), w(8), w(0x2028), w(8)));
    R0(); r.a[0] = RW(0);
    CASE("MOVE.L #imm32,(d16,A0)",      r, 0x01020304u, (w(0x217C), w32(0x01020304u), w(8), w(0x2028), w(8)));
    R0(); r.a[0] = RW(0); r.a[1] = B(0); r.d[2] = 4;
    CASE("MOVE.L (d16,A1),(d8,A0,D2.W)",     r, V(1), (w(0x21A9), w(4), w(0x2000), w(0x2030), w(0x2000)));
    R0(); r.a[0] = RW(0); r.a[1] = B(0); r.d[1] = 8; r.d[2] = 12;
    CASE("MOVE.L (d8,A1,D1.L),(d8,A0,D2.L)", r, V(2), (w(0x21B1), w(0x1800), w(0x2800), w(0x2030), w(0x2800)));
    R0(); r.a[0] = RW(0); r.a[1] = B(0); r.a[2] = 4; r.d[1] = 8; r.d[2] = 12;
    CASE("MOVE.L (d8,A1,A2.L),(d8,A0,D2.W) A-idx", r, V(1), (w(0x21B1), w(0xA800), w(0x2000), w(0x2030), w(0x2000)));
    R0(); r.a[0] = 0;
    CASE("MOVE.W #imm16,(abs.L)",       r, 0xBEEFu, (w(0x33FC), w(0xBEEF), w32(RW(8)), w(0x3039), w32(RW(8))));
    R0(); r.a[0] = B(0);
    CASE("MOVEA.L #imm32,A0 ; MOVE.L A0,D0", r, 0x12345678u, (w(0x207C), w32(0x12345678u), w(0x2008)));

    // ---- read-modify-write with an extension word on each side ---------------------
    R0(); r.a[0] = RW(0);
    CASE("MOVE #5,(8,A0); ADDI.L #7,(8,A0)", r, 12u, (w(0x217C), w32(5), w(8), w(0x06A8), w32(7), w(8), w(0x2028), w(8)));
    R0(); r.a[0] = RW(0);
    CASE("BSET #3,(d16,A0) ; MOVE.B",   r, 8u, (w(0x08E8), w(3), w(8), w(0x1028), w(8)));

    printf("[TEST] %d passed, %d failed\n", pass, fail);
    printf(fail ? "FAIL\n" : "PASS\n");
    qemu_exit();
    return fail != 0;
}
