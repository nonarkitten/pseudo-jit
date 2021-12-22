#include "m68k_common.h"
#include "m68k_emit_ea.h"
#include "m68k_registers.h"

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

// perform source effective address load, return 1 on success
int get_source_data( uint8_t* sRR, uint16_t sEA, uint8_t sR, uint16_t size ) {
	uint8_t tRR;
	
	if(sRR == NULL) return 0;
	
	switch(sEA) {
	default:
		printf("@ source ea error\n");
		return 0;
	case EA_DREG:
	case EA_AREG:
		return emit_get_reg(sRR, sR, size);

	case EA_ADDR:
		if(emit_get_reg( &tRR, sR, 4 ) == ALLOC_FAILED) return 0;
		if(reg_alloc_temp( sRR ) == ALLOC_FAILED) return 0;
		emit("\t%s   r%d, [r%d]\n", ldx(size), *sRR, tRR);
		if(size == 4) emit("\tror     r%d, #16\n", *sRR);
		return (reg_free(tRR) == ALLOC_OKAY);

	case EA_AINC:
		if(emit_get_reg( &tRR, sR, 4 ) == ALLOC_FAILED) return 0;
		if(reg_alloc_temp( sRR ) == ALLOC_FAILED) return 0;
		emit("\t%s   r%d, [r%d], #%d\n", ldx(size), *sRR, *sRR, size);
		if(size == 4) emit("\tror     r%d, #16\n", *sRR);
		if(reg_modified(tRR) == ALLOC_FAILED) return 0;
		return (reg_free(tRR) == ALLOC_OKAY);

	case EA_ADEC:
		if(emit_get_reg( &tRR, sR, 4 ) == ALLOC_FAILED) return 0;
		if(reg_alloc_temp( sRR ) == ALLOC_FAILED) return 0;
		emit("\t%s   r%d, [r%d, #-%d]!\n", ldx(size), *sRR, *sRR, size);
		if(size == 4) emit("\tror     r%d, #16\n", *sRR);
		if(reg_modified(tRR) == ALLOC_FAILED) return 0;
		return (reg_free(tRR) == ALLOC_OKAY);

	case EA_AIDX:
	case EA_ADIS:
		if(reg_alloc_arm(1) == ALLOC_FAILED) return 0;
		if(emit_get_reg( &tRR, sR, 4 ) == ALLOC_FAILED) return 0;
		if(reg_alloc_temp( sRR ) == ALLOC_FAILED) return 0;
		emit("\t%s   r%d, [r1, r%d]\n", ldx(size), *sRR, tRR);
		if(size == 4) emit("\tror     r%d, #16\n", *sRR);
		return (reg_free(1) == ALLOC_OKAY) && (reg_free(tRR) == ALLOC_OKAY);

	case EA_ABSW:
	case EA_ABSL:
	case EA_PIDX:
	case EA_PDIS:
		if(reg_alloc_arm(1) == ALLOC_FAILED) return 0;
		if(reg_alloc_temp( sRR ) == ALLOC_FAILED) return 0;
		emit("\t%s   r%d, [r%d]\n", ldx(size), *sRR, 1);
		if(size == 4) emit("\tror     r%d, #16\n", *sRR);
		return (reg_free(1) == ALLOC_OKAY);
		
	case EA_IMMD:
		*sRR = 1;
		return 1;
	} // end switch
}

static char _ea[32];

// perform destination effective address calculation and load
// data into temporary register; pass NULL to tRR if no load is needed
int get_destination_data( uint8_t* dRR, uint8_t* tRR, uint16_t dEA, uint8_t dR, uint16_t size ) {
	sprintf( _ea, "%s", "[r%d]" );
	switch(dEA) {
	default:
		printf("@ destination ea error\n");
		return 0;
	case EA_DREG:
	case EA_AREG:
		*dRR = reg_raw(dR);
		if(tRR) {
			//return emit_get_reg(tRR, dR, size);
			uint8_t arm;
			if(reg_alloc_68k(&arm, dR) == ALLOC_FAILED) return 0;
			if(arm < 4) emit("\t%s   r%d, [r12, #%d]\n", ldx(size), arm, dR * 4);
			*tRR = arm;
		}
		return 1;
		
	case EA_ADDR:
	case EA_AINC:
		if(emit_get_reg( dRR, dR, 4 ) == ALLOC_FAILED) return 0;
		if(tRR) {
			if(reg_alloc_temp( tRR ) == ALLOC_FAILED) return 0;
			emit("\t%s   r%d, [r%d]\n", ldx(size), *tRR, *dRR);
			if(size == 4) emit("\tror     r%d, #16\n", *tRR);
		}
		return 1;
		
	case EA_ADEC:
		if(emit_get_reg( dRR, dR, 4 ) == ALLOC_FAILED) return 0;
		if(tRR) {
			if(reg_alloc_temp( tRR ) == ALLOC_FAILED) return 0;
			emit("\t%s   r%d, [r%d, #-%d]!\n", ldx(size), *tRR, *dRR, size);
			if(size == 4) emit("\tror     r%d, #16\n", *tRR);
		} else {
			sprintf( _ea, "[r%%d, #-%d]", size );
		}
		return (reg_modified(*dRR) == ALLOC_OKAY);

	case EA_AIDX:
	case EA_ADIS:
		if(reg_alloc_arm(2) == ALLOC_FAILED) return 0;
		if(emit_get_reg( dRR, dR, 4 ) == ALLOC_FAILED) return 0;
		if(tRR) {
			if(reg_alloc_temp( tRR ) == ALLOC_FAILED) return 0;
			emit("\t%s   r%d, [r2, r%d]\n", ldx(size), *tRR, *dRR);
			if(size == 4) emit("\tror     r%d, #16\n", *tRR);
		} else {
			sprintf( _ea, "%s", "[r%d, r2]" );
		}
		return (reg_free(2) == ALLOC_OKAY);
		
	case EA_ABSW:
	case EA_ABSL:
		if(reg_alloc_arm(2) == ALLOC_FAILED) return 0;
		*dRR = 2;
		if(tRR) {
			if(reg_alloc_temp( tRR ) == ALLOC_FAILED) return 0;
			emit("\t%s   r%d, [r%d]\n", ldx(size), *tRR, 2);
			if(size == 4) emit("\tror     r%d, #16\n", *tRR);
		}
		return (reg_free(2) == ALLOC_OKAY);
		
	case EA_IMMD:
//		*tRR = 2;
		return 1;
	} // end switch
}

// write data to destination effective address
int set_destination_data( uint8_t* dRR, uint8_t* tRR, uint16_t dEA, uint16_t size ) {
	static char buffer[32];
	ALLOC_ERR_t err = ALLOC_OKAY;
	
	switch(dEA) {
	default:
		err = ALLOC_FAILED;
		break;
		
	case EA_DREG:
	case EA_AREG:
		if(*dRR == 0xFF) err = reg_modified(*tRR);
		else if(size == 4) emit("\tmov     r%d, r%d\n", *dRR, *tRR);
		else emit("\tbfi     r%d, r%d, #0, #%d\n", *dRR, *tRR, (size * 8));
		break;

	case EA_AINC:
		if(size == 4) emit("\tror     r%d, #16\n", *tRR);
		emit("\t%s    r%d, [r%d], #%d\n", stx(size), *tRR, *dRR, size);
		err = reg_modified(*dRR);
		break;

	case EA_ADDR:
	case EA_ADEC:
	case EA_AIDX:
	case EA_ADIS:
	case EA_ABSW:
	case EA_ABSL:
		if(size == 4) emit("\tror     r%d, #16\n", *tRR);
		sprintf( buffer, _ea, *dRR );
		emit("\t%s    r%d, %s\n", stx(size), *tRR, buffer);
		break;
	} // end switch

 	reg_flush();
	return err;
}
