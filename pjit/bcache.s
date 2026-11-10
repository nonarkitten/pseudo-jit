@ pjit/bcache.s -- the block cache: tag lookup over a bump-allocated, segmented code ring, with chaining links (design: pjit/bcache.h, docs/design/BLOCK_RUNTIME.md).
@
@ Runtime asm, E=LE, no stack (r4-r11 and lr are parked in bc->save and put back; the caller's A7 is the guest's), AAPCS-callable for the test:
@     bc_lookup(bc, guest) -> code address or 0            bc_reserve(bc, max) -> start of the reservation, or 0
@     bc_commit(bc, guest, used) -> code address            bc_link(bc, stub, type, target) -> 1 or 0 (list full)
@     bc_flush(bc)
@ Internal (bc in r4, everything saved by the caller): bc_turnover, bc_restart, bc_reset_hw.

	.syntax unified
	.arm
	.text
	.include "asm_offsets.inc"

	.ifndef	PJIT_BC_ALIGN
	.set	PJIT_BC_ALIGN, 16
	.endif
	.equ	BC_ALIGN, PJIT_BC_ALIGN
	.equ	BC_HORIZON, 0xE0000000                  @ = BC_HORIZON in bcache.h (tests/qemu/smoke_bcache.c checks they agree)

	.macro	ENTER                                   @ park r4-r11, lr; bc -> r4
	add	r12, r0, #BC_SAVE
	stmia	r12, {r4-r11, lr}
	mov	r4, r0
	.endm
	.macro	LEAVE                                   @ put them back and return (r0 already holds the result)
	add	r12, r4, #BC_SAVE
	ldmia	r12, {r4-r11, lr}
	bx	lr
	.endm
	.macro	SYNC a                                  @ after patching code: make the word at \a visible to instruction fetch
	mcr	p15, 0, \a, c7, c11, 1                  @ DCCMVAU
	mcr	p15, 0, \a, c7, c5, 1                   @ ICIMVAU
	.endm
	.macro	SYNC_END
	dsb
	mcr	p15, 0, r0, c7, c5, 6                   @ BPIALL
	dsb
	isb
	.endm

@ ---------------------------------------------------------------------------------------------------------------------------------
@ r0 = bc, r1 = guest -> r0 = code address of the live block, or 0
	.global	bc_lookup
	.type	bc_lookup, %function
bc_lookup:
	str	r4, [r0, #BC_SAVE]
	str	r5, [r0, #BC_SAVE + 4]
	ldr	r2, [r0, #BC_SET_MASK]
	ldr	r3, [r0, #BC_TAGS]
	and	r2, r2, r1, lsr #1                      @ set = (guest >> 1) & set_mask
	add	r3, r3, r2, lsl #5                      @ 4 ways x 8 bytes
	ldr	r12, [r0, #BC_HW]
	ldr	r2, [r0, #BC_MASK]
	.rept	4
	ldmia	r3!, {r4, r5}                           @ guest, seq
	cmp	r4, r1
	bne	1f
	sub	r4, r12, r5                             @ age = hw - seq
	cmp	r4, r2                                  @ live iff age <= mask (hw is the last byte of the current segment)
	bls	2f                                      @ r5 = seq
1:
	.endr
	mov	r1, #0                                  @ miss
	b	3f
2:	and	r5, r5, r2                              @ ring offset
	ldr	r1, [r0, #BC_RING]
	add	r1, r1, r5
3:	ldr	r4, [r0, #BC_SAVE]
	ldr	r5, [r0, #BC_SAVE + 4]
	mov	r0, r1
	bx	lr
	.size	bc_lookup, . - bc_lookup

@ ---------------------------------------------------------------------------------------------------------------------------------
@ r0 = bc, r1 = max (a multiple of 16) -> r0 = start address of max bytes at the cursor, 0 if max > one segment.
@ If max does not fit in what is left of the segment the cursor moves to the start of the next one (turnover), which kills its old content.
	.global	bc_reserve
	.type	bc_reserve, %function
bc_reserve:
	ENTER
	ldr	r3, [r4, #BC_SEG_SHIFT]
	mov	r2, #1
	mov	r2, r2, lsl r3                          @ S
	cmp	r1, r2
	bhi	8f
	ldr	r5, [r4, #BC_CURSOR]
	ldr	r7, [r4, #BC_HW]
	add	r6, r5, r1
	sub	r6, r6, #1
	cmp	r6, r7
	bls	7f                                      @ the reservation ends inside the current segment (hw is its last byte)
	add	r5, r7, #1                              @ else the next segment's start (the cursor may already sit there: commit does not enter it)
	sub	r6, r2, #1
	cmp	r5, #BC_HORIZON
	bhs	6f
	str	r5, [r4, #BC_CURSOR]
	add	r1, r5, r6
	str	r1, [r4, #BC_HW]                        @ its last byte
	ldr	r2, [r4, #BC_MASK]
	and	r1, r5, r2
	mov	r1, r1, lsr r3                          @ its slot
	bl	bc_turnover
	b	7f
6:	bl	bc_reset_hw                             @ past the 32-bit horizon: clear the tags, start over
	bl	bc_restart
7:	ldr	r1, [r4, #BC_CURSOR]
	ldr	r2, [r4, #BC_MASK]
	and	r1, r1, r2
	ldr	r0, [r4, #BC_RING]
	add	r0, r0, r1
	LEAVE
8:	mov	r0, #0
	LEAVE
	.size	bc_reserve, . - bc_reserve

@ ---------------------------------------------------------------------------------------------------------------------------------
@ r0 = bc, r1 = guest, r2 = used bytes -> r0 = the block's code address
@ The tag goes into the way of the set that holds this guest address already, else the oldest (dead ways are the oldest).
	.global	bc_commit
	.type	bc_commit, %function
bc_commit:
	ENTER
	ldr	r5, [r4, #BC_CURSOR]                    @ r5 = seq of this block
	add	r2, r2, #(BC_ALIGN - 1)             @ blocks start on BC_ALIGN boundaries (16 .. 64: the branch target buffer is indexed by fetch block, so layout moves timings 5-10%)
	bic	r2, r2, #(BC_ALIGN - 1)
	add	r3, r5, r2
	str	r3, [r4, #BC_CURSOR]                    @ committed
	ldr	r6, [r4, #BC_SET_MASK]
	ldr	r2, [r4, #BC_TAGS]
	and	r6, r6, r1, lsr #1
	add	r2, r2, r6, lsl #5                      @ r2 = the set
	ldr	r12, [r4, #BC_HW]
	mov	r7, #0                                  @ best key so far
	mov	r8, r2                                  @ best way so far
	.irp	w, 0, 8, 16, 24
	ldr	r3, [r2, #\w]                           @ guest
	ldr	r9, [r2, #\w + 4]                       @ seq
	sub	r9, r12, r9                             @ age
	cmp	r3, r1
	mvneq	r9, #0                                  @ the same guest address: always the victim
	cmp	r9, r7
	movhi	r7, r9
	addhi	r8, r2, #\w
	.endr
	str	r1, [r8]
	str	r5, [r8, #4]
	ldr	r0, [r4, #BC_RING]
	ldr	r3, [r4, #BC_MASK]
	and	r3, r5, r3
	add	r0, r0, r3                              @ the block's start
	LEAVE
	.size	bc_commit, . - bc_commit

@ ---------------------------------------------------------------------------------------------------------------------------------
@ r0 = bc, r1 = stub (the address of the patchable word), r2 = type (0 forward, 1 call, 2 backward), r3 = target (a live block's code address)
@ -> r0 = 1 linked, 0 the target segment's list is full (nothing changed)
	.global	bc_link
	.type	bc_link, %function
bc_link:
	ENTER
	ldr	r5, [r4, #BC_RING]
	ldr	r6, [r4, #BC_SEG_SHIFT]
	sub	r7, r3, r5
	mov	r7, r7, lsr r6                          @ the target's segment
	ldr	r8, [r4, #BC_LINKS]
	ldr	r12, [r4, #BC_LIST_SHIFT]
	add	r8, r8, r7, lsl r12                     @ its list: count, then the records
	ldr	r9, [r8]
	ldr	r12, [r4, #BC_LMAX_F]
	cmp	r9, r12
	movhs	r0, #0
	bhs	9f
	sub	r7, r1, r5                              @ the stub's ring offset
	mov	r10, r7, lsr r6                         @ its segment
	ldr	r11, [r4, #BC_EPOCHS]
	ldrb	r10, [r11, r10]
	and	r10, r10, #31
	mov	r7, r7, lsr #2
	and	r12, r2, #3
	orr	r7, r7, r12, lsl #24
	tst	r2, #4
	orrne	r7, r7, #0x04000000                     @ the stub has a site word (bit 2 of the type argument)
	orr	r7, r7, r10, lsl #27                    @ record: stub offset / 4, type, has-site, the stub's segment's epoch
	mov	r12, #1
3:	cmp	r12, r9                                 @ the record may exist already (a stub unlinked by the interrupt latch and linked again)
	bhi	4f
	ldr	r10, [r8, r12, lsl #2]
	cmp	r10, r7
	beq	6f
	add	r12, r12, #1
	b	3b
4:	add	r9, r9, #1
	str	r9, [r8]
	str	r7, [r8, r9, lsl #2]
6:
	sub	r12, r3, r1
	sub	r12, r12, #8
	mov	r12, r12, asr #2
	bic	r12, r12, #0xFF000000
	orr	r12, r12, #0xEA000000                   @ b (a linked call stub is a plain b too: the caller's `bl` already pushed the return address)
	str	r12, [r1]
	SYNC	r1
	tst	r2, #4
	beq	5f
	and	r2, r2, #3                              @ a stub with a site: the branch that goes to the stub (`b<cc> stub` or the `bl` of a call) is aimed at the target
	cmp	r2, #1
	moveq	r0, #4                                  @ (a call stub's patched word is its third: the site word follows it)
	movne	r0, #12                                 @ (otherwise it is the stub's fourth word)
	ldr	r0, [r1, r0]
	cmp	r0, #0
	beq	5f
	sub	r12, r3, r0
	sub	r12, r12, #8
	mov	r12, r12, asr #2
	bic	r12, r12, #0xFF000000
	ldr	r2, [r0]
	and	r2, r2, #0xFF000000
	orr	r12, r12, r2
	str	r12, [r0]
	SYNC	r0
5:	SYNC_END
	mov	r0, #1
9:	LEAVE
	.size	bc_link, . - bc_link

@ ---------------------------------------------------------------------------------------------------------------------------------
@ r0 = bc. Every block dies at once: hw moves a whole ring ahead and writing restarts in the segment that ends at it.
	.global	bc_flush
	.type	bc_flush, %function
bc_flush:
	ENTER
	ldr	r1, [r4, #BC_HW]
	ldr	r2, [r4, #BC_MASK]
	add	r1, r1, r2
	add	r1, r1, #1                              @ hw + ring_bytes
	cmp	r1, #BC_HORIZON
	blo	1f
	bl	bc_reset_hw
1:	bl	bc_restart
	LEAVE
	.size	bc_flush, . - bc_flush

@ ---------------------------------------------------------------------------------------------------------------------------------
@ r4 = bc: clear the tag array; r1 = the hw to restart at (2 * ring_bytes, one segment in)
bc_reset_hw:
	ldr	r1, [r4, #BC_TAGS]
	ldr	r2, [r4, #BC_SET_MASK]
	add	r2, r2, #1
	add	r2, r1, r2, lsl #5
	mov	r3, #0
1:	str	r3, [r1], #4
	cmp	r1, r2
	blo	1b
	ldr	r1, [r4, #BC_MASK]
	add	r1, r1, #1
	mov	r1, r1, lsl #1
	ldr	r3, [r4, #BC_SEG_SHIFT]
	mov	r2, #1
	mov	r2, r2, lsl r3
	add	r1, r1, r2
	sub	r1, r1, #1
	bx	lr

@ r4 = bc, r1 = the new hw (the last byte of a segment): writing restarts at that segment's start; every link list is emptied (everything is dead)
bc_restart:
	str	r1, [r4, #BC_HW]
	ldr	r3, [r4, #BC_SEG_SHIFT]
	mov	r2, #1
	mov	r2, r2, lsl r3
	sub	r2, r2, #1
	sub	r0, r1, r2
	str	r0, [r4, #BC_CURSOR]
	ldr	r12, [r4, #BC_MASK]
	and	r1, r0, r12
	mov	r1, r1, lsr r3                          @ the slot being entered
	mov	r2, r12, lsr r3
	add	r2, r2, #1                              @ number of segments
	ldr	r0, [r4, #BC_LINKS]
	mov	r3, #0
	ldr	r12, [r4, #BC_LIST_SHIFT]
	mov	r3, #1
	lsl	r12, r3, r12                    @ a list's size in bytes
	mov	r3, #0
1:	str	r3, [r0]
	add	r0, r0, r12
	subs	r2, r2, #1
	bne	1b
	@ falls into bc_turnover

@ r4 = bc, r1 = slot being entered. The segment's old blocks are dead from now on (the caller moved hw): put back every stub that still points into it,
@ empty its list, count the entry, call the hook. Clobbers r0-r3, r5-r12.
bc_turnover:
	mov	r11, lr
	ldr	r2, [r4, #BC_EPOCHS]
	ldrb	r3, [r2, r1]
	add	r3, r3, #1
	strb	r3, [r2, r1]                            @ (stubs inside this segment now fail the epoch test below)
	ldr	r5, [r4, #BC_LINKS]
	ldr	r12, [r4, #BC_LIST_SHIFT]
	add	r5, r5, r1, lsl r12
	ldr	r6, [r5]
	mov	r3, #0
	str	r3, [r5], #4
	cmp	r6, #0
	beq	8f
	ldr	r7, [r4, #BC_RING]
	ldr	r8, [r4, #BC_SEG_SHIFT]
	ldr	r9, [r4, #BC_H_EXIT]
	ldr	r10, [r4, #BC_H_CALL]
1:	ldr	r0, [r5], #4
	bic	r1, r0, #0xFF000000
	mov	r1, r1, lsl #2                          @ the stub's ring offset
	mov	r3, r1, lsr r8
	ldrb	r12, [r2, r3]                           @ the epoch of the stub's segment now ...
	eor	r12, r12, r0, lsr #27                   @ ... against the one when it was linked
	tst	r12, #31
	bne	7f                                      @ the stub's block is gone: nothing to put back (and nothing to write into)
	add	r1, r7, r1
	ubfx	r3, r0, #24, #2
	cmp	r3, #0
	bne	2f
	movw	r12, #0xE00F
	movt	r12, #0xE1A0                            @ mov lr, pc
	b	4f
2:	cmp	r3, #1
	moveq	r12, r10                                @ bl h_call
	movne	r12, r9                                 @ bl h_exit
	sub	r12, r12, r1
	sub	r12, r12, #8
	mov	r12, r12, asr #2
	bic	r12, r12, #0xFF000000
	orr	r12, r12, #0xEB000000
4:	str	r12, [r1]
	SYNC	r1
	tst	r0, #0x04000000
	beq	7f
	cmp	r3, #1                                  @ a stub with a site: put the site back on the stub
	moveq	r12, #4
	movne	r12, #12
	ldr	r12, [r1, r12]
	cmp	r12, #0
	beq	7f
	cmp	r3, #1
	subeq	r0, r1, #8                              @ the stub's first word
	movne	r0, r1
	sub	r0, r0, r12
	sub	r0, r0, #8
	mov	r0, r0, asr #2
	bic	r0, r0, #0xFF000000
	ldr	r3, [r12]
	and	r3, r3, #0xFF000000
	orr	r0, r0, r3
	str	r0, [r12]
	SYNC	r12
7:	subs	r6, r6, #1
	bne	1b
	SYNC_END
8:	ldr	r1, [r4, #BC_HOOK]
	cmp	r1, #0
	movne	r0, r4
	blxne	r1
	bx	r11
