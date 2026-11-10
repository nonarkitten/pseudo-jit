/*
 * Copyright (c) 2020-2023 Renee Cousins, the Buffee Project - http://www.buffee.ca
 *
 * Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (see LICENSE).
 */

@ DIVU.W/DIVS.W helpers: 32/16 restoring division, reporting 68K's
@ overflow case. Register-only -- the Cortex-A8 has no hardware divide and
@ PJIT links no libgcc, and a helper called from a JIT slot cannot push
@ (A7 *is* the ARM SP; see docs/design/OVERVIEW.md).
@
@ In:  r0 = dividend, r1 = divisor -- zero-extended for DIVU, sign-extended
@      for DIVS, as the DIV sequences' uxth/sxth guarantee.
@ Out: r0 = (remainder << 16) | quotient, r1 = overflow flag.
@      On overflow r0 is the original dividend, so the caller can store
@      unconditionally and stay branch-free: 68K leaves the destination
@      untouched, and storing the same value back is a no-op.
@ Clobbers r0-r3 and r12. r12 is A6, which the DIV sequences spill to cpu->r12
@ across the call.
	.text
	.code 32
	.syntax unified
	.include "asm_offsets.inc"

@ 16 steps of restoring division, quotient accumulating in the low bits
@ through the carry chain: each ADCS folds in the previous step's quotient
@ bit while shifting the working remainder up. Ends with the remainder in
@ the top half and the quotient in the bottom, which is the packing the
@ caller wants anyway.
@ In: r0 = dividend, r1 = divisor (nonzero, <= 0xFFFF). Clobbers r1.
	.macro	DIV_CORE
	rsb	r1, r1, #0
	mov	r1, r1, lsl #15
	adds	r0, r1, r0
	rsbcc	r0, r1, r0
	.rept	15
	adcs	r0, r1, r0, lsl #1
	rsbcc	r0, r1, r0
	.endr
	adcs	r0, r0, r0
	.endm

@ Two ways out of a helper. RET_LR is a normal return (the C-callable
@ handle_DIVU/handle_DIVS, used by tests/qemu/smoke_div.c). RET_MEM is for the
@ *_tail entry points the emitted code tail-branches into: LR belongs to the
@ emitted sequence (it is the way back to the cache or the interpreter's
@ prefetch handler and must stay untouched, so there is no `bl` here at all),
@ and the code to resume at was parked in cpu->scratch by the caller.
	.macro	RET_LR
	bx	lr
	.endm
	.macro	RET_MEM
	ldr	pc, [r5, #CPU_SCRATCH]
	.endm

	.macro	DIVU_IMPL name, ret
	.global	\name
\name:
	@ A 16-bit quotient exists iff dividend >> 16 < divisor. Testing it
	@ this way needs no shift of the dividend, and a zero divisor falls
	@ out as overflow for free.
	cmp	r0, r1, lsl #16
	bhs	1f
	DIV_CORE
	mov	r1, #0
	\ret
1:	mov	r1, #1
	\ret
	.endm

	.macro	DIVS_IMPL name, ret
	.global	\name
\name:
	mov	r3, r0			@ original dividend, for the overflow exit
	mov	r2, #0			@ bit0: quotient negative, bit1: remainder
	cmp	r0, #0
	rsblt	r0, r0, #0		@ |dividend| -- 0x80000000 is left as-is,
	orrlt	r2, r2, #3		@ which is its correct unsigned magnitude
	cmp	r1, #0
	rsblt	r1, r1, #0		@ |divisor|
	eorlt	r2, r2, #1
	@ Same 16-bit test as above, on magnitudes. Catches divide-by-zero
	@ and 0x80000000 / -1 without special-casing either.
	cmp	r0, r1, lsl #16
	bhs	3f
	DIV_CORE
	uxth	r12, r0			@ |quotient|
	tst	r2, #1
	bne	2f
	cmp	r12, #0x8000		@ positive: 0x8000 and up will not fit
	bhs	3f
	b	4f
2:	cmp	r12, #0x8000		@ negative: 0x8000 is exactly -32768
	bhi	3f
	rsb	r12, r12, #0
4:	mov	r0, r0, lsr #16		@ |remainder|
	tst	r2, #2
	rsbne	r0, r0, #0		@ remainder takes the dividend's sign
	uxth	r12, r12
	orr	r0, r12, r0, lsl #16
	mov	r1, #0
	\ret
3:	mov	r0, r3
	mov	r1, #1
	\ret
	.endm

	DIVU_IMPL handle_DIVU,      RET_LR
	DIVS_IMPL handle_DIVS,      RET_LR

@ The *_tail entry points (the ones emitted code branches to) also return the new flags, in r2, ready for
@ `msr cpsr_f, r2`: N/Z from the 16-bit quotient and V = C = 0 on success; on overflow V set, C clear and N/Z
@ as they were going in (cpu->div_cpsr: DIV leaves them undefined, this keeps them). Putting this here, where
@ the quotient is already in a register, takes about a dozen instructions out of every emitted DIV.
	.macro	DIV_FLAGS_OK
	lsls	r2, r0, #16			@ N/Z from the quotient alone
	mrs	r2, cpsr
	bic	r2, r2, #0x30000000		@ V = C = 0
	.endm
	.macro	DIV_FLAGS_OVERFLOW
	ldr	r2, [r5, #CPU_DIV_CPSR]
	and	r2, r2, #0xC0000000		@ the incoming N and Z
	orr	r2, r2, #0x10000000		@ V set
	.endm

	.global	handle_DIVU_tail
handle_DIVU_tail:
	cmp	r0, r1, lsl #16
	bhs	1f
	DIV_CORE
	DIV_FLAGS_OK
	RET_MEM
1:	DIV_FLAGS_OVERFLOW
	RET_MEM

	.global	handle_DIVS_tail
handle_DIVS_tail:
	mov	r3, r0
	mov	r2, #0
	cmp	r0, #0
	rsblt	r0, r0, #0
	orrlt	r2, r2, #3
	cmp	r1, #0
	rsblt	r1, r1, #0
	eorlt	r2, r2, #1
	cmp	r0, r1, lsl #16
	bhs	3f
	DIV_CORE
	uxth	r12, r0
	tst	r2, #1
	bne	2f
	cmp	r12, #0x8000
	bhs	3f
	b	4f
2:	cmp	r12, #0x8000
	bhi	3f
	rsb	r12, r12, #0
4:	mov	r0, r0, lsr #16
	tst	r2, #2
	rsbne	r0, r0, #0
	uxth	r12, r12
	orr	r0, r12, r0, lsl #16
	DIV_FLAGS_OK
	RET_MEM
3:	mov	r0, r3				@ overflow: the destination keeps the dividend
	DIV_FLAGS_OVERFLOW
	RET_MEM

	.end
