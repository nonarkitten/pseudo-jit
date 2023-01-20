	.text
	.thumb_func
	.syntax unified
	.global extword_src_0000_pc
	.global extword_src_0000
extword_src_0000_pc:
	mov     r0, lr
	bfi     r1, r0, #2, #18
	bic     r1, #1
extword_src_0000:
	sxth    r0, r3
	add     r1, r0
	bx      lr

	.global extword_src_1000_pc
	.global extword_src_1000
extword_src_1000_pc:
	mov     r0, lr
	bfi     r1, r0, #2, #18
	bic     r1, #1
extword_src_1000:
	sxth    r0, r4
	add     r1, r0
	bx      lr

	.global extword_src_2000_pc
	.global extword_src_2000
extword_src_2000_pc:
	mov     r0, lr
	bfi     r1, r0, #2, #18
	bic     r1, #1
extword_src_2000:
	ldrsh   r0, [r5, #10]
	add     r1, r0
	bx      lr

	.global extword_src_3000_pc
	.global extword_src_3000
extword_src_3000_pc:
	mov     r0, lr
	bfi     r1, r0, #2, #18
	bic     r1, #1
extword_src_3000:
	ldrsh   r0, [r5, #14]
	add     r1, r0
	bx      lr

	.global extword_src_4000_pc
	.global extword_src_4000
extword_src_4000_pc:
	mov     r0, lr
	bfi     r1, r0, #2, #18
	bic     r1, #1
extword_src_4000:
	ldrsh   r0, [r5, #18]
	add     r1, r0
	bx      lr

	.global extword_src_5000_pc
	.global extword_src_5000
extword_src_5000_pc:
	mov     r0, lr
	bfi     r1, r0, #2, #18
	bic     r1, #1
extword_src_5000:
	ldrsh   r0, [r5, #22]
	add     r1, r0
	bx      lr

	.global extword_src_6000_pc
	.global extword_src_6000
extword_src_6000_pc:
	mov     r0, lr
	bfi     r1, r0, #2, #18
	bic     r1, #1
extword_src_6000:
	ldrsh   r0, [r5, #26]
	add     r1, r0
	bx      lr

	.global extword_src_7000_pc
	.global extword_src_7000
extword_src_7000_pc:
	mov     r0, lr
	bfi     r1, r0, #2, #18
	bic     r1, #1
extword_src_7000:
	ldrsh   r0, [r5, #30]
	add     r1, r0
	bx      lr

	.global extword_src_8000_pc
	.global extword_src_8000
extword_src_8000_pc:
	mov     r0, lr
	bfi     r1, r0, #2, #18
	bic     r1, #1
extword_src_8000:
	sxth    r0, r6
	add     r1, r0
	bx      lr

	.global extword_src_9000_pc
	.global extword_src_9000
extword_src_9000_pc:
	mov     r0, lr
	bfi     r1, r0, #2, #18
	bic     r1, #1
extword_src_9000:
	sxth    r0, r7
	add     r1, r0
	bx      lr

	.global extword_src_A000_pc
	.global extword_src_A000
extword_src_A000_pc:
	mov     r0, lr
	bfi     r1, r0, #2, #18
	bic     r1, #1
extword_src_A000:
	sxth    r0, r8
	add     r1, r0
	bx      lr

	.global extword_src_B000_pc
	.global extword_src_B000
extword_src_B000_pc:
	mov     r0, lr
	bfi     r1, r0, #2, #18
	bic     r1, #1
extword_src_B000:
	sxth    r0, r9
	add     r1, r0
	bx      lr

	.global extword_src_C000_pc
	.global extword_src_C000
extword_src_C000_pc:
	mov     r0, lr
	bfi     r1, r0, #2, #18
	bic     r1, #1
extword_src_C000:
	sxth    r0, r10
	add     r1, r0
	bx      lr

	.global extword_src_D000_pc
	.global extword_src_D000
extword_src_D000_pc:
	mov     r0, lr
	bfi     r1, r0, #2, #18
	bic     r1, #1
extword_src_D000:
	sxth    r0, r11
	add     r1, r0
	bx      lr

	.global extword_src_E000_pc
	.global extword_src_E000
extword_src_E000_pc:
	mov     r0, lr
	bfi     r1, r0, #2, #18
	bic     r1, #1
extword_src_E000:
	sxth    r0, r12
	add     r1, r0
	bx      lr

	.global extword_src_F000_pc
	.global extword_src_F000
extword_src_F000_pc:
	mov     r0, lr
	bfi     r1, r0, #2, #18
	bic     r1, #1
extword_src_F000:
	mov     r0, r13
	sxth    r0, r0
	add     r1, r0
	bx      lr

	.global extword_src_0800_pc
	.global extword_src_0800
extword_src_0800_pc:
	mov     r0, lr
	bfi     r1, r0, #2, #18
	bic     r1, #1
extword_src_0800:
	add     r1, r3
	bx      lr

	.global extword_src_1800_pc
	.global extword_src_1800
extword_src_1800_pc:
	mov     r0, lr
	bfi     r1, r0, #2, #18
	bic     r1, #1
extword_src_1800:
	add     r1, r4
	bx      lr

	.global extword_src_2800_pc
	.global extword_src_2800
extword_src_2800_pc:
	mov     r0, lr
	bfi     r1, r0, #2, #18
	bic     r1, #1
extword_src_2800:
	ldr     r0, [r5, #8]
	add     r1, r0
	bx      lr

	.global extword_src_3800_pc
	.global extword_src_3800
extword_src_3800_pc:
	mov     r0, lr
	bfi     r1, r0, #2, #18
	bic     r1, #1
extword_src_3800:
	ldr     r0, [r5, #12]
	add     r1, r0
	bx      lr

	.global extword_src_4800_pc
	.global extword_src_4800
extword_src_4800_pc:
	mov     r0, lr
	bfi     r1, r0, #2, #18
	bic     r1, #1
extword_src_4800:
	ldr     r0, [r5, #16]
	add     r1, r0
	bx      lr

	.global extword_src_5800_pc
	.global extword_src_5800
extword_src_5800_pc:
	mov     r0, lr
	bfi     r1, r0, #2, #18
	bic     r1, #1
extword_src_5800:
	ldr     r0, [r5, #20]
	add     r1, r0
	bx      lr

	.global extword_src_6800_pc
	.global extword_src_6800
extword_src_6800_pc:
	mov     r0, lr
	bfi     r1, r0, #2, #18
	bic     r1, #1
extword_src_6800:
	ldr     r0, [r5, #24]
	add     r1, r0
	bx      lr

	.global extword_src_7800_pc
	.global extword_src_7800
extword_src_7800_pc:
	mov     r0, lr
	bfi     r1, r0, #2, #18
	bic     r1, #1
extword_src_7800:
	ldr     r0, [r5, #28]
	add     r1, r0
	bx      lr

	.global extword_src_8800_pc
	.global extword_src_8800
extword_src_8800_pc:
	mov     r0, lr
	bfi     r1, r0, #2, #18
	bic     r1, #1
extword_src_8800:
	add     r1, r6
	bx      lr

	.global extword_src_9800_pc
	.global extword_src_9800
extword_src_9800_pc:
	mov     r0, lr
	bfi     r1, r0, #2, #18
	bic     r1, #1
extword_src_9800:
	add     r1, r7
	bx      lr

	.global extword_src_A800_pc
	.global extword_src_A800
extword_src_A800_pc:
	mov     r0, lr
	bfi     r1, r0, #2, #18
	bic     r1, #1
extword_src_A800:
	add     r1, r8
	bx      lr

	.global extword_src_B800_pc
	.global extword_src_B800
extword_src_B800_pc:
	mov     r0, lr
	bfi     r1, r0, #2, #18
	bic     r1, #1
extword_src_B800:
	add     r1, r9
	bx      lr

	.global extword_src_C800_pc
	.global extword_src_C800
extword_src_C800_pc:
	mov     r0, lr
	bfi     r1, r0, #2, #18
	bic     r1, #1
extword_src_C800:
	add     r1, r10
	bx      lr

	.global extword_src_D800_pc
	.global extword_src_D800
extword_src_D800_pc:
	mov     r0, lr
	bfi     r1, r0, #2, #18
	bic     r1, #1
extword_src_D800:
	add     r1, r11
	bx      lr

	.global extword_src_E800_pc
	.global extword_src_E800
extword_src_E800_pc:
	mov     r0, lr
	bfi     r1, r0, #2, #18
	bic     r1, #1
extword_src_E800:
	add     r1, r12
	bx      lr

	.global extword_src_F800_pc
	.global extword_src_F800
extword_src_F800_pc:
	mov     r0, lr
	bfi     r1, r0, #2, #18
	bic     r1, #1
extword_src_F800:
	add     r1, r13
	bx      lr

	.text
	.thumb_func
	.syntax unified
	.global extword_dst_0000
extword_dst_0000:
	sxth    r0, r3
	add     r2, r0
	bx      lr

	.global extword_dst_1000
extword_dst_1000:
	sxth    r0, r4
	add     r2, r0
	bx      lr

	.global extword_dst_2000
extword_dst_2000:
	ldrsh   r0, [r5, #10]
	add     r2, r0
	bx      lr

	.global extword_dst_3000
extword_dst_3000:
	ldrsh   r0, [r5, #14]
	add     r2, r0
	bx      lr

	.global extword_dst_4000
extword_dst_4000:
	ldrsh   r0, [r5, #18]
	add     r2, r0
	bx      lr

	.global extword_dst_5000
extword_dst_5000:
	ldrsh   r0, [r5, #22]
	add     r2, r0
	bx      lr

	.global extword_dst_6000
extword_dst_6000:
	ldrsh   r0, [r5, #26]
	add     r2, r0
	bx      lr

	.global extword_dst_7000
extword_dst_7000:
	ldrsh   r0, [r5, #30]
	add     r2, r0
	bx      lr

	.global extword_dst_8000
extword_dst_8000:
	sxth    r0, r6
	add     r2, r0
	bx      lr

	.global extword_dst_9000
extword_dst_9000:
	sxth    r0, r7
	add     r2, r0
	bx      lr

	.global extword_dst_A000
extword_dst_A000:
	sxth    r0, r8
	add     r2, r0
	bx      lr

	.global extword_dst_B000
extword_dst_B000:
	sxth    r0, r9
	add     r2, r0
	bx      lr

	.global extword_dst_C000
extword_dst_C000:
	sxth    r0, r10
	add     r2, r0
	bx      lr

	.global extword_dst_D000
extword_dst_D000:
	sxth    r0, r11
	add     r2, r0
	bx      lr

	.global extword_dst_E000
extword_dst_E000:
	sxth    r0, r12
	add     r2, r0
	bx      lr

	.global extword_dst_F000
extword_dst_F000:
	mov     r0, r13
	sxth    r0, r0
	add     r2, r0
	bx      lr

	.global extword_dst_0800
extword_dst_0800:
	add     r2, r3
	bx      lr

	.global extword_dst_1800
extword_dst_1800:
	add     r2, r4
	bx      lr

	.global extword_dst_2800
extword_dst_2800:
	ldr     r0, [r5, #8]
	add     r2, r0
	bx      lr

	.global extword_dst_3800
extword_dst_3800:
	ldr     r0, [r5, #12]
	add     r2, r0
	bx      lr

	.global extword_dst_4800
extword_dst_4800:
	ldr     r0, [r5, #16]
	add     r2, r0
	bx      lr

	.global extword_dst_5800
extword_dst_5800:
	ldr     r0, [r5, #20]
	add     r2, r0
	bx      lr

	.global extword_dst_6800
extword_dst_6800:
	ldr     r0, [r5, #24]
	add     r2, r0
	bx      lr

	.global extword_dst_7800
extword_dst_7800:
	ldr     r0, [r5, #28]
	add     r2, r0
	bx      lr

	.global extword_dst_8800
extword_dst_8800:
	add     r2, r6
	bx      lr

	.global extword_dst_9800
extword_dst_9800:
	add     r2, r7
	bx      lr

	.global extword_dst_A800
extword_dst_A800:
	add     r2, r8
	bx      lr

	.global extword_dst_B800
extword_dst_B800:
	add     r2, r9
	bx      lr

	.global extword_dst_C800
extword_dst_C800:
	add     r2, r10
	bx      lr

	.global extword_dst_D800
extword_dst_D800:
	add     r2, r11
	bx      lr

	.global extword_dst_E800
extword_dst_E800:
	add     r2, r12
	bx      lr

	.global extword_dst_F800
extword_dst_F800:
	add     r2, r13
	bx      lr

