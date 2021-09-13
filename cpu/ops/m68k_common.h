#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

#ifndef __M68K_COMMON_H__
#define __M68K_COMMON_H__

#define OFFSETOF(TYPE, ELEMENT) ((size_t)&(((TYPE *)0)->ELEMENT))

#ifndef assert
#define assert()
#endif

extern int debug;
extern int lines;

typedef union {
	uint8_t raw;
	struct {
		uint8_t lock : 1; // is this ARM register locked
		uint8_t mod  : 1; // has this register been modified
		uint8_t used : 2; // has this register been alloc'd
		uint8_t reg  : 4; // 68k register index
	};
} reg_map_t;

typedef enum { 
	MOVE_BYTE = 1, MOVE_LONG, MOVE_WORD 
} MOVE_SIZE_t;

typedef enum { 
	// Basic effective addressing modes
	EA_DREG, EA_AREG, EA_ADDR, EA_AINC, EA_ADEC, 
	// Extension word based addressing modes
	EA_ADIS, EA_AIDX, 
	// End of any register-used modes
	EA_ABSW, EA_ABSL, EA_PDIS, EA_PIDX, EA_IMMD
} EA_MODE_t;

// GENERATOR 

enum { STREAM_SET, STREAM_INC, STREAM_GET };
static char * stream(char * set, int offset, int mode) {
	static volatile char * _stream;
	switch(mode) {
	case 0: _stream = (volatile char *)set; break;
	case 1: _stream += offset; break;
	}
	return (char *)_stream;
}

static void emit(char * format, ...) {
	extern int lines;
	char *_stream = stream( NULL, 0, STREAM_GET );
	int len;
	
	va_list args;
	va_start (args, format);
	len = vsprintf(_stream, format, args);
	if(debug) printf("@ %s", _stream);
	_stream = stream( NULL, len, STREAM_INC );
	va_end (args);
	lines += 1;
}

// REGISTER ALLOCATION

// Table of arm registers currently in-use
#define REG_MAP_COUNT 4
extern reg_map_t reg_map[REG_MAP_COUNT];

// Table of 68k registers as they relate to ARM ones
// We save 1460 bytes for each data register in ARM core
// We save 13140 bytes for each address register in ARM core
static const uint8_t reg68k_to_arm[16] = {
	0x04, 0x05, 0x06, 0x07, // D0-D3 (fixed)
	0xFF, 0xFF, 0xFF, 0xFF, // D4-D7 (dynamic)
	0x08, 0x09, 0x0A,       // A0-A2 (fixed)
	0xFF, 0xFF, 0xFF, 0xFF, // A3-A6 (dynamic)
	0x0B                    // A7    (fixed)		
};


// attempt to find a free register and return that
static inline int8_t reg_alloc_temp(void) {
	int8_t reg_arm;
	for(reg_arm=0; reg_arm<REG_MAP_COUNT; reg_arm++) {
		// is this a dynamic register and is it free?
		reg_map_t* r = &reg_map[reg_arm];
		if(!r->used && !r->lock) {
			reg_map[reg_arm].lock = 1;
			break;
		}
	}
	if(debug) printf("@ reg_alloc_temp r%d\n", reg_arm);
	assert(reg_arm < 4);
	return reg_arm;
}

// allocate a specific 68k register, returning it's arm register used
static inline int8_t reg_alloc_68k(int8_t reg_68k, int8_t no_load) {
	int8_t reg_arm;	
 	reg_68k &= 0xF;

	// return fixed registers immediately
	if(reg68k_to_arm[reg_68k] != 0xFF) return reg68k_to_arm[reg_68k];

	// check if it's already allocated
	for(reg_arm=0; reg_arm<REG_MAP_COUNT; reg_arm++) {
		reg_map_t* r = &reg_map[reg_arm];
		if((r->reg == reg_68k) && r->used && !r->lock) {
			r->used++;
			assert(r->used > 0);
			goto exit;
		}
	}
		
	// allocate a new register
	for(reg_arm=0; reg_arm<REG_MAP_COUNT; reg_arm++) {
		reg_map_t* r = &reg_map[reg_arm];
		if(!r->used && !r->lock) {
			reg_map[reg_arm].reg = reg_68k;
			reg_map[reg_arm].used = 1;
			reg_map[reg_arm].lock = 0;
			reg_map[reg_arm].mod = 0;
			if(!no_load) {
				uint16_t offset = reg_68k * 4;
				emit("\tldr     r%d, [r12, #%d]\n", reg_arm, offset);
			}
			goto exit;
		}
	}
	assert(reg_arm < 4);
exit:
	if(debug) printf("@ reg_alloc_68k %c%d to r%d\n", ((reg_68k > 7) ? 'a' : 'd'), reg_68k & 7, reg_arm);
	return reg_arm;
}		

// allocate a specific arm register
static inline void reg_alloc_arm(int8_t reg_arm) {
	// requires dynamic register and to not be in use
	if(debug) printf("@ reg_alloc r%d\n", reg_arm);
	if((reg_arm < REG_MAP_COUNT) && (reg_map[reg_arm].used == 0))
		reg_map[reg_arm].lock = 1;
}

// let the system know the register has been modified to ensure it gets written back
static inline void reg_modified(int8_t reg_arm) {
	// requires dynamic register and to be in use
	if(debug) printf("@ reg_modified r%d\n", reg_arm);
	if((reg_arm < REG_MAP_COUNT) && (reg_map[reg_arm].used))
		reg_map[reg_arm].mod = 1;
}

// when we're done with a register, we should free it; in most cases, it will be
// good practice to always free the register when it's not needed
static inline void reg_free(int8_t reg_arm) {
	if(debug) printf("@ reg_free r%d", reg_arm);
	if(reg_map[reg_arm].lock) {
		if(debug) printf(" (locked)");
		reg_map[reg_arm].lock = 0;
		reg_map[reg_arm].reg = 0;
	} else {
		// free used registers
		if(reg_map[reg_arm].used) {
			if(--reg_map[reg_arm].used) {
				if(debug) printf(" (used)");
			
			} else {
				if(reg_map[reg_arm].mod) {
					uint16_t offset = reg_map[reg_arm].reg * 4;
					emit("\tstr     r%d, [r12, #%d]\n", reg_arm, offset);				
					reg_map[reg_arm].mod = 0;
					if(debug) printf(" (modified)");
				}
				reg_map[reg_arm].reg = 0;
				if(debug) printf(" (freed)");
			}
		} else {
			if(debug) printf(" (fixed)");
		}
	}
	if(debug) printf("\n");
}

// dump (forcibly) all the registers
static inline void reg_flush(void) {
	if(debug) printf("@ reg_flush\n");
	for(int8_t reg_arm=0; reg_arm<REG_MAP_COUNT; reg_arm++) {
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
				uint16_t offset = reg_map[reg_arm].reg * 4;
				emit("\tstr     r%d, [r12, #%d]\n", reg_arm, offset);				
				reg_map[reg_arm].mod = 0;
			} else {
				if(debug) printf("@ r%d used\n", reg_arm);
			}
		}
		reg_map[reg_arm].reg = 0;
	}
}

// EFFECTIVE ADDRESS GENERATION

// REMEMBER
//
// Extension words are handled else where
// Program counter is also handled by the extension word handler
// Only extension words need the program counter, so there's no
// need to compute the program counter when just given immediates
//
// R0 will hold the Program Counter, if needed
//    if this is 0xFFFFFFFF then the extension handler should recalc it
//    otherwise, the extension handler should always leave it alone
//    should be reset to 0xFFFFFFFF before exiting the opcode handler
//
// R1 will hold the Source Index, if needed
//
// R2 will hold the Destination Index, if needed
//    however, the destination EA can never be PC relative

// given a 68k register and ea mode, allocate an ARM one if we need to
// and add the index if we're running low on registers
static inline uint8_t emit_get_reg(uint16_t* ea, int8_t reg_68k, int8_t reg_cea, int8_t no_load) {
	int8_t reg_arm = reg_cea;
	
	// grab our register
	if(*ea < EA_ABSW) {
		// find and optionally load reg
		reg_arm = reg_alloc_68k(reg_68k, no_load); 
		
		// if our register was loaded from the context, then we do not have enough spare 
		// registers to perform an index operation; in this case, we'll manually add them
		// now and free the offset register and use normal address mode
		if((reg_arm < 4) && ((*ea == EA_ADIS) || (*ea == EA_AIDX))) {
			emit("\tadd     r%d, r%d @ space saving\n", reg_arm, reg_cea);
			reg_free(reg_cea);
			//if(debug) printf("@ Changing EA to ADDR\n");
			*ea = EA_ADDR;
		}
	}
	return reg_arm;
}

// Transfer something from MEMORY and put it into a register; note that this does
// not include loading registers from context (see above) and if this is not loading
// anything from memory, it should not do anything and should just return
static inline int8_t emit_load(
	uint16_t ea, 
	int8_t reg_arm_dest,
	int8_t reg_arm, 
	int8_t reg_cea, 
	int8_t is_src, 
	int8_t size, 
	int8_t set_flags
) {
	static const char * ldx[] = { NULL, "ldrsb", "ldrsh", NULL, "ldr  " };
	
	if(ea == EA_IMMD) {
		if(debug) printf("@ immediate source\n");
		return reg_cea;
	}
	else if(ea <= EA_AREG) {
		return reg_arm;
	}
	
	if(reg_arm_dest == -1) reg_arm_dest = reg_alloc_temp();
	
	if((reg_arm == 11) && (size == 1)) size = 2;
	
	// off to memory, we'll need to allocate a spare register to
	// read the data into since we don't know what to do with it		
	switch(ea) {
	
	// address post-increment; if this is destination ea (rmw), don't do this
	case EA_AINC:
		if(!is_src) break;
		emit("\t%s   r%d, [r%d], #%d\n", ldx[size], reg_arm_dest, reg_arm, size);
		reg_modified(reg_arm);
		break;
			
	// absolute or PC fetch
	case EA_PDIS: case EA_PIDX: case EA_ABSW: case EA_ABSL:
		emit("\t%s   r%d, [r%d]\n", ldx[size], reg_arm_dest, reg_cea);
		if(is_src) reg_free(reg_cea);
		break;
		
	// basic address fetch
	case EA_ADDR: 
		emit("\t%s   r%d, [r%d]\n", ldx[size], reg_arm_dest, reg_arm);
		break;
	
	// address with pre-decrement; we can safely always do this
	case EA_ADEC:
		emit("\t%s   r%d, [r%d, #%d]!\n", ldx[size], reg_arm_dest, reg_arm, -size);
		reg_modified(reg_arm);
		break;
	
	// address-indexed mode, source may discard CEA but destination ea should keep it
	default:
		emit("\t%s   r%d, [r%d, r%d]!\n", ldx[size], reg_arm_dest, reg_cea, reg_arm);
		if(is_src) reg_free(reg_cea);
		break;
	}
	
	// adjust for endianess
	if(size == 4) emit("\tror%c    r%d, #16\n", (set_flags ? 's' : ' '), reg_arm_dest);

	reg_modified(reg_arm_dest);
	
	// we should be done with this now			
	reg_free(reg_arm);
	
	// return our allocated register
	return reg_arm_dest;
}

// Transfer something from a REGISTER to MEMORY; if the operation requested specifies
// a register destination, then return and do NOTHING
static inline void emit_store(
	uint16_t ea, 
	int8_t reg_arm_dest,
	int8_t reg_arm,
	int8_t reg_cea,
	int8_t size
) {
	if(ea <= EA_AREG || ea == EA_IMMD) return;
	
	if(size == 1) {
		if(ea == EA_AINC) {
			if((reg_arm == 11) && (size == 1)) size = 2;
			emit("\tstrb    r%d, [r%d, #%d]!\n", reg_arm, reg_arm_dest, size); 
			reg_modified(reg_arm_dest);
		} else {     
			emit("\tstrb    r%d, [r%d]\n", reg_arm, reg_arm_dest);
		}
	}	
	else if(size == 2) {
		if(ea == EA_AINC) {
			emit("\tstrh    r%d, [r%d, #2]!\n", reg_arm, reg_arm_dest); 
			reg_modified(reg_arm_dest);
		} else {
			emit("\tstrh    r%d, [r%d]\n", reg_arm, reg_arm_dest); 
		}
	}	
	else {
		int8_t temp_arm = reg_alloc_temp();
		emit("\tror     r%d, r%d, #16\n", temp_arm, reg_arm);
		if(ea == EA_AINC) {
			emit("\tstr     r%d, [r%d], #4\n", temp_arm, reg_arm_dest);
			reg_modified(reg_arm_dest);
		} else {
			emit("\tstr     r%d, [r%d]\n", temp_arm, reg_arm_dest); 
		}
		reg_free(temp_arm);
	}
}

#endif
