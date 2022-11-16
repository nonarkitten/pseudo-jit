#include "m68k_common.h"
#include "m68k_cpu.h"
#include "m68k_emit_ea.h"
#include "m68k_registers.h"

// load X and put it in the carry bit
static void get_x_flag(int is_sub) {
	uint8_t tRR;
	reg_alloc_temp(&tRR);	
	emit("\tldr     r%d, [" CPU ", #%d] @ get x flag into c flag\n", tRR, offsetof(cpu_t, x));
	emit("\trsbs    r%d, r%d, #%d\n", tRR, tRR, is_sub ? 1 : 0);
	reg_free(tRR);
}

// save X by copying the carry bit
static void set_x_flag(uint8_t tRR) {
	emit("\tmovcc   r%d, #0 @ save c flag to x\n", tRR);
	emit("\tmovcs   r%d, #1\n", tRR);
	emit("\tstr     r%d, [" CPU ", #%d]\n", tRR, offsetof(cpu_t, x));	
}

static int emit_add_sub_x(char *buffer, uint16_t opcode, int is_sub) {
	uint16_t size = 1 << ((opcode & 0x00C0) >> 6);
	if(size > 4) return -1;

	uint16_t EA = (opcode & 8) ? EA_ADEC : EA_DREG;
	
	uint8_t sRR, dRR, tRR, sR, dR;

	sR = (opcode & 0xE00) >> 9;
	dR = (opcode & 0x003) >> 0;
	
	lines = 0;
	emit_reset( buffer );	
	
	get_x_flag(is_sub); // put X into the C flag

	if(sR != dR) get_source_data( &sRR, EA, sR, size );
	//reg_alloc_temp( &tRR );
	get_destination_data( &dRR, &tRR, EA, dR, size );

	if(is_sub) {
		if(sR == dR) {
			emit("\tmovs    r%d, #0\n", tRR);
		} else {
			emit("\trscs    r%d, r%d, r%d\n", tRR, tRR, sRR);	
			emit("\trsb     r%d, #0\n", tRR);	
		}
	} else {
		if(sR == dR) {
			emit("\tadcs    r%d, r%d, r%d\n", tRR, tRR, tRR);
		} else {
			emit("\tadcs    r%d, r%d, r%d\n", tRR, tRR, sRR);
		}
	}
		
	set_destination_data( &dRR, &tRR, EA, size );	
	set_x_flag(tRR); // put C into the X flag
	return lines;	
}

int emit_SUBX(char *buffer, uint16_t opcode) { return emit_add_sub_x(buffer, opcode, 1); }
int emit_ADDX(char *buffer, uint16_t opcode) { return emit_add_sub_x(buffer, opcode, 0); }

// 0 - (Destination) - X -->; Destination
int emit_NEGX(char *buffer, uint16_t opcode) {
	uint16_t size = 1 << ((opcode & 0x00C0) >> 6);
	if(size > 4) return -1;
	
	uint16_t dEA = (opcode & 0x0038) >> 3; 	
	if(dEA == 7) dEA += (opcode & 0x0007) >> 0;
	
	if(dEA == 1 || dEA >= EA_PDIS) return -1;
	else if(dEA >  EA_ABSW)	return -(opcode & 0xFFF8); // change to EA_ABSW
	else if(dEA == EA_ADIS) return -(opcode ^ 0x0018); // change to EA_AIDX

	uint8_t dR = ((opcode & 0x0007) >> 0) | (dEA ? 0x8 : 0x0);
	uint8_t dRR, tRR;
	
	lines = 0;
	emit_reset( buffer );	

	get_x_flag(1);

	get_destination_data( &dRR, &tRR, dEA, dR, size );
	
	emit("\trscs    r%d, r%d, #0\n", tRR, tRR);

	set_destination_data( &dRR, &tRR, dEA, size );
	set_x_flag(tRR); // put C into the X flag

	return lines_ext(lines, 0, dEA, size );
}

