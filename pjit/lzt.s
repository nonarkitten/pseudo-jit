@ lzt.s -- the table image decoder: LZ4 block decoder + NEON byte-plane untranspose (format LZT1, see tools/pack/lzt.py).
@
@ The packed tables are 16-byte records (the DT's slots) split into 16 byte planes (plane k = byte k of every record), LZ4-compressed as one block. The planes
@ compress 2.6x better than the records (the arena: 291 KB -> 113 KB with `lz4 -9`).  Boot code, never guest code: it uses the stack, NEON q0-q15 (d8-d15 saved) and
@ unaligned word accesses (SCTLR.A must be clear).  NEON/VFP must be enabled (CPACR cp10/cp11, FPEXC.EN).
@
@   uint32_t lz4_block(const uint8_t *src, uint32_t srclen, uint8_t *dst)           -> bytes written.  Standard LZ4 block format.  Copies in 8-byte chunks, so dst needs
@                                                                                      16 bytes of slack past the end and src 8 readable bytes past its end.  No bounds checks.
@   void     lzt_untranspose(const uint8_t *planes, uint8_t *dst, uint32_t records)  records a multiple of 16; planes and dst 16-byte aligned; writes 16*records bytes.
@   uint32_t lzt_unpack(const uint8_t *image, uint8_t *dst, uint8_t *scratch)         -> raw size, or 0 if the header is wrong.  dst: roundup(raw, 256) bytes + 16;
@                                                                                      scratch: roundup(raw, 256) + 16 bytes (the planes), both 16-byte aligned.
	.syntax	unified
	.arm
	.fpu	neon
	.text

	.global	lz4_block
	.type	lz4_block, %function
lz4_block:
	push	{r4-r8, lr}
	add	r1, r0, r1                      @ r1 = end of the compressed data
	mov	r3, r2                          @ r3 = start of the output
.Ltoken:
	ldrb	r4, [r0], #1
	lsr	r5, r4, #4                      @ literal length
	cmp	r5, #15
	bne	.Lcopylit
.Llitlen:
	ldrb	r6, [r0], #1
	add	r5, r5, r6
	cmp	r6, #255
	beq	.Llitlen
.Lcopylit:
	add	r7, r0, r5                      @ r7 = source after the literals
	add	r8, r2, r5                      @ r8 = destination after the literals
	cmp	r5, #0
	beq	.Llitdone
.Llitloop:
	ldr	r6, [r0], #4
	ldr	r12, [r0], #4
	str	r6, [r2], #4
	str	r12, [r2], #4
	cmp	r2, r8
	blo	.Llitloop
.Llitdone:
	mov	r0, r7
	mov	r2, r8
	cmp	r0, r1
	bhs	.Ldone                          @ the last sequence has literals only
	ldrb	r6, [r0], #1                    @ match offset (16 bits, little endian)
	ldrb	r12, [r0], #1
	orr	r6, r6, r12, lsl #8
	and	r5, r4, #15
	cmp	r5, #15
	bne	.Lmatch
.Lmatchlen:
	ldrb	r12, [r0], #1
	add	r5, r5, r12
	cmp	r12, #255
	beq	.Lmatchlen
.Lmatch:
	add	r5, r5, #4                      @ minimum match length
	sub	r12, r2, r6                     @ r12 = match source
	add	r8, r2, r5                      @ r8 = destination after the match
	cmp	r6, #8
	bhs	.Lwild
	cmp	r6, #1
	beq	.Lrun
.Lbytes:                                        @ overlapping copy, offset 2..7
	ldrb	r6, [r12], #1
	strb	r6, [r2], #1
	cmp	r2, r8
	blo	.Lbytes
	b	.Ltoken
.Lrun:                                          @ offset 1: a run of one byte
	ldrb	r6, [r12]
	orr	r6, r6, r6, lsl #8
	orr	r6, r6, r6, lsl #16
.Lrunloop:
	str	r6, [r2], #4
	str	r6, [r2], #4
	cmp	r2, r8
	blo	.Lrunloop
	mov	r2, r8
	b	.Ltoken
.Lwild:                                         @ offset >= 8: 8 bytes at a time never read what it is about to write
	ldr	r6, [r12], #4
	ldr	r7, [r12], #4
	str	r6, [r2], #4
	str	r7, [r2], #4
	cmp	r2, r8
	blo	.Lwild
	mov	r2, r8
	b	.Ltoken
.Ldone:
	sub	r0, r2, r3
	pop	{r4-r8, pc}
	.size	lz4_block, . - lz4_block

@ 16 planes of `records` bytes -> records of 16 bytes: 16 records per pass, a 16x16 byte transpose in q0-q15 (three vtrn stages and a swap of halves).
	.global	lzt_untranspose
	.type	lzt_untranspose, %function
lzt_untranspose:
	push	{r4, r5}
	vpush	{d8-d15}
	mov	r3, r2                          @ stride between planes = records
	lsrs	r4, r2, #4                      @ passes
	beq	.Lundone
.Lpass:
	mov	r5, r0
	vld1.8	{d0, d1}, [r5:128], r3
	vld1.8	{d2, d3}, [r5:128], r3
	vld1.8	{d4, d5}, [r5:128], r3
	vld1.8	{d6, d7}, [r5:128], r3
	vld1.8	{d8, d9}, [r5:128], r3
	vld1.8	{d10, d11}, [r5:128], r3
	vld1.8	{d12, d13}, [r5:128], r3
	vld1.8	{d14, d15}, [r5:128], r3
	vld1.8	{d16, d17}, [r5:128], r3
	vld1.8	{d18, d19}, [r5:128], r3
	vld1.8	{d20, d21}, [r5:128], r3
	vld1.8	{d22, d23}, [r5:128], r3
	vld1.8	{d24, d25}, [r5:128], r3
	vld1.8	{d26, d27}, [r5:128], r3
	vld1.8	{d28, d29}, [r5:128], r3
	vld1.8	{d30, d31}, [r5:128], r3
	vtrn.8	q0, q1
	vtrn.8	q2, q3
	vtrn.8	q4, q5
	vtrn.8	q6, q7
	vtrn.8	q8, q9
	vtrn.8	q10, q11
	vtrn.8	q12, q13
	vtrn.8	q14, q15
	vtrn.16	q0, q2
	vtrn.16	q1, q3
	vtrn.16	q4, q6
	vtrn.16	q5, q7
	vtrn.16	q8, q10
	vtrn.16	q9, q11
	vtrn.16	q12, q14
	vtrn.16	q13, q15
	vtrn.32	q0, q4
	vtrn.32	q1, q5
	vtrn.32	q2, q6
	vtrn.32	q3, q7
	vtrn.32	q8, q12
	vtrn.32	q9, q13
	vtrn.32	q10, q14
	vtrn.32	q11, q15
	vswp	d1, d16
	vswp	d3, d18
	vswp	d5, d20
	vswp	d7, d22
	vswp	d9, d24
	vswp	d11, d26
	vswp	d13, d28
	vswp	d15, d30
	vst1.8	{d0, d1}, [r1:128]!
	vst1.8	{d2, d3}, [r1:128]!
	vst1.8	{d4, d5}, [r1:128]!
	vst1.8	{d6, d7}, [r1:128]!
	vst1.8	{d8, d9}, [r1:128]!
	vst1.8	{d10, d11}, [r1:128]!
	vst1.8	{d12, d13}, [r1:128]!
	vst1.8	{d14, d15}, [r1:128]!
	vst1.8	{d16, d17}, [r1:128]!
	vst1.8	{d18, d19}, [r1:128]!
	vst1.8	{d20, d21}, [r1:128]!
	vst1.8	{d22, d23}, [r1:128]!
	vst1.8	{d24, d25}, [r1:128]!
	vst1.8	{d26, d27}, [r1:128]!
	vst1.8	{d28, d29}, [r1:128]!
	vst1.8	{d30, d31}, [r1:128]!
	add	r0, r0, #16
	subs	r4, r4, #1
	bne	.Lpass
.Lundone:
	vpop	{d8-d15}
	pop	{r4, r5}
	bx	lr
	.size	lzt_untranspose, . - lzt_untranspose

@ The same for 8-byte records (8 planes, a 8x16 transpose: three vtrn stages; the low half of q_r is record r, the high half record r + 8).
	.global	lzt_untranspose8
	.type	lzt_untranspose8, %function
lzt_untranspose8:
	push	{r4, r5}
	vpush	{d8-d15}
	mov	r3, r2
	lsrs	r4, r2, #4
	beq	2f
1:	mov	r5, r0
	vld1.8	{d0, d1}, [r5:128], r3
	vld1.8	{d2, d3}, [r5:128], r3
	vld1.8	{d4, d5}, [r5:128], r3
	vld1.8	{d6, d7}, [r5:128], r3
	vld1.8	{d8, d9}, [r5:128], r3
	vld1.8	{d10, d11}, [r5:128], r3
	vld1.8	{d12, d13}, [r5:128], r3
	vld1.8	{d14, d15}, [r5:128], r3
	vtrn.8	q0, q1
	vtrn.8	q2, q3
	vtrn.8	q4, q5
	vtrn.8	q6, q7
	vtrn.16	q0, q2
	vtrn.16	q1, q3
	vtrn.16	q4, q6
	vtrn.16	q5, q7
	vtrn.32	q0, q4
	vtrn.32	q1, q5
	vtrn.32	q2, q6
	vtrn.32	q3, q7
	vst1.8	{d0}, [r1:64]!
	vst1.8	{d2}, [r1:64]!
	vst1.8	{d4}, [r1:64]!
	vst1.8	{d6}, [r1:64]!
	vst1.8	{d8}, [r1:64]!
	vst1.8	{d10}, [r1:64]!
	vst1.8	{d12}, [r1:64]!
	vst1.8	{d14}, [r1:64]!
	vst1.8	{d1}, [r1:64]!
	vst1.8	{d3}, [r1:64]!
	vst1.8	{d5}, [r1:64]!
	vst1.8	{d7}, [r1:64]!
	vst1.8	{d9}, [r1:64]!
	vst1.8	{d11}, [r1:64]!
	vst1.8	{d13}, [r1:64]!
	vst1.8	{d15}, [r1:64]!
	add	r0, r0, #16
	subs	r4, r4, #1
	bne	1b
2:	vpop	{d8-d15}
	pop	{r4, r5}
	bx	lr
	.size	lzt_untranspose8, . - lzt_untranspose8

@ The same for 4-byte records (4 planes, vst4 interleaves).
	.global	lzt_untranspose4
	.type	lzt_untranspose4, %function
lzt_untranspose4:
	push	{r4, r5}
	vpush	{d8-d15}
	mov	r3, r2
	lsrs	r4, r2, #4
	beq	2f
1:	mov	r5, r0
	vld1.8	{d0, d1}, [r5:128], r3
	vld1.8	{d2, d3}, [r5:128], r3
	vld1.8	{d4, d5}, [r5:128], r3
	vld1.8	{d6, d7}, [r5:128], r3
	vst4.8	{d0, d2, d4, d6}, [r1:128]!
	vst4.8	{d1, d3, d5, d7}, [r1:128]!
	add	r0, r0, #16
	subs	r4, r4, #1
	bne	1b
2:	vpop	{d8-d15}
	pop	{r4, r5}
	bx	lr
	.size	lzt_untranspose4, . - lzt_untranspose4

@ image: 'LZT1', raw size, records (a multiple of 16), compressed size (little-endian words), then the LZ4 block of the planes.
	.global	lzt_unpack
	.type	lzt_unpack, %function
lzt_unpack:
	push	{r4-r8, lr}
	ldr	r3, [r0]
	movw	r4, #0x5A4C                     @ 'L','Z','T','1' read as a little-endian word
	movt	r4, #0x3154
	cmp	r3, r4
	bne	.Lbad
	ldr	r4, [r0, #4]                    @ raw size
	ldr	r5, [r0, #8]                    @ records
	ldr	r6, [r0, #12]                   @ compressed size
	mov	r7, r1                          @ dst
	mov	r8, r2                          @ the planes
	add	r0, r0, #16
	mov	r1, r6
	mov	r2, r8
	bl	lz4_block
	mov	r0, r8
	mov	r1, r7
	mov	r2, r5
	bl	lzt_untranspose
	mov	r0, r4
	pop	{r4-r8, pc}
.Lbad:
	mov	r0, #0
	pop	{r4-r8, pc}
	.size	lzt_unpack, . - lzt_unpack

@ Turns NEON/VFP on (CPACR cp10 / cp11 full access, FPEXC.EN); the boot code calls it once before lzt_untranspose.
	.global	lzt_neon_enable
	.type	lzt_neon_enable, %function
lzt_neon_enable:
	mrc	p15, 0, r0, c1, c0, 2
	orr	r0, r0, #(0xF << 20)
	mcr	p15, 0, r0, c1, c0, 2
	isb
	mov	r0, #0x40000000
	vmsr	fpexc, r0
	bx	lr
	.size	lzt_neon_enable, . - lzt_neon_enable
