#include "m68k_common.h"
#include "m68k_cpu.h"
#include "m68k_emit_ea.h"
#include "m68k_registers.h"

// ASR is ASR on ARM
// ASL, do LSL one fewer bit then ADDS (or just ADDS for memory)
// LSR and LSL are the same on ARM
// ROR is the same on ARM
// ROL needs the bits reversed in and out
// or use ADD/ADDC for single bit shift
// ROXR and ROXL are evil


// THESE SHIFT BY ONE
//     { 0xE0C0, 0x013F, emit_ASD_EA        },
//     { 0xE2C0, 0x013F, emit_LSD_EA        },
//     { 0xE4C0, 0x013F, emit_ROXD_EA       },
//     { 0xE6C0, 0x013F, emit_ROD_EA        },

static int emit_ea(char *buffer, uint16_t opcode, void(*callback)(uint8_t)) {
	uint16_t dEA = (opcode & 0x0038) >> 3; 	
	if(dEA == 7) dEA += opcode & 7;
	
	if(dEA <= EA_AREG || dEA >= EA_PDIS) return -1;
	else if(dEA >  EA_ABSW)	return -(opcode & 0xFFF8); // change to EA_ABSW
	else if(dEA == EA_ADIS) return -(opcode ^ 0x0018); // change to EA_AIDX
	
	lines = 0;
	emit_reset( buffer );	

	uint8_t dRR, tRR, dR = (opcode & 7) | 0x8;
	get_destination_data(&dRR, &tRR, dEA, dR, 2);	
	
	callback(tRR);
		
	set_destination_data(&dRR, &tRR, dEA, 2);	
	return lines_ext(lines, 0, dEA, 2);
}

static void emit_asl_ea(uint8_t tR2) {
	emit("\tadds    r%d, r%d, r%d\n", tR2, tR2, tR2); 
}
static void emit_asr_ea(uint8_t tR2) { 
	emit("\tasrs    r%d, #1\n", tR2);	
}
static void emit_lsl_ea(uint8_t tR2) {
	emit("\tlsls    r%d, #1\n", tR2);	
}
static void emit_lsr_ea(uint8_t tR2) {
	emit("\tlsrs    r%d, #1\n", tR2);	
}
static void emit_rol_ea(uint8_t tR2) {
	emit("\tadds    r%d, r%d, r%d\n", tR2, tR2, tR2); 
	emit("\tadc     r%d, r%d, #0\n", tR2, tR2);
}
static void emit_ror_ea(uint8_t tR2) {
	emit("\trors    r%d, #1\n", tR2);	
}
static void emit_roxl_ea(uint8_t tR2) {
	uint8_t tRR;
	reg_alloc_temp(&tRR);
	emit("\tldr     r%d, [r12, #%d]\n", tRR, offsetof(cpu_t, x));
	emit("\tadds    r%d, r%d, r%d\n", tR2, tR2, tR2);
	emit("\tadd     r%d, r%d, r%d\n", tR2, tR2, tRR);
	emit("\tmovcs   r%d, #1\n", tRR);
	emit("\tmovcc   r%d, #0\n", tRR);
	emit("\tstr     r%d, [r12, #%d]\n", tRR, offsetof(cpu_t, x));
}
static void emit_roxr_ea(uint8_t tR2) {
	uint8_t tRR;
	reg_alloc_temp(&tRR);
	emit("\tldr     r%d, [r12, #%d]\n", tRR, offsetof(cpu_t, x));
	emit("\tlsrs    r%d, #1\n", tR2);
	emit("\tadd     r%d, r%d, r%d, lsl #%d\n", tR2, tR2, tRR, 16);
	emit("\tmovcs   r%d, #1\n", tRR);
	emit("\tmovcc   r%d, #0\n", tRR);
	emit("\tstr     r%d, [r12, #%d]\n", tRR, offsetof(cpu_t, x));
}

int emit_ASD_EA(char *buffer, uint16_t opcode) { 
	return emit_ea(buffer, opcode, (opcode & 0x0100) ? emit_asl_ea : emit_asr_ea); 
}

int emit_LSD_EA(char *buffer, uint16_t opcode) { 
	return emit_ea(buffer, opcode, (opcode & 0x0100) ? emit_lsl_ea : emit_lsr_ea); 
}

int emit_ROXD_EA(char *buffer, uint16_t opcode) {
	return emit_ea(buffer, opcode, (opcode & 0x0100) ? emit_roxl_ea : emit_roxr_ea); 
}

int emit_ROD_EA(char *buffer, uint16_t opcode) { 
	return emit_ea(buffer, opcode, (opcode & 0x0100) ? emit_rol_ea : emit_ror_ea); 
}

static int emit_immd(char *buffer, uint16_t opcode, void(*callback)(uint8_t,uint16_t,uint16_t)) {	
	uint16_t size = 1 << ((opcode & 0x00C0) >> 6);
	if(size > 4) return -1;
	
	lines = 0;
	emit_reset( buffer );	

	uint8_t dRR, tRR, dR = opcode & 7;
	get_destination_data(&dRR, &tRR, 0, dR, size);
	
	uint8_t amt = (opcode >> 9) & 7;
	if(amt == 0) amt = 8;
	
	callback(tRR, amt, size);
	
	set_destination_data(&dRR, &tRR, 0, size);
	return lines;
}

static void emit_asl_immd(uint8_t tR2, uint16_t amt, uint16_t size) {
	if(amt > 1) emit("\tlsl     r%d, #%d\n", tR2, (amt - 1)); 
	emit("\tadds    r%d, r%d, r%d\n", tR2, tR2, tR2); 
}
static void emit_asr_immd(uint8_t tR2, uint16_t amt, uint16_t size) { 
	emit("\tasrs    r%d, #%d\n", tR2, amt);	
}
static void emit_lsl_immd(uint8_t tR2, uint16_t amt, uint16_t size) {
	emit("\tlsls    r%d, #%d\n", tR2, amt);	
}
static void emit_lsr_immd(uint8_t tR2, uint16_t amt, uint16_t size) {
	emit("\tlsrs    r%d, #%d\n", tR2, amt);	
}
static void emit_asl_reg(uint8_t tR2, uint16_t sR, uint16_t size) {
	uint8_t sRR;
	emit_get_reg(&sRR, sR, 1);
	emit("\tsubs    r%d, r%d, #1\n", sRR, sRR);
	emit("\tlslgts  r%d, r%d, r%d\n", tR2, tR2, sRR);
	emit("\taddges  r%d, r%d, r%d\n", tR2, tR2, tR2);
	reg_free(sRR);
}
static void emit_asr_reg(uint8_t tR2, uint16_t sR, uint16_t size) { 
	uint8_t sRR;
	emit_get_reg(&sRR, sR, 1);
	emit("\tasrs    r%d, r%d\n", tR2, sRR);	
	reg_free(sRR);
}
static void emit_lsl_reg(uint8_t tR2, uint16_t sR, uint16_t size) {
	uint8_t sRR;
	emit_get_reg(&sRR, sR, 1);
	emit("\tlsls    r%d, r%d\n", tR2, sRR);	
	reg_free(sRR);
}
static void emit_lsr_reg(uint8_t tR2, uint16_t sR, uint16_t size) {
	uint8_t sRR;
	emit_get_reg(&sRR, sR, 1);
	emit("\tlsrs    r%d, r%d\n", tR2, sRR);	
	reg_free(sRR);
}

int emit_ASD(char *buffer, uint16_t opcode) {
	if(opcode & 0x0020) { // register
		return emit_immd(buffer, opcode, (opcode & 0x0100) ? emit_asl_reg : emit_asr_reg); 
	} else { // immediate
		return emit_immd(buffer, opcode, (opcode & 0x0100) ? emit_asl_immd : emit_asr_immd); 
	}
}

int emit_LSD(char *buffer, uint16_t opcode) {
	if(opcode & 0x0020) { // register
		return emit_immd(buffer, opcode, (opcode & 0x0100) ? emit_lsl_reg : emit_lsr_reg);
	} else { // immediate
		return emit_immd(buffer, opcode, (opcode & 0x0100) ? emit_lsl_immd : emit_lsr_immd);
	}
}

static void emit_roxl_immd(uint8_t tR2, uint16_t amt, uint16_t size) {
	emit("\tb       handle_roxl_immd\n");
}
static void emit_roxr_immd(uint8_t tR2, uint16_t amt, uint16_t size) {
	emit("\tb       handle_roxr_immd\n");
}
static void emit_roxl_reg(uint8_t tR2, uint16_t sR, uint16_t size) {
	emit("\tb       handle_roxl_reg\n");
}
static void emit_roxr_reg(uint8_t tR2, uint16_t sR, uint16_t size) {
	emit("\tb       handle_roxr_reg\n");
}

int emit_ROXD(char *buffer, uint16_t opcode) {
	if(opcode & 0x0020) { // register
		return emit_immd(buffer, opcode, (opcode & 0x0100) ? emit_roxl_reg : emit_roxr_reg); 
	} else { // immediate
		return emit_immd(buffer, opcode, (opcode & 0x0100) ? emit_roxl_immd : emit_roxr_immd); 
	}
}

static void emit_rol_immd(uint8_t tR2, uint16_t amt, uint16_t size) {
	if(amt > 1) emit("\tror     r%d, #%d\n", tR2, (33 - amt) & 0x31);
	emit("\tadds    r%d, r%d, r%d\n", tR2, tR2, tR2); 
	emit("\tadc     r%d, r%d, #0\n", tR2, tR2);
}
static void emit_ror_immd(uint8_t tR2, uint16_t amt, uint16_t size) {
	emit("\trors    r%d, #%d\n", tR2, amt);	
}
static void emit_rol_reg(uint8_t tR2, uint16_t sR, uint16_t size) {
	uint8_t sRR;
	emit_get_reg(&sRR, sR, 1);
	emit("\tcmp     r%d, #0\n", tR2);
	emit("\tbgt     0f\n");
	
	emit("\trsb     r%d, r%d, #32\n", sRR, sRR);
	emit("\tror     r%d, r%d\n", tR2, sRR);		
	emit("\tadds    r%d, r%d, r%d\n", tR2, tR2, tR2); 
	emit("\tadc     r%d, r%d, #0\n", tR2, tR2);
	emit("0:\n");
	reg_free(sRR);
}
static void emit_ror_reg(uint8_t tR2, uint16_t sR, uint16_t size) {
	uint8_t sRR;
	emit_get_reg(&sRR, sR, 1);
	emit("\trors    r%d, r%d\n", tR2, sRR);	
	reg_free(sRR);
}

int emit_ROD(char *buffer, uint16_t opcode) { 
	if(opcode & 0x0020) { // register
		return emit_immd(buffer, opcode, (opcode & 0x0100) ? emit_rol_reg : emit_ror_reg); 
	} else { // immediate
		return emit_immd(buffer, opcode, (opcode & 0x0100) ? emit_rol_immd : emit_ror_immd); 
	}
}




