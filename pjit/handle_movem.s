/*
 * Copyright (c) 2020-2023 Renee Cousins, the Buffee Project - http://www.buffee.ca
 *
 * Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (see LICENSE).
 */

	.text
	.code 32
	.syntax unified

@ MOVEM helpers, shared by every addressing mode the generator does not
@ open-code inline ((An)+ and -(An), which need their own register order).
@
@ Calling convention, uniform across all four (the extension word, the register
@ list, is loaded into r1 by the slot's prefix):
@
@   r1 = register-list mask, bit 0 = D0 ... bit 7 = D7,
@                           bit 8 = A0 ... bit 15 = A7
@   r2 = the 68K address to transfer to/from, post-incremented
@   r0 = scratch
@   r3 = the caller's flags, saved on entry and restored on exit: the
@        rotate below is the flag-setting form (its carry-out is what
@        each transfer tests) and MOVEM must not disturb the CCR
@   r5 = dispatch table base (cpu_t is just below it), r6-r13 = A0-A7 (see pjit.h)
@
@ Each step rotates the mask right by one (RORS -- the flag-setting
@ form, or the carry the transfers test is never updated) and acts on
@ carry, so the bits
@ are consumed D0 first, A7 last -- 68K's own MOVEM order for every mode
@ except -(An).
	.include "asm_offsets.inc"
	.equ D0_OFF, CPU_D0

	@ D0 is pinned in r4 (PJIT_PIN_D0): its slot in cpu_t is not current while guest code runs, so the D0 transfers use r4 itself
	.macro	DLD cc, i			@ r0 = D<i> (long)
	.ifdef	PJIT_PIN_D0
	.if	\i == 0
	mov\cc	r0, r4
	.exitm
	.endif
	.endif
	ldr\cc	r0, [r5, #(D0_OFF + \i * 4)]
	.endm
	.macro	DST cc, i			@ D<i> = r0 (long)
	.ifdef	PJIT_PIN_D0
	.if	\i == 0
	mov\cc	r4, r0
	.exitm
	.endif
	.endif
	str\cc	r0, [r5, #(D0_OFF + \i * 4)]
	.endm
	.macro	DLDH cc, i			@ r0 = the low half of D<i>, zero-extended
	.ifdef	PJIT_PIN_D0
	.if	\i == 0
	uxth\cc	r0, r4
	.exitm
	.endif
	.endif
	ldrh\cc	r0, [r5, #(D0_OFF + \i * 4 + 2)]
	.endm

    // void r2m_word(int _, uint16_t mask, uint16_t* addr)
	.global r2m_word
r2m_word:
	mrs 	r3, cpsr
	rors	r1, r1, #1
	DLDH cs, 0
	strhcs	r0, [r2], #2
	rors	r1, r1, #1
	DLDH cs, 1
	strhcs	r0, [r2], #2
	rors	r1, r1, #1
	DLDH cs, 2
	strhcs	r0, [r2], #2
	rors	r1, r1, #1
	DLDH cs, 3
	strhcs	r0, [r2], #2
	rors	r1, r1, #1
	DLDH cs, 4
	strhcs	r0, [r2], #2
	rors	r1, r1, #1
	DLDH cs, 5
	strhcs	r0, [r2], #2
	rors	r1, r1, #1
	DLDH cs, 6
	strhcs	r0, [r2], #2
	rors	r1, r1, #1
	DLDH cs, 7
	strhcs	r0, [r2], #2
	rors	r1, r1, #1
	strhcs	r6, [r2], #2
	rors	r1, r1, #1
	strhcs	r7, [r2], #2
	rors	r1, r1, #1
	strhcs	r8, [r2], #2
	rors	r1, r1, #1
	strhcs	r9, [r2], #2
	rors	r1, r1, #1
	strhcs	r10, [r2], #2
	rors	r1, r1, #1
	strhcs	r11, [r2], #2
	rors	r1, r1, #1
	strhcs	r12, [r2], #2
	rors	r1, r1, #1
	strhcs	r13, [r2], #2
	msr 	cpsr_f, r3
	bx   	lr

    // void r2m_long(int _, uint16_t mask, uint32_t* addr)
	.global r2m_long
r2m_long:
	mrs 	r3, cpsr
	rors	r1, r1, #1
	DLD cs, 0
	strcs 	r0, [r2], #4
	rors	r1, r1, #1
	DLD cs, 1
	strcs 	r0, [r2], #4
	rors	r1, r1, #1
	DLD cs, 2
	strcs 	r0, [r2], #4
	rors	r1, r1, #1
	DLD cs, 3
	strcs 	r0, [r2], #4
	rors	r1, r1, #1
	DLD cs, 4
	strcs 	r0, [r2], #4
	rors	r1, r1, #1
	DLD cs, 5
	strcs 	r0, [r2], #4
	rors	r1, r1, #1
	DLD cs, 6
	strcs 	r0, [r2], #4
	rors	r1, r1, #1
	DLD cs, 7
	strcs 	r0, [r2], #4
	rors	r1, r1, #1
	strcs 	r6, [r2], #4
	rors	r1, r1, #1
	strcs 	r7, [r2], #4
	rors	r1, r1, #1
	strcs 	r8, [r2], #4
	rors	r1, r1, #1
	strcs 	r9, [r2], #4
	rors	r1, r1, #1
	strcs 	r10, [r2], #4
	rors	r1, r1, #1
	strcs 	r11, [r2], #4
	rors	r1, r1, #1
	strcs 	r12, [r2], #4
	rors	r1, r1, #1
	strcs 	r13, [r2], #4
	msr 	cpsr_f, r3
	bx   	lr

    // void m2r_word(int _, uint16_t mask, uint16_t* addr)
    // Word transfers *to* registers are sign-extended to 32 bits by the
    // 68K, for address and data registers alike -- hence ldrsh and a full
    // 32-bit store back to cpu_state.
	.global m2r_word
m2r_word:
	mrs 	r3, cpsr
	rors	r1, r1, #1
	ldrshcs	r0, [r2], #2
	DST cs, 0
	rors	r1, r1, #1
	ldrshcs	r0, [r2], #2
	DST cs, 1
	rors	r1, r1, #1
	ldrshcs	r0, [r2], #2
	DST cs, 2
	rors	r1, r1, #1
	ldrshcs	r0, [r2], #2
	DST cs, 3
	rors	r1, r1, #1
	ldrshcs	r0, [r2], #2
	DST cs, 4
	rors	r1, r1, #1
	ldrshcs	r0, [r2], #2
	DST cs, 5
	rors	r1, r1, #1
	ldrshcs	r0, [r2], #2
	DST cs, 6
	rors	r1, r1, #1
	ldrshcs	r0, [r2], #2
	DST cs, 7
	rors	r1, r1, #1
	ldrshcs	r6, [r2], #2
	rors	r1, r1, #1
	ldrshcs	r7, [r2], #2
	rors	r1, r1, #1
	ldrshcs	r8, [r2], #2
	rors	r1, r1, #1
	ldrshcs	r9, [r2], #2
	rors	r1, r1, #1
	ldrshcs	r10, [r2], #2
	rors	r1, r1, #1
	ldrshcs	r11, [r2], #2
	rors	r1, r1, #1
	ldrshcs	r12, [r2], #2
	rors	r1, r1, #1
	ldrshcs	r13, [r2], #2
	msr 	cpsr_f, r3
	bx   	lr

    // void m2r_long(int _, uint16_t mask, uint32_t* addr)
	.global m2r_long
m2r_long:
	mrs 	r3, cpsr
	rors	r1, r1, #1
	ldrcs 	r0, [r2], #4
	DST cs, 0
	rors	r1, r1, #1
	ldrcs 	r0, [r2], #4
	DST cs, 1
	rors	r1, r1, #1
	ldrcs 	r0, [r2], #4
	DST cs, 2
	rors	r1, r1, #1
	ldrcs 	r0, [r2], #4
	DST cs, 3
	rors	r1, r1, #1
	ldrcs 	r0, [r2], #4
	DST cs, 4
	rors	r1, r1, #1
	ldrcs 	r0, [r2], #4
	DST cs, 5
	rors	r1, r1, #1
	ldrcs 	r0, [r2], #4
	DST cs, 6
	rors	r1, r1, #1
	ldrcs 	r0, [r2], #4
	DST cs, 7
	rors	r1, r1, #1
	ldrcs 	r6, [r2], #4
	rors	r1, r1, #1
	ldrcs 	r7, [r2], #4
	rors	r1, r1, #1
	ldrcs 	r8, [r2], #4
	rors	r1, r1, #1
	ldrcs 	r9, [r2], #4
	rors	r1, r1, #1
	ldrcs 	r10, [r2], #4
	rors	r1, r1, #1
	ldrcs 	r11, [r2], #4
	rors	r1, r1, #1
	ldrcs 	r12, [r2], #4
	rors	r1, r1, #1
	ldrcs 	r13, [r2], #4
	msr 	cpsr_f, r3
	bx   	lr
@ ---- MOVEM.L with A7 as the base: the stack-frame forms --------------------------------------
@ MOVEM.L regs,-(A7) and MOVEM.L (A7)+,regs are what compilers emit around a function body, with a mask
@ drawn from D2-D7/A2-A6. Those get a routine per mask, built by tools/gen/maketables.py (pjit_arena.movem)
@ out of LDM/STM runs: about 8 instructions where the generic code below takes about 46 whatever the mask.
@ Chosen here, at run time, from the mask in r1. Flag-free: the generic path is taken for anything else (a
@ mask with other registers in it, or an SP that is not word-aligned, which LDM/STM would fault on).
@ In: r1 = the mask as the instruction has it (-(An): bit 0 = A7 ... bit 15 = D0; (An)+: bit 0 = D0 ...
@ bit 15 = A7; possibly sign-extended), lr = the way out; tail-branched to, never `bl`'d.
	.macro	MOVEM_SP_DISPATCH half
	uxth	r1, r1
	.if \half == 0
	rbit	r2, r1
	lsr	r2, r2, #16			@ the mask in D0..A7 order
	.else
	mov	r2, r1
	.endif
	bic	r0, r2, #0x7C00			@ anything but D2-D7/A2-A6 in it?
	bic	r0, r0, #0xFC
	usat	r0, #1, r0			@ 0 = no, 1 = yes
	and	r3, r2, #0xFC
	lsr	r3, r3, #2
	and	r2, r2, #0x7C00
	lsr	r2, r2, #10
	orr	r3, r3, r2, lsl #6		@ the packed mask, 0-2047
	add	r2, r5, r3, lsl #7
	add	r2, r2, #(ARENA_MOVEM + \half * 0x40000)
	add	pc, pc, r0, lsl #2		@ pc reads as .+8: the entry below the nop
	nop
	bx	r2				@ the mask's own routine, which checks the stack's alignment itself and ends in `bx lr`
	.endm

	.global	movem_sp_save
movem_sp_save:
	MOVEM_SP_DISPATCH 0
	.global	movem_sp_save_generic
movem_sp_save_generic:
	mrs	r3, cpsr			@ RORS clobbers the CCR
	mov	r2, r13				@ A7's initial value is what is stored if A7 is in the list
	.irp	reg, r2, r12, r11, r10, r9, r8, r7, r6	@ A7 (its initial value, kept in r2), then A6 ... A0
	rors	r1, r1, #1
	strcs	\reg, [r13, #-4]!
	.endr
	.irp	i, 7, 6, 5, 4, 3, 2, 1, 0
	rors	r1, r1, #1
	DLD cs, \i
	strcs	r0, [r13, #-4]!
	.endr
	msr	cpsr_f, r3
	bx	lr

	.global	movem_sp_restore
movem_sp_restore:
	MOVEM_SP_DISPATCH 1
	.global	movem_sp_restore_generic
movem_sp_restore_generic:
	mrs	r3, cpsr
	.irp	i, 0, 1, 2, 3, 4, 5, 6, 7
	rors	r1, r1, #1
	ldrcs	r0, [r13], #4
	DST cs, \i
	.endr
	.irp	reg, r6, r7, r8, r9, r10, r11, r12
	rors	r1, r1, #1
	ldrcs	\reg, [r13], #4
	.endr
	rors	r1, r1, #1
	ldrcs	r2, [r13], #4			@ A7 itself: the loaded value is ignored, A7 keeps the postincremented address (the 68000's rule)
	msr	cpsr_f, r3
	bx	lr

	.end
