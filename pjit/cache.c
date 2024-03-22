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
    INDEX_BITS,                  // cache_index_bits, Cache size = 8 << (cache_index_bits + cache_block_bits)
    BLOCK_BITS,                  // cache_block_bits
    0xFF,                        // MapROM page from 24-bit RAM (single 512KB), 0xFF to disable
    8000,                        // Default kHz
    0,                           // not dirty
};

cpu_t cpu_state = {0};

static uint32_t * cache_clear_block(uint32_t* block) {
    uint32_t *end = (uint32_t)block + (1 << (BLOCK_BITS + CACHE_OP_BITS));
    while(block < end) {
        // *block++ = nop();
        *block++ = blx_reg(r5);
    }
    return end;
}

static void cache_clear(void) {
    uint32_t *start = cpu->cache_data;
    uint32_t *end = (uint32_t)start + (1 << (BLOCK_BITS + INDEX_BITS + CACHE_OP_BITS));
    while(start < end) start = cache_clear_block(start);
}

uint32_t cache_find_entry(uint32_t m68k) {
    // m68k address is composed of the following:

    // assuming 11 index bits and 7 block bits
    // tttt tttt tttt tiii iiii iiii bbbb bbb0

    // which corresponds to an offset into the pjit cache of
    // 0000 0000 0000 iiii iiii iiib bbbb bb00
    
	uint16_t tag = m68k >> (1 + BLOCK_BITS + INDEX_BITS);
	uint16_t index = (m68k >> (1 + BLOCK_BITS)) & ((1 << INDEX_BITS) - 1);
	uint16_t block = (m68k >> 1) & ((1 << BLOCK_BITS) - 1);
		
	if(cpu->cache_tags[index] != tag) {
		cpu->cache_tags[index] = tag;
		cache_clear_block(index << (CACHE_OP_BITS + BLOCK_BITS));
	}

	return (uint32_t)cpu->cache_data | (m68k & ((1 << (1 + BLOCK_BITS + INDEX_BITS)) - 1));
}

pjit_cache_init(uint32_t top) {
    // TOP OF RAM
    uint8_t* memory = (uint8_t*)top;

    // 2MB*     PJIT Instruction Cache  cpu->cache_data
    //          Actual size is 8 << (cache_index_bits + cache_block_bits)
    //                                  config.cache_index_bits (11 default)
    //                                  config.cache_block_bits (7 default)
    //                                  bits must be >= 16 total
    uint32_t data_cache_size = 1 << (CACHE_OP_BITS + INDEX_BITS + BLOCK_BITS);
    memory -= data_cache_size;
    cpu_state.cache_data = (uint32_t*)memory;
    printf("[INIT] Allocated %d bytes for cache at %p\n", data_cache_size, cpu_state.cache_data);

    // 2MB*     PJIT MapROM Cache       cpu->maprom_data
    //          (optional)              config.maprom_page (00-1F, FF disabled)
    uint32_t maprom_size = (cpu_state.config.maprom_page != 0xFF) ? 0x200000 : 0;
    memory -= data_cache_size;
    cpu_state.maprom_data = (uint32_t*)memory;
    printf("[INIT] Allocated %d bytes for maprom at %p\n", maprom_size, cpu_state.maprom_data);

    // 256KB    PJIT Opcode Table
    //
    const uint32_t table_size = 0x40000;
    memory -= table_size;
    cpu_state.opcode_table = (uint32_t*)memory;
    printf("[INIT] Allocated %d bytes for PJIT table at %p\n", table_size, cpu_state.opcode_table);

    // 2MB    PJIT Opcode Stubs
    //
    const uint32_t opcode_size = 0x200000;
    memory -= opcode_size;
    cpu_state.opcode_stubs = (uint32_t*)memory;
    printf("[INIT] Allocated %d bytes for PJIT table at %p\n", opcode_size, cpu_state.opcode_stubs);

    // 128KB    PJIT Core
    //
    // memory -= 0x20000;
    // cpu_state.pjit_core = (void*)memory;

    // 4KB      PJIT Cache Tags         cpu->cache_tags
    //          Size is 2 << cache_index_bits
    //
    uint32_t cache_tags_size = 2 << INDEX_BITS;
    memory -= cache_tags_size;
    cpu_state.cache_tags = (uint16_t*)memory;
    bzero(cpu_state.cache_tags, cache_tags_size);
    printf("[INIT] Allocated %d bytes for PJIT cache tags at %p\n", cache_tags_size, cpu_state.cache_tags);

    // Align downwards to a 1MB boundary, this is top of RAM for
    // AmigaOS and Ataro
    memory           = (uint8_t*)(((uint32_t)memory) & 0xFFF00000);
    cpu_state.free_memory = ((uint32_t)memory - 0x80000000) >> 20;
    printf("[INIT] Aligned memory base to %p, %dMB free memory\n", memory, cpu_state.free_memory);

    // Clear all of this
    // memset(memory, 0, (uint8_t*)0xA0000000 - memory);

    // Clear CPU state
    cpu_state.b_lookup = b_imm(calc_offset(&cpu_state, &pjit_lookup));

    cpu = &cpu_state;

    // Clear caches
    cache_clear();

}