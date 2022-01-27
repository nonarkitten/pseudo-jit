@*****************************************************************************
@ This API invlidates I-cache lines from the star address till the length   
@ specified to PoU.
@ r0 - Start Address 
@ r1 - End Address (non-inclusive)
@*****************************************************************************

	.text 
	.code 32
	.global ICacheFlush
ICacheFlush:
    DMB
    MRC     p15, #0, r2, c0, c0, #1   @ Read Cache Type Register
    UBFX    r2, r2, #0, #4            @ Extract the DMinLine
    MOV     r3, #2
    ADD     r3, r3, r2
    MOV     r2, #1
    LSL     r2, r2, r3                @ Calculate the line size
   
    SUB     r3, r2, #1                @ Calculate the mask
    BIC     r0, r0, r3                @ Align to cache line boundary   
    TST     r3, r1
    BIC     r1, r1, r3
    MCRNE   p15, #0, r14, c7, c5, #1  @ Invalidate by MVA to PoU

iflushloop:    
    MCR     p15, #0, r0, c7, c5, #1   @ Invalidate by MVA to PoU
    ADDS    r0, r0, r2                @ Go to next line
    CMP     r0, r1 
    BLT     iflushloop
 
    DSB
    BX      lr

	.global SDiv32x16	
SDiv32x16:
	MOV     r2, #0
	CMP		r0, #0
	RSBLT   r0, r0, #0
	EORLT   r2, r2, #1
	CMP		r1, #0
	RSBLT   r1, r1, #0
	EORLT   r2, r2, #1

	RSB		r1, r1, #0
	
	MOVS	r1, r1, lsl #15
	ADDS	r0, r1, r0
	RSBCC	r0, r1, r0

	.rept	15
	ADCS	r0, r1, r0, lsl #1
	RSBCC	r0, r1, r0
	.endr

	ADC		r0, r0, r0
	CMP     r2, #1
	RSBEQ   r0, r0, #0

	BX		lr
	
	.global UDiv32x16	
UDiv32x16:
	RSB		r1, r1, #0
	MOVS	r1, r1, lsl #15
	ADDS	r0, r1, r0
	RSBCC	r0, r1, r0

	.rept	15
	ADCS	r0, r1, r0, lsl #1
	RSBCC	r0, r1, r0
	.endr

	ADCS	r0, r0, r0

	BX		lr
























