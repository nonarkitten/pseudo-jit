@ pjit/runtime_dr.s -- the direct-mapped DR cache: lookup, fill (the translator),
@ and the control-flow cores the cached code branches into. Hard gate: no C.
@
@ Geometry (pjit.h): a 4MB cache of 1024 lines (CL). A CL covers 512 guest bytes, 256 guest
@ halfwords at 16 host bytes each (4 ARM words per halfword), so guest address
@ -> host address is arithmetic:
@     host = cache_data + ((guest & (2^19 - 1)) << 3)
@ and each CL has a 16-bit tag (guest >> 19). A cleared CL is all `bl pjit_fill`: the first time
@ execution enters a slot, pjit_fill translates the
@ guest code from that slot to the end of its 64-byte cache-line entry (CLE) --
@ an instruction that starts in the CLE finishes even if it spills into the next
@ one -- by writing, per guest instruction:
@     [extension-word prefix: r1 (source) / r2 (destination)] [leader] [bl body]
@ in 2 ARM words per guest word. The last slot of every CL is never written (it stays a `bl
@ pjit_fill`), and anything that cannot be translated -- it does not fit before
@ that slot, or it is a (d8,PC,Xn) -- is single-stepped by the interpreter with
@ LR = pjit_resume, which re-enters the cache at the next instruction.
@
@ Everything here runs E=LE (the tables are LE; guest memory is read with rev16 /
@ written with rev) and brackets that with `setend le` ... `setend be`. The
@ guest's flags are live on entry from cached code: they are parked in
@ cpu->jmp_cpsr first and put back just before the jump.

	.include	"ipl.inc"
	.syntax unified
	.arm
	.text
		.ifdef	PJIT_PAD
	.rept	PJIT_PAD
	nop
	.endr
	.endif
.include "asm_offsets.inc"

	.equ	VEC_ADDRESSERR, 3
	.equ	DR_SPAN_BITS,   DR_TAG_SHIFT            @ 19: log2 of the guest bytes the cache covers
	.equ	S_H,     CPU_SPILL + 0                  @ fill: the cold word that was entered
	.equ	S_SI,    CPU_SPILL + 4                  @       slot index of the write cursor
	.equ	S_END,   CPU_SPILL + 8                  @       slot index where this CLE ends
	.equ	S_OP,    CPU_SPILL + 12                 @       the opcode being translated
	.equ	S_N,     CPU_SPILL + 16                 @       its length in guest words
	.equ	S_E,     CPU_SPILL + 20                 @       its (src << 4) | dst extension modes
	.equ	S_G0,    CPU_SPILL + 24                 @       guest address of the entry slot
	.ifdef	PJIT_PIN_D0
	.set	defined_pin, 1
	.else
	.set	defined_pin, 0
	.endif
	.equ	S_FLAG,  CPU_SPILL + 28                 @ "r4 already holds the guest PC" (exceptions)
	.equ	S_DONE,  CPU_SPILL + 32                 @ fill: the entry slot was translated
	.equ	S_SEL,   CPU_SPILL + 36                 @       fill_emit_ea: 0 = source, 1 = destination
	.equ	S_T,     CPU_SPILL + 40                 @       fill_emit_ea scratch
	.equ	S_START, CPU_SPILL + 44
	.equ	S_CLL,   CPU_SPILL + 48                 @ claim_line: the line ...
	.equ	S_CLT,   CPU_SPILL + 52                 @ ... its tag ...
	.equ	S_CLR,   CPU_SPILL + 56                 @ ... and its return address
	.equ	S_LKR,   CPU_SPILL + 60                 @ fill_link: return address,
	.equ	S_LK0,   CPU_SPILL + 64                 @ the host cursor,
	.equ	S_LK1,   CPU_SPILL + 68                 @ the guest cursor and
	.equ	S_LKT,   CPU_SPILL + 72                 @ the target                 @       host address where this instruction began
	.equ	S_XOT,   CPU_SPILL + 76                 @ fill: byte offset from ot to the table in use (0, or to ot_x on an exact line)
	.equ	S_XINL,  CPU_SPILL + 80                 @       ... and from inl to inl_x
	.equ	S_XS0,   CPU_SPILL + 84                 @       the scan's saved r0 ...
	.equ	S_XS1,   CPU_SPILL + 88                 @       ... and r2
	.equ	S_FOLD,  CPU_SPILL + 92                 @       the leader with a displacement folded into it, or 0
	.equ	S_FZ0,   CPU_SPILL + 96                 @ the scratch of the loop compactor (c_info, c_words, ...) ...
	.equ	S_FZ1,   CPU_SPILL + 100
	.equ	S_FZ2,   CPU_SPILL + 104
	.equ	S_FZ3,   CPU_SPILL + 108
	.equ	S_FZ4,   CPU_SPILL + 112
	.equ	S_FZ5,   CPU_SPILL + 116                @ (also the in-slot MOVEM expansion's scratch)
	.ifndef	PJIT_TIER_T
	.set	PJIT_TIER_T, 64                 @ tiered build: taken-transfer events at one target before it is promoted to a block (1..128; 0 = never)
	.endif
	.set	PJIT_TIER_TB, 1                  @ the count's trigger bit: the smallest TB with 2^TB >= PJIT_TIER_T (7 for 128); the 'promoted before' mark is the bit above it
	.rept	10
	.if	(1 << PJIT_TIER_TB) < PJIT_TIER_T
	.set	PJIT_TIER_TB, PJIT_TIER_TB + 1
	.endif
	.endr
	.set	PJIT_TIER_K, (1 << PJIT_TIER_TB) + 1 - PJIT_TIER_T    @ a new target's first count; the count has bit TB set after PJIT_TIER_T events
	.set	PJIT_TIER_MARK, 1 << (PJIT_TIER_TB + 1)
	.ifdef	PJIT_TIER
	.set	PJIT_TIER_ON, PJIT_TIER_T
	.else
	.set	PJIT_TIER_ON, 0
	.endif
	.ifndef	PJIT_COMPACT
	.set	PJIT_COMPACT, 1                 @ loop compaction (make COMPACT=0 to measure without it)
	.endif
	.equ	S_LOOPT, CPU_SPILL + 124                @ fill: the guest address the backward branch just translated goes to, 0 if the instruction was not one (see compact_try)
	.equ	S_CLAIMED, CPU_SPILL + 120              @ fill: this translation has claimed a line for a link (a claim is 16 k cycles: one per translation, the instruction that needs a second waits for the next)
	.equ	LINK_MAX, 64                            @ lines that may hold direct links at once: an eviction invalidates all of them (~75 cycles each)

@ Flush [\start, \end) from the data cache and the instruction cache (64-byte lines).
@ Clobbers \start and \tmp.
	.macro	FLUSH_RANGE start, end, tmp
	bic	\start, \start, #63
	mov	\tmp, \start
1:	mcr	p15, 0, \tmp, c7, c10, 1        @ DCCMVAC: clean the line to the point of coherency
	add	\tmp, \tmp, #64
	cmp	\tmp, \end
	blo	1b
	dsb
2:	mcr	p15, 0, \start, c7, c5, 1       @ ICIMVAU: invalidate the instruction line
	add	\start, \start, #64
	cmp	\start, \end
	blo	2b
	dsb                                     @ (no BPIALL: a stale predictor entry for rewritten code only costs a misprediction, and emptying the whole predictor
	isb                                     @ on every translation made every later branch in the program mispredict)
	.endm

@ \rd = a host address inside the cache -> the guest address it translates
@ (tag << 19 | offset / 4; even only if the host address is a slot's first word).
@ E=LE. Clobbers \t1 and \t2.
	.macro	HOST2GUEST rd, t1, t2
	ldr	\t1, [r5, #CPU_CACHE_DATA]
	sub	\rd, \rd, \t1                   @ offset into the cache
	ldr	\t1, [r5, #CPU_TAGS]
	lsr	\t2, \rd, #DR_HL_SHIFT_                    @ line
	add	\t1, \t1, \t2, lsl #1
	ldrh	\t1, [\t1]                      @ that line's tag
	lsr	\rd, \rd, #3
	orr	\rd, \rd, \t1, lsl #DR_TAG_SHIFT
	.endm

@ The same, for code running big-endian with the flags live (the cores): the pointers come from their
@ byte-swapped copies and the tag, stored by little-endian code, is swapped back.
	.macro	HOST2GUEST_BE rd, t1, t2
	ldr	\t1, [r5, #CPU_CACHE_BE]
	sub	\rd, \rd, \t1
	ldr	\t1, [r5, #CPU_TAGS_BE]
	lsr	\t2, \rd, #DR_HL_SHIFT_
	add	\t1, \t1, \t2, lsl #1
	ldrh	\t1, [\t1]
	rev16	\t1, \t1
	lsr	\rd, \rd, #3
	orr	\rd, \rd, \t1, lsl #DR_TAG_SHIFT
	.endm

@ clear the I bit if cpu->irq_unmask says so (the latch model: guest code runs with the host's IRQ enabled); no guest flag touched
	.macro	UNMASK t1, t2
	ldrb	\t1, [r5, #CPU_IRQ_UNMASK]
	mrs	\t2, cpsr
	bic	\t2, \t2, \t1
	msr	cpsr_c, \t2
	.endm

@ D0 is pinned in r4 (PJIT_PIN_D0, runtime.s): the fast path leaves r4 alone; the slow paths (the router below, the fill) use r4 as scratch, so they PARK it in cpu->d0 on
@ the way in (pjit_goto_slow, pjit_fill) and UNPARK it on every way back into guest code.
	.ifdef	PJIT_PIN_D0
	.macro	PARK_LE t
	rev	\t, r4
	str	\t, [r5, #CPU_D0]
	.endm
	.macro	UNPARK_BE
	ldr	r4, [r5, #CPU_D0]
	.endm
	.endif

@ ---- the router --------------------------------------------------------------
@ pjit_goto:  r0 = the guest address to continue at; the guest's flags
@ are live; E=BE. Interpretive below cpu->dr_threshold, otherwise through the cache.
@ The fast path (E=BE, flags live and left alone): the target is a hit if the line's tag is the
@ target's, the address is even and no interrupt windows are on; the three are folded into one
@ number (zero = hit) with USAT, and a computed jump picks `bx host` or the slow path. Nothing sets
@ a flag, so nothing is parked.
	.balign	64
	.skip	40                              @ (so that pjit_jsr_ind's 24 bytes end where pjit_goto starts: it falls into it)
	.global	pjit_jsr_ind
pjit_jsr_ind:                           @ JSR (An) / d16(An): r0 = the guest return address, r1 = the target, lr = the next instruction (exact)
	str	r0, [sp, #-4]!
	mov	r0, r1
	.balign	64                       @ (hot core: its address, and so its branch target buffer sets, must not depend on what precedes it)
	.global	pjit_goto
	.type	pjit_goto, %function
pjit_goto:
	.ifdef	PJIT_TIER
	@ tiered build: a promoted block is found first (the table blk.s keeps: {guest, block}, both byte-swapped), flag-free
	movw	r1, #:lower16:blk_fast
	movt	r1, #:upper16:blk_fast
	ubfx	r2, r0, #1, #BLK_FAST_BITS
	add	r1, r1, r2, lsl #3
	ldr	r2, [r1]
	ldr	r3, [r1, #4]
	eor	r2, r2, r0
	clz	r2, r2
	lsr	r2, r2, #5
	eor	r2, r2, #1                      @ 0 = a promoted block, 1 = none
	add	pc, pc, r2, lsl #2              @ pc reads as .+8: the entry below the nop
	nop
	bx	r3
	.endif
	ldr	r2, [r5, #CPU_TAGS_GOTO]        @ the tags, or an all-miss array while interrupt windows are on (the slow path has them)
	ubfx	r1, r0, #(DR_SPAN_BITS - DR_LINE_BITS), #DR_LINE_BITS   @ the line
	add	r2, r2, r1, lsl #1
	ldrh	r2, [r2]
	ubfx	r3, r0, #0, #DR_SPAN_BITS       @ the offset into the cache's span
	rev16	r2, r2                          @ (the tag was stored little-endian)
	eor	r2, r2, r0, lsr #DR_TAG_SHIFT   @ 0 when it is this address's tag
	and	r1, r0, #1
	orr	r2, r2, r1                      @ an odd address is never a hit
	ldr	r1, [r5, #CPU_CACHE_BE]
	usat	r2, #1, r2                      @ 0 = hit, 1 = anything else
	add	r1, r1, r3, lsl #3              @ the host address of the target's first slot word
	.if	PJIT_TIER_ON
	@ tiered build: a slot hit is counted. The table pjit_tier_cnt (1024 x {target, count}, big-endian words, direct-mapped) holds, per index, the last target seen there and
	@ the number of times it was; a different target restarts at K. The count reaches 128 (bit 7) after PJIT_TIER_T events: the target is promoted to a block (pjit_goto_promote).
	@ Flag-free. Hit: the 32 words below run; miss: `add pc` skips them.
	add	pc, pc, r2, lsl #7
	nop
	movw	lr, #:lower16:pjit_tier_cnt
	movt	lr, #:upper16:pjit_tier_cnt
	ubfx	r2, r0, #1, #10
	add	lr, lr, r2, lsl #3
	ldr	r2, [lr]
	ldr	r3, [lr, #4]
	eor	r2, r2, r0
	clz	r2, r2
	lsr	r2, r2, #5                      @ 1 = the entry is this target's
	rsb	r2, r2, #0                      @ -1 = same, 0 = another target's
	sub	r3, r3, #(PJIT_TIER_K - 1)
	and	r3, r3, r2
	add	r3, r3, #PJIT_TIER_K            @ same: count + 1; another: K (the first event of a new target)
	str	r0, [lr]
	str	r3, [lr, #4]
	ubfx	r2, r3, #PJIT_TIER_TB, #1       @ 1 when the count reached the threshold (the bit above is the 'had a block before' mark and rides along)
	add	pc, pc, r2, lsl #2
	nop
	bx	r1                              @ below the threshold: the slot
	b	pjit_goto_promote
	.rept	12
	nop
	.endr
	.else
	add	pc, pc, r2, lsl #2              @ pc reads as .+8: the entry below the nop
	nop
	bx	r1
	.endif
	@ fall into the slow path
	.global	pjit_goto_slow
pjit_goto_slow:
	mrs	r1, cpsr
	setend	le
	str	r1, [r5, #CPU_JMP_CPSR]
	.ifdef	PJIT_PIN_D0
	PARK_LE	r1                              @ the guest's D0 into cpu->d0 (r4 is scratch from here)
	.endif
	.global	pjit_goto_parked
pjit_goto_parked:                       @ (E=LE, flags parked) r0 = the target
	.ifdef	PJIT_TIER
	ldr	r1, [r5, #CPU_FLUSH_REQ]        @ a guest write hit translated code (pjit_post_flush): both tiers are flushed by the block router, on its private stack, and it comes back here
	cmp	r1, #0
	movne	r1, #16                         @ blk.s BLK_STUB_FLUSH
	bne	pjit_blk_park
	ldrb	r1, [r5, #CPU_IRQ_FLAG]         @ the latch model: a posted level that beats the SR mask is delivered here, at this boundary (the flag only brought us here)
	cmp	r1, #0
	beq	8f
	mov	r2, #0
	str	r2, [r5, #CPU_IRQ_FLAG]
	ldr	r2, [r5, #CPU_TAGS_BE]
	str	r2, [r5, #CPU_TAGS_GOTO]        @ the fast path is back
	ldrb	r1, [r5, #CPU_IRQ_LEVEL]
	IRQ_VERIFY r1, r2, r3
	cmp	r1, #0
	beq	8f
	cmp	r1, #7
	beq	7f
	ldrb	r2, [r5, #CPU_SR]               @ the SR's high byte (big-endian in memory)
	and	r2, r2, #7
	cmp	r1, r2
	bls	8f
7:	rev	r1, r0                          @ the guest resumes at the target after the handler returns
	str	r1, [r5, #CPU_IPC]
	mov	r1, #0
	str	r1, [r5, #CPU_IRQ_SITE]
	b	pjit_irq_deliver
8:
	.endif
	ldr	r1, [r5, #CPU_IRQ_WINDOWS]
	cmp	r1, #0
	beq	9f
	rev	r1, r0                          @ the interrupt window: cpu->ipc = where the guest resumes
	str	r1, [r5, #CPU_IPC]
	cpsie	i
	nop
	cpsid	i
9:
	tst	r0, #1
	bne	pjit_goto_odd
	ldr	r1, [r5, #CPU_DR_THRESHOLD]
	cmp	r0, r1
	blo	pjit_goto_interp
	ldr	r2, [r5, #CPU_XLAT]             @ the platform's map of the megabytes that are translated (none: everything above the threshold is)
	cmp	r2, #0
	beq	2f
	lsr	r1, r0, #20
	add	r2, r2, r1, lsl #1
	ldrh	r2, [r2]
	cmp	r2, #0
	beq	pjit_goto_interp
2:
	ldr	r2, [r5, #CPU_TAGS]
	lsl	r1, r0, #(32 - DR_SPAN_BITS)    @ (0xFFF is not an ARM immediate: two shifts instead)
	lsr	r1, r1, #(32 - DR_LINE_BITS)    @ r1 = the line: guest bits 18:7
	add	r2, r2, r1, lsl #1
	ldrh	r2, [r2]
	lsr	r3, r0, #DR_TAG_SHIFT           @ r3 = this address's tag
	cmp	r2, r3
	bne	pjit_goto_miss
pjit_goto_hit:
	ldr	r2, [r5, #CPU_CACHE_DATA]
	lsl	r1, r0, #(32 - DR_SPAN_BITS)
	lsr	r1, r1, #(32 - DR_SPAN_BITS - 3)    @ (guest & (2^19 - 1)) << 3
	add	r2, r2, r1
	.ifdef	PJIT_TIER
	UNMASK	r0, r3
	.endif
	ldr	r1, [r5, #CPU_JMP_CPSR]
	setend	be
	.ifdef	PJIT_PIN_D0
	UNPARK_BE
	.endif
	msr	cpsr_f, r1
	bx	r2

pjit_goto_interp:                       @ below the threshold: the interpreter, PC = r0
	ldr	r1, [r5, #CPU_JMP_CPSR]
	ldr	lr, [r5, #CPU_PREFETCH_HOOK]    @ pjit_prefetch, or its interrupt-window twin
	.ifdef	PJIT_TIER
	UNMASK	r2, r3
	.endif
	setend	be
	.ifdef	PJIT_PIN_D0
	UNPARK_BE
	.endif
	str	r0, [r5, #CPU_IPC]
	msr	cpsr_f, r1
	bx	lr                              @ through the hook (lr = cpu->prefetch_hook, which ends in pjit_prefetch): the first instruction of a handler is counted / polled too

pjit_goto_odd:                          @ an odd PC: address error on the fetch
	.ifdef	PJIT_PIN_D0
	rev	r2, r0                          @ the frame's PC is the odd target; r0 stays the fault address
	str	r2, [r5, #CPU_IPC]
	mov	r1, #1
	str	r1, [r5, #S_FLAG]               @ tell the exception entry cpu->ipc already holds the PC
	ldr	r2, [r5, #CPU_JMP_CPSR]
	mov	r1, #0x18                       @ program space, read
	setend	be
	UNPARK_BE                               @ (the exception entry parks D0 again)
	msr	cpsr_f, r2
	svc	#VEC_ADDRESSERR
	.else
	rev	r4, r0                          @ the frame's PC is the odd target; r0 stays the fault address
	str	r4, [r5, #CPU_IPC]
	mov	r1, #1
	str	r1, [r5, #S_FLAG]               @ tell the exception entry r4 already holds the PC
	ldr	r2, [r5, #CPU_JMP_CPSR]
	mov	r1, #0x18                       @ program space, read
	setend	be
	msr	cpsr_f, r2
	svc	#VEC_ADDRESSERR
	.endif

	.ifdef	PJIT_TIER
pjit_goto_promote:                      @ (E=BE, flags live) r0 = target, lr = its counter entry: the count restarts (marked: this target has been promoted), the block router builds / finds the block and enters it
	ldr	r2, [lr, #4]
	ubfx	r1, r2, #(PJIT_TIER_TB + 1), #1 @ promoted before?
	mov	r2, #PJIT_TIER_MARK
	str	r2, [lr, #4]
	add	r1, r1, r1                      @ blk.s: BLK_STUB_PROMOTE -- a forced translation, nothing to link: 8, or 12 for a target that had a block before (a rebuild, for the thrash guard)
	add	r1, r1, r1
	add	r1, r1, #8
	b	pjit_blk_go
	.endif

pjit_goto_miss:                         @ r0 = target, r1 = line, r3 = tag: claim the line
	str	r0, [r5, #CPU_SPILL]
	.ifdef	PJIT_TIER
	ldr	r2, [r5, #CPU_CODEBITS]         @ write detection (pjit_platform_code): a 128-byte line that is not registered yet is announced to the platform before its first translation
	cmp	r2, #0
	beq	1f
	str	r1, [r5, #S_XS0]                @ (r1 = the line: used as a scratch below, put back before claim_line; r4 is free: D0 is parked)
	ldr	r4, [r5, #CPU_XLAT]
	lsr	r1, r0, #20
	add	r4, r4, r1, lsl #1
	ldrh	r4, [r4]                        @ 1 + the tracked megabyte (0: not tracked)
	cmp	r4, #0
	beq	2f
	sub	r4, r4, #1
	lsl	r4, r4, #20
	bfi	r4, r0, #0, #20                 @ r4 = the tracked offset of the target
	lsr	r1, r4, #12
	add	r2, r2, r1, lsl #2              @ the bitmap word of this 4 KB (32 lines of 128 bytes)
	ubfx	r1, r4, #7, #5
	ldr	r2, [r2]
	lsr	r2, r2, r1
	tst	r2, #1
	ldr	r1, [r5, #S_XS0]
	moveq	r1, #20                         @ not registered (bit clear): blk.s BLK_STUB_CODE (the block router calls the platform on its private stack and gives the guest back here)
	beq	pjit_blk_park
	b	1f
2:	ldr	r1, [r5, #S_XS0]
1:
	.endif
	bl	claim_line
	ldr	r0, [r5, #CPU_SPILL]
	b	pjit_goto_hit

@ Makes line r1 this tag r3's: if it holds another address's code that is an eviction, and every line that
@ holds a direct link is invalidated first (a link into the evicted line would run the new code at the
@ old address); then the line becomes `bl pjit_fill` words. E=LE. Clobbers r0-r4, keeps lr in a spill slot.
claim_line:
	str	lr, [r5, #S_CLR]
	str	r1, [r5, #S_CLL]
	str	r3, [r5, #S_CLT]
	movw	r0, #:lower16:ARENA_XEXACT      @ a new owner of the line: look for X readers near it again (see pjit_fill)
	movt	r0, #:upper16:ARENA_XEXACT
	add	r0, r0, r5
	mov	r2, #0
	strb	r2, [r0, r1]
	ldr	r2, [r5, #CPU_TAGS]
	add	r2, r2, r1, lsl #1
	ldrh	r0, [r2]                        @ the line's current tag
	movw	r4, #0xFFFF
	cmp	r0, r4
	beq	4f                              @ free: nothing to evict
	cmp	r0, r3
	beq	4f
	add	r4, r5, #ARENA_LINKS
	ldr	r0, [r4, #LK_EVICT]
	add	r0, r0, #1
	str	r0, [r4, #LK_EVICT]
	ldr	r0, [r4, #LK_COUNT]
	cmp	r0, #0
	beq	4f
	ldr	r2, [r5, #CPU_TAGS]
	add	r1, r4, #LK_LIST
	movw	r3, #0xFFFF
1:	ldrh	lr, [r1], #2                    @ pass 1: the tags of the lines that hold links
	add	lr, r2, lr, lsl #1
	strh	r3, [lr]
	subs	r0, r0, #1
	bne	1b
	ldr	r0, [r4, #LK_COUNT]
	add	r1, r4, #LK_LIST
	mov	r2, #0
	add	r3, r4, #LK_FLAG
2:	ldrh	lr, [r1], #2                    @ pass 2: ... and their flags
	strb	r2, [r3, lr]
	subs	r0, r0, #1
	bne	2b
	ldr	r0, [r4, #LK_COUNT]
	ldr	r1, [r4, #LK_INVAL]
	add	r1, r1, r0
	str	r1, [r4, #LK_INVAL]
	str	r2, [r4, #LK_COUNT]
4:	ldr	r1, [r5, #S_CLL]
	ldr	r3, [r5, #S_CLT]
	ldr	r2, [r5, #CPU_TAGS]
	add	r2, r2, r1, lsl #1
	strh	r3, [r2]
	ldr	r2, [r5, #CPU_CACHE_DATA]
	add	r2, r2, r1, lsl #DR_HL_SHIFT_      @ the line's host base
	str	r2, [r5, #(CPU_SPILL + 4)]
	movw	r1, #:lower16:pjit_fill
	movt	r1, #:upper16:pjit_fill
	sub	r3, r1, r2
	sub	r3, r3, #8
	asr	r3, r3, #2                      @ the first word's `bl` offset, in words
	add	r1, r2, #(1 << DR_HL_SHIFT_)
5:	bic	r0, r3, #0xFF000000             @ every word becomes `bl pjit_fill`
	orr	r0, r0, #0xEB000000
	str	r0, [r2], #4
	sub	r3, r3, #1                      @ the next word is 4 bytes further on
	cmp	r2, r1
	bne	5b
	ldr	r2, [r5, #(CPU_SPILL + 4)]
	add	r1, r2, #(1 << DR_HL_SHIFT_)
	FLUSH_RANGE r2, r1, r3
	ldr	lr, [r5, #S_CLR]
	bx	lr

@ A direct link to a known guest address: r1 = the target; returns r1 = the host address of its slot, or 0
@ if it cannot be linked (below the cache threshold, odd, interrupt windows on, or its line holds another
@ address's code). A free target line is claimed on the spot. The line this instruction is in is recorded as
@ holding a link (see claim_line). Preserves r0 (host cursor) and r2 (guest cursor); clobbers r3, r4, lr. E=LE.
fill_link:
	str	lr, [r5, #S_LKR]
	str	r0, [r5, #S_LK0]
	str	r2, [r5, #S_LK1]
	str	r1, [r5, #S_LKT]
	ldr	r3, [r5, #CPU_DR_THRESHOLD]
	cmp	r1, r3
	blo	9f
	ldr	r3, [r5, #CPU_XLAT]             @ (see pjit_goto_slow: only a translated megabyte gets linked)
	cmp	r3, #0
	beq	2f
	lsr	r4, r1, #20
	add	r3, r3, r4, lsl #1
	ldrh	r3, [r3]
	cmp	r3, #0
	beq	9f
2:
	tst	r1, #1
	bne	9f
	ldr	r3, [r5, #CPU_IRQ_WINDOWS]
	cmp	r3, #0
	bne	9f
	@ a bound on what an eviction can cost: at most LINK_MAX lines hold links (a new holder is refused beyond that: the router does it)
	ldr	r0, [r5, #S_G0]
	lsl	r0, r0, #(32 - DR_SPAN_BITS)
	lsr	r0, r0, #(32 - DR_LINE_BITS)    @ this instruction's own line
	add	r4, r5, #ARENA_LINKS
	add	r3, r4, #LK_FLAG
	ldrb	r3, [r3, r0]
	cmp	r3, #0
	bne	3f
	ldr	r3, [r4, #LK_COUNT]
	cmp	r3, #LINK_MAX
	bhs	9f
3:
	lsl	r0, r1, #(32 - DR_SPAN_BITS)
	lsr	r0, r0, #(32 - DR_LINE_BITS)    @ the target's line
	lsr	r3, r1, #DR_TAG_SHIFT           @ its tag
	ldr	r2, [r5, #CPU_TAGS]
	add	r2, r2, r0, lsl #1
	ldrh	r4, [r2]
	cmp	r4, r3
	beq	5f
	movw	lr, #0xFFFF
	cmp	r4, lr
	bne	9f                              @ another address's code lives there: use the router
	ldr	lr, [r5, #S_CLAIMED]
	cmp	lr, #0
	bne	7f                              @ a second claim in one translation: stop here, this instruction is translated by the next fill
	mov	r1, r0
	bl	claim_line
	mov	r3, #1
	str	r3, [r5, #S_CLAIMED]
5:	ldr	r1, [r5, #S_G0]                 @ this instruction's own line ...
	lsl	r1, r1, #(32 - DR_SPAN_BITS)
	lsr	r1, r1, #(32 - DR_LINE_BITS)
	add	r4, r5, #ARENA_LINKS
	add	r3, r4, #LK_FLAG
	ldrb	r0, [r3, r1]
	cmp	r0, #0
	bne	6f
	mov	r0, #1
	strb	r0, [r3, r1]                    @ ... joins the list of lines that hold links
	ldr	r0, [r4, #LK_COUNT]
	add	r2, r4, #LK_LIST
	add	r3, r2, r0, lsl #1
	strh	r1, [r3]
	add	r0, r0, #1
	str	r0, [r4, #LK_COUNT]
6:	ldr	r0, [r4, #LK_MADE]
	add	r0, r0, #1
	str	r0, [r4, #LK_MADE]
	ldr	r1, [r5, #S_LKT]
	lsl	r1, r1, #(32 - DR_SPAN_BITS)
	lsr	r1, r1, #(32 - DR_SPAN_BITS - 3)
	ldr	r3, [r5, #CPU_CACHE_DATA]
	add	r1, r1, r3                      @ the target's first slot word
	b	8f
7:	ldr	r0, [r5, #S_START]              @ (nothing of this instruction has been written: its slots are still cold `bl pjit_fill` words)
	b	fill_done
9:	mov	r1, #0
8:	ldr	r0, [r5, #S_LK0]
	ldr	r2, [r5, #S_LK1]
	ldr	lr, [r5, #S_LKR]
	bx	lr

@ ---- single-stepping: the interpreter, then back into the cache -----------------
@ The interpreted instruction's own `bx lr` lands here, with r4 = the next PC.
	.global	pjit_resume
	.type	pjit_resume, %function
pjit_resume:
	ldr	r0, [r5, #CPU_IPC]
	b	pjit_goto

	.section	.text.cold,"ax",%progbits         @ the translator (not hot: its size must not move the cores below, whose placement matters to the branch predictor)
@ ---- the fill routine ---------------------------------------------------------
@ Entered by `bl` from a cold word: lr = that word + 4, E=BE, flags live; r0-r4 are
@ free (they are scratch between guest instructions), r5-r13 are not.
	.global	pjit_fill
	.type	pjit_fill, %function
pjit_fill:
	mrs	r0, cpsr
	setend	le
	str	r0, [r5, #CPU_JMP_CPSR]
	.ifdef	PJIT_PIN_D0
	PARK_LE	r1                              @ D0 (r4) into cpu->d0: the translator uses r4 as scratch
	.endif
	mov	r1, #0
	str	r1, [r5, #S_CLAIMED]
	sub	r0, lr, #4                      @ r0 = H: the cold word, a slot's first word
	str	r0, [r5, #S_H]
	mov	r1, #0
	str	r1, [r5, #S_DONE]
	str	r1, [r5, #S_OP]                 @ (the previous instruction's opcode, looked at by fill_next: none yet)
	ldr	r1, [r5, #CPU_CACHE_DATA]
	sub	r1, r0, r1                      @ offset into the cache
	ldr	r2, [r5, #CPU_TAGS]
	lsr	r3, r1, #DR_HL_SHIFT_
	add	r2, r2, r3, lsl #1
	ldrh	r2, [r2]
	lsr	r3, r1, #3
	orr	r2, r3, r2, lsl #DR_TAG_SHIFT
	bic	r2, r2, #1                      @ r2 = g: the guest address of this slot
	str	r2, [r5, #S_G0]
	ldr	r3, [r5, #CPU_CODEXACT]         @ write detection: this 16-byte group (and the next: the entered instruction may spill into it) now holds translated code
	cmp	r3, #0
	beq	8f
	str	r1, [r5, #S_XS0]                @ (r1 = the cache offset, needed below; the X scan sets S_XS0 afresh)
	ldr	r4, [r5, #CPU_XLAT]
	lsr	r1, r2, #20
	add	r4, r4, r1, lsl #1
	ldrh	r4, [r4]                        @ 1 + the tracked megabyte (0: not tracked)
	cmp	r4, #0
	beq	7f
	sub	r4, r4, #1
	lsl	r4, r4, #20
	bfi	r4, r2, #0, #20                 @ r4 = the tracked offset of this slot's guest address
	lsr	r1, r4, #9
	add	r3, r3, r1, lsl #2              @ the bitmap word of this 512-byte span
	lsr	r1, r4, #4
	and	r1, r1, #31                     @ the group's bit in it
	str	r1, [r5, #S_XS1]
	mov	r4, #3
	lsl	r4, r4, r1                      @ this group and the next
	ldr	r1, [r3]
	orr	r1, r1, r4
	str	r1, [r3]
	ldr	r4, [r5, #S_XS1]
	cmp	r4, #31                         @ the next group is bit 0 of the next word
	ldreq	r1, [r3, #4]
	orreq	r1, r1, #1
	streq	r1, [r3, #4]
7:
	ldr	r1, [r5, #S_XS0]
8:
	lsr	r1, r1, #4
	and	r1, r1, #DR_SLOT_MASK_            @ r1 = si: the slot's index in its line
	str	r1, [r5, #S_SI]
	orr	r1, r1, #7
	add	r1, r1, #1                      @ the end of its CLE
	str	r1, [r5, #S_END]
	@ ---- is this line near an instruction that reads X? In an X_LAZY build the pure X producers (ADD, SUB, ...) do not save X; a line, and the
	@ one after it, that holds an ADDX / SUBX / NEGX / ROXd / ABCD / SBCD / NBCD / MOVE from SR is translated from the exact twins of those
	@ sequences instead (ot_x / inl_x; in an exact build they are copies, so this only costs the scan). The answer is kept per line (xexact:
	@ 0 = not looked at, 1 = lazy, 2 = exact) until the line changes owner (claim_line).
	ldr	r1, [r5, #CPU_CACHE_DATA]
	sub	r1, r0, r1
	lsr	r1, r1, #DR_HL_SHIFT_           @ the line
	movw	r3, #:lower16:ARENA_XEXACT
	movt	r3, #:upper16:ARENA_XEXACT
	add	r3, r3, r5
	ldrb	r4, [r3, r1]
	cmp	r4, #0
	bne	2f
	str	r1, [r5, #S_XOT]                @ (the line, while the scan uses the registers)
	str	r0, [r5, #S_XS0]
	str	r2, [r5, #S_XS1]
	lsr	r1, r2, #DR_CL_SHIFT
	lsl	r1, r1, #DR_CL_SHIFT            @ this line's first guest byte
	add	r3, r1, #(2 << DR_CL_SHIFT)     @ the end of the next line
	movw	r0, #:lower16:ARENA_XCONS
	movt	r0, #:upper16:ARENA_XCONS
	add	r0, r0, r5                      @ r0 = the scan table: flags | extension words << 2
1:	ldrh	r4, [r1], #2                    @ an instruction's first word (guest words are big-endian)
	rev16	r4, r4
	ldrb	r4, [r0, r4]
	tst	r4, #3
	bne	4f                              @ it reads X
	lsrs	r4, r4, #2                      @ its extension words: look at each for the patterns an extension word cannot imitate
	beq	6f
7:	ldrh	r2, [r1], #2
	rev16	r2, r2
	ldrb	r2, [r0, r2]
	tst	r2, #1
	bne	4f
	subs	r4, r4, #1
	bne	7b
6:	cmp	r1, r3
	blo	1b
	ldr	r0, [r5, #S_XS0]
	ldr	r2, [r5, #S_XS1]
	mov	r4, #1                          @ none: lazy
	b	5f
4:	ldr	r0, [r5, #S_XS0]                @ found one: exact
	ldr	r2, [r5, #S_XS1]
	mov	r4, #2
5:	ldr	r1, [r5, #S_XOT]
	movw	r3, #:lower16:ARENA_XEXACT
	movt	r3, #:upper16:ARENA_XEXACT
	add	r3, r3, r5
	strb	r4, [r3, r1]
2:	mov	r3, #0
	mov	r1, #0
	cmp	r4, #2
	movweq	r3, #:lower16:(ARENA_OTX - ARENA_OT)
	movteq	r3, #:upper16:(ARENA_OTX - ARENA_OT)
	movweq	r1, #:lower16:(ARENA_INLX - ARENA_INL)
	movteq	r1, #:upper16:(ARENA_INLX - ARENA_INL)
	str	r3, [r5, #S_XOT]
	str	r1, [r5, #S_XINL]
					@ r0 = host write cursor, r2 = guest read cursor

fill_next:
	@ Nothing is translated after an unconditional transfer (RTS, RTE, RTR, JMP, BRA): what follows is padding, data, or the next routine, and decoding it from here would be out of step with
	@ its real instruction boundaries (`rts ; 0000 ; link a5,...`: the 0000 is an ORI.B #imm,D0 that swallows the LINK as its immediate and leaves its leftovers in the slot where the routine
	@ starts, so a later call to it runs garbage). Those slots stay cold: a jump to them translates from there. (Bcc / DBcc / JSR / BSR continue: their fall-through is real code.)
	ldr	r1, [r5, #S_OP]                 @ the instruction translated just before
	movw	r3, #0x4E73
	subs	r4, r1, r3
	cmp	r4, #4
	bhi	1f
	tst	r4, #1
	beq	fill_done                       @ RTE, RTS, RTR (4E73, 4E75, 4E77)
1:	bic	r4, r1, #0x3F
	movw	r3, #0x4EC0
	cmp	r4, r3
	beq	fill_done                       @ JMP
	lsr	r4, r1, #8
	cmp	r4, #0x60
	beq	fill_done                       @ BRA
	ldr	r1, [r5, #S_SI]
	ldr	r4, [r5, #S_END]
	cmp	r1, r4
	bhs	fill_done                       @ past this CLE
	cmp	r1, #DR_SLOT_MASK_
	bhs	fill_done                       @ the last slot of a line is never written
	ldrh	r3, [r2], #2                    @ the opcode (guest words are big-endian)
	rev16	r3, r3
	str	r3, [r5, #S_OP]
	mov	r1, #0
	str	r1, [r5, #S_LOOPT]
	add	r4, r5, #ARENA_EXT
	ldrb	r4, [r4, r3]                    @ (src << 4) | dst extension modes
	str	r4, [r5, #S_E]
	lsr	lr, r4, #4                      @ lr = source mode
	and	r4, r4, #15                     @ r4 = destination mode
	movw	r1, #:lower16:fill_wtab
	movt	r1, #:upper16:fill_wtab
	ldrb	r3, [r1, lr]                    @ extension words of the source
	ldrb	r1, [r1, r4]                    @ ... of the destination
	add	r1, r1, r3
	add	r1, r1, #1                      @ n = guest words in the instruction
	str	r1, [r5, #S_N]
	ldr	r3, [r5, #S_SI]
	add	r3, r3, r1
	cmp	r3, #DR_SLOT_MASK_
	bhi	fill_done                       @ it would run into the last slot
	ldr	r4, [r5, #S_END]
	cmp	r3, r4
	bls	fill_fits                       @ inside this CLE
	ldr	r4, [r5, #S_H]
	cmp	r0, r4
	bne	fill_done                       @ only the instruction that was entered may spill into the next CLE: anything else is read-ahead (padding after an RTS, data), and its extension
						@ words would overwrite the slot of a real instruction there (a later jump to it found the leftovers of the ORI.B #imm that `0000` decodes to)
fill_fits:
	str	r0, [r5, #S_START]
	ldr	r1, [r5, #S_H]
	cmp	r0, r1
	bne	1f
	mov	r1, #1
	str	r1, [r5, #S_DONE]               @ this instruction starts at the entry slot
1:
	@ ---- calls, jumps and branches to a known address: JSR/JMP abs.L/abs.W, BSR.W. If the target can be linked
	@ (see fill_link) the instruction becomes `push the return address ; b target` (JSR/BSR) or `b target`: a
	@ direct branch, which the predictor handles, instead of the router's indirect one.
	ldr	r3, [r5, #S_OP]
	movw	r4, #0x4EB8
	subs	r4, r3, r4
	beq	ctl_w                           @ JSR abs.W
	cmp	r4, #1
	beq	ctl_l                           @ JSR abs.L
	movw	r4, #0x4EF8
	subs	r4, r3, r4
	beq	ctl_w                           @ JMP abs.W
	cmp	r4, #1
	beq	ctl_l                           @ JMP abs.L
	and	r4, r3, #0xFF00
	cmp	r4, #0x6100
	bne	1f
	ands	r4, r3, #0xFF
	beq	ctl_bsr                         @ BSR.W
	cmp	r4, #0xFF
	bne	ctl_bsrb                        @ BSR.B (0xFF would be the 68020's long form: not here)
1:	movw	r4, #0x4E75
	cmp	r3, r4
	beq	ctl_rts                         @ RTS
	b	fill_bcc
ctl_rts:                                @ the leader (pop the guest return address), then the shadow-stack return
	ldr	r4, [r5, #CPU_IRQ_WINDOWS]
	cmp	r4, #0
	bne	fill_bcc                        @ (interrupt windows are the router's)
	ldr	r3, [r5, #S_OP]
	add	r3, r5, r3, lsl #4
	ldr	r3, [r3]
	str	r3, [r0], #4                    @ ldr r0, [sp], #4
	movw	r1, #:lower16:pjit_rts_fast
	movt	r1, #:upper16:pjit_rts_fast
	sub	r3, r1, r0
	sub	r3, r3, #8
	asr	r3, r3, #2
	bic	r3, r3, #0xFF000000
	orr	r3, r3, #0xEA000000
	str	r3, [r0], #4                    @ b pjit_rts_fast
	b	fill_adv
ctl_l:	ldrh	r1, [r2]
	rev16	r1, r1
	ldrh	r4, [r2, #2]
	rev16	r4, r4
	orr	r1, r4, r1, lsl #16
	mov	r4, #4
	b	ctl_common
ctl_w:	ldrh	r1, [r2]
	rev16	r1, r1
	sxth	r1, r1
	mov	r4, #2
	b	ctl_common
ctl_bsrb:                               @ BSR.B: the displacement is the opcode's low byte, from the end of the opcode (r2)
	sxtb	r1, r3
	add	r1, r1, r2
	mov	r4, #0
	b	ctl_common
ctl_bsr:
	ldrh	r1, [r2]
	rev16	r1, r1
	sxth	r1, r1
	add	r1, r1, r2                      @ the displacement is from the extension word
	mov	r4, #2
ctl_common:                             @ r1 = the target, r4 = bytes of extension words
	str	r4, [r5, #S_T]
	bl	fill_link
	cmp	r1, #0
	beq	fill_bcc                        @ cannot link: the usual way
	ldr	r4, [r5, #S_T]
	ldr	r3, [r5, #S_OP]
	lsr	lr, r3, #8
	cmp	lr, #0x61
	beq	ctl_call                        @ BSR.W / BSR.B
	tst	r3, #0x40
	bne	ctl_emit_b                      @ JMP
ctl_call:                               @ push the return address (this instruction's end), 3 words
	.ifdef	PJIT_NOLINK_CALLS
	b	fill_bcc                        @ (experiment: calls stay unlinked, through the router's cores: what a tier that delays cross-linking until a target is hot pays)
	.endif
	add	lr, r2, r4                      @ r2 = this instruction + 2
	and	r3, lr, #0xF000
	lsl	r3, r3, #4
	lsl	r4, lr, #20
	lsr	r4, r4, #20                    @ (0xFFF is not an ARM immediate)
	orr	r3, r3, r4
	orr	r3, r3, #0xE3000000
	str	r3, [r0], #4                    @ movw r0, #lo
	lsr	lr, lr, #16
	and	r3, lr, #0xF000
	lsl	r3, r3, #4
	lsl	r4, lr, #20
	lsr	r4, r4, #20                    @ (0xFFF is not an ARM immediate)
	orr	r3, r3, r4
	orr	r3, r3, #0xE3000000
	orr	r3, r3, #0x00400000
	str	r3, [r0], #4                    @ movt r0, #hi
	@ the push, then the jump as a real `bl` that is the span's last word: the hardware return stack holds the next instruction, which is where
	@ pjit_rts_fast (a lookup by the popped guest address, ending in `bx lr`) goes, so the return predicts. No shadow stack.
	str	r1, [r5, #S_LKT]                @ the host target
	movw	r3, #0x0004
	movt	r3, #0xE52D
	str	r3, [r0], #4                    @ str r0, [sp, #-4]!
	ldr	r1, [r5, #S_START]
	ldr	r3, [r5, #S_N]
	add	r1, r1, r3, lsl #4
	sub	r1, r1, #4
	bl	fill_pad                        @ everything but the `bl`
	ldr	r1, [r5, #S_LKT]
	sub	r3, r1, r0
	sub	r3, r3, #8
	asr	r3, r3, #2
	bic	r3, r3, #0xFF000000
	orr	r3, r3, #0xEB000000
	str	r3, [r0], #4                    @ bl target
	ldr	r4, [r5, #S_T]
	add	r2, r2, r4
	b	fill_adv
ctl_emit_b:
	sub	r3, r1, r0
	sub	r3, r3, #8
	asr	r3, r3, #2
	bic	r3, r3, #0xFF000000
	orr	r3, r3, #0xEA000000
	str	r3, [r0], #4                    @ b target
	ldr	r4, [r5, #S_T]
	add	r2, r2, r4                      @ past the extension words
	b	fill_adv
fill_bcc:
	@ ---- a branch to a slot of this same line: a plain ARM branch to that slot. The line's
	@ tag is already claimed, and a slot not yet translated is a `bl pjit_fill`, so there is
	@ no router, no flag parking and no PC recovery; a taken loop costs one branch.
	ldr	r3, [r5, #S_OP]
	and	r4, r3, #0xF000
	cmp	r4, #0x6000
	bne	fill_dbf                        @ not a Bcc/BRA/BSR
	and	r4, r3, #0x0F00
	cmp	r4, #0x0100
	beq	fill_prefix                     @ BSR: it pushes a return address
	sxtb	r1, r3                          @ the displacement of the .B form ...
	ands	r4, r3, #0xFF
	bne	8f
	ldrh	r1, [r2]                        @ ... or of the .W form: the next word
	rev16	r1, r1
	sxth	r1, r1
8:	add	r1, r1, r2                      @ the target (the displacement is from the PC + 2)
	tst	r1, #1
	bne	fill_prefix                     @ odd: the router raises the address error
	sub	r4, r2, #2                      @ this instruction's own address
	eor	lr, r4, r1
	lsr	lr, lr, #DR_CL_SHIFT
	cmp	lr, #0
	bne	fill_bcc_far                    @ another line
	sub	r1, r1, r4                      @ target - here, in guest bytes: 4 host bytes each
	mov	r3, r1                          @ (kept: <= 0 means a backward branch)
	add	r4, r0, r1, lsl #3              @ the target slot's first word (8 host bytes per guest byte)
fill_b_host:
	str	r3, [r5, #S_T]                  @ (the displacement in bytes, for the loop compactor: <= 0 is a backward branch)
	sub	r1, r4, r0
	sub	r1, r1, #8
	asr	r1, r1, #2
	bic	r1, r1, #0xFF000000             @ the displacement field for the first word
	sub	lr, r1, #1
	bic	lr, lr, #0xFF000000             @ ... and for the second
	ldr	r4, [r5, #S_OP]
	lsr	r4, r4, #8
	and	r4, r4, #15                     @ the 68K condition
	cmp	r3, #0
	ble	fill_b_back
fill_b_plain:
	ldr	r3, [r5, #S_T]
	cmp	r3, #0
	bgt	1f
	add	r3, r3, r2
	sub	r3, r3, #2                      @ a backward branch: its target, for the loop compactor
	str	r3, [r5, #S_LOOPT]
1:	cmp	r4, #2
	beq	fill_b_hi
	cmp	r4, #3
	beq	fill_b_ls
	movw	r3, #:lower16:fill_cctab
	movt	r3, #:upper16:fill_cctab
	ldrb	r4, [r3, r4]                    @ the ARM condition
	orr	r1, r1, r4, lsl #28
	orr	r1, r1, #0x0A000000
	str	r1, [r0], #4                    @ b<cc> target
	movw	r1, #0xF000
	movt	r1, #0xE320
	str	r1, [r0], #4                    @ nop (never reached when taken; falls through otherwise)
	b	fill_b_done
fill_b_back:                            @ backward: through the loop core, which is an interrupt
	.if	PJIT_TIER_ON
	sub	r3, r4, #2
	cmp	r3, #1
	bls	fill_prefix                     @ HI and LS: the router
	movw	r3, #:lower16:fill_cctab
	movt	r3, #:upper16:fill_cctab
	ldrb	r4, [r3, r4]                    @ the ARM condition
	str	lr, [r5, #S_FZ0]                @ (the displacement field of the second word; the `b<cc>` ends up in the fourth: field - 2)
	ldr	r3, [r5, #S_T]
	add	r3, r3, r2
	sub	r3, r3, #2                      @ the guest target
	lsl	lr, r3, #20
	lsr	lr, lr, #20                     @ imm12
	and	r1, r3, #0xF000
	orr	r1, lr, r1, lsl #4              @ imm4 << 16 | imm12
	orr	r1, r1, #0xE3000000
	orr	r1, r1, #0x00001000
	str	r1, [r0], #4                    @ movw r1, #lo
	lsr	r3, r3, #16
	lsl	lr, r3, #20
	lsr	lr, lr, #20
	and	r1, r3, #0xF000
	orr	r1, lr, r1, lsl #4
	orr	r1, r1, #0xE3000000
	orr	r1, r1, #0x00400000
	orr	r1, r1, #0x00001000
	str	r1, [r0], #4                    @ movt r1, #hi
	movw	r1, #:lower16:pjit_d_bcount
	movt	r1, #:upper16:pjit_d_bcount
	sub	r1, r1, r0
	sub	r1, r1, #8
	asr	r1, r1, #2
	bic	r1, r1, #0xFF000000
	orr	r1, r1, r4, lsl #28
	orr	r1, r1, #0x0B000000
	str	r1, [r0], #4                    @ bl<cc> pjit_d_bcount
	ldr	lr, [r5, #S_FZ0]
	sub	lr, lr, #2
	bic	lr, lr, #0xFF000000
	orr	lr, lr, r4, lsl #28
	orr	lr, lr, #0x0A000000
	str	lr, [r0], #4                    @ b<cc> target
	b	fill_b_done
	.endif
	ldr	r3, [r5, #CPU_IRQ_WINDOWS]
	cmp	r3, #0
	beq	fill_b_plain                    @ windows off: no different from a forward branch
	sub	r3, r4, #2                      @ window. HI and LS need two words of their own: the router
	cmp	r3, #1
	bls	fill_prefix
	movw	r3, #:lower16:fill_cctab
	movt	r3, #:upper16:fill_cctab
	ldrb	r4, [r3, r4]
	movw	r1, #:lower16:pjit_d_loop
	movt	r1, #:upper16:pjit_d_loop
	sub	r1, r1, r0
	sub	r1, r1, #8
	asr	r1, r1, #2
	bic	r1, r1, #0xFF000000
	orr	r1, r1, r4, lsl #28
	orr	r1, r1, #0x0B000000
	str	r1, [r0], #4                    @ bl<cc> pjit_d_loop
	orr	lr, lr, r4, lsl #28
	orr	lr, lr, #0x0A000000
	str	lr, [r0], #4                    @ b<cc> target
	b	fill_b_done
fill_b_hi:                              @ HI = !C && !Z: skip the second word if C, else NE
	mov	r1, #0x2A000000                 @ bcs +0: over the next word
	str	r1, [r0], #4
	orr	lr, lr, #0x1A000000             @ bne target
	str	lr, [r0], #4
	b	fill_b_done
fill_b_ls:                              @ LS = C || Z
	orr	r1, r1, #0x2A000000             @ bcs target
	str	r1, [r0], #4
	orr	lr, lr, #0x0A000000             @ beq target
	str	lr, [r0], #4
fill_b_done:
	ldr	r3, [r5, #S_OP]
	ands	r3, r3, #0xFF
	addeq	r2, r2, #2                      @ the .W form's extension word
	b	fill_adv
fill_bcc_far:                           @ r1 = the target, in another line: link to it, or the router
	bl	fill_link
	cmp	r1, #0
	beq	fill_prefix
	mov	r4, r1
	mov	r3, #1
	b	fill_b_host
fill_dbf:
	@ ---- DBF Dn,<same line> (the DBRA loop), seven words, inline: load Dn's low word, decrement and store it, take the 0/1 "counter was
	@ zero" from the sign of the decremented value, then `add pc` over the final `b target` when done: no flags touched, no call; the loop is not inline when interrupt windows are on (the router is the window).
	ldr	r3, [r5, #S_OP]
	eor	r4, r3, #0x5100
	bic	r4, r4, #7
	cmp	r4, #0xC8
	bne	fill_prefix                     @ 0x51C8: DBF (the loop idiom); DBcc proper takes the router
	ldr	r4, [r5, #CPU_IRQ_WINDOWS]
	cmp	r4, #0
	bne	fill_prefix
	ldrh	r1, [r2]                        @ the displacement (from the extension word)
	rev16	r1, r1
	sxth	r1, r1
	add	r1, r1, r2                      @ the target
	tst	r1, #1
	bne	fill_prefix
	sub	r4, r2, #2                      @ this instruction's own address
	eor	lr, r4, r1
	lsr	lr, lr, #DR_CL_SHIFT
	cmp	lr, #0
	bne	fill_prefix                     @ another line
	cmp	r1, r4
	strlo	r1, [r5, #S_LOOPT]              @ a backward DBF: the loop compactor's
	sub	r1, r1, r4
	add	r1, r0, r1, lsl #3              @ the target slot's first word (host)
	mov	lr, r1                          @ (kept for the branch)
	and	r3, r3, #7
	.ifdef	PJIT_PIN_D0
	cmp	r3, #0
	bne	1f
	@ DBF D0: the counter is r4 itself. Eight words: the decremented value, the 'was zero' flag from the old low half (clz of 0 is 32), the low half written back, then the same
	@ `add pc` / nop / `b target` tail as below (no flags touched).
	movw	r3, #0x0001
	movt	r3, #0xE244
	str	r3, [r0], #4                    @ sub r0, r4, #1
	movw	r3, #0x1074
	movt	r3, #0xE6FF
	str	r3, [r0], #4                    @ uxth r1, r4
	movw	r3, #0x4010
	movt	r3, #0xE7CF
	str	r3, [r0], #4                    @ bfi r4, r0, #0, #16
	movw	r3, #0x1F11
	movt	r3, #0xE16F
	str	r3, [r0], #4                    @ clz r1, r1
	movw	r3, #0x12A1
	movt	r3, #0xE1A0
	str	r3, [r0], #4                    @ lsr r1, r1, #5       (1 when the counter went from 0 to -1: the loop is over)
	b	fill_dbf_tail
1:
	.endif
	lsl	r3, r3, #2
	rsb	r3, r3, #(-(CPU_D0) - 2)        @ the offset below r5 of Dn's low word: 62 - 4n
	and	r4, r3, #0xF0
	lsl	r4, r4, #4
	and	r1, r3, #0xF
	orr	r4, r4, r1                      @ imm4H:imm4L of the halfword access
	orr	r4, r4, #0xE1000000
	orr	r4, r4, #0x00550000
	orr	r4, r4, #0x000000B0             @ ldrh r0, [r5, #-off]
	str	r4, [r0], #4
	movw	r3, #0x0001
	movt	r3, #0xE240
	str	r3, [r0], #4                    @ sub r0, r0, #1       (no flags: the guest's are live)
	eor	r4, r4, #0x00100000             @ ldrh -> strh
	str	r4, [r0], #4                    @ strh r0, [r5, #-off]
	movw	r3, #0x1FA0
	movt	r3, #0xE1A0
	str	r3, [r0], #4                    @ lsr r1, r0, #31      (1 when the counter went from 0 to -1: the loop is over)
fill_dbf_tail:
	movw	r3, #0xF101
	movt	r3, #0xE08F
	str	r3, [r0], #4                    @ add pc, pc, r1, lsl #2: skips the `b` when the loop is over
	movw	r3, #0xF000
	movt	r3, #0xE320
	str	r3, [r0], #4                    @ nop (the slot pc + 8 skips)
	.if	PJIT_TIER_ON && defined_pin
	movw	r3, #:lower16:pjit_d_dbf_taken
	movt	r3, #:upper16:pjit_d_dbf_taken
	sub	r3, r3, r0
	sub	r3, r3, #8
	asr	r3, r3, #2
	bic	r3, r3, #0xFF000000
	orr	r3, r3, #0xEB000000
	str	r3, [r0], #4                    @ bl pjit_d_dbf_taken: the taken edge goes through the router (which counts the target: a hot DBF loop is promoted to a block)
	.else
	sub	r3, lr, r0
	sub	r3, r3, #8
	asr	r3, r3, #2
	bic	r3, r3, #0xFF000000
	orr	r3, r3, #0xEA000000
	str	r3, [r0], #4                    @ b target: the last word, so "done" falls into the next instruction
	.endif
	add	r2, r2, #2                      @ the extension word
	b	fill_adv
fill_prefix:
	@ ---- the extension-word prefix: source -> r1, destination -> r2. A constant 16-bit displacement that the leader only uses as the register
	@ offset of a load or store ([Rn, r1] / [Rn, r2], see fold_bits in tools/gen/maketables.py) is folded into the leader instead (fill_try_fold):
	@ `ldr r0, [sp, #16]` in place of `movw r1,#16 ; sxth r1,r1 ; ldr r0, [sp, r1]`.
	@ ---- MOVEM.L regs,(An) / regs,d16(An) / (An),regs / d16(An),regs with a few registers: the transfer in the slots (fill_movem_ea)
	ldr	r3, [r5, #S_OP]
	and	r4, r3, #0xFB00
	cmp	r4, #0x4800
	bne	5f
	and	r4, r3, #0xF8
	cmp	r4, #0xD0
	moveq	r1, #0                          @ r1 = the displacement: none for (An)
	beq	4f
	cmp	r4, #0xE8
	bne	5f
	ldrh	r1, [r2, #2]
	rev16	r1, r1
	sxth	r1, r1                          @ the displacement of d16(An)
	add	r4, r1, #4000
	cmp	r4, #8000
	bhi	5f                              @ (the offsets must fit the LDR / STR immediate)
4:	str	r1, [r5, #S_FZ4]
	ubfx	r1, r3, #10, #1
	str	r1, [r5, #S_FZ0]                @ 0: registers to memory, 1: memory to registers
	and	r1, r3, #7
	cmp	r1, #7
	addne	r1, r1, #6                      @ A0-A6 are r6-r12
	moveq	r1, #13                         @ A7 is the stack pointer
	str	r1, [r5, #S_FZ3]
	ldrh	r4, [r2]
	rev16	r4, r4                          @ the mask: D0 = bit 0 ... A7 = bit 15 (these modes are not reversed)
	bl	fill_movem_ea
	cmp	r1, #0
	beq	5f
	ldr	r3, [r5, #S_OP]
	and	r3, r3, #0x38
	cmp	r3, #0x28
	moveq	r1, #4
	movne	r1, #2
	add	r2, r2, r1                      @ past the mask (and the displacement)
	b	fill_adv
5:
	@ ---- MOVEM.L regs,-(A7) / MOVEM.L (A7)+,regs with a mask over D2-D7/A2-A6: nothing but a `bl` straight to that mask's routine in pjit_arena.movem (the
	@ mask is a constant: no prefix, no leader; the routine's own alignment guard hands a half-aligned stack on to the generic code with the mask)
	ldr	r3, [r5, #S_OP]
	eor	r4, r3, #0x4800
	cmp	r4, #0xE7
	moveq	lr, #0                          @ lr = 0: save (-(A7)), 1: restore ((A7)+)
	beq	1f
	eor	r4, r3, #0x4C00
	cmp	r4, #0xDF
	bne	6f
	mov	lr, #1
1:	ldrh	r4, [r2]
	rev16	r4, r4
	cmp	lr, #0
	bne	2f
	rbit	r4, r4
	lsr	r4, r4, #16                     @ -(An) masks are reversed: back to D0..A7 order
2:	str	lr, [r5, #S_FZ0]
	str	r4, [r5, #S_FZ2]
	bl	fill_movem_inline               @ a few registers: the whole transfer in the slots, no call (r1 = 1 when done)
	cmp	r1, #0
	beq	3f
	add	r2, r2, #2                      @ past the mask word
	b	fill_adv
3:	ldr	lr, [r5, #S_FZ0]
	ldr	r4, [r5, #S_FZ2]
	bic	r1, r4, #0x7C00
	bic	r1, r1, #0xFC
	cmp	r1, #0
	bne	6f                              @ other registers in it: the generic code
	and	r1, r4, #0xFC
	lsr	r1, r1, #2
	and	r4, r4, #0x7C00
	lsr	r4, r4, #10
	orr	r1, r1, r4, lsl #6              @ the packed mask
	add	r1, r5, r1, lsl #7
	add	r1, r1, #ARENA_MOVEM
	add	r1, r1, lr, lsl #18             @ + 0x40000 for the restore half
	str	r1, [r5, #S_T]
	bl	fill_pad_last                   @ the `bl` is the span's last word: lr = the next instruction
	ldr	r1, [r5, #S_T]
	sub	r3, r1, r0
	sub	r3, r3, #8
	asr	r3, r3, #2
	bic	r3, r3, #0xFF000000
	orr	r3, r3, #0xEB000000             @ bl the routine
	str	r3, [r0], #4
	add	r2, r2, #2                      @ past the mask word
	b	fill_adv
6:
	mov	r1, #0
	str	r1, [r5, #S_FOLD]               @ 0: the leader goes in as it is
	ldr	r4, [r5, #S_E]
	lsr	r4, r4, #4
	cmp	r4, #0
	beq	2f
	mov	r1, #1                          @ the role: the source, whose register is r1
	bl	fill_try_fold
	cmp	r1, #0
	bne	2f
	ldr	r4, [r5, #S_E]
	lsr	r4, r4, #4
	mov	r1, #0x1000
	mov	r3, #0
	bl	fill_emit_ea
2:	ldr	r4, [r5, #S_E]
	and	r4, r4, #15
	cmp	r4, #0
	beq	3f
	mov	r1, #2                          @ the destination, r2
	bl	fill_try_fold
	cmp	r1, #0
	bne	3f
	ldr	r4, [r5, #S_E]
	and	r4, r4, #15
	mov	r1, #0x2000
	mov	r3, #1
	bl	fill_emit_ea
3:
	@ ---- a split entry (pjit_arena.form[op] = n > 0): the opcode's own n words from the DT, verbatim, then the shared routine from the OT (the body code below).
	ldr	r3, [r5, #S_OP]
	movw	r4, #:lower16:ARENA_FORM
	movt	r4, #:upper16:ARENA_FORM
	add	r4, r4, r5
	ldrb	r4, [r4, r3]
	cmp	r4, #0
	beq	9f
	add	r3, r5, r3, lsl #4
	ldr	r1, [r3], #4
	ldr	lr, [r5, #S_FOLD]
	cmp	lr, #0
	movne	r1, lr                          @ the first word with its displacement folded in (fill_try_fold), as for a leader
	str	r1, [r0], #4
	subs	r4, r4, #1
	beq	fill_body
2:	ldr	r1, [r3], #4
	str	r1, [r0], #4
	subs	r4, r4, #1
	bne	2b
	b	fill_body
9:
	@ ---- the leader: DT word 0, verbatim. An svc leader goes at the start of the instruction's last slot (the
	@ exception code takes the slot's guest word + 2 as the end of the instruction); any other follows the prefix.
	ldr	r3, [r5, #S_OP]
	add	r3, r5, r3, lsl #4
	ldr	r3, [r3]
	and	r4, r3, #0x0F000000
	cmp	r4, #0x0F000000
	bne	4f
	ldr	r1, [r5, #S_START]
	ldr	r4, [r5, #S_N]
	sub	r4, r4, #1
	add	r1, r1, r4, lsl #4
	bl	fill_pad
	ldr	r3, [r5, #S_OP]
	add	r3, r5, r3, lsl #4
	ldr	r3, [r3]
4:	ldr	r4, [r5, #S_FOLD]
	cmp	r4, #0
	movne	r3, r4                          @ the leader with its displacement folded in
	str	r3, [r0], #4
	@ ---- MOVEM.L regs,-(A7) / MOVEM.L (A7)+,regs with a mask over D2-D7/A2-A6: `bl` straight to that mask's
	@ routine in pjit_arena.movem (the mask is a constant here; the prefix above has also left it in r1, which the
	@ routine's alignment guard hands on to the generic code if the stack is only 2-byte aligned)
	ldr	r3, [r5, #S_OP]
	eor	r4, r3, #0x4800
	cmp	r4, #0xE7
	moveq	lr, #0                          @ lr = 0: save (-(A7)), 1: restore ((A7)+)
	beq	1f
	eor	r4, r3, #0x4C00
	cmp	r4, #0xDF
	bne	8f
	mov	lr, #1
1:	ldrh	r4, [r2, #-2]                   @ the mask word (the prefix consumed it)
	rev16	r4, r4
	cmp	lr, #0
	bne	2f
	rbit	r4, r4
	lsr	r4, r4, #16                     @ -(An) masks are reversed: back to D0..A7 order
2:	bic	r1, r4, #0x7C00
	bic	r1, r1, #0xFC
	cmp	r1, #0
	bne	8f                              @ other registers in it: the generic code
	and	r1, r4, #0xFC
	lsr	r1, r1, #2
	and	r4, r4, #0x7C00
	lsr	r4, r4, #10
	orr	r1, r1, r4, lsl #6              @ the packed mask
	add	r1, r5, r1, lsl #7
	add	r1, r1, #ARENA_MOVEM
	add	r1, r1, lr, lsl #18             @ + 0x40000 for the restore half
	str	r1, [r5, #S_T]
	bl	fill_pad_last                   @ the `bl` is the span's last word: lr = the next instruction
	ldr	r1, [r5, #S_T]
	sub	r3, r1, r0
	sub	r3, r3, #8
	asr	r3, r3, #2
	bic	r3, r3, #0xFF000000
	orr	r3, r3, #0xEB000000             @ bl the routine
	str	r3, [r0], #4
	b	fill_adv
8:
	@ ---- JSR (An) / JSR d16(An): the return address is known now, so the span is `<prefix> ; <leader: r1 = the target> ; movw r0 ; movt r0 ; bl pjit_jsr_ind`,
	@ the `bl` its last word (lr = the next instruction exactly, no rounding). The core pushes, records the call and falls into the router.
	ldr	r3, [r5, #S_OP]
	bic	r4, r3, #7
	movw	r1, #0x4E90
	cmp	r4, r1
	beq	1f
	movw	r1, #0x4EA8
	cmp	r4, r1
	bne	7f
1:	ldr	r1, [r5, #S_START]
	ldr	r3, [r5, #S_N]
	add	r1, r1, r3, lsl #4              @ the host return address: the end of this span
	sub	r3, r1, r0
	cmp	r3, #12
	blo	7f                              @ no room for movw ; movt ; bl
	sub	r1, r1, #12
	bl	fill_pad                        @ (clobbers r1, r3, r4; r2 stays)
	and	r3, r2, #0xF000
	lsl	r3, r3, #4
	lsl	r4, r2, #20
	lsr	r4, r4, #20                     @ (0xFFF is not an ARM immediate)
	orr	r3, r3, r4
	orr	r3, r3, #0xE3000000
	str	r3, [r0], #4                    @ movw r0, #lo
	lsr	lr, r2, #16
	and	r3, lr, #0xF000
	lsl	r3, r3, #4
	lsl	r4, lr, #20
	lsr	r4, r4, #20
	orr	r3, r3, r4
	orr	r3, r3, #0xE3000000
	orr	r3, r3, #0x00400000
	str	r3, [r0], #4                    @ movt r0, #hi
	movw	r1, #:lower16:pjit_jsr_ind
	movt	r1, #:upper16:pjit_jsr_ind
	sub	r3, r1, r0
	sub	r3, r3, #8
	asr	r3, r3, #2
	bic	r3, r3, #0xFF000000
	orr	r3, r3, #0xEB000000
	str	r3, [r0], #4                    @ bl pjit_jsr_ind
	b	fill_adv
7:
fill_body:
	@ ---- the body. The offset table names its DRSR entry (F).
	ldr	r3, [r5, #S_OP]
	ldr	r1, [r5, #S_XOT]
	add	r4, r5, #ARENA_OT
	add	r4, r4, r1                      @ (the exact twin on an exact line)
	add	r4, r4, r3, lsl #1
	ldrh	r4, [r4]                        @ the OT entry
	lsr	r4, r4, #2
	ldr	r1, [r5, #CPU_DRSR]
	add	r1, r1, r4, lsl #DRSR_SHIFT_    @ r1 = the DRSR entry (F)
	str	r1, [r5, #S_T]
	@ ... a straight-line body that fits what is left of the span (after the prefix and the leader) is copied in:
	@ no `bl`, no `bx lr` (pjit_arena.inl, built by tools/gen/maketables.py)
	ldr	lr, [r5, #S_XINL]
	add	r4, r5, #ARENA_INL
	add	r4, r4, lr
	ldrb	r4, [r4, r3]
	cmp	r4, #0
	beq	9f
	sub	r4, r4, #1                      @ its length in words, not counting the final `bx lr`
	ldr	r3, [r5, #S_START]
	ldr	lr, [r5, #S_N]
	add	r3, r3, lr, lsl #4
	sub	r3, r3, r0
	asr	r3, r3, #2                      @ the words left in this instruction's span
	cmp	r4, r3
	bhi	9f
	cmp	r4, #0
	beq	fill_adv
7:	ldr	r3, [r1], #4
	str	r3, [r0], #4
	subs	r4, r4, #1
	bne	7b
	b	fill_adv
9:	@ ... a one-word body is copied (a branch relocated), anything longer is `bl`'d
	ldr	r3, [r1, #4]
	movw	lr, #:lower16:DRSR_PAD_
	movt	lr, #:upper16:DRSR_PAD_
	cmp	r3, lr
	beq	fill_inline
	.ifdef	PJIT_BL_LAST
	bl	fill_pad_last
	ldr	r1, [r5, #S_T]
	sub	r3, r1, r0
	sub	r3, r3, #8
	asr	r3, r3, #2
	bic	r3, r3, #0xFF000000
	orr	r3, r3, #0xEB000000             @ bl entry
	str	r3, [r0], #4
	b	fill_adv
	.else
	bl	fill_pad_slot                   @ (the `bl` goes in the span's last slot, right after the leader)
	ldr	r1, [r5, #S_T]
	sub	r3, r1, r0
	sub	r3, r3, #8
	asr	r3, r3, #2
	bic	r3, r3, #0xFF000000
	orr	r3, r3, #0xEB000000             @ bl entry
	str	r3, [r0], #4
	@ nops after the `bl`, up to the end of the span: the body's `bx lr` runs through them into the next instruction. That is the
	@ same number of nops as before the `bl`, but it moves the `bl` off a fixed address (12 mod 16 put every site in half of the
	@ Cortex-A8's branch target buffer); anything that reads lr as "the next instruction" rounds it up to 16 (CORE_ENTER, the svc entry).
	ldr	r1, [r5, #S_START]
	ldr	r3, [r5, #S_N]
	add	r1, r1, r3, lsl #4
	movw	r4, #0xF000
	movt	r4, #0xE320                     @ nop
1:	cmp	r0, r1
	strlo	r4, [r0], #4
	blo	1b
	b	fill_adv
	.endif

fill_inline:                            @ a one-word body: copy it, relocated
	ldr	r3, [r1]
	bic	r4, r3, #0xF0000000
	movw	lr, #0xFF1E
	movt	lr, #0x012F
	cmp	r4, lr
	beq	fill_nop                        @ bx lr (any condition): falls through to the next slot
	and	r4, r3, #0x0F000000
	cmp	r4, #0x0A000000
	bne	fill_copy
	lsr	r4, r3, #28
	cmp	r4, #15
	beq	fill_copy                       @ (a BLX: not a plain branch)
	bl	fill_pad_last                   @ a `bl` must be the span's last word (see above)
	ldr	r1, [r5, #S_T]
	ldr	r3, [r1]
	lsl	r4, r3, #8
	asr	r4, r4, #6                      @ the branch's displacement, sign-extended, in bytes
	add	r4, r4, r1
	sub	r4, r4, r0                      @ F + disp - O: the same target from the new address
	lsr	r4, r4, #2
	bic	r4, r4, #0xFF000000
	and	r3, r3, #0xF0000000             @ keep the condition
	orr	r3, r3, r4
	orr	r3, r3, #0x0B000000             @ B -> BL: the trampoline reads its return PC from LR
	str	r3, [r0], #4
	b	fill_adv
fill_nop:
	movw	r3, #0xF000
	movt	r3, #0xE320
fill_copy:
	str	r3, [r0], #4

fill_adv:                               @ pad to the end of the span, then on to the next instruction
	ldr	r1, [r5, #S_START]
	ldr	r3, [r5, #S_N]
	add	r1, r1, r3, lsl #4              @ where this instruction's slots end
	bl	fill_pad
	ldr	r1, [r5, #S_SI]
	ldr	r3, [r5, #S_N]
	add	r1, r1, r3
	str	r1, [r5, #S_SI]
	ldr	r1, [r5, #S_LOOPT]
	cmp	r1, #0
	.if	PJIT_COMPACT
	blne	compact_try                     @ this was a backward branch: a loop?
	.endif
	b	fill_next

@ Pads with nops from r0 up to r1 (a nop or two is cheaper than a branch; more than two words get a branch over
@ the rest). Clobbers r3, r4; r0 advances. fill_pad_last pads so that the next word written is the last of the
@ instruction's span (a `bl` to a shared body must be: lr is then the start of the next slot, which is how the
@ exception and branch cores find the guest PC). Clobbers r1, r3, r4, lr.
@ Folds a constant displacement into the leader. In: r1 = the role (1: the source, whose offset register is r1; 2: the destination, r2), r4 = that
@ operand's extension mode, r2 = the guest cursor at its extension word. Out: r1 = 1 if it was folded (the patched leader is in S_FOLD, r2 is past the
@ word), else r1 = 0 and nothing has changed. A word / byte load or store reaches +-4095, a halfword / signed byte one +-255.
fill_try_fold:
	cmp	r4, #1                          @ EXT_I16: d16(An), absolute short, immediate word
	bne	9f
	str	lr, [r5, #S_LKR]
	ldr	r3, [r5, #S_OP]
	movw	r4, #:lower16:ARENA_XCONS
	movt	r4, #:upper16:ARENA_XCONS
	add	r4, r4, r5
	ldrb	r4, [r4, r3]                    @ bit 5: the leader's offset register is r1; bit 6: r2 (maketables fold_bits)
	tst	r4, r1, lsl #5
	beq	7f
	add	r3, r5, r3, lsl #4
	ldr	r4, [r3]                        @ the leader
	and	r3, r4, #15
	cmp	r3, r1
	bne	7f
	ldrh	r3, [r2]
	rev16	r3, r3
	sxth	r3, r3                          @ the displacement
	cmp	r3, #0
	rsblt	lr, r3, #0
	movge	lr, r3                          @ lr = its size
	and	r1, r4, #0x0E000000
	cmp	r1, #0x06000000
	bne	5f
	tst	r4, #0x0FF0                     @ ldr / str / ldrb / strb: no shift ...
	bne	7f
	and	r1, r4, #0x01200000
	cmp	r1, #0x01000000                 @ ... P = 1, W = 0
	bne	7f
	cmp	lr, #4096
	bhs	7f
	bic	r4, r4, #0x02000000             @ the immediate form
	bic	r4, r4, #0x00800000
	bic	r4, r4, #0x0F00
	bic	r4, r4, #0x00FF
	orr	r4, r4, lr
	b	6f
5:	movw	r1, #0x0F90                     @ ldrh / strh / ldrsb / ldrsh with a register offset
	movt	r1, #0x0E40
	and	r1, r4, r1
	cmp	r1, #0x90
	bne	7f
	tst	r4, #0x60                       @ (SH = 00 would be SWP)
	beq	7f
	and	r1, r4, #0x01200000
	cmp	r1, #0x01000000
	bne	7f
	cmp	lr, #256
	bhs	7f
	bic	r4, r4, #0x00800000
	bic	r4, r4, #0x0F00
	bic	r4, r4, #0x000F
	orr	r4, r4, #0x00400000
	and	r1, lr, #0xF0
	orr	r4, r4, r1, lsl #4              @ imm4H
	and	r1, lr, #0x0F
	orr	r4, r4, r1                      @ imm4L
6:	cmp	r3, #0
	orrge	r4, r4, #0x00800000             @ U: add (the displacement is not negative)
	str	r4, [r5, #S_FOLD]
	add	r2, r2, #2                      @ the extension word
	mov	r1, #1
	ldr	lr, [r5, #S_LKR]
	bx	lr
7:	ldr	lr, [r5, #S_LKR]
9:	mov	r1, #0
	bx	lr

@ Pads (as fill_pad does) from r0 up to the start of this instruction's last slot, if the prefix and leader have not got there yet.
@ MOVEM.L with a constant mask and A7 as the base, a few registers (2 words per D register, 1 per A register: at most the 8 words of the span, A7 not in it):
@ the transfer itself, as single LDR / STR, in the slots. No call, no alignment guard (single accesses do not fault on a half-aligned stack). Save order is
@ A6..A0 then D7..D0 (-(A7)), restore D0..D7 then A0..A6; D registers go in pairs (both loads, then both stores). A0-A6 are r6-r12; Dn is at [r5, #-(64 - 4n)].
@ In: r4 = the mask in D0..A7 order, S_FZ0 = 0 (save) / 1 (restore), r0 = the host cursor. Out: r1 = 1 when emitted (the span padded with nops), else 0.
@ Clobbers r1, r3, r4, lr and the S_FZ slots; r2 is untouched.
fill_movem_inline:
	str	lr, [r5, #S_FZ1]
	tst	r4, #0x8000
	bne	9f
	mov	r3, #0                          @ r3 = words needed
	and	r1, r4, #0xFF
1:	cmp	r1, #0
	beq	2f
	sub	lr, r1, #1
	and	r1, r1, lr
	add	r3, r3, #2
	b	1b
2:	lsr	r1, r4, #8
3:	cmp	r1, #0
	beq	4f
	sub	lr, r1, #1
	and	r1, r1, lr
	add	r3, r3, #1
	b	3b
4:	cmp	r3, #8
	bhi	9f
	ldr	r1, [r5, #S_FZ0]
	cmp	r1, #0
	bne	20f
	@ ---- save
	mov	r3, #6
5:	lsr	r1, r4, #8
	lsr	r1, r1, r3
	tst	r1, #1
	beq	6f
	movw	r1, #0x0004
	movt	r1, #0xE52D
	orr	r1, r1, r3, lsl #12
	add	r1, r1, #0x6000                 @ str r(6+k), [sp, #-4]!
	str	r1, [r0], #4
6:	subs	r3, r3, #1
	bpl	5b
	mov	lr, #0xFF                       @ lr = a D register waiting for its partner (0xFF: none)
	mov	r3, #7
7:	lsr	r1, r4, r3
	tst	r1, #1
	beq	8f
	cmp	lr, #0xFF
	moveq	lr, r3
	beq	8f
	rsb	r1, lr, #16
	lsl	r1, r1, #2
	orr	r1, r1, #0xE5000000
	orr	r1, r1, #0x150000
	str	r1, [r0], #4                    @ ldr r0, [r5, #-(64 - 4 * first)]
	rsb	r1, r3, #16
	lsl	r1, r1, #2
	orr	r1, r1, #0xE5000000
	orr	r1, r1, #0x150000
	orr	r1, r1, #0x1000
	str	r1, [r0], #4                    @ ldr r1, [r5, #-(64 - 4 * second)]
	movw	r1, #0x0004
	movt	r1, #0xE52D
	str	r1, [r0], #4                    @ str r0, [sp, #-4]!
	orr	r1, r1, #0x1000
	str	r1, [r0], #4                    @ str r1, [sp, #-4]!
	mov	lr, #0xFF
8:	subs	r3, r3, #1
	.ifdef	PJIT_PIN_D0
	bhi	7b                              @ D7..D1 (the pinned D0 is pushed last, from r4)
	.else
	bpl	7b
	.endif
	cmp	lr, #0xFF
	.ifdef	PJIT_PIN_D0
	beq	31f
	.else
	beq	30f
	.endif
	rsb	r1, lr, #16
	lsl	r1, r1, #2
	orr	r1, r1, #0xE5000000
	orr	r1, r1, #0x150000
	str	r1, [r0], #4                    @ ldr r0, [r5, #-(64 - 4n)]
	movw	r1, #0x0004
	movt	r1, #0xE52D
	str	r1, [r0], #4                    @ str r0, [sp, #-4]!
	.ifdef	PJIT_PIN_D0
31:	tst	r4, #1                          @ D0: str r4, [sp, #-4]!
	beq	30f
	movw	r1, #0x4004
	movt	r1, #0xE52D
	str	r1, [r0], #4
	.endif
	b	30f
	@ ---- restore
20:	mov	lr, #0xFF
	.ifdef	PJIT_PIN_D0
	tst	r4, #1                          @ D0 first, straight into r4: ldr r4, [sp], #4
	beq	26f
	movw	r1, #0x4004
	movt	r1, #0xE49D
	str	r1, [r0], #4
26:	mov	r3, #1
	.else
	mov	r3, #0
	.endif
21:	lsr	r1, r4, r3
	tst	r1, #1
	beq	22f
	cmp	lr, #0xFF
	moveq	lr, r3
	beq	22f
	movw	r1, #0x0004
	movt	r1, #0xE49D
	str	r1, [r0], #4                    @ ldr r0, [sp], #4
	orr	r1, r1, #0x1000
	str	r1, [r0], #4                    @ ldr r1, [sp], #4
	rsb	r1, lr, #16
	lsl	r1, r1, #2
	orr	r1, r1, #0xE5000000
	orr	r1, r1, #0x050000
	str	r1, [r0], #4                    @ str r0, [r5, #-(64 - 4 * first)]
	rsb	r1, r3, #16
	lsl	r1, r1, #2
	orr	r1, r1, #0xE5000000
	orr	r1, r1, #0x050000
	orr	r1, r1, #0x1000
	str	r1, [r0], #4                    @ str r1, [r5, #-(64 - 4 * second)]
	mov	lr, #0xFF
22:	add	r3, r3, #1
	cmp	r3, #8
	blo	21b
	cmp	lr, #0xFF
	beq	23f
	movw	r1, #0x0004
	movt	r1, #0xE49D
	str	r1, [r0], #4                    @ ldr r0, [sp], #4
	rsb	r1, lr, #16
	lsl	r1, r1, #2
	orr	r1, r1, #0xE5000000
	orr	r1, r1, #0x050000
	str	r1, [r0], #4                    @ str r0, [r5, #-(64 - 4n)]
23:	mov	r3, #0
24:	lsr	r1, r4, #8
	lsr	r1, r1, r3
	tst	r1, #1
	beq	25f
	movw	r1, #0x0004
	movt	r1, #0xE49D
	orr	r1, r1, r3, lsl #12
	add	r1, r1, #0x6000                 @ ldr r(6+k), [sp], #4
	str	r1, [r0], #4
25:	add	r3, r3, #1
	cmp	r3, #7
	blo	24b
30:	ldr	r1, [r5, #S_START]
	ldr	r3, [r5, #S_N]
	add	r1, r1, r3, lsl #4
	bl	fill_pad                        @ nops up to the end of the span
	mov	r1, #1
	ldr	lr, [r5, #S_FZ1]
	bx	lr
9:	mov	r1, #0
	ldr	lr, [r5, #S_FZ1]
	bx	lr

@ The same for a base register other than the stack's, no pre/post-decrement: MOVEM.L regs,(An) / regs,d16(An) / (An),regs / d16(An),regs. The registers go in the
@ order D0..D7, A0..A6 at ascending addresses from base + displacement, D registers in pairs. A7 in the mask, or (loading) the base register itself in it, take the
@ generic code. In: r4 = the mask (D0..A7 order), S_FZ0 = 0 (to memory) / 1 (to registers), S_FZ3 = the base register's number, S_FZ4 = the displacement.
@ Out: r1 = 1 when emitted. Clobbers r1, r3, r4, lr and S_FZ1/2/5, S_XS0/1; r2 stays.
fill_movem_memop:                       @ r1 = Rt, r3 = 0x00100000 (a load) or 0: writes `ldr/str Rt, [base, #off]`, off += 4
	str	lr, [r5, #S_FZ2]
	ldr	lr, [r5, #S_FZ4]
	orr	r3, r3, #0xE5000000
	orr	r3, r3, r1, lsl #12
	ldr	r1, [r5, #S_FZ3]
	orr	r3, r3, r1, lsl #16
	cmp	lr, #0
	orrge	r3, r3, #0x00800000             @ U: add the offset
	movge	r1, lr
	rsblt	r1, lr, #0
	orr	r3, r3, r1
	add	lr, lr, #4
	str	lr, [r5, #S_FZ4]
	str	r3, [r0], #4
	ldr	lr, [r5, #S_FZ2]
	bx	lr
fill_movem_dreg:                        @ r1 = D register n, r3 = Rt: `str Rt, [r5, #-(64 - 4n)]` (memory to registers) or `ldr Rt, ...` (registers to memory)
	str	lr, [r5, #S_FZ2]
	.ifdef	PJIT_PIN_D0
	cmp	r1, #0
	bne	1f
	ldr	lr, [r5, #S_FZ0]                @ the pinned D0 is r4: `mov r4, Rt` (memory to registers) or `mov Rt, r4` (registers to memory)
	cmp	lr, #0
	movne	r1, #0xE1000000
	orrne	r1, r1, #0x00A00000
	orrne	r1, r1, #0x00004000
	orrne	r1, r1, r3                      @ mov r4, Rt
	moveq	r1, #0xE1000000
	orreq	r1, r1, #0x00A00000
	orreq	r1, r1, #4
	orreq	r1, r1, r3, lsl #12             @ mov Rt, r4
	str	r1, [r0], #4
	ldr	lr, [r5, #S_FZ2]
	bx	lr
1:
	.endif
	lsl	r1, r1, #2
	rsb	r1, r1, #64
	orr	r1, r1, #0xE5000000
	orr	r1, r1, r3, lsl #12
	ldr	lr, [r5, #S_FZ0]
	cmp	lr, #0
	orrne	r1, r1, #0x00050000
	orreq	r1, r1, #0x00150000
	str	r1, [r0], #4
	ldr	lr, [r5, #S_FZ2]
	bx	lr
fill_movem_flush:                       @ the D register waiting for a partner (S_FZ5, 0xFF: none), on its own
	ldr	r1, [r5, #S_FZ5]
	cmp	r1, #0xFF
	bxeq	lr
	str	lr, [r5, #S_XS1]
	ldr	r3, [r5, #S_FZ0]
	cmp	r3, #0
	bne	1f
	mov	r3, #0
	bl	fill_movem_dreg                 @ ldr r0, [file]
	mov	r1, #0
	mov	r3, #0
	bl	fill_movem_memop                @ str r0, [base, #off]
	b	2f
1:	mov	r1, #0
	mov	r3, #0x00100000
	bl	fill_movem_memop                @ ldr r0, [base, #off]
	ldr	r1, [r5, #S_FZ5]
	mov	r3, #0
	bl	fill_movem_dreg                 @ str r0, [file]
2:	mov	r1, #0xFF
	str	r1, [r5, #S_FZ5]
	ldr	lr, [r5, #S_XS1]
	bx	lr
fill_movem_ea:
	str	lr, [r5, #S_FZ1]
	tst	r4, #0x8000
	bne	9f
	ldr	r1, [r5, #S_FZ0]
	cmp	r1, #0
	beq	2f
	ldr	r1, [r5, #S_FZ3]                @ loading: the base register must not be in the list
	subs	r1, r1, #6
	bmi	2f
	cmp	r1, #6
	bhi	2f
	lsr	r3, r4, #8
	lsr	r3, r3, r1
	tst	r3, #1
	bne	9f
2:	mov	r3, #0                          @ words needed
	and	r1, r4, #0xFF
11:	cmp	r1, #0
	beq	12f
	sub	lr, r1, #1
	and	r1, r1, lr
	add	r3, r3, #2
	b	11b
12:	lsr	r1, r4, #8
13:	cmp	r1, #0
	beq	14f
	sub	lr, r1, #1
	and	r1, r1, lr
	add	r3, r3, #1
	b	13b
14:	ldr	r1, [r5, #S_N]
	lsl	r1, r1, #2
	cmp	r3, r1
	bhi	9f
	mov	r1, #0xFF
	str	r1, [r5, #S_FZ5]
15:	cmp	r4, #0                          @ the registers, lowest first: D0..D7, then A0..A6
	beq	30f
	rbit	r1, r4
	clz	r1, r1                          @ k = the lowest set bit
	mov	r3, #1
	bic	r4, r4, r3, lsl r1
	cmp	r1, #8
	bhs	20f
	ldr	r3, [r5, #S_FZ5]
	cmp	r3, #0xFF
	streq	r1, [r5, #S_FZ5]                @ the first of a pair
	beq	15b
	str	r1, [r5, #S_XS0]                @ the second: the pair is r3, S_XS0
	ldr	r1, [r5, #S_FZ0]
	cmp	r1, #0
	bne	17f
	mov	r1, r3
	mov	r3, #0
	bl	fill_movem_dreg                 @ ldr r0, [file: first]
	ldr	r1, [r5, #S_XS0]
	mov	r3, #1
	bl	fill_movem_dreg                 @ ldr r1, [file: second]
	mov	r1, #0
	mov	r3, #0
	bl	fill_movem_memop                @ str r0, [base, #off]
	mov	r1, #1
	mov	r3, #0
	bl	fill_movem_memop                @ str r1, [base, #off + 4]
	b	18f
17:	mov	r1, #0
	mov	r3, #0x00100000
	bl	fill_movem_memop                @ ldr r0, [base, #off]
	mov	r1, #1
	mov	r3, #0x00100000
	bl	fill_movem_memop                @ ldr r1, [base, #off + 4]
	ldr	r1, [r5, #S_FZ5]
	mov	r3, #0
	bl	fill_movem_dreg                 @ str r0, [file: first]
	ldr	r1, [r5, #S_XS0]
	mov	r3, #1
	bl	fill_movem_dreg                 @ str r1, [file: second]
18:	mov	r1, #0xFF
	str	r1, [r5, #S_FZ5]
	b	15b
20:	str	r1, [r5, #S_XS0]                @ an A register (k = 8 + n): a lone D register before it goes first
	bl	fill_movem_flush
	ldr	r1, [r5, #S_FZ0]
	cmp	r1, #0
	movne	r3, #0x00100000
	moveq	r3, #0
	ldr	r1, [r5, #S_XS0]
	sub	r1, r1, #2                      @ An is r(6 + n)
	bl	fill_movem_memop
	b	15b
30:	bl	fill_movem_flush
	ldr	r1, [r5, #S_START]
	ldr	r3, [r5, #S_N]
	add	r1, r1, r3, lsl #4
	bl	fill_pad
	mov	r1, #1
	ldr	lr, [r5, #S_FZ1]
	bx	lr
9:	mov	r1, #0
	ldr	lr, [r5, #S_FZ1]
	bx	lr

@ ---- loop compaction ----------------------------------------------------------------------------------------------------------------------------------
@ A backward branch (Bcc, BRA, DBF) to a slot of its own line has just been translated: the instructions from its target to it may be a loop. If every one of them
@ is in the cache already, is plain (its code is its slots' words, or a `bl` to its own body in the DRSR) and the branches among them go to other instructions of the
@ loop or to slots of this line, the loop is copied into a block in pjit_arena.fuse: the instructions' words back to back (no padding nops, no `b` over padding), the
@ bodies that can be (cinl, and pjit_crsr: the body without its `bx lr`, a conditional one turned into a branch to the end) inlined, the branches among the instructions
@ retargeted to the block, and `b <the slot after the loop>` after the last one. The slot of the loop's first instruction becomes `b block`; every other slot is as it
@ was, so a jump into the middle of the loop still works, and a line that is evicted takes its only way into the block with it.
@ Scratch (pjit_arena.comp): [2i] = an instruction's first slot word, [2i+1] = its first word in the block; [48 + 2k] = a branch word of the block to patch, [49 + 2k]
@ = its target slot; [120] first slot of the loop, [121] the slot after it, [122] its line's first byte, [123] the block's write cursor, [124] instructions emitted,
@ [125] patches, [126] the first instruction's slots, [127] blocks made.
@ In: r0 = host cursor, r2 = guest cursor (after the branch), r1 = S_LOOPT. Preserves r0 and r2. E=LE. Clobbers r1, r3, r4, lr and S_* scratch.
	.macro	COMP_ADDR rd, off
	movw	\rd, #:lower16:(ARENA_COMP + \off * 4)
	movt	\rd, #:upper16:(ARENA_COMP + \off * 4)
	add	\rd, \rd, r5
	.endm
	.macro	HOST_OF rd, rg                  @ the first slot word of guest address \rg (clobbers lr)
	ubfx	\rd, \rg, #0, #DR_SPAN_BITS
	ldr	lr, [r5, #CPU_CACHE_DATA]
	add	\rd, lr, \rd, lsl #3
	.endm
compact_try:
	str	lr, [r5, #S_FZ1]
	str	r0, [r5, #S_XS0]
	str	r2, [r5, #S_XS1]
	str	r1, [r5, #S_FZ0]                @ the loop's first instruction (guest)
	mov	r3, #0
	str	r3, [r5, #S_LOOPT]
	ldr	r3, [r5, #S_N]
	sub	r4, r2, r3, lsl #1
	str	r4, [r5, #S_FZ5]                @ the closing branch's own address
	cmp	r4, r1
	bls	c_fail
	sub	r3, r4, r1
	cmp	r3, #80
	bhi	c_fail                          @ (at most 40 halfwords)
	COMP_ADDR r0, 0
	HOST_OF	r3, r1
	str	r3, [r0, #(120 * 4)]            @ the first slot of the loop
	ldr	r2, [r5, #S_XS1]
	HOST_OF	r3, r2
	str	r3, [r0, #(121 * 4)]            @ the slot after it
	ldr	r4, [r5, #S_FZ5]
	HOST_OF	r3, r4
	lsr	r3, r3, #DR_HL_SHIFT_
	lsl	r3, r3, #DR_HL_SHIFT_
	str	r3, [r0, #(122 * 4)]            @ its line's first byte (the cache is aligned to its lines)
	mov	r3, #0
	str	r3, [r0, #(124 * 4)]
	str	r3, [r0, #(125 * 4)]
	str	r3, [r0, #(115 * 4)]            @ branches out of the loop, seen in pass 1
	str	r3, [r0, #(116 * 4)]            @ DBF Dn closes it: n + 1
	ldr	r4, [r5, #S_FZ5]
	ldrh	r3, [r4]
	rev16	r3, r3
	bic	r1, r3, #7
	movw	lr, #0x51C8
	cmp	r1, lr
	andeq	r3, r3, #7
	addeq	r3, r3, #1
	movne	r3, #0
	str	r3, [r0, #(116 * 4)]
	@ ---- pass 1: every instruction must be translated and plain; count the words of the block
	ldr	r1, [r5, #S_FZ0]
	str	r1, [r5, #S_FZ2]
	str	r3, [r5, #S_FZ3]
	str	r3, [r5, #S_FZ4]
c_p1:
	ldr	r1, [r5, #S_FZ2]
	bl	c_info
	cmp	r1, #0
	bne	c_fail
	bl	c_words
	cmp	r1, #0
	blt	c_fail
	ldr	r3, [r5, #S_FZ3]
	add	r3, r3, r1
	cmp	r3, #110
	bhi	c_fail
	str	r3, [r5, #S_FZ3]
	ldr	r3, [r5, #S_FZ4]
	add	r3, r3, #1
	cmp	r3, #24
	bhi	c_fail
	str	r3, [r5, #S_FZ4]
	ldr	r1, [r5, #S_FZ2]
	ldr	r4, [r5, #S_FZ5]
	cmp	r1, r4
	beq	c_p1_done                       @ that was the closing branch
	ldr	r3, [r5, #S_LK0]
	add	r1, r1, r3, lsl #1
	cmp	r1, r4
	bhi	c_fail                          @ (the walk overshot the branch: the decode did not land on it)
	str	r1, [r5, #S_FZ2]
	b	c_p1
c_p1_done:
	@ ---- the block
	movw	r1, #:lower16:ARENA_FUSE_NEXT
	movt	r1, #:upper16:ARENA_FUSE_NEXT
	add	r1, r1, r5
	ldr	r3, [r1]
	ldr	r4, [r5, #S_FZ3]
	add	r4, r4, #(3 + 15)
	bic	r4, r4, #15                     @ words, with a prologue word, a spare and the exit branch, in whole 16-word blocks
	add	r4, r4, r3
	movw	lr, #:lower16:(FUSE_LIMIT + 16)
	cmp	r4, lr
	bhi	c_fail
	str	r4, [r1]
	add	r1, r1, #64
	add	r1, r1, r3, lsl #2              @ the block
	movw	r3, #0xF000
	movt	r3, #0xE320
	str	r3, [r1]                        @ (the prologue: a nop unless c_dbf finds a use for it)
	COMP_ADDR r0, 0
	str	r1, [r0, #(119 * 4)]            @ the block; its first word is a prologue (the counter of a DBF loop, see c_dbf), the instructions follow
	add	r1, r1, #4
	str	r1, [r0, #(123 * 4)]
	ldr	r1, [r5, #S_FZ0]
	str	r1, [r5, #S_FZ2]
	mov	r3, #0
	str	r3, [r5, #S_FZ4]
c_p2:
	ldr	r1, [r5, #S_FZ2]
	bl	c_info
	bl	c_emit
	cmp	r1, #0
	bne	c_fail
	ldr	r1, [r5, #S_FZ2]
	ldr	r4, [r5, #S_FZ5]
	cmp	r1, r4
	beq	c_p2_done
	ldr	r3, [r5, #S_LK0]
	add	r1, r1, r3, lsl #1
	str	r1, [r5, #S_FZ2]
	b	c_p2
c_p2_done:
	@ ---- the exit: `b <the slot after the loop>`, which must be a written slot of this line (not its last)
	ldr	r2, [r5, #S_XS1]
	ldr	r4, [r5, #S_FZ5]
	eor	r1, r2, r4
	lsr	r1, r1, #DR_CL_SHIFT
	cmp	r1, #0
	bne	c_fail
	ubfx	r1, r2, #1, #(DR_CL_SHIFT - 1)
	cmp	r1, #DR_SLOT_MASK_
	bhs	c_fail
	COMP_ADDR r0, 0
	ldr	r1, [r0, #(123 * 4)]
	str	r1, [r0, #(117 * 4)]            @ the spare word (a nop; the DBF counter's store when it is kept in r4)
	movw	r3, #0xF000
	movt	r3, #0xE320
	str	r3, [r1], #4
	ldr	r3, [r0, #(121 * 4)]
	sub	r3, r3, r1
	sub	r3, r3, #8
	asr	r3, r3, #2
	bic	r3, r3, #0xFF000000
	orr	r3, r3, #0xEA000000
	str	r3, [r1], #4
	str	r1, [r0, #(123 * 4)]
	@ ---- the branches among the instructions
	mov	r3, #0
	str	r3, [r5, #S_CLL]
c_patch:
	COMP_ADDR r0, 0
	ldr	r3, [r5, #S_CLL]
	ldr	r4, [r0, #(125 * 4)]
	cmp	r3, r4
	bhs	c_patched
	add	r1, r0, #(48 * 4)
	add	r1, r1, r3, lsl #3
	ldr	r2, [r1]                        @ the branch word (in the block)
	ldr	r1, [r1, #4]                    @ its target slot
	ldr	r4, [r0, #(124 * 4)]            @ instructions
c_find:
	subs	r4, r4, #1
	blt	c_fail
	ldr	lr, [r0, r4, lsl #3]
	cmp	lr, r1
	bne	c_find
	add	lr, r0, r4, lsl #3
	ldr	lr, [lr, #4]                    @ that instruction's first word in the block
	sub	lr, lr, r2
	sub	lr, lr, #8
	asr	lr, lr, #2
	bic	lr, lr, #0xFF000000
	ldr	r4, [r2]
	orr	r4, r4, lr
	str	r4, [r2]
	ldr	r3, [r5, #S_CLL]
	add	r3, r3, #1
	str	r3, [r5, #S_CLL]
	b	c_patch
c_patched:
	bl	c_dbf                           @ a DBF loop keeps its counter in r4 when it can
	@ ---- the loop's first slot jumps to the block; the rest of that instruction's slots are nops; flush both
	COMP_ADDR r0, 0
	ldr	r2, [r0, #(120 * 4)]
	ldr	r3, [r0, #(126 * 4)]
	ldr	r1, [r0, #(119 * 4)]
	sub	r1, r1, r2
	sub	r1, r1, #8
	asr	r1, r1, #2
	bic	r1, r1, #0xFF000000
	orr	r1, r1, #0xEA000000
	str	r1, [r2]
	movw	r4, #0xF000
	movt	r4, #0xE320
	add	r1, r2, r3, lsl #4
	add	r2, r2, #4
c_nops:
	cmp	r2, r1
	strlo	r4, [r2], #4
	blo	c_nops
	ldr	r1, [r0, #(120 * 4)]
	ldr	r2, [r0, #(126 * 4)]
	add	r3, r1, r2, lsl #4
	FLUSH_RANGE r1, r3, r4
	COMP_ADDR r0, 0
	ldr	r1, [r0, #(119 * 4)]
	ldr	r3, [r0, #(123 * 4)]
	FLUSH_RANGE r1, r3, r4
	COMP_ADDR r0, 0
	ldr	r1, [r0, #(127 * 4)]
	add	r1, r1, #1
	str	r1, [r0, #(127 * 4)]
c_fail:
	ldr	lr, [r5, #S_FZ1]
	ldr	r0, [r5, #S_XS0]
	ldr	r2, [r5, #S_XS1]
	bx	lr

@ One instruction of the loop, from guest address r1: S_T = its opcode, S_LK0 = its length in slots, S_LK1 = its first slot word, S_LKT = the end of its code (its slots'
@ words without the padding nops and the `b` over them). Out: r1 = 0, or 1 if the instruction is not in the cache yet.
c_info:
	str	lr, [r5, #S_LKR]
	ldrh	r3, [r1]
	rev16	r3, r3
	str	r3, [r5, #S_T]
	add	r4, r5, #ARENA_EXT
	ldrb	r4, [r4, r3]
	lsr	lr, r4, #4
	and	r4, r4, #15
	movw	r2, #:lower16:fill_wtab
	movt	r2, #:upper16:fill_wtab
	ldrb	lr, [r2, lr]
	ldrb	r4, [r2, r4]
	add	lr, lr, r4
	add	lr, lr, #1                      @ n
	str	lr, [r5, #S_LK0]
	ubfx	r2, r1, #0, #DR_SPAN_BITS
	ldr	r3, [r5, #CPU_CACHE_DATA]
	add	r2, r3, r2, lsl #3              @ H
	str	r2, [r5, #S_LK1]
	ldr	r3, [r2]
	movw	r4, #:lower16:pjit_fill
	movt	r4, #:upper16:pjit_fill
	sub	r4, r4, r2
	sub	r4, r4, #8
	asr	r4, r4, #2
	bic	r4, r4, #0xFF000000
	orr	r4, r4, #0xEB000000
	cmp	r3, r4
	beq	ci_cold                         @ still `bl pjit_fill`
	add	r1, r2, lr, lsl #4              @ Eh, the end of the span
	mov	r3, r1                          @ p
	movw	r4, #0xF000
	movt	r4, #0xE320                     @ nop
	.macro	STRIP_NOPS
1:	cmp	r3, r2
	bls	2f
	ldr	r0, [r3, #-4]
	cmp	r0, r4
	subeq	r3, r3, #4
	beq	1b
2:
	.endm
	STRIP_NOPS
	cmp	r3, r2
	bls	ci_done
	ldr	r0, [r3, #-4]
	lsr	lr, r0, #24
	cmp	lr, #0xEA
	bne	ci_done                         @ not a `b`
	lsl	lr, r0, #8
	asr	lr, lr, #6
	sub	r0, r3, #4
	add	r0, r0, lr
	add	r0, r0, #8                      @ its target
	cmp	r0, r1
	bne	ci_done                         @ not the `b` over the padding
	sub	r3, r3, #4
	STRIP_NOPS
ci_done:
	str	r3, [r5, #S_LKT]
	mov	r1, #0
	ldr	lr, [r5, #S_LKR]
	bx	lr
ci_cold:
	mov	r1, #1
	ldr	lr, [r5, #S_LKR]
	bx	lr

@ Is the instruction (c_info's) fit for a block, and how many words does it add? Out: r1 = the words, or -1.
c_words:
	str	lr, [r5, #S_CLR]
	ldr	r2, [r5, #S_LK1]
	ldr	r3, [r5, #S_LKT]
	ldr	r4, [r5, #S_LK0]
	add	r4, r2, r4, lsl #4              @ the end of its span
	sub	r0, r3, r2
	lsr	r0, r0, #2
	str	r0, [r5, #S_CLL]                @ the words so far: its own
	mov	r0, #0
	str	r0, [r5, #S_CLT]                @ 1 once a branch inside its own span has been seen
cw_loop:
	cmp	r2, r3
	bhs	cw_done
	ldr	r0, [r2], #4
	lsr	r1, r0, #25
	and	r1, r1, #7
	cmp	r1, #5
	bne	cw_other
	lsr	r1, r0, #28
	cmp	r1, #15
	beq	cwf1
	lsl	r1, r0, #8
	asr	r1, r1, #6
	add	r1, r1, r2
	add	r1, r1, #4                      @ the target
	tst	r0, #0x01000000
	bne	cw_bl
	ldr	lr, [r5, #S_LK1]
	cmp	r1, lr
	blo	cw_notown
	cmp	r1, r4
	bhs	cw_notown
	cmp	r1, r3
	bhi	cwf2
	mov	r0, #1
	str	r0, [r5, #S_CLT]
	b	cw_loop
cw_notown:
	COMP_ADDR r0, 120
	ldr	lr, [r0]
	cmp	r1, lr
	blo	cw_exit
	ldr	lr, [r0, #4]
	cmp	r1, lr
	blo	cw_loop                         @ among the loop's instructions: found when the branches are resolved
cw_exit:
	COMP_ADDR r0, 115
	ldr	lr, [r0]
	add	lr, lr, #1
	str	lr, [r0]
	COMP_ADDR r0, 122
	ldr	lr, [r0]
	sub	lr, r1, lr
	cmp	lr, #(1 << DR_HL_SHIFT_)
	bhs	cwf3
	b	cw_loop
cw_other:
	and	r1, r0, #0x0F000000
	cmp	r1, #0x0F000000
	beq	cwf4
	bic	r1, r0, #0xF0000000
	bic	r1, r1, #0x2F
	movw	lr, #0xFF10
	movt	lr, #0x012F
	cmp	r1, lr
	beq	cwf5
	b	cw_loop
cw_bl:                                  @ r1 = the target: it must be this opcode's own body, and the last word
	cmp	r2, r3
	bne	cwf6
	ldr	lr, [r5, #S_T]
	ldr	r0, [r5, #S_XOT]
	add	r0, r0, lr, lsl #1
	add	r0, r0, r5
	add	r0, r0, #ARENA_OT
	ldrh	r0, [r0]
	lsr	r0, r0, #2
	ldr	lr, [r5, #CPU_DRSR]
	add	lr, lr, r0, lsl #DRSR_SHIFT_
	cmp	r1, lr
	bne	cwf7
	ldr	lr, [r5, #S_T]
	movw	r0, #:lower16:ARENA_CINL
	movt	r0, #:upper16:ARENA_CINL
	add	r0, r0, r5
	ldr	r1, [r5, #S_XOT]
	cmp	r1, #0
	addne	r0, r0, #0x10000                @ cinl_x follows cinl
	ldrb	r0, [r0, lr]
	ands	r1, r0, #0x7F
	bne	cw_inline
	tst	r0, #0x80
	beq	cwf8
	b	cw_done                         @ called from the block
cw_inline:
	ldr	r0, [r5, #S_CLT]
	cmp	r0, #0
	bne	cwf9
	ldr	r0, [r5, #S_CLL]
	add	r0, r0, r1
	sub	r0, r0, #2                      @ its words, less the `bl`, plus the body (r1 = length + 1)
	str	r0, [r5, #S_CLL]
cw_done:
	ldr	r1, [r5, #S_CLL]
	ldr	lr, [r5, #S_CLR]
	bx	lr
cw_fail:
	mvn	r1, #0
	ldr	lr, [r5, #S_CLR]
	bx	lr

@ Emits the instruction (c_info's) into the block. Out: r1 = 0, or 1 on a failure.
cwf1:
	mov	r0, #1
	b	cw_fail_rec
cwf2:
	mov	r0, #2
	b	cw_fail_rec
cwf3:
	mov	r0, #3
	b	cw_fail_rec
cwf4:
	mov	r0, #4
	b	cw_fail_rec
cwf5:
	mov	r0, #5
	b	cw_fail_rec
cwf6:
	mov	r0, #6
	b	cw_fail_rec
cwf7:
	mov	r0, #7
	b	cw_fail_rec
cwf8:
	mov	r0, #8
	b	cw_fail_rec
cwf9:
	mov	r0, #9
	b	cw_fail_rec
cw_fail_rec:
	COMP_ADDR r1, 118
	str	r0, [r1]
	b	cw_fail
c_emit:
	str	lr, [r5, #S_CLR]
	COMP_ADDR r0, 0
	ldr	r1, [r0, #(124 * 4)]
	cmp	r1, #24
	bhs	ce_fail
	ldr	r3, [r5, #S_LK1]
	ldr	r4, [r0, #(123 * 4)]
	add	lr, r0, r1, lsl #3
	str	r3, [lr]
	str	r4, [lr, #4]
	cmp	r1, #0
	ldreq	r3, [r5, #S_LK0]
	streq	r3, [r0, #(126 * 4)]
	add	r1, r1, #1
	str	r1, [r0, #(124 * 4)]
	str	r1, [r5, #S_FZ4]
	ldr	r2, [r5, #S_LK1]
	ldr	r3, [r5, #S_LKT]
ce_loop:
	cmp	r2, r3
	bhs	ce_ok
	ldr	r1, [r2], #4
	lsr	lr, r1, #25
	and	lr, lr, #7
	cmp	lr, #5
	bne	ce_word
	lsl	lr, r1, #8
	asr	lr, lr, #6
	add	lr, lr, r2
	add	lr, lr, #4                      @ the target
	tst	r1, #0x01000000
	bne	ce_bl
	ldr	r0, [r5, #S_LK1]
	cmp	lr, r0
	blo	ce_notown
	ldr	r4, [r5, #S_LK0]
	add	r4, r0, r4, lsl #4
	cmp	lr, r4
	bhs	ce_notown
	b	ce_word                         @ inside this instruction's own span: as it is
ce_notown:
	COMP_ADDR r0, 120
	ldr	r4, [r0]
	cmp	lr, r4
	blo	ce_exit
	ldr	r4, [r0, #4]
	cmp	lr, r4
	bhs	ce_exit
	COMP_ADDR r0, 0                     @ among the loop's instructions: a patch for later
	ldr	r4, [r0, #(125 * 4)]
	cmp	r4, #24
	bhs	ce_fail
	add	r4, r4, #1
	str	r4, [r0, #(125 * 4)]
	sub	r4, r4, #1
	add	r0, r0, #(48 * 4)
	add	r0, r0, r4, lsl #3
	str	lr, [r0, #4]
	COMP_ADDR r4, 0
	ldr	r4, [r4, #(123 * 4)]
	str	r4, [r0]
	bic	r1, r1, #0xFFFFFF
	b	ce_word
ce_exit:                                @ out of the loop, to a slot of this line: the same target from the new place
	COMP_ADDR r0, 0
	ldr	r4, [r0, #(123 * 4)]
	sub	lr, lr, r4
	sub	lr, lr, #8
	asr	lr, lr, #2
	bic	lr, lr, #0xFF000000
	and	r1, r1, #0xFF000000
	orr	r1, r1, lr
ce_word:
	COMP_ADDR r0, 0
	ldr	r4, [r0, #(123 * 4)]
	str	r1, [r4], #4
	str	r4, [r0, #(123 * 4)]
	b	ce_loop
ce_bl:                                  @ lr = the opcode's body: copy it (without its `bx lr`) or call it from the block
	str	lr, [r5, #S_LKT]
	ldr	r0, [r5, #S_T]
	movw	r4, #:lower16:ARENA_CINL
	movt	r4, #:upper16:ARENA_CINL
	add	r4, r4, r5
	ldr	r3, [r5, #S_XOT]
	cmp	r3, #0
	addne	r4, r4, #0x10000
	ldrb	r4, [r4, r0]
	ands	r3, r4, #0x7F
	bne	ce_inline
	COMP_ADDR r0, 0
	ldr	r4, [r0, #(123 * 4)]
	ldr	lr, [r5, #S_LKT]
	sub	lr, lr, r4
	sub	lr, lr, #8
	asr	lr, lr, #2
	bic	lr, lr, #0xFF000000
	and	r1, r1, #0xFF000000
	orr	r1, r1, lr
	b	ce_word
ce_inline:
	sub	r3, r3, #1                      @ the body's words
	ldr	lr, [r5, #S_LKT]
	ldr	r0, [r5, #CPU_DRSR]
	sub	lr, lr, r0
	movw	r0, #:lower16:pjit_crsr
	movt	r0, #:upper16:pjit_crsr
	add	lr, lr, r0                      @ the same body in pjit_crsr
	COMP_ADDR r0, 0
	ldr	r4, [r0, #(123 * 4)]
ce_copy:
	cmp	r3, #0
	beq	ce_copied
	ldr	r1, [lr], #4
	str	r1, [r4], #4
	sub	r3, r3, #1
	b	ce_copy
ce_copied:
	str	r4, [r0, #(123 * 4)]
ce_ok:
	mov	r1, #0
	ldr	lr, [r5, #S_CLR]
	bx	lr
ce_fail:
	mov	r1, #1
	ldr	lr, [r5, #S_CLR]
	bx	lr

@ A DBF Dn loop whose body neither touches Dn's low word nor r4, and that has no way out but its end, keeps the counter in r4: `ldrh r4, [Dn]` in the block's prologue,
@ `sub r4, r4, #1 ; lsr r1, r4, #31 ; ldr pc, [pc, r1, lsl #2]` (a literal table of the two targets, stored byte-swapped: the load is big-endian) closing it, `strh r4, [Dn]` in
@ the exit. One indirect branch, no load, no store per trip. The branch is predicted by its last target.
c_dbf:
	str	lr, [r5, #S_CLR]
	.ifdef	PJIT_PIN_D0
	b	cd_no                           @ the compacted loop keeps the DBF counter in r4, which is D0 now
	.endif
	COMP_ADDR r0, 0
	ldr	r1, [r0, #(116 * 4)]
	cmp	r1, #0
	beq	cd_no                           @ not closed by a DBF
	ldr	r2, [r0, #(115 * 4)]
	cmp	r2, #0
	bne	cd_no                           @ a way out in the middle: the counter would be stale there
	sub	r1, r1, #1                      @ n
	lsl	r2, r1, #2
	sub	r2, r2, #64
	str	r2, [r5, #S_LKT]                @ Dn's word, as an offset from r5 (negative)
	rsb	r1, r2, #0
	sub	r1, r1, #2                      @ 62 - 4n: the low halfword's
	and	r4, r1, #0xF0
	lsl	r4, r4, #4
	and	r2, r1, #0xF
	orr	r4, r4, r2
	orr	r4, r4, #0xE1000000
	orr	r4, r4, #0x00550000
	orr	r4, r4, #0x000000B0             @ ldrh r0, [r5, #-off]
	str	r4, [r5, #S_T]
	ldr	r2, [r0, #(124 * 4)]
	sub	r2, r2, #1
	add	r3, r0, r2, lsl #3
	ldr	r3, [r3, #4]                    @ the DBF's seven words in the block: they must be exactly what fill_dbf wrote
	ldr	r2, [r3]
	cmp	r2, r4
	bne	cd_no
	ldr	r2, [r3, #4]
	movw	lr, #0x0001
	movt	lr, #0xE240                     @ sub r0, r0, #1
	cmp	r2, lr
	bne	cd_no
	ldr	r2, [r3, #8]
	eor	lr, r4, #0x00100000             @ strh r0, [r5, #-off]
	cmp	r2, lr
	bne	cd_no
	ldr	r2, [r3, #12]
	movw	lr, #0x1FA0
	movt	lr, #0xE1A0                     @ lsr r1, r0, #31
	cmp	r2, lr
	bne	cd_no
	ldr	r2, [r3, #16]
	movw	lr, #0xF101
	movt	lr, #0xE08F                     @ add pc, pc, r1, lsl #2
	cmp	r2, lr
	bne	cd_no
	ldr	r2, [r3, #24]
	lsr	lr, r2, #24
	cmp	lr, #0xEA
	bne	cd_no                           @ b top
	@ the instructions before it: nothing may use r4, and nothing may touch Dn's word
	ldr	r2, [r0, #(119 * 4)]
	add	r2, r2, #4
cd_scan:
	cmp	r2, r3
	bhs	cd_ok
	ldr	lr, [r2], #4
	ubfx	r4, lr, #12, #4
	cmp	r4, #4
	beq	cd_no
	ubfx	r4, lr, #16, #4
	cmp	r4, #4
	beq	cd_no
	cmp	r4, #5
	bne	cd_scan                         @ not through r5
	ubfx	r4, lr, #25, #3
	cmp	r4, #2
	beq	cd_sdt                          @ ldr / str / ldrb / strb, immediate offset
	cmp	r4, #0
	bne	cd_no                           @ (a register offset, a load / store multiple, a branch...)
	and	r4, lr, #0x90
	cmp	r4, #0x90
	bne	cd_no                           @ (data processing on r5)
	tst	lr, #0x00400000
	beq	cd_no
	and	r4, lr, #0xF
	lsr	r1, lr, #4
	and	r1, r1, #0xF0
	orr	r4, r4, r1                      @ the halfword forms' 8-bit offset
	b	cd_off
cd_sdt:
	ubfx	r4, lr, #0, #12
cd_off:
	tst	lr, #0x00800000
	rsbeq	r4, r4, #0                      @ U = 0: it is subtracted
	ldr	r1, [r5, #S_LKT]
	sub	r1, r1, #4
	cmp	r4, r1
	ble	cd_scan                         @ the access is wholly below Dn's word ...
	add	r1, r1, #8
	cmp	r4, r1
	bge	cd_scan                         @ ... or wholly above it
	b	cd_no
cd_ok:
	ldr	r4, [r5, #S_T]
	orr	r2, r4, #0x4000                 @ ldrh r4, [r5, #-off]: the block's prologue
	ldr	r1, [r0, #(119 * 4)]
	str	r2, [r1]
	movw	r2, #0x4001
	movt	r2, #0xE244                     @ sub r4, r4, #1
	str	r2, [r3]
	movw	r2, #0x1FA4
	movt	r2, #0xE1A0                     @ lsr r1, r4, #31
	str	r2, [r3, #4]
	movw	r2, #0xF101
	movt	r2, #0xE79F                     @ ldr pc, [pc, r1, lsl #2]
	str	r2, [r3, #8]
	movw	r2, #0xF000
	movt	r2, #0xE320                     @ nop: the word pc + 4 steps over
	str	r2, [r3, #12]
	str	r2, [r3, #24]
	ldr	r1, [r0, #4]                    @ the first instruction's word in the block: the loop's top
	rev	r1, r1
	str	r1, [r3, #16]                   @ [pc + 0], big-endian
	ldr	r1, [r0, #(117 * 4)]            @ the exit's spare word: `strh r4, [Dn]`, then the branch out
	rev	r2, r1
	str	r2, [r3, #20]                   @ [pc + 4]
	eor	r4, r4, #0x00100000
	orr	r4, r4, #0x4000                 @ strh r4, [r5, #-off]
	str	r4, [r1]
cd_no:
	ldr	lr, [r5, #S_CLR]
	bx	lr

fill_pad_slot:
	str	lr, [r5, #S_LKR]
	ldr	r1, [r5, #S_START]
	ldr	r3, [r5, #S_N]
	add	r1, r1, r3, lsl #4
	sub	r1, r1, #16
	cmp	r0, r1
	bhs	1f
	bl	fill_pad
1:	ldr	lr, [r5, #S_LKR]
	bx	lr
fill_pad_last:
	str	lr, [r5, #S_LKR]
	ldr	r1, [r5, #S_START]
	ldr	r3, [r5, #S_N]
	add	r1, r1, r3, lsl #4
	sub	r1, r1, #4
	bl	fill_pad
	ldr	lr, [r5, #S_LKR]
	bx	lr
fill_pad:
	sub	r3, r1, r0
	asr	r3, r3, #2                      @ words to write
	cmp	r3, #3
	blo	2f
	sub	r4, r3, #2
	orr	r4, r4, #0xEA000000             @ b over the rest
	str	r4, [r0], #4
2:	movw	r4, #0xF000
	movt	r4, #0xE320                     @ the architectural NOP hint: `mov r0,r0` would wait on r0
3:	cmp	r0, r1
	strlo	r4, [r0], #4
	blo	3b
	bx	lr

fill_skip:                              @ an untranslatable instruction: leave its slots cold
	ldr	r1, [r5, #S_N]
	sub	r3, r1, #1
	add	r2, r2, r3, lsl #1              @ past its extension words
	add	r0, r0, r1, lsl #4
	ldr	r3, [r5, #S_SI]
	add	r3, r3, r1
	str	r3, [r5, #S_SI]
	b	fill_next

fill_done:
	ldr	r1, [r5, #S_H]
	cmp	r0, r1
	bls	fill_nothing                    @ nothing was written
	FLUSH_RANGE r1, r0, r3
fill_nothing:
	ldr	r1, [r5, #S_DONE]
	cmp	r1, #0
	beq	fill_fallback
	ldr	r2, [r5, #S_H]
	ldr	r1, [r5, #CPU_JMP_CPSR]
	setend	be
	.ifdef	PJIT_PIN_D0
	UNPARK_BE
	.endif
	msr	cpsr_f, r1
	bx	r2                              @ run what was just translated

fill_fallback:                          @ the entry instruction could not be translated:
	ldr	r4, [r5, #S_G0]                 @ interpret it, then come back (LR = pjit_resume)
	ldr	r1, [r5, #CPU_JMP_CPSR]
	movw	lr, #:lower16:pjit_resume
	movt	lr, #:upper16:pjit_resume
	setend	be
	str	r4, [r5, #CPU_IPC]
	.ifdef	PJIT_PIN_D0
	UNPARK_BE
	.endif
	msr	cpsr_f, r1
	b	pjit_prefetch


	.ltorg

@ words of extension a mode takes: none, I16, I32, (d8,An,Xn), (d16,PC), (d8,PC,Xn), BDISP
fill_wtab:
	.byte	0, 1, 2, 1, 1, 1, 1, 0
@ 68K condition -> ARM condition (HI and LS have none: 2 and 3 are never looked up)
fill_cctab:
	.byte	0xE, 0xF, 0, 0, 3, 2, 1, 0, 7, 6, 5, 4, 0xA, 0xB, 0xC, 0xD
	.balign	4

@ ---- one extension word (or two): the code that leaves its value in r1 / r2 ----------
@ in:  r4 = mode (ext_t), r1 = Rd << 12 (0x1000 or 0x2000), r3 = 0 (source) / 1 (destination)
@      r0 = host cursor, r2 = guest cursor (at the extension word); lr = return
@ out: r0, r2 advanced. Clobbers r1, r3, r4.
@ The instructions written are exactly what the C translator emitted:
@     I16     movw rd, #imm ; sxth rd, rd              BDISP / D16PC  movw ; movt  (32-bit)
@     I32     movw rd, #lo ; movt rd, #hi            d8,An,Xn       mov rd, #d8 ; index add (see ea_index)
@ \base 0xE3000000 | (kind 0x40<<16 for movt) | (imm >> 12) << 16 | rd << 12 | imm & 0xFFF.
	.macro	ENC16 movt
	and	r4, r3, #0xF000
	lsl	r4, r4, #4
	orr	r4, r4, r1
	bic	r3, r3, #0xF000
	orr	r3, r3, r4
	orr	r3, r3, #0xE3000000
	.if \movt
	orr	r3, r3, #0x00400000
	.endif
	.endm

fill_emit_ea:
	str	r3, [r5, #S_SEL]
	cmp	r4, #1
	beq	ea_i16
	cmp	r4, #2
	beq	ea_i32
	cmp	r4, #3
	beq	ea_d8
	cmp	r4, #4
	beq	ea_d16pc
	cmp	r4, #5
	beq	ea_d8pc
	cmp	r4, #6
	beq	ea_bdisp
	bx	lr

ea_i16:
	ldrh	r3, [r2], #2
	rev16	r3, r3
	ENC16	0
	str	r3, [r0], #4                    @ movw rd, #imm16
	lsr	r3, r1, #12
	orr	r3, r3, r1
	orr	r3, r3, #0xE6000000
	orr	r3, r3, #0x00BF0000
	orr	r3, r3, #0x70
	str	r3, [r0], #4                    @ sxth rd, rd
	bx	lr

ea_i32:
	ldrh	r3, [r2], #2
	rev16	r3, r3
	str	r3, [r5, #S_T]                  @ the high word
	ldrh	r3, [r2], #2
	rev16	r3, r3                          @ the low word
	ENC16	0
	str	r3, [r0], #4                    @ movw rd, #lo
	ldr	r3, [r5, #S_T]
	ENC16	1
	str	r3, [r0], #4                    @ movt rd, #hi
	bx	lr

ea_bdisp:                               @ Bcc/BRA.W: sign-extended displacement - 2
	ldrh	r3, [r2], #2
	rev16	r3, r3
	sxth	r3, r3
	sub	r3, r3, #2
	b	ea_const32
ea_d16pc:                               @ (d16,PC): the extension word's own address + d16
	ldrh	r3, [r2], #2
	rev16	r3, r3
	sxth	r3, r3
	sub	r4, r2, #2
	add	r3, r3, r4
ea_const32:                             @ r3 = a 32-bit constant for rd: movw ; movt
	str	r3, [r5, #S_T]
	uxth	r3, r3
	ENC16	0
	str	r3, [r0], #4
	ldr	r3, [r5, #S_T]
	lsr	r3, r3, #16
	ENC16	1
	str	r3, [r0], #4
	bx	lr

ea_d8:                                  @ (d8,An,Xn): mov rd, #d8 ; the index register added in line
	ldrh	r3, [r2], #2
	rev16	r3, r3
	str	r3, [r5, #S_LKT]                @ the whole brief extension word
	sxtb	r3, r3
	cmp	r3, #0
	blt	1f
	orr	r3, r3, r1
	orr	r3, r3, #0xE3000000
	orr	r3, r3, #0x00A00000             @ mov rd, #d8
	b	2f
1:	mvn	r3, r3
	and	r3, r3, #0xFF
	orr	r3, r3, r1
	orr	r3, r3, #0xE3000000
	orr	r3, r3, #0x00E00000             @ mvn rd, #~d8
2:	str	r3, [r0], #4
	b	ea_index
ea_d8pc:                                @ (d8,PC,Xn): the extension word's address + d8 as a constant, then the index
	ldrh	r3, [r2], #2
	rev16	r3, r3
	str	r3, [r5, #S_LKT]
	sxtb	r3, r3
	sub	r4, r2, #2
	add	r3, r3, r4
	str	r3, [r5, #S_T]
	uxth	r3, r3
	ENC16	0
	str	r3, [r0], #4                    @ movw rd, #lo
	ldr	r3, [r5, #S_T]
	lsr	r3, r3, #16
	ENC16	1
	str	r3, [r0], #4                    @ movt rd, #hi
@ rd += the index register of the brief extension word in S_LKT: bit 15 = A/D, 14:12 = the register, bit 11 =
@ long (else the sign-extended low word). D registers are in cpu_t (ldr / ldrsh r0), A registers are r6-r13.
@ In: r1 = Rd << 12. Ends the extension-word emitter (returns to the fill).
ea_index:
	ldr	r3, [r5, #S_LKT]
	lsr	r4, r3, #12
	and	r4, r4, #15                     @ the register, 0-15
	lsr	r3, r3, #11
	and	r3, r3, #1                      @ 1 = long
	cmp	r4, #8
	bhs	ea_idx_a
	.ifdef	PJIT_PIN_D0
	cmp	r4, #0
	bne	1f
	cmp	r3, #0                          @ the index register is the pinned D0 (r4 itself): `sxth r0, r4` or `mov r0, r4`
	movw	r3, #0x0074
	movt	r3, #0xE6BF                     @ sxth r0, r4
	beq	2f
	movw	r3, #0x0004
	movt	r3, #0xE1A0                     @ mov r0, r4
2:	str	r3, [r0], #4
	mov	r4, #0
	b	ea_idx_add
1:
	.endif
	lsl	r4, r4, #2
	rsb	r4, r4, #64                     @ the register's offset below r5: 64 - 4n
	cmp	r3, #0
	bne	ea_idx_dl
	sub	r4, r4, #2                      @ the low word of a big-endian long: 62 - 4n
	and	r3, r4, #0xF0
	lsl	r3, r3, #4
	and	r4, r4, #0xF
	orr	r3, r3, r4
	orr	r3, r3, #0xE1000000
	orr	r3, r3, #0x00550000
	orr	r3, r3, #0x000000F0             @ ldrsh r0, [r5, #-off]
	str	r3, [r0], #4
	mov	r4, #0
	b	ea_idx_add
ea_idx_dl:
	orr	r3, r4, #0xE5000000
	orr	r3, r3, #0x00150000             @ ldr r0, [r5, #-off]
	str	r3, [r0], #4
	mov	r4, #0
	b	ea_idx_add
ea_idx_a:
	sub	r4, r4, #2                      @ A(n) is r(6 + n)
	cmp	r3, #0
	bne	ea_idx_add                      @ long: add it as it is
	orr	r3, r4, #0xE6000000
	orr	r3, r3, #0x00BF0000
	orr	r3, r3, #0x00000070             @ sxth r0, rA
	str	r3, [r0], #4
	mov	r4, #0
ea_idx_add:                             @ add rd, rd, r<r4>
	orr	r3, r4, r1
	orr	r3, r3, r1, lsl #4
	orr	r3, r3, #0xE0000000
	orr	r3, r3, #0x00800000
	str	r3, [r0], #4
	bx	lr

	.ltorg

	.text

@ ---- the cores cached code branches into ---------------------------------------------
@ Called by a `b<cc>` from a DRSR body that itself was reached by `bl` from the cache:
@ lr = the end of the instruction in the cache, so HOST2GUEST(lr) is the guest PC just
@ past it; r1 holds the displacement/target the leader or the prefix built.
	.macro	CORE_ENTER
	add	r0, lr, #15                     @ lr is somewhere in the last slot of the instruction's span (see fill: the `bl` floats
	bic	r0, r0, #15                     @ there to spread over the branch target buffer): round it up to the next instruction.
	                                        @ (lr itself stays as it is: the hardware return stack holds it, so a later `bx lr` predicts)
	HOST2GUEST_BE r0, r2, r3                @ r0 = the next instruction's guest address
	.endm

@ push r0 (a return address) on the guest stack: A7 is the real SP, memory is big-endian
	.macro	PUSH_R0
	str	r0, [sp, #-4]!
	.endm


	.balign	64                       @ (hot core: its address, and so its branch target buffer sets, must not depend on what precedes it)
	.global	pjit_d_bra
pjit_d_bra:                             @ taken Bcc/BRA: the displacement (-2 for the .W form) is in r1
	CORE_ENTER
	add	r0, r0, r1
	b	pjit_goto

	.if	PJIT_TIER_ON && defined_pin
@ A taken DBF in a tiered build with the pinned D0 (fill_dbf): the inline form's last word is `bl pjit_d_dbf_taken`; the loop is counted like any other transfer (no loop compaction
@ with the pin). lr = the word after the `bl`; the target is the DBF's displacement word, read from guest memory. Flags untouched.
	.global	pjit_d_dbf_taken
pjit_d_dbf_taken:
	sub	r0, lr, #4
	add	r0, r0, #15
	bic	r0, r0, #15                     @ the next slot: the guest instruction after the DBF (its span is whole slots; the bl is in the last)
	HOST2GUEST_BE r0, r2, r3                @ r0 = the address after the DBF's displacement word
	ldrsh	r1, [r0, #-2]
	add	r0, r0, r1
	sub	r0, r0, #2
	b	pjit_goto
	.endif

@ A backward branch inside the line (see pjit_fill): `bl<cc> pjit_d_loop` then, in the
@ next word, `b<cc> target`. The core is just an interrupt window; the condition is tested
@ twice, but the flags are never touched. 16 bytes: pjit_irq_deliver relies on that.
	.global	pjit_d_loop
pjit_d_loop:
	cpsie	i
	nop
	cpsid	i
	bx	lr

	.ifdef	PJIT_TIER
	.if	PJIT_TIER_ON
@ A backward branch in a tiered build, while its target is cold: `movw r1 ; movt r1 ; bl<cc> pjit_d_bcount ; b<cc> target` (the bl is taken exactly when the branch is). r1 = the guest
@ target, lr = the `b<cc> target` word. Counts the target (the slot router's table, flag-free); the 128th event promotes it (pjit_goto_promote), a posted interrupt level sends the
@ transfer to the router's slow path, which polls and delivers. Otherwise it returns to the branch.
	.balign	64
	.global	pjit_d_bcount
pjit_d_bcount:
	movw	r3, #:lower16:pjit_tier_cnt
	movt	r3, #:upper16:pjit_tier_cnt
	ubfx	r2, r1, #1, #10
	add	r3, r3, r2, lsl #3
	ldr	r2, [r3]
	ldr	r0, [r3, #4]
	eor	r2, r2, r1
	clz	r2, r2
	lsr	r2, r2, #5
	rsb	r2, r2, #0
	sub	r0, r0, #(PJIT_TIER_K - 1)
	and	r0, r0, r2
	add	r0, r0, #PJIT_TIER_K
	str	r1, [r3]
	str	r0, [r3, #4]
	ubfx	r0, r0, #PJIT_TIER_TB, #1
	ldrb	r2, [r5, #CPU_IRQ_FLAG]
	orr	r0, r0, r2, lsl #1
	add	pc, pc, r0, lsl #2
	nop
	bx	lr
	b	1f                              @ promote
	b	2f                              @ a posted interrupt level
	b	2f
1:	mov	r0, r1
	mov	lr, r3                          @ pjit_goto_promote wants the counter entry in lr
	b	pjit_goto_promote
2:	mov	r0, r1
	b	pjit_goto_slow
	.endif
	.endif

@ RTS (cached): r0 = the guest return address just popped. A lookup by that address (pjit_goto's fast path) whose hit ends in `bx lr`, so the
@ hardware return stack, pushed by the call's `bl`, predicts it; no state besides the cache itself. A miss, an odd address or open interrupt windows go
@ to the router. Flag-free (the guest's flags are live).
	.balign	64                       @ (hot core: its address, and so its branch target buffer sets, must not depend on what precedes it)
	.global	pjit_rts_fast
pjit_rts_fast:
	ldr	r2, [r5, #CPU_TAGS_GOTO]
	ubfx	r1, r0, #(DR_SPAN_BITS - DR_LINE_BITS), #DR_LINE_BITS
	add	r2, r2, r1, lsl #1
	ldrh	r2, [r2]
	ubfx	r3, r0, #0, #DR_SPAN_BITS
	rev16	r2, r2
	eor	r2, r2, r0, lsr #DR_TAG_SHIFT
	and	r1, r0, #1
	orr	r2, r2, r1
	ldr	lr, [r5, #CPU_CACHE_BE]
	usat	r2, #1, r2
	add	lr, lr, r3, lsl #3
	add	pc, pc, r2, lsl #2              @ pc reads as .+8: the entry below the nop
	nop
	bx	lr
	b	pjit_goto_slow

	.balign	64                       @ (hot core: its address, and so its branch target buffer sets, must not depend on what precedes it)
	.global	pjit_d_bsr
pjit_d_bsr:                             @ BSR.B: displacement in r1, measured from the next instruction
	CORE_ENTER
	PUSH_R0
	add	r0, r0, r1
	b	pjit_goto

	.balign	64                       @ (hot core: its address, and so its branch target buffer sets, must not depend on what precedes it)
	.global	pjit_d_bsr2p
pjit_d_bsr2p:                           @ BSR.W: displacement from the extension word, 2 bytes back
	CORE_ENTER
	PUSH_R0
	add	r0, r0, r1
	sub	r0, r0, #2
	b	pjit_goto

	.balign	64                       @ (hot core: its address, and so its branch target buffer sets, must not depend on what precedes it)
	.global	pjit_d_jsr
pjit_d_jsr:                             @ JSR: the target is in r1
	CORE_ENTER
	PUSH_R0
	mov	r0, r1
	b	pjit_goto

	.ltorg
