/// @file pjit_side.c
/// @brief ARM32/QEMU driver for the golden-oracle harness: runs the same
///        deterministic test cases (cases.h) through the real PJIT core
///        and prints final register state in the same format
///        golden/oracle.c does, so the two can be diffed directly. See
///        golden/README.md. Not part of tests/ or `make test` on
///        purpose -- this is an either-or check against a golden
///        reference, not a smoke test with its own hand-picked expected
///        values, and it is a superset of what several of those already
///        cover.
#include <stdint.h>
#include "pjit.h"

#include "cases.h"

extern void qemu_exit(void);
extern void emit_opcode_table(void);

static uint8_t  ram_pool[12 * 1024 * 1024] __attribute__((aligned(4096)));
static uint16_t progmem[32768u/2] __attribute__((aligned(32768u)));

static void wbe(uint16_t *a, uint16_t v) { ((uint8_t*)a)[0] = v >> 8; ((uint8_t*)a)[1] = v; }

#define NCASES 2000         // WORDS_PER_CASE * NCASES must stay within
                            // progmem's 32768u words.
#define WORDS_PER_CASE 16   // 8 used, generous headroom -- see smoke_bcd_exec.c's
                            // and smoke_movep_exec.c's notes on why an
                            // underestimate here corrupts the *next* case
                            // silently rather than failing loudly.

uint32_t saved_sp;

static void run_case(uint32_t e, uint32_t np, uint32_t stk)
{
    uint8_t *stop = (uint8_t *)(e + np * 2);        // TRAP #15 after the case ends pjit_run()
    stop[0] = 0x4E; stop[1] = 0x4F;
    pjit_set_an(7, stk);
    pjit_cache_flush();
    (void)pjit_run((uint16_t *)e);
}

int main(void)
{
    uint32_t seed = 0x9E3779B9u;

    printf("=== PJIT golden-oracle side (seed=%08X, %u cases) ===\n", seed, (unsigned)NCASES);

    pjit_cache_init((uint32_t)(ram_pool + sizeof(ram_pool)));
    emit_opcode_table();

    for (uint32_t i = 0; i < sizeof(progmem)/2; i++) wbe(&progmem[i], 0x4E71); // NOP fill

    static uint32_t stk[64] __attribute__((aligned(8)));
    uint32_t stk_top = (uint32_t)&stk[64];

    // Every case's bytes are written into progmem before the first case runs: the cache
    // translates by guest address, so bytes written after a run has reached them would be stale.
    static int nwords_of[NCASES];
    for (uint32_t i = 0; i < NCASES; i++) {
        golden_case_t c;
        golden_make_case(seed, i, &c);
        uint16_t words[8];
        int nwords = golden_build_program(&c, words);
        nwords_of[i] = nwords;
        uint32_t base = i * WORDS_PER_CASE;
        for (int w = 0; w < nwords; w++) wbe(&progmem[base + (uint32_t)w], words[w]);
    }

    for (uint32_t i = 0; i < NCASES; i++) {
        golden_case_t c;
        golden_make_case(seed, i, &c);
        int nwords = nwords_of[i];
        uint32_t base = i * WORDS_PER_CASE;

        uint32_t e = (uint32_t)&progmem[base];

        // 0x2700: supervisor, no trace, interrupt mask 7, CCR clear --
        // matches golden/mcl68k.c's own Reset_routine() default
        // exactly (S=1, mask=0b111), which golden/oracle.c has no hook
        // to override. None of this harness's v1 opcodes touch the mask
        // bits, but they'd still show up as a spurious mismatch on every
        // single case if the two sides' starting SR merely *differed*
        // here without either side's opcode actually being at fault.
        pjit_set_sr(0x2700);
        cpu->ssp = cpu->usp = __builtin_bswap32(stk_top);
        pjit_set_dn(0, 0xDEADBEEFu);
        pjit_set_dn(1, 0xDEADBEEFu);
        // Zeroed, not poisoned: MOVE SR,D2 (see cases.h) is a word op, so
        // only D2's low 16 bits change -- zero first so the full 32-bit
        // readback is an unambiguous 16-bit SR value.
        pjit_set_dn(2, 0);

        run_case(e, (uint32_t)nwords, stk_top);

        uint32_t d0 = pjit_get_dn(0), d1 = pjit_get_dn(1);
        // The real SR, materialized by the program's own MOVE SR,D2 --
        // *not* pjit_get_sr() read straight off cpu_state. Ordinary ALU
        // ops leave their flags live in ARM's own CPSR; cpu->sr is only
        // ever touched by an instruction that explicitly reads or writes
        // SR/CCR as such, so reading it directly here would see whatever
        // it last held for unrelated reasons, not this case's actual
        // result.
        uint16_t sr = (uint16_t)pjit_get_dn(2);

        printf("case %5u: %-4s.%-2d a=%08X b=%08X D0=%08X D1=%08X SR=%04X\n",
               i, golden_family_names[c.fam], c.size, c.a, c.b, d0, d1, sr);
    }

    qemu_exit();
    return 0;
}
