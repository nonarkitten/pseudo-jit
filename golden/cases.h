/// @file cases.h
/// @brief Shared, deterministic test-case generator for the golden-oracle
///        harness. Included by both golden/oracle.c (host-native, drives
///        golden/mcl68k.c) and golden/pjit_side.c (ARM32/QEMU, drives
///        the real PJIT core) so both sides derive byte-identical 68K test
///        programs from the same (seed, index) pair with no runtime
///        communication between them -- each side just prints its own
///        final register state per case, and the two logs are diffed.
///
/// v1 scope, deliberately narrow to get the whole pipeline proven before
/// widening it: ADD/SUB/AND/OR/CMP, Dn,Dn register-direct form only,
/// sizes B/W/L. See golden/README.md for how to extend this.
#ifndef GOLDEN_CASES_H
#define GOLDEN_CASES_H

#include <stdint.h>

/// @brief xorshift32. Deterministic and identical on both sides -- no
///        libc dependency, no host/target-specific behaviour.
static inline uint32_t golden_rand(uint32_t *state) {
    uint32_t x = *state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *state = x;
    return x;
}

#define GOLDEN_FAMILY_COUNT 5
#define GOLDEN_SIZE_COUNT   3

static const char *const golden_family_names[GOLDEN_FAMILY_COUNT] = { "ADD", "SUB", "AND", "OR", "CMP" };
// <ea>,Dn opcode base for each family (Dn-direct source, mode=000, in the
// low 3 bits; size goes in bits7-6). All five share this exact field
// layout (the ADD/SUB/AND/OR/CMP rows of tools/gen/optab_rows.py).
static const uint16_t golden_family_base[GOLDEN_FAMILY_COUNT] = { 0xD000, 0x9000, 0xC000, 0x8000, 0xB000 };
static const int golden_sizes[GOLDEN_SIZE_COUNT] = { 8, 16, 32 };

typedef struct {
    uint32_t idx;
    int      fam;   // index into golden_family_names/golden_family_base
    int      size;  // 8, 16, or 32
    uint32_t a, b;  // initial D0, D1
} golden_case_t;

/// @brief Fills `c` for test #idx, deterministically from `seed`. Called
///        identically on both sides -- same (seed, idx) always yields the
///        same case.
static inline void golden_make_case(uint32_t seed, uint32_t idx, golden_case_t *c) {
    uint32_t s = seed ^ (idx * 0x9E3779B1u) ^ (idx << 3);
    golden_rand(&s);
    golden_rand(&s);

    c->idx  = idx;
    c->fam  = (int)(golden_rand(&s) % GOLDEN_FAMILY_COUNT);
    c->size = golden_sizes[golden_rand(&s) % GOLDEN_SIZE_COUNT];
    c->a    = golden_rand(&s);
    c->b    = golden_rand(&s);

    // Bias toward edge values sometimes -- all-zero and all-one operands
    // exercise carry/overflow boundaries random 32-bit values rarely hit
    // on their own, especially at .B/.W widths.
    if ((golden_rand(&s) & 7) == 0) c->a = 0;
    if ((golden_rand(&s) & 7) == 0) c->b = 0;
    if ((golden_rand(&s) & 7) == 0) c->a = 0xFFFFFFFFu;
    if ((golden_rand(&s) & 7) == 0) c->b = 0xFFFFFFFFu;
}

/// @brief Builds the 68K program for `c` into `out` (big-endian words),
///        returning the word count. Fixed shape: MOVE.L #a,D0 ;
///        MOVE.L #b,D1 ; <OP>.<size> D1,D0 ; MOVE SR,D2 -- D0 := D0 OP D1
///        (or, for CMP, D0 unchanged and only flags set), D2 := SR.
///
/// The trailing MOVE SR,D2 is not incidental: PJIT leaves an ordinary
/// ALU op's flags live in ARM's own CPSR rather than eagerly writing
/// them into cpu->sr (cpu->sr is only ever touched by instructions that
/// explicitly read or write SR/CCR as such -- see the ALU emitters in tools/gen/emitters/alu.py). Reading flags via a plain
/// C-side accessor after running the opcode under test sees whatever
/// cpu->sr last held for unrelated reasons, not the real result --
/// golden/pjit_side.c has to do this the same way every other real 68K
/// program would, with a genuine SR-reading instruction. Harmless on the
/// oracle side too: mcl68k.c's own mc68k_flags is always live, so this
/// is just a redundant (but byte-identical-program-preserving) extra
/// instruction there.
static inline int golden_build_program(const golden_case_t *c, uint16_t out[8]) {
    uint16_t sizefield = (c->size == 8) ? 0 : (c->size == 16) ? 1 : 2;
    out[0] = 0x203C;                          // MOVE.L #a,D0
    out[1] = (uint16_t)(c->a >> 16);
    out[2] = (uint16_t)(c->a);
    out[3] = 0x223C;                          // MOVE.L #b,D1
    out[4] = (uint16_t)(c->b >> 16);
    out[5] = (uint16_t)(c->b);
    out[6] = golden_family_base[c->fam] | (sizefield << 6) | 1; // <OP>.sz D1,D0
    out[7] = 0x40C2;                          // MOVE SR,D2
    return 8;
}

#endif
