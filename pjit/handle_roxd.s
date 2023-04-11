/*
 * Copyright (c) 2020-2023 Renee Cousins, the Buffee Project - http://www.buffee.ca
 *
 * This is part of PJIT the Pseudo-JIT 68K emulator.
 *
 * PJIT is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * PJIT is licensed under a Creative Commons
 * Attribution-NonCommercial-ShareAlike 4.0 International License.
 *
 * Under the terms of this license you are free copy and redistribute
 * the material in any medium or format as well as remix, transform,
 * and build upon the material.
 *
 * You must give appropriate credit, provide a link to the license,
 * and indicate if changes were made. You may do so in any reasonable
 * manner, but not in any way that suggests the licensor endorses you
 * or your use.
 *
 * You may not use the material for commercial purposes.
 *
 * If you remix, transform, or build upon the material, you must
 * distribute your contributions under the same license as the original.
 *
 * You may not apply legal terms or technological measures that legally
 * restrict others from doing anything the license permits.
 *
 * Portions of PJIT have been derived from the following:
 *
 *     Castaway (formerly FAST), GPL version 2 License
 *     Copyright (c) 1994-2002 Martin Döring, Joachim Hönig
 *    
 *     Cyclone 68K, GPL version 2 License
 *     Copyright (c) 2004,2011 Dave "FinalDave" Haywood
 *     Copyright (c) 2005-2011 Graûvydas "notaz" Ignotas
 *    
 *     TI StarterWare, modified BSD 3-Clause License
 *     Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/
 *
 *     libbbb, Apache License, Version 2.0
 *     Copyright 2015 University of Applied Sciences Western Switzerland / Fribourg
 */

	.text 
	.code 32
	.syntax unified

	.global roxl_b
@ uint32_t value, uint8_t shift
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
