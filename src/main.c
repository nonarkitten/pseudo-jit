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

#include <stdint.h>
#include <unistd.h>

#include "main.h"

#define MB * 1024 * 1024

extern const uint8_t tiny_BASIC[];
extern const int tiny_BASIC_size;

extern const uint8_t m68kcode[];
extern const int m68kcode_size;

extern const uint8_t _acdhrystone;
extern const int _acdhrystone_size;

extern const config_t default_config;

const char* banner =
"              ____ ______________\n"
"  ______     |    |   \\__    ___/\n"
"  \\  __ \\    |    |   | |    |\n"
"  |  |_) )\\__|    |   | |    |\n"
"  |   __/\\________|___| |____|\n"
"  |__|\n\n";

cpu_t cpu_state = {0};

int main(int argc, char *argv[]) {
    void *cache=0, *opcodes=0;
    int testrom=0, tinbasic=0, dhrystone=0, loadhunk=0;
    const char *hunk=0;
    int c;

    cpu_state.config = default_config;

    while ((c = getopt (argc, argv, "rbdh:")) != -1) {
        switch (c) {
            case 'r': testrom = 1; break;
            case 'b': tinbasic = 1; break;
            case 'd': dhrystone = 1; break;
            case 'h': loadhunk = 1; hunk = optarg; break;
        }
    }

    if(!(testrom|tinbasic|dhrystone|loadhunk)
    || ((testrom+tinbasic+dhrystone+loadhunk) > 1)
    || (loadhunk && !hunk)) {
        fprintf(stderr, "Usage:\n"
                        "  r        Run built-in test ROM image\n"
                        "  b        Run built-in Tiny BASIC\n"
                        "  d        Run built-in Dhrystone test\n"
                        "  h <file> Load and run specified hunk file\n");
        return 1;
    }
    
    fprintf(stderr, "%s\n", banner);

    void* pjit = malloc(1 MB);
    fprintf(stderr, "Starting PJIT\n");
    if(testrom) memcpy(pjit, m68kcode, m68kcode_size); 
    else if(tinbasic) memcpy(pjit, tiny_BASIC, tiny_BASIC_size);
    else if(dhrystone) LoadHunkFile(pjit, _acdhrystone);
    else /* loadhunk */ {
		char *read_to = (char*)pjit;;
		FILE *ptr_hunk;

		if (!(ptr_hunk=fopen(hunk,"rb"))) {
			fprintf(stderr, "Unable to open file %s!", hunk);
			return 1;
		}
        fseek(ptr_hunk, 0, SEEK_END);
        size_t len = fetll(ptr_hunk);
        void *h = malloc(len);
		size_t read = fread(h, 1, len, ptr_hunk);
		fclose(ptr_hunk);

        if(len == read) LoadHunkFile(pjit, h);
        free(h);
        if(len != read) {
			fprintf(stderr, "Unable to open file %s!", hunk);
			return 1;
		}
    }

    const size_t cache_size = 2 MB;
    cache = malloc(cache_size);
    fprintf(stderr, "Initializing cache\n");
    pjit_cache_init(cache + cache_size); // uses top/end
    if(!pjit_cache_test()) {
        fprintf(stderr, "One or more cache tests failed\n");
        return 1;
    }

    fprintf(stderr, "Initializing opcode tables\n");
    emit_opcode_table();

    // Load our initial PC and SP
    // cpu_state.a7 = *(uint32_t*)0;
    // cpu_state.pc = *(uint32_t*)4;
    cpu_start(pjit);
}