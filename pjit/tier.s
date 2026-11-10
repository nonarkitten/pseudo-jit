@ pjit/tier.s -- the tiered build's pieces that are neither the slot runtime (runtime_dr.s) nor the block runtime (blk.s). See docs/design/TIERING.md.
@
@ The hotness counters: pjit_tier_cnt (pjit/blkstate.c) holds 1024 entries of two big-endian words {last target seen at this index, count}, indexed by (target >> 1) & 1023.
@ A different target restarts the entry at K = 129 - PJIT_TIER_T; the count has bit 7 set after PJIT_TIER_T events at one target, and the target is then promoted to a block.
@ The slot router counts in line (pjit_goto in runtime_dr.s, flag-free, big-endian); this is the same update for little-endian code that has parked the flags.

	.syntax unified
	.arm
	.text
	.include "asm_offsets.inc"
	.ifndef	PJIT_TIER_T
	.set	PJIT_TIER_T, 64
	.endif
	.set	PJIT_TIER_TB, 1                  @ the count's trigger bit: the smallest TB with 2^TB >= PJIT_TIER_T (7 for 128); the 'promoted before' mark is the bit above it
	.rept	10
	.if	(1 << PJIT_TIER_TB) < PJIT_TIER_T
	.set	PJIT_TIER_TB, PJIT_TIER_TB + 1
	.endif
	.endr
	.set	PJIT_TIER_K, (1 << PJIT_TIER_TB) + 1 - PJIT_TIER_T    @ a new target's first count; the count has bit TB set after PJIT_TIER_T events
	.set	PJIT_TIER_MARK, 1 << (PJIT_TIER_TB + 1)

@ r0 = a guest target (a plain register value), E=LE, flags free. Counts one event at it. -> r0 = 1 if it reached the threshold (the count restarts at 0), else 0.
@ Clobbers r1-r3. Called with `bl`.
	.global	pjit_tier_count
	.type	pjit_tier_count, %function
pjit_tier_count:
	movw	r1, #:lower16:pjit_tier_cnt
	movt	r1, #:upper16:pjit_tier_cnt
	ubfx	r2, r0, #1, #10
	add	r1, r1, r2, lsl #3
	ldr	r2, [r1]
	rev	r3, r0                          @ the word big-endian code compares with
	cmp	r2, r3
	str	r3, [r1]
	ldr	r2, [r1, #4]
	rev	r2, r2
	andeq	r3, r2, #PJIT_TIER_MARK            @ the 'promoted before' mark stays with its target
	movne	r3, #0
	ubfx	r2, r2, #0, #(PJIT_TIER_TB + 1)
	addeq	r2, r2, #1
	movne	r2, #PJIT_TIER_K
	cmp	r2, #(1 << PJIT_TIER_TB)
	movhs	r2, #0
	movhs	r0, #1
	movlo	r0, #0
	orr	r2, r2, r3
	rev	r2, r2
	str	r2, [r1, #4]
	bx	lr
