#include "m68k_common.h"
#include "m68k_cpu.h"
#include "m68k_emit_ea.h"
#include "m68k_registers.h"

// 0101ddd0ssmmmxxx addq
// 0101ddd1ssmmmxxx subq
//              --- reg
//           ---    ea
//         --       size
//        -         add/sub
//     ---          value (1-7)


int emit_ADDQ(char *buffer, uint16_t opcode) {
	char EA[32];
	uint16_t size = 1 << ((opcode & 0x00C0) >> 6);
	if(size > 4) return -1; // no such size
	
	uint16_t dEA  = (opcode & 0x0038) >> 3;
	if(dEA == 7) dEA += (opcode & 0x0007) >> 0;

	// ADDQ *,PC or ADDQ *,#imm not valid
	if(dEA >= EA_PDIS) return -1; 
	// ADDQ.B *.An is not valid	
	if((dEA == EA_AREG) && (size == 1)) return -1; 

	// Adjust aliases
	// aluq.w is same as aluq.l for An
	if((dEA == EA_AREG) && (size == 2)) return -(opcode & 0x00C0); 
	if(dEA >  EA_ABSW) return -(opcode & 0xFFF8); // change to EA_ABSW
	if(dEA == EA_ADIS) return -(opcode ^ 0x0018); // change to EA_AIDX
	
	uint8_t dR = ((opcode & 0x0007) >> 0) | (dEA ? 0x8 : 0x0);
	uint8_t dRR, tRR;
	
	//fprintf( stderr, "%04hX ", opcode );
	lines = 0;
	emit_reset( buffer );

	// get the 'quick' integer
	uint8_t val = (opcode >> 9) & 7;
	if(val == 0) val = 8;

	if(reg_raw(dR) == 0xFF) {
		reg_alloc_arm(0);
		emit("\tmov     r%d, #%d\n", 0, val);
		sprintf(EA, "r%d", 0);

	} else {
		sprintf(EA, "#%d", val);
	}

	get_destination_data( &dRR, &tRR, dEA, dR, size );
	
	if(dEA == EA_AREG) {
		if(opcode & 0x0100) { // subq
			emit("\tsub     r%d, r%d, %s\n", tRR, tRR, EA);		

		} else { // addq
			emit("\tadd     r%d, r%d, %s\n", tRR, tRR, EA);		
		}
		
	} else {
		if(opcode & 0x0100) { // subq
			emit("\trsbs    r%d, r%d, %s\n", tRR, tRR, EA);		
			emit("\trsb     r%d, #0\n", tRR);

		} else { // addq
			emit("\tadds    r%d, r%d, %s\n", tRR, tRR, EA);		
		}
	}
	
	set_destination_data( &dRR, &tRR, dEA, size );
	reg_flush();

	return lines_ext(lines, 0, dEA, size);
}

int emit_SUBQ(char *buffer, uint16_t opcode) {
	return emit_ADDQ(buffer, opcode);
}

// 0111nnn0dddddddd
//         -------- 8-bit sign-extended data
//     ---          Data register

int emit_MOVEQ(char *buffer, uint16_t opcode) {
	char EA[32];

	lines = 0;
	emit_reset( buffer );

	uint8_t dR = (opcode & 0x0E00) >> 9;
	int8_t dVal = (int8_t)(uint8_t)(opcode & 0xFF);
	uint8_t dRR;

	if(dVal >= 0) sprintf(EA, "#0x%02x", dVal);
	else sprintf(EA, "#0x%02x", (uint8_t)(~dVal));

	if(reg_raw(dR) == 0xFF) {
		reg_alloc_arm(0);
		if(dVal >= 0) emit("\tmov     r0, %s\n", EA);
		else emit("\tmvn     r0, %s\n", EA);
		sprintf(EA, "r0");

	}

	if(!emit_get_reg( &dRR, dR, 4 )) return -1;

	if(dVal >= 0)
		emit("\tmov     r%d, %s\n", dRR, EA);
	else 
		emit("\tmvn     r%d, %s\n", dRR, EA);

	reg_modified(dRR);
	reg_flush();

	return lines;	
}






