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

	.global handle_DIVU
@ uint32_t num, uint32_t den
handle_DIVU:
    rsb		r1, r1, #0
    movs	r1, r1, lsl #15
    adds	r0, r1, r0
    rsbcc	r0, r1, r0

    .rept	15
    adcs	r0, r1, r0, lsl #1
    rsbcc	r0, r1, r0
    .endr

    adcs	r0, r0, r0

    bx		lr

    .global handle_DIVS
@ int32_t num, int32_t den    
handle_DIVS:
    mov     r2, #0
    cmp		r0, #0
    rsblt   r0, r0, #0
    eorlt   r2, r2, #1
    cmp		r1, #0
    rsblt   r1, r1, #0
    eorlt   r2, r2, #1

    rsb		r1, r1, #0
    
    movs	r1, r1, lsl #15
    adds	r0, r1, r0
    rsbcc	r0, r1, r0

    .rept	15
    adcs	r0, r1, r0, lsl #1
    rsbcc	r0, r1, r0
    .endr

    adc		r0, r0, r0
    cmp     r2, #1
    rsbeq   r0, r0, #0

    bx		lr 

    .end
