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


// find and optionally load reg
int emit_get_reg(uint8_t* reg_arm, uint8_t reg_68k, uint8_t size) {
	uint8_t arm, t;
	if(reg_alloc_68k(&arm, reg_68k) == ALLOC_FAILED) return 0;
	
	if(reg_arm == NULL) return 0;
	
	if(arm < 4) { // dynamic
		emit("\tldr     r%d, [r12, #%d]\n", arm, reg_68k * 4); //
		*reg_arm = arm;
	
	} else if(size < 4) { // fixed, but need temp
		if(reg_alloc_temp(&t) == ALLOC_FAILED) return 0;
		emit("\tsxt%c    r%d, r%d\n", ((size == 2) ? 'h' : 'b'), t, arm);
		*reg_arm = t;
		
	} else { // fixed and ok as-is
		*reg_arm = arm;
	}
	
	return 1;
}

// \brief Perform effective address load
// \param sRR The register number which contains the read DATA
// \param dRR The register number which contains the real ARM address this came from; if sEA is a register mode, then this is not used
// \param sEA The source effective address
// \param sR The 68K source register
// \param size The size of the operation in bytes (1, 2 ro 4)
// \param is_src Set to true when this is a source EA mode and data will not be rewritten
// \return If successful, returns 1
static int emit_fetch_ea_data( uint8_t* dRR, uint8_t* sRR, uint16_t sEA, uint8_t sR, uint16_t size, uint8_t is_src ) {
	uint8_t tRR;
	uint8_t omit_bic = 0;

	if(dRR == NULL) {
		if(debug) printf("@ NULL passed for dRR\n");
		return 0;
	}
	if(sEA == EA_DREG || sEA == EA_AREG) {
		if(sEA == EA_AREG && size == 1) {
			if(debug) printf("@ AREG invalid for destination mode\n");
			return 0;
		}
		if(dRR) *dRR = reg_raw(sR);
		return sRR ? emit_get_reg(sRR, sR, size) : 1;

	} else if(sEA == EA_IMMD) {
		if(!is_src) {
			if(debug) printf("@ IMMD invalid for destination mode\n");
			return 0;
		}
		if(sRR) *sRR = is_src ? 1 : 2;
		return 1;

	} else {
		if(sEA ==  EA_PIDX || sEA == EA_PDIS) {
			if(!is_src) {
				if(debug) printf("@ PC EA invalid for destination mode\n");
			}
			*dRR = is_src ? 1 : 2;
		} else if(sEA == EA_ABSW || sEA ==  EA_ABSL) {
			// assumpe PJIT clips to 24-bits when scanning
			if(sEA == EA_ABSL) omit_bic = true;
			*dRR = is_src ? 1 : 2;
		} else if(emit_get_reg( dRR, sR, 4 ) == ALLOC_FAILED) {
			return 0;
		}
		if(sRR && reg_alloc_temp( sRR ) == ALLOC_FAILED) return 0;
		if(sEA == EA_AINC) {
			if(reg_alloc_temp(&tRR ) == ALLOC_FAILED) return 0;
			emit("\tmov     r%d, r%d\n", tRR, *dRR);
			emit("\tadd     r%d, r%d, #%d\n", *dRR, tRR, size);
			reg_modified(*dRR); reg_free(*dRR);
			*dRR = tRR;
		} else if(sEA == EA_ADEC) {
			if(reg_alloc_temp(&tRR ) == ALLOC_FAILED) return 0;
			emit("\tsub     r%d, r%d, #%d\n", *dRR, *dRR, size);
			emit("\tmov     r%d, r%d\n", tRR, *dRR);
//			reg_modified(*dRR); reg_free(*dRR); reg_alloc_arm(*dRR);
			reg_modified(*dRR); reg_free(*dRR);
			*dRR = tRR;
		} else if(sEA == EA_ADIS || sEA == EA_AIDX) {
			emit("\tadd     r%d, r%d, r%d\n", (is_src ? 1 : 2), *dRR, size);
			reg_modified(*dRR); reg_free(*dRR);
			*dRR = is_src ? 1 : 2;
		}

		// swap upper/lower bytes
		if(size == 1) emit("\teor     r%d, r%d, #1\n", *dRR, *dRR);
		// clear out high 24-bits		
		if(!omit_bic) emit("\tbic     r%d, r%d, #0xFF000000\n", *dRR, *dRR);
		// load the data
		if(sRR) emit("\t%s   r%d, [r%d]\n", ldx(size), *sRR, *dRR);
		if(is_src) reg_free(*dRR);
		// if we're long, swap words
		if(sRR && (size == 4)) emit("\tror     r%d, #16\n", *sRR);
	}
	return 1;
}

int get_source_data( uint8_t* sRR, uint16_t sEA, uint8_t sR, uint16_t size ) {
	uint8_t dRR; // scratch, don't care
	int err;

	if(debug) printf("@ in get_source_data\n");
	err = emit_fetch_ea_data( &dRR, sRR, sEA, sR, size, 1 );
	if(err == 0 && debug) printf("@ error\n");
	return err;
}

int get_destination_data( uint8_t* dRR, uint8_t* tRR, uint16_t dEA, uint8_t dR, uint16_t size ) {
	int err;

	if(debug) printf("@ in get_destination_data\n");
	err = emit_fetch_ea_data( dRR, tRR, dEA, dR, size, 0 );
	if(err == 0 && debug) printf("@ error\n");
	return err;
}

int set_destination_data( uint8_t* dRR, uint8_t* tRR, uint16_t dEA, uint16_t size ) {
	ALLOC_ERR_t err = ALLOC_OKAY;
	if(debug) printf("@ in set_destination_data\n");
	if(dEA == EA_AREG || dEA == EA_DREG) {
		if((*dRR == 0xFF) || (*tRR == *dRR))
			err = reg_modified(*tRR);
		else if(size == 4)
			emit("\tmov     r%d, r%d\n", *dRR, *tRR);
		else
			emit("\tbfi     r%d, r%d, #0, #%d\n", *dRR, *tRR, (size * 8));

	} else if(dEA == EA_PDIS || dEA == EA_PIDX || dEA == EA_IMMD) {
		err = ALLOC_FAILED;

	} else {
		if(size == 4) emit("\tror     r%d, #16\n", *tRR);
		emit("\t%s    r%d, [r%d]\n", stx(size), *tRR, *dRR);

	}
	reg_flush();
	return err;
}

