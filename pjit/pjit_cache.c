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

config_t config = {
    // Default Settings
    0x704A4954,                  // must be 0x704A4954
    0,                           // must increment from last
    0,                           // must check rest of config
    0,                           // see cpu_feature_t
    post_enable_long_mem         // enable long memory test
        | post_enable_gpack_ok   // enable greenpak test
        | post_enable_checkclk   // enable clock measurement
        | post_enable_autommap,  // enable wuto memory-map
    1000,                        // dpll_mul, CPU clock = 24 * dpll_mul / dpll_div
    24,                          // dpll_div;
    135,                         // pmic_voltage in decivolts, Recommend 1.35V (135) for 1GHz operation
    0,                           // dcache_mask_24b, 1 to allow data caching of region
    0,                           // icache_mask_24b, 1 to allow instruction caching
    11,                          // cache_index_bits, Cache size = 8 << (cache_index_bits + cache_block_bits)
    7,                           // cache_block_bits
    0xFF,                        // MapROM page from 24-bit RAM (single 512KB), 0xFF to disable
};

cpu_t cpu_state = {0};

pjit_cache_init(uint32_t top) {
    // TOP OF RAM
    uint8_t* memory = (uint8_t*)top;

    // 2MB*     PJIT Instruction Cache  cpu->cache_data
    //          Actual size is 8 << (cache_index_bits + cache_block_bits)
    //                                  config->cache_index_bits (11 default)
    //                                  config->cache_block_bits (7 default)
    //                                  bits must be >= 16 total
    uint32_t data_cache_size = 8 << (config->cache_index_bits + config->cache_block_bits);
    memory -= data_cache_size;
    cpu->cache_data = (uint32_t*)memory;

    // 2MB*     PJIT MapROM Cache       cpu->maprom_data
    //          (optional)              config->maprom_page (00-1F, FF disabled)
    uint32_t maprom_size = (config->maprom_page != 0xFF) ? 0x200000 : 0;
    memory -= data_cache_size;
    cpu->maprom_data = (uint32_t*)memory;

    // 512KB*   PJIT Opcode Table
    //
    memory -= 0x80000;
    cpu->opcode_table = (uint32_t*)memory;

    // 256KB    PJIT Opcode Stubs       should be smaller, but just in case
    //
    memory -= 0x40000;
    cpu->opcode_stubs = (uint32_t*)memory;

    // 128KB    PJIT Core
    //
    memory -= 0x20000;
    cpu->pjit_core = (void*)memory;

    // 4KB      PJIT Cache Tags         cpu->cache_tags
    //          Size is 2 << cache_index_bits
    //
    uint32_t cache_tags_size = 2 << config->cache_index_bits;
    memory -= cache_tags_size;
    cpu->cache_tags = (uint16_t*)memory;

    // Align downwards to a 1MB boundary, this is top of RAM for
    // AmigaOS and Ataro
    memory           = (uint8_t*)(((uint32_t)memory) & 0xFFF00000);
    cpu->free_memory = (memory - 0x80000000) >> 20;

    // Clear all of this
    memset(memory, 0, (uint8_t*)0xA0000000 - memory);

    // Clear CPU state
    cpu->b_lookup = emit_branch(cpu, &lookup_opcode);

    // Clear caches
    cache_clear();

    // Load our initial PC and SP
    cpu->a7 = *(uint32_t*)0;
    cpu->pc = *(uint32_t*)4;
}