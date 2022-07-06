/*
 * Copyright (c) 2020-2021 Renee Cousins, the Buffee Project
 * See pjit.h for details
 *
 * Contains the set-associative cache logic
 *
 */

#include "pjit.h"

//#define debug(...)
#define debug(...) printf(__VA_ARGS__)

static pjit_cache_t pjit_cache;
static pjit_tag_cache_t pjit_tag_cache;

#ifdef SET_ASSOCIATIVE
static uint8_t cache_lru[1 << PAGE_COUNT] = { 0 };
#endif

#define FN_PTR(X) ((void*)(int)X)

// wipe out a single cache page
#ifdef SET_ASSOCIATIVE
static void cache_clear(uint32_t set, uint32_t page) {
	uint32_t *mem = (uint32_t *)(&(*pjit_cache)[set][page]);
#else
static void cache_clear(uint32_t page) {
 	uint32_t *mem = (uint32_t *)(&(*pjit_cache)[page]);
#endif
	uint32_t opcode;
	for(int i=0; i<(1 << PAGE_SIZE); i++) {
		opcode = cpu_branch_offset(FN_PTR(cpu_lookup_nojit), mem);
//		if((i < 128) || (i >= ((1 << PAGE_SIZE)-128))) {
//			opcode = cpu_branch_offset(cpu_lookup_safe, mem);
//		} else {
//			opcode = cpu_branch_offset(cpu_lookup_inline, mem);
//		}
		*mem++ = opcode;
	}
}

// wipe out the entire cache
void cache_init(uint8_t* base) {
	pjit_cache = (pjit_cache_t)base;
	pjit_tag_cache = (pjit_tag_cache_t)(base + sizeof(*pjit_cache));

	debug("In cache_init.\n");
	debug("  cpu_lookup_safe at %p\n", FN_PTR(cpu_lookup_safe));
	debug("  cpu_lookup_inline at %p\n", FN_PTR(cpu_lookup_inline));
	debug("  clearing pages at %p:", base);
	for(int p = 0; p < (1 << PAGE_COUNT); p++) {
		//debug(" %d", p);
#ifdef SET_ASSOCIATIVE
		cache_clear(0, p);
		cache_clear(1, p);
#else
		cache_clear(p);
#endif
	}
	debug(" ... done\n");
}

// give a 68K address return the exact instruction to enter
// if the tags don't match, clear the cache first
uint32_t* cache_find_entry(uint32_t m68k_addr) {
	uint16_t index = cache_get_page_index(m68k_addr);
	uint16_t page = cache_get_page_select(m68k_addr);
	uint32_t tag = cache_get_tag(m68k_addr);
	
	debug("In cache_find_entry; m68k_addr=0x%04hX, index=%d, page=%d, tag=0x%04hX\n", (uint16_t)m68k_addr, index, page, (uint16_t)tag);
	
#ifdef SET_ASSOCIATIVE
	uint32_t set;
		 if((*pjit_tag_cache)[0][page] == tag) { set = 0; debug("Hit!\n"); }
	else if((*pjit_tag_cache)[1][page] == tag) { set = 1; debug("Hit!\n"); }
	else { // miss!
		set = 1 - cache_lru[page];
		(*pjit_tag_cache)[set][page] = tag;
		debug("Miss, using set %d and clearing page\n", set);
		cache_clear(set, page);
	}
	return &(*pjit_cache)[set][page][index];
#else
	if((*pjit_tag_cache)[page] != tag) {
		debug("Miss, clearing page\n");
		(*pjit_tag_cache)[page] = tag;
		cache_clear(page);
	}
	uint32_t *addr = &(*pjit_cache)[page][index];
	debug("Entering cache at %p", (void*)addr);
	debug(" (ARM op %08lX)\n", *addr);
	return addr;
#endif
}

// given a point within the PJIT cache, determine the 68K PC
// WARNING: this assumes that the cache_tag is valid and makes 
// no attempt to verify that it is not
uint32_t cache_reverse(uint32_t arm_addr) {
	debug("In cache_reverse, arm_addr=0x%08lX\n", arm_addr);
	
	arm_addr -= (uint32_t)pjit_cache;
	uint16_t index = (arm_addr >> 2) & ((1 << PAGE_SIZE) - 1);
	debug("  index=%d\n", index);
	uint16_t page = (arm_addr >> (2 + PAGE_SIZE)) & ((1 << PAGE_COUNT) - 1);
	debug("  page=%d\n", page);
	
#ifdef SET_ASSOCIATIVE
	uint32_t set = (arm_addr >> (2 + PAGE_SIZE + PAGE_COUNT)) & 1;
	return (*pjit_tag_cache)[set][page] | (index << 1);
#else
	uint32_t tag = (*pjit_tag_cache)[page];
	debug("  tag=%04hX\n", (uint16_t)tag);
	uint32_t addr = (tag << (1 + PAGE_SIZE)) | (index << 1);
	debug("  addr=%08lX\n", addr);
	return addr;
#endif
}




