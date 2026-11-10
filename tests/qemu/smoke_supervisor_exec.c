/// @file smoke_supervisor_exec.c
/// @brief Privileged/supervisor-only instructions: MOVE to/from SR, MOVE
///        to/from USP, ANDI/ORI/EORI to SR, RESET, STOP -- each checked
///        for both effect (run from supervisor mode) and privilege
///        enforcement (run from user mode, must take a real Privilege
///        Violation exception), plus RTR's "CCR only, not full SR" pop.
///
/// Modelled on smoke_exception_exec.c's harness (real vector table, real
/// handler code, RTE/RTR back to a stop slot), extended with a per-case
/// initial SR so both user- and supervisor-mode entry can be exercised.
#include <stdint.h>
#include "pjit.h"
#include "pjit_compat.h"
#include "mem_be.h"

extern void qemu_exit(void);
extern void emit_opcode_table(void);

static uint8_t  ram_pool[12 * 1024 * 1024] __attribute__((aligned(4096)));
static uint16_t progmem[32768u/2] __attribute__((aligned(32768u)));

static uint32_t vectors[64]    __attribute__((aligned(4)));
static uint32_t sup_stack[256] __attribute__((aligned(8)));

uint32_t saved_sp;   // global: named directly from asm below, see run_case()

#define HANDLER_BASE 4096
#define HANDLER_SIZE 8
#define PRIV_MARKER  0x2A  // must be 0x00-0x7F: handler()'s MOVEQ sign-extends

static uint32_t off, base, np, nh;
static int pass, fail;

static void wbe(uint16_t *a, uint16_t v) { ((uint8_t*)a)[0] = v >> 8; ((uint8_t*)a)[1] = v; }
static void w(uint16_t v)   { wbe(&progmem[base + np++], v); }
static void w32(uint32_t v) { w(v >> 16); w(v); }
static void start(void)     { base = off; np = 0; off += 32; }

/// @brief Installs a handler for `vector` returning `marker` in D0, the
///        pushed SR in D1, the pushed PC in D2 -- see smoke_exception_exec.c.
static void handler(int vector, uint8_t marker)
{
    uint16_t *h = &progmem[HANDLER_BASE + nh++ * HANDLER_SIZE];
    wbe(&h[0], 0x3217);           // MOVE.W (A7),D1    -- pushed SR
    wbe(&h[1], 0x242F);           // MOVE.L 2(A7),D2   -- pushed PC
    wbe(&h[2], 0x0002);
    wbe(&h[3], 0x7000 | marker);  // MOVEQ #marker,D0
    wbe(&h[4], 0x2F43);           // MOVE.L D3,2(A7)  -- resume after the case: the frame holds the instruction's own address
    wbe(&h[5], 0x0002);
    wbe(&h[6], 0x4E73);           // RTE
    write_BE_long((uint32_t)&vectors[vector], (uint32_t)h);
}

static struct {
    const char *name;
    uint32_t base, np;
    uint16_t init_sr;
    int      traps;       // 1: must take PRIV, check D2 == resume PC
    uint32_t want;         // traps: PRIV_MARKER. else: D0, *extra, or SR
    uint32_t *extra;       // non-NULL: compare *extra instead of D0
    int      use_sr;       // 1: compare pjit_get_sr() instead of D0/extra
    uint32_t custom_sp;    // nonzero: entry SP (and initial ssp/usp) for
                           // this case, instead of the shared `stk`
} cases[32];
static int ncases;

/// @brief Records a D0-checked case (trap or plain value).
static void def(const char *name, uint16_t init_sr, int traps, uint32_t want)
{
    cases[ncases] = (typeof(cases[0])){ name, base, np, init_sr, traps, want, 0, 0, 0 };
    ncases++;
}

/// @brief Records a case checked against a cpu_state field instead of D0
///        (MOVE to/from USP: the result lands in a pinned ARM register or
///        a plain memory field, not somewhere the shared D0 read-back can
///        see without an extra MOVE.L An,D0, which the USP-write case has
///        no register to spare for).
static void def_extra(const char *name, uint16_t init_sr, uint32_t *extra, uint32_t want)
{
    cases[ncases] = (typeof(cases[0])){ name, base, np, init_sr, 0, want, extra, 0, 0 };
    ncases++;
}

/// @brief Records a case checked via pjit_get_sr() straight off cpu_state,
///        instead of round-tripping the result back through a second
///        translated "MOVE SR,D0" -- that second read re-derives the CCR
///        nibble from whatever ARM's *live* condition flags are at that
///        later point rather than reading back what this case's own op
///        just stored, so it is not a trustworthy check on its own.
static void def_sr(const char *name, uint16_t init_sr, uint16_t want)
{
    cases[ncases] = (typeof(cases[0])){ name, base, np, init_sr, 0, want, 0, 1, 0 };
    ncases++;
}

#define POISON 0xDEADBEEFu

/// @brief Patches the stop slot, then runs the case on a real 68K stack.
/// @details See smoke_exception_exec.c's run_case() for why this is its
///          own noinline function and why saved_sp is a global.
static void run_case(uint32_t e, uint32_t np, uint32_t stk)
{
    uint8_t *stop = (uint8_t *)(e + np * 2);          // TRAP #15 after the case
    stop[0] = 0x4E; stop[1] = 0x4F;
    pjit_set_an(7, stk);
    (void)pjit_compat_run((uint16_t *)e);
}

static void exec_all(void)
{
    uint32_t stk = (uint32_t)&sup_stack[256];

    for (int i = 0; i < ncases; i++) {
        printf("[RUN] %s\n", cases[i].name);
        uint32_t e = pjit_cache_dispatch((uint32_t)&progmem[cases[i].base]);
        if (!e) { printf("[FAIL] %-14s dispatch returned 0\n", cases[i].name); fail++; continue; }

        uint32_t entry_sp = cases[i].custom_sp ? cases[i].custom_sp : stk;
        pjit_set_sr(cases[i].init_sr);
        PJIT_SET_STACKS(entry_sp);
        pjit_set_dn(0, POISON);
        pjit_set_dn(1, POISON);
        pjit_set_dn(2, POISON);
        pjit_set_dn(3, (uint32_t)&progmem[cases[i].base + cases[i].np]);   // where the handler resumes

        // want_pc for trap cases: the address right after this case's own
        // program, exactly like smoke_exception_exec.c -- every trapping
        // case here is a single privileged op (plus its own extension
        // words), so nothing after the trap point ever executes.
        uint32_t want_pc = (uint32_t)&progmem[cases[i].base + cases[i].np];

        run_case(e, cases[i].np, entry_sp);

        uint32_t d0 = pjit_get_dn(0), d1 = pjit_get_dn(1), d2 = pjit_get_dn(2);
        // cpu->usp/ssp are written by guest code (MOVE to/from USP, the
        // USP/SSP re-bank of the SR instructions) via plain str/ldr under
        // ambient E=BE, so a value this test wrote from guest code has to
        // be read back byte-swapped from ordinary (E=LE) C -- same reason
        // pjit_get_dn() exists for D0-D7.
        uint32_t got = cases[i].use_sr ? pjit_get_sr()
                     : cases[i].extra  ? __builtin_bswap32(*cases[i].extra)
                     : d0;
        int ok = (got == cases[i].want);

        if (ok && cases[i].traps && d2 != (uint32_t)&progmem[cases[i].base]) ok = 0;   // the frame has the instruction's own address

        if (ok) { pass++; printf("[ ok ] %-14s got=%08X\n", cases[i].name, got); }
        else {
            fail++;
            printf("[FAIL] %-14s got=%08X want=%08X | frame SR=%08X PC=%08X want %08X\n",
                   cases[i].name, got, cases[i].want, d1, d2, want_pc);
        }
    }
}

int main(void)
{
    printf("=== PJIT smoke test: privileged instructions and privilege violation ===\n");

    pjit_cache_init((uint32_t)(ram_pool + sizeof(ram_pool)));
    emit_opcode_table();

    for (uint32_t i = 0; i < sizeof(progmem)/2; i++) wbe(&progmem[i], 0x4E71); // NOP fill

    cpu->vbr = (uint32_t)vectors;

    handler(0, 0xEE);   // catch-all, so a wrong vector reports 0xEE not a hang
    for (int v = 0; v < 64; v++)
        write_BE_long((uint32_t)&vectors[v], (uint32_t)&progmem[HANDLER_BASE]);

    handler(PRIV, PRIV_MARKER);

    // ---- Privilege violation: each of these, attempted from user mode
    // (SR=0x0000), must trap PRIV instead of executing.
    start(); w(0x46C0);            def("PRIV MOVE SR",  0x0000, 1, PRIV_MARKER); // MOVE D0,SR
    start(); w(0x4E60);            def("PRIV MOVE USP<",0x0000, 1, PRIV_MARKER); // MOVE A0,USP
    start(); w(0x4E68);            def("PRIV MOVE USP>",0x0000, 1, PRIV_MARKER); // MOVE USP,A0
    start(); w(0x027C); w(0xFFFF); def("PRIV ANDI SR",  0x0000, 1, PRIV_MARKER); // ANDI #$FFFF,SR
    start(); w(0x007C); w(0x0000); def("PRIV ORI SR",   0x0000, 1, PRIV_MARKER); // ORI  #$0000,SR
    start(); w(0x0A7C); w(0x0000); def("PRIV EORI SR",  0x0000, 1, PRIV_MARKER); // EORI #$0000,SR
    start(); w(0x4E72); w(0x0000); def("PRIV STOP",     0x0000, 1, PRIV_MARKER); // STOP #$0000
    start(); w(0x4E70);            def("PRIV RESET",    0x0000, 1, PRIV_MARKER); // RESET

    // ---- MOVE from SR: unprivileged on the 68000 (the 68010 made it
    // privileged; that is out of scope). Runs from user mode on
    // purpose, to prove it does *not* trap. MOVEQ #0,D0 first: the CCR
    // nibble SR reads back is reconstructed from *live ARM flags*, not
    // from whatever pjit_set_sr() poked directly, so a known flag state
    // has to come from a real 68K op, not the C-side SR accessor.
    start(); w(0x7000);            // MOVEQ #0,D0   -- Z=1,N=V=C=0, D0=0
             w(0x40C0);             // MOVE SR,D0
                                    def("MOVE SR,D0 usr", 0x0000, 0, 0x00000004u);

    // ---- MOVE to SR (supervisor): D0's low word becomes the new SR;
    // readback via a second MOVE SR,D0 proves the store, the S-bit
    // re-bank (still supervisor, same stack) didn't corrupt anything, and
    // the CCR nibble round-trips through the ARM-flag mirror intact.
    start(); w(0x203C); w32(0x1234A700u);   // MOVE.L #$1234A700,D0 (S=1,I=7,CCR=0)
             w(0x46C0);                     // MOVE D0,SR
             w(0x40C0);                     // MOVE SR,D0
                                    def("MOVE D0,SR",     0x2000, 0, 0x1234A700u);

    // ---- MOVE to/from USP (supervisor): USP is plain memory
    // (cpu_state), checked directly rather than via D0.
    start(); w(0x207C); w32(0xCAFEBABEu);   // MOVEA.L #$CAFEBABE,A0
             w(0x4E60);                     // MOVE A0,USP
                                    def_extra("MOVE A0,USP", 0x2000, &cpu->usp, 0xCAFEBABEu);

    // Round-trips through both directions in one program: A0 primes USP,
    // gets cleared, then MOVE USP,A0 must reload the primed value -- so a
    // wrong readback can only come from the opcode under test, not from
    // whatever cpu->usp happened to hold beforehand.
    start(); w(0x207C); w32(0xF00DCAFEu);   // MOVEA.L #$F00DCAFE,A0
             w(0x4E60);                     // MOVE A0,USP
             w(0x207C); w32(0u);            // MOVEA.L #0,A0
             w(0x4E68);                     // MOVE USP,A0
             w(0x2008);                     // MOVE.L A0,D0
                                    def("MOVE USP,A0",   0x2000, 0, 0xF00DCAFEu);

    // ---- ANDI/ORI/EORI to SR (supervisor): full-word effect, including
    // flipping the S bit itself mid-instruction, proving the USP/SSP
    // re-bank survives it. Checked via pjit_get_sr() directly
    // (def_sr()), not a second translated "MOVE SR,D0" -- see def_sr()'s
    // comment. The CCR-nibble quirk still applies to the *input*
    // side (each op builds the current SR from the live flags before
    // combining it with the immediate) -- ANDI's and ORI's masks are
    // chosen with an all-0 or all-1 low nibble so the result's CCR bits
    // are forced regardless of what that read gave; EORI can't
    // be forced that way (XOR), so it primes a known flag state first.
    start(); w(0x027C); w(0xDFF0u);
                                    def_sr("ANDI SR clrS",  0x2700, 0x0700); // clears S, keeps I=7, forces CCR=0
    start(); w(0x007C); w(0x07FFu);
                                    def_sr("ORI SR setI",   0x2000, 0x271F); // sets I=7, forces CCR=1F, keeps S (SR bits 7-5 do not exist: they read 0, so $07FF gives $271F)
    start(); w(0x7200);                     // MOVEQ #0,D1 -- primes Z=1,N=V=C=0 (EORI can't force its input)
             w(0x0A7C); w(0x2700u);
                                    def_sr("EORI SR togSI", 0x2700, 0x0004); // toggles S and I off, Z survives

    // ---- RTR: pops CCR (low byte only) then PC, must leave the S bit
    // (and everything else outside CCR) alone. The frame is written
    // directly into memory from C, with the case's entry SP pointed
    // straight at it, rather than built with 68K "MOVE #imm,-(SP)"
    // pushes, so that the case tests RTR alone.
    //
    // The frame's start address has to be 2-mod-4, not 4-aligned: RTR's
    // ldr for the PC half reads at frame+2 *after* the CCR half's 2-byte
    // pop, and an ARM LDR of a 4-byte value needs a 4-aligned address.
    // The real exception-push path (pjit_svc_entry) gets this for free --
    // it decrements a 4-aligned SSP by 4 (PC, still 4-aligned) then 2
    // more (SR) -- but a hand-built frame here has to replicate it
    // explicitly, or the PC read faults.
    //
    // Checked via pjit_get_sr() (def_sr()), not by an instruction that
    // runs after the RTR's jump: nothing guarantees the ARM condition
    // flags survive the router's lookup code, so an SEQ placed after the
    // jump can't tell "RTR set Z" apart from "something in the
    // redispatch path happened to leave Z set/clear".
    {
        start();
        uint32_t target = (uint32_t)&progmem[base + 1];
        w(0x4E77);                 // RTR
        def_sr("RTR Z-only", 0x2000, 0x2004); // S survives (from init_sr), Z from the popped CCR
        uint8_t *frame = (uint8_t *)&sup_stack[0] + 2;
        cases[ncases - 1].custom_sp = (uint32_t)frame;
        // Plain byte stores, big-endian order -- same bytes write_BE_word/
        // write_BE_long() would produce, without depending on them here.
        frame[0] = 0x00; frame[1] = 0x04;             // CCR half: Z=1, S=0 (ignored)
        frame[2] = (uint8_t)(target >> 24);
        frame[3] = (uint8_t)(target >> 16);
        frame[4] = (uint8_t)(target >> 8);
        frame[5] = (uint8_t)(target);
    }

    // ---- MOVE to/from a predecrement/postincrement A7 destination: real
    // 68K "push"/"pop" via -(SP)/(SP)+, the ordinary way any function that
    // uses its own stack writes it -- as opposed to the RTR case above,
    // which builds its frame directly from C specifically to sidestep this.
    // A7 is the ARM stack pointer (r13) while guest code runs: the destination register must be
    // r13 for A7 and r6..r12 for A0..A6, whichever An the opcode names.
    start();
        w(0x2F3C); w32(0x12345678u);  // MOVE.L #$12345678,-(SP)
        w(0x201F);                    // MOVE.L (SP)+,D0
                                def("A7 -(SP)/(SP)+ .L", 0x2000, 0, 0x12345678u);
    start();
        w(0x3F3C); w(0x1234u);        // MOVE.W #$1234,-(SP)
        w(0x301F);                    // MOVE.W (SP)+,D0
                                // Word ops leave D0's upper 16 bits alone --
                                // POISON's own high half (0xDEAD) must survive.
                                def("A7 -(SP)/(SP)+ .W", 0x2000, 0, 0xDEAD1234u);

    // ---- RESET (supervisor): per the M68000 User's Manual (5.5 "Reset
    // Operation"), executing RESET only pulses the external reset line --
    // it has no effect on the processor's own internal state, and
    // execution simply continues with the next instruction (it must not
    // reload SSP/PC from the vector table, which would reboot the emulated
    // CPU mid-program).
    start(); w(0x4E70); w(0x702A); // RESET ; MOVEQ #$2A,D0
                                def("RESET falls through", 0x2000, 0, 0x0000002Au);

    // ---- RESET immediately followed by JMP: the AmigaOS boot-time idiom.
    // With RESET a true no-op for CPU
    // state, the JMP right after it is reached exactly like any other
    // next instruction -- no prefetch-queue modelling needed for this to
    // work, since RESET never diverts control flow in the first place. A
    // wrong marker placed directly after the JMP (never reached if the
    // JMP itself works -- already covered elsewhere, e.g.
    // smoke_call_exec.c) would catch a regression in either instruction.
    start();
    {
        uint32_t case_base = base;
        w(0x4E70);                                          // RESET
        w(0x4EF9); w32((uint32_t)&progmem[case_base + 5]);  // JMP abs.L -> target
        w(0x7099);                                           // skipped: MOVEQ #$99,D0
        w(0x7055);                                           // target:  MOVEQ #$55,D0
    }
                                def("RESET;JMP idiom", 0x2000, 0, 0x00000055u);

    exec_all();

    printf("[TEST] %d passed, %d failed\n", pass, fail);
    printf(fail ? "FAIL\n" : "PASS\n");
    qemu_exit();
    return 0;
}
