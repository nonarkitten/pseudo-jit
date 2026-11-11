/// @file oracle.c
/// @brief Host-native driver for the golden-oracle harness: runs the same
///        deterministic test cases (golden/cases.h) through
///        golden/mcl68k.c (Ted Fried's portable pure-C 68000
///        interpreter -- an independent implementation, not derived from
///        this project's own PJIT core) and prints final register state
///        in a format golden/pjit_side.c's own output can be diffed
///        against directly. See golden/README.md.
///
/// Compiles and runs as an ordinary host program -- no cross-compiler, no
/// QEMU, nothing freestanding. mcl68k.c reaches memory exclusively
/// through four function pointers (ReadWord/WriteWord/ReadByte/
/// WriteByte), wired here to a plain host buffer; `memory_base` is kept
/// at 0 so the address each callback receives is the raw 68K address
/// directly, sidestepping any question of whether a real (possibly
/// 64-bit) host pointer fits in mcl68k.c's 32-bit `memory_base`.
///
/// mcl68k.c's own main loop (run_mcl68k()) never returns -- it polls a
/// simulated bus interface unit forever. Stopped here by picking a halt
/// address just past each case's program and longjmp-ing out of the
/// ReadWord callback the moment mcl68k.c fetches from it: for this
/// harness's v1 scope (register-direct operands only, no memory-addressed
/// opcodes), every ReadWord call is an instruction/extension-word fetch,
/// so there is no ambiguity with a genuine data read yet -- revisit this
/// the moment a case touches memory.
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

#include "cases.h"

// ---- mcl68k.c's expected external interface ----
extern uint32_t m68k_data_reg[8];
extern uint32_t m68k_address_reg[8];
extern uint32_t m68k_a7_S;
extern uint32_t mc68k_pc;
extern uint16_t mc68k_flags;
extern uint32_t memory_base;

extern void (*WriteWord)(uint32_t address, uint16_t data);
extern uint16_t (*ReadWord)(uint32_t address);
extern void (*WriteByte)(uint32_t address, uint8_t data);
extern uint8_t (*ReadByte)(uint32_t address);

extern int16_t run_mcl68k(void *start);

// Externs mcl68k.c links against for I/O it never reaches in this harness
// (no UART traffic, no interrupts in any v1 test case).
uint16_t GetIPLx(void) { return 0; }
int UART0Ready(void) { return 0; }
uint8_t UART0Read(void) { return 0; }
void UART0Write(uint8_t ch) { (void)ch; }

#define MEM_SIZE   0x00100000u   // 1MB flat 68K address space
#define PROG_BASE  0x00001000u
#define SSP_INIT   0x00080000u

static uint8_t mem[MEM_SIZE];

static jmp_buf halt_jb;
static uint32_t halt_addr;
static int halted_unexpectedly;

static uint8_t  h_ReadByte(uint32_t addr)                  { return mem[addr % MEM_SIZE]; }
static void     h_WriteByte(uint32_t addr, uint8_t data)   { mem[addr % MEM_SIZE] = data; }
static uint16_t h_ReadWord(uint32_t addr) {
    if (addr == halt_addr) longjmp(halt_jb, 1);
    uint32_t a = addr % MEM_SIZE;
    return (uint16_t)((mem[a] << 8) | mem[a + 1]);
}
static void h_WriteWord(uint32_t addr, uint16_t data) {
    uint32_t a = addr % MEM_SIZE;
    mem[a]     = (uint8_t)(data >> 8);
    mem[a + 1] = (uint8_t)data;
}

static void mem_w16(uint32_t addr, uint16_t v) { mem[addr] = (uint8_t)(v >> 8); mem[addr + 1] = (uint8_t)v; }
static void mem_w32(uint32_t addr, uint32_t v) { mem_w16(addr, (uint16_t)(v >> 16)); mem_w16(addr + 2, (uint16_t)v); }

/// @brief Runs one case's program through mcl68k.c, halting right after
///        it (see file header) and reporting final D0/D1/D2 (== SR, via
///        the program's own trailing MOVE SR,D2 -- see cases.h).
static void run_one(uint32_t idx, uint16_t words[8], int nwords,
                     uint32_t *out_d0, uint32_t *out_d1, uint16_t *out_sr)
{
    memset(mem, 0, sizeof(mem));
    mem_w32(0, SSP_INIT);       // reset vector: SSP
    mem_w32(4, PROG_BASE);      // reset vector: PC

    for (int i = 0; i < nwords; i++) mem_w16(PROG_BASE + (uint32_t)i * 2, words[i]);
    halt_addr = PROG_BASE + (uint32_t)nwords * 2;

    memory_base = 0;
    ReadByte = h_ReadByte; WriteByte = h_WriteByte;
    ReadWord = h_ReadWord; WriteWord = h_WriteWord;
    m68k_data_reg[2] = 0;    // MOVE SR,D2 is a word op -- zero first so
                              // the full 32-bit compare is unambiguous.

    halted_unexpectedly = 0;
    if (setjmp(halt_jb) == 0) {
        run_mcl68k((void *)0);
        // run_mcl68k() never returns on its own -- reaching here would
        // itself be a harness bug, not a real-world case.
        halted_unexpectedly = 1;
    }

    *out_d0 = m68k_data_reg[0];
    *out_d1 = m68k_data_reg[1];
    *out_sr = (uint16_t)m68k_data_reg[2];
    (void)idx;
}

int main(int argc, char **argv) {
    uint32_t seed = 0x9E3779B9u;
    uint32_t ncases = 2000;   // matches pjit_side.c's NCASES by default;
                              // pass a smaller argv[2] to compare a
                              // prefix, but not a larger one -- pjit_side
                              // has no matching argv to raise its own cap.
    if (argc > 1) seed = (uint32_t)strtoul(argv[1], NULL, 0);
    if (argc > 2) ncases = (uint32_t)strtoul(argv[2], NULL, 0);

    for (uint32_t i = 0; i < ncases; i++) {
        golden_case_t c;
        golden_make_case(seed, i, &c);
        uint16_t words[8];
        int nwords = golden_build_program(&c, words);

        uint32_t d0, d1;
        uint16_t sr;
        run_one(i, words, nwords, &d0, &d1, &sr);

        if (halted_unexpectedly) {
            printf("case %5u: %-4s.%-2d a=%08X b=%08X HARNESS-ERROR: run_mcl68k returned\n",
                   i, golden_family_names[c.fam], c.size, c.a, c.b);
            continue;
        }
        printf("case %5u: %-4s.%-2d a=%08X b=%08X D0=%08X D1=%08X SR=%04X\n",
               i, golden_family_names[c.fam], c.size, c.a, c.b, d0, d1, sr);
    }
    return 0;
}
