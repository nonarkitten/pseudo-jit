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

/*
	EXPLANATION
 
    The PJIT cache is scalable from 256KB to 32MB in powers of 2 or a number of
    bits. A 256KB cache is 18 bits and 32MB is 25 bits. 
    
    The cache is divided into a number of equal sized pages. Smaller pages will
    provide for more total pages improving the cache-hit ratio. Larger pages
    reduce the chance that a branch will cross a page boundary. The default page
    size is 2048KB or 512 ARM instructions equivalent to 1KB of 68K program memory.
    In terms of bits, this is 10 bits for either 68K or ARM; in ARM it would be
    10-bits with 2 offset bits and in 68K it would be 10-bits with 1 offset bit.
    
    The cache is cleared with three look up routines. The default routine that
    uses most of the cache will always assume that branches are safe within the
    page and will always inline routines. At each border of the pages, within
    +128 or -126 bytes, the cache clear will use a look up routine that will still
    inline all instructions except for branches as they are no longer "safe".
     
	The tag store has a size which is determined by the size of the PJIT cache
	divided by the size of the page. For example, a 1MB PJIT cache with a 2KB page
	size will contain 512 pages. The tag itself is the high bits of the original
	address, not including the page bits and offset bits (1 in 68K and 2 in ARM),
	to a maximum size of 16-bits. This the tag store would take 1KB.
	
	The mapping between a 68K instruction and the PJIT cache is by the 68K 
	program counter (PC). For example, with our 1MB PJIT cache and corresponding
	1KB tag cache store, the least significant bit is first discarded, the next 
	10 bits comprise the Page Index into the page and the next 10 bits comprise the
	Page Selector.

	Each long word in the PJIT cache corresponds to one word of 68K instruction
	memory thus the Index must be shifted left by one bit when performing the Index.

	The remaining most significant bits comprise the tag. This is checked with 
	the tag store before entry. If the tags match, then execution into the 
	PJIT cache can begin immediately. If the tags do not match, then the page 
	is cleared first before executing into the PJIT cache.
*/

#include <arm_neon.h>

#include "arm.h"
#include "pjit.h"

const config_t default_config = {
    // Default Settings
    0x704A4954,                  // must be 0x704A4954
    0,                           // must check rest of config
    // CPU Features
    { 0, 0, 0, 0, 0, 0, 1, 1, 0 },
    // POST Enable
    { 1, 1, 1, 1 }, 
    1000,                        // dpll_mul, CPU clock = 24 * dpll_mul / dpll_div
    135,                         // pmic_voltage in decivolts, Recommend 1.35V (135) for 1GHz operation
    0,                           // dcache_mask_24b, 1 to allow data caching of region
    0,                           // icache_mask_24b, 1 to allow instruction caching
    0,                           // last boot good
    11,                          // cache_index_bits, Cache size = 8 << (cache_index_bits + cache_block_bits)
    7,                           // cache_block_bits
    0xFF,                        // MapROM page from 24-bit RAM (single 512KB), 0xFF to disable
    8000,                        // Default kHz
    0,                           // not dirty
};

cpu_t cpu_state = {0};
extern void lookup_opcode(void);

// ttttttttttt mmmmmmmmmm nnnnnnnnnn 0 M68k source addresses
// xxxxxxxxxx mmmmmmmmmm nnnnnnnnnn 00 ARM cache addresses (1 instruction)
// xxxxxxxxx mmmmmmmmmm nnnnnnnnnn 000 ARM cache addresses (2 instructions)
// xxxxxxxx mmmmmmmmmm nnnnnnnnnn 0000 ARM cache addresses (4 instructions)
// --------                            tag
//          ----------                 block_index
//                     ----------      block_offset

#define ARM_INSTRUCTIONS 2

#define CACHE_TAG_BITS (CACHE_INDEX_BITS + CACHE_OFFSET_BITS)
#define CACHE_IDX_MASK (((1 << CACHE_INDEX_BITS) - 1) << 1)
#define CACHE_OFF_MASK (((1 << CACHE_TAG_BITS) - 1) << 1)

static inline uint32_t cache_get_tag(uint32_t m68k_addr) { // t
	return m68k_addr >> CACHE_TAG_BITS;
}

static inline uint32_t cache_get_index(uint32_t m68k_addr) { // m
    return (m68k_addr >> CACHE_OFFSET_BITS) & CACHE_IDX_MASK;
}

static inline uint32_t cache_get_offset(uint32_t m68k_addr) { // n + m;
    return m68k_addr & CACHE_OFF_MASK;
}

register union { double f; uint32x2_t LL; } FPa asm("d16");
register union { double f; uint32x2_t LL; } FPb asm("d17");

// give a 68K address return the exact instruction to enter
// if the tags don't match, clear the cache first
uint32_t cache_find_entry(uint32_t m68k) {
	uint32_t tag = cache_get_tag(m68k);
	uint32_t off = cache_get_offset(m68k);
	uint32_t idx = cache_get_index(m68k);

    if(cpu->cache_tags[idx] != tag) {
        cpu->cache_tags[idx] = tag;
        idx <<= CACHE_OFFSET_BITS;
        FPa.LL = (uint32x2_t){ 0xE2800002, 0xE12FFF35 };
        uint32x2_t *mem = (uint32x2_t *)(cpu->cache_data + idx);
        uint32x2_t *end = (uint32x2_t *)(cpu->cache_data + idx + (1 << CACHE_OFFSET_BITS));
        while(mem < end) {
            *mem++ = FPa.LL; *mem++ = FPa.LL; *mem++ = FPa.LL; *mem++ = FPa.LL;
        }
    }
    return (uint32_t)(cpu->cache_data + (off * ARM_INSTRUCTIONS));
}

/*  Given a point within the PJIT cache, determine the 68K PC
    WARNING: this assumes that the cache_tag is valid and makes
    no attempt to verify that it is not */
uint32_t cache_reverse(uint32_t arm_addr) {    
    arm_addr -= (uint32_t)cpu->cache_data;
    arm_addr >>= ARM_INSTRUCTIONS;

	uint32_t idx = cache_get_index(arm_addr);
    uint32_t tag = cpu->cache_tags[idx];
	uint32_t off = cache_get_offset(arm_addr);

    return ((tag << CACHE_TAG_BITS) | off) << 1;
}

void pjit_cache_init(uint32_t top) {
    // TOP OF RAM
    uint8_t* memory = (uint8_t*)top;

    // 2MB*     PJIT Instruction Cache  cpu->cache_data
    //          Actual size is 8 << (cache_index_bits + cache_block_bits)
    //                                  config.cache_index_bits (11 default)
    //                                  config.cache_block_bits (7 default)
    //                                  bits must be >= 16 total
    uint32_t data_cache_size = 1 << (CACHE_INDEX_BITS + CACHE_OFFSET_BITS + ARM_INSTRUCTIONS);
    memory -= data_cache_size;
    cpu_state.cache_data = (uint32_t*)memory;

    // 2MB*     PJIT MapROM Cache       cpu->maprom_data
    //          (optional)              config.maprom_page (00-1F, FF disabled)
    uint32_t maprom_size = (cpu_state.config.maprom_page != 0xFF) ? 0x200000 : 0;
    memory -= data_cache_size;
    cpu_state.maprom_data = (uint32_t*)memory;

    // 512KB*   PJIT Opcode Table
    //
    memory -= 0x80000;
    cpu_state.opcode_table = (uint32_t*)memory;

    // 256KB    PJIT Opcode Stubs       should be smaller, but just in case
    //
    memory -= 0x40000;
    cpu_state.opcode_stubs = (uint32_t*)memory;

    // 128KB    PJIT Core
    //
    memory -= 0x20000;
    cpu_state.pjit_core = (void*)memory;

    // 4KB      PJIT Cache Tags         cpu->cache_tags
    //          Size is 2 << cache_index_bits
    //
    uint32_t cache_tags_size = 2 << CACHE_INDEX_BITS;
    memory -= cache_tags_size;
    cpu_state.cache_tags = (uint16_t*)memory;

    // Align downwards to a 1MB boundary, this is top of RAM for
    // AmigaOS and Ataro
    memory = (uint8_t*)(((uint32_t)memory) & 0xFFF00000);
    cpu_state.free_memory = ((uint32_t)memory - 0x80000000) >> 20;

    // Clear all of this
    memset(memory, 0, cpu_state.cache_data - memory);

    FPa.LL = (uint32x2_t){ 0xE2800002, 0xE12FFF35 };
    uint32x2_t *mem = (uint32x2_t *)(cpu_state.cache_data);
    uint32x2_t *end = (uint32x2_t *)(cpu_state.cache_data + data_cache_size);
    while(mem < end) {
        *mem++ = FPa.LL; *mem++ = FPa.LL; *mem++ = FPa.LL; *mem++ = FPa.LL;
    }

    // set branch CPU state
    cpu_state.b_lookup = b_imm(calc_offset(cpu, &lookup_opcode));

    // Clear caches
    cpu = &cpu_state;
    __builtin___clear_cache(memory, top);
}

int pjit_cache_test() {
    /*
        Setup a hypothetical -- we'll put here the first few instructions of Amiga ROM

        FC0002    0x4EF9    jmp FC00D2
        FC0004    0x00FC
        FC0006    0x00D2
        ...
        FC00D2    0x4FF9    lea       040000,SP
        FC00D4    0x0004
        FC00D6    0x0000

        FC00D8    0x203C    move.l    #$020000,D0
        FC00DA    0x0002
        FC00DC    0x0000

        FC00DE    0x5380    subq.l    #1,D0
        FC00E0    0x6EFC    bgt.s     FC00DE

        */

    

    return 0; // success!
}