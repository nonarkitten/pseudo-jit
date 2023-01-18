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
#ifndef MAIN_H_
#define MAIN_H_

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "pjit.h"

#include "init_power.h"
#include "hw_ddr.h"
#include "hw_flash.h"
#include "hw_gpmc.h"
#include "hw_init.h"
#include "hw_pru.h"
#include "hw_mmu.h"
#include "mem_be.h"
#include "pinmux.h"
#include "gpak.h"

/*
** The input clock speed
*/
#define FREQUENCY                    (24000000)

/*
** Number of entries in a page table. This is only for first level page table
*/
#define MMU_PAGETABLE_NUM_ENTRY      (4096)

/*
** Alignment for Page Table. The Page Table shall be aligned to 16KB boundary.
*/
#define MMU_PAGETABLE_ALIGN_SIZE     (16 * 1024)

extern void arm_flush_cache(uintptr_t addr, uint32_t length);
extern void arm_icache_invalidate(uintptr_t addr, uint32_t length);
extern void arm_dcache_invalidate(uintptr_t addr, uint32_t length);

extern void test_native_bogomips(void);
extern void test_pjit_bogomips(void);

extern void setecho(int);
extern void test_printf(void);

extern int pmic_detected;
extern int gpak_detected;
extern int prom_detected;

#ifndef _DEBUG
#define printf(...)
#endif

#endif /* MAIN_H_ */
