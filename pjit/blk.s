@ pjit/blk.s -- the block runtime, slice 1 (docs/design/BLOCK_RUNTIME.md). Replaces runtime_dr.s in a BLK=1 build.
@
@ The router (pjit_goto), the exit trampolines, and the translator: 68K code at a guest address -> one block of ARM code in the bcache ring, built from the
@ per-opcode tables of tools/gen/btables.py (build/btables.s): the generator's sequences, copied after the extension-word prefix. No liveness, forwarding,
@ folding or register allocation yet (those are the host prototype's, tools/gen/blockasm.py, and get ported on top of this). A block ends at the first BRA / JMP /
@ JSR / BSR / RTS, at an instruction that is not translated (the interpreter takes that one instruction), or at the caps.
@
@ Worlds. Guest code and the exit stubs run E=BE with the flags live. The router and translator run in a private world: the guest's A0-A7 (r6-r13) are parked in
@ pjit_blk.park, sp is a private stack, E=LE, the flags are parked in cpu->jmp_cpsr (GE, which holds X, is never written here, so it survives). r5 (the
@ dispatch table, cpu_t just below it) is never touched. The bcache routines keep r4-r11.
@
@ Exit stubs (3 words each, after the code):  mov lr,pc ; b pjit_blk_exit[_one] ; .word <guest target>
@ The first time a stub is taken the router links it (bc_link: the `mov lr,pc` becomes `b <target block>`; the cache unlinks it when the target dies).

	.include	"ipl.inc"
	.syntax unified
	.arm
	.text
	.include "asm_offsets.inc"

@ Build-time configuration (Makefile: BLK_LIVE, BLK_FWD, BLK_INLINE_RTS, BLK_INLINE_GOTO; the defaults are the measured winners, docs/building.md):
@   PJIT_BLK_LIVE         0  flag liveness (pass 2 and the per-instruction variants): -2% Tiny BASIC, +4% Dhrystone: a wash (1.9 MB of tables)
@   PJIT_BLK_FWD          1  load forwarding: -2%
@   PJIT_BLK_INLINE_RTS   1  the shadow-stack check inline at every RTS: -3.5% / -6.5%
@   PJIT_BLK_INLINE_GOTO  0  the fast-table lookup inline at dynamic JMP / JSR: +0.7% / -1.7%: off
@   PJIT_BLK_SCHED        1  the list scheduler + register allocator over each run of plain instructions (pjit/blksched.s)
@   PJIT_BLK_FOLD         1  offset folding: a prefix constant used as a load / store register offset becomes an immediate offset (with or without PJIT_BLK_LIVE)
	.ifndef	PJIT_BLK_LIVE
	.set	PJIT_BLK_LIVE, 0
	.endif
	.ifndef	PJIT_BLK_FWD
	.set	PJIT_BLK_FWD, 1
	.endif
	.ifndef	PJIT_BLK_INLINE_RTS
	.set	PJIT_BLK_INLINE_RTS, 1
	.endif
	.ifndef	PJIT_BLK_INLINE_GOTO
	.set	PJIT_BLK_INLINE_GOTO, 0
	.endif
	.ifndef	PJIT_BLK_FOLD
	.set	PJIT_BLK_FOLD, 1
	.endif
	.ifndef	PJIT_BLK_SCHED
	.set	PJIT_BLK_SCHED, 1
	.endif
	.ifndef	PJIT_BLK_SCHED_MAX
	.set	PJIT_BLK_SCHED_MAX, 1000000          @ (a bisecting aid: only the first N regions of a run are scheduled)
	.endif
	.ifndef	PJIT_BLK_SCHED_LOG
	.set	PJIT_BLK_SCHED_LOG, 0
	.endif
	.ifndef	PJIT_BLK_PROF
	.set	PJIT_BLK_PROF, 0
	.endif
	.ifndef	PJIT_BLK_SCHED_MIN
	.set	PJIT_BLK_SCHED_MIN, 0
	.endif
	.equ	VEC_ADDRESSERR, 3
	.equ	S_FLAG,  CPU_SPILL + 28         @ "r4 already holds the guest PC" (exceptions), as in runtime_dr.s
	.equ	TAIL_WORDS, (BLK_MAXSTUB * 4) + 24      @ words kept free at the end of a reservation for the stubs and a last instruction

	.macro	UNMASK t1, t2                   @ clear the I bit if cpu->irq_unmask says so; no flag touched
	ldrb	\t1, [r5, #CPU_IRQ_UNMASK]
	mrs	\t2, cpsr
	bic	\t2, \t2, \t1
	msr	cpsr_c, \t2
	.endm

	.macro	PMARK idx                       @ BLK_PROF=1: add the cycles since the previous mark to phase \idx (r8 = the runtime; no flag touched)
	.if	PJIT_BLK_PROF
	push	{r0, r1, r2}
	mrc	p15, 0, r0, c9, c13, 0
	ldr	r1, [r8, #BLK_PROF_LAST]
	sub	r2, r0, r1
	str	r0, [r8, #BLK_PROF_LAST]
	ldr	r1, [r8, #(BLK_PROF + 4 * \idx)]
	add	r1, r1, r2
	str	r1, [r8, #(BLK_PROF + 4 * \idx)]
	pop	{r0, r1, r2}
	.endif
	.endm

	.macro	CONST reg, val
	movw	\reg, #:lower16:(\val)
	movt	\reg, #:upper16:(\val)
	.endm

	.macro	BUMP off                        @ statistics: pjit_blk.\off += 1 (clobbers r12 and, through the macro below, nothing else)
	push	{r0}
	ldr	r0, [r8, #\off]
	add	r0, r0, #1
	str	r0, [r8, #\off]
	pop	{r0}
	.endm

	.macro	GET reg, sym                    @ \reg = the address of \sym (E-independent)
	movw	\reg, #:lower16:\sym
	movt	\reg, #:upper16:\sym
	.endm

@ Flush [\start, \end) from the data cache and the instruction cache (64-byte lines). Clobbers \start and \tmp.
	.macro	FLUSH_RANGE start, end, tmp
	bic	\start, \start, #63
	mov	\tmp, \start
1:	mcr	p15, 0, \tmp, c7, c10, 1        @ DCCMVAC
	add	\tmp, \tmp, #64
	cmp	\tmp, \end
	blo	1b
	dsb
2:	mcr	p15, 0, \start, c7, c5, 1       @ ICIMVAU
	add	\start, \start, #64
	cmp	\start, \end
	blo	2b
	dsb
	isb
	.endm

@ D0 is pinned in r4 (PJIT_PIN_D0, runtime.s): block code keeps it there; the router and translator use r4 as scratch, so the router PARKS it in cpu->d0 on the way in
@ (pjit_blk_go) and UNPARKS it on every way back into guest code.
	.ifdef	PJIT_PIN_D0
	.macro	PARK_LE t
	rev	\t, r4
	str	\t, [r5, #CPU_D0]
	.endm
	.macro	UNPARK_BE
	ldr	r4, [r5, #CPU_D0]
	.endm
	.endif

@ =================================================================================================================================
@ The router.  pjit_goto: r0 = the guest address to continue at; E=BE, the guest's flags live (nothing here may rely on r0-r3 being preserved).
@ Every control transfer that is not a linked block-to-block branch comes through here, as does pjit_run.
	.ifndef	PJIT_TIER                       @ (a tiered build has ONE pjit_goto, pjit/runtime_dr.s's, which also probes blk_fast: PLAN tiering)
	.global	pjit_goto
	.type	pjit_goto, %function
pjit_goto:                              @ the fast path: a direct-mapped table of {guest, block} (both byte-swapped), no flag touched, nothing parked
	GET	r1, blk_fast
	ubfx	r2, r0, #1, #BLK_FAST_BITS
	add	r1, r1, r2, lsl #3
	ldr	r2, [r1]
	ldr	r3, [r1, #4]
	eor	r2, r2, r0
	clz	r2, r2
	lsr	r2, r2, #5                      @ 1 on a hit
	add	pc, pc, r2, lsl #2              @ pc reads as .+8: the entry below the nop
	nop
	b	pjit_blk_slow
	bx	r3
	.endif
	.global	pjit_blk_slow
pjit_blk_slow:
	mov	r1, #0                          @ no stub to link
	.global	pjit_blk_go
pjit_blk_go:                            @ r0 = target, r1 = the stub that led here (0 none, 1 interpret-one)
	mrs	r2, cpsr
	setend	le
	str	r2, [r5, #CPU_JMP_CPSR]
	.ifdef	PJIT_PIN_D0
	PARK_LE	r2
	.endif
	b	pjit_blk_park

	.ifndef	PJIT_TIER
	.global	pjit_goto_parked
pjit_goto_parked:                       @ E=LE, flags already parked; r0 = target (exception delivery, STOP, ...)
	mov	r1, #0
	.endif
	.global	pjit_blk_park
pjit_blk_park:
	cpsid	i                              @ the router and translator run with the host IRQ masked (the latch would patch links under it); the ways back into guest code unmask
	GET	r3, pjit_blk
	str	r1, [r3, #BLK_STUB_IN]
	str	r0, [r3, #BLK_TARGET]
	add	r2, r3, #BLK_PARK
	stmia	r2, {r6-r13}                    @ A0-A7
	ldr	sp, [r3, #BLK_STACK]
	mov	r8, r3                          @ r8 = the block runtime's state, kept across the bc calls
	.if	PJIT_BLK_PROF
	mrc	p15, 0, r2, c9, c13, 0
	str	r2, [r8, #BLK_PROF_LAST]
	.endif
	mov	r9, r0                          @ r9 = the target
	ldr	r10, [r8, #BLK_STUB_IN]
	mov	r0, r10                         @ the epoch of the stub's segment now: if translating the target reclaims that segment the stub is gone and must not be linked (bc_link would patch whatever is there)
	bl	blk_stub_epoch
	str	r0, [r8, #BLK_STUB_EP]
	mov	r0, r9                          @ (the target again)
	.ifdef	PJIT_TIER
	cmp	r10, #20                        @ BLK_STUB_CODE: the slot router found an unregistered line of guest code (pjit_platform_code)
	beq	pjit_blk_code
	.endif
	ldr	r1, [r5, #CPU_FLUSH_REQ]        @ a guest write hit translated code: flush both tiers now, with the guest parked and on this private stack
	cmp	r1, #0
	bne	pjit_blk_flush
pjit_blk_cont:
	ldr	r1, [r8, #BLK_N_ROUTE]
	add	r1, r1, #1
	str	r1, [r8, #BLK_N_ROUTE]
	ldrb	r1, [r5, #CPU_IRQ_LEVEL]        @ the poll: a posted level that beats the SR mask is delivered now, at this boundary (the flag only brought us here)
	IRQ_VERIFY r1, r2, r3
	mov	r2, #0
	str	r2, [r5, #CPU_IRQ_FLAG]
	.ifdef	PJIT_TIER
	ldr	r2, [r5, #CPU_TAGS_BE]          @ the slot router's fast path is back
	str	r2, [r5, #CPU_TAGS_GOTO]
	mov	r2, #0
	.endif
	cmp	r1, #0
	beq	1f
	cmp	r1, #7
	beq	pjit_blk_irq
	ldrb	r2, [r5, #CPU_SR]
	and	r2, r2, #7
	cmp	r1, r2
	bhi	pjit_blk_irq
1:	tst	r0, #1
	bne	pjit_blk_odd
	cmp	r10, #1
	beq	pjit_blk_one                    @ interpret one instruction (the block ended in front of it)
	ldr	r1, [r5, #CPU_DR_THRESHOLD]
	cmp	r0, r1
	blo	pjit_blk_interp
	ldr	r2, [r5, #CPU_XLAT]             @ the platform's map of the megabytes that are translated (none: everything above the threshold is)
	cmp	r2, #0
	beq	2f
	lsr	r1, r0, #20
	add	r2, r2, r1, lsl #1
	ldrh	r2, [r2]
	cmp	r2, #0
	beq	pjit_blk_interp
2:
	mov	r1, r0
	mov	r0, r8
	bl	bc_lookup
	cmp	r0, #0
	bne	1f
	.ifdef	PJIT_TIER
	cmp	r10, #12                        @ BLK_STUB_PROMOTE (8; 12: a target that had a block before): the slot router found this target hot: translate it
	cmpne	r10, #8
	beq	pjit_blk_force
	mov	r0, r9                          @ a block exit to a target that has no block: that is one more event for the counter; below the threshold the slot tier runs it
	bl	pjit_tier_count
	cmp	r0, #0
	.ifndef	PJIT_TIER_NOSLOT
	beq	pjit_blk_to_slot
	.endif
pjit_blk_force:
	ldr	r0, [r8, #BLK_TIER_PAUSE]       @ the thrash guard: while it is refusing promotions the target stays in the slot tier
	cmp	r0, #0
	subne	r0, r0, #1
	strne	r0, [r8, #BLK_TIER_PAUSE]
	bne	pjit_blk_to_slot
	.endif
	mov	r0, r9
	bl	blk_translate
	.ifdef	PJIT_TIER
	ldr	r1, [r8, #BLK_STUB_IN]          @ a build: counted; a rebuild (the target had a block before) too. Per epoch of 32 builds, more than 8 rebuilds = the arena is too small for the
	ldr	r2, [r8, #BLK_TIER_BUILDS]      @ hot code (blocks evicted and rebuilt): refuse the next 256 promotions, so a thrashing arena costs no more than the slot tier
	ldr	r3, [r8, #BLK_TIER_REBUILDS]
	add	r2, r2, #1
	cmp	r1, #12
	addeq	r3, r3, #1
	cmp	r2, #32
	blo	2f
	cmp	r3, #8
	movhi	r1, #256
	strhi	r1, [r8, #BLK_TIER_PAUSE]
	mov	r2, #0
	mov	r3, #0
2:	str	r2, [r8, #BLK_TIER_BUILDS]
	str	r3, [r8, #BLK_TIER_REBUILDS]
	.endif
1:	mov	r11, r0                         @ r11 = the block
	ldr	r10, [r8, #BLK_STUB_IN]         @ (the translator uses r10)
	cmp	r10, #0x100
	blo	4f                              @ (a marker, not a stub address)
	mov	r0, r10
	bl	blk_stub_epoch
	ldr	r1, [r8, #BLK_STUB_EP]
	cmp	r0, r1
	movne	r10, #0                         @ the stub's segment was reclaimed during the translation: nothing to link
4:
	.ifdef	PJIT_TIER
	cmp	r10, #12
	cmpne	r10, #8
	moveq	r10, #0                         @ (a promotion has no stub to link)
	.endif
	ldr	r0, [r8, #BLK_TARGET]
	GET	r1, blk_fast                    @ the fast table now has this target
	ubfx	r2, r0, #1, #BLK_FAST_BITS
	add	r1, r1, r2, lsl #3
	rev	r0, r0
	str	r0, [r1]
	rev	r3, r11
	str	r3, [r1, #4]
	cmp	r10, #0
	beq	2f
	and	r3, r10, #3
	cmp	r3, #1
	bne	3f
	ldr	r3, [r8, #BLK_BK_N]             @ a backward jump stub: it is registered while linked (pjit_bk_unlink undoes it when an interrupt is latched); a full registry: stay unlinked
	cmp	r3, #BLK_MAXBK
	bhs	2f
3:	mov	r0, r8                          @ link the stub that brought us here to its target
	bic	r1, r10, #3
	mov	r2, #0
	tst	r10, #2
	movne	r2, #1                          @ a call stub
	moveq	r12, #12                        @ ... whose patched word is the third, the site word after it; a jump stub's is the fourth word
	movne	r12, #4
	ldr	r12, [r1, r12]
	cmp	r12, #0
	orrne	r2, r2, #4                      @ the stub has a site: bc_link aims it at the target as well
	mov	r3, r11
	bl	bc_link
	and	r3, r10, #3
	cmp	r3, #1
	bne	3f
	cmp	r0, #1
	bne	3f
	ldr	r3, [r8, #BLK_BK_N]
	bic	r1, r10, #3
	movw	r2, #BLK_BK
	add	r2, r8, r2
	str	r1, [r2, r3, lsl #2]
	add	r3, r3, #1
	str	r3, [r8, #BLK_BK_N]
3:	ldr	r1, [r8, #BLK_N_EXITS]
	add	r1, r1, r0                      @ (counts the links made, not the attempts)
	str	r1, [r8, #BLK_N_EXITS]
2:	PMARK	8
	.ifdef	PJIT_PIN_D0
	mov	lr, r11                         @ (r8-r11 are guest registers again after the ldmia: keep the block in lr; r4 is D0)
	.else
	mov	r4, r11                         @ (r8-r11 are guest registers again after the ldmia: keep the block in r4, which is not one)
	.endif
	add	r2, r8, #BLK_PARK
	ldmia	r2, {r6-r13}                    @ the guest's registers
	ldr	r1, [r5, #CPU_JMP_CPSR]
	UNMASK	r2, r3
	setend	be
	.ifdef	PJIT_PIN_D0
	UNPARK_BE
	msr	cpsr_f, r1
	bx	lr
	.else
	msr	cpsr_f, r1
	bx	r4
	.endif

pjit_blk_interp:                        @ below the threshold: the interpreter, PC = the target
	.ifdef	PJIT_PIN_D0
	mov	r0, r9                          @ (the PC rides in r0: r4 is D0, r9 is a guest register after the ldmia)
	.else
	mov	r4, r9
	.endif
	add	r2, r8, #BLK_PARK
	ldmia	r2, {r6-r13}
	ldr	lr, [r5, #CPU_PREFETCH_HOOK]
	ldr	r1, [r5, #CPU_JMP_CPSR]
	UNMASK	r2, r3
	setend	be
	.ifdef	PJIT_PIN_D0
	UNPARK_BE
	str	r0, [r5, #CPU_IPC]
	.else
	str	r4, [r5, #CPU_IPC]
	.endif
	msr	cpsr_f, r1
	b	pjit_prefetch

pjit_blk_flush:
	bl	pjit_cache_flush                @ (clears cpu->flush_req; r8-r10 survive the call; the stub that led here lies in the ring that was just emptied: it is not linked)
	mov	r0, r9
	cmp	r10, #16                        @ BLK_STUB_FLUSH: the slot router asked: back to it
	mov	r10, #0
	str	r10, [r8, #BLK_STUB_IN]
	.ifdef	PJIT_TIER
	beq	pjit_blk_to_slot
	.endif
	b	pjit_blk_cont

	.ifdef	PJIT_TIER
pjit_blk_code:                          @ r9 = a guest address in an unregistered 128-byte line: announce the line (and the 16 bytes an instruction may spill) to the platform
	bic	r0, r9, #127
	mov	r1, #144
	bl	pjit_platform_line
	b	pjit_blk_to_slot

pjit_blk_to_slot:                       @ (E=LE, flags parked, r8 = pjit_blk, r9 = the target) give the guest back to the slot tier
	mov	r0, r9                          @ (r8-r11 are guest registers again after the ldmia)
	add	r2, r8, #BLK_PARK
	ldmia	r2, {r6-r13}
	b	pjit_goto_parked
	.endif

pjit_blk_one:                           @ the interpreter, for exactly one instruction; it returns through pjit_blk_after_one
	ldr	r1, [r8, #BLK_N_ONE]
	add	r1, r1, #1
	str	r1, [r8, #BLK_N_ONE]
	.ifdef	PJIT_PIN_D0
	mov	r0, r9
	.else
	mov	r4, r9
	.endif
	add	r2, r8, #BLK_PARK
	ldmia	r2, {r6-r13}
	GET	lr, pjit_blk_after_one
	ldr	r1, [r5, #CPU_JMP_CPSR]
	UNMASK	r2, r3
	setend	be
	.ifdef	PJIT_PIN_D0
	UNPARK_BE
	str	r0, [r5, #CPU_IPC]
	.else
	str	r4, [r5, #CPU_IPC]
	.endif
	msr	cpsr_f, r1
	b	pjit_prefetch

pjit_blk_after_one:                     @ E=BE, flags live, cpu->ipc = the next guest PC (the interpreted instruction ended in `bx lr`)
	ldr	r0, [r5, #CPU_IPC]
	b	pjit_goto

pjit_blk_odd:                           @ an odd PC: address error on the fetch
	.ifdef	PJIT_PIN_D0
	rev	r3, r9
	str	r3, [r5, #CPU_IPC]
	.else
	rev	r4, r9
	str	r4, [r5, #CPU_IPC]
	.endif
	mov	r0, r9
	add	r2, r8, #BLK_PARK
	ldmia	r2, {r6-r13}
	mov	r1, #1
	str	r1, [r5, #S_FLAG]               @ cpu->ipc already holds the PC
	ldr	r2, [r5, #CPU_JMP_CPSR]
	mov	r1, #0x18                       @ program space, read
	setend	be
	.ifdef	PJIT_PIN_D0
	UNPARK_BE                               @ (the exception entry parks D0 again)
	.endif
	msr	cpsr_f, r2
	svc	#VEC_ADDRESSERR

@ ---- the exit stubs land here: E=BE, flags live, lr = the address of the stub's literal ----
	.global	pjit_blk_exit
pjit_blk_exit:
	ldr	r0, [lr]
	rev	r0, r0                          @ (the literal was written little-endian)
	sub	r1, lr, #8                      @ the stub: link it
	b	pjit_blk_go
	.global	pjit_blk_exit_one
pjit_blk_exit_one:
	ldr	r0, [lr]
	rev	r0, r0
	mov	r1, #1
	b	pjit_blk_go
	.global	pjit_blk_call_exit
pjit_blk_call_exit:                     @ a call stub's `bl`: lr = the stub's end, the patchable word is lr - 4; r0 = the target
	sub	r1, lr, #4
	orr	r1, r1, #2                      @ (a call stub)
	b	pjit_blk_go

	.global	pjit_blk_exit_b
pjit_blk_exit_b:                        @ a backward jump stub (its link is undone whenever an interrupt is latched, so the router polls): lr = the literal, the patched word is lr - 8
	ldr	r0, [lr]
	rev	r0, r0
	sub	r1, lr, #8
	orr	r1, r1, #1
	b	pjit_blk_go

@ the router found a deliverable level: r8 = the runtime, r9 = the guest PC to resume. E=LE, flags parked, A0-A7 parked.
pjit_blk_irq:
	mov	r4, r9
	rev	r1, r9
	str	r1, [r5, #CPU_IPC]
	add	r2, r8, #BLK_PARK
	ldmia	r2, {r6-r13}
	mov	r1, #0
	str	r1, [r5, #CPU_IRQ_SITE]         @ (not the loop core)
	b	pjit_irq_deliver

@ RTS: r0 = the popped return address, sp = A7 after the pop (the key the call used: A7 before its push). The shadow entry holds the guest return address and the host
@ continuation; a match returns with `bx lr`, which the hardware return predictor follows. E=BE, flags live (nothing here sets any).
	.global	pjit_blk_rts
pjit_blk_rts:
	and	r2, sp, #0x3FC
	add	r3, r5, r2, lsl #1
	ldr	r1, [r3, #CPU_SHADOW]
	ldr	lr, [r3, #(CPU_SHADOW + 4)]
	eor	r1, r1, r0
	clz	r1, r1
	lsr	r1, r1, #5
	add	pc, pc, r1, lsl #2
	nop
	b	pjit_goto
	bx	lr

@ Empties blk_fast: every entry becomes {guest key = an odd address (a big-endian 1), block = the slow path}. A zeroed entry would be {0, 0}, and a transfer to GUEST address 0 (a NULL
@ function pointer, an unset vector) matches it and branches to HOST address 0: the key is odd so that no even target matches, and the block word is the router's slow path (stored
@ byte-swapped like every block pointer: the fast path reads it big-endian) so that an odd target (an address error to come) that does match lands in the router, not at 0.
@ Any mode, E=LE data; callable from C; clobbers r0-r3.
	.global	pjit_blk_fast_clear
	.type	pjit_blk_fast_clear, %function
pjit_blk_fast_clear:
	GET	r0, blk_fast
	add	r1, r0, #(8 << BLK_FAST_BITS)
	mov	r2, #0x01000000                 @ the key: an odd address, as a big-endian 1
	.ifdef	PJIT_TIER
	GET	r3, pjit_goto_slow
	.else
	GET	r3, pjit_blk_slow
	.endif
	rev	r3, r3
1:	str	r2, [r0], #4
	str	r3, [r0], #4
	cmp	r0, r1
	blo	1b
	bx	lr

@ the cache calls this (E=LE, r0 = the bcache) whenever a segment is entered or everything is flushed: the fast table and the shadow stack name blocks that may be gone.
@ Clobbers r0-r3 only.
	.global	blk_hook
blk_hook:
	push	{lr}
	bl	pjit_bk_unlink                  @ (the stubs it names may be in the segment about to be overwritten)
	bl	pjit_blk_fast_clear
	pop	{lr}
	movw	r0, #:lower16:(pjit_arena + CPU_SIZE + CPU_SHADOW)
	movt	r0, #:upper16:(pjit_arena + CPU_SIZE + CPU_SHADOW)
	add	r1, r0, #2048
	mov	r2, #0x01000000                 @ the guest address field of an empty entry: odd, as a big-endian 1
	mov	r3, #0
2:	str	r2, [r0], #4
	str	r3, [r0], #4
	cmp	r0, r1
	blo	2b
	bx	lr

@ Undo every registered backward link: the stub's first word goes back to `mov lr,pc` and the first branch site back to the stub, exactly as bc's turnover restores a
@ stub (its link record stays: bc_link does not file a second one). Called by the IRQ latch (so a loop that never leaves chained blocks reaches the router within one
@ iteration and the router polls) and by blk_hook. Any mode, any endianness of data; clobbers r0-r3, r12.
	.global	pjit_bk_unlink
pjit_bk_unlink:
	GET	r0, pjit_blk
	ldr	r1, [r0, #BLK_BK_N]
	cmp	r1, #0
	bxeq	lr
	mov	r2, #0
	str	r2, [r0, #BLK_BK_N]
	movw	r2, #BLK_BK
	add	r2, r0, r2
	push	{r4}
1:	ldr	r3, [r2], #4
	.ifdef	PJIT_BK_DEBUG
	push	{r0-r3, r12, lr}                @ (debug) is this still a stub?
	mov	r0, r3
	bl	blk_bk_check
	pop	{r0-r3, r12, lr}
	.endif
	movw	r12, #0xE00F
	movt	r12, #0xE1A0                    @ mov lr, pc
	str	r12, [r3]
	mcr	p15, 0, r3, c7, c11, 1          @ DCCMVAU
	mcr	p15, 0, r3, c7, c5, 1           @ ICIMVAU
	ldr	r12, [r3, #12]                  @ the site
	cmp	r12, #0
	beq	2f
	sub	r4, r3, r12
	sub	r4, r4, #8
	mov	r4, r4, asr #2
	bic	r4, r4, #0xFF000000
	ldr	r0, [r12]
	and	r0, r0, #0xFF000000
	orr	r4, r4, r0
	str	r4, [r12]
	mcr	p15, 0, r12, c7, c11, 1
	mcr	p15, 0, r12, c7, c5, 1
2:	subs	r1, r1, #1
	bne	1b
	mov	r0, #0
	mcr	p15, 0, r0, c7, c5, 6           @ BPIALL
	dsb
	isb
	pop	{r4}
	bx	lr

	.ifndef	PJIT_TIER
@ the slot design's control cores, still named by the generated tables' (unused here) slot-cache sequences
	.irp	n, pjit_d_bra, pjit_d_bsr, pjit_d_bsr2p, pjit_d_jsr
	.global	\n
\n:
	bkpt	#3
	.endr

@ the interpreter's loop core is not here (runtime.s names it for its interrupt delivery in the slot design)
	.balign	16
	.global	pjit_d_loop
pjit_d_loop:
	.space	16

	.endif

@ =================================================================================================================================
@ The translator.  blk_translate: r0 = the guest address; E=LE, private stack, r5 = the table. -> r0 = the block's code address (in the cache, live).
@ Three passes over the instructions of a block:
@   1 decode: walk the guest code, one item (op | size << 16) per instruction, until a terminator (BRA, JMP, RTS), an instruction the interpreter takes, or a cap. The caps
@     are checked here against the worst case of what pass 3 will emit, so pass 3 never runs out of room (the liveness below assumed the items it saw).
@   2 liveness, backward: the flag groups wanted after each plain instruction (L: bit 0 = N Z V C, bit 1 = X). A control item (a branch, a call, anything that leaves
@     the block) wants everything before it; a plain instruction wants what its sequence reads once the dead parts are cut out (the transfer byte of pjit_bv).
@   3 emit: per item the variant of its sequence for its L (pjit_bv / pjit_bvar), the prefix, the control code, then the stubs.
@ Registers: pass 1: r4 guest cursor, r6 estimated words, r7 item pointer, r8 pjit_blk, r10 the start address. Pass 3: r4 guest cursor, r6 output, r7 item pointer, r9 block start.
	.global	blk_translate
	.type	blk_translate, %function
blk_translate:
	push	{lr}
	mov	r4, r0
	mov	r10, r0
	GET	r8, pjit_blk
	PMARK	0
	mov	r0, #0
	str	r0, [r8, #BLK_NSTUB]
	str	r0, [r8, #BLK_NSITE]
	mov	r6, #0                          @ the estimate of the words pass 3 will emit
	add	r7, r8, #BLK_ITEMS

@ ---- pass 1 ----
p1_loop:
	ldr	r0, [r8, #BLK_NSTUB]
	cmp	r0, #(BLK_MAXSTUB - 3)
	bhs	p1_fall
	ldr	r0, [r8, #BLK_NSITE]
	cmp	r0, #(BLK_MAXSITE - 4)
	bhs	p1_fall
	add	r0, r8, #BLK_ITEMS
	sub	r0, r7, r0
	cmp	r0, #((BLK_MAXITEM - 2) * 4)
	bhs	p1_fall
	sub	r0, r4, r10
	ldr	r1, [r8, #BLK_SRC_CAP]
	cmp	r0, r1
	bhs	p1_fall                         @ the guest-byte cap
	ldrh	r11, [r4]
	rev16	r11, r11                        @ the opcode
	GET	r0, pjit_bt
	add	r0, r0, r11, lsl #3
	ldmia	r0, {r1, r2}
	lsrs	r3, r2, #29                     @ the kind
	beq	p1_stop
	ubfx	r0, r2, #23, #6
	add	r0, r0, #40                     @ (the prefix, a few control words)
	add	r0, r6, r0
	ldr	r1, [r8, #BLK_CAP]
	sub	r1, r1, #(TAIL_WORDS * 4)
	cmp	r0, r1, lsr #2
	bhi	p1_fall                         @ no room for this instruction: end the block in front of it
	mov	r6, r0
	cmp	r3, #1
	beq	1f
	ldr	r0, [r8, #BLK_NSTUB]            @ a control item: up to one stub and two branches to it
	add	r0, r0, #1
	str	r0, [r8, #BLK_NSTUB]
	ldr	r0, [r8, #BLK_NSITE]
	add	r0, r0, #2
	str	r0, [r8, #BLK_NSITE]
1:	bl	blk_isize                       @ r0 = the instruction's size in bytes
	orr	r1, r11, r0, lsl #16
	str	r1, [r7], #4
	add	r4, r4, r0
	mov	r1, #2                          @ (endkind if this was a terminator)
	cmp	r3, #6                          @ JMP
	beq	p1_end
	cmp	r3, #7                          @ RTS
	beq	p1_end
	cmp	r3, #2                          @ BRA (Bcc with the condition T) ends the block
	bne	p1_loop
	ubfx	r0, r11, #8, #4
	cmp	r0, #0
	bne	p1_bcc
	b	p1_end
p1_bcc:
	.ifndef	PJIT_BLK_NATCMP
	.set	PJIT_BLK_NATCMP, 1              @ the natural-polarity compare + Bcc pairs (0 for the A/B measurement)
	.endif
	.if	(!PJIT_BLK_LIVE) && PJIT_BLK_NATCMP
	@ A Bcc after a compare: if the flags are dead on both ways out of the Bcc (the instruction at its target and the one after it do not read N Z V C), the compare can leave the
	@ ARM flags in their natural polarity (C = no borrow, one flag-setting instruction instead of mvn / adds / mvns) and the Bcc swaps CS / CC and uses the ARM HI / LS. Both items are marked
	@ (bit 26 of the item word); pass 3 takes the compare from pjit_bn and the condition from blk_ccn.
	add	r0, r8, #BLK_ITEMS
	sub	r0, r7, r0
	cmp	r0, #8
	blo	p1_loop                         @ the Bcc is the first item
	ldrh	r1, [r7, #-8]                   @ the previous item's opcode
	GET	r0, pjit_bn
	add	r0, r0, r1, lsl #3
	ldr	r0, [r0, #4]                    @ the descriptor of its natural-polarity variant (0 = none)
	cmp	r0, #0
	beq	p1_loop
	ldr	r2, [r7, #-4]
	ubfx	r2, r2, #16, #8
	sub	r2, r4, r2                      @ the Bcc's address
	sxtb	r0, r11                         @ its displacement: the byte ...
	cmp	r0, #0
	bne	1f
	ldrh	r0, [r2, #2]                    @ ... or the word after the opcode
	rev16	r0, r0
	sxth	r0, r0
1:	add	r1, r0, #4096
	cmp	r1, #8192
	bhs	p1_loop                         @ a far target: only code near the branch is looked at (the target may not be mapped)
	add	r2, r2, #2
	add	r2, r2, r0                      @ the target
	tst	r2, #1
	bne	p1_loop
	GET	r3, pjit_bfr
	ldrh	r1, [r2]
	rev16	r1, r1
	ldrb	r1, [r3, r1]                    @ does the instruction at the target read N Z V C?
	cmp	r1, #0
	bne	p1_loop
	ldrh	r1, [r4]
	rev16	r1, r1
	ldrb	r1, [r3, r1]                    @ ... and the one after the branch?
	cmp	r1, #0
	bne	p1_loop
	ldr	r0, [r7, #-8]
	orr	r0, r0, #0x04000000
	str	r0, [r7, #-8]
	ldr	r0, [r7, #-4]
	orr	r0, r0, #0x04000000
	str	r0, [r7, #-4]
	ldr	r0, [r8, #BLK_N_FUSED]
	add	r0, r0, #1
	str	r0, [r8, #BLK_N_FUSED]
	.endif
	b	p1_loop
p1_stop:
	mov	r1, #1
	b	p1_end
p1_fall:
	mov	r1, #0
p1_end:
	PMARK	1
	str	r1, [r8, #BLK_ENDKIND]
	str	r7, [r8, #BLK_ITEM_END]
	mov	r0, r10                         @ write detection: the guest range this block was decoded from
	sub	r1, r4, r10
	bl	pjit_platform_code
	add	r1, r6, #TAIL_WORDS             @ the ring space is reserved only now, for what pass 3 can emit at most (the estimate), not for the cap: a reservation that does not fit what is
	lsl	r1, r1, #2                      @ left of a segment moves on into the next one and kills its blocks, so reserving the cap for every block wasted the arena
	mov	r0, r8
	bl	bc_reserve
	cmp	r0, #0
	beq	blk_fatal
	mov	r9, r0

	.if	PJIT_BLK_LIVE
@ ---- pass 2: liveness, backward ----
	mov	r3, #3
p2_loop:
	add	r0, r8, #BLK_ITEMS
	cmp	r7, r0
	bls	p2_done
	ldr	r11, [r7, #-4]!
	ubfx	r12, r11, #0, #16               @ the opcode
	GET	r0, pjit_bt
	add	r0, r0, r12, lsl #3
	ldr	r2, [r0, #4]
	lsr	r2, r2, #29
	cmp	r2, #1
	bne	p2_ctl
	bic	r11, r11, #0x03000000
	orr	r11, r11, r3, lsl #24           @ the flag groups wanted after this instruction (3 in the first step, whatever the option)
	str	r11, [r7]
	GET	r0, pjit_bv
	ldr	r0, [r0, r12, lsl #2]
	and	r0, r0, #0xFF
	lsl	r1, r3, #1
	lsr	r3, r0, r1
	and	r3, r3, #3                      @ the groups it wants before it
	b	p2_loop
p2_ctl:
	mov	r3, #3
	b	p2_loop
p2_done:
	PMARK	2
	.endif

@ ---- pass 3: emit ----
	mov	r4, r10
	mov	r6, r9
	mov	r0, #0
	str	r0, [r8, #BLK_NSTUB]
	str	r0, [r8, #BLK_NSITE]
	add	r7, r8, #BLK_ITEMS
	mov	r0, #0
	str	r0, [r8, #BLK_MIRROR]
	str	r0, [r8, #BLK_FWDST]
	str	r0, [r8, #BLK_SCHSTART]
p3_loop:
	PMARK	13
	ldr	r0, [r8, #BLK_ITEM_END]
	cmp	r7, r0
	bhs	p3_end
	ldr	r12, [r7], #4
	ubfx	r11, r12, #0, #16               @ the opcode
	ubfx	r3, r12, #24, #3                @ the flag groups wanted after it (bit 2: the natural-polarity compare / Bcc pair, see p1_bcc)
	ubfx	r12, r12, #16, #8               @ its size in bytes
	GET	r0, pjit_bt
	add	r0, r0, r11, lsl #3
	ldmia	r0, {r1, r2}                    @ the leader, the descriptor of the full sequence
	lsr	r0, r2, #29                     @ the kind
	cmp	r0, #1
	beq	4f
	.if	PJIT_BLK_SCHED
	push	{r1, r2, r3, r11, r12}
	bl	blk_sflush
	pop	{r1, r2, r3, r11, r12}
	.endif
	mov	r0, #0                          @ a control item: forwarding knows nothing after it
	str	r0, [r8, #BLK_MIRROR]
	b	3f
4:	.if	!PJIT_BLK_LIVE
	tst	r3, #4
	beq	p3_notnat
	GET	r0, pjit_bn
	add	r0, r0, r11, lsl #3
	ldmia	r0, {r1, r2}                    @ the compare in its natural-polarity form (its Bcc is the only reader of the flags)
p3_notnat:
	.endif
	GET	r0, pjit_bv
	ldr	r0, [r0, r11, lsl #2]
	.if	PJIT_BLK_SCHED
	tst	r0, #0xC0000000                 @ an instruction the scheduler does not reorder (a branch, a call, an exception): the run of plain ones before it is scheduled
	beq	p3_sch1                         @ now, and forwarding forgets r0-r3 (the scheduler renames them, so nothing may rely on what they hold)
	push	{r0-r3, r12}
	bl	blk_sflush
	mov	r0, #0
	str	r0, [r8, #BLK_MIRROR]
	pop	{r0-r3, r12}
	b	p3_sch2
p3_sch1:
	push	{r1}
	ldr	r1, [r8, #BLK_SCHSTART]
	cmp	r1, #0
	streq	r6, [r8, #BLK_SCHSTART]
	pop	{r1}
p3_sch2:
	.endif
	tst	r0, #0x80000000                 @ an instruction that can raise an exception from inside its sequence (an svc, the DIV helpers): the exception entry
	beq	5f                              @ takes the guest PC from r4, which no generated sequence touches
	push	{r0-r3}
	add	r0, r4, r12                     @ the address after the instruction
	.ifdef	PJIT_PIN_D0
	mov	r1, #0                          @ (r4 is D0: the PC goes through r0, which is scratch between instructions; forwarding forgets r0-r3)
	bl	blk_loadc
	CONST	r0, (0xE5050000 | ((-CPU_IPC) & 0xFFF))     @ str r0, [r5, #CPU_IPC]: the exception entry reads the PC from cpu->ipc
	mov	r1, #0
	str	r1, [r8, #BLK_MIRROR]
	.else
	mov	r1, #4
	bl	blk_loadc
	CONST	r0, (0xE5054000 | ((-CPU_IPC) & 0xFFF))     @ str r4, [r5, #CPU_IPC]: the exception entry reads the PC from cpu->ipc
	.endif
	str	r0, [r6], #4
	pop	{r0-r3}
5:	and	r12, r0, #0x30000000            @ the leader's offset-folding mask (the variants' are looked up below)
	str	r12, [r8, #BLK_FOLDMASK]
	.if	PJIT_BLK_LIVE
	cmp	r3, #3
	beq	3f
	bic	r0, r0, #0xF0000000             @ (the trap and fold flags are not part of the index) a plain instruction whose live-out is not everything: its variant, if it has one
	lsrs	r0, r0, #8
	beq	3f
	sub	r0, r0, #1
	GET	r2, pjit_bvarf
	ldrb	r2, [r2, r0]                    @ the fold masks of the set's three variants
	lsl	r1, r3, #1
	lsr	r2, r2, r1
	and	r2, r2, #3
	lsl	r2, r2, #28
	str	r2, [r8, #BLK_FOLDMASK]
	add	r0, r0, r0, lsl #1              @ 24 bytes per set
	GET	r1, pjit_bvar
	add	r1, r1, r0, lsl #3
	add	r1, r1, r3, lsl #3
	ldmia	r1, {r1, r2}
	.endif
3:	lsr	r12, r2, #29
	add	pc, pc, r12, lsl #2
	nop
	b	blk_fatal
	b	k_plain
	b	k_bcc
	b	k_dbcc
	b	k_bsr
	b	k_jsr
	b	k_jmp
	b	k_rts
p3_end:
	.if	PJIT_BLK_SCHED
	bl	blk_sflush
	.endif
	PMARK	3
	ldr	r0, [r8, #BLK_ENDKIND]
	cmp	r0, #1
	beq	k_stop
	b	k_fall

@ ---- the scheduler: the plain instructions emitted since BLK_SCHSTART (r6 = the write cursor) are reordered in place ----
	.if	PJIT_BLK_SCHED
	.if	PJIT_BLK_SCHED_LOG
	.global	blk_schedlog
	.bss
	.balign 4
blk_schedlog:	.space	4 + 2048 * 12
	.text
	.endif
blk_sflush:                             @ clobbers r0-r3, r12
	ldr	r0, [r8, #BLK_SCHSTART]
	cmp	r0, #0
	bxeq	lr
	PMARK	3
	mov	r1, #0
	str	r1, [r8, #BLK_SCHSTART]
	sub	r1, r6, r0
	cmp	r1, #16
	bxlo	lr
	ldr	r1, [r8, #BLK_N_SCHT]           @ the number of regions seen so far (the bisecting window below)
	add	r2, r1, #1
	str	r2, [r8, #BLK_N_SCHT]
	CONST	r12, PJIT_BLK_SCHED_MAX
	cmp	r1, r12
	bxhs	lr
	CONST	r12, PJIT_BLK_SCHED_MIN
	cmp	r1, r12
	bxlo	lr
	push	{r0, lr}
	mov	r1, r6
	movw	r2, #BLK_SCHED
	add	r2, r8, r2
	CONST	r3, CPU_D0
	bl	blk_sched
	PMARK	4
	.if	PJIT_BLK_SCHED_LOG
	ldr	r3, [sp]                        @ a debugging log: per region its length, result and a checksum of the words after scheduling
	GET	r2, blk_schedlog
	ldr	r1, [r2]
	cmp	r1, #2048
	bhs	1f
	add	r12, r1, r1, lsl #1
	add	r2, r2, r12, lsl #2
	add	r2, r2, #4
	sub	r12, r6, r3
	lsr	r12, r12, #2
	stmia	r2!, {r0, r12}
	mov	r12, #0
2:	ldr	r1, [r3], #4
	add	r12, r1, r12, ror #7
	cmp	r3, r6
	blo	2b
	str	r12, [r2]
	GET	r2, blk_schedlog
	ldr	r1, [r2]
	add	r1, r1, #1
	str	r1, [r2]
1:
	.endif
	cmp	r0, #0
	ldrne	r1, [r8, #BLK_N_SCHO]
	addne	r1, r1, #1
	strne	r1, [r8, #BLK_N_SCHO]
	pop	{r1, pc}
	.endif

@ ---- an instruction the interpreter takes: the block ends in front of it with an interpret-one stub ----
k_stop:
	mov	r0, r4
	mov	r1, #1
	bl	blk_stub
	mov	r0, #0xEA000000
	bl	blk_br_stub
	b	blk_finish

@ ---- a cap: continue with a plain jump stub to the next instruction ----
k_fall:
	mov	r0, r4
	mov	r1, #0
	bl	blk_stub
	mov	r0, #0xEA000000
	bl	blk_br_stub
	b	blk_finish

@ ---- a plain instruction: the extension prefix (r1 = the source value, r2 = the destination value), the leader, the shared body ----
k_plain:
	add	r0, r5, #ARENA_EXT
	ldrb	r3, [r0, r11]                   @ the extension class: source << 4 | destination
	cmp	r3, #0
	bne	k_plain_x
	add	r11, r4, #2                     @ no extension words: no prefix, nothing to fold (the common case), straight to the leader and body
	bl	blk_emit_plain
	PMARK	12
	mov	r4, r11
	b	p3_loop
k_plain_x:
	push	{r1, r2}                        @ leader, descriptor
	add	r0, r5, #ARENA_EXT
	ldrb	r3, [r0, r11]                   @ the extension class: source << 4 | destination
	mov	r12, #0                         @ the operand register (1 / 2) whose prefix folds into the leader, or 0
	.if	PJIT_BLK_FOLD
	ldr	r0, [r8, #BLK_FOLDMASK]
	cmp	r0, #0                          @ the leader is a [Rn, r1 / r2] load / store whose offset register nothing reads afterwards
	beq	1f
	push	{r3}
	ldr	r1, [sp, #4]
	bl	blk_foldcheck                   @ r0 = the folded leader, r12 = the register, or r12 = 0
	pop	{r3}
	cmp	r12, #0
	strne	r0, [sp]                        @ the folded leader replaces the leader
1:
	.endif
	PMARK	9
	add	r11, r4, #2                     @ r11 = the first extension word
	push	{r3, r12}                       @ [sp] the class byte, [sp + 4] the folded register
	lsr	r0, r3, #4
	mov	r1, #1
	ldr	r2, [sp, #4]
	cmp	r2, #1
	bleq	blk_prefix_skip
	blne	blk_prefix
	ldr	r3, [sp]
	and	r0, r3, #15
	mov	r1, #2
	ldr	r2, [sp, #4]
	cmp	r2, #2
	bleq	blk_prefix_skip
	blne	blk_prefix
	PMARK	10
	ldr	r3, [sp]                        @ the prefix wrote r1 / r2 (and r3 as a scratch): forwarding forgets them
	cmp	r3, #0x10
	movhs	r0, #1
	blhs	blk_mdrop_reg
	ldr	r3, [sp]
	ands	r3, r3, #15
	movne	r0, #2
	blne	blk_mdrop_reg
	ldr	r3, [sp]
	cmp	r3, #0
	movne	r0, #3
	blne	blk_mdrop_reg
	PMARK	11
	add	sp, sp, #8
	pop	{r1, r2}
	bl	blk_emit_plain
	PMARK	12                  @ r1 = the leader (0: none), r2 = the descriptor
	mov	r4, r11                         @ the next instruction
	b	p3_loop

@ ---- Bcc / BRA ----
k_bcc:
	bl	blk_bdisp                       @ r0 = the target, r4 advanced past the instruction
	ubfx	r3, r11, #8, #4                 @ the condition
	cmp	r3, #0
	beq	1f
	mov	r1, #0
	cmp	r0, r4
	movlo	r1, #3                          @ a backward target: its stub polls for interrupts
	bl	blk_stub                        @ r2 = the stub
	ubfx	r3, r11, #8, #4
	.if	!PJIT_BLK_LIVE
	ldr	r0, [r7, #-4]
	tst	r0, #0x04000000
	bne	c_nat
	.endif
	cmp	r3, #2
	beq	c_hi
	cmp	r3, #3
	beq	c_ls
	GET	r0, blk_cc
	ldrb	r0, [r0, r3]
	lsl	r0, r0, #28
	orr	r0, r0, #0x0A000000
	bl	blk_br_stub
	b	p3_loop
	.if	!PJIT_BLK_LIVE
c_nat:                                  @ the compare in front left the ARM flags in their natural polarity (C = no borrow): CS / CC swap, HI and LS are single ARM conditions
	GET	r0, blk_ccn
	ldrb	r0, [r0, r3]
	lsl	r0, r0, #28
	orr	r0, r0, #0x0A000000
	bl	blk_br_stub
	b	p3_loop
	.endif
c_hi:                                   @ HI: not carry and not zero:  bcs .+8 ; bne stub
	mov	r0, #0x2A000000
	str	r0, [r6], #4
	mov	r0, #0x1A000000
	bl	blk_br_stub
	b	p3_loop
c_ls:                                   @ LS: carry or zero:  bcs stub ; beq stub
	mov	r0, #0x2A000000
	bl	blk_br_stub
	mov	r0, #0x0A000000
	bl	blk_br_stub
	b	p3_loop
1:	mov	r1, #0                          @ BRA
	cmp	r0, r4
	movlo	r1, #3
	bl	blk_stub
	mov	r0, #0xEA000000
	bl	blk_br_stub
	b	blk_finish

@ ---- DBcc ----
k_dbcc:
	ldrh	r0, [r4, #2]
	rev16	r0, r0
	sxth	r0, r0
	add	r0, r0, r4
	add	r0, r0, #2                      @ the target
	add	r4, r4, #4
	mov	r1, #0
	cmp	r0, r4
	movlo	r1, #3
	bl	blk_stub
	push	{r2}                            @ the stub
	ubfx	r3, r11, #8, #4                 @ the condition
	cmp	r3, #1
	beq	d_tpl                           @ F: no test
	cmp	r3, #0
	beq	d_t
	cmp	r3, #2
	beq	d_hi
	cmp	r3, #3
	beq	d_ls
	GET	r0, blk_cc
	ldrb	r0, [r0, r3]
	lsl	r0, r0, #28
	orr	r0, r0, #0x0A000000
	orr	r0, r0, #8                      @ one test word, then 8 template words and `b stub`: to the next instruction
	str	r0, [r6], #4
	b	d_tpl
d_t:	CONST	r0, 0xEA000008
	str	r0, [r6], #4
	b	d_tpl
d_hi:	mov	r0, #0x2A000000                 @ bcs .+8 (over the next word)
	str	r0, [r6], #4
	CONST	r0, 0x1A000008           @ bne next
	str	r0, [r6], #4
	b	d_tpl
d_ls:	CONST	r0, 0x2A000009           @ bcs next
	str	r0, [r6], #4
	CONST	r0, 0x0A000008           @ beq next
	str	r0, [r6], #4
d_tpl:	and	r3, r11, #7                     @ the counter register
	GET	r0, blk_dbtpl
	add	r0, r0, r3, lsl #5              @ 8 words each
	mov	r3, #8
1:	ldr	r1, [r0], #4
	str	r1, [r6], #4
	subs	r3, r3, #1
	bne	1b
	pop	{r2}
	mov	r0, #0xEA000000
	bl	blk_br_stub                     @ the counter did not run out: branch
	b	p3_loop

@ ---- BSR, JSR, JMP ----
k_bsr:
	bl	blk_bdisp                       @ r0 = the target; r4 = the return address
	mov	r1, #2                          @ a call stub
	bl	blk_stub
	bl	blk_emit_call
	b	p3_loop                        @ the block goes on at the return address

k_jsr:
	mov	r12, #1
	b	k_jump
k_jmp:
	mov	r12, #0
k_jump:                                 @ r12 = 1 for JSR; r1 = leader (0), r2 = the descriptor (the target-computing body, for a dynamic target)
	bl	blk_static                      @ r0 = the target and r3 = the size if static, else r3 = 0
	cmp	r3, #0
	beq	j_dyn
	add	r4, r4, r3                      @ r4 = the next instruction
	cmp	r12, #0
	beq	1f
	mov	r1, #2
	bl	blk_stub
	bl	blk_emit_call
	b	p3_loop
1:	mov	r1, #0
	cmp	r0, r4
	movlo	r1, #3
	bl	blk_stub
	mov	r0, #0xEA000000
	bl	blk_br_stub
	b	blk_finish
j_dyn:                                  @ dynamic: the prefix, then the generator's JMP body leaves the target in r0
	push	{r2, r12}
	add	r0, r5, #ARENA_EXT
	ldrb	r3, [r0, r11]
	add	r11, r4, #2
	lsr	r0, r3, #4
	mov	r1, #1
	bl	blk_prefix
	pop	{r2, r12}
	push	{r12}
	bl	blk_body
	mov	r4, r11                         @ the next instruction
	pop	{r12}
	cmp	r12, #0
	beq	1f
	GET	r0, blk_dtpl_a                  @ JSR: the shadow entry and the call, then the block goes on at the return address
	mov	r1, #1
	bl	blk_copyw
	mov	r0, r4
	mov	r1, #3
	bl	blk_loadc
	.if	PJIT_BLK_INLINE_GOTO
	GET	r0, blk_dtpl_b2                 @ (the fast-table lookup inline, so this site's own `blx r3` is what the branch predictor sees: the continuation is further on)
	mov	r1, #5
	bl	blk_copyw
	GET	r0, blk_gototpl
	mov	r1, #11
	bl	blk_copyw
	GET	r0, pjit_blk_slow
	mov	r1, #0xEA000000
	bl	blk_emit_br
	CONST	r0, 0xE12FFF33                  @ blx r3
	str	r0, [r6], #4
	b	p3_loop
	.else
	GET	r0, blk_dtpl_b
	mov	r1, #5
	bl	blk_copyw
	GET	r0, pjit_goto
	mov	r1, #0xEB000000
	bl	blk_emit_br
	b	p3_loop
	.endif
1:
	.if	PJIT_BLK_INLINE_GOTO
	GET	r0, blk_gototpl
	mov	r1, #11
	bl	blk_copyw
	GET	r0, pjit_blk_slow
	mov	r1, #0xEA000000
	bl	blk_emit_br
	CONST	r0, 0xE12FFF13                  @ bx r3
	str	r0, [r6], #4
	b	blk_finish
	.else
	GET	r0, pjit_goto
	mov	r1, #0xEA000000
	bl	blk_emit_br
	b	blk_finish
	.endif

k_rts:
	add	r4, r4, #2
	CONST	r0, 0xE49D0004                  @ ldr r0, [sp], #4
	str	r0, [r6], #4
	.if	PJIT_BLK_INLINE_RTS
	GET	r0, blk_rtstpl                  @ the shadow check inline (a shared `bx lr` predicts worse than one per site)
	mov	r1, #9
	bl	blk_copyw
	GET	r0, pjit_goto
	mov	r1, #0xEA000000
	bl	blk_emit_br
	CONST	r0, 0xE12FFF1E                  @ bx lr
	str	r0, [r6], #4
	.else
	GET	r0, pjit_blk_rts
	mov	r1, #0xEA000000
	bl	blk_emit_br
	.endif
	b	blk_finish

@ ---- the end of a block: the stubs, the patching of the branches that go to them, commit, flush ----
blk_finish:
	ldr	r11, [r8, #BLK_NSTUB]           @ the first branch site of each stub
	mov	r4, #0
	mov	r0, #0
	add	r1, r8, #BLK_STUB_SITE
1:	cmp	r4, r11
	strlo	r0, [r1, r4, lsl #2]
	addlo	r4, r4, #1
	blo	1b
	ldr	r3, [r8, #BLK_NSITE]
	mov	r4, #0
2:	cmp	r4, r3
	bhs	4f
	add	r0, r8, #BLK_SITE_STUB
	ldr	r2, [r0, r4, lsl #2]
	add	r0, r8, #BLK_STUB_SITE
	ldr	r12, [r0, r2, lsl #2]
	cmp	r12, #0
	bne	3f
	add	r1, r8, #BLK_SITE_AT
	ldr	r1, [r1, r4, lsl #2]
	str	r1, [r0, r2, lsl #2]
3:	add	r4, r4, #1
	b	2b
4:	mov	r4, #0                          @ stub index
	cmp	r11, #0
	beq	f_sites
f_stub:
	add	r0, r8, #BLK_STUB_KIND
	ldr	r1, [r0, r4, lsl #2]            @ the kind (it will hold the stub's address)
	str	r6, [r0, r4, lsl #2]
	cmp	r1, #2
	beq	f_call
	CONST	r0, 0xE1A0E00F                  @ mov lr, pc
	str	r0, [r6], #4
	mov	r12, r1                         @ the kind: 0 jump, 1 interpret-one, 3 backward jump
	GET	r0, pjit_blk_exit
	GET	r1, pjit_blk_exit_one
	cmp	r12, #1
	moveq	r0, r1
	GET	r1, pjit_blk_exit_b
	cmp	r12, #3
	moveq	r0, r1
	mov	r1, #0xEA000000
	bl	blk_emit_br
	add	r0, r8, #BLK_STUB_TGT
	ldr	r0, [r0, r4, lsl #2]
	str	r0, [r6], #4                    @ the literal: the guest target
	add	r0, r8, #BLK_STUB_SITE
	ldr	r0, [r0, r4, lsl #2]
	str	r0, [r6], #4                    @ the fourth word: the site
	b	f_next
f_call:                                 @ movw r0, #lo ; movt r0, #hi ; bl pjit_blk_call_exit
	add	r0, r8, #BLK_STUB_TGT
	ldr	r2, [r0, r4, lsl #2]
	mov	r1, #0
	movw	r3, #0
	movt	r3, #0xE300
	uxth	r0, r2
	push	{r2}
	mov	r2, r0
	bl	blk_movwt
	pop	{r2}
	lsr	r2, r2, #16
	movw	r3, #0
	movt	r3, #0xE340
	bl	blk_movwt
	GET	r0, pjit_blk_call_exit
	mov	r1, #0xEB000000
	bl	blk_emit_br
	add	r0, r8, #BLK_STUB_SITE
	ldr	r0, [r0, r4, lsl #2]
	str	r0, [r6], #4                    @ the fourth word: the site
f_next:
	add	r4, r4, #1
	cmp	r4, r11
	blo	f_stub
f_sites:
	ldr	r11, [r8, #BLK_NSITE]
	mov	r4, #0
	cmp	r11, #0
	beq	f_commit
f_site:
	add	r0, r8, #BLK_SITE_AT
	ldr	r0, [r0, r4, lsl #2]            @ the branch word
	add	r1, r8, #BLK_SITE_STUB
	ldr	r1, [r1, r4, lsl #2]
	add	r2, r8, #BLK_STUB_KIND
	ldr	r1, [r2, r1, lsl #2]            @ the stub's address
	sub	r1, r1, r0
	sub	r1, r1, #8
	mov	r1, r1, asr #2
	bic	r1, r1, #0xFF000000
	ldr	r2, [r0]
	bic	r2, r2, #0x00FFFFFF
	orr	r2, r2, r1
	str	r2, [r0]
	add	r4, r4, #1
	cmp	r4, r11
	blo	f_site
f_commit:
	PMARK	5
	sub	r2, r6, r9                      @ used
	mov	r0, r8
	mov	r1, r10
	bl	bc_commit
	PMARK	6
	mov	r11, r0
	ldr	r0, [r8, #BLK_N_TRANS]
	add	r0, r0, #1
	str	r0, [r8, #BLK_N_TRANS]
	mov	r0, r9
	mov	r1, r6
	FLUSH_RANGE r0, r1, r2
	PMARK	7
	mov	r0, r11
	pop	{pc}

blk_fatal:
	bkpt	#1

@ =================================================================================================================================
@ Helpers of the translator (r6 = the output cursor, r8 = pjit_blk, r4 = the guest cursor, r5 = the table). They clobber r0-r3 and r12 unless said otherwise.

@ the size in bytes of the instruction r11 (opcode) of kind r3 at r4: -> r0. Clobbers r1, r2, r12.
blk_isize:
	cmp	r3, #1
	beq	2f
	cmp	r3, #2
	beq	1f
	cmp	r3, #4
	beq	1f
	cmp	r3, #3
	moveq	r0, #4
	bxeq	lr
	cmp	r3, #7
	moveq	r0, #2
	bxeq	lr
	and	r0, r11, #0x3F                  @ JSR / JMP: the effective address decides
	cmp	r0, #0x38
	moveq	r0, #4
	bxeq	lr
	cmp	r0, #0x39
	moveq	r0, #6
	bxeq	lr
	cmp	r0, #0x3A
	moveq	r0, #4
	bxeq	lr
2:	add	r0, r5, #ARENA_EXT              @ extension words by the class of the source and the destination
	ldrb	r0, [r0, r11]
	GET	r2, blk_extn
	lsr	r1, r0, #4
	ldrb	r1, [r2, r1]
	and	r0, r0, #15
	ldrb	r0, [r2, r0]
	add	r0, r0, r1
	lsl	r0, r0, #1
	add	r0, r0, #2
	bx	lr
1:	and	r0, r11, #0xFF                  @ Bcc / BSR: 2 bytes, or 4 with a 16-bit displacement
	cmp	r0, #0
	moveq	r0, #4
	movne	r0, #2
	bx	lr
blk_extn:
	.byte	0, 1, 2, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0
	.balign	4

@ skip the extension words of an operand of class r0 (it is folded into the leader): r11 += 2 * words
blk_prefix_skip:
	GET	r2, blk_extn
	ldrb	r0, [r2, r0]
	add	r11, r11, r0, lsl #1
	bx	lr

	.if	PJIT_BLK_FOLD
@ Offset folding. r0 = the opcode's pjit_bv word (bits 29:28: which of r1 / r2 is the leader's dead offset register), r1 = the leader, r3 = the extension class byte, r4 = the
@ instruction's address. -> r12 = the register folded (1 / 2) and r0 = the leader with an immediate offset in place of that register; or r12 = 0 (the operand is not a
@ constant, or the displacement does not fit). The prefix of that operand then is not emitted at all (blockasm.constfold's `off` rule).
blk_foldcheck:
	push	{r7, r9, lr}
	mov	r12, #0
	and	r7, r1, #15                     @ rm, the offset register
	cmp	r7, #1
	bne	1f
	tst	r0, #0x10000000
	beq	9f
	lsr	r9, r3, #4                      @ the source operand's class
	add	r0, r4, #2                      @ its first extension word
	b	2f
1:	cmp	r7, #2
	bne	9f
	tst	r0, #0x20000000
	beq	9f
	and	r9, r3, #15                     @ the destination's
	lsr	r2, r3, #4
	GET	r0, blk_extn
	ldrb	r2, [r0, r2]
	add	r0, r4, #2
	add	r0, r0, r2, lsl #1              @ past the source's extension words
2:	cmp	r9, #1
	beq	3f
	cmp	r9, #2
	beq	4f
	cmp	r9, #4
	beq	5f
	b	9f                              @ an indexed operand: not a constant
3:	ldrh	r2, [r0]                        @ abs.W / #imm16 / d16(An): the sign-extended word
	rev16	r2, r2
	sxth	r2, r2
	b	6f
4:	ldrh	r2, [r0]                        @ a 32-bit constant
	rev16	r2, r2
	ldrh	r9, [r0, #2]
	rev16	r9, r9
	orr	r2, r9, r2, lsl #16
	b	6f
5:	ldrh	r9, [r0]                        @ d16(PC): the word's own address plus the displacement
	rev16	r9, r9
	sxth	r9, r9
	add	r2, r9, r0
6:	mov	r0, #0x00800000                 @ U (add) unless the displacement is negative
	cmp	r2, #0
	movlt	r0, #0
	mov	r9, r2
	rsblt	r9, r2, #0                      @ |v|
	tst	r1, #0x04000000                 @ bit 26: ldr / ldrb / str / strb; otherwise the halfword / signed forms
	beq	7f
	cmp	r9, #4096
	bhs	8f
	and	r2, r1, #0xF0000000
	orr	r0, r0, r2
	orr	r0, r0, #0x05000000
	and	r2, r1, #0x00500000             @ B, L
	orr	r0, r0, r2
	and	r2, r1, #0x000FF000             @ Rn, Rt
	orr	r0, r0, r2
	orr	r0, r0, r9
	mov	r12, r7
	b	9f
7:	cmp	r9, #256
	bhs	8f
	and	r2, r1, #0xF0000000
	orr	r0, r0, r2
	orr	r0, r0, #0x01400000
	and	r2, r1, #0x00100000             @ L
	orr	r0, r0, r2
	and	r2, r1, #0x000FF000             @ Rn, Rt
	orr	r0, r0, r2
	and	r2, r1, #0xF0                   @ 1SH1
	orr	r0, r0, r2
	lsr	r2, r9, #4
	orr	r0, r0, r2, lsl #8
	and	r2, r9, #15
	orr	r0, r0, r2
	mov	r12, r7
	b	9f
8:	mov	r12, #0                         @ does not fit
9:	pop	{r7, r9, pc}
	.endif

@ the stub table: r0 = the guest target, r1 = the kind -> r2 = the stub's index
blk_stub:
	ldr	r2, [r8, #BLK_NSTUB]
	add	r3, r8, #BLK_STUB_TGT
	str	r0, [r3, r2, lsl #2]
	add	r3, r8, #BLK_STUB_KIND
	str	r1, [r3, r2, lsl #2]
	add	r3, r2, #1
	str	r3, [r8, #BLK_NSTUB]
	bx	lr

@ emit a branch word r0 (condition and opcode bits, offset zero) that will go to stub r2
blk_br_stub:
	ldr	r3, [r8, #BLK_NSITE]
	add	r12, r8, #BLK_SITE_AT
	str	r6, [r12, r3, lsl #2]
	add	r12, r8, #BLK_SITE_STUB
	str	r2, [r12, r3, lsl #2]
	add	r3, r3, #1
	str	r3, [r8, #BLK_NSITE]
	str	r0, [r6], #4
	bx	lr

@ copy r1 words from r0 to the output
blk_copyw:
1:	ldr	r2, [r0], #4
	str	r2, [r6], #4
	subs	r1, r1, #1
	bne	1b
	bx	lr

@ a static call: r4 = the return address, r2 = the call stub's index. Emits: the shadow key, the push of the return address, the shadow entry (guest return address,
@ host continuation), and `bl <stub>` (the stub is `movw r0 ; movt r0 ; bl pjit_blk_call_exit`, which becomes `b <block>` once linked).
blk_emit_call:
	push	{r2, lr}
	GET	r0, blk_ctpl_a
	mov	r1, #1
	bl	blk_copyw
	mov	r0, r4
	mov	r1, #0
	bl	blk_loadc
	GET	r0, blk_ctpl_b
	mov	r1, #5
	bl	blk_copyw
	ldr	r2, [sp]
	mov	r0, #0xEB000000
	bl	blk_br_stub
	add	sp, sp, #4
	pop	{pc}

@ emit a branch to the absolute address r0, r1 = the opcode bits (cond | 0x0A000000 for b, 0x0B000000 for bl)
blk_emit_br:
	sub	r2, r0, r6
	sub	r2, r2, #8
	mov	r2, r2, asr #2
	bic	r2, r2, #0xFF000000
	orr	r2, r2, r1
	str	r2, [r6], #4
	bx	lr

@ Bcc / BRA / BSR displacement: -> r0 = the target, r4 += the instruction's size (r11 = the opcode)
blk_bdisp:
	and	r0, r11, #0xFF
	cmp	r0, #0
	bne	1f
	ldrh	r0, [r4, #2]
	rev16	r0, r0
	sxth	r0, r0
	mov	r12, #4
	b	2f
1:	sxtb	r0, r0
	mov	r12, #2
2:	add	r0, r0, r4
	add	r0, r0, #2
	add	r4, r4, r12
	bx	lr

@ JSR / JMP with a static target (abs.W, abs.L, d16(PC)): -> r0 = the target and r3 = the instruction's size, or r3 = 0 (dynamic). Keeps r1, r2, r12.
blk_static:
	and	r3, r11, #0x3F
	cmp	r3, #0x38
	beq	1f                              @ abs.W
	cmp	r3, #0x39
	beq	2f                              @ abs.L
	cmp	r3, #0x3A
	beq	3f                              @ d16(PC)
	mov	r3, #0
	bx	lr
1:	ldrh	r0, [r4, #2]
	rev16	r0, r0
	sxth	r0, r0
	mov	r3, #4
	bx	lr
2:	ldrh	r0, [r4, #2]
	rev16	r0, r0
	ldrh	r3, [r4, #4]
	rev16	r3, r3
	orr	r0, r3, r0, lsl #16
	mov	r3, #6
	bx	lr
3:	ldrh	r0, [r4, #2]
	rev16	r0, r0
	sxth	r0, r0
	add	r0, r0, r4
	add	r0, r0, #2
	mov	r3, #4
	bx	lr

@ ---- load forwarding (tools/gen/fwd.py) -------------------------------------------------------------------------------------------------------------
@ The D registers live in memory, so each instruction loads its sources and stores its result. blk->mirror says which of r0-r3 still holds which D register slot (a byte
@ per register: n << 4 | kind; kind 1 w, 2 h, 3 sh, 4 b, 5 sb, 6 l16, 7 l8); a load of a slot some register mirrors becomes a register move or an extend, or disappears.

@ mirror helpers (r8 = pjit_blk): clobber r1, r2, r3
blk_mdrop_reg:                          @ r0 = a register: forget what it mirrors (only r0-r3 are tracked)
	cmp	r0, #4
	bxhs	lr
	ldr	r1, [r8, #BLK_MIRROR]
	lsl	r2, r0, #3
	mov	r3, #0xFF
	bic	r1, r1, r3, lsl r2
	str	r1, [r8, #BLK_MIRROR]
	bx	lr
blk_mdrop_n:                            @ r0 = a D register: forget every mirror of it
	ldr	r1, [r8, #BLK_MIRROR]
	mov	r2, #0                          @ byte index * 8
1:	lsr	r3, r1, r2
	ands	r3, r3, #0xFF
	beq	2f
	lsr	r3, r3, #4
	cmp	r3, r0
	bne	2f
	mov	r3, #0xFF
	bic	r1, r1, r3, lsl r2
2:	add	r2, r2, #8
	cmp	r2, #32
	blo	1b
	str	r1, [r8, #BLK_MIRROR]
	bx	lr
blk_mset:                               @ r0 = a register, r1 = the entry: it mirrors that
	cmp	r0, #4
	bxhs	lr
	ldr	r2, [r8, #BLK_MIRROR]
	lsl	r3, r0, #3
	push	{r1}
	mov	r1, #0xFF
	bic	r2, r2, r1, lsl r3
	pop	{r1}
	orr	r2, r2, r1, lsl r3
	str	r2, [r8, #BLK_MIRROR]
	bx	lr
blk_mdrop_mask:                         @ r0 = a mask of registers (bits 3:0): forget what each mirrors
	ldr	r1, [r8, #BLK_MIRROR]
	mov	r2, #0
1:	mov	r3, #1
	tst	r0, r3, lsl r2
	beq	2f
	mov	r3, #0xFF
	lsl	r12, r2, #3
	bic	r1, r1, r3, lsl r12
2:	add	r2, r2, #1
	cmp	r2, #4
	blo	1b
	str	r1, [r8, #BLK_MIRROR]
	bx	lr

@ a load or store of a D register slot at r5 + imm?  r0 = the word -> r1 = n | part << 3 | load << 6 | cond << 7 | rt << 8, or -1. Clobbers r2, r3, r12.
blk_slot:
	push	{r4}
	.ifdef	PJIT_PIN_D0
	@ the pinned D0 (r4) as a slot: `mov rd, r4` / `uxt* sxt* rd, r4` are loads of D0, `mov r4, rs` / `bfi r4, rs, #0, #8|16` are stores (tools/gen/fwd.py slot_pinned)
	CONST	r3, 0x0FFF0FF0
	and	r2, r0, r3
	CONST	r12, 0x01A00000                 @ mov rd, rm
	cmp	r2, r12
	bne	6f
	and	r2, r0, #15
	ubfx	r3, r0, #12, #4
	mov	r12, #1                         @ part w
	cmp	r2, #4
	bne	1f
	cmp	r3, #4
	beq	s_pno
	mov	r1, #(0 | 1 << 3 | 1 << 6)      @ a load of D0, whole
	b	s_pdone
1:	cmp	r3, #4
	bne	s_pno
	mov	r1, #(0 | 1 << 3)               @ a store of D0, whole; rt = rm
	mov	r3, r2
	b	s_pdone
6:	and	r2, r0, #15
	cmp	r2, #4
	bne	7f
	ubfx	r3, r0, #12, #4
	cmp	r3, #4
	beq	7f
	CONST	r2, 0x06AF0070                  @ sxtb
	CONST	r12, 0x0FFF0FF0
	and	r12, r0, r12
	cmp	r12, r2
	moveq	r1, #(0 | 5 << 3 | 1 << 6)
	beq	s_pdone
	CONST	r2, 0x06EF0070                  @ uxtb
	cmp	r12, r2
	moveq	r1, #(0 | 4 << 3 | 1 << 6)
	beq	s_pdone
	CONST	r2, 0x06BF0070                  @ sxth
	cmp	r12, r2
	moveq	r1, #(0 | 3 << 3 | 1 << 6)
	beq	s_pdone
	CONST	r2, 0x06FF0070                  @ uxth
	cmp	r12, r2
	moveq	r1, #(0 | 2 << 3 | 1 << 6)
	beq	s_pdone
7:	CONST	r12, 0x0FFFFFF0
	and	r12, r0, r12
	CONST	r2, 0x07C74010                  @ bfi r4, rs, #0, #8
	cmp	r12, r2
	moveq	r1, #(0 | 7 << 3)
	andeq	r3, r0, #15
	beq	s_pdone
	CONST	r2, 0x07CF4010                  @ bfi r4, rs, #0, #16
	cmp	r12, r2
	bne	s_pno
	mov	r1, #(0 | 6 << 3)
	and	r3, r0, #15
s_pdone:                                @ r1 = n(0) | part << 3 | load << 6, r3 = rt
	ubfx	r2, r0, #28, #4
	cmp	r2, #14
	orrne	r1, r1, #0x80
	orr	r1, r1, r3, lsl #8
	pop	{r4}
	bx	lr
s_pno:
	.endif
	ubfx	r2, r0, #16, #4
	cmp	r2, #5
	bne	s_no
	tst	r0, #0x01000000                 @ P = 1
	beq	s_no
	tst	r0, #0x00200000                 @ W = 0
	bne	s_no
	and	r2, r0, #0x0E000000
	cmp	r2, #0x04000000
	beq	s_word
	cmp	r2, #0
	bne	s_no
	tst	r0, #0x00400000                 @ ldrh family, immediate offset
	beq	s_no
	and	r2, r0, #0x90
	cmp	r2, #0x90
	bne	s_no
	ubfx	r3, r0, #5, #2
	cmp	r3, #0
	beq	s_no
	and	r2, r0, #0xF
	lsr	r12, r0, #4
	and	r12, r12, #0xF0
	orr	r2, r2, r12                     @ |offset|
	ubfx	r12, r0, #20, #1                @ load
	cmp	r3, #1
	bne	1f
	cmp	r12, #0
	moveq	r3, #6                          @ strh: l16
	movne	r3, #2                          @ ldrh: h
	b	s_off
1:	cmp	r12, #0
	beq	s_no                            @ (ldrd / strd forms)
	cmp	r3, #2
	moveq	r3, #5                          @ ldrsb: sb
	movne	r3, #3                          @ ldrsh: sh
	b	s_off
s_word:
	lsl	r2, r0, #20
	lsr	r2, r2, #20                     @ |offset|
	ubfx	r12, r0, #20, #1
	tst	r0, #0x00400000                 @ byte
	bne	1f
	mov	r3, #1                          @ w
	b	s_off
1:	cmp	r12, #0
	movne	r3, #4                          @ ldrb: b
	moveq	r3, #7                          @ strb: l8
s_off:                                  @ r2 = |offset|, r3 = the part, r12 = load
	tst	r0, #0x00800000                 @ U = 0: negative
	rsbeq	r2, r2, #0
	add	r2, r2, #(-CPU_D0)              @ relative to D0
	cmp	r2, #32
	bhs	s_no
	and	r1, r2, #3                      @ byte offset in the register's big-endian word
	lsr	r2, r2, #2                      @ n
	GET	r4, blk_boff
	ldrb	r4, [r4, r3]
	cmp	r1, r4
	bne	s_no
	orr	r1, r2, r3, lsl #3
	orr	r1, r1, r12, lsl #6
	ubfx	r2, r0, #28, #4
	cmp	r2, #14
	orrne	r1, r1, #0x80
	ubfx	r2, r0, #12, #4
	orr	r1, r1, r2, lsl #8
	pop	{r4}
	bx	lr
s_no:
	mvn	r1, #0
	pop	{r4}
	bx	lr
blk_boff:                               @ the byte offset a part sits at inside the register's 4 bytes (big-endian): w 0, h / sh / l16 2, b / sb / l8 3
	.byte	0, 0, 2, 2, 3, 3, 2, 3
blk_extw:                               @ the extends: uxth, sxth, uxtb, sxtb (actions 2..5), index action - 2
	.word	0xE6FF0070, 0xE6BF0070, 0xE6EF0070, 0xE6AF0070
@ rows: the part being loaded (w h sh b sb); columns: the kind of the mirror (0, w h sh b sb l16 l8); entries: 0 none, 1 mov, 2 uxth, 3 sxth, 4 uxtb, 5 sxtb
blk_rules:
	.byte	0, 1, 0, 0, 0, 0, 0, 0
	.byte	0, 2, 1, 2, 0, 0, 2, 0
	.byte	0, 3, 3, 1, 0, 0, 3, 0
	.byte	0, 4, 4, 4, 1, 4, 4, 4
	.byte	0, 5, 5, 5, 5, 1, 5, 5
	.balign	4

@ r0 = n, r1 = the part being loaded (1..5) -> r0 = a register that mirrors it (0..3) or -1, r1 = the action (blk_rules). Clobbers r2, r3, r12.
blk_mfind:
	push	{r4}
	ldr	r2, [r8, #BLK_MIRROR]
	sub	r1, r1, #1
	lsl	r1, r1, #3
	GET	r3, blk_rules
	add	r3, r3, r1                      @ the row
	mov	r12, #0
1:	and	r1, r2, #0xFF
	lsr	r2, r2, #8
	cmp	r1, #0
	beq	2f
	lsr	r4, r1, #4
	cmp	r4, r0
	bne	2f
	and	r4, r1, #15
	ldrb	r4, [r3, r4]
	cmp	r4, #0
	beq	2f
	mov	r0, r12
	mov	r1, r4
	pop	{r4}
	bx	lr
2:	add	r12, r12, #1
	cmp	r12, #4
	blo	1b
	mvn	r0, #0
	mov	r1, #0
	pop	{r4}
	bx	lr

@ one word of a plain instruction: r0 = the word, r1 = its info (bits 3:0 registers r0-r3 written, 4 barrier, 5 has a branch). Emits it, or a move / extend in its place, or
@ nothing, and updates the mirror. Keeps r4-r11; clobbers r0-r3, r12.
blk_fwd_sd:                             @ r0 = a word that accesses a D register slot, r1 = its description (the generator's, as blk_slot would compute it); not blocked
	push	{r4, r5, lr}
	mov	r4, r0
	mov	r5, r1
	b	fw_slot_have
blk_fwd_word:
	push	{r4, r5, lr}
	mov	r4, r0
	mov	r5, r1
	ldr	r2, [r8, #BLK_FWDST]
	tst	r2, #1
	bne	fw_emit                         @ after a branch inside the instruction: copied as it is
	tst	r5, #0x10
	beq	1f
	mov	r3, #0                          @ a branch / barrier: the map ends here
	str	r3, [r8, #BLK_MIRROR]
	orr	r2, r2, #1
	str	r2, [r8, #BLK_FWDST]
	b	fw_emit
1:	mov	r0, r4
	bl	blk_slot
	cmn	r1, #1
	beq	fw_other
	mov	r5, r1                          @ r5 = the slot description
fw_slot_have:
	tst	r5, #0x80
	beq	fw_unc
	tst	r5, #0x40                       @ conditional: it may or may not have happened
	beq	1f
	ubfx	r0, r5, #8, #4
	bl	blk_mdrop_reg
	b	fw_emit
1:	and	r0, r5, #7
	bl	blk_mdrop_n
	b	fw_emit
fw_unc:
	tst	r5, #0x40
	beq	fw_store
	BUMP	BLK_N_SLD
	and	r0, r5, #7                      @ ---- a load of D(n), in this part
	ubfx	r1, r5, #3, #3
	bl	blk_mfind                       @ r0 = a register that holds it, r1 = how
	cmn	r0, #1
	beq	fw_miss
	ubfx	r2, r5, #8, #4                  @ rt
	cmp	r1, #1
	bne	fw_ext
	cmp	r0, r2
	bne	1f
	ldr	r3, [r8, #BLK_FWDST]            @ mov rt, rt: nothing to do, unless a branch inside the instruction has to land where it did
	tst	r3, #2
	bne	2f
	BUMP	BLK_N_FDEL
	b	fw_after
2:	CONST	r3, 0xE320F000                  @ nop
	str	r3, [r6], #4
	b	fw_after
1:	CONST	r3, 0xE1A00000
	orr	r3, r3, r2, lsl #12
	orr	r3, r3, r0
	str	r3, [r6], #4                    @ mov rt, rx
	BUMP	BLK_N_FMOV
	b	fw_after
fw_ext:
	GET	r3, blk_extw
	sub	r1, r1, #2
	ldr	r3, [r3, r1, lsl #2]
	orr	r3, r3, r2, lsl #12
	orr	r3, r3, r0
	str	r3, [r6], #4                    @ sxth / uxth / sxtb / uxtb rt, rx
	BUMP	BLK_N_FEXT
fw_after:                               @ rt now mirrors the register with the width it was loaded in
	ubfx	r0, r5, #8, #4
	bl	blk_mdrop_reg
	ubfx	r0, r5, #8, #4
	and	r1, r5, #7
	ubfx	r2, r5, #3, #3
	orr	r1, r2, r1, lsl #4
	bl	blk_mset
	b	fw_done
fw_miss:                                @ no register holds it: the load stays
	str	r4, [r6], #4
	b	fw_after
fw_store:
	BUMP	BLK_N_SST                       @ a store into the slot: every other mirror of that register is stale; rt mirrors what it stored
	str	r4, [r6], #4
	and	r0, r5, #7
	bl	blk_mdrop_n
	b	fw_after
fw_other:                               @ not a slot: a write to r0-r3 forgets what it mirrored
	str	r4, [r6], #4
	.ifdef	PJIT_PIN_D0
	ubfx	r0, r4, #12, #4                 @ a leader that may write the pinned D0 (r4 as Rd or Rn: more than needed is harmless): its mirrors go
	ubfx	r1, r4, #16, #4
	cmp	r0, #4
	cmpne	r1, #4
	bne	11f
	mov	r0, #0
	bl	blk_mdrop_n
11:
	.endif
	and	r0, r5, #0xF
	bl	blk_mdrop_mask
	b	fw_done
fw_emit:
	str	r4, [r6], #4
fw_done:
	pop	{r4, r5, pc}

@ a plain instruction's leader and body, through the load forwarder (or, with PJIT_BLK_FWD=0, copied as they are). r1 = the leader (0: none), r2 = the descriptor.
@ Keeps r4-r11; clobbers r0-r3, r12.
blk_emit_plain:
	.if	!PJIT_BLK_FWD
	cmp	r1, #0
	strne	r1, [r6], #4
	b	blk_body
	.else
	push	{r4, r5, r7, lr}
	ubfx	r4, r2, #23, #6                 @ the body's length
	ubfx	r3, r2, #0, #18
	GET	r0, pjit_binfo
	add	r5, r0, r3, lsl #2              @ r5 = its info words (pjit_binfo: tools/gen/btables.py winfo32)
	GET	r0, pjit_bbody
	add	r7, r0, r3, lsl #2              @ r7 = its words
	ubfx	r2, r2, #18, #5                 @ the leader's info
	mov	r12, r1                         @ the leader
	mov	r3, #0                          @ does the instruction contain a branch (bit 31 of the first info word)? bit 1 of FWDST = yes
	cmp	r4, #0
	ldrne	r0, [r5]
	tstne	r0, #0x80000000
	movne	r3, #2
	str	r3, [r8, #BLK_FWDST]            @ not blocked
	cmp	r12, #0
	beq	4f
	mov	r0, r12
	mov	r1, r2
	bl	blk_fwd_word                    @ the leader (a word that may be folded, so decoded here)
4:	cmp	r4, #0
	beq	9f
5:	ldr	r0, [r7], #4                    @ ---- the body, a word at a time: the generator has classified each
	ldr	r1, [r5], #4
	tst	r1, #0x40000000
	bne	8f                              @ a `bl` / `b` to a helper
	ldr	r2, [r8, #BLK_FWDST]
	tst	r2, #1
	bne	3f                              @ after a branch inside the instruction: copied as it is
	tst	r1, #0x50                       @ a barrier or a slot access: the long way
	bne	6f
	str	r0, [r6], #4                    @ an ordinary word; what it writes of r0-r3 is not mirrored
	.ifdef	PJIT_PIN_D0
	tst	r1, #0x20000000                 @ it writes the pinned D0 (r4) other than as a store the map knows: the mirrors of D0 are stale
	beq	10f
	push	{r1}
	mov	r0, #0
	bl	blk_mdrop_n
	pop	{r1}
10:
	.endif
	ands	r2, r1, #0xF
	beq	7f
	GET	r3, blk_mmask
	ldr	r2, [r3, r2, lsl #2]
	ldr	r3, [r8, #BLK_MIRROR]
	and	r3, r3, r2
	str	r3, [r8, #BLK_MIRROR]
	b	7f
3:	str	r0, [r6], #4
	b	7f
6:	tst	r1, #0x10
	bne	2f
	ubfx	r1, r1, #8, #12                 @ a slot access, described by the generator
	bl	blk_fwd_sd
	b	7f
2:	and	r1, r1, #0x3F
	bl	blk_fwd_word                    @ a barrier: the map ends here
	b	7f
8:	mov	r3, #0xEA000000                 @ `bl <helper>` (0xEB..) or `b <helper>` (0xEC..): aimed at the helper, and the end of the forwarding
	lsr	r2, r0, #24
	cmp	r2, #0xEB
	moveq	r3, #0xEB000000
	and	r0, r0, #0xFF
	GET	r1, pjit_bt_helpers
	ldr	r0, [r1, r0, lsl #2]
	mov	r1, r3
	bl	blk_emit_br
	mov	r3, #0
	str	r3, [r8, #BLK_MIRROR]
	ldr	r3, [r8, #BLK_FWDST]
	orr	r3, r3, #1
	str	r3, [r8, #BLK_FWDST]
7:	subs	r4, r4, #1
	bne	5b
9:	pop	{r4, r5, r7, pc}
	.endif

@ the AND masks that forget the mirrors of the registers r0-r3 in a 4-bit mask
blk_mmask:
	.irp	m, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
	.word	~(((\m & 1) * 0xFF) | (((\m >> 1) & 1) * 0xFF00) | (((\m >> 2) & 1) * 0xFF0000) | (((\m >> 3) & 1) * 0xFF000000))
	.endr

@ copy a body: r2 = the descriptor (length << 20 | offset into pjit_bbody); a `bl <helper>` (top byte 0xEB) is re-aimed. Clobbers r0-r3, r12.
blk_body:
	push	{lr}
	ubfx	r3, r2, #23, #6
	ubfx	r2, r2, #0, #18
	GET	r0, pjit_bbody
	add	r2, r0, r2, lsl #2
	cmp	r3, #0
	beq	9f
1:	ldr	r0, [r2], #4
	lsr	r1, r0, #24
	cmp	r1, #0xEB
	beq	2f
	cmp	r1, #0xEC
	beq	2f
	str	r0, [r6], #4
3:	subs	r3, r3, #1
	bne	1b
9:	pop	{pc}
2:	cmp	r1, #0xEB
	mov	r1, #0xEA000000
	moveq	r1, #0xEB000000
	and	r0, r0, #0xFF
	push	{r1}
	GET	r1, pjit_bt_helpers
	ldr	r0, [r1, r0, lsl #2]
	pop	{r1}
	push	{r2, r3}
	bl	blk_emit_br
	pop	{r2, r3}
	b	3b

@ the smallest rotation field (12 bits) that makes r0 an ARM immediate: -> r2 (-1 if none). Clobbers r3, r12.
blk_rotimm:
	mov	r12, #0
1:	lsl	r3, r12, #1
	rsb	r3, r3, #0
	and	r3, r3, #31
	ror	r2, r0, r3
	cmp	r2, #256
	blo	2f
	add	r12, r12, #1
	cmp	r12, #16
	blo	1b
	mvn	r2, #0
	bx	lr
2:	orr	r2, r2, r12, lsl #8
	bx	lr

@ emit code that loads the constant r0 into register r1 (mov / mvn / movw [+ movt]), as blockasm.load_const does
blk_loadc:
	push	{r0, r1, lr}
	bl	blk_rotimm
	cmn	r2, #1
	beq	2f
	ldr	r1, [sp, #4]
	movw	r3, #0
	movt	r3, #0xE3A0                     @ mov rd, #imm
	b	8f
2:	mvn	r0, r0
	bl	blk_rotimm
	cmn	r2, #1
	beq	3f
	ldr	r1, [sp, #4]
	movw	r3, #0
	movt	r3, #0xE3E0                     @ mvn rd, #imm
8:	orr	r3, r3, r2
	orr	r3, r3, r1, lsl #12
	str	r3, [r6], #4
	pop	{r0, r1, pc}
3:	ldr	r0, [sp]
	ldr	r1, [sp, #4]
	movw	r3, #0
	movt	r3, #0xE300                     @ movw rd, #lo
	uxth	r2, r0
	bl	blk_movwt
	ldr	r0, [sp]
	lsrs	r2, r0, #16
	beq	4f
	ldr	r1, [sp, #4]
	movw	r3, #0
	movt	r3, #0xE340                     @ movt rd, #hi
	bl	blk_movwt
4:	pop	{r0, r1, pc}
@ r3 = the base word (movw / movt), r2 = imm16, r1 = rd: emit it
blk_movwt:
	and	r12, r2, #0xF000
	orr	r3, r3, r12, lsl #4
	lsl	r12, r2, #20
	orr	r3, r3, r12, lsr #20
	orr	r3, r3, r1, lsl #12
	str	r3, [r6], #4
	bx	lr

@ emit rd = rn + v (v signed): r0 = v, r1 = rd, r2 = rn (blockasm.add_const)
blk_addc:
	push	{r0, r1, r2, lr}
	cmp	r0, #0
	bne	1f
	cmp	r1, r2
	beq	9f
	movw	r3, #0
	movt	r3, #0xE1A0                     @ mov rd, rn
	orr	r3, r3, r1, lsl #12
	orr	r3, r3, r2
	str	r3, [r6], #4
	b	9f
1:	bmi	5f
	bl	blk_rotimm                      @ v > 0
	cmn	r2, #1
	beq	6f
	movw	r3, #0
	movt	r3, #0xE280                     @ add rd, rn, #imm
	b	7f
5:	rsb	r0, r0, #0
	bl	blk_rotimm
	cmn	r2, #1
	beq	6f
	movw	r3, #0
	movt	r3, #0xE240                     @ sub rd, rn, #imm
7:	ldr	r1, [sp, #4]
	ldr	r0, [sp, #8]
	orr	r3, r3, r2
	orr	r3, r3, r1, lsl #12
	orr	r3, r3, r0, lsl #16
	str	r3, [r6], #4
	b	9f
6:	ldr	r0, [sp]                        @ the constant into r3, then add rd, rn, r3
	mov	r1, #3
	bl	blk_loadc
	ldr	r1, [sp, #4]
	ldr	r0, [sp, #8]
	movw	r3, #3
	movt	r3, #0xE080                     @ add rd, rn, r3
	orr	r3, r3, r1, lsl #12
	orr	r3, r3, r0, lsl #16
	str	r3, [r6], #4
9:	pop	{r0, r1, r2, pc}

@ the extension-word prefix of one operand: r0 = its class (1 I16, 2 I32, 3 d8(An,Xn), 4 d16(PC), 5 d8(PC,Xn); 0 none), r1 = rd (1 source, 2 destination), r11 = the
@ guest address of its first extension word (advanced past them). As blockasm.prefix.
blk_prefix:
	push	{lr}
	cmp	r0, #1
	beq	1f
	cmp	r0, #2
	beq	2f
	cmp	r0, #4
	beq	3f
	cmp	r0, #3
	beq	4f
	cmp	r0, #5
	beq	4f
	pop	{pc}
1:	ldrh	r0, [r11], #2
	rev16	r0, r0
	sxth	r0, r0
	bl	blk_loadc
	pop	{pc}
2:	ldrh	r0, [r11], #2
	rev16	r0, r0
	ldrh	r2, [r11], #2
	rev16	r2, r2
	orr	r0, r2, r0, lsl #16
	bl	blk_loadc
	pop	{pc}
3:	mov	r2, r11                         @ the extension word's own address
	ldrh	r0, [r11], #2
	rev16	r0, r0
	sxth	r0, r0
	add	r0, r0, r2
	bl	blk_loadc
	pop	{pc}
4:	ldrh	r2, [r11], #2
	rev16	r2, r2                          @ the extension word
	sub	r3, r11, #2                     @ its address
	cmp	r0, #5
	moveq	r12, r3
	movne	r12, #0                         @ the address, for the PC-relative form
	sxtb	r3, r2
	add	r12, r12, r3                    @ d8 (+ the address)
	push	{r1, r12}                       @ rd, the value for blk_addc
	lsr	r3, r2, #12
	and	r3, r3, #7                      @ xr
	ubfx	r12, r2, #11, #1                @ long
	ubfx	r2, r2, #15, #1                 @ A / D
	lsl	r2, r2, #1
	orr	r2, r2, r12
	lsl	r2, r2, #1
	sub	r12, r1, #1
	orr	r2, r2, r12
	lsl	r2, r2, #3
	orr	r2, r2, r3                      @ ((A * 2 + long) * 2 + (rd - 1)) * 8 + xr
	GET	r0, blk_xtab
	ldr	r0, [r0, r2, lsl #2]
	str	r0, [r6], #4                    @ the load / move of the index register
	pop	{r1, r12}
	mov	r0, r12
	mov	r2, r1
	bl	blk_addc
	pop	{pc}

@ 68K condition (4-bit field) -> the ARM condition nibble (HI and LS are done by the callers; T = AL, F is not a branch)
blk_cc:
	.byte	0xE, 0xE, 0x0, 0x0, 0x3, 0x2, 0x1, 0x0, 0x7, 0x6, 0x5, 0x4, 0xA, 0xB, 0xC, 0xD
	.balign	4
@ the same for flags left by a compare in their natural polarity (C = no borrow): 68K CC = ARM CS, CS = CC, HI = HI, LS = LS
blk_ccn:
	.byte	0xE, 0xE, 0x8, 0x9, 0x2, 0x3, 0x1, 0x0, 0x7, 0x6, 0x5, 0x4, 0xA, 0xB, 0xC, 0xD
	.balign	4

@ the index-register loads of the d8(An,Xn) / d8(PC,Xn) prefixes: ((A * 2 + long) * 2 + (rd - 1)) * 8 + xr
	.macro	AROW op, rd
	\op	r\rd, r6
	\op	r\rd, r7
	\op	r\rd, r8
	\op	r\rd, r9
	\op	r\rd, r10
	\op	r\rd, r11
	\op	r\rd, r12
	\op	r\rd, r13
	.endm
blk_xtab:
	.irp	rd, 1, 2
	.irp	xr, 0, 1, 2, 3, 4, 5, 6, 7
	.ifdef	PJIT_PIN_D0
	.if	\xr == 0
	sxth	r\rd, r4                        @ the pinned D0
	.else
	ldrsh	r\rd, [r5, #(CPU_D0 + 4 * \xr + 2)]
	.endif
	.else
	ldrsh	r\rd, [r5, #(CPU_D0 + 4 * \xr + 2)]
	.endif
	.endr
	.endr
	.irp	rd, 1, 2
	.irp	xr, 0, 1, 2, 3, 4, 5, 6, 7
	.ifdef	PJIT_PIN_D0
	.if	\xr == 0
	mov	r\rd, r4
	.else
	ldr	r\rd, [r5, #(CPU_D0 + 4 * \xr)]
	.endif
	.else
	ldr	r\rd, [r5, #(CPU_D0 + 4 * \xr)]
	.endif
	.endr
	.endr
	AROW	sxth, 1
	AROW	sxth, 2
	AROW	mov, 1
	AROW	mov, 2

@ DBcc's counter: DBcc Dn without touching the flags -- ldrsh / sub / strh the low word, then `clz` tells whether it reached -1 and `add pc` skips the branch
	.macro	DBTPL n
	.ifdef	PJIT_PIN_D0
	.if	\n == 0
	sxth	r0, r4                          @ the pinned D0
	sub	r0, r0, #1
	bfi	r4, r0, #0, #16
	.else
	ldrsh	r0, [r5, #(CPU_D0 + 4 * \n + 2)]
	sub	r0, r0, #1
	strh	r0, [r5, #(CPU_D0 + 4 * \n + 2)]
	.endif
	.else
	ldrsh	r0, [r5, #(CPU_D0 + 4 * \n + 2)]
	sub	r0, r0, #1
	strh	r0, [r5, #(CPU_D0 + 4 * \n + 2)]
	.endif
	add	r3, r0, #1
	clz	r3, r3
	lsr	r3, r3, #5
	add	pc, pc, r3, lsl #2
	nop
	.endm
@ the call sequences (copied into blocks): the shadow key is A7 before the push; the host continuation is the word after the `bl`
	.if	PJIT_BLK_INLINE_GOTO
blk_gototpl:                            @ r0 = the guest target; the hit is `bx r3` / `blx r3`, the miss branch (to pjit_blk_slow) follows the template
	movw	r1, #:lower16:blk_fast
	movt	r1, #:upper16:blk_fast
	ubfx	r2, r0, #1, #BLK_FAST_BITS
	add	r1, r1, r2, lsl #3
	ldr	r2, [r1]
	ldr	r3, [r1, #4]
	eor	r2, r2, r0
	clz	r2, r2
	lsr	r2, r2, #5
	add	pc, pc, r2, lsl #2
	nop
	.endif
blk_rtstpl:
	and	r2, sp, #0x3FC
	add	r3, r5, r2, lsl #1
	ldr	r1, [r3, #CPU_SHADOW]
	ldr	lr, [r3, #(CPU_SHADOW + 4)]
	eor	r1, r1, r0
	clz	r1, r1
	lsr	r1, r1, #5
	add	pc, pc, r1, lsl #2
	nop
blk_ctpl_a:
	and	r2, sp, #0x3FC
blk_ctpl_b:
	str	r0, [sp, #-4]!
	add	r3, r5, r2, lsl #1
	str	r0, [r3, #CPU_SHADOW]
	add	r1, pc, #4
	str	r1, [r3, #(CPU_SHADOW + 4)]
blk_dtpl_a:
	and	r2, sp, #0x3FC
blk_dtpl_b:                             @ (the dynamic call: the return address is in r3, the target in r0)
	str	r3, [sp, #-4]!
	add	r2, r5, r2, lsl #1
	str	r3, [r2, #CPU_SHADOW]
	add	r1, pc, #4
	str	r1, [r2, #(CPU_SHADOW + 4)]
	.if	PJIT_BLK_INLINE_GOTO
blk_dtpl_b2:
	str	r3, [sp, #-4]!
	add	r2, r5, r2, lsl #1
	str	r3, [r2, #CPU_SHADOW]
	add	r1, pc, #52                     @ the word after the inline `blx r3`: this word's pc + 8 + 52 (the rest of the 5-word sequence, 11 template words, `b slow`, `blx`)
	str	r1, [r2, #(CPU_SHADOW + 4)]
	.endif
blk_dbtpl:
	.irp	n, 0, 1, 2, 3, 4, 5, 6, 7
	DBTPL	\n
	.endr
	.ltorg
