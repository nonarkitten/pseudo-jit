/*
 * file: pjit_cache.h
 * author: Renee Cousins
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 *
 * Header file for the set-associative cache logic
 *
 */

#ifndef __PJIT_CACHE_H__
#define __PJIT_CACHE_H__

#include <stdint.h>
#include "pjit_ops.h"

// Give the main PJIT about 4MB of room
// TODO: is this enough?
#define CACHE_BASE_ADDRESS 0x80400000

// cache size = 4 * CACHE_WAYS * (1 << (OFF_BITS + SET_BITS))
// must be <= 32MB minus overhead
#define CACHE_WAYS      14
#define OFF_BITS		11
#define SET_BITS		8
#define TAG_MASK		(~((1<<(OFF_BITS + SET_BITS))-1))

// given a way and set return the address of the page
extern uint32_t* cache_page(uint16_t way, uint16_t set);

// wipe out a single cache page
extern void cache_clear(uint32_t* page);

// wipe out the entire cache
extern void cache_init(void);

// give a 68K address return the page this would fall in
extern uint32_t* cache_find_page(uint32_t m68k_addr);

// give a 68K address return the exact instruction to enter
extern uint32_t* cache_find_entry(uint32_t m68k_addr);

// given an entry point, determine the 68K PC
extern uint32_t cache_reverse(uint32_t entry);

#endif /* __PJIT_OPS_H_ */


