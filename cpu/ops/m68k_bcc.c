#include "m68k_common.h"
#include "m68k_cpu.h"
#include "m68k_emit_ea.h"
#include "m68k_registers.h"

static const char* arm_cc[16] = {
//  ARMop    M68K OP Description    ARMcc
	"  ", // 0000 T  True           1110
	"  ", // 0001 F  False          1111
	"hi", // 0010 HI Higher         emulate -- TODO this is wrong
	"ls", // 0011 LS Lower/Same     emulate -- TODO this is wrong
	"cc", // 0100 CC Carry Clear    0011
	"cs", // 0101 CS Carry Set      0010
	"ne", // 0110 NE Not Equal      0001
	"eq", // 0111 EQ Equal          0000
	"vc", // 1000 VC Overflow Clear 0111
	"vs", // 1001 VS Overflow Set   0110
	"pl", // 1010 PL Plus           0101
	"mi", // 1011 MI Minus          0100
	"ge", // 1100 GE Greater/Equal  1010
	"lt", // 1101 LT Lesser         1011
	"gt", // 1110 GT Greater        1100
	"le"  // 1111 LE Lesser/Equal   1101
};

// 0101cccc11mmmxxx

int emit_SCC(char *buffer, uint16_t opcode) {
	uint16_t dEA  = (opcode & 0x0038) >> 3;
	uint8_t dRR, tRR;
	if(dEA == 7) dEA += (opcode & 0x0007) >> 0;

	// Scc.B *.An, Scc *,PC or Scc *,#imm not valid
	if((dEA == EA_PDIS) || (dEA == EA_PIDX) || (dEA == EA_AREG)) return -1; 
	if(dEA > EA_ABSW)  return -(opcode & 0xFFF8); // alias to EA_ABSW
	if(dEA == EA_ADIS) return -(opcode ^ 0x0018); // alias to EA_AIDX
	
	uint8_t dR = ((opcode & 0x0007) >> 0) | (dEA ? 0x8 : 0x0);

	lines = 0;
	emit_reset( buffer );

	get_destination_data( &dRR, &tRR, dEA, dR, 1 );
	
	uint8_t cc = (opcode & 0x0F00) >> 8;
	
	if(cc == 0) {
		emit("\tmov     r%d, #0xFF\n", tRR);
	} else if(cc == 1) {
		emit("\tmov     r%d, #0\n", tRR);
	} else {
		if((cc == 2) || (cc == 3)) {
			emit("\trsc     r0, r0, #0\n"); // flip C
		}
		emit("\tmov%s   r%d, #0xFF\n", arm_cc[cc], tRR);
		emit("\tmov%s   r%d, #0\n", arm_cc[cc ^ 1], tRR);
	}

	set_destination_data( &dRR, &tRR, dEA, 1 );
	return lines_ext(lines, 0, dEA, 1);
}

// 0101cccc11001nnn
//              --- Data register
//     ----         Condition code

int emit_DBCC(char *buffer, uint16_t opcode) {
	static bool emit_extern = false;
	uint8_t tRR, dRR, dR = opcode & 0x0007;

	lines = 0;
	emit_reset( buffer );

	// get our condition code
	int cc = (opcode & 0x0F00) >> 8;
	if(!cc) return 0; // NOP

	if(!emit_extern) {
		emit_extern = true;
		emit("\t.extern branch_normal\n");
	}
	
	reg_alloc_arm(1); // our offset
	reg_alloc_temp(&tRR);

	
	// determine our destination real register
	if(!emit_get_reg( &dRR, dR, 2 )) return -1;
	// decrement
	emit("\tsub     r%d, r%d, #1\n", dRR, dRR);
	reg_modified(dRR);
	reg_flush();
	
	// skip if negative one
	emit("\tmrs     r%d, CPSR\n", tRR);
	emit("\tcmp     r%d, #-1\n", dRR);
	emit("\tbne     0f\n");
	emit("\tmsr     CPSR_fc, r%d\n", tRR);
	emit("\tbx      lr\n");

	// otherwise branch normally
	emit("0:  msr     CPSR_fc, r%d\n", tRR);

	// Between ARM and 68K, C is inverted
	// 68K HI /C & /Z    ARM HI C & /Z
	// 68K LS  C |  Z    ARM LS /C | Z
	if((cc == 2) || (cc == 3)) {
		emit("\trsc     r0, r0, #0\n"); // flip C
	}
	emit("\tb%s     branch_normal\n", arm_cc[cc]);

	return lines | NO_BX_LR;
}

// 0110ccccdddddddd
//         -------- 8-bit displacement
//     ----         condition code
static int addr_err_emitted = false;

int emit_BCC(char *buffer, uint16_t opcode) {
	lines = 0;
	emit_reset( buffer );

	int cc = (opcode & 0x0F00) >> 8;

	int8_t d = (int8_t)(opcode & 0xff);
	if((d == 0) || ((uint8_t)d == 0xFF)) { /* long branch already in r1 */ }
	else if(d & 1) {
		if(addr_err_emitted) {
			return addr_err_emitted;
		} else {
			addr_err_emitted = -opcode;
			emit("\tsvc     #%d\n", ADDRESSERR);
			return lines;
		}
	}	
	else if(d > 0) emit("\tmov     r1, #0x%02x\n", d);
	else if(d < 0) emit("\tmvn     r1, #0x%02x\n", (uint8_t)(-d));	
	
	if((cc == 2) || (cc == 3)) {
		emit("\trsc     r0, r0, #0\n"); // flip C
	}

	if(cc == 1) emit("\tb%s     branch_subroutine\n", arm_cc[cc ^ 1]);
	else emit("\tb%s     branch_normal\n", arm_cc[cc ^ 1]);
	
	return lines | NO_BX_LR;
}

int emit_BRA(char *buffer, uint16_t opcode) {
	return emit_BCC(buffer, opcode);
}

int emit_BSR(char *buffer, uint16_t opcode) {
	return emit_BCC(buffer, opcode);
}



