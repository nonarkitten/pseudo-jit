#include "m68k_common.h"
#include "m68k_emit_ea.h"
#include "m68k_registers.h"

//----------------------------------------------------------------------------

// EFFECTIVE ADDRESS GENERATION

// REMEMBER
//
// Extension words are handled else where
// Program counter is also handled by the extension word handler
// Only extension words need the program counter, so there's no
// need to compute the program counter when just given immediates
//
// R1 will hold the Source Index or Absolute address if needed
//    if EA is PC relative, this will be the same as Absolute
//
// R2 will hold the Destination Index or Absolute address, if needed
//    however, the destination EA can never be PC relative


// find and optionally load reg, return 1 if success
int emit_get_reg(uint8_t* reg_arm, uint8_t reg_68k, uint8_t size) {
	return reg_alloc_68k(reg_arm, reg_68k, size) == ALLOC_OKAY;
}

// \brief Perform effective address load.
// Calculates the EA (sEA), loads the required register into sRR, and
// loads the data from memory into dRR. If EA is a register mode, then
// dRR will hold the ARM register.
// \param dRR (out) The register number which contains the calculated address; cannot be NULL
// \param sRR (out) The register number which contains the read DATA; pass NULL to suppress actual load
// \param sEA The source effective address
// \param sR The 68K source register
// \param size The size of the operation in bytes (1, 2 ro 4)
// \param is_src Set to true when this is a source EA mode and data will not be rewritten
// \return If successful, returns 1
static int emit_fetch_ea_data( uint8_t* dRR, uint8_t* sRR, uint16_t sEA, uint8_t sR, uint16_t size, uint8_t is_src ) {
	uint8_t tRR;
	uint8_t oRR;
	uint8_t omit_bic = 0, omit_eor = 0;
	
	if(dRR == NULL) {
		if(debug) printf("@ NULL passed for dRR\n");
		exit(1);
	}

	switch(sEA) {
	default:
		if(debug) printf("@ unknown EA\n");
		return 0;
	
	case EA_DREG:
		*dRR = reg_raw(sR);
		if(sRR) emit_get_reg(sRR, sR, size);
		return 1;

	case EA_AREG:
		if(size == 1) {
			if(debug) printf("@ AREG invalid for byte mode\n");
			return 0;
		}
		if(sRR) emit_get_reg(sRR, sR, size);
		*dRR = reg_raw(sR);
		return 1;

	case EA_ADDR: case EA_AINC: // (Ax)+
		if(sRR && (reg_alloc_temp( sRR ) == ALLOC_FAILED)) return 0;
		if(emit_get_reg( dRR, sR, 4 ) == ALLOC_FAILED) return 0;
		break;
	
	case EA_ADEC: // -(Ax)
		if(sRR && (reg_alloc_temp( sRR ) == ALLOC_FAILED)) return 0;
		if(emit_get_reg( dRR, sR, 4 ) == ALLOC_FAILED) return 0;
		emit("\tsub     r%d, r%d, #%d @ pre-dec\n", *dRR, *dRR, size);
		break;

	case EA_ADIS: case EA_AIDX:
		if(sRR && (reg_alloc_temp( sRR ) == ALLOC_FAILED)) return 0;
		if(emit_get_reg( dRR, sR, 4 ) == ALLOC_FAILED) return 0;
		emit("\tadd     r%d, r%d, r%d\n", (is_src ? 1 : 2), (is_src ? 1 : 2), *dRR);
		reg_modified(*dRR); reg_free(*dRR);
		*dRR = is_src ? 1 : 2;
		// assume PJIT eors index
		// omit_eor = true;
		break;

	case EA_PIDX: case EA_PDIS:
		if(!is_src) {
			if(debug) printf("@ PC invalid for destination mode\n");
			return 0;
		}
	case EA_ABSW: case EA_ABSL:
		if(sRR && (reg_alloc_temp( sRR ) == ALLOC_FAILED)) return 0;
		*dRR = is_src ? 1 : 2;
		// assumpe PJIT clips to 24-bits when scanning
		omit_bic = true;
		// assumpe PJIT eors absolute addresses
		omit_eor = true;
		break;

	case EA_IMMD:
		if(!is_src) {
			if(debug) printf("@ IMMD invalid for destination mode\n");
			return 0;
		}
		if(sRR) {
			*sRR = is_src ? 1 : 2;
			reg_alloc_arm(*sRR);
			*dRR = *sRR; // ??? does this make sense?
		}
		return 1; // data already present

	}

	oRR = *dRR; // for EA_AINC

	// swap upper/lower bytes
	if(!omit_eor && (size == 1)) {
#ifndef __PJIT_BIG_ENDIAN
		if(*dRR < REG_MAP_COUNT) {
			emit("\teor     r%d, r%d, #1\n", *dRR, *dRR);
		} else {
			if(reg_alloc_temp(&tRR ) == ALLOC_FAILED) return 0;
			emit("\teor     r%d, r%d, #1\n", tRR, *dRR);
			*dRR = tRR;
		}
#endif
	}
	
	// clear out high 24-bits		
	if(!omit_bic) {
		if(*dRR < REG_MAP_COUNT) {
			emit("\tbic     r%d, r%d, #0xFF000000\n", *dRR, *dRR);
		} else {
			if(reg_alloc_temp(&tRR ) == ALLOC_FAILED) return 0;
			emit("\tbic     r%d, r%d, #0xFF000000\n", tRR, *dRR);
			*dRR = tRR;
		}
	}

	if(sEA == EA_AINC) {
		emit("\tadd     r%d, r%d, #%d @ post-inc\n", oRR, oRR, size);
	}

	// load the data
	if(sRR) {
		//if(/*is_src &&*/ reg_alloc_temp( sRR ) == ALLOC_FAILED) return 0;
		emit("\t%s   r%d, [r%d]\n", ldx(size), *sRR, *dRR);
		// if we're long, swap words
#ifndef __PJIT_BIG_ENDIAN
		if(size == 4) emit("\tror     r%d, r%d, #16\n", *sRR, *sRR);
#endif
	}
	
	if(is_src) reg_free(*dRR);
	return 1;
}
// baseline
// line average: 5.658190
// adding OR to each 24-bit memory access:
// line average: 6.430614 (13.65% slower)
// removing 24-bit address filter entirely:
// line average: 4.890251 (13.57% faster)
// all registers in ARM registers:
// line average: 4.648176 (4.95% faster)
// no registers in ARM registers (except A7)
// line average: 6.019260 (23.09% slower)
// just data register in ARM registers
// line average: 5.691180 (16.38% slower)
// no endian handling
// line average: 4.401896 (10% faster)


int get_source_data( uint8_t* sRR, uint16_t sEA, uint8_t sR, uint16_t size ) {
	uint8_t dRR; // scratch, don't care
	int err;

	if(debug) printf("@ in get_source_data\n");
	err = emit_fetch_ea_data( &dRR, sRR, sEA, sR, size, 1 );
	if(err == 0 && debug) printf("@ error\n");
	return err;
}

// \brief Perform effective address load for read-modify-write operations.
// Calculates the EA (dEA), loads the required register into tRR, and
// loads the data from memory into dRR. If EA is a register mode, then
// dRR will hold the ARM register.
// \param dRR (out) The register number which contains the calculated address
// \param tRR (out) The register number which contains the read DATA
// \param dEA The destination effective address
// \param dR The 68K destination register
// \param size The size of the operation in bytes (1, 2 or 4)
// \return If successful, returns 1
int get_destination_data( uint8_t* dRR, uint8_t* tRR, uint16_t dEA, uint8_t dR, uint16_t size ) {
	int err;

	if(debug) printf("@ in get_destination_data\n");
	err = emit_fetch_ea_data( dRR, tRR, dEA, dR, size, 0 );
	if(err == 0 && debug) printf("@ error\n");
	return err;
}
//	set_destination_data( &dRR, &tRR, dEA, size & 3 );

int set_destination_data( uint8_t* dRR, uint8_t* tRR, uint16_t dEA, uint16_t size ) {
	ALLOC_ERR_t err = ALLOC_OKAY;
	if(debug) printf("@ in set_destination_data\n");
	if(dEA == EA_AREG || dEA == EA_DREG) {
		if(*dRR == 0xFF) err = reg_modified(*tRR);
		else if(*tRR == *dRR) ;
		else if(size == 4) emit("\tmov     r%d, r%d\n", *dRR, *tRR);
		else emit("\tbfi     r%d, r%d, #0, #%d\n", *dRR, *tRR, size * 8);

	} else if(dEA == EA_PDIS || dEA == EA_PIDX || dEA == EA_IMMD) {
		err = ALLOC_FAILED;

	} else {
#ifndef __PJIT_BIG_ENDIAN
		if(size == 4) emit("\tror     r%d, #16\n", *tRR);
#endif
		emit("\t%s    r%d, [r%d]\n", stx(size), *tRR, *dRR);

	}
	reg_flush();
	return err;
}


