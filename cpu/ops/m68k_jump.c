#include "m68k_common.h"
#include "m68k_cpu.h"
#include "m68k_emit_ea.h"
#include "m68k_registers.h"

//     { 0x4E73, 0x0000, emit_RTE           },
//     { 0x4E75, 0x0000, emit_RTS           },
//     { 0x4E77, 0x0000, emit_RTR           },
//     { 0x4E80, 0x003F, emit_JSR           },
//     { 0x4EC0, 0x003F, emit_JMP           },

// void cpu_jump(uint32_t m68k_pc) {
// 	longjmp(jump_buffer, m68k_pc);
// }
// 
// void relative_branch(uint32_t _lr, int32_t offset) {
// 	cpu_jump(cache_reverse(_lr) + offset);
// }


/*
How we collapse EA modes:

Extension words are always handled before the opcode.
If the extension mode uses PC, that is folded in before.

	r1 holds any source value
	r2 holds any destination value

EA_PDIS, EA_PIDX, EA_ABSW, EA_ABSL, EA_IMMD	
	Same opcode
	r1/r2 holds ea
	
EA_ADIS, EA_AIDX			
	Same opcode
	r1/r2 holds index/displacement
	rX will hold address register
	
emit_get_reg
	does nothing for EA_PDIS, EA_PIDX, EA_ABSW, EA_ABSL, EA_IMMD
	if rX is temp register (<4)
		r1/r2 is added to temp then freed
		ea becomes EA_ADDR
	if rX is register
		r1/r2 remains and may be used later
		
emit_load
	returns reg with *DATA*
	EA_IMMD returns r1/r2
	EA_DREG and EA_AREG just return the register
	
	[r1/r2]		for EA_PDIS, EA_PIDX, EA_ABSW, EA_ABSL,
				can free unused r1/r2 for source
	[rX],+2		for EA_AINC (if source ONLY as it won't be "stored")
	[rX]		for EA_ADDR, EA_AINC (dest) and sometimes EA_ADIS, EA_AIDX
	[rX,-1]!	for EA_ADEC
	[r1/r2,rX]!	for EA_ADIS, EA_AIDX
				can free unused r1/r2 for source
				r1/r2 holds computed EA now for dest
	frees rX
	
emit_store
	does nothing for EA_PDIS, EA_PIDX or EA_IMMD
	[rX],+2		for EA_AINC
	[r1/r2]		for EA_ABSW, EA_ABSL
	[rX]		for EA_ADDR, EA_ADEC and sometimes EA_ADIS, EA_AIDX
	
*/
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
// 	emit("\ttst     r2, #0x4000\n");
// 	emit("\tsvcne   #%d\n", PRIV);
	emit("\tldrh    r2, [" CPU "], #-2\n");
//	emit("\tbfi     r2, r0, #0, #8\n");
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

	// if(jsr) {
    //     emit("\tsub     r3, lr, #4\n");
    //     emit("\tldr     r0, [" CPU ", #%d]\n", offsetof(cpu_t, m68k_page));
    //     emit("\tubfx    r3, r3, #1, #12\n");
    //     emit("\torr     r0, r3, r0\n");
	// 	emit("\tstr     r0, [r11, #-4]!\n");
	// }

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
	return emit_jump(buffer, opcode, 2);
}
