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

// Set C and Z flagz (do not care about V and N)
#define SAVEX() asm( \
    "lsls    r3, r0, #24   \n\t" \
    "ldrb    r0, [r5, %0]  \n\t" \
    "orrcs   r0, r0, #0x10 \n\t" \
    "biccc   r0, r0, #0x10 \n\t" \
    "strb    r0, [r5, %0]  \n\t" \
    :: "i"(offsetof(cpu_t, sr)))

__attribute__((naked)) void abcd_d0(uint8_t b) {
    asm("adc     r2, r3, #0x066     \n\t"
        "add     r1, r2, r0         \n\t"
        "eor     r2, r2, r0         \n\t"
        "eor     r2, r2, r1         \n\t"
        "mvn     r2, r2             \n\t"
        "and     r2, r2, #0x110     \n\t"
        "lsr     r0, r2, #3         \n\t"
        "orr     r2, r0, r2, lsr #2 \n\t"
        "sub     r2, r1, r2         \n\t");
    SAVEX();
    asm("bfi     r3, r2, #0, #8     \n\t"
        "bx      lr                 \n\t");
}

__attribute__((naked)) void abcd_d1(uint8_t b) {
    asm("adc     r2, r3, #0x066      \n\t"
        "add     r1, r2, r0          \n\t"
        "eor     r2, r2, r0          \n\t"
        "eor     r2, r2, r1          \n\t"
        "mvn     r2, r2              \n\t"
        "and     r2, r2, #0x110      \n\t"
        "lsr     r0, r2, #3          \n\t"
        "orr     r2, r0, r2, lsr #2  \n\t"
        "sub     r2, r1, r2          \n\t");
    SAVEX();
    asm("bfi     r4, r2, #0, #8      \n\t"
        "bx      lr                  \n\t");
}

__attribute__((naked)) void abcd_an(uint8_t b, uint8_t* a) {
    asm("str     r3, [r5, %0]        \n\t"
        "ldrb    r3, [r1]            \n\t"
        "adc     r3, r3, #0x066      \n\t"
        "add     r2, r3, r0          \n\t"
        "eor     r3, r3, r0          \n\t"
        "eor     r3, r3, r2          \n\t"
        "mvn     r3, r3              \n\t"
        "and     r3, r3, #0x110      \n\t"
        "lsr     r0, r3, #3          \n\t"
        "orr     r3, r0, r3, lsr #2  \n\t"
        "sub     r3, r2, r3          \n\t"
        :: "i"(offsetof(cpu_t, d0)));
    SAVEX();
    asm("strb    r3, [r1]            \n\t"
        "ldr     r3, [r5, %0]        \n\t"
        "bx      lr                  \n\t"
        :: "i"(offsetof(cpu_t, d0)));
}

__attribute__((naked)) void nbcd_d0(void) {
    asm("uxtb    r2, r3             \n\t"
        "rsc     r1, r2, #0xFF      \n\t"
        "rsb     r2, r2, #0         \n\t"
        "eor     r1, r1, #1         \n\t"
        "eor     r1, r1, r2         \n\t"
        "mvn     r1, r1             \n\t"
        "and     r1, r1, #0x110     \n\t"
        "lsr     r0, r1, #3         \n\t"
        "orr     r1, r0, r1, lsr #2 \n\t"
        "sub     r2, r2, r1         \n\t");
    SAVEX();
    asm("bfi     r3, r2, #0, #8     \n\t"
        "bx      lr                 \n\t");
}

__attribute__((naked)) void nbcd_d1(void) {
    asm("uxtb    r2, r4             \n\t"
        "rsc     r1, r2, #0xFF      \n\t"
        "rsb     r2, r2, #0         \n\t"
        "eor     r1, r1, #1         \n\t"
        "eor     r1, r1, r2         \n\t"
        "mvn     r1, r1             \n\t"
        "and     r1, r1, #0x110     \n\t"
        "lsr     r0, r1, #3         \n\t"
        "orr     r1, r0, r1, lsr #2 \n\t"
        "sub     r2, r2, r1         \n\t");
    SAVEX();
    asm("bfi     r4, r2, #0, #8     \n\t"
        "bx      lr                 \n\t");
}

__attribute__((naked)) void nbcd_an(uint8_t* addr) {
    asm("str     r3, [r5, %0]       \n\t"
        "ldrb    r2, [r0]           \n\t"
        "rsc     r3, r2, #0xFF      \n\t"
        "rsb     r2, r2, #0         \n\t"
        "eor     r3, r3, #1         \n\t"
        "eor     r3, r3, r2         \n\t"
        "mvn     r3, r3             \n\t"
        "and     r3, r3, #0x110     \n\t"
        "lsr     r1, r3, #3         \n\t"
        "orr     r3, r1, r3, lsr #2 \n\t"
        "sub     r3, r2, r3         \n\t"
        :: "i"(offsetof(cpu_t, d0)));
    SAVEX();
    asm("strb    r3, [r0]           \n\t"
        "ldr     r3, [r5, %0]       \n\t"
        "bx      lr                 \n\t"
        :: "i"(offsetof(cpu_t, d0)));
}

__attribute__((naked)) void sbcd_d0(uint8_t b) {
    asm("uxtb    r2, r3             \n\t"
        "rsc     r0, r0, #0x660     \n\t"
        "uxtab   r1, r0, r3         \n\t"
        "eor     r2, r2, r0         \n\t"
        "eor     r2, r2, r1         \n\t"
        "mvn     r2, r2             \n\t"
        "and     r2, r2, #0x110     \n\t"
        "lsr     r0, r2, #3         \n\t"
        "orr     r2, r0, r2, lsr #2 \n\t"
        "sub     r2, r1, r2         \n\t");
    SAVEX();
    asm("bfi     r3, r2, #0, #8     \n\t"
        "bx      lr                 \n\t");
}

__attribute__((naked)) void sbcd_d1(uint8_t b) {
    asm("uxtb    r2, r4             \n\t"
        "rsc     r0, r0, #0x660     \n\t"
        "uxtab   r1, r0, r4         \n\t"
        "eor     r2, r2, r0         \n\t"
        "eor     r2, r2, r1         \n\t"
        "mvn     r2, r2             \n\t"
        "and     r2, r2, #0x110     \n\t"
        "lsr     r0, r2, #3         \n\t"
        "orr     r2, r0, r2, lsr #2 \n\t"
        "sub     r2, r1, r2         \n\t");
    SAVEX();
    asm("bfi     r4, r2, #0, #8     \n\t"
        "bx      lr                 \n\t");
}

__attribute__((naked)) void sbcd_an(uint8_t b, uint8_t* addr) {
    asm("str     r3, [r5, %0]       \n\t"
        "ldrb    r3, [r1]           \n\t"
        "rsc     r0, r0, #0x660     \n\t"
        "add     r2, r0, r3         \n\t"
        "eor     r3, r3, r0         \n\t"
        "eor     r3, r3, r2         \n\t"
        "mvn     r3, r3             \n\t"
        "and     r3, r3, #0x110     \n\t"
        "lsr     r0, r3, #3         \n\t"
        "orr     r3, r0, r3, lsr #2 \n\t"
        "sub     r3, r2, r3         \n\t"
        :: "i"(offsetof(cpu_t, d0)));
    SAVEX();
    asm("strb    r3, [r1]           \n\t"
        "ldr     r3, [r5, %0]       \n\t"
        "bx      lr                 \n\t"
        :: "i"(offsetof(cpu_t, d0)));
}