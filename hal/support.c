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
#include "main.h"

void arm_flush_cache(uintptr_t addr, uint32_t length) {
    length = (length + 31) & ~31;
    while (length) {
        asm volatile("mcr p15, 0, %0, c7, c14, 1"::"r"(addr));
        addr += 32;
        length -= 32;
    }
    asm volatile("mcr p15, 0, %0, c7, c10, 4"::"r"(addr));
}

void arm_icache_invalidate(uintptr_t addr, uint32_t length) {
    length = (length + 31) & ~31;
    while (length) {
        asm volatile("mcr p15, 0, %0, c7, c5, 1"::"r"(addr));
        addr += 32;
        length -= 32;
    }
    asm volatile("mcr p15, 0, %0, c7, c10, 4"::"r"(addr));
}

void arm_dcache_invalidate(uintptr_t addr, uint32_t length) {
    length = (length + 31) & ~31;
    while (length) {
        asm volatile("mcr p15, 0, %0, c7, c6, 1"::"r"(addr));
        addr += 32;
        length -= 32;
    }
    asm volatile("mcr p15, 0, %0, c7, c10, 4"::"r"(addr));
}