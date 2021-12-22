#include "m68k_common.h"
#include "m68k_cpu.h"
#include "m68k_emit_ea.h"
#include "m68k_registers.h"

int emit_NBCD(char *buffer, uint16_t opcode) {
	//static int nbcd_emitted = false;
	
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

	emit("\tpush    {lr}\n");
	get_destination_data( &dRR, &tRR, dEA, dR, size );
	emit("\tbl      handle_nbcd\n");
	set_destination_data( &dRR, &tRR, dEA, size );
	emit("\tpop     {pc}\n");
	return lines_ext(lines, 0, dEA, size ) | NO_BX_LR;
}

static int emit_add_sub_bcd(char *buffer, uint16_t opcode, int is_sub) {
	const uint16_t size = 1;

	uint16_t EA = (opcode & 8) ? EA_ADEC : EA_DREG;
	uint8_t sRR, dRR, tRR;
	
	lines = 0;
	emit_reset( buffer );	

	emit("\tpush    {lr}\n");
	uint8_t sR = opcode & 7;
	get_source_data( &sRR, EA, sR, size );
	uint8_t dR = (opcode >> 9) & 7;
	get_destination_data( &dRR, &tRR, EA, dR, size );
	if(is_sub) emit("\tbl      handle_sbcd\n");
	else       emit("\tbl      handle_abcd\n");
	set_destination_data( &dRR, &tRR, EA, size );
	emit("\tpop     {pc}\n");
	return lines_ext(lines, EA, EA, size ) | NO_BX_LR;
}

int emit_SBCD(char *buffer, uint16_t opcode) { return emit_add_sub_bcd(buffer, opcode, 1); }
int emit_ABCD(char *buffer, uint16_t opcode) { return emit_add_sub_bcd(buffer, opcode, 0); }



