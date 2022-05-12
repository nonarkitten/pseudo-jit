#include "m68k_registers.h"

// PJIT Better Register Allocation
// --ARM-- 68K USE
// ------- --- ------
// R0      T0  Src Ext Addr, temp
// R1      T1  Dest Ext Addr, temp
// R2      T2  Temp

// R3      D0  Data registers
// R4      D1   "

// R5      CPU CPU state pointer

// R6      A0  Address registers
// R7      A1   "
// R8      A2   "
// R9      A3   "
// R10     A4   "
// R11     A5   "
// R12     A6   "

// R13 SP  SP* Stack Pointer
// R14 LR  N/A Link Register
// R15 PC  N/A ARM Program Counter

static int used_regs = 0;

// Table of 68k registers as they relate to ARM ones
static const uint8_t reg68k_to_arm[16] = {
	0x03, 0x04, 0xFF, 0xFF, 		// D0-D3 (fixed)
	0xFF, 0xFF, 0xFF, 0xFF, 		// D4-D7 (dynamic)
	0x06, 0x07, 0x08, 0x09,    		// A0-A4 (fixed)
	0x0A, 0x0B, 0x0C, 0x0D			// A5-A7 (also fixed)
};
// 0xB is the "cpu" pointer
// 0x0C, 0xD, 0xE, 0xF are reserved

extern int last_opcode;
static reg_map_t reg_map[REG_MAP_COUNT] = { 0 };

bool reg_unalloced(uint8_t reg_arm) {
	return reg_map[reg_arm].size == 0;
}

uint8_t reg_raw(uint8_t reg_68k) {
	return reg68k_to_arm[reg_68k & 0xF];
}

// attempt to find a free register and return that
ALLOC_ERR_t reg_alloc_temp(uint8_t *reg_arm) {
	for(uint8_t reg=0; reg<REG_MAP_COUNT; reg++) {
		// is this a dynamic register and is it free?
		reg_map_t* r = &reg_map[reg];
		if(!r->size && !r->lock) {
			r->lock = 1;
			if(debug) printf("@ reg_alloc_temp r%d\n", reg);
			*reg_arm = reg;
			used_regs++;
			return ALLOC_OKAY;
		}
	}
	printf("@ reg_alloc_temp failed at opcode %04X!\n", last_opcode);
	exit(1);
	return ALLOC_FAILED;
}

// allocate a specific arm register
ALLOC_ERR_t reg_alloc_arm(uint8_t reg_arm) {
	// requires dynamic register and to not be in use
	if((reg_arm < REG_MAP_COUNT) && (reg_map[reg_arm].size == 0)) {
		if(debug) printf("@ reg_alloc r%d\n", reg_arm);
		reg_map[reg_arm].lock = 1;
		used_regs++;
		return ALLOC_OKAY;
	} else {
		printf("@ reg_alloc_arm failed!\n");
		return ALLOC_FAILED;
	}
}

// allocate a specific 68k register, returning it's arm register used
// if a new temp register is needed, this will allocate it, but not load it
ALLOC_ERR_t reg_alloc_68k(uint8_t *reg_arm, uint8_t reg_68k, int size) {
	int suppress_load = 0;
 	reg_68k &= 0xF;

	if(size < 0) size = -size, suppress_load = 1;
	if(size > 3) size = 3;

	// return fixed registers immediately
	if(reg68k_to_arm[reg_68k] != 0xFF) {
		uint8_t reg = reg68k_to_arm[reg_68k];
		if(size < 3) {
			uint8_t tRR; 
			if(debug) printf("@ reg_alloc_68k r%d (need temp)\n", reg);
			if(reg_alloc_temp(&tRR) == ALLOC_OKAY) {
				reg_map[tRR].size = size;
				reg_map[tRR].reg = reg;

				char sgn = (ldx == ldsx) ? 's' : 'u';
				char sze = (size == 2) ? 'h' : 'b';
				if(reg == 13) {
				emit("\tmov     r%d, r%d\n", tRR, reg);
				reg = tRR;
				}
				emit("\t%cxt%c    r%d, r%d\n", sgn, sze, tRR, reg);
				*reg_arm = tRR;

			} else {
				printf("@ reg_alloc_68k failed!\n");
				return ALLOC_FAILED;				
			}
		} else {
			if(debug) printf("@ reg_alloc_68k r%d (fixed)\n", reg);
			*reg_arm = reg;
		}
		return ALLOC_OKAY;
	}

	// check if it's already allocated
	for(uint8_t reg=0; reg<REG_MAP_COUNT; reg++) {
		reg_map_t* r = &reg_map[reg];
		if((r->reg == reg_68k) && (r->size == size)) {
			if(debug) printf("@ reg_alloc_68k r%d (existing)\n", reg);
			*reg_arm = reg;
			return ALLOC_OKAY;
		}
	}
		
	// allocate a new register
	for(uint8_t reg=0; reg<REG_MAP_COUNT; reg++) {
		reg_map_t* r = &reg_map[reg];
		if(!r->size && !r->lock) {
			r->reg = reg_68k;
			r->size = size;
			if(debug) printf("@ reg_alloc_68k r%d (new)\n", reg);
			if(!suppress_load) {
			emit("\t%s   r%d, [" CPU ", %d]\n", ldx(size), reg, reg_68k * 4);
			*reg_arm = reg;
			}

			used_regs++;
			return ALLOC_OKAY;
		}
	}
	printf("@ reg_alloc_68k failed!\n");
	return ALLOC_FAILED;
}

// let the system know the register has been modified to ensure it gets written back
ALLOC_ERR_t reg_modified(uint8_t reg_arm) {
	// requires dynamic register and to be in use
	if((reg_arm < REG_MAP_COUNT) && (reg_map[reg_arm].size)) {
		if(debug) printf("@ reg_modified r%d\n", reg_arm);
		reg_map[reg_arm].mod = 1;
	}
	return ALLOC_OKAY;
}

// when we're done with a register, we should free it; in most cases, it will be
// good practice to always free the register when it's not needed
ALLOC_ERR_t reg_free(uint8_t reg_arm) {
	if(reg_arm < REG_MAP_COUNT) {
		if(debug) printf("@ reg_free r%d (%02X)\n", reg_arm, reg_map[reg_arm].raw);
		if(reg_map[reg_arm].size && reg_map[reg_arm].mod) {
			if(reg_map[reg_arm].lock) {
				// fixed register, but local copy
				uint8_t dRR = reg_map[reg_arm].reg;
				if(reg_map[reg_arm].size > 2) {
					emit("\tmov     r%d, r%d\n", dRR, reg_arm);
				} else {
					emit("\tbfi     r%d, r%d, #0, #%d\n", dRR, reg_arm, (reg_map[reg_arm].size * 8));
				}
				
			} else {
				uint16_t offset = reg_map[reg_arm].reg * 4;
				emit("\t%s    r%d, [" CPU ", %d]\n", stx(reg_map[reg_arm].size), reg_arm, offset);
			}
		}
		reg_map[reg_arm].raw = 0;
		if(used_regs >= 4) {
			extern int last_opcode;
			printf("@ opcode %04X needed %d regs %s\n", last_opcode, used_regs, ((used_regs > 4) ? "wtf?" : ""));
		}
		used_regs--;
	}
	
	return ALLOC_OKAY;
}

// dump (forcibly) all the registers
ALLOC_ERR_t reg_flush() {
	ALLOC_ERR_t err = ALLOC_OKAY;
	
	if(debug) printf("@ reg_flush\n");
	
	for(uint8_t reg_arm=0; reg_arm<REG_MAP_COUNT; reg_arm++) {
		if(reg_free(reg_arm) != ALLOC_OKAY) err = ALLOC_FAILED;
	}
	used_regs = 0;
	return err;
}

void reg_reset() {
	if(debug) printf("@ reg_reset\n");
	
	for(uint8_t reg_arm=0; reg_arm<REG_MAP_COUNT; reg_arm++) {
		reg_map[reg_arm].raw = 0;
	}
}
