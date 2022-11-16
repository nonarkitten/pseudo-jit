/*
 * Copyright (c) 2020-2021 Renee Cousins, the Buffee Project
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
 * Castaway (formerly FAST)
 * Copyright (c) 1994-2002 Martin Doering, Joachim Hˆnig
 *
 * Cyclone 68K
 * Copyright (c) 2004,2011 FinalDave
 * Copyright (c) 2005-2011 Graûvydas "notaz" Ignotas
 *
 * Both of which were distributed under GPL version 2.
 * 
 */

#include "pjit.h"

#define PJIT_CACHE_SIZE ((1<<SET_BITS) * (1 << PAGE_COUNT) * (1 << PAGE_SIZE))
#define PJIT_TAG_SIZE   ((1<<SET_BITS) * (1 << PAGE_COUNT))

static uint8_t pjit_cache_lru[1 << PAGE_COUNT] = { 0 };

__attribute__ ((aligned (1 << PAGE_COUNT)))
static uint32_t pjit_tag_cache[1<<SET_BITS][1 << PAGE_COUNT] = { 0 };

__attribute__ ((aligned (1 << PAGE_SIZE)))
static uint32_t pjit_cache[1<<SET_BITS][1 << PAGE_COUNT][1 << PAGE_SIZE] = { 0 };

extern const uint32_t optab[65536];
extern const uint16_t oplen[65536];

static void cpu_lookup_inline(void);
static void cpu_lookup_noinline(void);
static void cpu_lookup_nojit(void);

/*   ____          _        _____           _ _   _                
    / ___|___   __| | ___  | ____|_ __ ___ (_) |_| |_ ___ _ __ ___ 
   | |   / _ \ / _` |/ _ \ |  _| | '_ ` _ \| | __| __/ _ \ '__/ __|
   | |__| (_) | (_| |  __/ | |___| | | | | | | |_| ||  __/ |  \__ \
    \____\___/ \__,_|\___| |_____|_| |_| |_|_|\__|\__\___|_|  |___/ */

// save and restore the CPU registers
static inline void save_cpu(void) {
    asm __volatile("str\tr3, [%0,%1]" :: "r"(cpu),"i"(OFFSETOF(cpu_t,d0)));
    asm __volatile("str\tr4, [%0,%1]" :: "r"(cpu),"i"(OFFSETOF(cpu_t,d1)));
    uint32_t* base = &cpu->a0;
    asm __volatile("stm\t%0!, {r6-r13}" :: "r"(base));
}
static inline void restore_cpu(void) {
    asm __volatile("ldr\tr3, [%0,%1]" :: "r"(cpu),"i"(OFFSETOF(cpu_t,d0))); // + 1 cycle
    asm __volatile("ldr\tr4, [%0,%1]" :: "r"(cpu),"i"(OFFSETOF(cpu_t,d1))); // + 1 cycle
    uint32_t* base = &cpu->a0;                                              // + 1 cycle
    asm __volatile("ldm\t%0!, {r6-r13}" :: "r"(base));                      // + 4 cycles
}

// return ARM32 opcodes for various instructions
static inline uint32_t emit_cc_branch(uint8_t cc, int32_t offset) {
	return (cc << 24) | (0x00FFFFFF & (offset >> 2));
}
static inline uint32_t emit_branch(uint32_t current, uint32_t target) {
    return emit_cc_branch(0xEB, target - current - 8);
}
static inline uint32_t emit_movw(uint32_t reg, uint16_t value) {
	return 0xE3000000 | ((value & 0xF000) << 4) | (reg << 12) | (value & 0xFFF);
}
static inline uint32_t emit_movt(uint32_t reg, uint16_t value) {
	return 0xE3400000 | ((value & 0xF000) << 4) | (reg << 12) | (value & 0xFFF);
}
static inline uint32_t emit_src_ext(uint32_t current, uint16_t opcode) {
	static const uint32_t ext_handler[32] = {
		(uint32_t)extword_src_0000, (uint32_t)extword_src_1000, (uint32_t)extword_src_2000, (uint32_t)extword_src_3000, 
		(uint32_t)extword_src_4000, (uint32_t)extword_src_5000, (uint32_t)extword_src_6000, (uint32_t)extword_src_7000, 
		(uint32_t)extword_src_8000, (uint32_t)extword_src_9000, (uint32_t)extword_src_A000, (uint32_t)extword_src_B000, 
		(uint32_t)extword_src_C000, (uint32_t)extword_src_D000, (uint32_t)extword_src_E000, (uint32_t)extword_src_F000, 
		(uint32_t)extword_src_0800, (uint32_t)extword_src_1800, (uint32_t)extword_src_2800, (uint32_t)extword_src_3800, 
		(uint32_t)extword_src_4800, (uint32_t)extword_src_5800, (uint32_t)extword_src_6800, (uint32_t)extword_src_7800, 
		(uint32_t)extword_src_8800, (uint32_t)extword_src_9800, (uint32_t)extword_src_A800, (uint32_t)extword_src_B800, 
		(uint32_t)extword_src_C800, (uint32_t)extword_src_D800, (uint32_t)extword_src_E800, (uint32_t)extword_src_F800, 	
	};    
	return emit_branch(current, ext_handler[opcode >> 11] + ((opcode & 0xFF) * 8));
}
static inline uint32_t emit_dst_ext(uint32_t current, uint16_t opcode) {
	static const uint32_t ext_handler[32] = {
		(uint32_t)extword_dst_0000, (uint32_t)extword_dst_1000, (uint32_t)extword_dst_2000, (uint32_t)extword_dst_3000, 
		(uint32_t)extword_dst_4000, (uint32_t)extword_dst_5000, (uint32_t)extword_dst_6000, (uint32_t)extword_dst_7000, 
		(uint32_t)extword_dst_8000, (uint32_t)extword_dst_9000, (uint32_t)extword_dst_A000, (uint32_t)extword_dst_B000, 
		(uint32_t)extword_dst_C000, (uint32_t)extword_dst_D000, (uint32_t)extword_dst_E000, (uint32_t)extword_dst_F000, 
		(uint32_t)extword_dst_0800, (uint32_t)extword_dst_1800, (uint32_t)extword_dst_2800, (uint32_t)extword_dst_3800, 
		(uint32_t)extword_dst_4800, (uint32_t)extword_dst_5800, (uint32_t)extword_dst_6800, (uint32_t)extword_dst_7800, 
		(uint32_t)extword_dst_8800, (uint32_t)extword_dst_9800, (uint32_t)extword_dst_A800, (uint32_t)extword_dst_B800, 
		(uint32_t)extword_dst_C800, (uint32_t)extword_dst_D800, (uint32_t)extword_dst_E800, (uint32_t)extword_dst_F800, 	
	};
	return emit_branch(current, ext_handler[opcode >> 11] + ((opcode & 0xFF) * 8));
}
static inline uint32_t emit_return(void) {
	return 0xE12FFF1E;
}

/*
      ____           _            _   _                 _ _ _             
     / ___|__ _  ___| |__   ___  | | | | __ _ _ __   __| | (_)_ __   __ _ 
    | |   / _` |/ __| '_ \ / _ \ | |_| |/ _` | '_ \ / _` | | | '_ \ / _` |
    | |__| (_| | (__| | | |  __/ |  _  | (_| | | | | (_| | | | | | | (_| |
     \____\__,_|\___|_| |_|\___| |_| |_|\__,_|_| |_|\__,_|_|_|_| |_|\__, |
                                                                    |___/ 

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

/*  Given a point within the PJIT cache, determine the 68K PC
    WARNING: this assumes that the cache_tag is valid and makes 
    no attempt to verify that it is not */
static inline uint32_t cache_reverse(uint32_t arm_addr) {
	//arm_addr -= (uint32_t)pjit_cache;
	uint16_t index = (arm_addr >> 2) & ((1 << PAGE_SIZE) - 1);
	uint16_t page = (arm_addr >> (2 + PAGE_SIZE)) & ((1 << PAGE_COUNT) - 1);	
    uint32_t set = (arm_addr >> (2 + PAGE_SIZE + PAGE_COUNT)) & ((1<<SET_BITS) - 1);
    return pjit_tag_cache[set][page] | (index << 1);
}

/*  Wipe out a single cache page */
static void cache_clear(uint32_t set, uint32_t page) {
	uint32_t *mem = pjit_cache[set][page];
	for(int i=0; i<128; i++) {
        *mem = emit_branch((uint32_t)mem, (uint32_t)cpu_lookup_noinline);
        mem += 1;
    }
    for(int i=0; i<(PAGE_SIZE - 256 - 10); i++) {
        *mem = emit_branch((uint32_t)mem, (uint32_t)cpu_lookup_inline);
        mem += 1;
    }
	for(int i=0; i<(128-10); i++) {
        *mem = emit_branch((uint32_t)mem, (uint32_t)cpu_lookup_noinline);
        mem += 1;
    }
	for(int i=0; i<10; i++) {
        *mem = emit_branch((uint32_t)mem, (uint32_t)cpu_lookup_nojit);
        mem += 1;
    }
}

/*  Given a 68K address return the exact instruction to enter
    if the tags don't match, clear the cache first */
static uint32_t* cache_find_entry(uint32_t m68k_addr) {
	uint16_t index = (m68k_addr >> 1) & ((1 << PAGE_SIZE) - 1);
	uint32_t tag = m68k_addr >> (1 + PAGE_SIZE);
	uint16_t page = tag & ((1 << PAGE_COUNT) - 1);
		
	uint32_t set, found = 0;
    for(int i=0; i<(1<<SET_BITS); i++) {
        if(pjit_tag_cache[i][page] == tag) { set = i; found = 1; break; }
    }
    if(!found) {
		set = (pjit_cache_lru[page] + 1) & ((1<<SET_BITS) - 1);
		pjit_tag_cache[set][page] = tag;
		cache_clear(set, page);
    }
	return &pjit_cache[set][page][index];
}

/*  Look up code and if it's small enough, replace the caller with the
    function body, otherwise, replace with the branch to it and then execute it */
__attribute__((naked)) void cpu_lookup_inline(void) {
	static const uint8_t arm_bcc[16] = {
		//       M68K OP Description    ARMcc
		0xEB, // 0000 T  Always         1110
		0xFF, // 0001 SR Subroutine     emulate
		0xFF, // 0010 HI Higher         emulate
		0xFF, // 0011 LS Lower/Same     emulate
		0x3B, // 0100 CC Carry Clear    0011
		0x2B, // 0101 CS Carry Set      0010
		0x1B, // 0110 NE Not Equal      0001
		0x0B, // 0111 EQ Equal          0000
		0x7B, // 1000 VC Overflow Clear 0111
		0x6B, // 1001 VS Overflow Set   0110
		0x5B, // 1010 PL Plus           0101
		0x4B, // 1011 MI Minus          0100
		0xAB, // 1100 GE Greater/Equal  1010
		0xBB, // 1101 LT Lesser         1011
		0xCB, // 1110 GT Greater        1100
		0xDB, // 1111 LE Lesser/Equal   1101
	};
	register uint32_t *out asm("lr");

    save_cpu();

    /* Back step to the original branch and calculate our 68K Program Counter */
	uint16_t *pc = (uint16_t*)cache_reverse((uint32_t)--out);

	uint32_t i = 0;
	uint32_t opcode = pc[i++];
	uint32_t opaddr = optab[opcode];
	uint32_t opea = oplen[opcode];

    /* Handle Extension Words */
    if(opea & EXT_WORD_SRC_EXT) { out[i] = emit_src_ext((uint32_t)out, pc[i]); i += 1; }
    else {
        if(opea & EXT_WORD_SRC_16B) { out[i] = emit_movw(1, pc[i]); i += 1; }
        if(opea & EXT_WORD_SRC_32B) { out[i] = emit_movt(1, pc[i]); i += 1; }
    }

	if(opea & EXT_WORD_DST_EXT) { out[i] = emit_dst_ext((uint32_t)out, pc[i]); i += 1; }
    else {
        if(opea & EXT_WORD_DST_16B) { out[i] = emit_movw(2, pc[i]); i += 1; }
        if(opea & EXT_WORD_DST_32B) { out[i] = emit_movt(2, pc[i]); i += 1; }
    }

    uint32_t opcode_out = 0;

    if((opea & 0xFF) == 1) {
        /*  Can we inline? If so, then just copy it here.
            Note that the subroutine Bcc should NEVER report 1 line. */
        opcode_out = *(uint32_t*)opaddr;

    } else if((opcode & 0xF001) == 0x6000) {
        /*  Check if we can in-line Bcc
            1. we cannot BSR, BHI or BLS, those need to be handled
            2. we cannot inline branch if the offset if out-of-page */
        uint32_t cc = arm_bcc[(opcode & 0x0F00) >> 8];
        if(cc != 0xFF) opcode_out = emit_cc_branch(cc, (uint32_t)(int32_t)(int8_t)(opcode & 0xFE));
    }
    if(opcode_out) out[i] = opcode_out;
    else out[i] = emit_branch((uint32_t)out, opaddr);

    restore_cpu();
    asm("bx\t%0" :: "r"(out));
}

/*  Look up code and if it's small enough and replace our
    branch to jump to the opcode; this omits branch inlining. We use
    this version to pad the 'edges' of each page to avoid having to
    check if our 8-bit branch is outside of the page.  */
__attribute__((naked)) void cpu_lookup_noinline(void) {
	register uint32_t *out asm("lr");

    save_cpu();

    /* Back step to the original branch and calculate our 68K Program Counter */
	uint16_t *pc = (uint16_t*)cache_reverse((uint32_t)--out);

	uint32_t i = 0;
	uint32_t opcode = pc[i++];
	uint32_t opaddr = optab[opcode];
	uint32_t opea = oplen[opcode];

    /* Handle Extension Words */
    if(opea & EXT_WORD_SRC_EXT) { out[i] = emit_src_ext((uint32_t)out, pc[i]); i += 1; }
    else {
        if(opea & EXT_WORD_SRC_16B) { out[i] = emit_movw(1, pc[i]); i += 1; }
        if(opea & EXT_WORD_SRC_32B) { out[i] = emit_movt(1, pc[i]); i += 1; }
    }

	if(opea & EXT_WORD_DST_EXT) { out[i] = emit_dst_ext((uint32_t)out, pc[i]); i += 1; }
    else {
        if(opea & EXT_WORD_DST_16B) { out[i] = emit_movw(2, pc[i]); i += 1; }
        if(opea & EXT_WORD_DST_32B) { out[i] = emit_movt(2, pc[i]); i += 1; }
    }

    /*  Can we inline? If so, then just copy it here */
    if((opea & 0xFF) == 1) out[i] = *(uint32_t*)opaddr;
    else out[i] = emit_branch((uint32_t)out, opaddr);

    restore_cpu();
    asm("bx\t%0" :: "r"(out));
}

/*  Look up code and execute it. This version will never replace the calling branch.
    This is not the most efficient way to do this, since it needs to waste a whole
    cache page. FIXME */
__attribute__((naked)) void cpu_lookup_nojit(void) {
	register uint32_t *entry asm("lr");
    static uint32_t out[10];

    save_cpu();

    /* Back step to the original branch and calculate our 68K Program Counter */
	uint16_t *pc = (uint16_t*)cache_reverse((uint32_t)(entry - 1));

	uint32_t i = 0;
	uint32_t opcode = pc[i++];
	uint32_t opaddr = optab[opcode];
	uint32_t opea = oplen[opcode];
    
    /* Handle Extension Words */
    if(opea & EXT_WORD_SRC_EXT) { out[i] = emit_src_ext((uint32_t)out, pc[i]); i += 1; }
    else {
        if(opea & EXT_WORD_SRC_16B) { out[i] = emit_movw(1, pc[i]); i += 1; }
        if(opea & EXT_WORD_SRC_32B) { out[i] = emit_movt(1, pc[i]); i += 1; }
    }

	if(opea & EXT_WORD_DST_EXT) { out[i] = emit_dst_ext((uint32_t)out, pc[i]); i += 1; }
    else {
        if(opea & EXT_WORD_DST_16B) { out[i] = emit_movw(2, pc[i]); i += 1; }
        if(opea & EXT_WORD_DST_32B) { out[i] = emit_movt(2, pc[i]); i += 1; }
    }

    /*  Can we inline? If so, then just copy it here */
    if((opea & 0xFF) == 1) out[i++] = *(uint32_t*)opaddr;
    else out[i++] = emit_branch((uint32_t)out, opaddr);
    out[i++] = emit_return();

    restore_cpu();
    asm("bx\t%0" :: "r"(out));
}

/*  These are place holders for now, this code should be part of the handler. */
void cpu_jump(uint32_t m68k_pc) {}
void cpu_subroutine(uint32_t _lr, uint32_t new_m68k_pc) {}
void relative_branch(uint32_t _lr, int32_t offset) {}
void branch_subroutine(uint32_t _lr, int32_t offset) {}

/*  Start PJIT */
void cpu_start(void* base) {
    static cpu_t cpu_state;
	register uint32_t *out asm("lr");
    uint32_t* b = (uint32_t*)base;

    cpu = &cpu_state;
    memset(cpu, 0, sizeof(cpu_t));

    cpu->a7 = b[4];    
    out = cache_find_entry(b[0]);

    restore_cpu();
    asm("bx\t%0" :: "r"(out));
}