#include "m68k_common.h"
#include "m68k_cpu.h"
#include "m68k_emit_ea.h"
#include "m68k_registers.h"

void emit_get_cpsr(void) {	
	// get flags and conver them to 68K
	emit("\tmrs     r0, CPSR\n");
	emit("\tadd     r0, r12, r0, lsr #28\n");	
	emit("\tldrb    r0, [r0, %d]\n", offsetof(cpu_t, arm2cc));
	
	// load the sr_ccr and replace the low bytes
	emit("\tldrh    r2, [r12, #%d]\n", offsetof(cpu_t, sr));
	emit("\tbfi     r2, r0, #0, #4\n");

	// load X
	// emit("\tldr     r0, [r12, #%d]\n", offsetof(cpu_t, x));
	// emit("\tbfi     r2, r0, #4, #1\n");
}

void emit_save_cpsr(void) {
	// save sr_ccr
	emit("\tstrh    r2, [r12, #%d]\n", offsetof(cpu_t, sr));
	
	// save X
	// emit("\tror     r2, #4\n");
	// emit("\tstr     r2, [r12, #%d]\n", offsetof(cpu_t, x));
		
	// convert back to ARM flags
	emit("\tand     r2, r2, #0xF\n");
	emit("\tadd     r2, r2, #%d\n", offsetof(cpu_t, cc2arm));
	emit("\tldr     r0, [r12, r2, lsl #2]\n");
	emit("\tmsr     CPSR_fc, r0\n");
}

int emit_ORI_TO_CCR(char *buffer, uint16_t opcode) {
	lines = 0;
	emit_reset( buffer );	

	emit_get_cpsr();
	
	// or the low 8-bits of our immediate
	emit("\tuxtb    r1, r1\n");
	emit("\torr     r2, r1\n");
	
	emit_save_cpsr();
	
	return lines | EXT_WORD_SRC_16B;	
}
int emit_ORI_TO_SR(char *buffer, uint16_t opcode) {
	lines = 0;
	emit_reset( buffer );	

	emit_get_cpsr();
	
	// are we in supervisor mode?
	emit("\ttst     r2, #0x4000\n");
	emit("\tsvcne   #%d\n", PRIV);

	// or the low 16-bits of our immediate
	emit("\tuxth    r1, r1\n");
	emit("\torr     r2, r1\n");
	
	emit_save_cpsr();
	
	return lines | EXT_WORD_SRC_16B;
}
int emit_ANDI_TO_CCR(char *buffer, uint16_t opcode) {
	lines = 0;
	emit_reset( buffer );	

	emit_get_cpsr();
	
	// and the low 8-bits of our immediate
	emit("\tuxtb    r1, r1\n");
	emit("\tand     r2, r1\n");
	
	emit_save_cpsr();
	
	return lines | EXT_WORD_SRC_16B;	
}
int emit_ANDI_TO_SR(char *buffer, uint16_t opcode) {
	lines = 0;
	emit_reset( buffer );	

	emit_get_cpsr();
	
	// are we in supervisor mode?
	emit("\ttst     r2, #0x4000\n");
	emit("\tsvcne   #%d\n", PRIV);

	// and the low 16-bits of our immediate
	emit("\tuxth    r1, r1\n");
	emit("\tand     r2, r1\n");
	
	emit_save_cpsr();
	
	return lines | EXT_WORD_SRC_16B;
}
int emit_EORI_TO_CCR(char *buffer, uint16_t opcode) {
	lines = 0;
	emit_reset( buffer );	

	emit_get_cpsr();
	
	// or the low 8-bits of our immediate
	emit("\tuxtb    r1, r1\n");
	emit("\teor     r2, r1\n");
	
	emit_save_cpsr();
	
	return lines | EXT_WORD_SRC_16B;	
}
int emit_EORI_TO_SR(char *buffer, uint16_t opcode) {
	lines = 0;
	emit_reset( buffer );	

	emit_get_cpsr();
	
	// are we in supervisor mode?
	emit("\ttst     r2, #0x4000\n");
	emit("\tsvcne   #%d\n", PRIV);

	// eor the low 16-bits of our immediate
	emit("\tuxth    r1, r1\n");
	emit("\teor     r2, r1\n");
	
	emit_save_cpsr();
	
	return lines | EXT_WORD_SRC_16B;
}

int emit_MOVE_FROM_SR(char *buffer, uint16_t opcode) {
	uint16_t dEA;
	uint8_t dR, dRR, tRR = 2;
	uint16_t size = 2;

	lines = 0;
	emit_reset( buffer );	

	dR = opcode & 7; dEA = (opcode >> 3) & 7; if(dEA == 7) dEA += dR; if(dEA) dR += 8;

	if((dEA == 1) || (dEA >= EA_PDIS)) return -1;
	else if(dEA >  EA_ABSW)	return -(opcode & 0xFFF8); // change to EA_ABSL
	else if(dEA == EA_ADIS) return -(opcode ^ 0x0018); // change to EA_AIDX

	emit_get_cpsr(); // cpsr is in r2
	reg_alloc_arm(2);

	get_destination_data( &dRR, NULL, dEA, dR, size );
	set_destination_data( &dRR, &tRR, dEA, size );
	
	return lines_ext(lines, 0, dEA, size);	
}

static int emit_move_to(char *buffer, uint16_t opcode, int whole_sr) {
	uint16_t sEA;
	uint8_t sR, sRR;
	uint16_t size = 2;

	lines = 0;
	emit_reset( buffer );	

	sR = opcode & 7; sEA = (opcode >> 3) & 7; if(sEA == 7) sEA += sR; if(sEA) sR += 8;

	if(sEA == EA_AREG) return -1;
	else if(sEA == EA_IMMD) {}
	else if(sEA >  EA_ABSW)	return -(opcode & 0xFFF8); // change to EA_ABSW
	else if(sEA == EA_ADIS) return -(opcode ^ 0x0018); // change to EA_AIDX
	
	emit_get_cpsr(); // cpsr is in r2
	reg_alloc_arm(2);
	
	// are we in supervisor mode?
	if(whole_sr) {
		emit("\ttst     r2, #0x4000\n");
		emit("\tsvcne   #%d\n", PRIV);
	}
	
	if(get_source_data( &sRR, sEA, sR, size ) == 0) {
		printf("@ problem opcode %04X\n", opcode);
	}
	
	emit("\tbfi     r2, r%d, #0, #%d\n", sRR, (whole_sr ? 16 : 8));
	
	emit_save_cpsr();
	
	// everything's done, let's return and free all the registers
	reg_flush();
	return lines_ext(lines, sEA, 0, size);	
}

int emit_MOVE_TO_CCR(char *buffer, uint16_t opcode) {
	return emit_move_to(buffer, opcode, 0);
}
int emit_MOVE_TO_SR(char *buffer, uint16_t opcode) {
	return emit_move_to(buffer, opcode, 1);
}
int emit_STOP(char *buffer, uint16_t opcode) {
	int lines = emit_move_to(buffer, opcode, 1);
	emit("\twfi\n");
	return lines;
}
int emit_MOVE_USP(char *buffer, uint16_t opcode) {
	lines = 0;
	emit_reset( buffer );	

	emit_get_cpsr(); // cpsr is in r2
	emit("\ttst     r2, #0x4000\n");
	emit("\tsvcne   #%d\n", PRIV);

	uint8_t dRR, dR = (opcode & 0x0007) | 0x8;
	if(!emit_get_reg( &dRR, dR, 4 )) return -1;

	if(opcode & 0x0008) {
		// [Ax] -> USP
		emit("\tstr     r%d, [r12, #%d]\n", dRR, offsetof(cpu_t, usp));
	} else {
		// USP -> [Ax]
		emit("\tldr     r%d, [r12, #%d]\n", dRR, offsetof(cpu_t, usp));
	}
	reg_flush();
	return lines;
}







