#include "m68k_common.h"
#include "m68k_cpu.h"
#include "m68k_emit_ea.h"
#include "m68k_registers.h"

// BTST Dn,<ea>
// BTST #Immd,<ea>


int emit_bits(char* buffer, uint16_t opcode) {
	uint16_t dEA  = (opcode & 0x0038) >> 3;
	uint16_t size = (dEA > EA_AREG) ? 1 : 4;
	uint8_t sRR, dRR, dR, tRR;
	uint16_t sEA = 0;
	
	dR = opcode & 7; dEA = (opcode >> 3) & 7; 
	if(dEA == 7) dEA += dR; if(dEA) dR += 8;
	
	if((dEA == EA_AREG) || (dEA >= EA_PDIS)) return -1; 
	else if(dEA > EA_ABSW) return -(opcode & 0xFFF8); // change to EA_ABSW
	else if(dEA == EA_ADIS) return -(opcode ^ 0x0018); // change to EA_AIDX

	lines = 0;
	emit_reset( buffer );

	// get source data
	if(opcode & 0x0100) {
		uint8_t sR = ((opcode & 0x0E00) >> 9);
		if(!emit_get_reg( &sRR, sR, 4 )) return -1;

	} else {
		reg_alloc_arm(1);
		sRR = 1;
	}
	// convert to bit position
	reg_alloc_temp(&tRR);
	emit("\tmov     r%d, #1\n", tRR);
	emit("\tlsl     r%d, r%d, r%d\n", tRR, tRR, sRR);
	reg_free(sRR); sRR = tRR;
		
	get_destination_data( &dRR, &tRR, dEA, dR, size );

	// test the bit
	emit("\ttst     r%d, r%d\n", tRR, sRR);
	
	if(opcode & 0x00C0) {
		switch(opcode & 0x00C0) {
		case 0x0040: // BCHG
			emit("\teor     r%d, r%d, r%d\n", tRR, tRR, sRR);
			break;
		case 0x0080: // BCLR
			emit("\tbic     r%d, r%d, r%d\n", tRR, tRR, sRR);
			break;
		case 0x00C0: // BSET
			emit("\torr     r%d, r%d, r%d\n", tRR, tRR, sRR);
			break;
		}
		set_destination_data( &dRR, &tRR, dEA, size );
	} else {
		reg_flush();
	}
	return lines_ext(lines, sEA, dEA, 2);	

}

int emit_BTSTI(char *buffer, uint16_t opcode) { return emit_bits(buffer, opcode); }
int emit_BCHGI(char *buffer, uint16_t opcode) { return emit_bits(buffer, opcode); }
int emit_BCLRI(char *buffer, uint16_t opcode) { return emit_bits(buffer, opcode); }
int emit_BSETI(char *buffer, uint16_t opcode) { return emit_bits(buffer, opcode); }
int emit_BTST(char *buffer, uint16_t opcode) { return emit_bits(buffer, opcode); }
int emit_BCHG(char *buffer, uint16_t opcode) { return emit_bits(buffer, opcode); }
int emit_BCLR(char *buffer, uint16_t opcode) { return emit_bits(buffer, opcode); }
int emit_BSET(char *buffer, uint16_t opcode) { return emit_bits(buffer, opcode); }

