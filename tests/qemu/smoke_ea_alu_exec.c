/// @file smoke_ea_alu_exec.c
/// @brief Table-driven sweep of every addressing mode and every ALU
///        operation at all three widths, plus DBcc and MUL, run as one
///        batch inside a single QEMU boot.
/// @details Built twice: as-is it runs each case through the JIT cache; with
///        PJIT_TEST_INTERP defined (tests/qemu/smoke_ea_alu_interp.c) it runs the
///        same cases, with the same expected values, through the C-free
///        interpretive runtime (pjit_run).
#include <stdint.h>
#include "pjit.h"

extern void qemu_exit(void);
extern void emit_opcode_table(void);

static uint8_t  ram_pool[12 * 1024 * 1024] __attribute__((aligned(4096)));
// One 32 KB guest program area, aligned to it: the cache translates the 16-byte group around each
// instruction it enters (see the NOP fill below), so a smaller or unaligned array would have it
// reading whatever unrelated BSS follows.
static uint16_t progmem[32768u/2] __attribute__((aligned(32768u)));
static uint32_t data_buf[8]   __attribute__((aligned(4)));
static uint32_t scratch[16]   __attribute__((aligned(4)));   // MOVEM stores land here, zero-initialised

static uint32_t off;      // next free word index in progmem
static uint32_t base;     // word index this case started at
static uint32_t np;       // words appended to the current case
static int      pass, fail;

static void wbe(uint16_t *a, uint16_t v) { ((uint8_t*)a)[0] = v >> 8; ((uint8_t*)a)[1] = v; }
static void w(uint16_t v)   { wbe(&progmem[base + np++], v); }
static void w32(uint32_t v) { w(v >> 16); w(v); }
static void start(void)     { base = off; np = 0; off += 48; }

// Cases are defined first and executed afterward, never interleaved:
// the first dispatch translates the whole page, so any program written
// after that point would never be translated at all.
static struct { const char *name; uint32_t base, np, want; } cases[300];
static int ncases;

static void def(const char *name, uint32_t want)
{
    w(0x4E4F);                  // each case ends with TRAP #15, which pjit_run() returns on
    cases[ncases].name = name; cases[ncases].base = base;
    cases[ncases].np   = np;   cases[ncases].want = want;
    ncases++;
}

static uint8_t guest_stack[2048] __attribute__((aligned(8)));


static void exec_all(void)
{
    for (int i = 0; i < ncases; i++) {
        printf("[RUN] %s\n", cases[i].name);
        for (int r = 0; r < 8; r++) { pjit_set_dn(r, 0); pjit_set_an(r, 0); }
        pjit_set_dn(0, 0xDEADBEEFu);
        pjit_set_an(7, (uint32_t)(guest_stack + sizeof(guest_stack)));
        pjit_set_sr(0x2700);
        pjit_run(&progmem[cases[i].base]);
        {
            uint32_t d0 = pjit_get_dn(0);
            if (d0 == cases[i].want) pass++;
            else { printf("[FAIL] %-14s D0=0x%08X want 0x%08X\n", cases[i].name, d0, cases[i].want); fail++; }
        }
    }
}

// MOVEA.L #imm,An -- how every memory-EA case below gets its base pointer.
static void movea(uint8_t an, uint32_t v) { w(0x207C | (an << 9)); w32(v); }

int main(void)
{
    printf("=== PJIT smoke test: addressing modes, DBcc, ALU widths ===\n");

    pjit_cache_init((uint32_t)(ram_pool + sizeof(ram_pool)));
#ifdef PJIT_TEST_NO_DR
    cpu->dr_threshold = 0xFFFFFFFFu;
#endif
    emit_opcode_table();

    // NOP-fill: the fill reads on past the end of a case's program (to the end of the 16-byte
    // group), so the slack between cases has to be whole instructions.
    for (uint32_t i = 0; i < sizeof(progmem)/2; i++) wbe(&progmem[i], 0x4E71);

    // data_buf is read by JIT'd code running ambient E=BE, but written
    // here in E=LE -- store the bytes explicitly rather than assigning
    // uint32_t and hoping the significance order matches.
    uint8_t *db = (uint8_t *)data_buf;
    for (int i = 0; i < 8; i++) {
        uint32_t v = 0x11223340u + i;
        db[i*4+0] = v >> 24; db[i*4+1] = v >> 16; db[i*4+2] = v >> 8; db[i*4+3] = v;
    }
    uint32_t buf  = (uint32_t)data_buf;
    uint32_t sbuf = (uint32_t)scratch;

    // ---- addressing modes (MOVE.L <ea>,D0) ----
    start(); w(0x203C); w32(0xCAFEF00Du);                 def("#imm32",        0xCAFEF00Du);
    start(); w(0x7255); w(0x2001);                        def("Dn",            0x55u);
    start(); movea(0, buf); w(0x2008);                    def("An",            buf);
    start(); movea(0, buf); w(0x2010);                    def("(An)",          0x11223340u);
    start(); movea(0, buf); w(0x2018);                    def("(An)+",         0x11223340u);
    start(); movea(0, buf + 8); w(0x2020);                def("-(An)",         0x11223341u);
    start(); movea(0, buf); w(0x2028); w(4);              def("d16(An)",       0x11223341u);
    start(); movea(0, buf); w(0x7204); w(0x2030); w(0x1000);
                                                          def("d8(An,Dn.W)",   0x11223341u);
    start(); w(0x2039); w32(buf + 4);                     def("abs.L",         0x11223341u);

    // ---- ALU widths (D1 op D0), upper bits must survive .B/.W ----
    #define ALU(nm, op, a, b, want) \
        start(); w(0x203C); w32(a); w(0x223C); w32(b); w(op); def(nm, want);

    ALU("ADD.B", 0xD001, 0x11111180u, 0x00000090u, 0x11111110u);
    ALU("ADD.W", 0xD041, 0x1111FFFFu, 0x00000002u, 0x11110001u);
    ALU("ADD.L", 0xD081, 0x11111111u, 0x00000002u, 0x11111113u);
    ALU("SUB.B", 0x9001, 0x11111105u, 0x00000003u, 0x11111102u);
    ALU("SUB.W", 0x9041, 0x11110005u, 0x00000003u, 0x11110002u);
    ALU("SUB.L", 0x9081, 0x11111105u, 0x00000003u, 0x11111102u);
    ALU("AND.B", 0xC001, 0x111111FFu, 0x0000000Fu, 0x1111110Fu);
    ALU("AND.W", 0xC041, 0x1111FFFFu, 0x000000F0u, 0x111100F0u);
    ALU("AND.L", 0xC081, 0xFFFFFFFFu, 0x0F0F0F0Fu, 0x0F0F0F0Fu);
    ALU("OR.B",  0x8001, 0x11111100u, 0x0000000Fu, 0x1111110Fu);
    ALU("OR.W",  0x8041, 0x11110000u, 0x0000F00Fu, 0x1111F00Fu);
    ALU("OR.L",  0x8081, 0x00000000u, 0x0F0F0F0Fu, 0x0F0F0F0Fu);
    ALU("EOR.B", 0xB300, 0x111111FFu, 0x0000000Fu, 0x111111F0u);
    ALU("EOR.W", 0xB340, 0x1111FFFFu, 0x0000F00Fu, 0x11110FF0u);
    ALU("EOR.L", 0xB380, 0xFFFFFFFFu, 0x0F0F0F0Fu, 0xF0F0F0F0u);

    // CMP must not modify its destination at any width.
    ALU("CMP.B", 0xB001, 0x11111105u, 0x00000003u, 0x11111105u);
    ALU("CMP.W", 0xB041, 0x11110005u, 0x00000003u, 0x11110005u);
    ALU("CMP.L", 0xB081, 0x11111105u, 0x00000003u, 0x11111105u);

    // ---- unary ----
    start(); w(0x203C); w32(0x0000000Fu); w(0x4400);       def("NEG.B",  0x000000F1u);
    start(); w(0x203C); w32(0x0000000Fu); w(0x4440);       def("NEG.W",  0x0000FFF1u);
    start(); w(0x203C); w32(0x0000000Fu); w(0x4480);       def("NEG.L",  0xFFFFFFF1u);
    start(); w(0x203C); w32(0x0F0F0F0Fu); w(0x4680);       def("NOT.L",  0xF0F0F0F0u);
    start(); w(0x203C); w32(0x11111111u); w(0x4280);       def("CLR.L",  0x00000000u);
    start(); w(0x203C); w32(0x12345678u); w(0x4840);       def("SWAP",   0x56781234u);

    // ---- shifts (static count, Dn) ----
    start(); w(0x203C); w32(0x00000001u); w(0xE788);       def("LSL.L #3",  0x00000008u);
    start(); w(0x203C); w32(0x00000080u); w(0xE688);       def("LSR.L #3",  0x00000010u);
    start(); w(0x203C); w32(0x80000000u); w(0xE680);       def("ASR.L #3",  0xF0000000u);

    // ---- DBcc ----
    // MOVEQ #2,D1 ; MOVEQ #0,D0 ; loop: ADDQ.L #1,D0 ; DBF D1,loop
    // DBF decrements D1.W and branches while it isn't -1, so D1 = 2 runs
    // the body three times. The displacement is relative to the extension
    // word's own address, and loop sits 4 bytes before it.
    start();
    w(0x7202); w(0x7000); w(0x5280); w(0x51C9); w(0xFFFC);
    def("DBF loop x3", 3u);

    // ---- DIVU/DIVS + MUL edge cases ----
    // 68K DIVU/DIVS put (remainder << 16) | quotient in Dn, set N/Z from
    // the quotient alone, and always clear C. V is set -- and Dn left
    // untouched -- when the quotient will not fit in 16 bits. MULU/MULS
    // can never overflow 32 bits, so V is always clear there.
    //
    // DIVU.W Dn,Dm = 0x80C0 | (dst<<9) | src; DIVS.W = 0x81C0 | ...
    #define DIV(nm, op, dividend, divisor, want) \
        start(); w(0x203C); w32(dividend); w(0x223C); w32(divisor); w(op); def(nm, want);

    DIV("DIVU 100/7",    0x80C1, 100u, 7u, (2u << 16) | 14u);   // 14 rem 2
    DIV("DIVU exact",    0x80C1, 100u, 5u, (0u << 16) | 20u);   // no remainder
    DIV("DIVU max quot", 0x80C1, 0xFFFEu, 1u, 0xFFFEu);         // 65534, still fits
    DIV("DIVS -100/7",   0x81C1, (uint32_t)-100, 7u,
        ((uint32_t)(-2) << 16) | (uint32_t)(-14 & 0xFFFF));     // trunc toward 0, rem follows dividend
    DIV("DIVS 100/-7",   0x81C1, 100u, (uint32_t)-7,
        (2u << 16) | (uint32_t)(-14 & 0xFFFF));                 // rem follows dividend (positive)
    DIV("DIVS -32768",   0x81C1, (uint32_t)-32768, 1u,
        (0u << 16) | 0x8000u);                                  // the negative-limit quotient

    // Overflow: quotient needs more than 16 bits, so Dn must be left
    // exactly as it was. Checking the *value* is the strongest available
    // signal that the destination really was untouched.
    DIV("DIVU ovf",      0x80C1, 0x10000u, 1u, 0x10000u);       // 65536 doesn't fit
    DIV("DIVS ovf",      0x81C1, 0x00080000u, 1u, 0x00080000u); // 524288 doesn't fit

    // MULU/MULS: V must end up clear even when poisoned beforehand, and
    // the full 32-bit product must survive.
    start(); w(0x203C); w32(0xFFFFu); w(0x223C); w32(0xFFFFu); w(0xC0C1);
    def("MULU 65535^2", 0xFFFE0001u);
    start(); w(0x203C); w32((uint32_t)-2); w(0x223C); w32(3u); w(0xC1C1);
    def("MULS -2*3",    (uint32_t)-6);

    // ---- Sign extension of 16-bit extension words and .B/.W immediates ----
    start(); movea(0, buf + 8); w(0x2028); w(0xFFFC);                    def("-4(An)",         0x11223341u);
    start(); w(0x7203); w(0x7000); w(0x5280); w(0x5381); w(0x6600); w(0xFFFA);
                                                                         def("BNE.W backward", 3u);
    #define IMM_F(nm, pre, op, imm, bcc, want) \
        start(); pre; w(0x7001); w(op); w(imm); w(bcc); w(0x7000); def(nm, want);
    IMM_F("CMPI.B #$80", (w(0x243C), w32(0x80u)),   0x0C02, 0x0080, 0x6702, 1);   // BEQ: -128 == -128
    IMM_F("CMPI.W #$8000", (w(0x243C), w32(0x8000u)), 0x0C42, 0x8000, 0x6702, 1);

    // ---- MOVEM. Mask bits: D0-D7 = 0-7, A0-A7 = 8-15 (reversed for -(An)).
    // Results are read back into D0 with ordinary MOVEs.
    #define MOVEL(n, v)  do { w(0x203C | ((n) << 9)); w32(v); } while (0)
    #define LOAD_D123()  do { MOVEL(1, 0x11111111u); MOVEL(2, 0x22222222u); MOVEL(3, 0x33333333u); } while (0)
    start(); LOAD_D123(); movea(0, sbuf);      w(0x48D0); w(0x000E); w(0x2028); w(4);
                                                                        def("MOVEM.L (An) r2m",   0x22222222u);
    start(); movea(0, buf);                    w(0x4CD0); w(0x000E); w(0x2002);
                                                                        def("MOVEM.L (An) m2r",   0x11223341u);
    start(); LOAD_D123(); movea(0, sbuf + 16); w(0x48E0); w(0x7000); w(0x2008);
                                                                        def("MOVEM.L -(An) An",   sbuf + 4);
    start(); LOAD_D123(); movea(0, sbuf + 16); w(0x48E0); w(0x7000); w(0x2028); w(4);
                                                                        def("MOVEM.L -(An) order", 0x22222222u);
    start(); movea(0, buf);                    w(0x4CD8); w(0x000E); w(0x2008);
                                                                        def("MOVEM.L (An)+ An",   buf + 12);
    start(); movea(0, buf);                    w(0x4CD8); w(0x000E); w(0x2003);
                                                                        def("MOVEM.L (An)+ D3",   0x11223342u);
    start(); MOVEL(1, 0x1111AAAAu); MOVEL(2, 0x2222BBBBu); movea(0, sbuf);
                                               w(0x4890); w(0x0006); w(0x2010);
                                                                        def("MOVEM.W r2m",        0xAAAABBBBu);
    start(); movea(0, sbuf); w(0x20BC); w32(0x80000001u); w(0x4C90); w(0x0006); w(0x2001);
                                                                        def("MOVEM.W m2r sext D1", 0xFFFF8000u);
    start(); movea(0, sbuf); w(0x20BC); w32(0x80000001u); w(0x4C90); w(0x0006); w(0x2002);
                                                                        def("MOVEM.W m2r sext D2", 0x00000001u);
    start(); movea(1, 0x1234); movea(2, 0x5678); movea(0, sbuf);
             w(0x48D0); w(0x0600); w(0x4CD0); w(0x1800); w(0x200C);     def("MOVEM.L An list",    0x5678u);
    start(); LOAD_D123(); movea(0, sbuf);      w(0x48E8); w(0x0006); w(8); w(0x2028); w(12);
                                                                        def("MOVEM.L d16(An)",    0x22222222u);
    start(); LOAD_D123();                      w(0x48F9); w(0x0006); w32(sbuf); w(0x2039); w32(sbuf + 4);
                                                                        def("MOVEM.L abs.L",      0x22222222u);
    // (d16,PC), memory to register: the data sits in the instruction stream,
    // chosen to be single-word MOVEQs so translating it as code is harmless.
    // A NOP pads the data to a 4-byte boundary: ARM's LDR faults on a
    // word-aligned-only address that 68K would happily read.
    start(); w(0x4CFA); w(0x0006); w(8); w(0x2002); w(0x600A); w(0x4E71);
             w(0x7011); w(0x7022); w(0x7033); w(0x7044);                def("MOVEM.L (d16,PC)",   0x70337044u);

    // MOVEM must leave the CCR alone: Z (from MOVEQ #0,D5) and C (SUBQ.L #1
    // from 0 borrows) each survive every form; D0 = 1 if the branch is taken.
    #define MOVEM_FLAGS(nm, a0, mv, mask) \
        start(); movea(0, a0); w(0x7001); w(0x7A00); w(mv); w(mask); w(0x6702); w(0x7000); def(nm " keeps Z", 1); \
        start(); movea(0, a0); w(0x7001); w(0x7A00); w(0x5385); w(mv); w(mask); w(0x6502); w(0x7000); def(nm " keeps C", 1);
    MOVEM_FLAGS("MOVEM (An) r2m",   sbuf,      0x48D0, 0x000E)
    MOVEM_FLAGS("MOVEM (An) m2r",   buf,       0x4CD0, 0x000E)
    MOVEM_FLAGS("MOVEM -(An)",      sbuf + 16, 0x48E0, 0x7000)
    MOVEM_FLAGS("MOVEM (An)+",      buf,       0x4CD8, 0x000E)

    // ---- EXG ----
    start(); MOVEL(1, 0x11111111u); MOVEL(2, 0x22222222u); w(0xC342); w(0x2001);      def("EXG Dx,Dy",   0x22222222u);
    start(); movea(1, 0x1111); movea(2, 0x2222); w(0xC34A); w(0x2009);                def("EXG Ax,Ay",   0x2222u);
    start(); MOVEL(1, 0x1111u); movea(2, 0x2222); w(0xC38A); w(0x2001);               def("EXG Dx,Ay D", 0x2222u);
    start(); MOVEL(1, 0x1111u); movea(2, 0x2222); w(0xC38A); w(0x200A);               def("EXG Dx,Ay A", 0x1111u);

    // ---- LINK/UNLK on the real stack (A7 is SP). D5 keeps the original A7
    // (MOVE.L A7,Dn is 0x2n0F); A6 = 0x1234 must come back from the frame.
    // Every case must UNLK: A7 is the harness's own stack pointer.
    start(); movea(6, 0x1234); w(0x2A0F); w(0x4E56); w(0xFFF8); w(0x240F);
             w(0x9A82); w(0x4E5E); w(0x2005);                                         def("LINK size",   12u);  // SUB.L D2,D5
    start(); movea(6, 0x1234); w(0x2A0F); w(0x4E56); w(0xFFF8); w(0x4E5E);
             w(0x280F); w(0x9885); w(0x2004);                                         def("UNLK sp",     0u);   // SUB.L D5,D4
    start(); movea(6, 0x1234); w(0x4E56); w(0xFFF8); w(0x4E5E); w(0x200E);           def("UNLK A6",     0x1234u);

    // ---- BSET/BCLR/BCHG/BTST on a register and on memory ----
    start(); w(0x7000); w(0x08C0); w(3);                                              def("BSET #3,D0",  0x08u);
    start(); MOVEL(0, 0xFFu); w(0x0880); w(3);                                        def("BCLR #3,D0",  0xF7u);
    start(); MOVEL(0, 0x01u); w(0x0840); w(0);                                        def("BCHG #0,D0",  0x00u);
    start(); w(0x7205); w(0x7000); w(0x03C0);                                         def("BSET D1,D0",  0x20u);
    start(); w(0x7205); MOVEL(0, 0xFFu); w(0x0380);                                   def("BCLR D1,D0",  0xDFu);
    start(); w(0x7205); MOVEL(0, 0x20u); w(0x0340);                                   def("BCHG D1,D0",  0x00u);
    // Z is the tested bit's value *before* the modify.
    start(); w(0x7C01); w(0x7000); w(0x08C0); w(3); w(0x6702); w(0x7C00); w(0x2006); def("BSET Z (was 0)", 1u);
    start(); w(0x7C01); MOVEL(0, 0x08u); w(0x08C0); w(3); w(0x6602); w(0x7C00); w(0x2006);
                                                                                      def("BSET NZ (was 1)", 1u);
    start(); movea(0, sbuf + 32); w(0x7000); w(0x08D0); w(7); w(0x1010);              def("BSET #7,(An)", 0x80u);
    start(); movea(0, sbuf + 32); w(0x7000); w(0x0890); w(7); w(0x1010);              def("BCLR #7,(An)", 0x00u);

    // ---- ROL/ROR: pure rotates, upper bits of .B/.W untouched ----
    start(); MOVEL(0, 0x80000001u); w(0xE398);                                        def("ROL.L #1",    0x00000003u);
    start(); MOVEL(0, 0x80000001u); w(0xE298);                                        def("ROR.L #1",    0xC0000000u);
    start(); MOVEL(0, 0x11111234u); w(0xE958);                                        def("ROL.W #4",    0x11112341u);
    start(); MOVEL(0, 0x11111281u); w(0xE318);                                        def("ROL.B #1",    0x11111203u);
    start(); MOVEL(0, 0x11111203u); w(0xE218);                                        def("ROR.B #1",    0x11111281u);
    start(); MOVEL(0, 0x80000001u); w(0x7203); w(0xE3B8);                             def("ROL.L D1,D0", 0x0000000Cu);
    start(); MOVEL(0, 0x80000000u); w(0x7C01); w(0xE398); w(0x6502); w(0x7C00); w(0x2006);
                                                                                      def("ROL C out",   1u);
    start(); MOVEL(0, 0x00000001u); w(0x7C01); w(0xE298); w(0x6502); w(0x7C00); w(0x2006);
                                                                                      def("ROR C out",   1u);

    // ---- ANDI/ORI/EORI to CCR, MOVE to CCR (C = 0x01, V = 0x02, Z = 0x04, N = 0x08) ----
    start(); w(0x7001); w(0x7400); w(0x003C); w(0x0001); w(0x6502); w(0x7000);        def("ORI #1,CCR",  1u);   // sets C; BCS
    start(); w(0x7001); w(0x7400); w(0x023C); w(0x00FB); w(0x6602); w(0x7000);        def("ANDI #$FB,CCR", 1u); // clears Z; BNE
    start(); w(0x7001); w(0x7400); w(0x0A3C); w(0x0004); w(0x6602); w(0x7000);        def("EORI #4,CCR", 1u);   // toggles Z off; BNE
    start(); w(0x7001); w(0x7204); w(0x44C1); w(0x6702); w(0x7000);                   def("MOVE D1,CCR", 1u);   // Z = 1; BEQ

    // ---- Shifts and rotates: 1110 ccc d ss i tt rrr ----
    // size 0/1/2 = B/W/L, tt 0/1/2/3 = AS/LS/ROX/RO, i = count in Dn, ccc = count (0 = 8)
    enum { SZ_B = 0, SZ_W = 1, SZ_L = 2, AS = 0, LS = 1, ROX = 2, RO = 3 };
    #define SH(cnt, left, size, ireg, type, reg) \
        (uint16_t)(0xE000 | ((cnt) << 9) | ((left) << 8) | ((size) << 6) | ((ireg) << 5) | ((type) << 3) | (reg))
    #define SHV(nm, init, op, want) start(); MOVEL(0, init); w(op); def(nm, want);
    // D0 = 1 if the Bcc.S after the shift is taken (MOVEQ #1,D6 first: it
    // sets flags, so it must precede the shift).
    #define SHC(nm, init, op, bcc, want) \
        start(); MOVEL(0, init); w(0x7C01); w(op); w(bcc); w(0x7C00); w(0x2006); def(nm, want);
    // X, read back as ADDX.L D3,D3 with D3 = 0; xset preloads it (ORI #$10 / ANDI #$EF,CCR).
    #define XPRE(xset) do { if (xset) { w(0x003C); w(0x0010); } else { w(0x023C); w(0x00EF); } } while (0)
    #define SHX(nm, xset, init, op, want) \
        start(); MOVEL(0, init); XPRE(xset); w(op); w(0x7600); w(0xD783); w(0x2003); def(nm, want);
    #define SHXV(nm, xset, init, op, want) \
        start(); MOVEL(0, init); XPRE(xset); w(op); def(nm, want);

    // Values: the upper bits of .B/.W must survive, and right shifts of a
    // narrow operand must not drag in the sign-extension.
    SHV("LSR.B #1",    0x11111181u, SH(1, 0, SZ_B, 0, LS, 0), 0x11111140u);
    SHV("LSR.W #4",    0x1111F234u, SH(4, 0, SZ_W, 0, LS, 0), 0x11110F23u);
    SHV("LSR.L #4",    0xF0000000u, SH(4, 0, SZ_L, 0, LS, 0), 0x0F000000u);
    SHV("LSL.B #1",    0x11111181u, SH(1, 1, SZ_B, 0, LS, 0), 0x11111102u);
    SHV("LSL.W #4",    0x1111F234u, SH(4, 1, SZ_W, 0, LS, 0), 0x11112340u);
    SHV("LSL.L #8",    0x00000001u, SH(0, 1, SZ_L, 0, LS, 0), 0x00000100u);   // 0 encodes 8
    SHV("ASR.B #1",    0x11111180u, SH(1, 0, SZ_B, 0, AS, 0), 0x111111C0u);
    SHV("ASR.W #4",    0x11118000u, SH(4, 0, SZ_W, 0, AS, 0), 0x1111F800u);
    SHV("ASR.L #4",    0x80000000u, SH(4, 0, SZ_L, 0, AS, 0), 0xF8000000u);
    SHV("ASL.B #1",    0x11111140u, SH(1, 1, SZ_B, 0, AS, 0), 0x11111180u);
    SHV("ASL.L #2",    0x00000003u, SH(2, 1, SZ_L, 0, AS, 0), 0x0000000Cu);
    SHV("ROL.L #1",    0x80000001u, SH(1, 1, SZ_L, 0, RO, 0), 0x00000003u);
    SHV("ROR.L #1",    0x80000001u, SH(1, 0, SZ_L, 0, RO, 0), 0xC0000000u);
    SHV("ROL.W #4",    0x11111234u, SH(4, 1, SZ_W, 0, RO, 0), 0x11112341u);
    SHV("ROL.B #1",    0x11111281u, SH(1, 1, SZ_B, 0, RO, 0), 0x11111203u);
    SHV("ROR.B #1",    0x11111203u, SH(1, 0, SZ_B, 0, RO, 0), 0x11111281u);
    // Count in a register (mod 64): 4 shifts; 9 shifts a byte clean out.
    start(); MOVEL(0, 0x00000001u); w(0x7204); w(SH(1, 1, SZ_L, 1, LS, 0));         def("LSL.L D1,D0",  0x00000010u);
    start(); MOVEL(0, 0x111111FFu); w(0x7209); w(SH(1, 1, SZ_B, 1, LS, 0));         def("LSL.B D1=9",   0x11111100u);
    start(); MOVEL(0, 0x80000001u); w(0x7203); w(SH(1, 1, SZ_L, 1, RO, 0));         def("ROL.L D1,D0",  0x0000000Cu);

    // C, V (BCS/BVS/BVC/BCC/BEQ/BMI: 0x65/0x69/0x68/0x64/0x67/0x6B, displacement 2)
    SHC("LSL.B C out",  0x11111180u, SH(1, 1, SZ_B, 0, LS, 0), 0x6502, 1);   // bit 7 out
    SHC("LSL.W C out",  0x11118000u, SH(1, 1, SZ_W, 0, LS, 0), 0x6502, 1);   // bit 15 out
    SHC("LSR.B C out",  0x11111101u, SH(1, 0, SZ_B, 0, LS, 0), 0x6502, 1);
    SHC("ASR.B C out",  0x11111101u, SH(1, 0, SZ_B, 0, AS, 0), 0x6502, 1);
    SHC("LSL.B Z",      0x11111180u, SH(1, 1, SZ_B, 0, LS, 0), 0x6702, 1);   // byte becomes 0
    SHC("LSL.B N",      0x11111140u, SH(1, 1, SZ_B, 0, LS, 0), 0x6B02, 1);   // bit 7 set
    SHC("ASL.B V set",  0x11111140u, SH(1, 1, SZ_B, 0, AS, 0), 0x6902, 1);   // sign changed
    SHC("ASL.B V clr",  0x11111120u, SH(1, 1, SZ_B, 0, AS, 0), 0x6802, 1);
    SHC("ASL.L V set",  0x40000000u, SH(1, 1, SZ_L, 0, AS, 0), 0x6902, 1);
    SHC("ROL.B C out",  0x11111180u, SH(1, 1, SZ_B, 0, RO, 0), 0x6502, 1);   // wrapped bit
    SHC("ROR.B C out",  0x11111101u, SH(1, 0, SZ_B, 0, RO, 0), 0x6502, 1);
    SHC("ROL.W N",      0x11114000u, SH(1, 1, SZ_W, 0, RO, 0), 0x6B02, 1);

    // X: set from the last bit shifted out, except ROL/ROR which leave it alone.
    SHX("LSL.B X=out",   0, 0x11111180u, SH(1, 1, SZ_B, 0, LS, 0), 1u);
    SHX("LSR.L X clear", 1, 0x00000002u, SH(1, 0, SZ_L, 0, LS, 0), 0u);
    SHX("ASR.W X=out",   0, 0x11110001u, SH(1, 0, SZ_W, 0, AS, 0), 1u);
    SHX("ROL.L keeps X", 1, 0x00000000u, SH(1, 1, SZ_L, 0, RO, 0), 1u);
    SHX("ROR.L keeps X", 0, 0x00000001u, SH(1, 0, SZ_L, 0, RO, 0), 0u);   // C=1 out, X stays 0

    // ROXL/ROXR rotate through X.
    SHXV("ROXL.B X=1",   1, 0x11111180u, SH(1, 1, SZ_B, 0, ROX, 0), 0x11111101u);
    SHXV("ROXL.B X=0",   0, 0x11111180u, SH(1, 1, SZ_B, 0, ROX, 0), 0x11111100u);
    SHXV("ROXR.B X=1",   1, 0x11111101u, SH(1, 0, SZ_B, 0, ROX, 0), 0x11111180u);
    SHXV("ROXL.W #4",    0, 0x11111234u, SH(4, 1, SZ_W, 0, ROX, 0), 0x11112340u);
    SHXV("ROXL.L X=1",   1, 0x80000000u, SH(1, 1, SZ_L, 0, ROX, 0), 0x00000001u);
    SHXV("ROXR.L X=1",   1, 0x00000001u, SH(1, 0, SZ_L, 0, ROX, 0), 0x80000000u);
    SHX("ROXL.B X out",  0, 0x11111180u, SH(1, 1, SZ_B, 0, ROX, 0), 1u);
    SHX("ROXR.L X out",  0, 0x00000001u, SH(1, 0, SZ_L, 0, ROX, 0), 1u);
    SHC("ROXL.B C=X",    0x11111180u, SH(1, 1, SZ_B, 0, ROX, 0), 0x6502, 1);

    // Memory forms: one bit, word-sized (E0C0 AS, E2C0 LS, E4C0 ROX, E6C0 RO; bit 8 = left).
    #define MEMSH(nm, init, op, want) \
        start(); movea(0, sbuf + 48); w(0x30BC); w((uint16_t)(init)); w(op); w(0x7000); w(0x3010); def(nm, want);
    MEMSH("LSR.W (An)",  0x8001, 0xE2D0, 0x4000u);
    MEMSH("LSL.W (An)",  0x8001, 0xE3D0, 0x0002u);
    MEMSH("ASR.W (An)",  0x8000, 0xE0D0, 0xC000u);
    MEMSH("ASL.W (An)",  0x4000, 0xE1D0, 0x8000u);
    MEMSH("ROL.W (An)",  0x8001, 0xE7D0, 0x0003u);
    MEMSH("ROR.W (An)",  0x0001, 0xE6D0, 0x8000u);
    start(); movea(0, sbuf + 48); w(0x30BC); w(0x0001); XPRE(1); w(0xE4D0); w(0x7000); w(0x3010);
                                                                                     def("ROXR.W (An) X=1", 0x8000u);

    // ---- MOVEP: alternate bytes, most significant first. A0 = sbuf+16, which
    // nothing else has written; MOVE.B d16(A0),D0 = 0x1028 d16.
    start(); movea(0, sbuf + 16); MOVEL(1, 0x11223344u); w(0x03C8); w(0); w(0x7000); w(0x1028); w(2);
                                                                                     def("MOVEP.L byte +2",   0x22u);
    start(); movea(0, sbuf + 16); w(0x7000); w(0x1028); w(1);                       def("MOVEP.L gap byte",  0x00u);
    start(); movea(0, sbuf + 16); MOVEL(2, 0xDEADBEEFu); w(0x0548); w(0); w(0x2002);
                                                                                     def("MOVEP.L m2r",       0x11223344u);
    start(); movea(0, sbuf + 16); MOVEL(1, 0x1122AABBu); w(0x0388); w(8); MOVEL(2, 0x99990000u);
             w(0x0508); w(8); w(0x2002);                                             def("MOVEP.W d16 rt",    0x9999AABBu);
    start(); movea(0, sbuf + 32); MOVEL(1, 0x1122CCDDu); w(0x0388); w(0xFFF8); MOVEL(2, 0x88880000u);
             w(0x0508); w(0xFFF8); w(0x2002);                                        def("MOVEP.W -d16 rt",   0x8888CCDDu);

    // ---- Flags of .B/.W arithmetic describe the operand's own width. Bcc.S
    // opcodes: BCS 6502, BCC 6402, BEQ 6702, BNE 6602, BMI 6B02, BPL 6A02,
    // BVS 6902, BVC 6802. SHC2 is SHC for an op with one extension word.
    #define SHC2(nm, init, op, ext, bcc, want) \
        start(); MOVEL(0, init); w(0x7C01); w(op); w(ext); w(bcc); w(0x7C00); w(0x2006); def(nm, want);
    SHC("ADDQ.B N (7F+1)",  0x1111117Fu, 0x5200, 0x6B02, 1);     // 0x80 is negative as a byte
    SHC("ADDQ.B V (7F+1)",  0x1111117Fu, 0x5200, 0x6902, 1);
    SHC("ADDQ.B C (FF+1)",  0x111111FFu, 0x5200, 0x6502, 1);
    SHC("ADDQ.B Z (FF+1)",  0x111111FFu, 0x5200, 0x6702, 1);
    SHC("ADDQ.W N (7FFF+1)", 0x11117FFFu, 0x5240, 0x6B02, 1);
    SHC("ADDQ.W V (7FFF+1)", 0x11117FFFu, 0x5240, 0x6902, 1);
    SHC("ADDQ.W C (FFFF+1)", 0x1111FFFFu, 0x5240, 0x6502, 1);
    SHC2("ADDI.B C (80+80)", 0x11111180u, 0x0600, 0x0080, 0x6502, 1);     // carries out of the byte only
    SHC2("ADDI.B V (80+80)", 0x11111180u, 0x0600, 0x0080, 0x6902, 1);
    SHC("SUBQ.B V (80-1)",  0x11111180u, 0x5300, 0x6902, 1);     // wraps to 0x7F
    SHC("SUBQ.B N (80-1)",  0x11111180u, 0x5300, 0x6A02, 1);     // ...which is positive
    SHC("SUBQ.B C (00-1)",  0x11111100u, 0x5300, 0x6502, 1);
    SHC("SUBQ.W C (0-1)",   0x11110000u, 0x5340, 0x6502, 1);
    SHC("NEG.B V (80)",     0x11111180u, 0x4400, 0x6902, 1);
    SHC("NEG.B C (80)",     0x11111180u, 0x4400, 0x6502, 1);
    SHC("NEG.W V (8000)",   0x11118000u, 0x4440, 0x6902, 1);
    SHC2("CMPI.B V (80-1)", 0x11111180u, 0x0C00, 0x0001, 0x6902, 1);
    SHC2("CMPI.B C (01-02)", 0x11111101u, 0x0C00, 0x0002, 0x6502, 1);
    SHC2("CMPI.W V (8000-1)", 0x11118000u, 0x0C40, 0x0001, 0x6902, 1);
    // X follows C, and ADDX then carries it on: ADDQ.B carrying out of the byte,
    // then ADDX.B D3,D3 (D3 = 0) must see it.
    SHX("ADDQ.B X (FF+1)",  0, 0x111111FFu, 0x5200, 1u);
    SHX("SUBQ.W X (0-1)",   0, 0x11110000u, 0x5340, 1u);
    // ADDX/SUBX/NEGX: Z is sticky (cleared by a non-zero result, else kept). D3 = 1 first;
    // MOVEQ #0 sets Z, so the ops below start from Z=1 unless noted.
    //   MOVEQ #1,D6 ; MOVEQ #0,D2 ; <preload X> ; ADDX.B/SUBX.B ... ; BEQ/BNE
    #define XZ(nm, xset, a, b, op, bcc, want) \
        start(); MOVEL(0, a); MOVEL(1, b); w(0x7C01); w(0x7400); XPRE(xset); w(op); w(bcc); w(0x7C00); w(0x2006); def(nm, want);
    XZ("ADDX.B Z kept",    0, 0x11111100u, 0x11111100u, 0xD101, 0x6702, 1);   // 0+0+0=0: Z stays 1
    XZ("ADDX.B Z cleared", 0, 0x11111101u, 0x11111100u, 0xD101, 0x6602, 1);   // nonzero: Z cleared
    XZ("ADDX.B cin Z",     1, 0x111111FFu, 0x11111100u, 0xD101, 0x6702, 1);   // FF+0+1 = 0 (carry out): Z kept
    XZ("SUBX.B borrow Z",  1, 0x11111105u, 0x11111104u, 0x9101, 0x6702, 1);   // 5-4-1 = 0: Z kept
    XZ("SUBX.B nonzero",   0, 0x11111105u, 0x11111100u, 0x9101, 0x6602, 1);   // 5-0 = 5: Z cleared
    XZ("ADDX.W cin Z",     1, 0x1111FFFFu, 0x11110000u, 0xD141, 0x6702, 1);
    // Values must still be right: the result is shifted back down.
    SHV("ADDQ.B wrap",      0x111111FFu, 0x5200, 0x11111100u);
    SHV("SUBQ.W wrap",      0x11110000u, 0x5340, 0x1111FFFFu);
    SHV("NEG.B value",      0x11111105u, 0x4400, 0x111111FBu);

    // ---- MOVE-family flags: N/Z from the value, V/C cleared. D0 = 1 if the
    // branch was taken. C is poisoned first (SUBQ.L #1 from 0 borrows).
    start(); w(0x7001); w(0x7400); w(0x6702); w(0x7000);                 def("MOVEQ #0 -> Z",   1);
    start(); w(0x7001); w(0x74FF); w(0x6B02); w(0x7000);                 def("MOVEQ #-1 -> N",  1);
    start(); w(0x7600); w(0x5383); w(0x7001); w(0x6402); w(0x7000);     def("MOVEQ clears C",  1);
    start(); w(0x7001); w(0x7600); w(0x5383); w(0x2403); w(0x6402); w(0x7000);
                                                                         def("MOVE.L clears C", 1);
    start(); w(0x7001); w(0x7600); w(0x5383); w(0x3403); w(0x6402); w(0x7000);
                                                                         def("MOVE.W clears C", 1);
    start(); w(0x7001); w(0x7600); w(0x5383); w(0x1403); w(0x6402); w(0x7000);
                                                                         def("MOVE.B clears C", 1);
    start(); w(0x7001); w(0x143C); w(0x0080); w(0x6B02); w(0x7000);     def("MOVE.B #$80 -> N", 1);
    // Bcc.W: forward (displacement measured from the extension word).
    start(); w(0x7001); w(0x7400); w(0x6700); w(0x0004); w(0x7000);     def("BEQ.W forward",   1);
    #define BCCW_F(nm, a, b, want) \
        start(); w(0x243C); w32(a); w(0x223C); w32(b); w(0x7001); w(0xB481); \
        w(0x6200); w(0x0004); w(0x7000); def(nm, want);              // BHI.W: two-conditional, stub path
    BCCW_F("BHI.W a>b", 2u, 1u, 1);
    BCCW_F("BHI.W a<b", 1u, 2u, 0);

    // ---- ADDA/SUBA/CMPA: .W sign-extends the source, .L does not; ADDA/SUBA touch no flags ----
    #define AOP(nm, op, a0, d1, want) \
        start(); movea(0, a0); w(0x223C); w32(d1); w(op); w(0x2008); def(nm, want);
    AOP("ADDA.W -1", 0xD0C1, 0x1000u, 0x0000FFFFu, 0x0FFFu);
    AOP("ADDA.W +1", 0xD0C1, 0x1000u, 0xFFFF0001u, 0x1001u);   // upper half ignored
    AOP("ADDA.L",    0xD1C1, 0x1000u, 0x2000u,     0x3000u);
    AOP("SUBA.W -1", 0x90C1, 0x1000u, 0x0000FFFFu, 0x1001u);
    AOP("SUBA.L",    0x91C1, 0x3000u, 0x1000u,     0x2000u);
    start(); movea(0, 0x1000); w(0xD0FC); w(0xFFFF); w(0x2008);         def("ADDA.W #-1",    0x0FFFu);

    // CMPA flag checks: D0 = 1 if the branch is taken, 0 if not. The MOVEQ
    // sits ahead of the compare because MOVEQ itself sets flags.
    #define CMPA_F(nm, op, a0, d1, bcc, want) \
        start(); movea(0, a0); w(0x223C); w32(d1); w(0x7001); w(op); w(bcc); w(0x7000); def(nm, want);
    CMPA_F("CMPA.L eq",      0xB1C1, 5u, 5u,               0x6702, 1);  // BEQ
    CMPA_F("CMPA.L ne",      0xB1C1, 5u, 6u,               0x6702, 0);
    CMPA_F("CMPA.L borrow",  0xB1C1, 1u, 2u,               0x6502, 1);  // BCS
    CMPA_F("CMPA.L noborrw", 0xB1C1, 2u, 1u,               0x6502, 0);
    CMPA_F("CMPA.W sext",    0xB0C1, 0xFFFFFFFFu, 0xFFFFu, 0x6702, 1);  // source sign-extended
    CMPA_F("CMPA.L signed",  0xB1C1, 0x80000000u, 1u,      0x6D02, 1);  // BLT: V set, N clear

    // ---- ADDX/SUBX/NEGX: X comes from a preceding ADD.L D2,D2 (0xD482) --
    // 0x80000000 carries out (X=1), 0 does not (X=0). MOVE.L leaves X alone.
    #define XOP(nm, x, op, a, b, want) \
        start(); w(0x243C); w32((x) ? 0x80000000u : 0u); w(0xD482); \
        w(0x203C); w32(a); w(0x223C); w32(b); w(op); def(nm, want);
    XOP("ADDX.L X=1", 1, 0xD181, 5u, 3u, 9u);
    XOP("ADDX.L X=0", 0, 0xD181, 5u, 3u, 8u);
    XOP("SUBX.L X=1", 1, 0x9181, 5u, 3u, 1u);
    XOP("SUBX.L X=0", 0, 0x9181, 5u, 3u, 2u);
    XOP("ADDX.B wrap", 1, 0xD101, 0x111111FFu, 0u, 0x11111100u);
    XOP("ADDX.W wrap", 1, 0xD141, 0x1111FFFFu, 0u, 0x11110000u);
    XOP("NEGX.L X=1", 1, 0x4080, 5u, 0u, 0xFFFFFFFAu);
    XOP("NEGX.L X=0", 0, 0x4080, 5u, 0u, 0xFFFFFFFBu);
    XOP("NEGX.W X=1", 1, 0x4040, 0x11110005u, 0u, 0x1111FFFAu);

    // The -(An) form of the extend ops is selected by bit 3 (the R/M bit), not bit 2 (the low bit of the source register
    // field); every XOP case above uses source register D1, so this one uses a source register in {4,5,6,7}.
    // ADDX.B -(A4),-(A0): 0x05 + 0x09 + X(0) = 0x0E.
    start();
    w(0x13FC); w(0x0009); w32(buf + 16);   // MOVE.B #$09,buf+16 -- source
    w(0x13FC); w(0x0005); w32(buf + 20);   // MOVE.B #$05,buf+20 -- dest
    movea(4, buf + 17);
    movea(0, buf + 21);
    XPRE(0);
    w(0xD10C);                             // ADDX.B -(A4),-(A0)
    w(0x7000);                             // MOVEQ #0,D0
    w(0x1039); w32(buf + 20);              // MOVE.B buf+20,D0
    def("ADDX.B -(An) Ry>=4", 0x0000000E);

    // ---- CMPM: (Ay)+ is the source, (Ax)+ the destination ----
    start(); movea(0, buf); movea(1, buf);     w(0x7001); w(0xB189); w(0x6702); w(0x7000); def("CMPM.L eq",     1);
    start(); movea(0, buf); movea(1, buf + 4); w(0x7001); w(0xB189); w(0x6702); w(0x7000); def("CMPM.L ne",     0);
    start(); movea(0, buf); movea(1, buf + 4); w(0x7001); w(0xB189); w(0x6502); w(0x7000); def("CMPM.L borrow", 1);
    start(); movea(0, buf); movea(1, buf);     w(0xB189); w(0x2008);                        def("CMPM.L A0+4",   buf + 4);
    start(); movea(0, buf); movea(1, buf);     w(0xB109); w(0x2008);                        def("CMPM.B A0+1",   buf + 1);

    // ---- .B/.W widths of ADDQ/SUBQ/CLR/NOT: upper bits must survive ----
    #define UN(nm, op, a, want) start(); w(0x203C); w32(a); w(op); def(nm, want);
    UN("ADDQ.B wrap", 0x5200, 0x111100FFu, 0x11110000u);
    UN("ADDQ.W wrap", 0x5240, 0x1111FFFFu, 0x11110000u);
    UN("SUBQ.B wrap", 0x5300, 0x11110000u, 0x111100FFu);
    UN("SUBQ.W wrap", 0x5340, 0x11110000u, 0x1111FFFFu);
    UN("ADDQ.L #8",   0x5080, 1u,          9u);              // data field 0 means 8
    UN("CLR.B",       0x4200, 0x11223344u, 0x11223300u);
    UN("CLR.W",       0x4240, 0x11223344u, 0x11220000u);
    UN("NOT.B",       0x4600, 0x11223344u, 0x112233BBu);
    UN("NOT.W",       0x4640, 0x11223344u, 0x1122CCBBu);


    // ---- Flag polarity: 68K C is a *borrow* after CMP/SUB/NEG, and V/C are
    // always cleared by logical ops. D0 = 1 if the branch is taken. C is
    // poisoned to 1 first (SUBQ.L #1 from 0 borrows) where the op must clear it.
    #define CMP_F(nm, op, a, b, bcc, want) \
        start(); w(0x243C); w32(a); w(0x223C); w32(b); w(0x7001); w(op); w(bcc); w(0x7000); def(nm, want);
    CMP_F("CMP.L borrow",   0xB481, 1u, 2u,                  0x6502, 1);   // BCS
    CMP_F("CMP.L noborrow", 0xB481, 2u, 1u,                  0x6502, 0);
    CMP_F("CMP.L equal C",  0xB481, 7u, 7u,                  0x6402, 1);   // BCC
    CMP_F("CMP.B borrow",   0xB401, 0x11110001u, 2u,         0x6502, 1);   // byte borrow, not 32-bit
    CMP_F("CMP.W borrow",   0xB441, 0x00010001u, 2u,         0x6502, 1);
    CMP_F("CMP.L signed",   0xB481, 0x80000000u, 1u,         0x6D02, 1);   // BLT
    CMP_F("CMP.L unsigned", 0xB481, 0x80000000u, 1u,         0x6202, 1);   // BHI
    // HI = !C && !Z, LS = C || Z: three distinct C/Z combinations each.
    CMP_F("BHI  a>b",       0xB481, 2u, 1u,                  0x6202, 1);   // C=0 Z=0
    CMP_F("BHI  a<b",       0xB481, 1u, 2u,                  0x6202, 0);   // C=1 Z=0
    CMP_F("BHI  a==b",      0xB481, 7u, 7u,                  0x6202, 0);   // C=0 Z=1
    CMP_F("BLS  a>b",       0xB481, 2u, 1u,                  0x6302, 0);
    CMP_F("BLS  a<b",       0xB481, 1u, 2u,                  0x6302, 1);
    CMP_F("BLS  a==b",      0xB481, 7u, 7u,                  0x6302, 1);
    // Scc D0 after a CMP: byte is 0xFF if the condition holds, else 0x00.
    #define SCC_F(nm, scc, a, b, want) \
        start(); w(0x243C); w32(a); w(0x223C); w32(b); w(0x7000); w(0xB481); w(scc); def(nm, want);
    SCC_F("SHI a>b",  0x52C0, 2u, 1u, 0xFFu);
    SCC_F("SHI a<b",  0x52C0, 1u, 2u, 0x00u);
    SCC_F("SHI a==b", 0x52C0, 7u, 7u, 0x00u);
    SCC_F("SLS a>b",  0x53C0, 2u, 1u, 0x00u);
    SCC_F("SLS a<b",  0x53C0, 1u, 2u, 0xFFu);
    SCC_F("SLS a==b", 0x53C0, 7u, 7u, 0xFFu);
    // DBcc D3 after a CMP: condition true exits (falls through, D0=1);
    // false decrements D3 and takes the branch (D0=2). MOVEQ #5,D3 sits
    // ahead of the compare since MOVEQ sets flags. The displacement (6)
    // counts from the extension word to the "MOVEQ #2".
    #define DBCC_F(nm, db, a, b, want) \
        start(); w(0x243C); w32(a); w(0x223C); w32(b); w(0x7605); w(0xB481); \
        w(db); w(6); w(0x7001); w(0x6002); w(0x7002); def(nm, want);
    DBCC_F("DBHI a>b",  0x52CB, 2u, 1u, 1);
    DBCC_F("DBHI a<b",  0x52CB, 1u, 2u, 2);
    DBCC_F("DBHI a==b", 0x52CB, 7u, 7u, 2);
    DBCC_F("DBLS a>b",  0x53CB, 2u, 1u, 2);
    DBCC_F("DBLS a<b",  0x53CB, 1u, 2u, 1);
    DBCC_F("DBLS a==b", 0x53CB, 7u, 7u, 1);
    #define NEG_F(nm, op, a, bcc, want) \
        start(); w(0x243C); w32(a); w(0x7001); w(op); w(bcc); w(0x7000); def(nm, want);
    NEG_F("NEG.L C set",    0x4482, 5u,                      0x6502, 1);   // BCS: result nonzero
    NEG_F("NEG.L C clear",  0x4482, 0u,                      0x6402, 1);   // BCC: operand zero
    #define LOGIC_F(nm, op, a, b) \
        start(); w(0x243C); w32(a); w(0x223C); w32(b); w(0x7001); w(0x7600); w(0x5383); \
        w(op); w(0x6402); w(0x7000); def(nm, 1);   // MOVEQ #0,D3 ; SUBQ.L #1,D3 poisons C; BCC taken = C cleared
    LOGIC_F("AND.L clears C", 0xC481, 0xFFu, 0x0Fu);
    LOGIC_F("OR.L  clears C", 0x8481, 0xFFu, 0x0Fu);
    LOGIC_F("EOR.L clears C", 0xB382, 0xFFu, 0x0Fu);   // EOR.L D1,D2

    // ---- TAS: tests the byte (N/Z, V/C clear), then sets bit 7 ----
    UN("TAS D0",      0x4AC0, 0x11223300u, 0x11223380u);
    // Flag checks accumulate a failure code in D1, copied to D0 at the end.
    start(); w(0x7200); w(0x7000); w(0x4AC0);
    w(0x6702); w(0x7201); w(0x6402); w(0x7202); w(0x6802); w(0x7203); w(0x6A02); w(0x7204);
    w(0x2001);                                                          def("TAS D0 =0 flags", 0);
    start(); w(0x7200); w(0x203C); w32(0x80u); w(0x4AC0);
    w(0x6B02); w(0x7201); w(0x6602); w(0x7202); w(0x6402); w(0x7203);
    w(0x2001);                                                          def("TAS D0 <0 flags", 0);
    start(); movea(0, buf); w(0x4AD0); w(0x2010);                       def("TAS (A0)",    0x91223340u);

    // ---- TST: N/Z from the operand, V and C always cleared. C is poisoned
    // to 1 first (SUBQ.L #1 from 0 borrows); b1..b3 are the Bcc.S +2 words
    // for C, Z and N, each skipping a MOVEQ that records which one failed.
    #define TSTC(nm, pre, op, b1, b2, b3) \
        start(); pre; w(0x7200); w(0x7400); w(0x5382); w(op); \
        w(b1); w(0x7201); w(b2); w(0x7202); w(b3); w(0x7203); w(0x2001); def(nm, 0);
    TSTC("TST.L D0",   (w(0x203C), w32(5u)),          0x4A80, 0x6402, 0x6602, 0x6A02);  // BCC BNE BPL
    TSTC("TST.B neg",  (w(0x203C), w32(0x11223380u)), 0x4A00, 0x6402, 0x6602, 0x6B02);  // BCC BNE BMI
    TSTC("TST.W zero", (w(0x203C), w32(0x11220000u)), 0x4A40, 0x6402, 0x6702, 0x6A02);  // BCC BEQ BPL
    TSTC("TST.L (A0)", movea(0, buf + 8),                 0x4A90, 0x6402, 0x6602, 0x6A02);

    start(); movea(0, buf); w(0x4AE8); w(4); w(0x2028); w(4);           def("TAS d16(A0)", 0x91223341u);

    exec_all();

    printf("[TEST] %d passed, %d failed\n", pass, fail);
    printf(fail ? "FAIL\n" : "PASS\n");
    qemu_exit();
    return 0;
}
