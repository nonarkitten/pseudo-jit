/*
 * Copyright (c) 2020-2021 Renee Cousins, the Buffee Project
 * See pjit.h for details
 *
 * The core interpreter of PJIT; finds 68K opcodes and does things
 * with them like stuff them back in the cache, run them, etc
 *
 */
#include "pjit.h"

extern ICacheFlush(void *start, void *end);

static jmp_buf jump_buffer;

static uint32_t exec_temp[10] = { 0 };

// return the ARM32 opcode for an unconditional branch to the target
static uint32_t emit_branch(uint32_t target, uint32_t current) {
	return 0xEB000000 | (0x00FFFFFF & (((target & ~3) - (current & ~3) + 8) >> 2));
}
static uint32_t emit_movw(uint32_t reg, uint16_t value) {
	return 0xE3000000 | ((value & 0xF000) << 4) | ((reg & 0xF) << 12) | (value & 0xFFF);
}
static uint32_t emit_movt(uint32_t reg, uint16_t value) {
	return 0xE3400000 | ((value & 0xF000) << 4) | ((reg & 0xF) << 12) | (value & 0xFFF);
}
static void emit_restore_cpsr(uint32_t **arm) {
	// mrs	r3, CPSR				0xE10F3000
	*(*arm)++ = 0xE10F3000;
	// str	r3, [ip, #0x84]			0xE58C3084
	*(*arm)++ = 0xE58C3000 | offsetof(cpu_t, cpsr);
}
static void emit_save_cpsr(uint32_t **arm) {
	// ldr	r3, [ip, #0x84]			0xE59C3084
	*(*arm)++ = 0xE59C3000 | offsetof(cpu_t, cpsr);
	// msr	CPSR_fc, r3				0xE129F003
	*(*arm)++ = 0xE129F003;
}
static void emit_return(uint32_t **arm) {
	// bx	lr						0xE12FFF1E
	*(*arm)++ = 0xE12FFF1E;
}

static uint32_t emit_src_ext(uint32_t current, uint16_t opcode) {
	uint32_t ext_handler[32] = {
		(uint32_t)extword_src_0000, (uint32_t)extword_src_1000, (uint32_t)extword_src_2000, (uint32_t)extword_src_3000, 
		(uint32_t)extword_src_4000, (uint32_t)extword_src_5000, (uint32_t)extword_src_6000, (uint32_t)extword_src_7000, 
		(uint32_t)extword_src_8000, (uint32_t)extword_src_9000, (uint32_t)extword_src_A000, (uint32_t)extword_src_B000, 
		(uint32_t)extword_src_C000, (uint32_t)extword_src_D000, (uint32_t)extword_src_E000, (uint32_t)extword_src_F000, 
		(uint32_t)extword_src_0800, (uint32_t)extword_src_1800, (uint32_t)extword_src_2800, (uint32_t)extword_src_3800, 
		(uint32_t)extword_src_4800, (uint32_t)extword_src_5800, (uint32_t)extword_src_6800, (uint32_t)extword_src_7800, 
		(uint32_t)extword_src_8800, (uint32_t)extword_src_9800, (uint32_t)extword_src_A800, (uint32_t)extword_src_B800, 
		(uint32_t)extword_src_C800, (uint32_t)extword_src_D800, (uint32_t)extword_src_E800, (uint32_t)extword_src_F800, 	
	};
	return emit_branch(ext_handler[opcode >> 11] + ((opcode & 0xFF) * 8), current);
}
static uint32_t emit_dst_ext(uint32_t current, uint16_t opcode) {
	uint32_t ext_handler[32] = {
		(uint32_t)extword_dst_0000, (uint32_t)extword_dst_1000, (uint32_t)extword_dst_2000, (uint32_t)extword_dst_3000, 
		(uint32_t)extword_dst_4000, (uint32_t)extword_dst_5000, (uint32_t)extword_dst_6000, (uint32_t)extword_dst_7000, 
		(uint32_t)extword_dst_8000, (uint32_t)extword_dst_9000, (uint32_t)extword_dst_A000, (uint32_t)extword_dst_B000, 
		(uint32_t)extword_dst_C000, (uint32_t)extword_dst_D000, (uint32_t)extword_dst_E000, (uint32_t)extword_dst_F000, 
		(uint32_t)extword_dst_0800, (uint32_t)extword_dst_1800, (uint32_t)extword_dst_2800, (uint32_t)extword_dst_3800, 
		(uint32_t)extword_dst_4800, (uint32_t)extword_dst_5800, (uint32_t)extword_dst_6800, (uint32_t)extword_dst_7800, 
		(uint32_t)extword_dst_8800, (uint32_t)extword_dst_9800, (uint32_t)extword_dst_A800, (uint32_t)extword_dst_B800, 
		(uint32_t)extword_dst_C800, (uint32_t)extword_dst_D800, (uint32_t)extword_dst_E800, (uint32_t)extword_dst_F800, 	
	};
	return emit_branch(ext_handler[opcode >> 11] + ((opcode & 0xFF) * 8), current);
}

// given an opcode, write out the necessary steps to execute it, assuming
// we're writing directly to the PJIT cache
static uint16_t* copy_opcode(uint32_t** out, uint16_t *pc) {
	// TODO fix up for 32-bit memory
	// TODO fix up for page boundary crossing
	uint16_t opcode = *pc++;
	uint32_t opaddr = optab[opcode];
	uint16_t opea = oplen[opcode];
		
	switch(opea & 0x0F00) {
	case EXT_WORD_SRC_EXT: 
		*(*out)++ = emit_src_ext((uint32_t)out, *pc++); 
		break;
	case EXT_WORD_SRC_16B: 
		*(*out)++ = emit_movw(1, *pc++); 
		break;
	case EXT_WORD_SRC_32B: 
		*(*out)++ = emit_movw(1, *pc++);
		*(*out)++ = emit_movt(1, *pc++);
		break;
	}

	switch(opea & 0xF000) {
	case EXT_WORD_DST_EXT: 
		*(*out)++ = emit_dst_ext((uint32_t)out, *pc++); 
		break;
	case EXT_WORD_DST_16B: 
		*(*out)++ = emit_movw(1, *pc++); 
		break;
	case EXT_WORD_DST_32B: 
		*(*out)++ = emit_movw(1, *pc++);
		*(*out)++ = emit_movt(1, *pc++);
		break;
	}
	
	if((opea & 0xFF) == 1) *out++ = *(uint32_t*)opaddr;
	else *out++ = emit_branch(opaddr, (uint32_t)out);
	
	return pc;
}

// look up opcode and execute it, but never replace it
void cpu_lookup_nojit(void) {
	uint16_t *pc = cache_reverse(lr - 4);
	uint32_t *out = (uint32_t*)&&killme;

	// given an opcode, write out the necessary steps to execute it, assuming
	// we're going to run this immediately in interpreter mode which means we
	// have to restore and save the CPSR register (flags) and add an explicit
	// return -- note that we should never branch-and-link to immediate code

	// TODO fix up for 32-bit memory
	emit_restore_cpsr(&out);
	
	// emit the opcode proper
	pc = copy_opcode(&out, pc);
	
	emit_save_cpsr(&out);
	//emit_return(&out);

	ICacheFlush(&&killme, &&killme2);
	isb(); // flush the pipeline
killme:
	asm volatile("nop");
	asm volatile("nop");
	asm volatile("nop");
	asm volatile("nop");
	asm volatile("nop");
	asm volatile("nop");
	asm volatile("nop");
	asm volatile("nop");
	asm volatile("nop");
	asm volatile("nop");
	asm volatile("nop");
	asm volatile("nop");
	asm volatile("nop");
	asm volatile("nop");
killme2:
	longjmp(jump_buffer, (uint32_t)pc);
}

// look up opcode and replace our call with the opcode
// and then also execute it
void cpu_lookup_safe(void) {
	uint32_t* entry = (uint32_t*)(lr -= 4);
	uint32_t* end = copy_opcode(entry, cache_reverse(entry));
	ICacheFlush(entry, end);
	isb(); // flush the pipeline
	// automagically execute the PJIT cache on return
}

// look up code and if it's small enough, replace the
// caller with the function body, otherwise, replace with
// the branch to it and then execute it
void cpu_lookup_inline(void) {
	static const uint8_t arm_bcc[16] = {
			//       M68K OP Description    ARMcc
			0xEB, // 0000 T  True           1110
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

	uint32_t* entry = (uint32_t*)(lr - 4);
	uint16_t* pc = cache_reverse(lr);
	uint16_t inst = *pc;

	// Handle in-line Bcc
	if((inst & 0xF001 == 0x6000) && (inst & 0xFF != 0xFF)) {
		// TODO fold flags back into ARM status register
		// TODO change this to allow conditional expressions
		uint32_t o = arm_bcc[(inst & 0x0F00) >> 8];
		if(o != 0xFF) {
			*entry = (o << 24) | ((((int8_t)inst) << 2) - 4);
			ICacheFlush(entry, entry + 1);
		}
		goto *optab[inst];
		
	} else {
		uint32_t* end = copy_opcode(entry, cache_reverse(entry));
		ICacheFlush(entry, end);
		isb(); // flush the pipeline
		// automagically execute the PJIT cache on return
	}
}

uint32_t cpu_branch_offset(void* target, void* current) {
	return emit_branch((uint32_t)target, (uint32_t)current);
}

void cpu_dump_state(void) {
	fprintf(stderr, "D%d: %08X  ", 0, D0);
	fprintf(stderr, "D%d: %08X  ", 1, D1);
	fprintf(stderr, "D%d: %08X  ", 2, D2);
	fprintf(stderr, "D%d: %08X\n", 3, D3);
	fprintf(stderr, "D%d: %08X  ", 4, D4);
	fprintf(stderr, "D%d: %08X  ", 5, D5);
	fprintf(stderr, "D%d: %08X  ", 6, D6);
	fprintf(stderr, "D%d: %08X\n", 7, D7);
	
	fprintf(stderr, "A%d: %08X  ", 0, A0);
	fprintf(stderr, "A%d: %08X  ", 1, A1);
	fprintf(stderr, "A%d: %08X  ", 2, A2);
	fprintf(stderr, "A%d: %08X\n", 3, A3);
	fprintf(stderr, "A%d: %08X  ", 4, A4);
	fprintf(stderr, "A%d: %08X  ", 5, A5);
	fprintf(stderr, "A%d: %08X  ", 6, A6);
	fprintf(stderr, "A%d: %08X\n", 7, A7);

}

// This should be odd-aligned to be impossible addresses to jump to
#define PJIT_EXIT (0xC0DEBABE | 1)

void cpu_jump(uint32_t m68k_pc) {
	longjmp(jump_buffer, m68k_pc);
}

void cpu_exit(void) {
	longjmp(jump_buffer, PJIT_EXIT);
}

// start may either point to the PJIT cache or the interpreter function
// if we enter the PJIT cache, we'll only return here through the setjmp
// when the system hits another interpreter command; that is, this is a
// loop, even if it doesn't look like one
void cpu_start(uint32_t m68k_pc) {
	static uint32_t m68k_jump_to;
	// When first called, this is 0
	m68k_jump_to = setjmp(jump_buffer);
	// When we need to jump somewhere, we'll come back here with the new
	// 68K program counter (and should never be 0)
	if(m68k_jump_to) {
		// Special condition to exit PJIT when testing
		if(m68k_jump_to == PJIT_EXIT) return;
		// Because longjmp cannot pass 0, we need a special case for it
		else if(m68k_jump_to == 1) m68k_pc = 0;
		// And for all other cases
		else m68k_pc = m68k_jump_to;
	}
	// And, engage!
	goto *cache_find_entry(m68k_pc);
}

#undef EXIT_PJIT



