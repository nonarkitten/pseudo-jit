/*
 * Copyright (c) 2020-2023 Renee Cousins, the Buffee Project - http://www.buffee.ca
 *
 * Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (see LICENSE).
 */

/// @file pjit.h
/// @brief cpu_t, the fixed ARM/68K register mapping, and the JIT cache
///        geometry constants.
/// @see docs/design/OVERVIEW.md
#include <stdint.h>
#include "support.h"

#ifndef __PJIT_H__
#define __PJIT_H__

#define WAY_FAST 1

// PJIT register allocation while guest code runs
//
// --ARM-- 68K  USE
// R0      T0   scratch
// R1      T1   the source extension value (an immediate, an address, or a displacement + index) that the translated prefix computed; scratch
// R2      T2   the same for the destination
// R3      T3   scratch
// R4      D0   guest D0 (pin-d0, the default; without it a scratch register). cpu_state.d0 is the master copy only while the guest is parked
// R5      CPU  the dispatch table, with cpu_t just below it (every cpu_t field is at a negative offset)
// R6-R13  A0-A7  address registers (A7 is the ARM stack pointer; A6 = R12 is clobbered by calls)
// R14 LR       the next instruction (interpreter: the prefetch handler)
//
// D1-D7 live in cpu_state as big-endian words. N Z V C are the ARM flags; X is in GE[3:0], inverted (GE = 1111 when X is clear). Guest code runs under E=BE.

/// A pseudo-vector, not a 68K one: the interpretive STOP raises it to end
/// pjit_run() (nothing can wake a STOP yet). pjit_run() returns it.
#define PJIT_VEC_STOP 0x100

typedef enum {
    RESET_SP,       // 0  Initial stack pointer
    RESET_PC,       // 1  Inital program counter
    BUSERR,         // 2  Bus error
    ADDRESSERR,     // 3  Address error
    ILLINSTR,       // 4  Illegal instruction
    DIVZ,           // 5  Division by zero
    TRAPCHK,        // 6  CHK, CHK2 instruction
    TRAPV,          // 7  cpTRAPcc, TRAPcc, TRAPV instructions
    PRIV,           // 8  Privillege violation
    TRACE,          // 9  Trace emulation
    LINE_A,         // 10 Unhandled Line 1010 Emulator
    LINE_F,         // 11 Unhandled Line 1111 Emulation
    RESERVED_12,    // 12 Unassigned
    COPROPV,        // 13 Coprocessor protocol violation
    FORMATERR,      // 14 Format error
    UNINITIRQ,      // 15 Uninitialized interrupt
    RESERVED_16,    // 16 Unassigned
    RESERVED_17,    // 17 Unassigned
    RESERVED_18,    // 18 Unassigned
    RESERVED_19,    // 19 Unassigned
    RESERVED_20,    // 20 Unassigned
    RESERVED_21,    // 21 Unassigned
    RESERVED_22,    // 22 Unassigned
    RESERVED_23,    // 23 Unassigned
    SPURIOUS,       // 24 Spurious interrupt
    AUTOINT1,       // 25 Level 1 Autovector (IPL)
    AUTOINT2,       // 26 Level 2 Autovector (IPL)
    AUTOINT3,       // 27 Level 3 Autovector (IPL)
    AUTOINT4,       // 28 Level 4 Autovector (IPL)
    AUTOINT5,       // 29 Level 5 Autovector (IPL)
    AUTOINT6,       // 30 Level 6 Autovector (IPL)
    AUTOINT7,       // 31 Level 7 Autovector (IPL)
    TRAP0,          // 32 TRAP #0 instruction
    TRAP1,          // 33 TRAP #1 instruction
    TRAP2,          // 34 TRAP #2 instruction
    TRAP3,          // 35 TRAP #3 instruction
    TRAP4,          // 36 TRAP #4 instruction
    TRAP5,          // 37 TRAP #5 instruction
    TRAP6,          // 38 TRAP #6 instruction
    TRAP7,          // 39 TRAP #7 instruction
    TRAP8,          // 40 TRAP #8 instruction
    TRAP9,          // 41 TRAP #9 instruction
    TRAP10,         // 42 TRAP #10 instruction
    TRAP11,         // 43 TRAP #11 instruction
    TRAP12,         // 44 TRAP #12 instruction
    TRAP13,         // 45 TRAP #13 instruction
    TRAP14,         // 46 TRAP #14 instruction
    TRAP15,         // 47 TRAP #15 instruction
    FPUNORDERED,    // 48 FPCP Branch or Set on Unordered
    FPINEXACT,      // 49 FPCP Inexact result
    FPDIVZ,         // 50 FPCP Division by zero
    FPUNDERFLOW,    // 51 FPCP Underflow
    FPOPERROR,      // 52 FPCP Operand error
    FPOVERFLOW,     // 53 FPCP Overflow
    FPSNAN,         // 54 FPCP Signalling NAN
    RESERVED_55,    // 55 Unassigned
    PMMUCONFIG,     // 56 PMMU Configuration
    PMMUILLEGALOP,  // 57 PMMU Illegal operation
    PMMUACCESSV,    // 58 PMMU Access level violation
} vector_t;



// cpu_t is laid out for r5 = the base of the dispatch table, which sits
// immediately *after* it (see pjit_arena below): JIT'd code reaches every
// field at a NEGATIVE offset from r5, which costs the same as a positive
// one and saves loading the DT base on every "next". Fields are ordered so
// the most-used are nearest r5 (i.e. last). Everything reached with
// ldrh/ldrsh/ldrd/strd (imm8, +/-255) must be within 255 bytes of the end
// of the struct -- see the static asserts after CPU_OFF().
typedef struct {
    // ---- cold: config, pointers, rarely-touched state ----


    // LR Backup
    uint32_t* lr;

    // 68K Program Counter
    uint16_t* pc;

    // CPU control registers
    uint32_t sfc;    // Source Function Code
    uint32_t dfc;    // Destination Function Code
    uint32_t vbr;    // Vector Base Register
    uint32_t cacr;   // Cache Control Register
    uint32_t caar;   // Cache Address Register
    uint32_t tc;     // MMU Translation Control Register
    uint32_t itt0;   // Instruction Transparent Translation Register 0
    uint32_t isp;    // Interrupt Stack Pointer
    uint32_t itt1;   // Instruction Transparent Translation Register 1
    uint32_t mmusr;  // MMU Status Register
    uint32_t dtt0;   // Data Transparent Translation Register 0
    uint32_t urp;    // User Root Pointer
    uint32_t dtt1;   // Data Transparent Translation Register 1
    uint32_t srp;    // Supervisor Root Pointer


    // Pointers to everything
    uint32_t*  cache_data;   // the DR cache: DR_CACHE_BYTES of translated code, direct-mapped (see below)
    uint32_t* opcode_table;  // the DT: four ARM words per opcode (pjit_arena.dt)
    uint16_t* ot;            // the offset table: one 16-bit entry per opcode (pjit_arena.ot)
    uint32_t* drsr;          // base of the dynamic-recompilation semantic routines



    /* FPU regs: */

    // Where pjit_run() parks the host's (C's) stack pointer while guest
    // code owns SP (A7 *is* SP).
    uint32_t host_sp;

    // Which of TRAP #0-#15 end pjit_run() instead of being delivered to the
    // guest (bit n = TRAP #n); the vector number is its return value. Default
    // 0x8000 (TRAP #15), set by pjit_cache_init(). The golden harness widens it.
    uint32_t trap_exit_mask;
    // Interrupts (pjit/runtime.s, pjit_irq_entry): the highest 68K interrupt level (1-7)
    // posted and not yet delivered, and, in the slot-cache-only build (interrupt windows, no latch),
    // where the host IRQ landed (a window in the interpreter's prefetch handler or the slot router);
    // 0 in the builds with the latch (docs/design/TIERING.md), where the guest resumes at cpu->ipc.
    uint32_t irq_level;
    uint32_t irq_site;
    // Block runtime (PJIT_BLK): 1 when a posted level beats the SR mask -- set by the IRQ latch (pjit_irq_entry), polled by the router and the backward exit stubs, cleared by whoever polls.
    uint32_t irq_flag;
    uint32_t flush_req;     // nonzero: translated code may be stale (a guest write hit a page that holds code, platform/buffee/buffee.c): the router flushes both tiers at its next entry (pjit_post_flush)
    uint32_t codeexact;     // the platform's bitmap of the 16-byte guest groups that were translated (exact: a write to one of them is a write to code); the slot fill sets the bits of its group and the next one, 0 = off
    uint32_t xlat;          // the platform's table of 4096 halfwords, one per guest megabyte: 0 = code there is not translated (the interpreter runs it), else 1 + the megabyte's index in the tracked space (code bitmaps: a guest address is (index << 20) | its low 20 bits); 0 = no table (every address at or above dr_threshold is translated)
    uint32_t codebits;      // the platform's bitmap of 128-byte guest lines that hold translated code (pjit_platform_code), tested by the slot router before a line claim; 0 = no tracking
    uint32_t irq_unmask;    // 0x80 when pjit_irq_enable(1): the block runtime clears the I bit of CPSR on every way into guest code (a byte: read big-endian too)
    // Interrupt windows (see pjit_irq_enable): off = the guest never sees a host IRQ and pays
    // nothing for the possibility; prefetch_hook is what interpretive code returns to between
    // instructions (LR), with or without the window.
    uint32_t irq_windows;
    uint32_t prefetch_hook;
    // Byte-swapped copies of cache_data and tags, for the router's flag-free fast path, which runs
    // big-endian like the guest (a pointer loaded there would otherwise read swapped).
    uint32_t cache_be;
    uint32_t tags_be;
    // The shadow return stack: every cached call records (guest return address, host return address) here, and a
    // cached RTS whose popped guest address matches its entry jumps to the host address with `bx lr`, which the
    // hardware return predictor has seen the matching `bl` for. 256 entries of two words, indexed by the guest stack
    // pointer: entry ((A7 >> 2) & 0xFF) holds the call made with that A7 (before the push), which is the A7 an RTS leaves
    // behind when it pops it. No pointer to keep; a deeper stack aliasing an entry, or an unwound one, just fails the
    // guest-address compare and takes the router. Only big-endian (cached) code reads or writes these; an entry whose
    // guest address is odd never matches. Flushed (every guest address set to 1) when a miss evicts a line, so no entry
    // can name replaced code.
    uint32_t shadow[512];
    uint32_t ipc;         // the interpreter's guest PC (big-endian, raw: only BE code reads it; LE code byte-swaps): r4 is free in every body
    uint32_t tags_goto;   // the byte-swapped tag pointer pjit_goto reads: tags_be, or an all-miss array while interrupt windows are on (one check less in the router)
    uint32_t tags_miss;   // the byte-swapped pointer to an all-miss tag array (tiered build: the IRQ latch points tags_goto at it so the next slot fast-path transfer enters the router, which polls)

    // ---- the direct-mapped DR cache (pjit/runtime_dr.s) ----
    uint16_t* tags;          // DR_CL_COUNT 16-bit tags, one per cache line; 0xFFFF = invalid
    // A guest control transfer whose target is >= this runs from the DR cache,
    // below it in interpretive mode. 24-bit address space = uncacheable: the
    // MMU is never consulted. 0xFFFFFFFF = interpretive only (the interpreter
    // tests); pjit_cache_init() sets DR_THRESHOLD_DEFAULT.
    uint32_t dr_threshold;
    // Spill slots for the cache-fill routine, which has more live values than
    // registers it may use (r6-r13 are the guest's A0-A7).
    uint32_t spill[32];       // [7] = "r4 already holds the guest PC" for the exception entry (set by the
                              // router / pjit_i_chk_pc before they raise an address error)

    // ---- hot, least- to most-used; the last field is nearest r5 ----
    // (sizeof(cpu_t) must be a multiple of 8 with no trailing padding, or
    // every CPU_OFF() shifts; a spare word somewhere keeps it so.)

    // Live ARM CPSR at the start of DIVU/DIVS, kept across the helper call
    // so an overflow can leave N and Z exactly as they were (see handle_divw.s).
    uint32_t div_cpsr;

    // A word of scratch for a semantic body that needs one more temporary
    // than r0-r3 give it (r4 is the interpreter's guest PC, so it is never
    // available). Dead between instructions; bodies never nest.
    uint32_t scratch;

    // Where DIVU/DIVS put the quotient / remainder when the helper returns (the address of the destination data register: the helper clobbers r0-r3, so
    // the shared DIV routine parks it here). Was lr_save, then padding, so every hot offset stays where it was.
    uint32_t div_dst;


    // The guest's live CPSR, parked by the router and the cores while they use
    // flag-setting instructions (the live N Z C V are the 68K's CCR).
    uint32_t jmp_cpsr;

    // ARM registers for backup: r0-r3 are T0-T3 (transient decode temps,
    // not real 68K state -- backed up here only so exception frame
    // construction can still see them). r0..r3 must stay adjacent and
    // 8-byte aligned: pjit_svc_entry() saves them with two STRDs.
    //
    // r12 is A6, and is the one piece of live 68K state in an AAPCS
    // call-clobbered register: A0-A5 (r6-r11) are callee-saved, so any
    // ABI-conforming C function preserves them for free, but anything
    // calling into normal C (as the SVC exception path does) must
    // save/restore r12 explicitly or lose A6 (pjit_svc_entry in runtime.s keeps it in cpu->r12).
    uint32_t r12;
    uint32_t r0, r1, r2, r3;

    uint32_t ssp;    // Master Stack Pointer
    uint32_t usp;    // User Stack Pointer
    uint32_t spare;  // pads r0 to the 8-byte boundary the STRDs need

    // Status register. Big-endian, like D0-D7; the CCR is the byte at sr+1.
    uint16_t sr;
    uint16_t sr_pad;

    // The 68K register file as one array, D0-D7 then A0-A7 -- the order of
    // the register field in an extension word (bit 15 = A/D, bits 14:12 =
    // number), so an index-register extension can address it directly. D0-D7
    // are the live copies; A0-A7 are only *shadows*: the live ones are r6-r13,
    // and pjit_run()/the exit path (and indexed-mode extension handlers)
    // `stm`/`ldm` between the two. Stored big-endian like everything else
    // JIT'd code touches -- use pjit_get_an()/pjit_set_an() from C.
    uint32_t d0, d1, d2, d3, d4, d5, d6, d7;
    uint32_t a0, a1, a2, a3, a4, a5, a6, a7;
} cpu_t;

/// One arena so the dispatch table sits directly after cpu_t: r5 points at
/// dt[0], and `cpu` (below) recovers &state from it.
#define DR_CL_COUNT_ 4096         /* the most lines any geometry has (128-byte lines) */
#define FUSE_WORDS   32768        /* the fused-loop area: 2048 blocks of 16 words */
typedef struct {
    cpu_t    state;
    uint32_t dt[4 * 65536];   // four ARM words per 68K opcode (one cache slot); see cache.c
    uint8_t  ext[65536];      // pjit_ext_table, copied here so the runtime reaches it as r5 + ARENA_EXT
                              // (a literal-pool load would be byte-swapped under E=BE)
    uint16_t ot[65536];       // offset table: see the OT_* / DRSR_* notes below
    // MOVEM.L with A7 as the base, specialised per register mask: [0] saves (MOVEM.L regs,-(A7)), [1] restores
    // (MOVEM.L (A7)+,regs); entry = packed mask ((m >> 2) & 0x3F) | ((m >> 10) & 0x1F) << 6, i.e. D2-D7 and A2-A6
    // (what a compiler saves). 32 words (128 bytes) each: a 3-instruction alignment guard that hands a
    // 2-mod-4 stack to the generic code (LDM/STM fault on it), then the routine. Built by tools/gen/maketables.py.
    uint32_t movem[2][2048][32];
    // Per opcode: 0 = the DRSR body is called with `bl`; n + 1 = the body is n straight-line words and the fill copies it
    // into the slot (built by tools/gen/maketables.py, used by pjit_fill). 64KB so it can sit at an immediate offset from r5.
    uint8_t  inl[65536];
    // Direct links between cache lines (known-target JSR/BSR/JMP/BRA/Bcc, see pjit_fill): the lines that hold
    // one. When a miss evicts a valid line of another address, every such line is invalidated (a link into
    // the evicted line would run the wrong code); they are retranslated, and relinked, as they are entered.
    struct {
        uint32_t count;           // lines in list[]
        uint32_t evictions;       // debug counters: valid lines replaced by another address ...
        uint32_t invalidated;     // ... lines invalidated because of them ...
        uint32_t made;            // ... and links written
        uint32_t pad[60];         // (keeps flag[] and list[] at offsets the assembler can add as immediates)
        uint8_t  flag[DR_CL_COUNT_];
        uint16_t list[DR_CL_COUNT_];
    } links;
    // X_LAZY tables (tools/gen/gen.py, README "Lazy X"): the same offset / inline tables for the *exact* sequences (the ones that save X), which
    // pjit_fill uses for a line that is near an instruction that reads X; in an exact build they are copies of ot[] / inl[]. xcons[op] = 1
    // info for the scan of pjit_fill: bits 0-1 = the opcode reads X (1: always, 2: only trusted at an instruction start), bits 2.. = its extension
    // words (see xcons_table in tools/gen/maketables.py). xexact[line]: 0 = not looked at yet, 1 = lazy, 2 = exact.
    uint16_t ot_x[65536];
    uint8_t  inl_x[65536];
    uint8_t  xcons[65536];
    // The loop compactor (pjit_fill, compact_try): per opcode bits 0-6 = the length of its body + 1 when the body can be copied into a block, bit 7 = it can be called from
    // one (see compact_table in tools/gen/maketables.py); the exact twin for lines near an X reader. comp[]: scratch for the compactor.
    uint8_t  cinl[65536];
    uint8_t  cinl_x[65536];
    // Per opcode: 0 = the DT entry is a leader (word 0) and a branch to the body; n (1-3) = the entry is n words of the opcode's own code, nop padding and a branch to a
    // routine shared with the family (the fill copies the n words into the slot, then the routine: see Seq in tools/gen/gen.py).
    uint8_t  form[65536];
    uint8_t  xexact[DR_CL_COUNT_];
    uint32_t comp[128];
    // Native loops for string / copy idioms (pjit_fill, fill_fuse_try): a `MOVE.B (An)+,(Am)+` that a DBF (or BNE) branches straight back to becomes a
    // branch into a tight ARM loop here, in blocks of 16 words (64 bytes, one cache line). fuse_next = words used; pjit_cache_flush empties it.
    uint32_t fuse_next __attribute__((aligned(64)));      // (the area starts exactly 64 bytes after it: fill_fuse_try relies on that)
    uint32_t fuse_pad[15];
    uint32_t fuse[FUSE_WORDS] __attribute__((aligned(64)));
} pjit_arena_t;
_Static_assert(__builtin_offsetof(pjit_arena_t, fuse) == __builtin_offsetof(pjit_arena_t, fuse_next) + 64, "fill_fuse_try: the fused-loop area must start 64 bytes after fuse_next");
_Static_assert(__builtin_offsetof(pjit_arena_t, movem) - __builtin_offsetof(pjit_arena_t, dt) == 0x130000,
               "ARENA_MOVEM must be an ARM immediate from r5: dt 1MB + ext 64KB + ot 128KB");

/// Byte offset of a cpu_t field from r5, i.e. from the end of cpu_t. Always
/// negative. Use this, never offsetof(), for anything addressed via r5.
#define CPU_OFF(f) ((int)__builtin_offsetof(cpu_t, f) - (int)sizeof(cpu_t))

_Static_assert(sizeof(cpu_t) % 8 == 0, "DT must be 8-byte aligned after cpu_t");
_Static_assert(CPU_OFF(d0) == -64 && CPU_OFF(a0) == -32, "d0-d7 then a0-a7 must be the last 16 words of cpu_t");
_Static_assert(CPU_OFF(div_cpsr) >= -255, "hot state must stay within ldrh/strd's +/-255 reach of r5");
_Static_assert(CPU_OFF(r0) % 8 == 0, "pjit_svc_entry() STRDs r0/r1 and r2/r3");

#define INLINE __attribute__((always_inline)) static inline

extern pjit_arena_t pjit_arena;
#define cpu_state (pjit_arena.state)

// D0-D7 are deliberately NOT pinned to ARM registers; all eight live in
// cpu_state, stored big-endian like every other 68K-visible value. JIT'd
// code runs with ambient E=BE, so ordinary C code (running E=LE) must go
// through these accessors rather than touching cpu->dN directly.
//
// r5 is the base of the dispatch table, not of cpu_t: cpu_t is the
// sizeof(cpu_t) bytes just below it. Assign pjit_dt (see pjit_cache_init()),
// read the state through `cpu`.
register uint32_t* pjit_dt asm("r5");
#define cpu ((cpu_t*)((uint8_t*)pjit_dt - sizeof(cpu_t)))

INLINE uint32_t pjit_get_dn(int n)             { return __builtin_bswap32((&cpu->d0)[n]); }
INLINE void     pjit_set_dn(int n, uint32_t v) { (&cpu->d0)[n] = __builtin_bswap32(v); }
// The A0-A7 shadows (see cpu_t): where pjit_run() loads A0-A7 from and the
// exit path stores them to. Big-endian, like D0-D7.
INLINE uint32_t pjit_get_an(int n)             { return __builtin_bswap32((&cpu->a0)[n]); }
INLINE void     pjit_set_an(int n, uint32_t v) { (&cpu->a0)[n] = __builtin_bswap32(v); }

// SR is big-endian for the same reason, and the generated code depends on it: it
// reaches the CCR as the byte at sr + 1, which is only the low half of the word
// under big-endian storage.
INLINE uint16_t pjit_get_sr(void)       { return __builtin_bswap16(cpu->sr); }
INLINE void     pjit_set_sr(uint16_t v) { cpu->sr = __builtin_bswap16(v); }

register union { uint32_t L; uint16_t W; } A0 asm("r6");
register union { uint32_t L; uint16_t W; } A1 asm("r7");
register union { uint32_t L; uint16_t W; } A2 asm("r8");
register union { uint32_t L; uint16_t W; } A3 asm("r9");
register union { uint32_t L; uint16_t W; } A4 asm("r10");
register union { uint32_t L; uint16_t W; } A5 asm("r11");
register union { uint32_t L; uint16_t W; } A6 asm("r12");
register union { uint32_t L; uint16_t W; } A7 asm("r13");

extern void pjit_cache_init(uint32_t top);


extern void pjit_svc_entry(void);       // runtime.s: the SVC vector's target

/// @brief Runs guest code from `pc` in interpretive mode until it executes
///        a TRAP whose bit is set in cpu->trap_exit_mask (TRAP #15 by
///        default), then returns that TRAP's exception vector (32 + n). C -> asm: the only
///        way in. Guest registers come from, and go back to, cpu_state
///        (D0-D7, A0-A7 shadows, SR, USP/SSP); cpu->pc is left just past the TRAP.
extern uint32_t pjit_run(uint16_t *pc);

// The offset table (OT), one 16-bit entry per opcode. It is what lets the
// dispatch table (whose word 1 leads to the *interpretive* semantic routine)
// and the DR cache (which needs the *dynamic-recompilation* semantic routine
// -- the same instruction's tail, minus the extension-word fetching, ending in
// `bx lr` back to the cache) point at different code for the same opcode.
//
//     bits 15:2  DRSR entry index; the entry is at drsr + (index << DRSR_SHIFT)
//     bits  1:0  OT_LEN_* : the guest instruction's length class
//
// Every DRSR entry starts on a (1 << DRSR_SHIFT)-byte boundary and is
// followed by DRSR_PAD when it is shorter than a whole unit, so a translator
// can tell a one-word body (DRSR_PAD in word 1) without knowing its length.
#define DRSR_SHIFT      4               /* a DRSR unit is 16 bytes: a quarter of a Cortex-A8 cache line */
#define DRSR_UNIT       (1u << DRSR_SHIFT)
#define DRSR_MAX_UNITS  (1u << 14)
#define DRSR_PAD        0xE7F000F0u     /* UDF #0: never executed, only sits in padding */

#define OT_LEN_2        0u              /* normal, two-byte guest instruction   */
#define OT_SPECIAL      1u              /* special case: needs bespoke handling */
#define OT_LEN_4        2u              /* normal, four-byte guest instruction  */
#define OT_LEN_6        3u              /* normal, six-byte guest instruction   */

/// @brief The DRSR entry (a host address) for `opcode`.
static inline uint32_t *pjit_drsr_entry(uint16_t opcode) {
    return (uint32_t *)((uint32_t)cpu->drsr + ((uint32_t)(cpu->ot[opcode] >> 2) << DRSR_SHIFT));
}
/// @brief Does ARM word `w` read register `r`? (Conservative; used to audit leaders.)
extern int pjit_arm_reads_reg(uint32_t w, unsigned r);
/// @brief How many 68K words (opcode plus extension words) `opcode` occupies.
extern uint32_t pjit_ext_word_count(uint16_t opcode);

// The direct-mapped DR cache. Guest address -> host address is arithmetic:
//
//   tttt tttt tttt t lll llll llll l ooo oooo 0     (4MB cache, 13-bit tags)
//   |-- tag --|     |---- line ----| |- word -|     one guest word = 16 host bytes (4 ARM words)
//
// A cache line (CL) is 1 << DR_HL_SHIFT host bytes = 1 << (DR_CL_SHIFT - 1) guest words, made of
// cache-line entries (CLE) of 64 bytes (16 ARM words, 4 slots = 4 guest words). Default 512 guest bytes (4KB
// host, 64 entries): Dhrystone, ARM instructions per run: 128 B 4,924; 256 B 4,820; 512 B 4,614 (more same-line
// branches). The upper bound is the slot mask having to be an ARM immediate (<= 255, so 512 B).
#define DR_CACHE_BYTES  (4u * 1024u * 1024u)
#ifndef DR_CL_SHIFT
#define DR_CL_SHIFT     9                               /* guest bytes per CL = 1 << this (9 = 512) */
#endif
#define DR_HL_SHIFT     (DR_CL_SHIFT + 3)               /* host bytes per CL = 1 << this: 8 per guest byte (a guest halfword is a 16-byte slot, 4 ARM words) */
#define DR_CL_COUNT     (DR_CACHE_BYTES >> DR_HL_SHIFT) /* 1024 at 512 */
#define DR_LINE_BITS    (22 - DR_HL_SHIFT)              /* log2(DR_CL_COUNT) */
_Static_assert(DR_CL_COUNT <= DR_CL_COUNT_, "pjit_arena_t.links is too small for this geometry");
#define DR_SLOT_MASK    ((1 << (DR_CL_SHIFT - 1)) - 1)  /* slots (16 host bytes) per CL, minus one */
#define DR_TAG_SHIFT    (DR_CL_SHIFT + DR_LINE_BITS)    /* 19: the guest bytes the cache spans, as a shift */
#define DR_THRESHOLD_DEFAULT 0x01000000u                /* the 24-bit threshold */

/// @brief Invalidates the whole DR cache (a tag of 0xFFFF matches nothing).
///        A harness that rewrites guest code in place calls this first.
extern void pjit_cache_flush(void);
/// Turns interrupt delivery on or off (it is off by default: no windows, no cost). Flushes the DR cache,
/// since the loops translated with or without the loop core differ. Call before pjit_run().
extern void pjit_irq_enable(int on);


#endif
