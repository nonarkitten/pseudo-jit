#include "m68k_common.h"
#include "m68k_cpu.h"
#include "m68k_emit_ea.h"
#include "m68k_registers.h"

extern void emit_get_cpsr(void);
extern void emit_save_cpsr(void);

int emit_RTE(char *buffer, uint16_t opcode) {
	lines = 0;
	emit_reset( buffer );

	emit_get_cpsr(); // cpsr is in r2
	emit("\ttst     r2, #0x4000\n");
	emit("\tsvcne   #%d\n", PRIV);
	emit("\tldrh    r2, [" CPU "], #-2\n");
	//emit("\tbfi     r2, r0, #0, #16\n");
	emit_save_cpsr();

	emit("\tldr     r0, [" CPU "], #-4\n");
	emit("\tb       cpu_jump\n");
	return lines | NO_BX_LR;
}
int emit_RTS(char *buffer, uint16_t opcode) {
	lines = 0;
	emit_reset( buffer );
	emit("\tldr     r0, [" CPU "], #-4\n");
	emit("\tb       cpu_jump\n");
	return lines | NO_BX_LR;
}
int emit_RTR(char *buffer, uint16_t opcode) {
	lines = 0;
	emit_reset( buffer );

	emit_get_cpsr(); // cpsr is in r2
	emit("\tldrh    r2, [" CPU "], #-2\n");
	emit_save_cpsr();

	emit("\tldr     r0, [" CPU "], #-4\n");
	emit("\tb       cpu_jump\n");
	return lines | NO_BX_LR;
}

int emit_jump(char *buffer, uint16_t opcode, int jsr) {
	uint8_t sR, sRR, sEA;

	sR = opcode & 7; sEA = (opcode >> 3) & 7;
	if(sEA == 7) sEA += sR; if(sEA) sR += 8;

	if(sEA <= EA_AREG || (sEA == EA_AINC) || (sEA == EA_ADEC)) return -1;
	else if(sEA >  EA_ABSW)	return -(opcode & 0xFFF8); // change to EA_ABSW
	else if(sEA == EA_ADIS) return -(opcode ^ 0x0018); // change to EA_AIDX
	
	lines = 0;
	emit_reset( buffer );	
	get_source_data( &sRR, sEA, sR, 4 );
	if(sRR != 0) emit("\tmov     r0, r%d\n", sRR);
	reg_free( sRR );

    if(jsr) emit("\tb       cpu_subroutine\n");
	else emit("\tb       cpu_jump\n");

	return lines_ext(lines, sEA, 0, 4) | NO_BX_LR;	
}

int emit_JSR(char *buffer, uint16_t opcode) {
	return emit_jump(buffer, opcode, 1);
}
int emit_JMP(char *buffer, uint16_t opcode) {
	return emit_jump(buffer, opcode, 0);
}
