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

#include "pjit.h"

__attribute__((naked)) void handle_DIVS(uint16_t opccode) {

}
__attribute__((naked)) void handle_DIVU(uint16_t opccode) {

}

INLINE void save_cpu(void) {
    asm __volatile("str\tr3, [%0,%1]" ::"r"(cpu), "i"(offsetof(cpu_t, d0)));
    asm __volatile("str\tr4, [%0,%1]" ::"r"(cpu), "i"(offsetof(cpu_t, d1)));
    uint32_t *base = &cpu->a0;
    asm __volatile("stm\t%0!, {r6-r13}" ::"r"(base));
}

INLINE void restore_cpu(void) {
    asm __volatile("ldr\tr3, [%0,%1]" ::"r"(cpu), "i"(offsetof(cpu_t, d0)));
    asm __volatile("ldr\tr4, [%0,%1]" ::"r"(cpu), "i"(offsetof(cpu_t, d1)));
    uint32_t *base = &cpu->a0;
    asm __volatile("ldm\t%0!, {r6-r13}" ::"r"(base));
}

__attribute__((naked)) void handle_ROXd(uint16_t opccode) { }



