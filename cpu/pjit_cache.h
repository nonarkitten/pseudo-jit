/*
 * Copyright (c) 2020-2021 Renee Cousins, the Buffee Project
 * See pjit.h for details
 *
 * Header file for the set-associative cache logic
 *
 *
 
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

#ifndef __PJIT_CACHE_H__
#define __PJIT_CACHE_H__

#include "pjit.h"

// If this is defined, we'll use a simple 2-way set association instead of direct
// mapped; this doubles the size of the PJIT cache so be forewarned!
//#define SET_ASSOCIATIVE

// must be <= 32MB minus overhead
#define PAGE_SIZE       10		/* 2 ^ 10 = 1KB PJIT PAGE SIZE */
#define PAGE_COUNT		10		/* 2 ^ 10 * 1KB = 1MB PJIT CACHE*/

#define CACHE_BASE_ADDRESS 0x80000000

static inline uint32_t cache_get_page_index(uint32_t m68k_addr) {
	return (m68k_addr >> 1) & ((1 << PAGE_SIZE) - 1);
}

static inline uint32_t cache_get_tag(uint32_t m68k_addr) {
	return (m68k_addr >> (1 + PAGE_SIZE));
	//return m68k_addr & ~((1 << PAGE_SIZE) - 1);
}

static inline uint16_t cache_get_page_select(uint32_t m68k_addr) {
	return cache_get_tag(m68k_addr) & ((1 << PAGE_COUNT) - 1);
}

#ifdef SET_ASSOCIATIVE
typedef uint32_t (*pjit_cache_t)[2][1 << PAGE_COUNT][1 << PAGE_SIZE];
typedef uint32_t (*pjit_tag_cache_t)[2][1 << PAGE_COUNT];
#define PJIT_CACHE_SIZE (2 * (1 << PAGE_COUNT) * (1 << PAGE_SIZE))
#define PJIT_TAG_SIZE   (2 * (1 << PAGE_COUNT))
#else
typedef uint32_t (*pjit_cache_t)[1 << PAGE_COUNT][1 << PAGE_SIZE];
typedef uint32_t (*pjit_tag_cache_t)[1 << PAGE_COUNT];
#define PJIT_CACHE_SIZE ((1 << PAGE_COUNT) * (1 << PAGE_SIZE))
#define PJIT_TAG_SIZE   ((1 << PAGE_COUNT))
#endif

// wipe out the entire cache
extern void cache_init(uint8_t* base);

// give a 68K address return the exact instruction to enter
// if the tags don't match, clear the cache first
extern uint32_t* cache_find_entry(uint32_t m68k_addr);

// given a point within the PJIT cache, determine the 68K PC
// WARNING: this assumes that the cache_tag is valid and makes 
// no attempt to verify that it is not
extern uint32_t cache_reverse(uint32_t arm_addr);

#endif /* __PJIT_OPS_H_ */


