#include "m68k_common.h"

#ifndef __REG_ALLOC_H__
#define __REG_ALLOC_H__

// Number of temporary ARM registers
#define REG_MAP_COUNT 3

// Register for CPU pointer
#define CPU "r5"

// REGISTER ALLOCATION
typedef enum { ALLOC_FAILED, ALLOC_OKAY } ALLOC_ERR_t;

typedef union {
	uint8_t raw;
	struct {
		uint8_t lock : 1; // is this register locked (reg_alloc_temp or reg_alloc_arm)
		uint8_t mod  : 1; // has this register been modified
		uint8_t size : 2; // size of the register, 1, 2 or 4
		uint8_t reg  : 4; // 68k register index
	};
} reg_map_t;

uint8_t reg_raw(uint8_t reg_68k);

bool reg_unalloced(uint8_t reg_arm);

// attempt to find any free register and return that
ALLOC_ERR_t reg_alloc_temp(uint8_t *reg_arm);

// allocate a specific 68k register, returning it's arm register used
// if a new temp register is needed, this will allocate it, but not load it
ALLOC_ERR_t reg_alloc_68k(uint8_t *reg_arm, uint8_t reg_68k, int size);

// allocate a specific arm register
ALLOC_ERR_t reg_alloc_arm(uint8_t reg_arm);

// let the system know the register has been modified to ensure it gets written back
ALLOC_ERR_t reg_modified(uint8_t reg_arm);

// when we're done with a register, we should free it; in most cases, it will be
// good practice to always free the register when it's not needed
ALLOC_ERR_t reg_free(uint8_t reg_arm);

// dump (forcibly) all the registers
ALLOC_ERR_t reg_flush(void);

void reg_reset();

#endif // __REG_ALLOC_H__
