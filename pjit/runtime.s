@ pjit/runtime.s -- the PJIT runtime: the interpreter, exception and interrupt entry and delivery, STOP, the pace governor. Everything in here runs while guest code is
@ running, and none of it is C: once pjit_run() has been entered, no compiler-generated code touches live guest state (docs/design/OVERVIEW.md).
@
@ Interpreter register map:
@     r0-r2   scratch / read data / EA values (r1 = source EA, r2 = dest EA)
@     r3      the opcode, until it has dispatched; scratch after that
@     r4      D0, pinned as in every tier (cpu->d0 is the master copy only while the guest is parked); without PIN_D0 the guest PC while the prefetch handler runs
@     r5      the dispatch table (cpu_t is the sizeof(cpu_t) bytes just below it)
@     r6-r12  A0-A6        sp  A7        lr  prefetch handler (the interrupt hook)
@ The guest PC lives in cpu->ipc; bodies that change it end in pjit_i_chk_pc. Flags are live in CPSR (N Z C V) exactly as in translated code, and X is in the GE flags.
@ Data accesses run E=BE, so guest memory reads and writes are plain ldr/str.

	.include	"ipl.inc"
	.syntax unified
	.arm
	.text
	.include "asm_offsets.inc"

	.equ	MODE_SYS,   0x1F
	.equ	IF_BITS,    0xC0
	.macro	UNMASK t1, t2                   @ clear the I bit if cpu->irq_unmask says so; no flag touched
	ldrb	\t1, [r5, #CPU_IRQ_UNMASK]
	mrs	\t2, cpsr
	bic	\t2, \t2, \t1
	msr	cpsr_c, \t2
	.endm
	.equ	VEC_TRAP15, 47          @ vector_t TRAP15 in pjit.h
	.equ	VEC_STOP,   0x100       @ PJIT_VEC_STOP in pjit.h

@ \rd = a host address inside the DR cache -> the guest address it translates. E=LE.
@ (Also defined in runtime_dr.s, where the cores use it.)
	.macro	HOST2GUEST rd, t1, t2
	ldr	\t1, [r5, #CPU_CACHE_DATA]
	sub	\rd, \rd, \t1
	ldr	\t1, [r5, #CPU_TAGS]
	lsr	\t2, \rd, #DR_HL_SHIFT_
	add	\t1, \t1, \t2, lsl #1
	ldrh	\t1, [\t1]
	lsr	\rd, \rd, #3
	orr	\rd, \rd, \t1, lsl #DR_TAG_SHIFT
	.endm

@ ---- 68K CCR <-> ARM flags -------------------------------------------------
@ SR bits:   X=0x10 N=8 Z=4 V=2 C=1.    CPSR: N=31 Z=30 C=29 V=28.

@ \rs = a 68K SR (or CCR) value; sets the live ARM flags from it. Clobbers \rt.
	.macro	SR_TO_FLAGS rs, rt
	and	\rt, \rs, #0x10                @ X is kept in the GE flags while the guest runs (GE[3:0] = 1111 when X is CLEAR: tools/gen/gen.py save_x)
	eor	\rt, \rt, #0x10
	lsl	\rt, \rt, #12
	rsb	\rt, \rt, \rt, lsl #4
	msr	apsr_g, \rt
	mov	\rt, #0
	tst	\rs, #8
	orrne	\rt, \rt, #0x80000000
	tst	\rs, #4
	orrne	\rt, \rt, #0x40000000
	tst	\rs, #1
	orrne	\rt, \rt, #0x20000000
	tst	\rs, #2
	orrne	\rt, \rt, #0x10000000
	msr	cpsr_f, \rt
	.endm

@ \rs = a CPSR/SPSR value; leaves the 68K X NZVC bits (X = bit 4, from the GE flags) in \rt. Clobbers flags.
	.macro	FLAGS_TO_CCR rs, rt
	mov	\rt, #0
	tst	\rs, #0x80000000
	orrne	\rt, \rt, #8
	tst	\rs, #0x40000000
	orrne	\rt, \rt, #4
	tst	\rs, #0x20000000
	orrne	\rt, \rt, #1
	tst	\rs, #0x10000000
	orrne	\rt, \rt, #2
	tst	\rs, #0x10000                   @ GE0: set when X is clear
	orreq	\rt, \rt, #0x10                @ ... so X is bit 4 of the result
	.endm

@ ---- D0 is pinned in r4 (PJIT_PIN_D0) ----------------------------------------------------
@ Guest code in every tier keeps D0 in r4 (like A0-A7 in r6-r13); cpu->d0 (a big-endian word) is the master copy only while the guest is PARKED: in the exception and
@ interrupt paths below and in the routers' slow paths, which use r4 as scratch. PARK_* stores r4 there, UNPARK_* loads it back just before guest code runs again.
	.ifdef	PJIT_PIN_D0
	.ifndef	PJIT_LATCH
	.error	"PJIT_PIN_D0 needs the latch interrupt model (TIER=1 or BLK=1)"
	.endif
	.macro	PARK_LE t			@ E=LE
	rev	\t, r4
	str	\t, [r5, #CPU_D0]
	.endm
	.macro	UNPARK_BE			@ E=BE
	ldr	r4, [r5, #CPU_D0]
	.endm
	.macro	UNPARK_LE t			@ E=LE
	ldr	\t, [r5, #CPU_D0]
	rev	r4, \t
	.endm
	.endif

@ ---- pjit_run ----------------------------------------------------------------
@ uint32_t pjit_run(uint16_t *pc)       C -> guest
@ Loads the guest from cpu_state, then runs the prefetch loop until the guest
@ executes TRAP #15 (see pjit_i_svc), which stores the guest back and returns.
	.global	pjit_run
	.type	pjit_run, %function
pjit_run:
	push	{r4-r11, lr}            @ the host's callee-saved registers, on the host stack
	mov	r4, r0                  @ guest PC
	movw	r5, #:lower16:(pjit_arena + CPU_SIZE)      @ r5 = the dispatch table
	movt	r5, #:upper16:(pjit_arena + CPU_SIZE)
	str	sp, [r5, #CPU_HOST_SP]  @ (LE: host-side bookkeeping, never read by guest code)
	setend	be                      @ guest data is big-endian from here on. NB: from here a
	                                @ literal-pool load (ldr rX,=sym) reads byte-swapped -- use
	                                @ adr / movw+movt / r5-relative addressing instead.

	sub	r0, r5, #-CPU_A0
	ldmia	r0, {r6-r13}            @ A0-A7; r13 is SP, so the guest stack is live from here
	ldrh	r0, [r5, #CPU_SR]
	SR_TO_FLAGS r0, r1              @ the SR's CCR -> live flags

	mov	r0, r4                  @ the router picks the world: interpretive, or the DR cache
	.ifdef	PJIT_PIN_D0
	UNPARK_BE                       @ (the guest's D0 into r4)
	.endif
	b	pjit_goto

@ ---- extension-word handlers -------------------------------------------------
@ The DR cache bakes each instruction's extension words into its code as
@ constants; here they are read from guest memory as the instruction is
@ executed. The *result* is the same: the source operand's value or address in
@ r1, the destination's in r2, with the guest PC (r4) advanced past the words
@ consumed. (tests compare the two modes; they must agree.)
@
@ Rules every handler follows:
@   - NEVER touch the flags. The guest's N Z C V are live in CPSR between
@     instructions, so no tst/cmp/adds here -- arithmetic only.
@   - never touch LR or the stack: handlers do not return, they tail-jump into
@     the dispatch table, so LR stays = pjit_prefetch for the whole instruction.
@   - r3 holds the opcode (the dispatch needs it); r4 is the guest PC.
@   - scratch is r0 plus whichever of r1/r2 the handler has not produced yet.
@
@ Modes (pjit_ext_table's nibbles, ext_t in pjit.c):
@   1 I16       r = sign-extended word            (abs.W, #imm, (d16,An), ...)
@   2 I32       r = long                          (abs.L, #imm32): two ldrh, since
@                                                  a guest long need only be 2-aligned
@   3 d8,An,Xn  r = d8 + Xn                       (the body adds An)
@   4 d16,PC    r = absolute address of the extension word + d16
@   5 d8,PC,Xn  r = absolute address of the extension word + d8 + Xn
@   6 BDISP     r = d16 - 2                       (Bcc/BRA.W, source only)

.ifdef PJIT_PIN_D0
@ With D0 in r4 the guest PC cannot be kept in a register across the handlers: each macro loads it from cpu->ipc into r0, reads its words through it, and stores it back
@ (r0 is scratch to every macro; \t is the other scratch, as before). The outputs are the same.
	.macro	X_I16 o
	ldr	r0, [r5, #CPU_IPC]
	ldrsh	\o, [r0], #2
	str	r0, [r5, #CPU_IPC]
	.endm
	.macro	X_BDISP o
	ldr	r0, [r5, #CPU_IPC]
	ldrsh	\o, [r0], #2
	str	r0, [r5, #CPU_IPC]
	sub	\o, \o, #2
	.endm
	.macro	X_I32 o
	ldr	r0, [r5, #CPU_IPC]
	ldrh	\o, [r0]
	lsl	\o, \o, #16
	add	r0, r0, #4
	str	r0, [r5, #CPU_IPC]
	ldrh	r0, [r0, #-2]
	orr	\o, \o, r0
	.endm
	.macro	X_D16PC o
	ldr	r0, [r5, #CPU_IPC]
	ldrsh	\o, [r0], #2
	str	r0, [r5, #CPU_IPC]
	add	\o, \o, r0
	sub	\o, \o, #2              @ the PC is already past the word; PC-relative is from the word
	.endm
	.macro	X_SHADOW_A
	str	r4,  [r5, #(CPU_D0)]              @ D0 is pinned: its slot is shadowed too, so an index register can be fetched by number
	str	r6,  [r5, #(CPU_A0 + 0)]
	str	r7,  [r5, #(CPU_A0 + 4)]
	str	r8,  [r5, #(CPU_A0 + 8)]
	str	r9,  [r5, #(CPU_A0 + 12)]
	str	r10, [r5, #(CPU_A0 + 16)]
	str	r11, [r5, #(CPU_A0 + 20)]
	str	r12, [r5, #(CPU_A0 + 24)]
	str	sp,  [r5, #(CPU_A0 + 28)]
	.endm
	.macro	X_INDEX o, t, pc
	ldr	r0, [r5, #CPU_IPC]
	ldrh	\t, [r0], #2            @ the extension word
	str	r0, [r5, #CPU_IPC]
	sxtb	\o, \t                  @ d8
	.if \pc
	add	\o, \o, r0
	sub	\o, \o, #2              @ ... plus the extension word's address
	.endif
	X_SHADOW_A
	lsr	r0, \t, #12             @ register number 0-15
	add	r0, r5, r0, lsl #2
	ldr	r0, [r0, #CPU_D0]       @ its value
	and	\t, \t, #0x800
	eor	\t, \t, #0x800
	lsr	\t, \t, #7              @ 16 for .W, 0 for .L ...
	lsl	r0, r0, \t
	asr	r0, r0, \t              @ ... so this sign-extends the low word for .W and is a no-op for .L
	add	\o, \o, r0
	.endm
	.else
@ \o = the 16-bit extension word, sign-extended.
	.macro	X_I16 o
	ldrsh	\o, [r4], #2
	.endm

@ Bcc/BRA.W: the displacement is measured from the extension word, but the
@ branch cores add it to the address *after* it.
	.macro	X_BDISP o
	ldrsh	\o, [r4], #2
	sub	\o, \o, #2
	.endm

@ \o = a 32-bit extension (two words, high word first). Scratch: r0.
	.macro	X_I32 o
	ldrh	r0, [r4], #2
	ldrh	\o, [r4], #2
	orr	\o, \o, r0, lsl #16
	.endm

@ \o = (d16,PC): the extension word's own address + d16.
	.macro	X_D16PC o
	ldrsh	\o, [r4], #2
	add	\o, \o, r4
	sub	\o, \o, #2              @ r4 is already past the word; PC-relative is from the word
	.endm

@ Shadow A0-A7 (r6-r13) into the register-file array next to D0-D7, so an index
@ register can be fetched by number. Plain stores: no base register, no flags.
	.macro	X_SHADOW_A
	str	r6,  [r5, #(CPU_A0 + 0)]
	str	r7,  [r5, #(CPU_A0 + 4)]
	str	r8,  [r5, #(CPU_A0 + 8)]
	str	r9,  [r5, #(CPU_A0 + 12)]
	str	r10, [r5, #(CPU_A0 + 16)]
	str	r11, [r5, #(CPU_A0 + 20)]
	str	r12, [r5, #(CPU_A0 + 24)]
	str	sp,  [r5, #(CPU_A0 + 28)]
	.endm

@ \o = d8 + Xn from a brief extension word (68000: d8 and a D/A register, W or L;
@ no scale, no full format). \pc = 1 adds the extension word's address first.
@ Scratch: r0 and \t.
@   bit 15 = A/D, bits 14:12 = register, so (word >> 12) indexes D0-D7, A0-A7 as
@   one array; bit 11 clear = the register's low word, sign-extended.
	.macro	X_INDEX o, t, pc
	ldrh	r0, [r4], #2
	sxtb	\o, r0                  @ d8
	.if \pc
	add	\o, \o, r4
	sub	\o, \o, #2              @ ... plus the extension word's address
	.endif
	X_SHADOW_A
	lsr	\t, r0, #12             @ register number 0-15
	add	\t, r5, \t, lsl #2
	ldr	\t, [\t, #CPU_D0]       @ its value
	and	r0, r0, #0x800
	eor	r0, r0, #0x800
	lsr	r0, r0, #7              @ 16 for .W, 0 for .L ...
	lsl	\t, \t, r0
	asr	\t, \t, r0              @ ... so this sign-extends the low word for .W and is a no-op for .L
	add	\o, \o, \t
	.endm

	.endif

@ \m = a mode number above; result in \o; scratch r0 and \t.
	.macro	X_EA m, o, t
	.if \m == 1
	X_I16	\o
	.elseif \m == 2
	X_I32	\o
	.elseif \m == 3
	X_INDEX	\o, \t, 0
	.elseif \m == 4
	X_D16PC	\o
	.elseif \m == 5
	X_INDEX	\o, \t, 1
	.elseif \m == 6
	X_BDISP	\o
	.endif
	.endm

@ One handler per (source mode \s, destination mode \d) pair: the source goes to
@ r1, the destination to r2, then straight into the dispatch table.
	.macro	X_HANDLER s, d
pjit_x_\s\()_\d:
	.if \s != 0
	X_EA	\s, r1, r2              @ r2 is not produced yet: free scratch
	.endif
	.if \d != 0
	.if (\s != 0) && ((\d == 3) || (\d == 5))
	@ A destination index mode needs a second temporary; r1 holds the source
	@ and r3 the opcode, so borrow r3 for the duration.
	str	r3, [r5, #CPU_SCRATCH]
	X_EA	\d, r2, r3
	ldr	r3, [r5, #CPU_SCRATCH]
	.else
	X_EA	\d, r2, r1              @ r1 is free if there was no source word
	.endif
	.endif
	.ifndef	PJIT_PIN_D0
	str	r4, [r5, #CPU_IPC]      @ the PC past the extension words
	.endif
	add	pc, r5, r3, lsl #4      @ -> the DT; no return (cpu->ipc is the PC past the extension words)
	.endm

	.irp	sm, 0, 1, 2, 3, 4, 5, 6
	.irp	dm, 0, 1, 2, 3, 4, 5
	.if (\sm != 0) || (\dm != 0)
	X_HANDLER \sm, \dm
	.endif
	.endr
	.endr

@ ---- the prefetch handler ----------------------------------------------------
@ Fetch the opcode, then let the opcode's extension class (pjit_ext_table:
@ (source mode << 4) | destination mode, 0 = none) pick a handler that leaves
@ r1/r2 as the DR cache's prefix would and tail-jumps into the dispatch table.
@ No handler returns and none touches LR, so LR stays = pjit_prefetch for the
@ whole instruction; the instruction's own final `bx lr` is the way back (and
@ the place an interrupt can take over).
	.global	pjit_prefetch
	.type	pjit_prefetch, %function
@ Profiling twin of pjit_prefetch (select it with cpu->prefetch_hook): counts each opcode in
@ the weak table pjit_hist[65536] (big-endian words) before executing it. Tests only.
@ And a second one that records the instruction stream into the weak ring buffer pjit_trace (512KB of
@ big-endian 8-byte entries: the guest PC, then the opcode; pjit_trace_pos is the byte offset of the next
@ entry). For tools/study/pairstat.py and tools/data/cycles68k.py.
	.weak	pjit_trace
	.weak	pjit_trace_pos
	.global	pjit_prefetch_trace
pjit_prefetch_trace:
	.ifdef	PJIT_PIN_D0
	str	r4, [r5, #CPU_D0]               @ (test-only hook: D0 is lent to the slot while it uses r4 as before, and put back)
	.endif
	ldr	r4, [r5, #CPU_IPC]
	ldrh	r3, [r4]
	movw	r0, #:lower16:pjit_trace_pos
	movt	r0, #:upper16:pjit_trace_pos
	ldr	r1, [r0]
	movw	r2, #:lower16:pjit_trace
	movt	r2, #:upper16:pjit_trace
	str	r4, [r2, r1]
	add	r1, r1, #4
	str	r3, [r2, r1]
	add	r1, r1, #4
	bic	r1, r1, #0x80000                @ wraps at 512KB
	str	r1, [r0]
	.ifdef	PJIT_PIN_D0
	ldr	r4, [r5, #CPU_D0]
	.endif
	b	pjit_prefetch

	@ A longer trace that stops when it is full instead of wrapping: pjit_trace2 (8 bytes an instruction: PC, opcode), pjit_trace2_pos (bytes so far), pjit_trace2_max.
	.weak	pjit_trace2
	.weak	pjit_trace2_pos
	.weak	pjit_trace2_max
	.global	pjit_prefetch_trace2
pjit_prefetch_trace2:                   @ (the guest's flags are live here: nothing below sets any)
	.ifdef	PJIT_PIN_D0
	str	r4, [r5, #CPU_D0]
	.endif
	ldr	r4, [r5, #CPU_IPC]
	ldrh	r3, [r4]
	movw	r0, #:lower16:pjit_trace2_pos
	movt	r0, #:upper16:pjit_trace2_pos
	ldr	r1, [r0]
	movw	r2, #:lower16:pjit_trace2_max
	movt	r2, #:upper16:pjit_trace2_max
	ldr	r2, [r2]
	sub	r2, r1, r2
	lsr	r2, r2, #31                     @ 1 while there is room
	add	pc, pc, r2, lsl #2              @ pc reads as .+8: the entry below the nop
	nop
	.ifdef	PJIT_PIN_D0
	b	1f                              @ full (one word: the computed jump's entry)
	.else
	b	pjit_prefetch                   @ full
	.endif
	movw	r2, #:lower16:pjit_trace2
	movt	r2, #:upper16:pjit_trace2
	str	r4, [r2, r1]
	add	r1, r1, #4
	str	r3, [r2, r1]
	add	r1, r1, #4
	str	r1, [r0]
	.ifdef	PJIT_PIN_D0
1:	ldr	r4, [r5, #CPU_D0]
	.endif
	b	pjit_prefetch

	.weak	pjit_hist
	.global	pjit_prefetch_count
pjit_prefetch_count:
	.ifdef	PJIT_PIN_D0
	str	r4, [r5, #CPU_D0]
	.endif
	ldr	r4, [r5, #CPU_IPC]
	ldrh	r3, [r4]
	movw	r0, #:lower16:pjit_hist
	movt	r0, #:upper16:pjit_hist
	add	r0, r0, r3, lsl #2
	ldr	r1, [r0]
	add	r1, r1, #1
	str	r1, [r0]
	.ifdef	PJIT_PIN_D0
	ldr	r4, [r5, #CPU_D0]
	.endif
	b	pjit_prefetch
	.ifdef	PJIT_LATCH
	.ifdef	PJIT_PACE
	.global	pjit_prefetch_pace
	.weak	pjit_pace_tab
	.weak	pjit_pace_k
	.weak	pjit_pace_dl
	.weak	pjit_pace_gap
	.weak	pjit_pace_wraps
	.weak	pjit_pace_mb
pjit_prefetch_pace:                     @ the pace governor's hook (cpu->prefetch_hook, chosen at start-up by the platform): the code below runs first, then it falls into the poll hook's own body
	str	r4, [r5, #CPU_D0]               @ the 68000 PACE GOVERNOR (debug / compatibility build): the guest runs against a virtual MC68000 clock. D0 is parked (r4 is scratch here), the opcode's cycle estimate
	ldr	r4, [r5, #CPU_IPC]              @ (pjit_pace_tab, one byte per opcode word) times pjit_pace_k (ARM cycles per 68000 cycle) advances the virtual clock pjit_pace_dl[0]; the instruction waits
	ldrh	r3, [r4]                        @ until the PMU cycle counter has caught up with the clock as it stood after the PREVIOUS instruction; more than ~20 us behind = resync, never a catch-up burst.
	movw	r2, #:lower16:pjit_pace_tab     @ N Z V C and GE (X) are live between guest instructions: saved in pjit_pace_dl[1] around the wait and restored. pjit_pace_dl[2] = the guest PC of the previous
	movt	r2, #:upper16:pjit_pace_tab     @ instruction: a gap of more than ~134 ms between two instructions is logged in pjit_pace_gap (count, then {gap in ns, that instruction's PC} x 16): what stalled.
	ldrb	r3, [r2, r3]
	movw	r2, #:lower16:pjit_pace_k
	movt	r2, #:upper16:pjit_pace_k
	ldr	r2, [r2]
	mul	r3, r3, r2
	lsr	r0, r4, #20                     @ an UNPACED megabyte (pjit_pace_mb, one byte per guest MB, set by the platform for a zone with the F flag: Fast RAM on the bus): run at the interpreter's own speed
	movw	r2, #:lower16:pjit_pace_mb      @ (bit 31 of the cycle product carries the flag past the flag-free part of the hook)
	movt	r2, #:upper16:pjit_pace_mb
	ldrb	r0, [r2, r0]
	orr	r3, r3, r0, lsl #31
	movw	r1, #:lower16:pjit_pace_dl
	movt	r1, #:upper16:pjit_pace_dl
	ldr	r0, [r1]
	mrs	r2, cpsr
	str	r2, [r1, #4]
	tst	r3, #0x80000000
	mrcne	p15, 0, r0, c9, c13, 0          @ unpaced: no wait; the virtual clock follows the real one, so the paced code after it resumes without a lag burst
	bne	4f
1:	mrc	p15, 0, r4, c9, c13, 0
	subs	r4, r4, r0                      @ now - clock
	bge	2f                              @ at or past the clock: go
	cmn	r4, #0x18000
	bge	1b                              @ a short way ahead (under ~98 us, an instruction's worth): wait
	add	r0, r0, r4                      @ implausibly far ahead: the guest was away for over half a wrap of the 32-bit counter (2.1 s), the signed difference flipped: resync (r4 = the gap, unsigned)
	movw	r2, #:lower16:pjit_pace_wraps
	movt	r2, #:upper16:pjit_pace_wraps
	ldr	r3, [r2]
	add	r3, r3, #1
	str	r3, [r2]
	b	5f
2:	cmp	r4, #0x5000
	ble	3f
	cmp	r4, #0x1000000
	bgt	7f                              @ over ~17 ms behind is idle time (STOP waiting for a frame, a disk wait), not slowness: resync without counting it in the lag statistics
	ldr	r2, [r1, #12]                   @ lag statistics (pjit_pace_dl[3..5], flags are saved): total us behind, number of resyncs, worst lag in ns: is the guest slower than the virtual 68000?
	add	r2, r2, r4, lsr #10
	str	r2, [r1, #12]
	ldr	r2, [r1, #16]
	add	r2, r2, #1
	str	r2, [r1, #16]
	ldr	r2, [r1, #20]
	cmp	r4, r2
	strhi	r4, [r1, #20]
	ldrhi	r2, [r5, #CPU_IPC]              @ the PC of the instruction that took the worst lag, and the number of lags over 1 ms (about a video line's worth of frames lost every ~20 of them)
	strhi	r2, [r1, #24]
	movw	r2, #0x4240
	movt	r2, #0x000F
	cmp	r4, r2
	ldrhi	r2, [r1, #28]
	addhi	r2, r2, #1
	strhi	r2, [r1, #28]
	add	r0, r0, r4                      @ more than ~20 us behind: resync, no catch-up burst
	cmp	r4, #0x1000000
	bgt	5f                              @ ... and more than ~17 ms (a lost frame): log it (a nasty-blitter lockout of the bus is ~0.7 ms and routine)
7:	add	r0, r0, r4
	b	5f
3:	add	r0, r0, r3
4:	str	r0, [r1]
	ldr	r4, [r5, #CPU_IPC]
	str	r4, [r1, #8]
	ldr	r2, [r1, #4]
	msr	cpsr_fs, r2
	ldr	r4, [r5, #CPU_D0]
	b	6f
5:	movw	r2, #:lower16:pjit_pace_gap     @ log: count, then the entry (gap, previous instruction's PC)
	movt	r2, #:upper16:pjit_pace_gap
	ldr	r3, [r2]
	add	r3, r3, #1
	str	r3, [r2]
	and	r3, r3, #15
	add	r2, r2, r3, lsl #3
	str	r4, [r2, #8]
	ldr	r3, [r1, #8]
	str	r3, [r2, #12]
	b	4b
6:
	.endif
	.global	pjit_prefetch_poll
pjit_prefetch_poll:                     @ the latch model's prefetch hook (cpu->prefetch_hook): one flag test per interpreted instruction; the flag set sends the PC to the router, which polls
	.ifdef	PJIT_ITRACE
	.weak	pjit_trace
	.weak	pjit_trace_pos
	.weak	pjit_trace_frz
	str	r4, [r5, #CPU_D0]               @ debug (PJIT_ITRACE): the (PC, opcode) ring of pjit_prefetch_trace, kept by the poll hook so interrupts still work; it stops advancing once
	ldr	r4, [r5, #CPU_IPC]              @ pjit_trace_frz is 8 (the exception delivery sets it for an illegal instruction outside the ROM), so the trail into the crash survives
	ldrh	r3, [r4]
	movw	r0, #:lower16:pjit_trace_pos
	movt	r0, #:upper16:pjit_trace_pos
	ldr	r1, [r0]
	movw	r2, #:lower16:pjit_trace
	movt	r2, #:upper16:pjit_trace
	str	r4, [r2, r1]
	add	r1, r1, #4
	str	r3, [r2, r1]
	add	r1, r1, #4
	movw	r2, #:lower16:pjit_trace_frz
	movt	r2, #:upper16:pjit_trace_frz
	ldrb	r3, [r2]
	sub	r1, r1, r3
	bic	r1, r1, #0x80000
	str	r1, [r0]
	ldr	r4, [r5, #CPU_D0]
	.endif
	ldrb	r0, [r5, #CPU_IRQ_FLAG]
	add	pc, pc, r0, lsl #2              @ pc reads as .+8: the entry below the nop
	nop
	b	pjit_prefetch
	ldr	r0, [r5, #CPU_IPC]
	.ifdef	PJIT_TIER
	b	pjit_goto_slow
	.else
	b	pjit_blk_slow
	.endif
	.endif
	.global	pjit_prefetch_irq
pjit_prefetch_irq:
	cpsie	i                       @ the interrupt window: the guest is between instructions
	nop                             @ (an IRQ taken here is delivered by pjit_irq_deliver)
	cpsid	i
pjit_prefetch:
	.ifdef	PJIT_PIN_D0
	ldr	r2, [r5, #CPU_IPC]              @ (r4 is D0: the PC lives in cpu->ipc between the handlers)
	ldrh	r3, [r2], #2            @ the opcode; r2 now = the first extension word
	add	r0, r5, #ARENA_EXT
	ldrb	r0, [r0, r3]
	str	r2, [r5, #CPU_IPC]
	.else
	ldr	r4, [r5, #CPU_IPC]
	ldrh	r3, [r4], #2            @ the opcode; r4 now = the first extension word
	add	r0, r5, #ARENA_EXT      @ the table sits just after the DT (a literal load would swap)
	ldrb	r0, [r0, r3]            @ extension class
	str	r4, [r5, #CPU_IPC]      @ (class 0: this is the PC the body sees; the handlers store it again past their words)
	.endif
	add	pc, pc, r0, lsl #2      @ pc reads as .+8: the entry below the nop
	nop
	add	pc, r5, r3, lsl #4      @ class 0: no extension words -> the DT, four words per opcode
	@ the rest of the table, indexed (source mode << 4) | destination mode;
	@ entry 0 is the line above. Modes above 6 / 5 never occur.
	.irp	sm, 0, 1, 2, 3, 4, 5, 6
	.irp	dm, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
	.if (\sm == 0) && (\dm == 0)
	.elseif \dm <= 5
	b	pjit_x_\sm\()_\dm
	.else
	b	pjit_x_unimplemented
	.endif
	.endr
	.endr

pjit_x_unimplemented:
	bkpt	#2

@ ---- the guest PC must be even -------------------------------------------------
@ Every interpretive body that changes the guest PC (Bcc, BRA, BSR, DBcc, JMP,
@ JSR, RTS, RTR, RTE) ends here instead of `bx lr`. An odd PC raises an address
@ error on the instruction fetch, as on a real 68000; otherwise this is just the
@ instruction's `bx lr`, so LR is still the prefetch handler / interrupt hook.
@ The test is flag-free (the guest's N Z C V are live): the parity picks one of
@ the two words below the nop.
	.global	pjit_i_chk_pc
	.type	pjit_i_chk_pc, %function
pjit_i_chk_pc:
	.ifdef	PJIT_PIN_D0
	str	r0, [r5, #CPU_IPC]      @ the new PC (r0; r4 is D0)
	and	r1, r0, #1
	add	pc, pc, r1, lsl #2      @ pc reads as .+8: the entry below the nop
	nop
	bx	lr                      @ even
	mov	r2, #1                  @ odd: r0 = the fault address,
	str	r2, [r5, #(CPU_SPILL + 28)]     @ (tell the exception entry cpu->ipc already holds the PC)
	mov	r1, #0x18               @ r1 = program space, read (see pjit_i_deliver)
	svc	#VEC_ADDRESSERR
	.else
	str	r4, [r5, #CPU_IPC]      @ the new PC
	and	r0, r4, #1
	add	pc, pc, r0, lsl #2      @ pc reads as .+8: the entry below the nop
	nop
	bx	lr                      @ even
	mov	r0, r4                  @ odd: r0 = the fault address,
	mov	r2, #1
	str	r2, [r5, #(CPU_SPILL + 28)]     @ (tell the exception entry r4 already holds the PC)
	mov	r1, #0x18               @ r1 = program space, read (see pjit_i_deliver)
	svc	#VEC_ADDRESSERR
	.endif

@ ---- interpretive-mode exceptions ----------------------------------------------
@ Reached from the SVC vector with r0-r12 still the guest's, SVC-mode sp/lr free
@ scratch, and E=LE. `svc #n` is how every emitted sequence raises 68K vector n
@ (TRAP, CHK, DIVZ, TRAPV, privilege, illegal, line A/F). r4 is the guest PC:
@ just past the instruction that raised it, so a frame's PC is exact for
@ multi-word instructions too.
@
@ Register use from here: r3 = the vector. The guest's flags are saved into the
@ SR in cpu_state first (the SPSR holds them), so nothing below depends on CPSR.
	.equ	CPU_R1,       CPU_R0 + 4
	.equ	VEC_BUSERR,   2
	.equ	VEC_ADDRESSERR, 3
	.equ	VEC_TRAP0,    32
	.equ	VEC_TRAP13,   45
	.equ	VEC_TRAP14,   46

@ ---- abort to exception ---------------------------------------------------------------------------------------------------------
@ A data abort in guest code that the platform cannot (or need not) emulate becomes a 68K bus error (vector 2) or address error (vector 3): the platform's abort handler
@ (platform/buffee/buffee.c buffee_guest_abort) patches the saved registers so that the exception return lands HERE in System mode, with the guest's registers and flags back,
@ r0 = the fault address, r1 = the function-code hint (bit 3 = program space, bit 4 = read, as pjit_i_chk_pc passes it), cpu->ipc = the guest PC and cpu->spill[7] = 1
@ (the entry below then takes the PC from cpu->ipc). The svc is a normal one from here on: the same frame, the same handler dispatch.
	.global	pjit_abort_bus
	.global	pjit_abort_addr
pjit_abort_bus:
	svc	#VEC_BUSERR
pjit_abort_addr:
	svc	#VEC_ADDRESSERR

@ The SVC vector (boot.S) lands here, in SVC mode. r0-r12 still hold live guest
@ state; only the SVC-banked sp and lr are free scratch. Interpretive and cached
@ code raise exceptions the same way; the code below tells them apart.
	.global	pjit_svc_entry
	.type	pjit_svc_entry, %function
	.global	pjit_exc_begin
pjit_exc_begin:
pjit_svc_entry:
pjit_i_svc:
	setend	le
	sub	sp, lr, #4              @ the SVC instruction's address ...
	str	sp, [r5, #CPU_LR]       @ ... parked (the exception code asks where it ran)
	ldr	sp, [sp]                @ the instruction itself ...
	bic	sp, sp, #0xFF000000     @ ... whose immediate is the 68K vector
	@ r0-r3 are scratch between instructions, but an exception frame for an
	@ address/bus error reads the faulting address and function code from them.
	strd	r0, r1, [r5, #CPU_R0]
	strd	r2, r3, [r5, #CPU_R2]
	str	r12, [r5, #CPU_R12]
	.ifdef	PJIT_PIN_D0
	PARK_LE	r0                      @ D0 (r4) into cpu->d0: from here r4 is scratch until the guest runs again
	.endif
	mov	r3, sp                  @ r3 = the vector
	mrs	r0, spsr                @ the guest's live flags, as of the svc
	FLAGS_TO_CCR r0, r1
	ldrb	r2, [r5, #(CPU_SR + 1)] @ the SR's low byte (X, from the GE flags, comes with r1 now)
	bic	r2, r2, #0x1F
	orr	r2, r2, r1
	cmp	r3, #8                  @ a privilege violation: its S-bit test (tst) has already changed N Z C, but the sequence loaded the SR image into r2 before it:
	ldrbeq	r1, [r5, #CPU_R2]       @ the frame's CCR is that image's (a 68000 leaves the flags alone)
	andeq	r1, r1, #0x1F
	biceq	r2, r2, #0x1F
	orreq	r2, r2, r1
	strb	r2, [r5, #(CPU_SR + 1)] @ (the SR in memory is now current)
	msr	cpsr_c, #(MODE_SYS | IF_BITS)   @ the guest's world: its registers, SP and LR

	@ The guest PC for the frame. Interpretive code has it in r4. Cached code does
	@ not: from the address of the svc (a leader in a cache slot) or the guest's LR (a
	@ body that cached code `bl`'d into) it is worked out through the line's tag.
	ldr	r0, [r5, #(CPU_SPILL + 28)]     @ the router / pjit_i_chk_pc already put it in cpu->ipc
	cmp	r0, #0
	movne	r1, #0
	strne	r1, [r5, #(CPU_SPILL + 28)]
	bne	pjit_i_pc_mem
	ldr	r0, [r5, #CPU_LR]               @ where the svc ran
	ldr	r1, [r5, #CPU_CACHE_DATA]
	sub	r2, r0, r1
	cmp	r2, #DR_CACHE_BYTES
	blo	pjit_i_pc_leader                @ in the cache: a leader (TRAP, ILLEGAL, ...)
	sub	r2, lr, r1                      @ not in the cache: a shared body. Whose?
	cmp	r2, #DR_CACHE_BYTES
	bhs	pjit_i_pc_mem                   @ lr is not in the cache either: interpretive or a block, the PC is in cpu->ipc
	add	r0, lr, #15                     @ cached: the body was `bl`'d into, so lr is in the guest instruction's last slot,
	bic	r0, r0, #15                     @ (rounded up: the instruction's end) in the cache
	HOST2GUEST r0, r1, r2
	mov	r4, r0
	b	1f
pjit_i_pc_mem:
	ldr	r4, [r5, #CPU_IPC]
	rev	r4, r4
	b	1f
pjit_i_pc_leader:                       @ the svc is the instruction's leader, in its last slot:
	HOST2GUEST r0, r1, r2                   @ that slot's guest word + 2 = the end of the instruction
	bic	r0, r0, #1
	add	r4, r0, #2
1:	str	r4, [r5, #CPU_PC]       @ the guest PC, just past the instruction

	cmp	r3, #0x104              @ a step request (the differential harness's prefetch hook, tests/qemu/amiga_hook.S): the guest is parked, pjit_run() returns 0x104
	beq	pjit_i_leave
	cmp	r3, #VEC_STOP           @ STOP: with interrupts on it waits for one; otherwise it ends pjit_run()
	bne	1f
	.ifdef	PJIT_LATCH
	ldrb	r0, [r5, #CPU_IRQ_UNMASK]       @ (the latch model has no windows: interrupts are on when irq_unmask says so)
	.else
	ldr	r0, [r5, #CPU_IRQ_WINDOWS]
	.endif
	cmp	r0, #0
	beq	pjit_i_leave
	b	pjit_i_stop
1:
	@ TRAP #n with its bit set in cpu->trap_exit_mask ends pjit_run().
	sub	r0, r3, #VEC_TRAP0
	cmp	r0, #15
	bhi	pjit_i_deliver
	ldr	r1, [r5, #CPU_TRAP_EXIT_MASK]
	lsr	r1, r1, r0
	tst	r1, #1
	bne	pjit_i_leave
	cmp	r3, #VEC_TRAP13         @ host fast paths: D0 = the timer frequency / count
	beq	pjit_i_trap13
	cmp	r3, #VEC_TRAP14
	beq	pjit_i_trap14
	b	pjit_i_deliver

@ Test exit: put the guest back in cpu_state and return to pjit_run()'s caller.
pjit_i_leave:
	msr	cpsr_c, #(MODE_SYS | IF_BITS)   @ System mode: r13 is the guest SP again
	setend	be
	sub	r0, r5, #-CPU_A0
	stmia	r0, {r6-r13}            @ A0-A7 back to their shadows
	setend	le
	ldr	sp, [r5, #CPU_HOST_SP]
	mov	r0, r3                  @ pjit_run()'s result: the vector
	pop	{r4-r11, pc}            @ the host's registers; back to the C caller

@ TRAP #13 / #14: not exceptions, host services for the timing tests.
pjit_i_trap13:
	.ifdef	PJIT_A8
	movw	r0, #:lower16:PJIT_A8_HZ        @ no generic timer on a Cortex-A8: the CPU clock, which the cycle counter counts
	movt	r0, #:upper16:PJIT_A8_HZ
	.else
	mrc	p15, 0, r0, c14, c0, 0  @ CNTFRQ
	.endif
	b	pjit_i_set_d0
pjit_i_trap14:
	.ifdef	PJIT_A8
	mrc	p15, 0, r0, c9, c13, 0  @ PMCCNTR: the cycle counter (platform/beaglebone starts it)
	.else
	mrrc	p15, 0, r0, r1, c14     @ CNTPCT: r0 = the low word
	.endif
pjit_i_set_d0:
	rev	r0, r0
	str	r0, [r5, #CPU_D0]       @ D0 is big-endian in memory
pjit_i_resume_next:
	setend	be
	ldrb	r0, [r5, #(CPU_SR + 1)] @ resume after the TRAP with the guest's flags
	SR_TO_FLAGS r0, r1
	mov	r0, r4
	.ifdef	PJIT_PIN_D0
	UNPARK_BE
	.endif
	b	pjit_goto

@ ---- interrupts ------------------------------------------------------------------
@ Guest code runs with the host's IRQ masked, except in short *windows* at instruction
@ boundaries: the interpreter's prefetch handler, the cache router (pjit_goto_parked)
@ and the loop core (pjit_d_loop, which every backward branch inside a cache line goes
@ through, so no loop can starve an interrupt). Between instructions r0-r3 are free and
@ the guest PC is known, so an IRQ taken in a window can be delivered exactly:
@
@   pjit_irq_entry (IRQ mode): the platform hook acknowledges the device and says which
@   68K level it posts; the highest posted level is kept in cpu->irq_level. If the SR's
@   mask lets it through (level 7 always does), the interrupted window is abandoned and
@   execution resumes at pjit_irq_deliver instead; otherwise it just resumes, and the
@   level stays posted for pjit_irq_sr_i (run by the SR-writing instructions).
@
@   pjit_irq_deliver (System mode, IRQ masked): the guest PC goes in r4, the flags are
@   parked, and the autovector (24 + level) is delivered like any other exception.
	.ifdef	PJIT_LATCH
@ The block runtime (pjit/blk.s, docs/design/BLOCK_RUNTIME.md section 7) has no windows: IRQs stay enabled in guest code and this handler only LATCHES. It runs in IRQ mode on its own
@ banked stack, saves what it touches, asks the platform which 68K level the device posts (cpu->irq_level keeps the highest), and if that level beats the SR mask sets
@ cpu->irq_flag and empties the router's fast table (blk_fast), so the next dynamic transfer misses and enters the router, which polls. It returns with `pop {..., pc}^`,
@ which restores CPSR from SPSR (N Z C V, GE = X, E): the interrupted code never knows. The guest polls at the router, at backward exit stubs and after an interpreted instruction.
	.global	pjit_irq_entry
	.type	pjit_irq_entry, %function
pjit_irq_entry:
	sub	lr, lr, #4
	push	{r0-r3, r12, lr}
	bl	pjit_platform_irq               @ r0 = the level the device pins show now (0 = none); may clobber r0-r3
	ldr	r1, [r5, #CPU_IRQ_LEVEL]        @ level 8 = the platform's RESET (the Amiga's reset line fell): kept until it is delivered
	cmp	r1, #8
	moveq	r0, r1
	mov	r1, r0                          @ the pins ARE the state (a 68000 samples IPL2-0): a withdrawn request vanishes, a held one stays
	str	r1, [r5, #CPU_IRQ_LEVEL]
	cmp	r1, #0
	beq	2f
	cmp	r1, #7
	beq	1f
	ldrb	r2, [r5, #CPU_SR]               @ the SR's high byte (big-endian in memory)
	and	r2, r2, #7
	cmp	r1, r2
	bls	2f                              @ masked: stays posted, the SR-writing instructions look again
1:	mov	r2, #1
	str	r2, [r5, #CPU_IRQ_FLAG]
	.ifdef	PJIT_TIER
	ldr	r2, [r5, #CPU_TAGS_MISS]        @ the slot router's fast path misses from now on: its slow path polls
	str	r2, [r5, #CPU_TAGS_GOTO]
	.endif
	bl	pjit_bk_unlink                  @ chained loops reach the router within an iteration
	bl	pjit_blk_fast_clear             @ every entry {an odd key, the slow path}: see blk.s (a zeroed entry matched a transfer to guest address 0)
2:	ldmfd	sp!, {r0-r3, r12, pc}^
	.else
	.global	pjit_irq_entry
	.type	pjit_irq_entry, %function
pjit_irq_entry:
	sub	lr, lr, #4
	str	lr, [r5, #CPU_IRQ_SITE]         @ where the window was interrupted
	bl	pjit_platform_irq               @ r0 = the level the device pins show now (0 = none)
	ldr	r1, [r5, #CPU_IRQ_LEVEL]
	cmp	r1, #8
	moveq	r0, r1
	mov	r1, r0
	str	r1, [r5, #CPU_IRQ_LEVEL]
	cmp	r1, #0
	beq	2f
	cmp	r1, #7
	beq	1f                              @ level 7 is not maskable
	ldrb	r2, [r5, #CPU_SR]               @ the SR's high byte (big-endian in memory)
	and	r2, r2, #7
	cmp	r1, r2
	bls	2f                              @ masked: stays posted
1:	mrs	r0, spsr
	orr	r0, r0, #0x80
	msr	spsr_c, r0                      @ come back with the IRQ masked
	movw	lr, #:lower16:pjit_irq_deliver
	movt	lr, #:upper16:pjit_irq_deliver
	movs	pc, lr
2:	ldr	lr, [r5, #CPU_IRQ_SITE]
	movs	pc, lr                          @ back into the window

	.endif

@ An SR-writing instruction found an interrupt posted (cpu->irq_level != 0): the new mask may
@ let it through. The SR in memory is current; the flags are not (they are rebuilt from its CCR).
@ Three ways in, by where the guest PC is: r4 (interpretive), lr (a cached body that was
@ `bl`'d into: the instruction's end in the cache) or r0 (RTE in the cache).
	.global	pjit_irq_sr_i
pjit_irq_sr_i:
	setend	le
	.ifdef	PJIT_PIN_D0
	PARK_LE	r1
	.endif
	ldr	r0, [r5, #CPU_IPC]
	rev	r0, r0
	b	pjit_irq_poll
	.global	pjit_irq_sr_d
pjit_irq_sr_d:
	setend	le
	.ifdef	PJIT_PIN_D0
	PARK_LE	r1
	.endif
	add	r0, lr, #15                     @ lr is in the instruction's last slot: round up to the next instruction
	bic	r0, r0, #15
	HOST2GUEST r0, r1, r2
	b	pjit_irq_poll
	.global	pjit_irq_sr_r0
pjit_irq_sr_r0:
	setend	le
	.ifdef	PJIT_PIN_D0
	PARK_LE	r1
	.endif
pjit_irq_poll:                          @ E=LE, r0 = the guest PC
	mov	r4, r0
	rev	r1, r0
	str	r1, [r5, #CPU_IPC]
	ldrb	r1, [r5, #(CPU_SR + 1)]
	SR_TO_FLAGS r1, r2
	mrs	r1, cpsr
	str	r1, [r5, #CPU_JMP_CPSR]         @ the guest's flags, parked
	ldrb	r1, [r5, #CPU_IRQ_LEVEL]
	IRQ_VERIFY r1, r2, r3
	ldrb	r2, [r5, #CPU_SR]
	and	r2, r2, #7
	cmp	r1, #7
	beq	1f
	cmp	r1, r2
	bls	2f                              @ still masked: carry on
1:	mov	r1, #0
	str	r1, [r5, #CPU_IRQ_SITE]         @ (not the loop core)
	b	pjit_irq_deliver
2:	mov	r0, r4
	b	pjit_goto_parked

@ STOP with interrupts enabled: the guest has set its SR and now waits. The wait is an interrupt window
@ (WFI between CPSIE and CPSID); an interrupt that the SR's mask lets through is delivered, with the
@ PC after the STOP, exactly as at any other boundary; one that it holds back leaves the wait going.
@ (Not reached by the SR-writing instructions' own poll: STOP's SR write is done by its body.)
pjit_i_stop:
	mov	r0, r4
pjit_i_stop_poll:                       @ E=LE, r0 = the PC after the STOP
	mov	r4, r0
	rev	r1, r0
	str	r1, [r5, #CPU_IPC]
	ldrb	r1, [r5, #(CPU_SR + 1)]
	SR_TO_FLAGS r1, r2
	mrs	r1, cpsr
	str	r1, [r5, #CPU_JMP_CPSR]
	ldrb	r1, [r5, #CPU_IRQ_LEVEL]
	ldrb	r2, [r5, #CPU_SR]
	and	r2, r2, #7
	cmp	r1, #7
	beq	1f
	cmp	r1, r2
	bhi	1f
	@ Sleep with the host IRQ still MASKED: a pending interrupt ends a WFI whatever the mask, and is not taken yet; it is taken in the short unmasked window below, its handler
	@ latches the level, and the poll above looks again. (`cpsie i ; wfi` was a lost wake-up: an interrupt already pending is taken AT the cpsie, the handler latches the level, and
	@ the WFI then sleeps with a deliverable interrupt in the latch until the next IPL edge: exec's idle loop stalled, ~2% of the boots.)
	wfi
	cpsie	i
	isb                                     @ (a pending IRQ is taken here)
	cpsid	i
	mov	r0, r4
	b	pjit_i_stop_poll
1:	mov	r1, #0
	str	r1, [r5, #CPU_IRQ_SITE]
	b	pjit_irq_deliver

	.global	pjit_irq_deliver
	.type	pjit_irq_deliver, %function
pjit_irq_deliver:
	mrs	r0, cpsr
	tst	r0, #0x200                      @ E set: the guest's own world, flags live
	setend	le
	strne	r0, [r5, #CPU_JMP_CPSR]         @ (otherwise the router had already parked them)
	@ the window the IRQ interrupted decides where the guest resumes
	ldr	r1, [r5, #CPU_IRQ_SITE]
	movw	r2, #:lower16:pjit_d_loop
	movt	r2, #:upper16:pjit_d_loop
	sub	r1, r1, r2
	cmp	r1, #16
	bhs	pjit_irq_pc_mem                 @ not the loop core: cpu->ipc is the resume PC
	ldr	r0, [lr]                        @ the loop core: lr = the `b<cc> target` after its `bl`
	lsl	r0, r0, #8
	asr	r0, r0, #6                      @ its displacement, in bytes
	add	r0, r0, lr
	add	r0, r0, #8                      @ the host target (pc reads as .+8)
	HOST2GUEST r0, r1, r2
	mov	r4, r0
	b	1f
pjit_irq_pc_mem:
	ldr	r4, [r5, #CPU_IPC]
	rev	r4, r4
1:	ldr	r0, [r5, #CPU_JMP_CPSR]
	FLAGS_TO_CCR r0, r1
	ldrb	r2, [r5, #(CPU_SR + 1)]
	bic	r2, r2, #0x1F
	orr	r2, r2, r1
	strb	r2, [r5, #(CPU_SR + 1)]         @ the SR in memory is current
	ldr	r3, [r5, #CPU_IRQ_LEVEL]
	mov	r0, #0
	cmp	r3, #7
	strhs	r0, [r5, #CPU_IRQ_LEVEL]        @ level 7 is an edge (NMI); a lower level stays until the device drops it (the SR mask now equals it; an RTE that lowers the mask looks again)
	str	r0, [r5, #CPU_IRQ_FLAG]
	.ifdef	PJIT_TIER
	ldr	r0, [r5, #CPU_TAGS_BE]          @ the slot router's fast path is back
	str	r0, [r5, #CPU_TAGS_GOTO]
	mov	r0, #0
	.endif
	cmp	r3, #8                          @ the platform's RESET: pjit_run() returns 0x100 (the C side restarts the machine)
	moveq	r3, #0x100
	beq	pjit_i_leave
	add	r3, r3, #24                     @ the autovector
	b	pjit_i_deliver

@ Deliver vector r3 to the guest: build the 68000 frame on the supervisor stack,
@ set S, and continue at the vector table's entry.
pjit_i_deliver:
	.ifdef	PJIT_VBLLOG
	.weak	pjit_vbl_ring
	.weak	pjit_vbl_pos
	.weak	pjit_pace_dl
	movw	r0, #:lower16:pjit_vbl_pos      @ telemetry: a ring of 16-byte records {cycle counter, vector, guest PC, total us the pace governor was behind (raw, big-endian)} of every exception delivered;
	movt	r0, #:upper16:pjit_vbl_pos      @ the position counts bytes and never wraps, the ring index is its low 18 bits (r3 = the vector and r4 = the PC are kept)
	ldr	r1, [r0]
	add	r2, r1, #16
	str	r2, [r0]
	ubfx	r1, r1, #0, #18
	movw	r0, #:lower16:pjit_vbl_ring
	movt	r0, #:upper16:pjit_vbl_ring
	add	r2, r0, r1
	str	r3, [r2, #4]
	str	r4, [r2, #8]
	mrc	p15, 0, r0, c9, c13, 0
	str	r0, [r2]
	movw	r0, #:lower16:pjit_pace_dl
	movt	r0, #:upper16:pjit_pace_dl
	ldr	r0, [r0, #12]
	str	r0, [r2, #12]
	.endif
	.ifdef	PJIT_DLOG
	.weak	pjit_dlog
	.weak	pjit_dlog_pos
	movw	r0, #:lower16:pjit_dlog_pos     @ debug: a ring of (vector, guest PC) of every exception delivered
	movt	r0, #:upper16:pjit_dlog_pos
	ldr	r1, [r0]
	movw	r2, #:lower16:pjit_dlog
	movt	r2, #:upper16:pjit_dlog
	add	r2, r2, r1
	str	r3, [r2]
	str	r4, [r2, #4]
	add	r1, r1, #8
	and	r1, r1, #0xFF
	str	r1, [r0]
	.weak	pjit_dlog_count
	movw	r0, #:lower16:pjit_dlog_count   @ ... and a plain count (the ring wraps after 32)
	movt	r0, #:upper16:pjit_dlog_count
	ldr	r1, [r0]
	add	r1, r1, #1
	str	r1, [r0]
	.weak	pjit_xlog
	.weak	pjit_xlog_pos
	sub	r1, r3, #24                     @ ... and a second ring of the exceptions that are NOT an autovector interrupt (24..31) or exec's Supervisor() (8)
	cmp	r1, #8
	blo	1f
	cmp	r3, #8
	beq	1f
	movw	r0, #:lower16:pjit_xlog_pos
	movt	r0, #:upper16:pjit_xlog_pos
	ldr	r1, [r0]
	movw	r2, #:lower16:pjit_xlog
	movt	r2, #:upper16:pjit_xlog
	add	r2, r2, r1
	str	r3, [r2]
	str	r4, [r2, #4]
	add	r1, r1, #8
	and	r1, r1, #0x7F
	str	r1, [r0]
	.ifdef	PJIT_ITRACE
	sub	r1, r4, #0xF80000               @ any exception (not an interrupt / Supervisor()) with the PC outside the ROM ($F80000-$FFFFFF): freeze the instruction trace
	cmp	r1, #0x80000
	blo	1f
	movw	r0, #:lower16:pjit_trace_frz
	movt	r0, #:upper16:pjit_trace_frz
	mov	r1, #8
	str	r1, [r0]
	.weak	pjit_xmem
	movw	r0, #:lower16:pjit_xmem         @ and snapshot the 640 bytes of guest memory around the PC (raw halfwords: the window's lane order is undone by the C side) before anything else touches them
	movt	r0, #:upper16:pjit_xmem
	sub	r1, r4, #512
	add	r4, r4, #128
2:	ldrh	r2, [r1], #2
	strh	r2, [r0], #2
	cmp	r1, r4
	bne	2b
	sub	r4, r4, #128
	.endif
1:
	.endif
	@ The 68000 stacks the address of the instruction itself (not the one after it) for illegal (4), privilege violation (8), line A (10) and line F (11); r4 is the end
	@ of the instruction here. Illegal and line A/F opcodes are one word. The privileged ones: ORI/ANDI/EORI #,SR and STOP (opcode + 1 word), RESET / RTE / MOVE USP
	@ (1 word), MOVE <ea>,SR (opcode + the <ea>'s extension words, found from the mode). exec's Supervisor() depends on it (its handler compares the stacked PC).
	cmp	r3, #4
	beq	.Lback2
	cmp	r3, #10
	beq	.Lback2
	cmp	r3, #11
	beq	.Lback2
	cmp	r3, #8
	bne	.Lbackdone
	ldrh	r0, [r4, #-4]
	rev16	r0, r0
	bic	r1, r0, #0x0A00
	cmp	r1, #0x7C                       @ ORI / ANDI / EORI to SR
	beq	.Lback4
	movw	r1, #0x4E72
	cmp	r0, r1                          @ STOP
	beq	.Lback4
	ldrh	r0, [r4, #-2]
	rev16	r0, r0
	movw	r1, #0x4E70
	cmp	r0, r1                          @ RESET
	beq	.Lback2
	movw	r1, #0x4E73
	cmp	r0, r1                          @ RTE
	beq	.Lback2
	bic	r1, r0, #0xF
	movw	r2, #0x4E60
	cmp	r1, r2                          @ MOVE USP
	beq	.Lback2
	bic	r1, r0, #0x3F                   @ MOVE <ea>,SR: the opcode is the last word (no extension words), or one or two words earlier
	movw	r2, #0x46C0
	cmp	r1, r2
	bne	1f
	and	r1, r0, #0x38
	cmp	r1, #0x28                       @ modes 5, 6, 7 have extension words: not this word
	blo	.Lback2
1:	ldrh	r0, [r4, #-4]
	rev16	r0, r0
	bic	r1, r0, #0x3F
	cmp	r1, r2
	bne	.Lback6
	and	r1, r0, #0x38
	cmp	r1, #0x28
	blo	.Lback6
	cmp	r1, #0x38
	bne	.Lback4                         @ (d16,An) / (d8,An,Xn): one word
	and	r1, r0, #7
	cmp	r1, #1
	bne	.Lback4                         @ abs.W, (d16,PC), #imm.W: one word; abs.L is two
	b	.Lback6
.Lback6:
	sub	r4, r4, #2
.Lback4:
	sub	r4, r4, #2
.Lback2:
	sub	r4, r4, #2
.Lbackdone:
	ldr	r1, [r5, #CPU_VBR]      @ (a plain LE word: the harness sets it from C)
	msr	cpsr_c, #(MODE_SYS | IF_BITS)   @ the guest's registers and SP
	setend	be
	add	r1, r1, r3, lsl #2
	ldr	r2, [r1]                @ r2 = the handler's address (a big-endian table entry)
	@ The first read of the table after an interrupt or a store to the Amiga window can return stale bus data (seen as an 0xAAAAAAAA vector for level 3 where the second read was right;
	@ a posted write / the previous cycle's data): read until two consecutive reads agree.
.Lvec_again:
	ldr	r0, [r1]
	cmp	r0, r2
	movne	r2, r0
	bne	.Lvec_again
	ldrh	r0, [r5, #CPU_SR]       @ r0 = the SR as the program had it

	tst	r0, #0x2000             @ from user mode? park A7 as USP and use the
	streq	sp, [r5, #CPU_USP]      @ supervisor stack: frames always go there
	ldreq	sp, [r5, #CPU_SSP]

	str	r4, [sp, #-4]!          @ the frame: PC (long) first ...
	strh	r0, [sp, #-2]!          @ ... then SR, which ends up on top

	cmp	r3, #VEC_ADDRESSERR     @ bus and address errors push three more words:
	bhi	3f
	ldrb	r1, [r4, #-2]           @   the instruction word, by bytes: after an
	ldrb	r3, [r4, #-1]           @   odd-PC fetch error r4 is odd (r3, the
	orr	r1, r3, r1, lsl #8      @   vector, is spent by now)
	strh	r1, [sp, #-2]!
	ldr	r1, [r5, #CPU_R0]
	rev	r1, r1                  @   the access address (saved LE by pjit_i_svc)
	str	r1, [sp, #-4]!
	ldr	r1, [r5, #CPU_R1]
	rev	r1, r1                  @   the function-code hint: bit 3 = program space,
	tst	r1, #8                  @   bit 4 = read
	and	r1, r1, #0x10
	orrne	r1, r1, #2              @   FC1 (program) ...
	orreq	r1, r1, #1              @   ... or FC0 (data)
	tst	r0, #0x2000
	orrne	r1, r1, #4              @   supervisor
	strh	r1, [sp, #-2]!
	b	1f
3:	sub	r1, r3, #25             @ an interrupt autovector (25-31 = levels 1-7): the SR's
	cmp	r1, #6                  @ interrupt mask becomes the level
	bhi	1f
	bic	r0, r0, #0x700
	add	r1, r1, #1
	orr	r0, r0, r1, lsl #8
1:
	orr	r0, r0, #0x2000         @ S set ...
	bic	r0, r0, #0x8000         @ ... T clear; the interrupt mask and CCR are unchanged
	strh	r0, [r5, #CPU_SR]
	.ifdef	PJIT_FC
	movw	r3, #0xC194                    @ FC2 (GPIO2_10) high: the exception runs in supervisor state (GPIO2 SETDATAOUT; the BE store of 0x40000 is the LE word 0x400)
	movt	r3, #0x481A
	mov	r1, #0x40000
	str	r1, [r3]
	dsb	sy
	.endif
	.ifdef	PJIT_LATCH
	UNMASK	r3, r1                          @ interrupts on: the guest runs with the host's IRQ enabled (the latch)
	.endif
	.ifdef	PJIT_DLOG
	.weak	pjit_flog
	str	r2, [r5, #CPU_R2]               @ debug: a ring of {sp after the push, the new SR, the first two frame words read back} per delivery (big-endian words; word 0 = the byte offset of the next entry)
	movw	r1, #:lower16:pjit_flog
	movt	r1, #:upper16:pjit_flog
	ldr	r3, [r1]
	add	r2, r3, #16
	and	r2, r2, #0xFF
	str	r2, [r1]
	add	r1, r1, r3
	str	sp, [r1, #4]
	str	r0, [r1, #8]
	ldr	r2, [sp]
	str	r2, [r1, #12]
	ldr	r2, [sp, #4]
	str	r2, [r1, #16]
	ldr	r2, [r5, #CPU_R2]
	.endif
	.global	pjit_exc_end
pjit_exc_end:                           @ (a data abort between pjit_exc_begin and here is a fault while delivering one: the 68000 halts)
	SR_TO_FLAGS r0, r1              @ (the exception leaves the CCR as it was)
	mov	r0, r2                  @ run the handler: interpretive or cached, the router decides
	.ifdef	PJIT_PIN_D0
	UNPARK_BE
	.endif
	b	pjit_goto
