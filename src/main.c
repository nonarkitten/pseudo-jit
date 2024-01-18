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

#include "main.h"

const char* banner =
"              ____ ______________\n"
"  ______     |    |   \\__    ___/\n"
"  \\  __ \\    |    |   | |    |\n"
"  |  |_) )\\__|    |   | |    |\n"
"  |   __/\\________|___| |____|\n"
"  |__|\n\n";

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

#define MB * 1024 * 1024

cpu_t cpu_state = {0};

int main(int argc, char *argv[]) {
    void *cache=0, *opcodes=0;
    int testrom=0, tinbasic=0, dhrystone=0, loadhunk=0;
    const char *hunk=0;

    cpu_state.config = default_config;

    while ((c = getopt (argc, argv, "rbdh:")) != -1) {
        switch (c) {
        case 'r': testrom = 1; break;
        case 'b': tinbasic = 1; break;
        case 'd': dhrystone = 1; break;
        case 'h': loadhunk = 1; hunk = optarg; break;
    }

    if(!(testrom|tinbasic|dhrystone|loadhunk)
    || ((testrom+tinbasic+dhrystone+loadhunk) > 1)
    || (loadhunk && !hunk)) {
        printf( "Usage:\n"
                "  r        Run built-in test ROM image\n"
                "  b        Run built-in Tiny BASIC\n"
                "  d        Run built-in Dhrystone test\n"
                "  h <file> Load and run specified hunk file\n");
        return 1;
    }
    
    cache = malloc(2 MB);
    printf("[BOOT] Initializing cache\n");
    pjit_cache_init(cache);

    void* opcodes = malloc(2 MB);
    printf("[BOOT] Initializing opcode tables\n");
    emit_opcode_table(opcodes);

    void* pjit = malloc(1 MB);
    printf("[BOOT] Starting PJIT\n");
    if(testrom) memcpy(pjit, m68kcode, sizeof(m68kcode)); 
    else if(tinbasic) memcpy(pjit, tiny_BASIC, sizeof(tiny_BASIC));
    else if(dhrystone) LoadHunkFile(pjit, dhrystone);
    else {
		char *read_to = (char*)pjit;;
		FILE *ptr_hunk;
		struct rec my_record;

		if (!(ptr_hunk=fopen(hunk,"rb"))) {
			fprintf(stderr, "Unable to open file %s!", hunk);
			return 1;
		}
        fseek(ptr_hunk, 0, SEEK_END);
        size_t len = fetll(ptr_hunk);
        void *h = malloc(len);
		size_t read = fread(read_to, 1, len, ptr_hunk);
		fclose(ptr_myfptr_hunkile);

        if(len == read) LoadHunkFile(pjit, h);
        free(h);
        if(len != read) {
			fprintf(stderr, "Unable to open file %s!", hunk);
			return 1;
		}
    }
    pjit_start(pjit);

}