/*
 * file: pjit_ops.c
 * author: Renee Cousins
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 *
 * The core interpreter of PJIT; finds 68K opcodes and does things
 * with them like stuff them back in the cache, run them, etc
 *
 */
#include "pjit.h"

#define ARM_NOP 0xE320F000

extern void (*jmp_table[8192])(void);
extern uint8_t len_table[8192];

// return the ARM32 opcode for an unconditional branch to the target
uint32_t cpu_branch_offset(uint32_t target, uint32_t current) {
	return 0xEB000000 | (((target & ~3) - (current & ~3) + 8) >> 2);
}

// look up opcode and execute it, but never replace it
void cpu_lookup_nojit(void) {
	uint16_t inst = pjit_get_instr((uint16_t*)cache_reverse(lr));
	// TODO fill out table
	uint8_t operand_length = jt->operand_len[inst >> 3];
	uint32_t opcode = jt->opcode_fn[inst >> 3];
	// Skip operands on way back
	lr += operand_length;
	goto *(void*)opcode;
}

// look up opcode and replace our call with the opcode
// and then also execute it
void cpu_lookup_safe(void) {
	uint32_t* patch = (uint32_t*)(lr - 4);
	uint16_t inst = pjit_get_instr((uint16_t*)cache_reverse(lr));
	// TODO fill out table
	uint8_t operand_length = jt->operand_len[inst >> 3];
	uint32_t opcode = jt->opcode_fn[inst >> 3];
	*patch++ = cpu_branch_offset(opcode, lr);
	while(operand_length--) *patch++ = ARM_NOP;
	goto *(void*)opcode;
}

// look up code and if it's small enough, replace the
// caller with the function body, otherwise, replace with
// the branch to it and then execute it
void cpu_lookup_inline(void) {
	static const uint8_t arm_bcc[16] = {
			//       M68K OP Description    ARMcc
			0xEB, // 0000 T  True	        1110
			0xFB, // 0001 F  False          1111
			0xFF, // 0010 HI Higher         emulate
			0xFF, // 0011 LS Lower/Same     emulate
			0x3B, // 0100 CC Carry Clear    0011
			0x2B, // 0101 CS Carry Set      0010
			0x1B, // 0110 NE Not Equal      0001
			0x0B, // 0111 EQ Equal          0000
			0x7B, // 1000 VC Overflow Clear 0111
			0x6B, // 1001 VS Overflow Set   0110
			0x5B, // 1010 PL Plus           0101
			0x4B, // 1011 MI Minus          0100
			0xAB, // 1100 GE Greater/Equal  1010
			0xBB, // 1101 LT Lesser         1011
			0xCB, // 1110 GT Greater        1100
			0xDB, // 1111 LE Lesser/Equal   1101
	};

	uint32_t* patch = (uint32_t*)(lr - 4);
	uint16_t inst = pjit_get_instr((uint16_t*)cache_reverse(lr));
	uint32_t opcode = jt->opcode_fn[inst >> 3];
	uint8_t operand_length = jt->operand_len[inst >> 3];

	// Handle in-line Bcc
	if((inst & 0xF001 == 0x6000) && (inst & 0xFF != 0xFF)) {
		// TODO fold flags back into ARM status register
		// TODO change this to allow conditional expressions
		uint32_t o = arm_bcc[(inst & 0x0F00) >> 8];
		if(o == 0xFF) goto emulate; else o <<= 24;
		*patch++ = o | ((((int8_t)inst) << 2) - 4);
	}

	// Handle in-line NOP
	else if(inst == 0x4E71) {
		*patch++ = ARM_NOP;
	}

	else {
		// TODO fill out table
		emulate:
		*patch++ = cpu_branch_offset(opcode, lr);
	}
	while(operand_length--) *patch++ = ARM_NOP;
	goto *(void*)opcode;
}

