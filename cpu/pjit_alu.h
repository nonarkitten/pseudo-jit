/*
 * Copyright (c) 2020-2021 Renee Cousins, the Buffee Project
 * See pjit.h for details
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

#define ALU_B(op,S,D) D = op((D)<<24,(S)<<24)>>24;
#define ALU_W(op,S,D) D = op((D)<<16,(S)<<16)>>16;
#define ALU_L(op,S,D) D = op((D),(S));

#define save_x_flag() do { uint32_t x; \
    /* Copy the carry flag into X */ \
    ASM("movcc\t%0, #0" : "=r"(x)); \
	ASM("movcs\t%0, #1" : "=r"(x)); \
	cpu->x_flag = x; } while(0)

#define restore_x_flag(N) do {\
	uint32_t x = cpu->x_flag; \
	/* put the X flag back into the carry */ \
	ASM("rsb\t%0, %0, %1" : "+r"(x) : "n"(N)); \
	} while(0)

/* ABDC */
static uint8_t alu_abcd(uint8_t a, uint8_t b) {
	uint32_t s = (a & 0x0F) + (b & 0x0F) + !!(cpu->x_flag);
	if(s >= 0x0A) s += 0x06;
	s += (a & 0xF0) + (b & 0xF0);
	if(s >= 0xA0) s += 0x60;

	cpu->x_flag = sr.X = sr.C = !!(s & 0xF00);
	sr.Z = !!(0 == (s & 0xFF));
	pjit_restore_ccr();

	return s;
}

/* ADD */
static uint32_t alu_add(uint32_t a, uint32_t b) {
	ASM("adds\t%0, %0, %1" : "+r"(a) : "r"(b));
	return a;
}

/* ADDX */
static inline uint32_t alu_addx(uint32_t a, uint32_t b) {
	restore_x_flag(0);
	ASM("adcs\t%0, %0, %1" : "+r"(a) : "r"(b));
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
	ASM("orr %0, %0, %1" : "+r"(a) : "r"(b));
	return a;
}

/* BTST */
static inline uint32_t alu_btst(uint32_t a, uint32_t b) {
	b = 1 << b;
	ASM("tst %0, %1" :: "r"(a),"r"(b));
	return a;
}

/* CMP */
static inline uint32_t alu_cmp(uint32_t a, uint32_t b) {
	ASM("rsbs\t%0, %0, %1" : "=r"(a) : "r"(b));
	return a;
}

///* DIVUW 32b / 16b => 16b (rem) : 16b (quot) result, signed */
//static inline uint32_t alu_divuw(uint32_t a, uint32_t b) {
//    if (!b) pjit_exception(pc, inst, DIVZ);
//    div_t div_result = div(a, (uint16_t)b);
//    if ((div_result.quot > 32767) || (div_result.quot < -32768)) {
//    	sr.V = 1, sr.C = 0;
//    } else {
//    	a = (div_result.rem << 16) | (uint16_t)div_result.quot;
//        sr.N = (a < 0), sr.Z = (a == 0);
//        sr.C = sr.V = 0;
//    }
//	pjit_restore_ccr();
//	return a;
//}
//
///* DIVSW 32b / 16b => 16b (rem) : 16b (quot) result, unsigned */
//static inline uint32_t alu_divsw(uint32_t a, uint32_t b) {
//    if (!b) pjit_exception(pc, inst, DIVZ);
//    div_t div_result = div(a, (int16_t)b);
//    if (((uint32_t)div_result.quot) & 0xffff0000) {
//    	sr.V = 1, sr.C = 0;
//    } else {
//    	a = (div_result.rem << 16) | (uint16_t)div_result.quot;
//        sr.N = (a < 0), sr.Z = (a == 0);
//        sr.C = sr.V = 0;
//    }
//	pjit_restore_ccr();
//	return a;
//}
//
///* DIVUL 32b / 32b => 32b (quot), signed */
//static inline uint32_t alu_divul(uint32_t a, uint32_t b) {
//    if (!b) pjit_exception(pc, inst, DIVZ);
//	// TODO: implement me
//}
//
///* DIVSL 32b / 32b => 32b (quot), unsigned */
//static inline uint32_t alu_divsl(uint32_t a, uint32_t b) {
//    if (!b) pjit_exception(pc, inst, DIVZ);
//	// TODO: implement me
//}
//
///* DIVUL 64b / 32b => 32b (quot), signed */
//static inline uint32_t alu_divull(uint64_t a, uint32_t b) {
//    if (!b) pjit_exception(pc, inst, DIVZ);
//	// TODO: implement me
//}
//
///* DIVSL 64b / 32b => 32b (quot), unsigned */
//static inline uint32_t alu_divsll(uint64_t a, uint32_t b) {
//    if (!b) pjit_exception(pc, inst, DIVZ);
//	// TODO: implement me
//}
/* EOR */
static inline uint32_t alu_eor(uint32_t a, uint32_t b) {
	ASM("eors %0, %0, %1" : "+r"(a) : "r"(b));
	return a;
}

/* EXTBW */
static inline uint32_t alu_extbw(uint32_t a) {
	uint32_t junk;
	a = (a & 0xFFFF0000u) | ((int16_t)(int8_t)a);
	ASM("rors %0, %1, #16" : "+r"(junk) : "r"(a));
	return a;
}

/* EXTWL */
static inline uint32_t alu_extwl(uint32_t a) {
	a = (uint32_t)(int32_t)(int16_t)a;
	ASM("tst %0, #0" :: "r"(a));
	return a;
}

/* EXTBL */
static inline uint32_t alu_extbl(uint32_t a) {
	a = (uint32_t)(int32_t)(int8_t)a;
	ASM("tst %0, #0" :: "r"(a));
	return a;
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
	ASM("tst %0, #0" :: "r"(a));
	return a;
}

/* MUL on ARM does not set the overflow flag! */
/* MULSW 16b x 16b => 32b result, signed */
static inline uint32_t alu_mulsw(uint32_t a, uint32_t b) {
	//ASM("smulbbs\t%0, %0, %1" : "+r"(a) : "r"(b));
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

/* NBDC */
static uint8_t alu_nbcd(uint8_t a) {
    int32_t s = -(a & 0xF) - !!(cpu->x_flag);
    if (s > 9) s -= 6;
    s -= (a & 0xF0);
    if(sr.C = ((s & 0x1F0) > 0x90)) s -= 0x60;
	sr.Z = !!(0 == (s & 0xFF));
	pjit_restore_ccr();

	return s;
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
	ASM("mvns\t%0, %0" : "+r"(a));
    return a;
}

/* OR */
static inline uint32_t alu_or(uint32_t a, uint32_t b) {
	ASM("orrs\t%0, %0, %1" : "+r"(a) : "r"(b));
	return a;
}

/* ROL */
// c < -------------------------------- <.
// '-------------------------------------'
static inline uint8_t alu_rol_b(uint8_t a, uint8_t b) {
	uint32_t junk;
	if(b &= 7) a = (a << b) | (a >> (8 - b));
	ASM("rors\t%1, %0, #1" : "+r"(a),"=r"(junk));
	return a;
}

static inline uint16_t alu_rol_w(uint16_t a, uint16_t b) {
	uint32_t junk;
	if(b &= 15) a = (a << b) | (a >> (16 - b));
	ASM("rors\t%1, %0, #1" : "+r"(a),"=r"(junk));
	return a;
}

static inline uint32_t alu_rol_l(uint32_t a, uint32_t b) {
	uint32_t junk;
	if(b &= 31) a = (a << b) | (a >> (32 - b));
	ASM("rors\t%1, %0, #1" : "+r"(a),"=r"(junk));
	return a;
}

/* ROR */
static inline uint8_t alu_ror_b(uint8_t a, uint8_t b) {
	uint32_t junk;
	if(b &= 7) a = (a >> b) | (a << (8 - b));
	ASM("lsls\t%1, %0, #25" : "+r"(a),"=r"(junk));
	return a;
}

static inline uint16_t alu_ror_w(uint16_t a, uint16_t b) {
	uint32_t junk;
	if(b &= 15) a = (a >> b) | (a << (16 - b));
	ASM("lsls\t%1, %0, #17" : "+r"(a),"=r"(junk));
	return a;
}

static inline uint32_t alu_ror_l(uint32_t a, uint32_t b) {
	ASM("rors\t%0, %0, %1" : "+r"(a) : "r"(b));
	return a;
}

static const uint8_t mod9[] = {
		0, 1, 2, 3, 4, 5, 6, 7, 8,
		0, 1, 2, 3, 4, 5, 6, 7, 8,
		0, 1, 2, 3, 4, 5, 6, 7, 8,
		0, 1, 2, 3, 4, 5, 6, 7, 8,
		0, 1, 2, 3, 4, 5, 6, 7, 8,
		0, 1, 2, 3, 4, 5, 6, 7, 8,
		0, 1, 2, 3, 4, 5, 6, 7, 8,
		0, 1, 2, 3, 4, 5, 6, 7, 8,
};
static const uint8_t mod17[] = {
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
};
static const uint8_t mod33[] = {
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17,
		18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17,
		18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
};
/* ROXL */
static inline uint8_t alu_roxl_b(uint8_t a, uint8_t b) {
	uint32_t junk;
	uint32_t _a = a | (cpu->x_flag) ? 0x800 : 0;
	if(b = mod9[b & 63]) _a = (_a << b) | (_a >> (9 - b));
	ASM("rors\t%1, %0, #1" : "+r"(_a),"=r"(junk));
	cpu->x_flag = _a & 0x800;
	return _a;
}

static inline uint16_t alu_roxl_w(uint16_t a, uint16_t b) {
	uint32_t junk;
	uint32_t _a = a | (cpu->x_flag) ? 0x80000 : 0;
	if(b = mod17[b & 63]) _a = (_a << b) | (_a >> (17 - b));
	ASM("rors\t%1, %0, #1" : "+r"(_a),"=r"(junk));
	cpu->x_flag = _a & 0x80000;
	return a;
}

static inline uint32_t alu_roxl_l(uint32_t a, uint32_t b) {
	// TODO: implement me
	return a;
}

/* ROXR */
static inline uint8_t alu_roxr_b(uint8_t a, uint8_t b) {
	uint32_t junk;
	uint32_t _a = a | (cpu->x_flag) ? 0x800 : 0;
	if(b = mod9[b & 63]) _a = (_a >> b) | (_a << (9 - b));
	ASM("rors\t%1, %0, #1" : "+r"(_a),"=r"(junk));
	cpu->x_flag = _a & 0x800;
	return _a;
}

static inline uint16_t alu_roxr_w(uint16_t a, uint16_t b) {
	uint32_t junk;
	uint32_t _a = a | (cpu->x_flag) ? 0x80000 : 0;
	if(b = mod9[b & 63]) _a = (_a >> b) | (_a << (17 - b));
	ASM("rors\t%1, %0, #1" : "+r"(_a),"=r"(junk));
	cpu->x_flag = _a & 0x80000;
	return a;
}

static inline uint32_t alu_roxr_l(uint32_t a, uint32_t b) {
	// TODO: implement me
	return a;
}

/* SBDC */
static uint8_t alu_sbcd(uint8_t a, uint8_t b) {
	int32_t l = (a & 0x0F) - (b & 0x0F) - !!(cpu->x_flag);
	int32_t h = (a & 0xF0) - (b & 0xF0);
	if(l < 0) { l += 0x0A; h -= 0x10; }
	if(h < 0) { h -= 0x60; }
	l += h;

	cpu->x_flag = sr.X = sr.C = !!(l & 0xF00);
	sr.Z = !!(0 == (l & 0xFF));
	pjit_restore_ccr();

	return l;
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
	ASM("tst %0, #0" :: "r"(a));
	a |= 0x80;
	return a;
}

static inline uint32_t alu_tst(uint32_t a) {
	ASM("tst %0, #0" :: "r"(a));
	return a;
}

#endif /* CPU_PJIT_ALU_H_ */
