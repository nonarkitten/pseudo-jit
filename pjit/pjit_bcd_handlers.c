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

#include "pjit.h"

#pragma push
#pragma optimize (O3)

static inline uint32_t save_x(uint32_t value) {
    uint32_t dump;
    // Set C and Z flagz (do not care about V and N)
    asm volatile("lsls    %0, %1, #24" : "=r"(dump) : "r"(value) : "cc");
    // Convert our C flag into an X flag
    asm volatile("ldrb    r0, [r5, %0]" :: "i"(1 + offsetof(cpu_t, sr)));
    asm volatile("orrcs   r0, r0, #0x10");
    asm volatile("biccc   r0, r0, #0x10");
    asm volatile("strb    r0, [r5, %0]" :: "i"(1 + offsetof(cpu_t, sr)));
    return value;
}

static inline uint8_t abcd(uint8_t a, uint8_t b) {
    uint32_t t1;
    asm("adc\t%0, %1, #0x066" : "=r"(t1) : "r"(b));
    uint32_t t2 = t1 + a;
    uint32_t t3 = t1 ^ a;
    uint32_t t4 = t2 ^ t3;
    uint32_t t5 = ~t4 & 0x110;
    uint32_t t6 = (t5 >> 2) | (t5 >> 3);
    return save_x(t2 - t6);
}

void abcd_d0(uint8_t a) { D0.B = abcd(a, D0.B);  }
void abcd_d1(uint8_t a) { D1.B = abcd(a, D0.B);  }
// TODO: This presently will hit the stack; this might be a problem
void abcd_an(uint8_t a, uint8_t* addr) { *addr = abcd(a, *addr); }

static inline uint8_t nbcd(uint8_t a) {
    uint32_t t1;
    asm("rsc\t%0, %1, #0xFF" : "=r"(t1) : "r"(a));
    uint32_t t2 = - a;
    uint32_t t3 = t1 ^ 0x1;
    uint32_t t4 = t2 ^ t3;
    uint32_t t5 = ~t4 & 0x110;
    uint32_t t6 = (t5 >> 2) | (t5 >> 3);
    return save_x(t2 - t6);
}

void nbcd_d0() { D0.B = nbcd(D0.B); }
void nbcd_d1() { D1.B = nbcd(D1.B); }
void nbcd_an(uint8_t* addr) { *addr = nbcd(*addr); }

static inline uint8_t sbcd(uint8_t a, uint8_t b) {
    uint32_t t1;
    asm("rsc\t%0, %1, #0x660" : "=r"(t1) : "r"(b));
    uint32_t t2 = t1 + a;
    uint32_t t3 = t1 ^ a;
    uint32_t t4 = t2 ^ t3;
    uint32_t t5 = ~t4 & 0x110;
    uint32_t t6 = (t5 >> 2) | (t5 >> 3);
    return save_x(t2 - t6);
}

void sbcd_d0(uint8_t b) { D0.B = sbcd(D0.B, b); }
void sbcd_d1(uint8_t b) { D1.B = sbcd(D1.B, b); }
// TODO: This presently will hit the stack; this might be a problem
void sbcd_an(uint8_t b, uint8_t* addr) { *addr = sbcd(*addr, b); }

#pragma pop
