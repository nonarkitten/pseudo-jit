@ pjit/blksched.s -- the block translator's instruction scheduler.
@
@ blk_sched(r0 = start, r1 = end, r2 = workspace (S_SIZE bytes, word aligned), r3 = the cpu state's D0 offset from r5) -> r0 = 1 if the words in [start, end) were reordered
@ and their r0-r3 renamed in place, 0 if they were left alone (too short / too long, an encoding it does not know, pressure above four registers). Preserves r4-r11.
@
@ A port of tools/study/blocksched.py + tools/gen/regalloc.py (what they predicted: -15% on the hot segments; the board measured -7.3% weighted, -13% on a 540-word mix):
@   A  decode every word (tools/gen/schedtab.py is the bit-exact mirror: register fields, flags, load / store class, latency) and rename: r0-r3 are virtual, every
@      definition is a new value; the other registers, the flags N Z C V G and the memory classes (D slot k, other cpu state, guest memory) are resources
@   B  dependences: for each earlier word j, read-after-write (the producer's latency), write-after-write (1), write-after-read (0), as bit masks per node
@   C  priority: the longest latency-weighted path to the end
@   D  list scheduling for a dual-issue in-order core: per cycle up to two ready words, one load / store, the highest priority first
@   E  linear-scan allocation of r0-r3 to the values in the new order (a value keeps its original register when it can), then the words are rewritten
@ Guest-memory loads stay in order with every other guest access (device registers). r4 and up are never renamed. A word that reads or writes pc is refused.
@ The workspace is plain data; no stack beyond 32 bytes + the pushes; the code is position independent and runs in either endianness's data view (no literal pool).

	.syntax unified
	.arm
	.text
	.include "blksched_tab.inc"
	.global	blk_sched

	.equ	S_NODE,   0x0000        @ 64 x 32 bytes: RA RB RO WA WB WO LAT UNIT  (R / W: value bits 0-31, 32-63, other: bits 0-11 r4-r15, 12-16 flags N Z C V G, 17-26 memory)
	.equ	S_WORD,   0x0800        @ 64 words
	.equ	S_INFO,   0x0900
	.equ	S_VIDS,   0x0A00        @ a byte per field: the value id (0xFF none)
	.equ	S_PRIO,   0x0B00
	.equ	S_READY,  0x0C00
	.equ	S_NPRED,  0x0D00        @ bytes from here on
	.equ	S_ISSUED, 0x0D40
	.equ	S_ORDER,  0x0D80
	.equ	S_VORIG,  0x0DC0
	.equ	S_VFIRST, 0x0E00
	.equ	S_VLAST,  0x0E40
	.equ	S_VASSIGN,0x0E80
	.equ	S_CUR,    0x0EC0        @ the value id currently in r0-r3 (0xFF none)
	.equ	S_NEWV,   0x0ED0
	.equ	S_OUT,    0x0F00        @ 64 words
	.equ	S_DEP,    0x1000        @ dep[i][j] (j < i), 64 bytes per i: 0 none, else edge latency + 1
	.equ	S_SIZE,   0x2000

	.macro	CONSTI reg, val
	movw	\reg, #:lower16:(\val)
	movt	\reg, #:upper16:(\val)
	.endm
	.global	blk_sched_size
	.equ	blk_sched_size, S_SIZE

@ ---- helpers --------------------------------------------------------------------------------------------------------------------------------------------------
@ r0 = node, r1 = word offset in it (0 reads, 12 writes), r2 = value id: set its bit. Clobbers r0, r1, r3, r12; keeps r2.
s_setbit:
	add	r0, r0, r1
	cmp	r2, #32
	movlo	r1, r2
	subhs	r1, r2, #32
	addhs	r0, r0, #4
	mov	r3, #1
	ldr	r12, [r0]
	orr	r12, r12, r3, lsl r1
	str	r12, [r0]
	bx	lr

@ r0 = a word -> r0 = its info word (tools/gen/schedtab.py), 0 = an encoding this scheduler does not know. Clobbers r1-r3, r12.
s_decode:
	lsr	r12, r0, #28
	cmp	r12, #15
	beq	sd_fail
	adr	r3, s_pat
sd_t:	ldmia	r3!, {r1, r2, r12}
	cmp	r1, #0
	beq	sd_code
	and	r1, r0, r1
	cmp	r1, r2
	moveq	r0, r12
	bxeq	lr
	b	sd_t
sd_fail:
	mov	r0, #0
	bx	lr
sd_code:
	ubfx	r1, r0, #25, #3                 @ k
	cmp	r1, #0
	bne	sd_k1
	movw	r2, #0x00F0                     @ k = 0: multiply?  (w & 0x0FC000F0) == 0x90
	movt	r2, #0x0FC0
	and	r2, r0, r2
	cmp	r2, #0x90
	beq	sd_mul
	and	r2, r0, #0x90
	cmp	r2, #0x90
	bne	sd_k0b
	tst	r0, #0x60                       @ ldrh / ldrsb / ldrsh / strh (an extra load / store); anything else with these bits is not ours
	beq	sd_fail
	ubfx	r1, r0, #22, #1
	eor	r12, r1, #1                     @ a third field (Rm) unless the offset is an immediate
	b	sd_ls
sd_k0b:	ubfx	r2, r0, #21, #4                 @ the sbc idiom: sbc rX, rX, rX
	cmp	r2, #6
	bne	sd_dp0
	movw	r2, #0x0FF0
	tst	r0, r2
	bne	sd_dp0
	ubfx	r2, r0, #16, #4
	ubfx	r3, r0, #12, #4
	cmp	r2, r3
	bne	sd_dp0
	and	r3, r0, #15
	cmp	r2, r3
	bne	sd_dp0
	CONSTI	r0, I_IDIOM
	bx	lr
sd_dp0:	movw	r2, #0x0000                     @ (w & 0x0F900000) == 0x01000000: the miscellaneous group (mrs / msr / clz ... that the table did not take)
	movt	r2, #0x0F90
	and	r2, r0, r2
	cmp	r2, #0x01000000
	beq	sd_fail
	b	sd_dp
sd_k1:	cmp	r1, #1
	bne	sd_k23
	movw	r2, #0x0000                     @ k = 1: (w & 0x0F900000) == 0x03000000 without S: not ours (movw / movt were in the table)
	movt	r2, #0x0F90
	and	r2, r0, r2
	cmp	r2, #0x03000000
	bne	sd_dp
	tst	r0, #0x00100000
	beq	sd_fail
	b	sd_dp
sd_k23:	cmp	r1, #3
	bhi	sd_fail
	mov	r12, #0
	bne	sd_ls                           @ k = 2: a load / store with an immediate offset
	tst	r0, #0x10                       @ k = 3: a register offset (shifted by an immediate); a register-controlled shift is not a load / store
	bne	sd_fail
	mov	r12, #1
sd_ls:	@ r0 = the word, r12 = a third field (Rm)
	ubfx	r1, r0, #20, #1                 @ load
	ubfx	r2, r0, #21, #1                 @ W
	ubfx	r3, r0, #24, #1                 @ P
	eor	r3, r3, #1
	orr	r2, r2, r3                      @ write-back
	tst	r0, #0x00100000
	moveq	r1, #1
	movne	r1, #2
	orr	r1, r1, #8                      @ f0
	add	r2, r2, r2
	add	r2, r2, #1                      @ Rn: u (1) or ud (3), shift 16
	orr	r2, r2, #12
	orr	r1, r1, r2, lsl #4              @ f1
	cmp	r12, #0
	orrne	r1, r1, #0x100                  @ f2: (0, u)
	tst	r0, #0x00100000
	CONSTI	r0, I_STBASE
	CONSTI	r2, I_LDBASE
	movne	r0, r2
	orr	r0, r0, r1
	bx	lr
sd_mul:	tst	r0, #0x00200000
	CONSTI	r0, I_MUL3
	CONSTI	r1, I_MUL4
	movne	r0, r1
	bx	lr
sd_dp:	ubfx	r1, r0, #21, #4                 @ opc
	mov	r2, #0
	mov	r3, #0
	cmp	r1, #13
	cmpne	r1, #15
	beq	1f
	mov	r12, #13                        @ Rn: (16, u)
	orr	r2, r2, r12, lsl r3
	add	r3, r3, #4
1:	sub	r12, r1, #8
	cmp	r12, #3
	bls	2f
	mov	r12, #10                        @ Rd: (12, d)
	orr	r2, r2, r12, lsl r3
	add	r3, r3, #4
2:	tst	r0, #0x02000000                 @ an immediate operand 2: no register fields
	bne	3f
	mov	r12, #1                         @ Rm: (0, u)
	orr	r2, r2, r12, lsl r3
	add	r3, r3, #4
	tst	r0, #0x10
	beq	3f
	mov	r12, #5                         @ Rs: (8, u)
	orr	r2, r2, r12, lsl r3
3:	CONSTI	r0, I_DPBASE
	orr	r0, r0, r2
	bx	lr

@ r0 = a word, r1 = its info -> r2 = the flags it reads, r3 = the flags it writes (N Z C V G = 1 2 4 8 16); r12 = 1 if it touches flags in a way we do not model.
@ A conditional word reads what it writes; a maybe-written flag (the shifter's carry) is read too. Clobbers r0, r1.
s_flags:
	lsr	r12, r0, #28
	adr	r2, s_condtab
	ldrb	r2, [r2, r12]
	mov	r3, #0
	ubfx	r1, r1, #16, #3
	add	pc, pc, r1, lsl #2
	nop
	b	sf_none
	b	sf_dp
	b	sf_mul
	b	sf_ge
	b	sf_sel
	b	sf_mrs
	b	sf_msr
	b	sf_fail
sf_dp:	ubfx	r1, r0, #21, #4                 @ opc
	sub	r12, r1, #5
	cmp	r12, #2
	orrls	r2, r2, #4                      @ adc sbc rsc read C
	tst	r0, #0x0E000000
	bne	1f
	and	r12, r0, #0xF0                  @ rrx: ror #0 of an immediate shift
	cmp	r12, #0x60
	bne	1f
	ubfx	r12, r0, #7, #5
	cmp	r12, #0
	orreq	r2, r2, #4
1:	tst	r0, #0x00100000
	beq	sf_cond
	movw	r12, #0x0CFC                    @ the arithmetic opcodes write all four; the logical ones N, Z and (maybe) C
	lsr	r12, r12, r1
	tst	r12, #1
	movne	r3, #15
	bne	sf_cond
	mov	r3, #7
	orr	r2, r2, #4
	b	sf_cond
sf_mul:	tst	r0, #0x00100000
	beq	sf_cond
	mov	r3, #7
	orr	r2, r2, #4
	b	sf_cond
sf_ge:	mov	r3, #16
	b	sf_cond
sf_sel:	orr	r2, r2, #16
	b	sf_cond
sf_mrs:	mov	r2, #31
	b	sf_cond
sf_msr:	ubfx	r1, r0, #16, #4
	tst	r0, #0x00400000
	bne	sf_fail
	tst	r1, #3
	bne	sf_fail
	tst	r1, #8
	orrne	r3, r3, #15
	tst	r1, #4
	orrne	r3, r3, #16
sf_cond:
	lsr	r12, r0, #28
	cmp	r12, #14
	orrlo	r2, r2, r3
sf_none:
	mov	r12, #0
	bx	lr
sf_fail:
	mov	r12, #1
	bx	lr

@ r0 = a load / store word, r1 = its info, r2 = the D0 offset -> r2 = resource bits it reads, r3 = resource bits it writes (0-7 D slot k, 8 other cpu state, 9 guest memory)
s_mem:
	push	{r4}
	ubfx	r4, r0, #16, #4                 @ base register
	mov	r3, #0x200                      @ guest memory
	cmp	r4, #5
	bne	sm_done
	@ r5-based: a D slot, other cpu state, or (a register offset / post-index) anything of it
	ubfx	r12, r0, #25, #3
	cmp	r12, #2
	beq	sm_imm12
	cmp	r12, #0
	bne	sm_any
	tst	r0, #0x00400000                 @ an extra load / store with a register offset
	beq	sm_any
	and	r12, r0, #15
	and	r4, r0, #0xF00
	orr	r12, r12, r4, lsr #4            @ the split 8-bit immediate
	b	sm_off
sm_imm12:
	ubfx	r12, r0, #0, #12
sm_off:	tst	r0, #0x01000000                 @ P
	beq	sm_any
	tst	r0, #0x00200000                 @ W
	bne	sm_any
	tst	r0, #0x00800000                 @ U
	rsbeq	r12, r12, #0
	sub	r12, r12, r2                    @ the offset from D0
	cmp	r12, #32
	movhs	r3, #0x100                      @ not a D register slot: other cpu state
	lsrlo	r12, r12, #2
	movlo	r3, #1
	lsllo	r3, r3, r12
	b	sm_done
sm_any:	movw	r3, #0x1FF
sm_done:
	tst	r1, #0x00200000                 @ a load
	beq	1f
	mov	r2, r3
	cmp	r3, #0x200
	moveq	r3, r2                          @ a guest load is ordered like a store against everything guest, too
	movne	r3, #0
	b	2f
1:	mov	r2, #0
2:	pop	{r4}
	bx	lr

@ ---- the scheduler ----------------------------------------------------------------------------------------------------------------------------------------------
@ Field helpers (macros: j is the field number 0..3). Given r8 = info, r9 = word: leave the register number in r1 (r0 = the field's kind).
	.macro	FIELDREG j, skip
	ubfx	r0, r8, #(4*\j), #2             @ kind
	cmp	r0, #0
	beq	\skip
	ubfx	r1, r8, #(4*\j+2), #2           @ shift index -> shift
	cmp	r1, #0
	lslne	r1, r1, #2
	addne	r1, r1, #4
	lsr	r1, r9, r1
	and	r1, r1, #15
	.endm

	.macro	SETVID j                        @ r6 (the node's value ids): byte j = r2
	bic	r6, r6, #(0xFF << (8*\j))
	orr	r6, r6, r2, lsl #(8*\j)
	.endm

	.macro	SWEEP1 j                        @ reads: tracked registers name their current value; the others are resources
	FIELDREG \j, 9f
	tst	r0, #1
	beq	9f
	cmp	r1, #15
	beq	s_no
	cmp	r1, #4
	blo	2f
	sub	r1, r1, #4                      @ r4-r15: a bit of RO
	mov	r2, #1
	lsl	r2, r2, r1
	ldr	r3, [r7, #8]
	orr	r3, r3, r2
	str	r3, [r7, #8]
	b	9f
2:	add	r3, r10, #S_CUR
	ldrb	r2, [r3, r1]
	cmp	r2, #0xFF
	beq	s_no
	SETVID	\j
	mov	r0, r7
	mov	r1, #0
	bl	s_setbit
9:
	.endm

	.macro	SWEEP2 j                        @ writes: a new value (or the same one for a conditional / read-modify-write field)
	FIELDREG \j, 9f
	tst	r0, #2
	beq	9f
	cmp	r1, #15
	beq	s_no
	cmp	r1, #4
	blo	2f
	sub	r1, r1, #4
	mov	r2, #1
	lsl	r2, r2, r1
	ldr	r3, [r7, #20]
	orr	r3, r3, r2
	str	r3, [r7, #20]
	b	9f
2:	orr	r12, r0, r5                     @ in place when conditional or also read
	tst	r12, #1
	beq	4f
	add	r3, r10, #S_CUR
	ldrb	r2, [r3, r1]
	cmp	r2, #0xFF
	beq	s_no
	SETVID	\j
	mov	r0, r7
	mov	r1, #12
	bl	s_setbit
	mov	r0, r7
	mov	r1, #0
	bl	s_setbit
	b	9f
4:	add	r3, r10, #S_NEWV
	ldrb	r2, [r3, r1]
	cmp	r2, #0xFF
	bne	5f
	ldr	r2, [sp, #12]                   @ a new value
	cmp	r2, #64
	bhs	s_no
	add	r12, r2, #1
	str	r12, [sp, #12]
	strb	r2, [r3, r1]
	add	r12, r10, #S_VORIG
	strb	r1, [r12, r2]
5:	SETVID	\j
	mov	r0, r7
	mov	r1, #12
	bl	s_setbit
9:
	.endm

blk_sched:
	push	{r4-r11, lr}
	sub	sp, sp, #32
	mov	r10, r2
	sub	r1, r1, r0
	lsr	r1, r1, #2                      @ n
	cmp	r1, #4
	blo	s_no
	cmp	r1, #64
	bhi	s_no
	str	r0, [sp]
	str	r1, [sp, #4]
	str	r3, [sp, #8]
	mov	r0, #0
	str	r0, [sp, #12]                   @ values so far
	mvn	r0, #0
	str	r0, [r10, #S_CUR]
	mov	r11, r1
	mov	r4, #0

@ ---- A: decode and rename ----
s_a_loop:
	cmp	r4, r11
	bhs	s_a_done
	ldr	r0, [sp]
	ldr	r9, [r0, r4, lsl #2]
	add	r0, r10, r4, lsl #2
	str	r9, [r0, #S_WORD]
	mov	r0, r9
	bl	s_decode
	cmp	r0, #0
	beq	s_no
	mov	r8, r0
	add	r0, r10, r4, lsl #2
	str	r8, [r0, #S_INFO]
	add	r7, r10, r4, lsl #5             @ the node
	mov	r0, #0
	mov	r1, #0
	mov	r2, #0
	mov	r3, #0
	stmia	r7, {r0-r3}
	add	r12, r7, #16
	stmia	r12, {r0-r3}
	mvn	r6, #0                          @ the value ids of the fields
	mvn	r0, #0
	str	r0, [r10, #S_NEWV]
	lsr	r5, r9, #28
	cmp	r5, #14
	movlo	r5, #1                          @ r5 = conditional
	movhs	r5, #0
	SWEEP1	0
	SWEEP1	1
	SWEEP1	2
	SWEEP1	3
	SWEEP2	0
	SWEEP2	1
	SWEEP2	2
	SWEEP2	3
	add	r3, r10, #S_NEWV                @ the new values become the current ones
	add	r12, r10, #S_CUR
	mov	r0, #0
1:	ldrb	r2, [r3, r0]
	cmp	r2, #0xFF
	strbne	r2, [r12, r0]
	add	r0, r0, #1
	cmp	r0, #4
	blo	1b
	add	r0, r10, r4, lsl #2
	str	r6, [r0, #S_VIDS]
	mov	r0, r9                          @ flags
	mov	r1, r8
	bl	s_flags
	cmp	r12, #0
	bne	s_no
	ldr	r0, [r7, #8]
	orr	r0, r0, r2, lsl #12
	str	r0, [r7, #8]
	ldr	r0, [r7, #20]
	orr	r0, r0, r3, lsl #12
	str	r0, [r7, #20]
	ubfx	r0, r8, #19, #2                 @ unit
	str	r0, [r7, #28]
	cmp	r0, #1
	bne	1f
	mov	r0, r9                          @ memory
	mov	r1, r8
	ldr	r2, [sp, #8]
	bl	s_mem
	ldr	r0, [r7, #8]
	orr	r0, r0, r2, lsl #17
	str	r0, [r7, #8]
	ldr	r0, [r7, #20]
	orr	r0, r0, r3, lsl #17
	str	r0, [r7, #20]
1:	ubfx	r0, r8, #23, #3                 @ latency
	str	r0, [r7, #24]
	add	r4, r4, #1
	b	s_a_loop
s_a_done:

@ ---- B: dependences ----
	mov	r0, #0
	str	r0, [sp, #16]                   @ i
s_b_i:
	ldr	r0, [sp, #16]
	ldr	r1, [sp, #4]
	cmp	r0, r1
	bhs	s_b_done
	add	r2, r10, r0, lsl #5
	ldmia	r2, {r4-r9}                     @ RA RB RO WA WB WO of i
	add	r1, r10, r0, lsl #6
	add	r1, r1, #S_DEP
	str	r1, [sp, #20]                   @ its row
	mov	r3, #0
	str	r3, [sp, #24]                   @ predecessors
	mov	r11, #0
	cmp	r0, #0
	beq	s_b_end
s_b_j:
	add	r0, r10, r11, lsl #5
	ldmia	r0, {r0-r3, r12, lr}
	and	r0, r0, r7                      @ write-after-read
	and	r1, r1, r8
	and	r2, r2, r9
	orr	r0, r0, r1
	orr	r0, r0, r2
	and	r1, r3, r4                      @ read-after-write
	and	r2, r12, r5
	orr	r1, r1, r2
	and	r2, lr, r6
	orr	r1, r1, r2
	and	r3, r3, r7                      @ write-after-write
	and	r12, r12, r8
	and	lr, lr, r9
	orr	r3, r3, r12
	orr	r3, r3, lr
	orr	r2, r0, r1
	orrs	r2, r2, r3
	beq	1f
	mov	r2, #1                          @ the edge: latency + 1
	cmp	r3, #0
	movne	r2, #2
	cmp	r1, #0
	beq	2f
	add	r12, r10, r11, lsl #5
	ldr	r12, [r12, #24]
	add	r12, r12, #1
	cmp	r12, r2
	movhi	r2, r12
2:	ldr	r12, [sp, #24]
	add	r12, r12, #1
	str	r12, [sp, #24]
1:	ldr	r12, [sp, #20]
	strb	r2, [r12, r11]
	ldr	r0, [sp, #16]
	add	r11, r11, #1
	cmp	r11, r0
	blo	s_b_j
s_b_end:
	ldr	r0, [sp, #16]
	ldr	r1, [sp, #24]
	add	r2, r10, #S_NPRED
	strb	r1, [r2, r0]
	add	r0, r0, #1
	str	r0, [sp, #16]
	b	s_b_i
s_b_done:

@ ---- C: priorities ----
	ldr	r5, [sp, #4]
	sub	r4, r5, #1
c_k:	add	r0, r10, r4, lsl #5
	ldr	r6, [r0, #24]
	mov	r7, r6
	add	r8, r4, #1
c_s:	cmp	r8, r5
	bhs	c_sd
	add	r0, r10, r8, lsl #6
	add	r0, r0, #S_DEP
	ldrb	r1, [r0, r4]
	cmp	r1, #0
	beq	c_sn
	add	r0, r10, r8, lsl #2
	ldr	r2, [r0, #S_PRIO]
	add	r2, r2, r6
	cmp	r2, r7
	movhi	r7, r2
c_sn:	add	r8, r8, #1
	b	c_s
c_sd:	add	r0, r10, r4, lsl #2
	str	r7, [r0, #S_PRIO]
	subs	r4, r4, #1
	bpl	c_k

@ ---- D: list scheduling ----
	mov	r0, #0                          @ issued / ready: zero
	add	r1, r10, #S_ISSUED
	add	r2, r10, #S_READY
	mov	r3, #0
1:	strb	r0, [r1, r3]
	str	r0, [r2, r3, lsl #2]
	add	r3, r3, #1
	cmp	r3, r5
	blo	1b
	mov	r4, #0                          @ cycle
	mov	r6, #0                          @ words placed
d_cycle:
	cmp	r6, r5
	bhs	d_done
	add	r0, r5, r5, lsl #2              @ a guard against a stuck schedule: 5 cycles per word
	cmp	r4, r0
	bhi	s_no
	mov	r7, #0                          @ words in this cycle
	mov	r8, #0                          @ a load / store issued in it
d_pick:	cmp	r7, #2
	bhs	d_end
	mvn	r9, #0                          @ best
	mov	r11, #0
	mov	r3, #0
d_scan:	cmp	r3, r5
	bhs	d_chosen
	add	r0, r10, #S_ISSUED
	ldrb	r1, [r0, r3]
	cmp	r1, #0
	bne	d_next
	add	r0, r10, #S_NPRED
	ldrb	r1, [r0, r3]
	cmp	r1, #0
	bne	d_next
	add	r0, r10, #S_READY
	ldr	r1, [r0, r3, lsl #2]
	cmp	r1, r4
	bhi	d_next
	add	r0, r10, r3, lsl #5
	ldr	r1, [r0, #28]
	cmp	r1, #1
	bne	1f
	cmp	r8, #0
	bne	d_next
1:	add	r0, r10, r3, lsl #2
	ldr	r1, [r0, #S_PRIO]
	cmn	r9, #1
	beq	2f
	cmp	r1, r11
	bls	d_next
2:	mov	r9, r3
	mov	r11, r1
d_next:	add	r3, r3, #1
	b	d_scan
d_chosen:
	cmn	r9, #1
	beq	d_end
	add	r0, r10, #S_ISSUED
	mov	r1, #1
	strb	r1, [r0, r9]
	add	r0, r10, #S_ORDER
	strb	r9, [r0, r6]
	add	r6, r6, #1
	add	r7, r7, #1
	add	r0, r10, r9, lsl #5
	ldr	r1, [r0, #28]
	cmp	r1, #1
	moveq	r8, #1
	add	r3, r9, #1
d_succ:	cmp	r3, r5
	bhs	d_pick
	add	r0, r10, r3, lsl #6
	add	r0, r0, #S_DEP
	ldrb	r1, [r0, r9]
	cmp	r1, #0
	beq	d_sn
	add	r0, r10, #S_NPRED
	ldrb	r2, [r0, r3]
	sub	r2, r2, #1
	strb	r2, [r0, r3]
	sub	r1, r1, #1
	add	r1, r1, r4
	add	r0, r10, #S_READY
	ldr	r2, [r0, r3, lsl #2]
	cmp	r1, r2
	strhi	r1, [r0, r3, lsl #2]
d_sn:	add	r3, r3, #1
	b	d_succ
d_end:	add	r4, r4, #1
	b	d_cycle
d_done:

@ ---- E: registers for the values in the new order ----
	ldr	r0, [sp, #12]                   @ nv
	add	r1, r10, #S_VFIRST
	add	r2, r10, #S_VASSIGN
	mov	r3, #0xFF
	mov	r4, #0
1:	cmp	r4, r0
	bhs	2f
	strb	r3, [r1, r4]
	strb	r3, [r2, r4]
	add	r4, r4, #1
	b	1b
2:	mov	r4, #0                          @ position
e_p:	cmp	r4, r5
	bhs	e_pd
	add	r0, r10, #S_ORDER
	ldrb	r0, [r0, r4]                    @ the node
	add	r0, r10, r0, lsl #2
	ldr	r6, [r0, #S_VIDS]
	mov	r7, #0
1:	lsr	r0, r6, r7
	and	r0, r0, #0xFF
	cmp	r0, #0xFF
	beq	2f
	add	r1, r10, #S_VFIRST
	ldrb	r2, [r1, r0]
	cmp	r2, #0xFF
	strbeq	r4, [r1, r0]
	add	r1, r10, #S_VLAST
	strb	r4, [r1, r0]
2:	add	r7, r7, #8
	cmp	r7, #32
	blo	1b
	add	r4, r4, #1
	b	e_p
e_pd:	mov	r11, #0xF                       @ free registers
	mov	r4, #0
e_q:	cmp	r4, r5
	bhs	e_done
	add	r0, r10, #S_ORDER
	ldrb	r0, [r0, r4]
	add	r0, r10, r0, lsl #2
	ldr	r6, [r0, #S_VIDS]
	mov	r7, #0
e_f:	lsr	r8, r6, r7                      @ a field's value
	and	r8, r8, #0xFF
	cmp	r8, #0xFF
	beq	e_fn
	add	r0, r10, #S_VFIRST
	ldrb	r0, [r0, r8]
	cmp	r0, r4
	bne	e_fn                            @ not its first position
	add	r0, r10, #S_VASSIGN
	ldrb	r1, [r0, r8]
	cmp	r1, #0xFF
	bne	e_fn                            @ (the same value in two fields of this word)
	@ expire: values whose last use is not after this position give their register back
	ldr	r9, [sp, #12]
	mov	r3, #0
e_x:	cmp	r3, r9
	bhs	e_xd
	add	r0, r10, #S_VASSIGN
	ldrb	r1, [r0, r3]
	cmp	r1, #4
	bhs	e_xn                            @ not assigned yet (0xFF) or already given back (0x10 | reg)
	add	r2, r10, #S_VLAST
	ldrb	r2, [r2, r3]
	cmp	r2, r4
	bhi	e_xn
	bne	1f
	add	r2, r10, #S_VFIRST              @ last == this position: a value defined by this very word stays (two results of one word must not share a register)
	ldrb	r2, [r2, r3]
	cmp	r2, r4
	bhs	e_xn
1:	mov	r2, #1
	orr	r11, r11, r2, lsl r1
	orr	r1, r1, #0x10
	strb	r1, [r0, r3]
e_xn:	add	r3, r3, #1
	b	e_x
e_xd:	cmp	r11, #0
	beq	s_no                            @ more than four values live at once
	add	r0, r10, #S_VORIG
	ldrb	r0, [r0, r8]
	mov	r1, #1
	tst	r11, r1, lsl r0
	bne	1f
	rbit	r0, r11                         @ the lowest free
	clz	r0, r0
1:	bic	r11, r11, r1, lsl r0
	add	r1, r10, #S_VASSIGN
	strb	r0, [r1, r8]
e_fn:	add	r7, r7, #8
	cmp	r7, #32
	blo	e_f
	add	r4, r4, #1
	b	e_q
e_done:

@ ---- F: rewrite the words in the new order ----
	mov	r4, #0
f_p:	cmp	r4, r5
	bhs	f_done
	add	r0, r10, #S_ORDER
	ldrb	r11, [r0, r4]                   @ the node
	add	r0, r10, r11, lsl #2
	ldr	r9, [r0, #S_WORD]
	ldr	r8, [r0, #S_INFO]
	ldr	r6, [r0, #S_VIDS]
	mov	r7, #0                          @ field
f_f:	lsr	r0, r6, r7
	and	r0, r0, #0xFF
	cmp	r0, #0xFF
	beq	f_fn
	add	r1, r10, #S_VASSIGN
	ldrb	r1, [r1, r0]
	and	r1, r1, #3                      @ the register
	lsr	r2, r7, #1                      @ the field's info nibble: 4 * j = r7 / 2
	lsr	r3, r8, r2
	lsr	r3, r3, #2
	and	r3, r3, #3                      @ shift index
	cmp	r3, #0
	lslne	r3, r3, #2
	addne	r3, r3, #4
	mov	r12, #15
	bic	r9, r9, r12, lsl r3
	orr	r9, r9, r1, lsl r3
f_fn:	add	r7, r7, #8
	cmp	r7, #32
	blo	f_f
	add	r0, r10, r4, lsl #2
	str	r9, [r0, #S_OUT]
	add	r4, r4, #1
	b	f_p
f_done:
	ldr	r0, [sp]
	mov	r3, #0
1:	add	r1, r10, r3, lsl #2
	ldr	r2, [r1, #S_OUT]
	str	r2, [r0, r3, lsl #2]
	add	r3, r3, #1
	cmp	r3, r5
	blo	1b
	mov	r0, #1
	b	s_ret
s_no:	mov	r0, #0
s_ret:	add	sp, sp, #32
	pop	{r4-r11, pc}
	.ltorg
