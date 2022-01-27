/*
**  alu.h
**
**  Contains all the ALU (arithmetic logic unit) primitives
**  required for the 68000 using correct flag behaviour, etc.
**
**  For many operations there is a register-to-register form
**  as well as a register-to-immediate. Don't confuse immediate
**  here with preloading a variable with a value, these have
**  to be compile-time constants. For everything else, just use
**  the register-to-register version.
**
*/

#include <stdint.h>


/* Everything else we'll put into a RAM based struct with a
** single pointer to the base
*/
typedef struct {
    uint32_t d[8];          // Lt Data registers
    uint32_t a[8];          // Place to store address reg's
    uint32_t page;          // Base address of the current cache page
    uint32_t x_flag;
} cpu_t; 

static cpu_t __cpu;

/* These registers are to be kept in ARM registers
** for maximum performance; if there's any changes
** made here, be sure to also fix up pjit.c as well
*/
register uint32_t __d0 asm("r4");
register uint32_t __d1 asm("r5");

register uint32_t __a0 asm("r6");
register uint32_t __a1 asm("r7");

register uint32_t __a4 asm("r8");
register uint32_t __a5 asm("r9");
register uint32_t __a6 asm("r10");
register uint32_t __a7 asm("r11");

register cpu_t* cpu asm("r12");

#define D0 __d0
#define D1 __d1
#define D2 cpu->d[2]
#define D3 cpu->d[3]
#define D4 cpu->d[4]
#define D5 cpu->d[5]
#define D6 cpu->d[6]
#define D7 cpu->d[7]

#define A0 __a0
#define A1 __a1
#define A2 cpu->a[2]
#define A3 cpu->a[3]
#define A4 __a4
#define A5 __a5
#define A6 __a6
#define A7 __a7

#define ASM __asm__ __volatile__

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
static uint32_t add(uint32_t a, uint32_t b) {
	ASM("adds\t%0, %0, %1" : "+r"(a) : "r"(b));
	return a;
}

/* ADDX */
static inline uint32_t addx(uint32_t a, uint32_t b) { 
	restore_x_flag(0);
	ASM("adcst\t%0, %0, %1" : "+r"(a) : "r"(b)); 
	save_x_flag();
    return a;
}

/* AND */
static inline uint32_t and(uint32_t a, uint32_t b) {
	ASM("ands %0, %0, %1" : "+r"(a) : "r"(b));
	return a;
}

/* ASL */
static inline uint32_t asl(uint32_t a, uint32_t b) {
	ASM("lsls %0, %0, %1" : "+r"(a) : "r"(b));
	save_x_flag();
	return a;
}

/* ASR */
static inline uint32_t asr(uint32_t a, uint32_t b) {
	ASM("asrs %0, %0, %1" : "+r"(a) : "r"(b));
	save_x_flag();
	return a;
}

/* BCHG */
static inline uint32_t bchg(uint32_t a, uint32_t b) {
	b = 1 << b;
	ASM("tst %0, %1" :: "r"(a),"r"(b));
	ASM("eor %0, %0, %1" : "+r"(a) : "r"(b));
	return a;
}

/* BCLR */
static inline uint32_t bclr(uint32_t a, uint32_t b) {
	b = 1 << b;
	ASM("tst %0, %1" :: "r"(a),"r"(b));
	ASM("bic %0, %0, %1" : "+r"(a) : "r"(b));
	return a;
}

/* BITREV */
// static inline uint32_t bitrev(uint32_t a) {
// 	ASM("rbit %0, %0" : "+r" (a));
// 	return a;
// }

/* BSET */
static inline uint32_t bset(uint32_t a, uint32_t b) {
	b = 1 << b;
	ASM("tst %0, %1" :: "r"(a),"r"(b));
	ASM("or %0, %0, %1" : "+r"(a) : "r"(b));
	return a;
}

/* BYTEREV */
// static inline uint32_t divsw_r(uint32_t a) {
// 	ASM("rev %0, %0" : "=r" (a));
// 	return a;
// }

/* DIVUW 32b / 16b => 16b (rem) : 16b (quot) result, signed */
static inline uint32_t divuw(uint32_t a, uint32_t b) {
	b = (uint32_t)(int32_t)(int16_t)b;
}

/* DIVSW 32b / 16b => 16b (rem) : 16b (quot) result, unsigned */
static inline uint32_t divsw(uint32_t a, uint32_t b) {
	b = (uint16_t)b;
}

/* DIVUL 32b / 32b => 32b (quot), signed */
static inline uint32_t divul(uint32_t a, uint32_t b) {
}

/* DIVSL 32b / 32b => 32b (quot), unsigned */
static inline uint32_t divsl(uint32_t a, uint32_t b) {
}

/* EOR */
static inline uint32_t eor(uint32_t a, uint32_t b) {
	ASM("eors %0, %0, %1" : "+r"(a) : "r"(b));
	return a;
}

/* EXTBW */
static inline uint32_t extbw(uint32_t a) {
	return (uint32_t)(int16_t)(int8_t)a;
}

/* EXTWL */
static inline uint32_t extwl(uint32_t a) {
	return (uint32_t)(int32_t)(int16_t)a;
}

/* EXTBL */
static inline uint32_t extbl(uint32_t a) {
	return (uint32_t)(int32_t)(int8_t)a;
}

/* FF1 */
// static inline uint32_t ff1(uint32_t a) {
// 	ASM("clz %0, %0" : "=r"(a));
// 	return a;
// }

/* LSL */
static inline uint32_t lsl(uint32_t a, uint32_t b) {
	ASM("lsls %0, %0, %1" : "+r"(a) : "r"(b));
	return a;
}

/* LSR */
static inline uint32_t lsr(uint32_t a, uint32_t b) {
	ASM("lsrs %0, %0, %1" : "+r"(a) : "r"(b));
	return a;
}

/* MUL on ARM does not set the overflow flag! */
/* MULSW 16b x 16b => 32b result, signed */
static inline uint32_t mulsw(uint32_t a, uint32_t b) {
	ASM("smulbbs\t%0, %0, %1" : "+r"(a) : "r"(b));
	return a;
}

/* MULUW 16b x 16b => 32b result, unsigned */
static inline uint32_t muluw(uint16_t a, uint16_t b) {
	a &= 0xFFFF; b &= 0xFFFF;
	ASM("muls\t%0, %0, %1" : "+r"(a) : "r"(b));
	return a;
}

/* MULSL */
static inline uint32_t mulsl(uint32_t a, uint32_t b) {
	ASM("muls\t%0, %0, %1" : "+r"(a) : "r"(b));
	return a;
}

/* MULUL */
static inline uint32_t mulul(uint32_t a, uint32_t b) {
	ASM("muls\t%0, %0, %1" : "+r"(a) : "r"(b));
	return a;
}

/* NEG */
static inline uint32_t neg(uint32_t a) {
	ASM("subs\t%0, #0" : "+r"(a));
	ASM("rsb\t%0, #0" : "=r"(a));
	return a;
}

/* NEGX */
static inline uint32_t negx(uint32_t a) {
	ASM("subs\t%0, #0" : "+r"(a));
	ASM("rsb\t%0, #0" : "=r"(a));
	save_x_flag();
	return a;
}

/* NOT */
static inline uint32_t not(uint32_t a) {
	ASM("eors\t%0, #ffffffff" : "+r"(a));
    return a;
}

/* OR */
static inline uint32_t or(uint32_t a, uint32_t b) {
	ASM("ors\t%0, %0, %1" : "+r"(a) : "r"(b));
	return a;
}

/* SATS */

/* SUB */
// carry is set whenever there is no borrow and clear whenever there is
// a - b		borrow := b > a, c = !borrow (ARM style)
// -( b - a )   borrow := a > b, c =  borrow (68K style)
static inline uint32_t sub(uint32_t a, uint32_t b) {
	ASM("rsbs\t%0, %0, %1" : "=r"(a) : "r"(b));
	ASM("rsb\t%0, #0" : "=r"(a));
	return a;
}

/* SUBX */
static inline uint32_t subx(uint32_t a, uint32_t b) {
    restore_x_flag(1);
	// perform regular sub-with-carry
	ASM("rscs\t%0, %0, %1" : "+r"(a) : "r"(b));
	ASM("rsb\t%0, #0" : "+r"(a));
	save_x_flag();
	return a;
}

/* SWAP */
static inline uint32_t swap(uint32_t a) {
	ASM("rors\t%0, %0, #16" : "+r"(a));
	return a;
}

/* TAS */
static inline uint32_t tas(uint32_t a) {
	return a;
}


// TEST
void subx_d0_d1(void) {
    D1 = subxs(D1, D0);
    //D0 = ors_i(D2,7);
    //D0 = D1;
}




