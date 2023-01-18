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
		// use vfp
		.global __aeabi_uidivmod
		.global __aeabi_idivmod
		.global __aeabi_uidiv
		.global __aeabi_idiv

		.global rev32

        .text

        @ This code is assembled for ARM instructions
        .code 32

__aeabi_uidivmod:
		vmov			s0, r0
		vcvt.f64.u32	d0, s0 @ num
		vmov			s2, r1
		vcvt.f64.u32	d1, s2 @ denom

		vdiv.f64		d2, d0, d1
		vcvt.u32.f64	s4, d2
		vmov			r0, s4 @ q = int(num / denom)

		vcvt.f64.u32	d2, s4 @ (double)q
		vmls.f64   		d0, d1, d2 @ num - (q * n)
		vcvt.u32.f64	s2, d0
		vmov			r1, s2

		bx				lr

__aeabi_idivmod:
		vmov			s0, r0
		vcvt.f64.s32	d0, s0 @ num
		vmov			s2, r1
		vcvt.f64.s32	d1, s2 @ denom

		vdiv.f64		d2, d0, d1
		vcvt.s32.f64	s4, d2
		vmov			r0, s4 @ q = int(num / denom)

		vcvt.f64.s32	d2, s4 @ (double)q
		vmls.f64   		d0, d1, d2 @ num - (q * n)
		vcvt.s32.f64	s2, d0
		vmov			r1, s2

		bx				lr

__aeabi_uidiv:
		vmov			s0, r0
		vcvt.f64.u32	d0, s0 @ num
		vmov			s2, r1
		vcvt.f64.u32	d1, s2 @ denom

		vdiv.f64		d2, d0, d1
		vcvt.u32.f64	s4, d2
		vmov			r0, s4 @ q = int(num / denom)

		bx				lr

__aeabi_idiv:
		vmov			s0, r0
		vcvt.f64.s32	d0, s0 @ num
		vmov			s2, r1
		vcvt.f64.s32	d1, s2 @ denom

		vdiv.f64		d2, d0, d1
		vcvt.s32.f64	s4, d2
		vmov			r0, s4 @ q = int(num / denom)

		bx				lr

rev32:
		rev				r0, r0
		bx				lr

		.end

