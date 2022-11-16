#include "m68k_common.h"
#include "m68k_cpu.h"
#include "m68k_emit_ea.h"
#include "m68k_registers.h"

int emit_store_EA(char *buffer, uint16_t opcode, int is_pea) {
	uint16_t sEA = (opcode & 0x0038) >> 3; 	
	if(sEA == 7) sEA += opcode & 0x0007;

	// EA of registers or immediate makes no sense
	if(sEA <= EA_AREG || sEA == EA_IMMD) return -1;
	// we also don't allow auto-incrementing modes
	else if(sEA == EA_AINC || sEA == EA_ADEC) return -1;
	// handle common aliases
	else if(sEA >  EA_ABSW)	return -(opcode & 0xFFF8); // change to EA_ABSW
	else if(sEA == EA_ADIS) return -(opcode ^ 0x0018); // change to EA_AIDX

	uint8_t sR = (opcode & 0x0007) | 8;
	uint8_t dR = ((opcode & 0x0E00) > 9) | 8;

	// LEA (Ax), Ay -> MOVEA.L Ax, Ay
	if(!is_pea && (sEA == EA_ADDR)) {
		return -((opcode & 0x0E07) | 0x2048);
	}
 
	lines = 0;
	emit_reset( buffer );	

	reg_alloc_arm(1);
	uint8_t dRR, sRR;
	if(!emit_get_reg( &sRR, sR, 4 )) return -1;

	if(!is_pea) reg_alloc_68k( &dRR, dR, 4 );
	//else reg_alloc_temp(&dRR);

	switch(sEA) {
	case EA_ADDR:
		if(is_pea) 
			emit("\tstr     r%d, [" CPU ", #-4]!\n", sRR);
		else 
			emit("\tmov     r%d, r%d\n", dRR, sRR);
		break;
	case EA_AIDX: case EA_ADIS:
		if(is_pea) {
			emit("\tadd     r1, r1, r%d\n", sRR);
			emit("\tstr     r1, [" CPU ", #-4]!\n");
		} else {
			emit("\tadd     r%d, r1, r%d\n", dRR, sRR);
		}
		break;

	case EA_ABSW: case EA_ABSL: case EA_PDIS: case EA_PIDX:
		if(is_pea)
			emit("\tstr     r1, [" CPU ", #-4]!\n");
		else
			emit("\tmov     r%d, r1\n", dRR);
		break;
	}
// mov
	reg_flush();
	return lines_ext(lines, sEA, 0, 4 );
}
int emit_LEA(char *buffer, uint16_t opcode) {
	return emit_store_EA( buffer, opcode, 0 );
}
int emit_PEA(char *buffer, uint16_t opcode) {
	return emit_store_EA( buffer, opcode, 1 );
}

int emit_LINK(char *buffer, uint16_t opcode) {
	// 0100111001010yyy  word
	// SP – 4 → SP; An → (SP); SP → An; SP + dn → SP
	lines = 0;
	emit_reset( buffer );	
	uint8_t dRR, dR = ((opcode & 0x0007) >> 0) |  0x8;
	reg_alloc_arm(1);
	if(!emit_get_reg( &dRR, dR, 4 )) return -1;
	emit("\tstr     r%d, [sp, #-4]!\n", dRR );
	emit("\tmov     r%d, sp\n", dRR);
	emit("\tadd     sp, r1\n");
	reg_modified( dRR );
	reg_flush();	

	return lines | EXT_WORD_SRC_16B;
}
int emit_UNLK(char *buffer, uint16_t opcode) {
	// 0100111001011nnn
	// An → SP; (SP) → An; SP + 4 → SP
	lines = 0;
	emit_reset( buffer );	

	uint8_t dRR, dR = ((opcode & 0x0007) >> 0) |  0x8;
	if(!emit_get_reg( &dRR, dR, 4 )) return -1;
	emit("\tmov     sp, r%d\n", dRR);
	emit("\tldr     r%d, [sp], #4\n", dRR );
	reg_flush();		
	return lines;
}

int emit_SWAP(char *buffer, uint16_t opcode) {
	lines = 0;
	emit_reset( buffer );	

	uint8_t dRR, dR = (opcode & 0x0007) >> 0;
	if(!emit_get_reg( &dRR, dR, 4 )) return -1;
	emit("\trors    r%d, r%d, #16\n", dRR, dRR);
	reg_modified(dRR);
	reg_flush();
	return lines;	
}
int emit_EXT(char *buffer, uint16_t opcode) {
	lines = 0;
	emit_reset( buffer );	
	
	uint8_t dRR, dR = (opcode & 0x0007) >> 0;
	//uint8_t dRR = emit_get_reg( &dEA, dR, 1, 0 );
	if(!emit_get_reg( &dRR, dR, 4 )) return -1;
	
	if(opcode & 0x0040) {
		// EXT.L extend word to long
		emit("\tsxth    r%d, r%d\n", dRR, dRR);
		emit("\tcmp     r%d, #0\n", dRR, dRR);
	
	} else {
		// EXT.W extend byte to word
		uint8_t tRR;
		reg_alloc_temp(&tRR);
		emit("\tsxtb    r%d, r%d\n", tRR, dRR);
		emit("\tcmp     r%d, #0\n", tRR);
		emit("\tbfi     r%d, r%d, #0, #16\n", dRR, tRR);				
	}
	
	reg_modified(dRR);
	reg_flush();
	
	return lines;	
}
int emit_EXG(char *buffer, uint16_t opcode) {
	lines = 0;
	emit_reset( buffer );	

	uint8_t ry = (opcode & 0x0007) >> 0;
	uint8_t rx = (opcode & 0x0E00) >> 9;

	switch(opcode & 0x00F8) {
	case 0x0048: // Ax <-> Ay
		rx |= 0x8;
	case 0x0088: // Dx <-> Ay
		ry |= 0x8;
	case 0x0040: // Dx <-> Dy
		break;
	default:
		return -1;
	}

	if(rx == ry) return 0;
	
	//printf("Rx: %02x  Ry: %02x  (Opcode %04x)\n", rx, ry, opcode);

	// memory <-> cpu reg? use SWP
	// SWP only allows simple direct addressing
	if((reg_raw(rx) == 0xFF) && (reg_raw(ry) == 0xFF)) {
		// both in memory
		emit("\tldr     r0, [" CPU ", #%d]\n", rx * 4);
		emit("\tldr     r1, [" CPU ", #%d]\n", ry * 4);
		emit("\tstr     r1, [" CPU ", #%d]\n", rx * 4);
		emit("\tstr     r0, [" CPU ", #%d]\n", ry * 4);

	} else if((reg_raw(rx) == 0xFF) && (reg_raw(ry) != 0xFF)) {
		// rx is memory, ry is in ARM
		uint8_t dRR = reg_raw(ry);
		uint8_t tmp; 
		reg_alloc_temp(&tmp);
		emit("\tldr     r%d, [" CPU ", #%d]\n", tmp, rx * 4);
		emit("\tstr     r%d, [" CPU ", #%d]\n", dRR, rx * 4);
		emit("\tmov     r%d, r%d\n", dRR, tmp);
		reg_free(tmp);


		//if(rx) emit("\tadd     r0, " CPU ", #%d\n", rx * 4);
		//emit("\tswp     r%d, r%d, [r0]\n", dRR, dRR);

	} else if((reg_raw(rx) != 0xFF) && (reg_raw(ry) == 0xFF)) {
		// ry is memory, rx is in arm
		uint8_t dRR = reg_raw(rx);
		uint8_t tmp; 
		reg_alloc_temp(&tmp);
		emit("\tldr     r%d, [" CPU ", #%d]\n", tmp, ry * 4);
		emit("\tstr     r%d, [" CPU ", #%d]\n", dRR, ry * 4);
		emit("\tmov     r%d, r%d\n", dRR, tmp);
		reg_free(tmp);


		// if(ry) emit("\tadd     r0, " CPU ", #%d\n", ry * 4);
		// emit("\tswp     r%d, r%d, [r0]\n", dRR, dRR);

	} else {
		// both in ARM registers

		uint8_t rry, rrx;
		reg_alloc_68k( &rry, ry, 4 );
		reg_alloc_68k( &rrx, rx, 4 );

		emit("\tmov     r0, r%d\n", rry);
		emit("\tmov     r%d, r%d\n", rry, rrx);
		emit("\tmov     r%d, r0\n", rrx);

		reg_modified(rry);
		reg_modified(rrx);
	}
	reg_flush();
	
	return lines;	
		
}

int emit_CLR(char *buffer, uint16_t opcode) {
	uint8_t dR, dRR, tRR = 0;
	uint16_t dEA;
	
	uint16_t size = 1 << ((opcode & 0x00C0) >> 6);
	if(size > 4) return -1;
	
	dR = opcode & 7; dEA = (opcode >> 3) & 7; 
	if(dEA == 7) dEA += dR; if(dEA) dR += 8;
		
	if(dEA == EA_AREG || dEA >= EA_PDIS) return -1;
	else if(dEA >  EA_ABSW)	return -(opcode & 0xFFF8); // change to EA_ABSW
	else if(dEA == EA_ADIS) return -(opcode ^ 0x0018); // change to EA_AIDX
	else if(size == 4 && dEA == EA_DREG) return -(0x7000 | (dR << 9)); // change to moveq.l #0,dn
	lines = 0;
	emit_reset( buffer );
	reg_alloc_arm( 0 );

	get_destination_data( &dRR, NULL, dEA, dR, size );
	emit("\tmov     r0, #0\n", tRR);
	set_destination_data( &dRR, &tRR, dEA, size & 3 );

	return lines_ext(lines, 0, dEA, size );
}
int emit_NEG(char *buffer, uint16_t opcode) {
	uint8_t dR, dRR, tRR;
	uint16_t dEA;
	
	uint16_t size = 1 << ((opcode & 0x00C0) >> 6);
	if(size > 4) return -1;
	
	dR = opcode & 7; dEA = (opcode >> 3) & 7; 
	if(dEA == 7) dEA += dR; if(dEA) dR += 8;
		
	if(dEA == EA_AREG || dEA >= EA_PDIS) return -1;
	else if(dEA >  EA_ABSW)	return -(opcode & 0xFFF8); // change to EA_ABSW
	else if(dEA == EA_ADIS) return -(opcode ^ 0x0018); // change to EA_AIDX
	
	lines = 0;
	emit_reset( buffer );	

	get_destination_data( &dRR, &tRR, dEA, dR, size );
	emit("\trsbs    r%d, #0\n", tRR);
	set_destination_data( &dRR, &tRR, dEA, size );

	return lines_ext(lines, 0, dEA, size );
}
int emit_NOT(char *buffer, uint16_t opcode) {
	uint8_t dR, dRR, tRR;
	uint16_t dEA;
	
	uint16_t size = 1 << ((opcode & 0x00C0) >> 6);
	if(size > 4) return -1;
	
	dR = opcode & 7; dEA = (opcode >> 3) & 7; 
	if(dEA == 7) dEA += dR; if(dEA) dR += 8;
		
	if(dEA == EA_AREG || dEA >= EA_PDIS) return -1;
	else if(dEA >  EA_ABSW)	return -(opcode & 0xFFF8); // change to EA_ABSW
	else if(dEA == EA_ADIS) return -(opcode ^ 0x0018); // change to EA_AIDX
	
	lines = 0;
	emit_reset( buffer );	

	get_destination_data( &dRR, &tRR, dEA, dR, size );
	emit("\tmvns    r%d, r%d\n", tRR, tRR);
	set_destination_data( &dRR, &tRR, dEA, size );

	return lines_ext(lines, 0, dEA, size );
}

int emit_CHK(char *buffer, uint16_t opcode) {
	uint8_t sR, sRR, dR, dRR;
	uint16_t sEA;

	sR = opcode & 7; sEA = (opcode >> 3) & 7; 
	if(sEA == 7) sEA += sR; if(sEA) sR += 8;
	dR = (opcode >> 9) & 8;
	
	if(sEA == EA_AREG || sEA > EA_ABSL) return -1;
	else if(sEA >  EA_ABSW)	return -(opcode & 0xFFF8); // change to EA_ABSW
	else if(sEA == EA_ADIS) return -(opcode ^ 0x0018); // change to EA_AIDX
	
	lines = 0;
	emit_reset( buffer );	

	get_source_data( &sRR, sEA, sR, 2 );
	if(!emit_get_reg( &dRR, dR, 2 )) return -1;
		
	emit("\tcmp     r%d, #0\n", dRR);
	emit("\tblt     0f\n");
	emit("\tcmp     r%d, r%d\n", dRR, sRR);
	emit("\tbxle    lr\n");
	emit("0:  svc     #%d\n", TRAPCHK);

	reg_flush();		
	return lines_ext(lines, sEA, 0, 2 );
}
int emit_TAS(char *buffer, uint16_t opcode) {
	uint8_t dR, dRR, tRR;
	uint16_t dEA;

	dR = opcode & 7; dEA = (opcode >> 3) & 7; 
	if(dEA == 7) dEA += dR; if(dEA) dR += 8;

	if(dEA == EA_AREG || dEA >= EA_PDIS) return -1;
	else if(dEA >  EA_ABSW)	return -(opcode & 0xFFF8); // change to EA_ABSW
	else if(dEA == EA_ADIS) return -(opcode ^ 0x0018); // change to EA_AIDX
	
	lines = 0;
	emit_reset( buffer );
	
	emit("\tcpsid   if\n"); // make atomic
	get_destination_data( &dRR, &tRR, dEA, dR, 1 );
	emit("\tcmp     r%d, #0\n", tRR);		
	emit("\torr     r%d, r%d, #0x80\n", tRR, tRR);
	set_destination_data( &dRR, &tRR, dEA, 1 );
	emit("\tcpsie   if\n");
	return lines_ext(lines, 0, dEA, 1 );
}
int emit_TST(char *buffer, uint16_t opcode) {
	uint8_t sR, sRR;
	uint16_t sEA;

	uint16_t size = 1 << ((opcode & 0x00C0) >> 6);
	if(size > 4) return -1;

	sR = opcode & 7; sEA = (opcode >> 3) & 7; 
	if(sEA == 7) sEA += sR; if(sEA) sR += 8;

	if((sEA == EA_AREG) || (sEA >= EA_PDIS)) return -1;

	if(sEA == EA_ABSL)	return -(opcode & 0xFFF8); // change to EA_ABSW
	else if(sEA == EA_ADIS) return -(opcode ^ 0x0018); // change to EA_AIDX
	
	lines = 0;
	emit_reset( buffer );
	
	get_source_data( &sRR, sEA, sR, size );
	emit("\tcmp     r%d, #0\n", sRR);
	reg_flush();	
	return lines_ext(lines, sEA, 0, size );
}

