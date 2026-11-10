/*
 * Copyright (c) 2020-2023 Renee Cousins, the Buffee Project - http://www.buffee.ca
 *
 * Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (see LICENSE).
 */

@ ROXL/ROXR helpers: rotate through the X flag, which ARM has no single
@ instruction for. Loop-free -- each entry computes a jump into an
@ unrolled chain of adcs, so the cost is the shift count, not a branch
@ per bit. r0 = value in and out, r1 = shift count, ARM C = 68K X.
@
@ NB these clobber r3 (adr r3, 0f), which is scratch for every caller (D0 is pinned in r4).
	.text 
	.code 32
	.syntax unified

	.global roxl_b
roxl_b:
    adr     r3, 0f
    and     r1, r1, #7
    sub     pc, r3, r1, lsl #3
    .rept 8
    uxtb    r0, r0
    adcs    r0, r0, r0
    .endr
0:  bx      lr

	.global roxl_w
@ uint32_t value, uint8_t shift
roxl_w:
    adr     r3, 0f
    and     r1, r1, #15
    sub     pc, r3, r1, lsl #3
    .rept 16
    uxth    r0, r0
    adcs    r0, r0, r0
    .endr
0:  bx      lr

	.global roxl_l
@ uint32_t value, uint8_t shift
roxl_l:
    adr     r3, 0f
    and     r1, r1, #31
    sub     pc, r3, r1, lsl #2
    .rept 32
    adcs    r0, r0, r0
    .endr
0:  bx      lr

	.global roxr_b
@ uint32_t value, uint8_t shift
roxr_b:
    rbit    r0, r0
    adr     r3, 0f
    and     r1, r1, #7
    sub     pc, r3, r1, lsl #3
    .rept 8
    uxtb    r0, r0
    adcs    r0, r0, r0
    .endr
0:  rbit    r0, r0
    bx      lr

	.global roxr_w
@ uint32_t value, uint8_t shift
roxr_w:
    rbit    r0, r0
    adr     r3, 0f
    and     r1, r1, #15
    sub     pc, r3, r1, lsl #3
    .rept 16
    uxth    r0, r0
    adcs    r0, r0, r0
    .endr
0:  rbit    r0, r0
    bx      lr

	.global roxr_l
@ uint32_t value, uint8_t shift
roxr_l:
    rbit    r0, r0
    adr     r3, 0f
    and     r1, r1, #31
    sub     pc, r3, r1, lsl #2
    .rept 32
    adcs    r0, r0, r0
    .endr
0:  rbit    r0, r0
    bx      lr

    .end
