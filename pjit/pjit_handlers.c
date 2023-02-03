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

// #include "ops/m68k_common.h"
// #include "ops/m68k_cpu.h"
// #include "ops/m68k_emit_ea.h"
// #include "ops/m68k_registers.h"

#include <stdint.h>

#include "arm.h"
#include "pjit.h"

uint32_t handle_DIVU(uint32_t num, uint32_t den) {
    uint32_t quotient = 0;
    // Unroll division completely
    if (den <= (num >> 15)) { quotient |= 1 << 15; num -= den << 15; }
    if (den <= (num >> 14)) { quotient |= 1 << 14; num -= den << 14; }
    if (den <= (num >> 13)) { quotient |= 1 << 13; num -= den << 13; }
    if (den <= (num >> 12)) { quotient |= 1 << 12; num -= den << 12; }
    if (den <= (num >> 11)) { quotient |= 1 << 11; num -= den << 11; }
    if (den <= (num >> 10)) { quotient |= 1 << 10; num -= den << 10; }
    if (den <= (num >>  9)) { quotient |= 1 <<  9; num -= den <<  9; }
    if (den <= (num >>  8)) { quotient |= 1 <<  8; num -= den <<  8; }
    if (den <= (num >>  7)) { quotient |= 1 <<  7; num -= den <<  7; }
    if (den <= (num >>  6)) { quotient |= 1 <<  6; num -= den <<  6; }
    if (den <= (num >>  5)) { quotient |= 1 <<  5; num -= den <<  5; }
    if (den <= (num >>  4)) { quotient |= 1 <<  4; num -= den <<  4; }
    if (den <= (num >>  3)) { quotient |= 1 <<  3; num -= den <<  3; }
    if (den <= (num >>  2)) { quotient |= 1 <<  2; num -= den <<  2; }
    if (den <= (num >>  1)) { quotient |= 1 <<  1; num -= den <<  1; }
    if (den <= (num >>  0)) { quotient |= 1 <<  0; num -= den <<  0; } 

    // Set flags for N and Z
    asm volatile("cmp\t%0, #0" :: "r"(quotient));
    // Set V if overflow
    asm volatile("mrs\t%0, cpsr" : "=r"(den));
    if((quotient > 65535) || (num > 65535)) den |= 0x10000000;
    asm volatile("msr\tcpsr_f, %0" :: "r"(den));
    return (num << 16) | quotient;
}

int32_t handle_DIVS(int32_t num, int32_t den) {
    uint32_t quotient = 0;
    if(num < 0) num = -num, quotient = ~quotient;
    if(den < 0) den = -den, quotient = ~quotient;

    if(quotient) {
        // Unroll division completely
        if (den <= (num >> 15)) { quotient &= ~(1 << 15); num -= den << 15; }
        if (den <= (num >> 14)) { quotient &= ~(1 << 14); num -= den << 14; }
        if (den <= (num >> 13)) { quotient &= ~(1 << 13); num -= den << 13; }
        if (den <= (num >> 12)) { quotient &= ~(1 << 12); num -= den << 12; }
        if (den <= (num >> 11)) { quotient &= ~(1 << 11); num -= den << 11; }
        if (den <= (num >> 10)) { quotient &= ~(1 << 10); num -= den << 10; }
        if (den <= (num >>  9)) { quotient &= ~(1 <<  9); num -= den <<  9; }
        if (den <= (num >>  8)) { quotient &= ~(1 <<  8); num -= den <<  8; }
        if (den <= (num >>  7)) { quotient &= ~(1 <<  7); num -= den <<  7; }
        if (den <= (num >>  6)) { quotient &= ~(1 <<  6); num -= den <<  6; }
        if (den <= (num >>  5)) { quotient &= ~(1 <<  5); num -= den <<  5; }
        if (den <= (num >>  4)) { quotient &= ~(1 <<  4); num -= den <<  4; }
        if (den <= (num >>  3)) { quotient &= ~(1 <<  3); num -= den <<  3; }
        if (den <= (num >>  2)) { quotient &= ~(1 <<  2); num -= den <<  2; }
        if (den <= (num >>  1)) { quotient &= ~(1 <<  1); num -= den <<  1; }
        if (den <= (num >>  0)) { quotient &= ~(1 <<  0); num -= den <<  0; }

        quotient += 1;

        // Set flags for N and Z
        asm volatile("cmp\t%0, #0" :: "r"(quotient));
        // Set V if overflow
        asm volatile("mrs\t%0, cpsr" : "=r"(den));
        if(num > 65535) den |= 0x10000000;
        asm volatile("msr\tcpsr_f, %0" :: "r"(den));
        return (-num << 16) | quotient;

    } else {
        // Unroll division completely
        if (den <= (num >> 15)) { quotient |= 1 << 15; num -= den << 15; }
        if (den <= (num >> 14)) { quotient |= 1 << 14; num -= den << 14; }
        if (den <= (num >> 13)) { quotient |= 1 << 13; num -= den << 13; }
        if (den <= (num >> 12)) { quotient |= 1 << 12; num -= den << 12; }
        if (den <= (num >> 11)) { quotient |= 1 << 11; num -= den << 11; }
        if (den <= (num >> 10)) { quotient |= 1 << 10; num -= den << 10; }
        if (den <= (num >>  9)) { quotient |= 1 <<  9; num -= den <<  9; }
        if (den <= (num >>  8)) { quotient |= 1 <<  8; num -= den <<  8; }
        if (den <= (num >>  7)) { quotient |= 1 <<  7; num -= den <<  7; }
        if (den <= (num >>  6)) { quotient |= 1 <<  6; num -= den <<  6; }
        if (den <= (num >>  5)) { quotient |= 1 <<  5; num -= den <<  5; }
        if (den <= (num >>  4)) { quotient |= 1 <<  4; num -= den <<  4; }
        if (den <= (num >>  3)) { quotient |= 1 <<  3; num -= den <<  3; }
        if (den <= (num >>  2)) { quotient |= 1 <<  2; num -= den <<  2; }
        if (den <= (num >>  1)) { quotient |= 1 <<  1; num -= den <<  1; }
        if (den <= (num >>  0)) { quotient |= 1 <<  0; num -= den <<  0; } 
    
        // Set flags for N and Z
        asm volatile("cmp\t%0, #0" :: "r"(quotient));
        // Set V if overflow
        asm volatile("mrs\t%0, cpsr" : "=r"(den));
        if(num > 65535) den |= 0x10000000;
        asm volatile("msr\tcpsr_f, %0" :: "r"(den));
        return (num << 16) | quotient;
    }
}

// this is ONLY called for register ROXd
uint32_t roxl_l(uint32_t value, uint8_t shift) {
    static const uint32_t* back = &&r0;
    static const uint32_t* exit = &&r31;
    goto *(void*)(back - (shift & 0x1F));
r31:asm volatile("adcs\t%0, %0, %0" : "+r"(value));
r30:asm volatile("adcs\t%0, %0, %0" : "+r"(value));
r29:asm volatile("adcs\t%0, %0, %0" : "+r"(value));
r28:asm volatile("adcs\t%0, %0, %0" : "+r"(value));
r27:asm volatile("adcs\t%0, %0, %0" : "+r"(value));
r26:asm volatile("adcs\t%0, %0, %0" : "+r"(value));
r25:asm volatile("adcs\t%0, %0, %0" : "+r"(value));
r24:asm volatile("adcs\t%0, %0, %0" : "+r"(value));
r23:asm volatile("adcs\t%0, %0, %0" : "+r"(value));
r22:asm volatile("adcs\t%0, %0, %0" : "+r"(value));
r21:asm volatile("adcs\t%0, %0, %0" : "+r"(value));
r20:asm volatile("adcs\t%0, %0, %0" : "+r"(value));
r19:asm volatile("adcs\t%0, %0, %0" : "+r"(value));
r18:asm volatile("adcs\t%0, %0, %0" : "+r"(value));
r17:asm volatile("adcs\t%0, %0, %0" : "+r"(value));
r16:asm volatile("adcs\t%0, %0, %0" : "+r"(value));
r15:asm volatile("adcs\t%0, %0, %0" : "+r"(value));
r14:asm volatile("adcs\t%0, %0, %0" : "+r"(value));
r13:asm volatile("adcs\t%0, %0, %0" : "+r"(value));
r12:asm volatile("adcs\t%0, %0, %0" : "+r"(value));
r11:asm volatile("adcs\t%0, %0, %0" : "+r"(value));
r10:asm volatile("adcs\t%0, %0, %0" : "+r"(value));
r9: asm volatile("adcs\t%0, %0, %0" : "+r"(value));
r8: asm volatile("adcs\t%0, %0, %0" : "+r"(value));
r7: asm volatile("adcs\t%0, %0, %0" : "+r"(value));
r6: asm volatile("adcs\t%0, %0, %0" : "+r"(value));
r5: asm volatile("adcs\t%0, %0, %0" : "+r"(value));
r4: asm volatile("adcs\t%0, %0, %0" : "+r"(value));
r3: asm volatile("adcs\t%0, %0, %0" : "+r"(value));
r2: asm volatile("adcs\t%0, %0, %0" : "+r"(value));
r1: asm volatile("adcs\t%0, %0, %0" : "+r"(value));
r0: return value;
}

uint32_t roxl_w(uint32_t value, uint8_t shift) {
    static const uint32_t* back = &&r0;
    static const uint32_t* exit = &&r15;
    goto *(void*)(back - (shift & 0xF));
r15:asm volatile("uxth\t%0, %0\n\tadcs\t%0, %0, %0" : "+r"(value));
r14:asm volatile("uxth\t%0, %0\n\tadcs\t%0, %0, %0" : "+r"(value));
r13:asm volatile("uxth\t%0, %0\n\tadcs\t%0, %0, %0" : "+r"(value));
r12:asm volatile("uxth\t%0, %0\n\tadcs\t%0, %0, %0" : "+r"(value));
r11:asm volatile("uxth\t%0, %0\n\tadcs\t%0, %0, %0" : "+r"(value));
r10:asm volatile("uxth\t%0, %0\n\tadcs\t%0, %0, %0" : "+r"(value));
r9: asm volatile("uxth\t%0, %0\n\tadcs\t%0, %0, %0" : "+r"(value));
r8: asm volatile("uxth\t%0, %0\n\tadcs\t%0, %0, %0" : "+r"(value));
r7: asm volatile("uxth\t%0, %0\n\tadcs\t%0, %0, %0" : "+r"(value));
r6: asm volatile("uxth\t%0, %0\n\tadcs\t%0, %0, %0" : "+r"(value));
r5: asm volatile("uxth\t%0, %0\n\tadcs\t%0, %0, %0" : "+r"(value));
r4: asm volatile("uxth\t%0, %0\n\tadcs\t%0, %0, %0" : "+r"(value));
r3: asm volatile("uxth\t%0, %0\n\tadcs\t%0, %0, %0" : "+r"(value));
r2: asm volatile("uxth\t%0, %0\n\tadcs\t%0, %0, %0" : "+r"(value));
r1: asm volatile("uxth\t%0, %0\n\tadcs\t%0, %0, %0" : "+r"(value));
r0: return value;
}

uint32_t roxl_b(uint32_t value, uint8_t shift) {
    static const uint32_t* back = &&r0;
    static const uint32_t* exit = &&r7;
    goto *(void*)(back - (shift & 0x7));
r7: asm volatile("uxtb\t%0, %0\n\tadcs\t%0, %0, %0" : "+r"(value));
r6: asm volatile("uxtb\t%0, %0\n\tadcs\t%0, %0, %0" : "+r"(value));
r5: asm volatile("uxtb\t%0, %0\n\tadcs\t%0, %0, %0" : "+r"(value));
r4: asm volatile("uxtb\t%0, %0\n\tadcs\t%0, %0, %0" : "+r"(value));
r3: asm volatile("uxtb\t%0, %0\n\tadcs\t%0, %0, %0" : "+r"(value));
r2: asm volatile("uxtb\t%0, %0\n\tadcs\t%0, %0, %0" : "+r"(value));
r1: asm volatile("uxtb\t%0, %0\n\tadcs\t%0, %0, %0" : "+r"(value));
r0: return value;
}

// this is ONLY called for register ROXd
uint32_t roxr_l(uint32_t value, uint8_t shift) {
    static const uint32_t* back = &&r0;
    static const uint32_t* exit = &&r31;
    asm volatile("rbit\t%0, %0" : "+r"(value));
    goto *(void*)(back - (shift & 0x1F));
r31:asm volatile("adcs\t%0, %0, %0" : "+r"(value));
r30:asm volatile("adcs\t%0, %0, %0" : "+r"(value));
r29:asm volatile("adcs\t%0, %0, %0" : "+r"(value));
r28:asm volatile("adcs\t%0, %0, %0" : "+r"(value));
r27:asm volatile("adcs\t%0, %0, %0" : "+r"(value));
r26:asm volatile("adcs\t%0, %0, %0" : "+r"(value));
r25:asm volatile("adcs\t%0, %0, %0" : "+r"(value));
r24:asm volatile("adcs\t%0, %0, %0" : "+r"(value));
r23:asm volatile("adcs\t%0, %0, %0" : "+r"(value));
r22:asm volatile("adcs\t%0, %0, %0" : "+r"(value));
r21:asm volatile("adcs\t%0, %0, %0" : "+r"(value));
r20:asm volatile("adcs\t%0, %0, %0" : "+r"(value));
r19:asm volatile("adcs\t%0, %0, %0" : "+r"(value));
r18:asm volatile("adcs\t%0, %0, %0" : "+r"(value));
r17:asm volatile("adcs\t%0, %0, %0" : "+r"(value));
r16:asm volatile("adcs\t%0, %0, %0" : "+r"(value));
r15:asm volatile("adcs\t%0, %0, %0" : "+r"(value));
r14:asm volatile("adcs\t%0, %0, %0" : "+r"(value));
r13:asm volatile("adcs\t%0, %0, %0" : "+r"(value));
r12:asm volatile("adcs\t%0, %0, %0" : "+r"(value));
r11:asm volatile("adcs\t%0, %0, %0" : "+r"(value));
r10:asm volatile("adcs\t%0, %0, %0" : "+r"(value));
r9: asm volatile("adcs\t%0, %0, %0" : "+r"(value));
r8: asm volatile("adcs\t%0, %0, %0" : "+r"(value));
r7: asm volatile("adcs\t%0, %0, %0" : "+r"(value));
r6: asm volatile("adcs\t%0, %0, %0" : "+r"(value));
r5: asm volatile("adcs\t%0, %0, %0" : "+r"(value));
r4: asm volatile("adcs\t%0, %0, %0" : "+r"(value));
r3: asm volatile("adcs\t%0, %0, %0" : "+r"(value));
r2: asm volatile("adcs\t%0, %0, %0" : "+r"(value));
r1: asm volatile("adcs\t%0, %0, %0" : "+r"(value));
r0: asm volatile("rbit\t%0, %0" : "+r"(value));
    return value;
}

uint32_t roxr_w(uint32_t value, uint8_t shift) {
    static const uint32_t* back = &&r0;
    static const uint32_t* exit = &&r15;
    asm volatile("rbit\t%0, %0" : "+r"(value));
    goto *(void*)(back - (shift & 0xF));
r15:asm volatile("uxth\t%0, %0\n\tadcs\t%0, %0, %0" : "+r"(value));
r14:asm volatile("uxth\t%0, %0\n\tadcs\t%0, %0, %0" : "+r"(value));
r13:asm volatile("uxth\t%0, %0\n\tadcs\t%0, %0, %0" : "+r"(value));
r12:asm volatile("uxth\t%0, %0\n\tadcs\t%0, %0, %0" : "+r"(value));
r11:asm volatile("uxth\t%0, %0\n\tadcs\t%0, %0, %0" : "+r"(value));
r10:asm volatile("uxth\t%0, %0\n\tadcs\t%0, %0, %0" : "+r"(value));
r9: asm volatile("uxth\t%0, %0\n\tadcs\t%0, %0, %0" : "+r"(value));
r8: asm volatile("uxth\t%0, %0\n\tadcs\t%0, %0, %0" : "+r"(value));
r7: asm volatile("uxth\t%0, %0\n\tadcs\t%0, %0, %0" : "+r"(value));
r6: asm volatile("uxth\t%0, %0\n\tadcs\t%0, %0, %0" : "+r"(value));
r5: asm volatile("uxth\t%0, %0\n\tadcs\t%0, %0, %0" : "+r"(value));
r4: asm volatile("uxth\t%0, %0\n\tadcs\t%0, %0, %0" : "+r"(value));
r3: asm volatile("uxth\t%0, %0\n\tadcs\t%0, %0, %0" : "+r"(value));
r2: asm volatile("uxth\t%0, %0\n\tadcs\t%0, %0, %0" : "+r"(value));
r1: asm volatile("uxth\t%0, %0\n\tadcs\t%0, %0, %0" : "+r"(value));
r0: asm volatile("rbit\t%0, %0" : "+r"(value));
    return value;
}

uint32_t roxr_b(uint32_t value, uint8_t shift) {
    static const uint32_t* back = &&r0;
    static const uint32_t* exit = &&r7;
    asm volatile("rbit\t%0, %0" : "+r"(value));
    goto *(void*)(back - (shift & 0x7));
r7: asm volatile("uxtb\t%0, %0\n\tadcs\t%0, %0, %0" : "+r"(value));
r6: asm volatile("uxtb\t%0, %0\n\tadcs\t%0, %0, %0" : "+r"(value));
r5: asm volatile("uxtb\t%0, %0\n\tadcs\t%0, %0, %0" : "+r"(value));
r4: asm volatile("uxtb\t%0, %0\n\tadcs\t%0, %0, %0" : "+r"(value));
r3: asm volatile("uxtb\t%0, %0\n\tadcs\t%0, %0, %0" : "+r"(value));
r2: asm volatile("uxtb\t%0, %0\n\tadcs\t%0, %0, %0" : "+r"(value));
r1: asm volatile("uxtb\t%0, %0\n\tadcs\t%0, %0, %0" : "+r"(value));
r0: asm volatile("rbit\t%0, %0" : "+r"(value));
    return value;
}




