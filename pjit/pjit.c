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
 *
 *     emu68 (https://github.com/michalsc), Mozilla Public License, v. 2.0
 *     Copyright © 2019 Michal Schulz <michal.schulz@gmx.de>
 */

#include "arm.h"
#include "pjit.h"
#include "extword.h"

extern const uint32_t optab[65536];
extern const uint8_t  oplen[65536];

static void cpu_lookup_inline(void);
static void cpu_lookup_noinline(void);
static void cpu_lookup_nojit(void);

// The following Copyrights are included as the original author wrote them
// modified BSD 3-Clause License requires inclusion of Copyright with binaries
__attribute__((used)) const char *ti_c = "Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/";
// Apache License requires inclusion of Copyright with binaries
__attribute__((used)) const char *libbbb_c = "Copyright 2015 University of Applied Sciences Western Switzerland / Fribourg";
// Mozilla Public License, v. 2.0 requires inclusion of Copyright with binaries
__attribute__((used)) const char *emu68_c = "Copyright © 2019 Michal Schulz <michal.schulz@gmx.de>";
// GPL does NOT require inclusion of Copyright with binaries but are included anyway
__attribute__((used)) const char *c68k_c1 = "Copyright (c) 2004,2011 Dave \"FinalDave\" Haywood (emudave (at) gmail.com)";
__attribute__((used)) const char *c68k_c2 = "Copyright (c) 2005-2011 Graûvydas \"notaz\" Ignotas (notasas (at) gmail.com)";
__attribute__((used)) const char *cast_c  = "Copyright (c) 1994-2002 Martin Döring, Joachim Hönig";
// Our copyright
__attribute__((used)) const char *buffe_c = "Copyright (c) 2020-2023 Renee Cousins, the Buffee Project - http://www.buffee.ca";

/*   ____          _        _____           _ _   _
    / ___|___   __| | ___  | ____|_ __ ___ (_) |_| |_ ___ _ __ ___
   | |   / _ \ / _` |/ _ \ |  _| | '_ ` _ \| | __| __/ _ \ '__/ __|
   | |__| (_) | (_| |  __/ | |___| | | | | | | |_| ||  __/ |  \__ \
    \____\___/ \__,_|\___| |_____|_| |_| |_|_|\__|\__\___|_|  |___/ */

static inline uint32_t emit_src_ext(uint32_t current, uint16_t opcode) {
    static const uint32_t ext_handler[32] = {
        (uint32_t)extword_src_0000,(uint32_t)extword_src_1000,(uint32_t)extword_src_2000,(uint32_t)extword_src_3000,
        (uint32_t)extword_src_4000,(uint32_t)extword_src_5000,(uint32_t)extword_src_6000,(uint32_t)extword_src_7000,
        (uint32_t)extword_src_8000,(uint32_t)extword_src_9000,(uint32_t)extword_src_A000,(uint32_t)extword_src_B000,
        (uint32_t)extword_src_C000,(uint32_t)extword_src_D000,(uint32_t)extword_src_E000,(uint32_t)extword_src_F000,
        (uint32_t)extword_src_0800,(uint32_t)extword_src_1800,(uint32_t)extword_src_2800,(uint32_t)extword_src_3800,
        (uint32_t)extword_src_4800,(uint32_t)extword_src_5800,(uint32_t)extword_src_6800,(uint32_t)extword_src_7800,
        (uint32_t)extword_src_8800,(uint32_t)extword_src_9800,(uint32_t)extword_src_A800,(uint32_t)extword_src_B800,
        (uint32_t)extword_src_C800,(uint32_t)extword_src_D800,(uint32_t)extword_src_E800,(uint32_t)extword_src_F800,
    };  
    return b_imm(calc_offset(current, ext_handler[opcode >> 11] + ((opcode & 0xFF) * 8)));
}
static inline uint32_t emit_src_ext_pc(uint32_t current, uint16_t opcode) {
    static const uint32_t ext_pc_handler[32] = {
        (uint32_t)extword_src_0000_pc,(uint32_t)extword_src_1000_pc,(uint32_t)extword_src_2000_pc,
        (uint32_t)extword_src_3000_pc,(uint32_t)extword_src_4000_pc,(uint32_t)extword_src_5000_pc,
        (uint32_t)extword_src_6000_pc,(uint32_t)extword_src_7000_pc,(uint32_t)extword_src_8000_pc,
        (uint32_t)extword_src_9000_pc,(uint32_t)extword_src_A000_pc,(uint32_t)extword_src_B000_pc,
        (uint32_t)extword_src_C000_pc,(uint32_t)extword_src_D000_pc,(uint32_t)extword_src_E000_pc,
        (uint32_t)extword_src_F000_pc,(uint32_t)extword_src_0800_pc,(uint32_t)extword_src_1800_pc,
        (uint32_t)extword_src_2800_pc,(uint32_t)extword_src_3800_pc,(uint32_t)extword_src_4800_pc,
        (uint32_t)extword_src_5800_pc,(uint32_t)extword_src_6800_pc,(uint32_t)extword_src_7800_pc,
        (uint32_t)extword_src_8800_pc,(uint32_t)extword_src_9800_pc,(uint32_t)extword_src_A800_pc,
        (uint32_t)extword_src_B800_pc,(uint32_t)extword_src_C800_pc,(uint32_t)extword_src_D800_pc,
        (uint32_t)extword_src_E800_pc,(uint32_t)extword_src_F800_pc,
    };    
    return b_imm(calc_offset(current, ext_pc_handler[opcode >> 11] + ((opcode & 0xFF) * 8)));
}
static inline uint32_t emit_dst_ext(uint32_t current, uint16_t opcode) {
    static const uint32_t ext_handler[32] = {
        (uint32_t)extword_dst_0000,(uint32_t)extword_dst_1000,(uint32_t)extword_dst_2000,(uint32_t)extword_dst_3000,
        (uint32_t)extword_dst_4000,(uint32_t)extword_dst_5000,(uint32_t)extword_dst_6000,(uint32_t)extword_dst_7000,
        (uint32_t)extword_dst_8000,(uint32_t)extword_dst_9000,(uint32_t)extword_dst_A000,(uint32_t)extword_dst_B000,
        (uint32_t)extword_dst_C000,(uint32_t)extword_dst_D000,(uint32_t)extword_dst_E000,(uint32_t)extword_dst_F000,
        (uint32_t)extword_dst_0800,(uint32_t)extword_dst_1800,(uint32_t)extword_dst_2800,(uint32_t)extword_dst_3800,
        (uint32_t)extword_dst_4800,(uint32_t)extword_dst_5800,(uint32_t)extword_dst_6800,(uint32_t)extword_dst_7800,
        (uint32_t)extword_dst_8800,(uint32_t)extword_dst_9800,(uint32_t)extword_dst_A800,(uint32_t)extword_dst_B800,
        (uint32_t)extword_dst_C800,(uint32_t)extword_dst_D800,(uint32_t)extword_dst_E800,(uint32_t)extword_dst_F800,
    };
    return b_imm(calc_offset(current, ext_handler[opcode >> 11] + ((opcode & 0xFF) * 8)));
}

typedef enum {
    EXT_NONE,
    EXT_I16,
    EXT_I32,
    EXT_D8_XAREG,
    EXT_D16_PC,
    EXT_D8_XPC,
    EXT_COUNT
} ext_t;

static uint8_t get_ext(uint16_t opcode) {
    static const uint8_t ext_modes[31][8] = {
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, 
        { 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01 }, 
        { 0x01, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, 
        { 0x01, 0x02, 0x04, 0x05, 0x00, 0x00, 0x00, 0x00 }, 
        { 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02 }, 
        { 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03 }, 
        { 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10 }, 
        { 0x10, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, 
        { 0x10, 0x20, 0x40, 0x50, 0x00, 0x00, 0x00, 0x00 }, 
        { 0x10, 0x20, 0x40, 0x50, 0x10, 0x00, 0x00, 0x00 }, 
        { 0x10, 0x20, 0x40, 0x50, 0x20, 0x00, 0x00, 0x00 }, 
        { 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11 }, 
        { 0x11, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, 
        { 0x11, 0x12, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00 }, 
        { 0x11, 0x21, 0x41, 0x51, 0x11, 0x00, 0x00, 0x00 }, 
        { 0x11, 0x21, 0x41, 0x51, 0x21, 0x00, 0x00, 0x00 }, 
        { 0x11, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, 
        { 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12 }, 
        { 0x12, 0x22, 0x42, 0x52, 0x12, 0x00, 0x00, 0x00 }, 
        { 0x12, 0x22, 0x42, 0x52, 0x22, 0x00, 0x00, 0x00 }, 
        { 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13 }, 
        { 0x13, 0x23, 0x43, 0x53, 0x13, 0x00, 0x00, 0x00 }, 
        { 0x13, 0x23, 0x43, 0x53, 0x23, 0x00, 0x00, 0x00 }, 
        { 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20 }, 
        { 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21 }, 
        { 0x21, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, 
        { 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23 }, 
        { 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30 }, 
        { 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31 }, 
        { 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32 }, 
        { 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33 }
    };
    static const uint8_t ext_index[20][8] = {
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x05, 0x02 },
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x05, 0x03 },
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x1B, 0x07 },
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x1B, 0x08 },
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x1B, 0x09 },
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x1B, 0x0A },
        { 0x01, 0x00, 0x01, 0x01, 0x01, 0x0B, 0x1C, 0x0E },
        { 0x01, 0x01, 0x01, 0x01, 0x01, 0x0B, 0x1C, 0x0E },
        { 0x01, 0x01, 0x01, 0x01, 0x01, 0x0B, 0x1C, 0x0F },
        { 0x04, 0x00, 0x04, 0x04, 0x04, 0x11, 0x1D, 0x12 },
        { 0x04, 0x04, 0x04, 0x04, 0x04, 0x11, 0x1D, 0x12 },
        { 0x04, 0x04, 0x04, 0x04, 0x04, 0x11, 0x1D, 0x13 },
        { 0x05, 0x00, 0x05, 0x05, 0x05, 0x14, 0x1E, 0x15 },
        { 0x05, 0x05, 0x05, 0x05, 0x05, 0x14, 0x1E, 0x15 },
        { 0x05, 0x05, 0x05, 0x05, 0x05, 0x14, 0x1E, 0x16 },
        { 0x06, 0x00, 0x06, 0x06, 0x06, 0x0B, 0x14, 0x0C },
        { 0x06, 0x00, 0x06, 0x06, 0x06, 0x0B, 0x14, 0x0D },
        { 0x06, 0x00, 0x06, 0x06, 0x06, 0x0B, 0x14, 0x10 },
        { 0x17, 0x00, 0x17, 0x17, 0x17, 0x18, 0x1A, 0x19 },
    };    
    static const uint8_t ext_index2[26][8] = {
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, 
        { 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01 }, 
        { 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01 }, 
        { 0x00, 0x00, 0x02, 0x02, 0x00, 0x00, 0x03, 0x04 }, 
        { 0x00, 0x00, 0x04, 0x04, 0x00, 0x00, 0x03, 0x04 }, 
        { 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x03, 0x04 }, 
        { 0x01, 0x01, 0x01, 0x00, 0x01, 0x01, 0x01, 0x00 }, 
        { 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x03, 0x04 }, 
        { 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01 }, 
        { 0x01, 0x01, 0x01, 0x05, 0x00, 0x00, 0x03, 0x04 }, 
        { 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0x03, 0x04 }, 
        { 0x05, 0x00, 0x05, 0x05, 0x05, 0x07, 0x0D, 0x00 }, 
        { 0x05, 0x00, 0x05, 0x05, 0x05, 0x07, 0x0D, 0x07 }, 
        { 0x05, 0x00, 0x05, 0x05, 0x05, 0x07, 0x0D, 0x0A }, 
        { 0x05, 0x05, 0x05, 0x05, 0x05, 0x08, 0x0E, 0x00 }, 
        { 0x05, 0x05, 0x05, 0x05, 0x05, 0x08, 0x0E, 0x08 }, 
        { 0x05, 0x05, 0x05, 0x05, 0x05, 0x08, 0x0E, 0x0B }, 
        { 0x05, 0x05, 0x06, 0x05, 0x01, 0x01, 0x01, 0x05 }, 
        { 0x05, 0x05, 0x06, 0x05, 0x01, 0x01, 0x01, 0x06 }, 
        { 0x05, 0x05, 0x06, 0x05, 0x03, 0x03, 0x01, 0x06 }, 
        { 0x06, 0x06, 0x06, 0x06, 0x06, 0x09, 0x0F, 0x00 }, 
        { 0x06, 0x06, 0x06, 0x06, 0x06, 0x09, 0x0F, 0x09 }, 
        { 0x06, 0x06, 0x06, 0x06, 0x06, 0x09, 0x0F, 0x0C }, 
        { 0x10, 0x10, 0x13, 0x00, 0x01, 0x01, 0x01, 0x01 }, 
        { 0x11, 0x11, 0x13, 0x00, 0x01, 0x01, 0x01, 0x01 }, 
        { 0x12, 0x12, 0x12, 0x12, 0x01, 0x01, 0x01, 0x01 }
    };
    static const uint8_t ext_index3[128] = {
        0x18, 0x18, 0x17, 0x17, 0x19, 0x18, 0x17, 0x01,
        0x0C, 0x0D, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B,
        0x15, 0x16, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14,
        0x0F, 0x10, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E,
        0x07, 0x05, 0x09, 0x09, 0x0A, 0x07, 0x04, 0x03,
        0x06, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
        0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13,
        0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
        0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12,
        0x02, 0x02, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };

    uint8_t a = ext_index3[opcode >> 9];
    uint8_t b = ext_index2[a][(opcode >> 6) & 7];
    uint8_t c = ext_index[b][(opcode >> 3) & 7];
    uint8_t d = ext_modes[c][opcode & 7];
    return d;
}

__attribute__((naked,used,target("arm")))
void pjit_lookup(void) {
    pjit_jmp((uint16_t*)pjit_cache_reverse());
}

__attribute__((naked,used,target("arm")))
void pjit_bsr4p(void) {
    register uint32_t r1 asm("r1");
    r1 += pjit_cache_reverse();
    asm("add     r0, r0, #4");
    asm("str     r0, [sp, #-4]");
    pjit_jmp((uint16_t*)r1);
}

__attribute__((naked,used,target("arm")))
void pjit_bsr2p(void) {
    register uint32_t r1 asm("r1");
    r1 += pjit_cache_reverse();
    asm("add     r0, r0, #2");
    asm("str     r0, [sp, #-4]");
    pjit_jmp((uint16_t*)r1);
}

__attribute__((naked,used,target("arm")))
void pjit_bsr(void) {
    register uint32_t r1 asm("r1");
    r1 += pjit_cache_reverse();
    asm("str     r0, [sp, #-4]");
    pjit_jmp((uint16_t*)r1);
}

__attribute__((naked,used,target("arm")))
void pjit_bra(void) {
    register uint32_t r1 asm("r1");
    r1 += pjit_cache_reverse();
    asm("add     r0, r0, #2");
    asm("str     r0, [sp, #-4]");
    pjit_jmp((uint16_t*)r1);
}

__attribute__((naked,used,target("arm")))
void pjit_jsr4p(void) {
    register uint32_t r1 asm("r1");
    pjit_cache_reverse();
    asm("add     r0, r0, #4");
    asm("str     r0, [sp, #-4]");
    pjit_jmp((uint16_t*)r1);
}

__attribute__((naked,used,target("arm")))
void pjit_jsr2p(void) {
    register uint32_t r1 asm("r1");
    pjit_cache_reverse();
    asm("add     r0, r0, #2");
    asm("str     r0, [sp, #-4]");
    pjit_jmp((uint16_t*)r1);
}

__attribute__((naked,used,target("arm")))
void pjit_jsr(void) {
    register uint32_t r1 asm("r1");
    pjit_cache_reverse();
    asm("str     r0, [sp, #-4]");
    pjit_jmp((uint16_t*)r1);
}

__attribute__((naked,used,target("arm")))
void pjit_jmp(uint16_t *m68k_pc) {
    #define CACHE_PAGE_MASK ((2 << BLOCK_BITS) - 1)
    #define CACHE_PAGE_SAFE_LIMIT (CACHE_PAGE_MASK - 10)

    asm("ldrh    r1, [r5, %0]" :: "i"(__offsetof(cpu_t, sr)));
    asm("bic     r1, r1, #0xF");
    asm("orrcs    r1, r1, #0x1");
    asm("orrvs    r1, r1, #0x2");
    asm("orreq    r1, r1, #0x4");
    asm("orrmi    r1, r1, #0x8");
    asm("strh    r1, [r5, %0]" :: "i"(__offsetof(cpu_t, sr)));

    register uint32_t *out;
    {
        static uint32_t exec_buffer[8];
        register uint32_t *lr asm("lr");

        if(((uint32_t)m68k_pc) & 1) {
            asm("svc    %0" :: "i"(ADDRESSERR));
            // does not return
        }
        
        // are we cached or uncached (write to buffer or write to cache)
        // just read the MMU table to see how we should proceed
        // the C bit tells us if we're cacheable.
        extern uint32_t *mmu_table;
        if ((mmu_table[((uint32_t)m68k_pc) >> 20] & 0x4)
        && ((((uint32_t)m68k_pc) & CACHE_PAGE_MASK) < CACHE_PAGE_SAFE_LIMIT)) {
            // cache yea, but did we come from cache?
            if (lr >= cpu->cache_data) {
                // came from cache; adjust back to prior instruction
                lr -= 4;
            } else {
                // we did NOT come from cache, find out where we're going
                lr = cache_find_entry(m68k_pc);
            }
            out = lr;
            
        } else {
            // no cache, so point to our mini-buffer instead
            out = exec_buffer;
        }

        // get the opcode from 68K memory based on m68k_pc
        uint32_t opcode = *m68k_pc++;
        uint32_t i = 0;
                
        // determine opcode length (extension words)
        uint32_t ext = get_ext(opcode);
        
        // write out the extension word opcodes, incrementing m68k_pc			
        switch((ext_t)(ext & 0x0F)) {
        case EXT_I16:
            out[i++] = movw(r1, *m68k_pc++);				out[i++] = nop();
            break;
        case EXT_I32:
            out[i++] = movw(r1, m68k_pc[1]);				out[i++] = nop();
            out[i++] = movt(r1, m68k_pc[0]);				out[i++] = nop();
            m68k_pc += 2;
            break;
        case EXT_D8_XAREG:
            out[i++] = mov_signed(r1, (int8_t)opcode);
            out[i++] = emit_src_ext((uint32_t)out, *m68k_pc++);
            break;
        case EXT_D16_PC:
            out[i++] = movw(r1, m68k_pc[1]);
            out[i++] = emit_src_ext_pc((uint32_t)out, m68k_pc[0]);
            m68k_pc += 2;
            break;
        case EXT_D8_XPC:       
            out[i++] = mov_signed(r1, (int8_t)opcode);
            out[i++] = emit_src_ext_pc((uint32_t)out, *m68k_pc++);
        default:
            break;
        }

        switch((ext_t)(ext & 0x0F)) {
        case EXT_I16:
            out[i++] = movw(r2, *m68k_pc++);				out[i++] = nop();
            break;
        case EXT_I32:
            out[i++] = movw(r2, m68k_pc[1]);				out[i++] = nop();
            out[i++] = movt(r2, m68k_pc[0]);				out[i++] = nop();
            m68k_pc += 2;
            break;
        case EXT_D8_XAREG:
            out[i++] = mov_signed(r2, (int8_t)opcode);
            out[i++] = emit_dst_ext((uint32_t)out, *m68k_pc++);
            break;
        default:
            break;
        }
        
        cpu->pc = m68k_pc;
        
        // write out (copy) the opcode instruction
        // opcode fits entire in one or two instructions
        // We need to trust the emitters to align to the
        // no-branch-branch rule of instruction order

        uint32_t *fetch = cpu->opcode_table + opcode;
        // out[i++] = *fetch++;
        uint32_t op2 = *fetch;
        
        // is the instruction a branch, then fixup the offset
        if((op2 & 0xFF000000) == 0xEA000000) {
            op2 = op2 + (uint32_t)out - (uint32_t)fetch;
        }
        out[i++] = op2;
        
        // are we in cache mode?
        if (out == exec_buffer) {
            // no, write postable to buffer and call it			
            out[i++] = ldr(r0, r5, __offsetof(cpu_t, pc));
            // postamble should return to pjit_iagf
            out[i++] = b_imm(calc_offset(out, pjit_lookup));
        }
    }

    asm("ldrh    r1, [r5, %0]" :: "i"(__offsetof(cpu_t, sr)));
    asm("rbit    r1, r1");
    asm("bfi     r1, r1, #2, #2");
    asm("ror     r1, r1, #2");
    asm("msr     cpsr_f, r1");

	goto *out;
}
