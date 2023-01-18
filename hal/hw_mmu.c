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

#include <stdio.h>
#include <stdint.h>

#include "hw_mmu.h"

int MMUEnabled = 0;

__attribute__ ((used, aligned (MMU_PAGETABLE_ALIGN_SIZE)))
static volatile uint32_t mmu_table[MMU_PAGETABLE_NUM_ENTRY] = { 0 };

void InitMMU(void) {
    // 0x000 to 0x7FF, GPMC/PIO strongly ordered, no caching
    for (int i=0x000; i < 0x020; i++) mmu_table[i] = ((i | 0x020) << 20) | 0x0c06;    
    for (int i=0x020; i < 0x800; i++) mmu_table[i] = (i << 20) | 0x0c06;
    // 0x800 to 0x9FF, SDRAM, Caches write-through, write allocate, access for all
    for (int i=0x800; i < 0xA00; i++) mmu_table[i] = (i << 20) | 0x1c0e;
    // 0x400 to 0x403, internal ROM and SRAM, Caches write-through, write allocate, access for all
    for (int i=0x400; i < 0x403; i++) mmu_table[i] = (i << 20) | 0x1c0e;

    arm_flush_cache((uint32_t)mmu_table, sizeof(mmu_table));
    
    // Load new pointer to the mmu table
    asm volatile(
    "       dsb                         \n"
    "       mcr     p15,0,r0,c7,c5,0    \n" /* invalidate icache */
    "       isb                         \n"
    "       ldr     r4,=mmu_table       \n" /* Load MMU table pointer */
    "       mcr     p15,0,r4,c2,c0,0    \n" /* Write page_dir address to ttbr0 */
    "       mov     r8, #0              \n"
    "       mcr     p15,0,r8,c2,c0,2    \n" /* Write ttbr control N = 0 (use only ttbr0) */
    "       mov     r4, #1              \n"
    "       mcr     p15,0,r4,c3,c0,0    \n" /* Set domains - Dom0 is usable, rest is disabled */
    "       mrc     p15,0,r4,c1,c0,0    \n" /* Load control register */
    "       orr     r4,r4,#0x800000     \n" /* v6 page tables, subpages disabled */
    "       orr     r4,r4,#1            \n" /* MMU enabled */
    "       orr     r4,r4,#1<<25        \n" /* MMU tables in big endian */
    "       dsb                         \n"
    "       mcr     p15,0,r4,c1,c0,0    \n" /* Set control register and thus really enable mmu */
    "       isb                         \n"
    );

    MMUEnabled = 1;
}