#include "m68k_common.h"
#include "m68k_cpu.h"

reg_map_t reg_map[REG_MAP_COUNT];

#define SET_EA_MODES() \
	uint16_t sEA  = (opcode & 0x0038) >> 3; \
	uint16_t dEA  = (opcode & 0x01C0) >> 6; \
	/* extend address modes in implicit modes */\
	if(sEA == 7) sEA += (opcode & 0x0007) >> 0; \
	if(dEA == 7) dEA += (opcode & 0x0E00) >> 9 \

// see if there's a later opcode that this will be a copy of
uint16_t alias_mode(uint16_t opcode) {
	SET_EA_MODES();
	
	if(dEA > EA_ABSW) 		opcode &= 0xF1FF; // change to EA_ABSW
	else if(dEA == EA_ADIS) opcode ^= 0x00C0; // change to EA_AIDX

	if(sEA == EA_IMMD)      return opcode;
	else if(sEA >  EA_ABSW)	return opcode & 0xFFF8; // change to EA_ABSW
	else if(sEA == EA_ADIS) return opcode ^ 0x0018; // change to EA_AIDX
	else					return opcode;
}

static int illegal_opcode(uint16_t opcode) {
	uint16_t size = (opcode & 0x3000) >> 12;
	SET_EA_MODES();
	
	// EA out of range; not valid
	if(sEA > EA_IMMD) return -1; 
	// MOVE *,PC or MOVE *,#imm not valid
	if(dEA >= EA_PDIS) return -1; 
	// MOVEA.B *,An and MOVE.B An,* not valid
	if(((sEA == 1) || (dEA == 1)) && (size == MOVE_BYTE)) return -1; 
	
	return 0;
}

static int emit_move(uint16_t opcode) {
	// reject invalid operations
	assert(opcode >= 0x1000 || opcode < 0x4000);
	if(illegal_opcode(opcode)) return -1;

	uint16_t alias = alias_mode(opcode);
	if(illegal_opcode(alias)) return -1;
	else if(alias != opcode) return -alias;
	
	// get our effective addressing modes and operation size
	uint16_t size = (opcode & 0x3000) >> 12;
	SET_EA_MODES();

	if(debug) printf("sEA %d dEA %d\n", sEA, dEA);

	// registers are simply 0 thru 15
	int8_t sR = ((opcode & 0x0007) >> 0) | (sEA ? 0x8 : 0x0);
	int8_t dR = ((opcode & 0x0E00) >> 9) | (dEA ? 0x8 : 0x0);
	
	// move a handles 16-bits differently and doesn't set flags
	uint16_t movea = (dEA == 1);

	// convert the operand size to the actual number of bytes		
	if(size == 3) size = 2; else if(size == 2) size = 4; else size = 1;

	// okay, we're committed to this opcode!
	// are these aliased?
	//emit("%s", alias_modes(opcode));

	// skip pointless operation
	if((sR != dR) || (sEA != EA_AREG) || (dEA == EA_AREG)) {
		// guard r1 or r2 if the EA is indexed mode
		if(sEA >= EA_ADIS) reg_alloc_arm(1);
		if(dEA >= EA_ADIS) reg_alloc_arm(2);
	
		// determine our source and destination real registers
		int8_t sRR = emit_get_reg( &sEA, sR, 1, 0 );
	
		// if the destination is a register, then skip loading it
		int8_t dRR = emit_get_reg( &dEA, dR, 2, dEA <= EA_AREG );
	
		// Perform the load unless this is an immediate or register already
		sRR = emit_load(sEA, -1, sRR, 1, 1, size, !movea);
	
		// Compute our condition codes
		if(!movea && (size < 4)) {
			int8_t treg = reg_alloc_temp();
			emit("\tlsls    r%d, r%d, #%d \n", treg, sRR, (4-size) * 8);
			reg_free(treg);
		}
			
		// Move from register-to-register
		if((dEA <= EA_AREG)) {
			switch(size + movea) {
			case 1: emit("\tbfi     r%d, r%d, #0, #8\n", dRR, sRR); break;
			case 2: emit("\tbfi     r%d, r%d, #0, #16\n", dRR, sRR); break;
			case 3: emit("\tsxth    r%d, r%d\n", dRR, sRR); break;
			case 4: emit("\tmovs    r%d, r%d\n", dRR, sRR); break; 
			case 5: emit("\tmov     r%d, r%d\n", dRR, sRR); break;
			}
			if(dRR != sRR) reg_modified(dRR);
		} 
	
		// Or perform a memory store
		else {
			emit_store(dEA, dRR, sRR, 2, size);
		}
	}		
	
	// everything's done, let's return and free all the registers
	reg_flush();
	
	uint16_t r = lines;
	
	if((sEA == EA_ADIS) || (sEA == EA_PDIS) || (sEA == EA_ABSW) || ((sEA == EA_IMMD) && (size < 4)))
		r |= EXT_WORD_SRC_16B;
	else if((sEA == EA_ABSL) || ((sEA == EA_IMMD) && (size == 4)))
		r |= EXT_WORD_SRC_32B;
	else if((sEA == EA_AIDX) || (sEA == EA_PIDX))
		r |= EXT_WORD_SRC_EXT;
	
	if((dEA == EA_ADIS) || (dEA == EA_PDIS) || (dEA == EA_ABSW) || ((dEA == EA_IMMD) && (size < 4)))
		r |= EXT_WORD_DST_16B;
	else if((dEA == EA_ABSL) || ((dEA == EA_IMMD) && (size == 4)))
		r |= EXT_WORD_DST_32B;
	else if((dEA == EA_AIDX) || (dEA == EA_PIDX))
		r |= EXT_WORD_DST_EXT;
	
	return r;
}

int emit_MOVEB(char *buffer, uint16_t opcode) {
	lines = 0;
	stream( buffer, 0, STREAM_SET );
	return emit_move(opcode);
}

int emit_MOVEW(char *buffer, uint16_t opcode) {
	lines = 0;
	stream( buffer, 0, STREAM_SET );
	return emit_move(opcode);
}

int emit_MOVEL(char *buffer, uint16_t opcode) {
	lines = 0;
	stream( buffer, 0, STREAM_SET );
	return emit_move(opcode);
}






