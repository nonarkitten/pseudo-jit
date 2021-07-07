/*
 * file: pjit_alu.h
 * author: Renee Cousins
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 *
 * Contains all the ALU (arithmetic logic unit) primitives
 * required for the 68000 using correct flag behaviour, etc.
 *
 * For peak efficiency, PJIT uses the ARM status registers.
 * Since GCC offers no reasonable way to get these, we need to
 * use inline assembly for each and every base ALU operation.
 *
 */

#ifndef CPU_PJIT_ALU_H_
#define CPU_PJIT_ALU_H_

#include "pjit.h"

#define save_x_flag() do { uint32_t x; \
    /* Copy the carry flag into X */ \
    ASM("movcc\t%0, #0" : "=r"(x)); \
	ASM("movcs\t%0, #1" : "=r"(x)); \
	cpu->x_flag = x; } while(0)

#define restore_x_flag(N) do {\
	uint32_t x = cpu->x_flag; \
	/* put the X flag back into the carry */ \
	ASM("rsb\t%0, %0, %1" : "+r"(x) : "n"(N)); } while(0)

/* ADD */
static uint32_t alu_add(uint32_t a, uint32_t b) {
	ASM("adds\t%0, %0, %1" : "+r"(a) : "r"(b));
	return a;
}

/* ADDX */
static inline uint32_t alu_addx(uint32_t a, uint32_t b) {
	restore_x_flag(0);
	ASM("adcst\t%0, %0, %1" : "+r"(a) : "r"(b));
	save_x_flag();
    return a;
}

/* AND */
static inline uint32_t alu_and(uint32_t a, uint32_t b) {
	ASM("ands %0, %0, %1" : "+r"(a) : "r"(b));
	return a;
}

/* ASL */
static inline uint32_t alu_asl(uint32_t a, uint32_t b) {
	ASM("lsls %0, %0, %1" : "+r"(a) : "r"(b));
	save_x_flag();
	return a;
}

/* ASR */
static inline uint32_t alu_asr(uint32_t a, uint32_t b) {
	ASM("asrs %0, %0, %1" : "+r"(a) : "r"(b));
	save_x_flag();
	return a;
}

/* BCHG */
static inline uint32_t alu_bchg(uint32_t a, uint32_t b) {
	b = 1 << b;
	ASM("tst %0, %1" :: "r"(a),"r"(b));
	ASM("eor %0, %0, %1" : "+r"(a) : "r"(b));
	return a;
}

/* BCLR */
static inline uint32_t alu_bclr(uint32_t a, uint32_t b) {
	b = 1 << b;
	ASM("tst %0, %1" :: "r"(a),"r"(b));
	ASM("bic %0, %0, %1" : "+r"(a) : "r"(b));
	return a;
}

/* BSET */
static inline uint32_t alu_bset(uint32_t a, uint32_t b) {
	b = 1 << b;
	ASM("tst %0, %1" :: "r"(a),"r"(b));
	ASM("or %0, %0, %1" : "+r"(a) : "r"(b));
	return a;
}

/* DIVUW 32b / 16b => 16b (rem) : 16b (quot) result, signed */
static inline uint32_t alu_divuw(uint32_t a, uint32_t b) {
	b = (uint32_t)(int32_t)(int16_t)b;
	// TODO: implement me
}

/* DIVSW 32b / 16b => 16b (rem) : 16b (quot) result, unsigned */
static inline uint32_t alu_divsw(uint32_t a, uint32_t b) {
	b = (uint16_t)b;
	// TODO: implement me
}

/* DIVUL 32b / 32b => 32b (quot), signed */
static inline uint32_t alu_divul(uint32_t a, uint32_t b) {
	// TODO: implement me
}

/* DIVSL 32b / 32b => 32b (quot), unsigned */
static inline uint32_t alu_divsl(uint32_t a, uint32_t b) {
	// TODO: implement me
}

/* EOR */
static inline uint32_t alu_eor(uint32_t a, uint32_t b) {
	ASM("eors %0, %0, %1" : "+r"(a) : "r"(b));
	return a;
}

/* EXTBW */
static inline uint32_t alu_extbw(uint32_t a) {
	// TODO: add flag check
	return (uint32_t)(int16_t)(int8_t)a;
}

/* EXTWL */
static inline uint32_t alu_extwl(uint32_t a) {
	// TODO: add flag check
	return (uint32_t)(int32_t)(int16_t)a;
}

/* EXTBL */
static inline uint32_t alu_extbl(uint32_t a) {
	// TODO: add flag check
	return (uint32_t)(int32_t)(int8_t)a;
}

/* LSL */
static inline uint32_t alu_lsl(uint32_t a, uint32_t b) {
	ASM("lsls %0, %0, %1" : "+r"(a) : "r"(b));
	return a;
}

/* LSR */
static inline uint32_t alu_lsr(uint32_t a, uint32_t b) {
	ASM("lsrs %0, %0, %1" : "+r"(a) : "r"(b));
	return a;
}

static inline uint32_t alu_move(uint32_t a) {
	// TODO: implement me
}

/* MUL on ARM does not set the overflow flag! */
/* MULSW 16b x 16b => 32b result, signed */
static inline uint32_t alu_mulsw(uint32_t a, uint32_t b) {
	ASM("smulbbs\t%0, %0, %1" : "+r"(a) : "r"(b));
	return a;
}

/* MULUW 16b x 16b => 32b result, unsigned */
static inline uint32_t alu_muluw(uint16_t a, uint16_t b) {
	a &= 0xFFFF; b &= 0xFFFF;
	ASM("muls\t%0, %0, %1" : "+r"(a) : "r"(b));
	return a;
}

/* MULSL */
static inline uint32_t alu_mulsl(uint32_t a, uint32_t b) {
	ASM("muls\t%0, %0, %1" : "+r"(a) : "r"(b));
	return a;
}

/* MULUL */
static inline uint32_t alu_mulul(uint32_t a, uint32_t b) {
	ASM("muls\t%0, %0, %1" : "+r"(a) : "r"(b));
	return a;
}

/* NEG */
static inline uint32_t alu_neg(uint32_t a) {
	ASM("subs\t%0, #0" : "+r"(a));
	ASM("rsb\t%0, #0" : "=r"(a));
	return a;
}

/* NEGX */
static inline uint32_t alu_negx(uint32_t a) {
	ASM("subs\t%0, #0" : "+r"(a));
	ASM("rsb\t%0, #0" : "=r"(a));
	save_x_flag();
	return a;
}

/* NOT */
static inline uint32_t alu_not(uint32_t a) {
	ASM("eors\t%0, #ffffffff" : "+r"(a));
    return a;
}

/* OR */
static inline uint32_t alu_or(uint32_t a, uint32_t b) {
	ASM("ors\t%0, %0, %1" : "+r"(a) : "r"(b));
	return a;
}

/* SUB */
// carry is set whenever there is no borrow and clear whenever there is
// a - b		borrow := b > a, c = !borrow (ARM style)
// -( b - a )   borrow := a > b, c =  borrow (68K style)
static inline uint32_t alu_sub(uint32_t a, uint32_t b) {
	ASM("rsbs\t%0, %0, %1" : "=r"(a) : "r"(b));
	ASM("rsb\t%0, #0" : "=r"(a));
	return a;
}

/* SUBX */
static inline uint32_t alu_subx(uint32_t a, uint32_t b) {
    restore_x_flag(1);
	// perform regular sub-with-carry
	ASM("rscs\t%0, %0, %1" : "+r"(a) : "r"(b));
	ASM("rsb\t%0, #0" : "+r"(a));
	save_x_flag();
	return a;
}

/* SWAP */
static inline uint32_t alu_swap(uint32_t a) {
	ASM("rors\t%0, %0, #16" : "+r"(a));
	return a;
}

/* TAS */
static inline uint32_t alu_tas(uint32_t a) {
	// TODO: implement me
	return a;
}

#endif /* CPU_PJIT_ALU_H_ */
