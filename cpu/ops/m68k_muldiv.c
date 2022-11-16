#include "m68k_common.h"
#include "m68k_cpu.h"
#include "m68k_emit_ea.h"
#include "m68k_registers.h"

static int emit_mul(char *buffer, uint16_t opcode, void(*op)(uint8_t,uint8_t)) {
	uint16_t sEA  = (opcode & 0x0038) >> 3;
	
	if(sEA == 7) sEA += (opcode & 0x0007) >> 0;
	if(sEA == EA_AREG || sEA > EA_IMMD) return -1;

	lines = 0;
	emit_reset( buffer );

	uint8_t sRR, sR = ((opcode & 0x0007) >> 0) | (sEA ? 0x8 : 0x0);
	get_source_data( &sRR, sEA, sR, 2 );

	uint8_t dRR, tRR, dR = ((opcode & 0x0E00) >> 9);
	get_destination_data( &dRR, &tRR, 0, dR, 2 );
	
	op(tRR, sRR);
	set_destination_data( &dRR, &tRR, 0, 4 );
	return lines_ext(lines, sEA, 0, 2);
}

static void emit_mulu(uint8_t dRR, uint8_t sRR) {
	emit("\tmuls    r%d, r%d\n", dRR, sRR);
}

static void emit_muls(uint8_t dRR, uint8_t sRR) {
	emit("\tmuls    r%d, r%d\n", dRR, sRR);
}

int emit_MULU(char *buffer, uint16_t opcode) {
	ldx = ldux;
	int lines = emit_mul(buffer, opcode, emit_mulu);
	ldx = ldsx;
	return lines;
}
int emit_MULS(char *buffer, uint16_t opcode) {
	ldx = ldsx;
	int lines = emit_mul(buffer, opcode, emit_muls);
	ldx = ldsx;
	return lines;
}

static int emit_div(char *buffer, uint16_t opcode, void(*op)(uint8_t,uint8_t)) {
	uint16_t sEA  = (opcode & 0x0038) >> 3;
	
	if(sEA == 7) sEA += (opcode & 0x0007) >> 0;
	if(sEA == EA_AREG || sEA > EA_IMMD) return -1;

	lines = 0;
	emit_reset( buffer );

	// reserve R1 as our source
	reg_alloc_arm(1);
	
	uint8_t sRR, sR = ((opcode & 0x0007) >> 0) | (sEA ? 0x8 : 0x0);
	get_source_data( &sRR, sEA, sR, 2 );

	emit("\tcmp     r%d, #0\n", sRR);
	emit("\tsvceq   #%d\n", DIVZ);

	uint8_t dRR = 1, dR = ((opcode & 0x0E00) >> 9);
	if(reg_raw(dR) == 0xFF) {
		emit("\tldr     r1, [" CPU ", #%d]\n", OFFSETOF(cpu_t,d[dR]));
	} else {
		emit("\tmov     r1, r%d\n", reg_raw(dR));
	}

	//get_destination_data( &dRR, &tRR, 0, dR, 4 );

	emit("\tstr     lr, [" CPU ", #%d]\n", OFFSETOF(cpu_t,lr));	
	op(dRR, sRR);
	//set_destination_data( &dRR, &tRR, 0, 4 );
	reg_flush();
	if(reg_raw(dR) == 0xFF) {
		emit("\tstr     r0, [" CPU ", #%d]\n", OFFSETOF(cpu_t,d[dR]));
	} else {
		emit("\tmov     r%d, r0\n", reg_raw(dR));
	}	
	emit("\tldr     pc, [" CPU ", #%d]\n", OFFSETOF(cpu_t,lr));
	return lines_ext(lines, sEA, 0, 2);
}

static void emit_divu(uint8_t dRR, uint8_t sRR) {
	emit("\tbl      UDiv32x16\n");
}

static void emit_divs(uint8_t dRR, uint8_t sRR) {
	emit("\tbl      SDiv32x16\n");
}

int emit_DIVU(char *buffer, uint16_t opcode) {
	ldx = ldux;
	int lines = emit_div(buffer, opcode, emit_divu);
	ldx = ldsx;
	return (lines < 0) ? lines : lines | NO_BX_LR;	
}
int emit_DIVS(char *buffer, uint16_t opcode) {
	ldx = ldsx;
	int lines = emit_div(buffer, opcode, emit_divs);
	ldx = ldsx;
	return (lines < 0) ? lines : lines | NO_BX_LR;	
}
