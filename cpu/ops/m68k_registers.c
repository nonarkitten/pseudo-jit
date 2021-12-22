#include "m68k_registers.h"

// Number of temporary ARM registers
#define REG_MAP_COUNT 4

// Table of 68k registers as they relate to ARM ones
static const uint8_t reg68k_to_arm[16] = {
	0x04, 0x05, 0x06, 0x07, // D0-D3 (fixed)
	0xFF, 0xFF, 0xFF, 0xFF, // D4-D7 (dynamic)
	0x08, 0x09, 0x0A,       // A0-A2 (fixed)
	0xFF, 0xFF, 0xFF, 0xFF, // A3-A6 (dynamic)
	0x0B                    // A7    (fixed)
};
// 0xC, 0xD, 0xE, 0xF are reserved

static reg_map_t reg_map[REG_MAP_COUNT] = { 0 };

uint8_t reg_raw(uint8_t reg_68k) {
	return reg68k_to_arm[reg_68k & 0xF];
}

// attempt to find a free register and return that
ALLOC_ERR_t reg_alloc_temp(uint8_t *reg_arm) {
	for(uint8_t reg=0; reg<REG_MAP_COUNT; reg++) {
		// is this a dynamic register and is it free?
		reg_map_t* r = &reg_map[reg];
		if(!r->used && !r->lock) {
			reg_map[reg].lock = 1;
			if(debug) printf("@ reg_alloc_temp r%d\n", reg);
			*reg_arm = reg;
			return ALLOC_OKAY;
		}
	}
	printf("@ reg_alloc_temp failed!\n");
	return ALLOC_FAILED;
}

// allocate a specific 68k register, returning it's arm register used
// if a new temp register is needed, this will allocate it, but not load it
ALLOC_ERR_t reg_alloc_68k(uint8_t *reg_arm, uint8_t reg_68k) {
 	reg_68k &= 0xF;
 	
 	if(!reg_arm) return ALLOC_FAILED;

	// return fixed registers immediately
	if(reg68k_to_arm[reg_68k] != 0xFF) {
		uint8_t reg = reg68k_to_arm[reg_68k];
		if(debug) printf("@ reg_alloc_68k r%d\n", reg);
		*reg_arm = reg;
		return ALLOC_OKAY;
	}

	// check if it's already allocated
	for(uint8_t reg=0; reg<REG_MAP_COUNT; reg++) {
		reg_map_t* r = &reg_map[reg];
		if((r->reg == reg_68k) && r->used && !r->lock) {
			r->used++;
			if(debug) printf("@ reg_alloc_68k r%d\n", reg);
			*reg_arm = reg;
			return ALLOC_OKAY;
		}
	}
		
	// allocate a new register
	for(uint8_t reg=0; reg<REG_MAP_COUNT; reg++) {
		reg_map_t* r = &reg_map[reg];
		if(!r->used && !r->lock) {
			r->reg = reg_68k;
			r->used = 1;
			r->lock = 0;
			r->mod = 0;
			if(debug) printf("@ reg_alloc_68k r%d\n", reg);
			*reg_arm = reg;
			return ALLOC_OKAY;
		}
	}
	printf("@ reg_alloc_68k failed!\n");
	return ALLOC_FAILED;
}		

// allocate a specific arm register
ALLOC_ERR_t reg_alloc_arm(uint8_t reg_arm) {
	// requires dynamic register and to not be in use
	if((reg_arm < REG_MAP_COUNT) && (reg_map[reg_arm].used == 0)) {
		if(debug) printf("@ reg_alloc r%d\n", reg_arm);
		reg_map[reg_arm].lock = 1;
		return ALLOC_OKAY;
	} else {
		printf("@ reg_alloc_arm failed!\n");
		return ALLOC_FAILED;
	}
}

// let the system know the register has been modified to ensure it gets written back
ALLOC_ERR_t reg_modified(uint8_t reg_arm) {
	// requires dynamic register and to be in use
	if((reg_arm < REG_MAP_COUNT) && (reg_map[reg_arm].used)) {
		if(debug) printf("@ reg_modified r%d\n", reg_arm);
		reg_map[reg_arm].mod = 1;
// 	} else {
// 		printf("@ r%d reg_modified failed!\n", reg_arm);
// 		return ALLOC_FAILED;
	}
	return ALLOC_OKAY;
}

// when we're done with a register, we should free it; in most cases, it will be
// good practice to always free the register when it's not needed
ALLOC_ERR_t reg_free(uint8_t reg_arm) {
// 	if(reg68k_to_arm[reg_68k] != 0xFF) {
// 		printf("@ reg_free failed!\n");
// 		return ALLOC_FAILED;
// 	}
	char *was = "";
	
	if(reg_map[reg_arm].lock) {
		was = "locked";
		reg_map[reg_arm].lock = 0;
		reg_map[reg_arm].reg = 0;
		
	} else {
		// free used registers
		if(reg_map[reg_arm].used) {
			if(--reg_map[reg_arm].used) {
				was = "used";
			
			} else {
				if(reg_map[reg_arm].mod) {
					uint16_t offset = reg_map[reg_arm].reg * 4;
					emit("\tstr     r%d, [r12, #%d]\n", reg_arm, offset);				
					reg_map[reg_arm].mod = 0;
					was = "modified";
				}
				reg_map[reg_arm].reg = 0;
				was = "freed";
			}
		} else {
			was = "fixed";
		}
	}
	
	if(debug) {
		printf("@ reg_free r%d (%s)\n", reg_arm, was);
		//printf("\n");
	}
	return ALLOC_OKAY;
}

// dump (forcibly) all the registers
ALLOC_ERR_t reg_flush(void) {
	if(debug) printf("@ reg_flush\n");
	
	for(uint8_t reg_arm=0; reg_arm<REG_MAP_COUNT; reg_arm++) {
		// dump locks
		if(reg_map[reg_arm].lock) {
			if(debug) printf("@ r%d locked\n", reg_arm);
			reg_map[reg_arm].lock = 0;
		}
		
		// free used registers
		else if(reg_map[reg_arm].used) {
			reg_map[reg_arm].used = 0;
			if(reg_map[reg_arm].mod) {
				if(debug) printf("@ r%d modified\n", reg_arm);
				// TODO: need to fix-up for size
				uint16_t offset = reg_map[reg_arm].reg * 4;
				emit("\tstr     r%d, [r12, #%d]\n", reg_arm, offset);				
				reg_map[reg_arm].mod = 0;
			} else {
				if(debug) printf("@ r%d used\n", reg_arm);
			}
		}
		
		reg_map[reg_arm].reg = 0;
	}
	return ALLOC_OKAY;
}