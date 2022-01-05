#include "m68k_common.h"
#include "m68k_cpu.h"
#include "m68k_emit_ea.h"
#include "m68k_registers.h"

// const static char* alu_ops[ALU_OP_COUNT] = {
// 	"orrs", "ands", "rsbs", "adds", "eors", "cmp "
// }

// source is always 16 or 32-bit immediate
// for destination, register-direct, pc-relative and immediate is not allowed

int emit_alu(char *buffer, uint16_t size, uint16_t sEA, uint16_t dEA, ALU_OP_t alu_op) {
	uint8_t dR, dRR, sR, sRR, tRR;

	if(size == 3 || size > 4) return -1;
	
	if(debug) printf("@ emit_alu, sEA %02X, dEA %02X, size %d\n", sEA, dEA, size);

	sR = sEA & 7; sEA = (sEA >> 3) & 7; if(sEA == 7) sEA += sR; if(sEA) sR += 8;
	dR = dEA & 7; dEA = (dEA >> 3) & 7; if(dEA == 7) dEA += dR; if(dEA) dR += 8;
	
	// Universal EA exceptions
	// Cannot be byte and an address register
	if((size == 1) && (sEA == EA_AREG)) return -1;
	if((size == 1) && (dEA == EA_AREG)) return -1;
	// PC-relative and Immediate are disallowed as destinations
	if((dEA == EA_PDIS) || (dEA == EA_PIDX) || (dEA == EA_IMMD)) return -1;
	// Exclude obvious not-real addressing modes above immediate
	if((sEA > EA_IMMD) || (dEA > EA_IMMD)) return -1;
	
	// Guard standard aliasing of addressing modes -- this is an error
	// Source PC-Relative should be in R1 as an absolute
	// Both ABS.W and ABS.L should also be in R1 as an absolute
	// And finally, indexed and displacement are the same modes
	//if((sEA == EA_ABSW) || (sEA == EA_ADIS) || (sEA == EA_PDIS) || (sEA == EA_PIDX)) return -1;
	//printf("@ %d\n", __LINE__);
	// Destination PC modes are already illegal, but R2
	//if((dEA == EA_ABSW) || (dEA == EA_ADIS)) return -1;
	//printf("@ %d\n", __LINE__);
	
	char flags = (dEA == EA_AREG) ? ' ' : 's';

	lines = 0;
	emit_reset( buffer );
	
	// using the sEA and size, get the data into sRR
	get_source_data( &sRR, sEA, sR, size );
	
	// using the dEA and size, get the destination data
	// into tRR and the return address to write to in dRR
	if(alu_op == ALU_OP_MOVE) {
		get_destination_data( &dRR, NULL, dEA, dR, size );		
	} else {
		get_destination_data( &dRR, &tRR, dEA, dR, size );	
		if(sRR < 4) { uint8_t _t = sRR; sRR = tRR; tRR = _t; }
	}
	
		
	switch(alu_op) {
	case ALU_OP_OR:  	emit("\torr%c    r%d, r%d, r%d\n", flags, tRR, tRR, sRR); break;
	case ALU_OP_AND: 	emit("\tand%c    r%d, r%d, r%d\n", flags, tRR, tRR, sRR); break;
	case ALU_OP_ADD: 	emit("\tadd%c    r%d, r%d, r%d\n", flags, tRR, tRR, sRR); break;
	case ALU_OP_EOR: 	emit("\teor%c    r%d, r%d, r%d\n", flags, tRR, tRR, sRR); break;
	case ALU_OP_SUB: 	emit("\trsb%c    r%d, r%d, r%d\n", flags, tRR, tRR, sRR);
						emit("\trsb     r%d, #0\n", tRR);
						break;
	case ALU_OP_CMP:
						emit("\tcmp     r%d, r%d\n", tRR, sRR);
						break;
	case ALU_OP_MOVE:
		if(dEA != EA_AREG) {
			if(size < 4) emit("\tcmp     r%d, #0\n", sRR);
			else {
				// emit_fixup("ror ", "rors");
				char* ror = strstr( buffer, "ror ");
				if(ror) memcpy(ror, "rors", 4);
			}
		}
		{ uint8_t _t = tRR; tRR = sRR; sRR = _t; } // SWAP
		break;
	} // end switch
	reg_free(sRR);
	
	set_destination_data( &dRR, &tRR, dEA, size );
	
	return lines_ext(lines, sEA, dEA, size);
}

static int emit_immd(char *buffer, uint16_t opcode, ALU_OP_t op) {
	uint16_t sEA = 0b111100, dEA = (opcode & 0x3F);
	
	// no address register here
	if((dEA & 0x38) == 0x08) return -1;
	// standard substitutions
	if(dEA == 0x39) return -(opcode & 0xFFF8); // change EA_ABSL to EA_ABSW
	if((dEA & 0x38) == 0x28) return -(opcode ^ 0x0018); // change to EA_AIDX
	uint16_t size = 1 << ((opcode & 0x00C0) >> 6);
	return emit_alu(buffer, size, sEA, dEA, op);
}
int emit_ORI(char *buffer, uint16_t opcode) {  return emit_immd(buffer, opcode, ALU_OP_OR  ); }
int emit_ANDI(char *buffer, uint16_t opcode) { return emit_immd(buffer, opcode, ALU_OP_AND ); }
int emit_SUBI(char *buffer, uint16_t opcode) { return emit_immd(buffer, opcode, ALU_OP_SUB ); }
int emit_ADDI(char *buffer, uint16_t opcode) { return emit_immd(buffer, opcode, ALU_OP_ADD ); }
int emit_EORI(char *buffer, uint16_t opcode) { return emit_immd(buffer, opcode, ALU_OP_EOR ); }
int emit_CMPI(char *buffer, uint16_t opcode) { return emit_immd(buffer, opcode, ALU_OP_CMP ); }

// for source, all ALU ops allow all addressing modes
// for destination, register-direct, pc-relative and immediate is not allowed

// for OR, SUB, AND and ADD, direction is bit 0x0100
// for all other ALU operations, direction is fixed
// Dn = Dn <op> EA (source)

// EOR does not allow Ax as a source

// Ax as a register is otherwise prohibited only with byte operations
static int emit_ea(char *buffer, uint16_t opcode, ALU_OP_t alu_op) {
	uint16_t dEA, sEA;
	uint16_t size = 1 << ((opcode & 0x00C0) >> 6);

	int m2r;
	
	if(alu_op == ALU_OP_EOR) m2r = false;
	else if(alu_op == ALU_OP_CMP) m2r = true;
	else m2r = !(opcode & 0x0100);

	if(m2r) {
		dEA = (opcode & 0x0E00) >> 9;
		sEA = (opcode & 0x003F) >> 0;
		// no address register here
		if((size == 1) && (sEA & 0x38) == 0x08) return -1;

		// standard substitutions
		if(sEA == 0x39) return -(opcode & 0xFFF8); // change EA_ABSL to EA_ABSW
		if((sEA & 0x38) == 0x28) return -(opcode ^ 0x0018); // change to EA_AIDX
// 		if((sEA == 0x3C) && (opcode != 0x823C)) {
// 			if(debug) printf("@ alu -> alui optimization\n");
// 			// or  -> ori 
// 			if((opcode & 0xF000) == 0x8000) 
// 				return -((opcode & 0xC0) | ((opcode & 0xE00) >> 9) | 0x0000);
// 			// and -> andi
// 			if((opcode & 0xF000) == 0xC000) 
// 				return -((opcode & 0xC0) | ((opcode & 0xE00) >> 9) | 0x0200);
// 			// sub -> subi
// 			if((opcode & 0xF000) == 0x9000) 
// 				return -((opcode & 0xC0) | ((opcode & 0xE00) >> 9) | 0x0400);
// 			// add -> addi
// 			if((opcode & 0xF000) == 0xD000) 
// 				return -((opcode & 0xC0) | ((opcode & 0xE00) >> 9) | 0x0600);
// 			// cmp -> cmpi
// 			if((opcode & 0xF100) == 0xB000) 
// 				return -((opcode & 0xC0) | ((opcode & 0xE00) >> 9) | 0x0C00);
// 			//
// 			return -1;
// 		}
		
		if(debug) printf("@ R = R op EA (%04X)\n", (opcode & 0x0100));
	} else {
		sEA = (opcode & 0x0E00) >> 9;	
		dEA = (opcode & 0x003F) >> 0;
		// no registers of ANY kind here
		//printf("@!! %d\n", alu_op);
		if((alu_op != ALU_OP_EOR) && (dEA <= 0x0F)) return -1;
		// standard substitutions
		if(dEA == 0x39) return -(opcode & 0xFFF8); // change EA_ABSL to EA_ABSW
		if((dEA & 0x38) == 0x28) return -(opcode ^ 0x0018); // change to EA_AIDX
			
		if(debug) printf("@ EA = EA op R (%04X)\n", (opcode & 0x0100));
	}
	return emit_alu(buffer, size, sEA, dEA, alu_op);
}
int emit_OR(char *buffer, uint16_t opcode) {  return emit_ea(buffer, opcode, ALU_OP_OR  ); }
int emit_AND(char *buffer, uint16_t opcode) { return emit_ea(buffer, opcode, ALU_OP_AND ); }
int emit_SUB(char *buffer, uint16_t opcode) { return emit_ea(buffer, opcode, ALU_OP_SUB ); }
int emit_ADD(char *buffer, uint16_t opcode) { return emit_ea(buffer, opcode, ALU_OP_ADD ); }
int emit_EOR(char *buffer, uint16_t opcode) { return emit_ea(buffer, opcode, ALU_OP_EOR ); }
int emit_CMP(char *buffer, uint16_t opcode) { return emit_ea(buffer, opcode, ALU_OP_CMP ); }

int emit_CMPM(char *buffer, uint16_t opcode) {
	// 1011xxx1ss001yyy
	//              --- Ay
	//         --       Size
	//     ---          Ax
	// CMPM (Ay)+,(Ax)+

	uint16_t sEA = 0x10 | (opcode & 0xF);
	uint16_t dEA = 0x10 | ((opcode & 0x1E00) >> 9);
	uint16_t size = 1 << ((opcode & 0x00C0) >> 6);
	
	return emit_alu(buffer, size, sEA, dEA, ALU_OP_CMP);
}

static int emit_addr(char *buffer, uint16_t opcode, ALU_OP_t alu_op) {
	uint16_t sEA = (opcode & 0x3F);
	uint16_t dEA = 0x08 | ((opcode & 0xE00) >> 9);
	
	// standard substitutions
	if(sEA == 0x39) return -(opcode & 0xFFF8); // change EA_ABSL to EA_ABSW
	if((sEA & 0x38) == 0x28) return -(opcode ^ 0x0018); // change to EA_AIDX
	
	uint16_t size = (opcode & 0x0100) ? 4 : 2;
	return emit_alu(buffer, size, sEA, dEA, alu_op);
}
int emit_CMPA(char *buffer, uint16_t opcode) { return emit_addr(buffer, opcode, ALU_OP_CMP ); }
int emit_SUBA(char *buffer, uint16_t opcode) { return emit_addr(buffer, opcode, ALU_OP_SUB ); }
int emit_ADDA(char *buffer, uint16_t opcode) { return emit_addr(buffer, opcode, ALU_OP_ADD ); }



