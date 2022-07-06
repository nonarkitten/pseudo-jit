/*
 * Copyright (c) 2020-2021 Renee Cousins, the Buffee Project
 * See pjit.h for details
 *
 * The core interpreter of PJIT; finds 68K opcodes and does things
 * with them like stuff them back in the cache, run them, etc
 *
 */
#include <stdarg.h>
#include <stddef.h>
#include <stdbool.h>

#include "pjit.h"

//extern ICacheFlush(void *start, void *end);

// This should be odd-aligned to be impossible addresses to jump to
#define PJIT_EXIT (0xC0DEBABE | 1)

extern cpu_t cpu_state;

static jmp_buf jump_buffer;
static uint32_t exec_temp[16] __attribute__ ((aligned (16))) = { 0 };

//extern void __clear_cache(char* beg, char* end);

#define debug(...) printf(__VA_ARGS__)

// return the ARM32 opcode for an unconditional branch to the target
static uint32_t emit_branch_link(uint32_t target, uint32_t current) {
	int32_t offset = target - current - 8;
	if(offset > (int32_t)0x03FFFFFC || offset < (int32_t)0xFC000004) {
		printf("\n*** Branch out of range (%08lX).\n", offset);
		__asm__("BKPT 0");
		//exit(1);
	}
	uint32_t op = 0xEB000000 | (0x00FFFFFF & (offset >> 2));
	debug("EMIT ARM %08lX (BL %+ld)\n", op, offset);
	return op;
}
static uint32_t emit_branch(uint32_t target, uint32_t current) {
	int32_t offset = target - current - 8;
	if(offset > (int32_t)0x03FFFFFC || offset < (int32_t)0xFC000004) {
		printf("\n*** Branch out of range (%08lX).\n", offset);
		__asm__("BKPT 0");
		//exit(1);
	}
	uint32_t op = 0xEA000000 | (0x00FFFFFF & (offset >> 2));
	debug("EMIT ARM %08lX (BL %+ld)\n", op, offset);
	return op;
}
static uint32_t emit_movw(uint32_t reg, uint16_t value) {
	uint32_t op = 0xE3000000 | ((value & 0xF000) << 4) | ((reg & 0xF) << 12) | (value & 0xFFF);
	debug("EMIT ARM %08lX (MOVW R%ld, #%04hX)\n", op, reg, value);
	return op;
}
static uint32_t emit_movt(uint32_t reg, uint16_t value) {
	uint32_t op = 0xE3400000 | ((value & 0xF000) << 4) | ((reg & 0xF) << 12) | (value & 0xFFF);
	debug("EMIT ARM %08lX (MOVT R%ld, #%04hX)\n", op, reg, value);
	return op;
}
// static void emit_restore_cpsr(uint32_t **arm) {
// 	// mrs	r3, CPSR				0xE10F3000
// 	debug("EMIT ARM E10F3000 (MRS R3, CPSR)\n");
// 	*(*arm)++ = 0xE10F3000;
// 	// str	r3, [ip, #0x84]			0xE58C3084
// 	debug("EMIT ARM E58C3084 (STR R3, [IP, #0x84])\n");
// 	*(*arm)++ = 0xE58C3000 | offsetof(cpu_t, cpsr);
// }
// static void emit_save_cpsr(uint32_t **arm) {
// 	// ldr	r3, [ip, #0x84]			0xE59C3084
// 	debug("EMIT ARM E59C3000 (LDR R3, [IP, #0x84])\n");
// 	*(*arm)++ = 0xE59C3000 | offsetof(cpu_t, cpsr);
// 	// msr	CPSR_fc, r3				0xE129F003
// 	debug("EMIT ARM E10F3000 (MRS CPSR, R3)\n");
// 	*(*arm)++ = 0xE129F003;
// }
// static void emit_return(uint32_t **arm) {
// 	// bx	lr						0xE12FFF1E
// 	debug("EMIT ARM E12FFF1E (BX LR)\n");
// 	*(*arm)++ = 0xE12FFF1E;
// }

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
static uint16_t copy_opcode(uint32_t** out, uint16_t *pc, bool link) {
	extern const char* m68k_disasm(uint16_t op);
	// TODO fix up for 32-bit memory
	// TODO fix up for page boundary crossing
	uint16_t i = 0;
	uint16_t opcode = pc[i++];
	uint32_t opaddr = optab[opcode];
	uint16_t opea = oplen[opcode];

	debug("In copy_opcode, opcode=0x%04hX (%s), opaddr=0x%08lX, opea=0x%04hX\n", opcode, m68k_disasm(opcode), opaddr, opea);
	if(opcode == 0xFFFF) longjmp( jump_buffer, PJIT_EXIT );
	
	switch(opea & 0x0F00) {
	case EXT_WORD_SRC_EXT: 
		debug("EMIT extension word.\n");
		*(*out)++ = emit_src_ext((uint32_t)out, pc[i++]); 
		break;
	case EXT_WORD_SRC_16B: 
		debug("EMIT 16-bit immediate.\n");
		*(*out)++ = emit_movw(1, pc[i++]); 
		break;
	case EXT_WORD_SRC_32B: 
		debug("EMIT 32-bit immediate.\n");
		*(*out)++ = emit_movw(1, pc[i++]);
		*(*out)++ = emit_movt(1, pc[i++]);
		break;
	}

	switch(opea & 0xF000) {
	case EXT_WORD_DST_EXT: 
		debug("EMIT extension word.\n");
		*(*out)++ = emit_dst_ext((uint32_t)out, pc[i++]); 
		break;
	case EXT_WORD_DST_16B: 
		debug("EMIT 16-bit immediate.\n");
		*(*out)++ = emit_movw(2, pc[i++]); 
		break;
	case EXT_WORD_DST_32B: 
		debug("EMIT 32-bit immediate.\n");
		*(*out)++ = emit_movw(2, pc[i++]);
		*(*out)++ = emit_movt(2, pc[i++]);
		break;
	}
	
	if(!link) {
		uint32_t x = (uint32_t)*out;
		*(*out)++ = emit_branch(opaddr, x);
	} else if((opea & 0xFF) == 1) {
		*(*out)++ = *(uint32_t*)opaddr;
	} else {
		uint32_t x = (uint32_t)*out;
		*(*out)++ = emit_branch_link(opaddr, x);
	}
	
	return i;
}

static inline void clear_cache(uint32_t *start) {
	#if __unix__
	__clear_cache(start, start+16);
	#else
	asm volatile("mcr p15, 0,%0,c7,c5,#1" :: "r"(start));
	isb(); // flush the pipeline
	#endif	
}

void wrap_pjit(int jumpto) {
	extern cpu_t cpu_state;
	cpu = &cpu_state;
	asm volatile("ldr r3, [ip, #0x84]");
	asm volatile("msr CPSR_fc, r3");
	((void(*)(void))jumpto)();
	//asm volatile("bl  %0" :: "r"(jumpto));
	asm volatile("mrs r3, CPSR");
	asm volatile("str r3, [ip, #0x84]");
}

// look up opcode and execute it, but never replace it
void cpu_lookup_nojit(void) {
	uint32_t entry;// = lr - 4;
	uint16_t *pc;// = cache_reverse(lr - 4);
	uint32_t *out;// = (uint32_t*)exec_temp;

	asm("\tmov %0, lr" : "=r"(entry));
	entry -= 4;
	pc = (uint16_t*)cache_reverse(entry);
	out = (uint32_t*)exec_temp;

	//debug("In cpu_lookup_nojit\n");

	// given an opcode, write out the necessary steps to execute it, assuming
	// we're going to run this immediately in interpreter mode which means we
	// have to restore and save the CPSR register (flags) and add an explicit
	// return -- note that we should never branch-and-link to immediate code

	// emit the opcode into a temp buffer
	uint16_t len = copy_opcode(&out, pc, false);
	
	debug("Executing single op\n");	
	clear_cache(exec_temp);
	wrap_pjit((int)exec_temp);
	
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
	goto *(void*)(entry + len * 4);
#pragma GCC diagnostic pop
}

// look up opcode and replace our call with the opcode
// and then also execute it
void cpu_lookup_safe(void) {
	uint32_t entry;// = lr - 4;
	uint16_t *pc;// = cache_reverse(lr - 4);
	//uint32_t reg = 0;

	asm("\tmov %0, lr" : "=r"(entry));
	entry -= 4;
	pc = (uint16_t*)cache_reverse(entry);

	debug("In cpu_lookup_safe, entry %08lX (pc=%08lX)\n", entry, (uint32_t)pc);

	// TODO this smells...
	(void)copy_opcode((uint32_t**)&entry, pc, true);

	// L1 Instruction and Data Cache of 32KB, 4-way, 16-word line, 128 sets
	// L2 Unified cache of 256 KB, 8-way, 16 word line, 512 sets

	//ICacheFlush(entry, end);
	#if __unix__
	__clear_cache(entry, entry+1);
	#else
	asm volatile("mcr p15, 0,%0,c7,c5,#1" :: "r"(entry));		
	isb(); // flush the pipeline
	#endif
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

	uint32_t *entry;
	uint16_t *pc;
	uint16_t inst;

	asm("\tmov %0, lr" : "=r"(entry));
	pc = (uint16_t*)cache_reverse((uint32_t)entry);
	entry--;
	inst = *pc;

	debug("In cpu_lookup_inline\n");

	// Handle in-line Bcc
	if(((inst & 0xF001) == 0x6000) && ((inst & 0xFF) != 0xFF)) {
		// TODO fold flags back into ARM status register
		// TODO change this to allow conditional expressions
		uint32_t o = arm_bcc[(inst & 0x0F00) >> 8];
		if(o != 0xFF) {
			uint32_t emit = (o << 24) | ((((uint8_t)inst) << 2) - 4);
			debug("EMIT ARM %08lX (Inlined branch)\n", emit);
			*entry = emit;
			//ICacheFlush(entry, entry + 1);
			//asm volatile("mcr p15, 0,%0,c7,c5,#1" :: "r"(entry));
			#if __unix__
			__clear_cache(entry, entry+1);
			#else
			asm volatile("mcr p15, 0,%0,c7,c5,#1" :: "r"(entry));
			isb(); // flush the pipeline
			#endif
		}
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
		goto *optab[inst];
#pragma GCC diagnostic pop
		
	} else {
		(void)copy_opcode(&entry, (uint16_t*)cache_reverse((uint32_t)entry), true);
		//ICacheFlush(entry, end);
		#if __unix__
		__clear_cache(entry, entry+1);
		#else
		asm volatile("mcr p15, 0,%0,c7,c5,#1" :: "r"(entry));
		isb(); // flush the pipeline
		#endif
		// automagically execute the PJIT cache on return
	}
}

uint32_t cpu_branch_offset(void* target, void* current) {
	int32_t _t = (int32_t)target;
	int32_t _c = (int32_t)current;
	// offset=(target-bra_addr-8)>>2
	int32_t offset = (_t - _c - 8) >> 2;
	if(offset > (int32_t)0x00FFFFFF || offset < (int32_t)0xFF000001) {
		printf("\n*** Branch out of range (%08lX).\n", offset);
		exit(1);
		
	} else {
		uint32_t op = 0xEB000000 | (0x00FFFFFF & offset);
		return op;			
	}
//	return emit_branch((uint32_t)target, (uint32_t)current);
}

void cpu_dump_state(void) {
	cpu = &cpu_state;
	printf("D%d: %08lX  ", 0, D0);
	cpu = &cpu_state;
	printf("D%d: %08lX  ", 1, D1);
	cpu = &cpu_state;
	printf("D%d: %08lX  ", 2, D2);
	cpu = &cpu_state;
	printf("D%d: %08lX\n", 3, D3);
	cpu = &cpu_state;
	printf("D%d: %08lX  ", 4, D4);
	cpu = &cpu_state;
	printf("D%d: %08lX  ", 5, D5);
	cpu = &cpu_state;
	printf("D%d: %08lX  ", 6, D6);
	cpu = &cpu_state;
	printf("D%d: %08lX\n", 7, D7);
	
	cpu = &cpu_state;
	printf("A%d: %08lX  ", 0, A0);
	cpu = &cpu_state;
	printf("A%d: %08lX  ", 1, A1);
	cpu = &cpu_state;
	printf("A%d: %08lX  ", 2, A2);
	cpu = &cpu_state;
	printf("A%d: %08lX\n", 3, A3);
	cpu = &cpu_state;
	printf("A%d: %08lX  ", 4, A4);
	cpu = &cpu_state;
	printf("A%d: %08lX  ", 5, A5);
	cpu = &cpu_state;
	printf("A%d: %08lX  ", 6, A6);
	cpu = &cpu_state;
	printf("A%d: %08lX\n", 7, A7);
	cpu = &cpu_state;
}

void cpu_exit(void) {
	longjmp(jump_buffer, PJIT_EXIT);
}

void cpu_jump(uint32_t m68k_pc) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
	goto *cache_find_entry(m68k_pc);
#pragma GCC diagnostic pop
}

void cpu_subroutine(uint32_t _lr, uint32_t new_m68k_pc) {
	//cpu = &cpu_state; // fuck Linux
	A7 -= 4; *(uint32_t*)A7 =cache_reverse(_lr);
	//m68k_pc += offset;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
	goto *cache_find_entry(new_m68k_pc);
#pragma GCC diagnostic pop
}

void relative_branch(uint32_t _lr, int32_t offset) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
	goto *cache_find_entry(cache_reverse(_lr) + offset);
#pragma GCC diagnostic pop
}

void branch_subroutine(uint32_t _lr, int32_t offset) {
	//cpu = &cpu_state; // fuck Linux
	uint32_t m68k_pc = cache_reverse(_lr);
	A7 -= 4; *(uint32_t*)A7 = m68k_pc;
	m68k_pc += offset;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
	goto *cache_find_entry(m68k_pc);
#pragma GCC diagnostic pop
}

// start may either point to the PJIT cache or the interpreter function
// if we enter the PJIT cache, we'll only return here through the setjmp
// when the system hits another interpreter command; that is, this is a
// loop, even if it doesn't look like one
void cpu_start(uint32_t m68k_pc) {
	//m68k_disasm(0);
	
	while((uint32_t)setjmp(jump_buffer) != PJIT_EXIT) {
		//static uint32_t m68k_jump_to;
		
		bzero(&cpu_state, sizeof(cpu_t));
		cpu = &cpu_state; // fuck Linux
		
		D0 = D1 = D2 = D3 = D4 = D5 = D6 = D7 = 0;
		A0 = A1 = A2 = A3 = A4 = A5 = A6 = A7 = 0;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
		goto *cache_find_entry(m68k_pc);
#pragma GCC diagnostic pop
	}
}

#if 0
// All memory above 24bit is cacheable; memory below this is split into
// 64KB chunks where we can define if it's cacheable
static struct pjit_page_ctrl_t {
    uint8_t cacheable : 1;
} pjit_ctrl[256] = { 0 };

register cpu_t cpu asm(CPU);

/*
    PJIT can run in one of two basic operating modes; interpreter and JIT.
    In interpreter mode, we'll fetch and handle each instruction one-at-a-time.
    This is really slow, but gaurantees execution in DMA-able memory where it's
    possible to have code modified during execution. For example, on the Amiga,
    all chip RAM is DMA-able by the chipset and all 24-bit fast RAM is DMA-able
    from storage devices, so we have to stick with interpreted mode for these.
    However, the ROMs are always safe to cache. 
*/
void pjit_run(void) {
    
    cpu->a7 = *(uint32_t*)0;
    cpu->pc = *(uint32_t*)4;
    
}
#endif

#undef EXIT_PJIT



