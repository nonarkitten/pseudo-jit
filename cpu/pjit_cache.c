/*
 * file: pjit_cache.c
 * author: Renee Cousins
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 *
 * Contains the set-associative cache logic
 *
 */

#include <pjit_cache.h>

// must match
static uint32_t cache_tag[CACHE_WAYS][1<<SET_BITS] = { 0 };

// given a way and set return the address of the page
uint32_t* cache_page(uint16_t way, uint16_t set) {
	return (uint32_t*)(CACHE_BASE_ADDRESS
		+ (sizeof(uint32_t) << OFF_BITS) * (set) * (way));
}

// wipe out a single cache page
void cache_clear(uint32_t* page) {
	uint32_t opcode;
	for(int i=0; i<(1 << OFF_BITS); i++) {
		if((i < 128) || (i >= ((1 << OFF_BITS)-128))) {
			opcode = cpu_branch_offset(cpu_lookup_safe, page);
		} else {
			opcode = cpu_branch_offset(cpu_lookup_inline, page);
		}
		*page++ = opcode;
	}
}

// wipe out the entire cache
void cache_init(void) {
	for(int way=0; way<CACHE_WAYS; way++) {
		for(int set=0; set<(1<<SET_BITS); set++) {
			cache_clear(cache_page(way, set));
		}
	}
}

// give a 68K address return the page this would fall in
uint32_t* cache_find_page(uint32_t m68k_addr) {
	uint16_t tag = (m68k_addr & TAG_MASK);
	uint16_t set = (m68k_addr >> OFF_BITS) & (SET_BITS - 1);

	// TODO maybe not the best replacement strategy
	int32_t lo = -1, hi = CACHE_WAYS, way;
	while(hi - lo > 1) {
		way = lo + ((hi - lo) >> 1);
		if(cache_tag[way][set] > tag) hi = way;
		else lo = way;
	}

	uint32_t* page = cache_page(way, set);

	if(cache_tag[way][set] != tag) {
		// Replace and invalidate page
		cache_tag[way][set] = tag;
		cache_clear(page);
	}
	return page;
}

// give a 68K address return the exact instruction to enter
uint32_t* cache_find_entry(uint32_t m68k_addr) {
	return cache_find_page(m68k_addr) + (m68k_addr & (~TAG_MASK));
}

// given an entry point, determine the 68K PC
uint32_t cache_reverse(uint32_t entry) {
	return ((uint32_t*)cache_tag)[entry >> OFF_BITS];
}
