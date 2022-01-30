#include "ops/m68k_common.h"
#include "ops/m68k_cpu.h"
#include "ops/m68k_emit_ea.h"
#include "ops/m68k_registers.h"

__attribute__((always_inline))
static inline unsigned __get_cpsr(void) {
    unsigned long retval;
    asm volatile ("mrs\t%0, cpsr" : "=r" (retval) :  );
    return retval;
}

__attribute__((always_inline))
static inline void __set_cpsr(unsigned val) {
    asm volatile ("msr\tcpsr, %0":  :"r" (val));
}

__attribute__((always_inline))
static inline unsigned __get_cpsr_x(const int is_sub) {
    unsigned long retval;
	asm volatile ("ldr\t%0, [r12, %1]" : "=r"(retval) : "i"(offsetof(cpu_t, x)));
	asm volatile ("rsb\t%0, %0, %1" : "+r"(retval) : "i"(is_sub ? 1 : 0));
    asm volatile ("mrs\t%0, cpsr" : "=r" (retval) :  );
    return retval;
}

__attribute__((always_inline))
static inline void __set_cpsr_x(unsigned val) {
    asm volatile ("msr\tcpsr, %0" : :"r" (val));
	asm volatile ("movcc\t%0, #0" : "=r" (val));
	asm volatile ("movcs\t%0, #1" : "=r" (val));
	asm volatile ("str\t%0, [r12, %1]" : "=r"(val) : "i"(offsetof(cpu_t, x)));
}

unsigned handle_abcd(uint8_t xx, uint8_t yy) {
    cpsr_t cpsr = { .word = __get_cpsr_x(0) };

	unsigned ss = xx + yy + cpsr.C;
	// Normal carry computation for addition:
	// (sm & dm) | (~rm & dm) | (sm & ~rm)
	unsigned bc = ((xx & yy) | (~ss & xx) | (~ss & yy)) & 0x88;
	// Compute if we have a decimal carry in both nibbles.
	// Note: 0x66 is type "int", so the entire computation is
	// promoted to "int", which is why the "& 0x110" works.
	unsigned dc = (((ss + 0x66) ^ ss) & 0x110) >> 1;
	unsigned corf = (bc | dc) - ((bc | dc) >> 2);
	unsigned rr = ss + corf;

	// Compute flags.
	// Carry has two parts: normal carry for addition
	// (computed above) OR'ed with normal carry for
	// addition with corf:
	// (sm & dm) | (~rm & dm) | (sm & ~rm)
	// but simplified because sm = 0 and ~sm = 1 for corf:
	cpsr.C = (bc | (ss & ~rr)) >> 7;
	// Normal overflow computation for addition with corf:
	// (sm & dm & ~rm) | (~sm & ~dm & rm)
	// but simplified because sm = 0 and ~sm = 1 for corf:
	cpsr.V = (~ss & rr) >> 7;
	// Accumulate zero flag:
	cpsr.Z &= (rr == 0);
	cpsr.N = rr >> 7;

    __set_cpsr_x(cpsr.word);
    return rr;
}

unsigned handle_sbcd(uint8_t xx, uint8_t yy) {
    cpsr_t cpsr = { .word = __get_cpsr_x(0) };

	unsigned dd = xx - yy - cpsr.C;
	// Normal carry computation for subtraction:
	// (sm & ~dm) | (rm & ~dm) | (sm & rm)
	unsigned bc = ((~xx & yy) | (dd & ~xx) | (dd & yy)) & 0x88;
	unsigned corf = bc - (bc >> 2);
	unsigned rr = dd - corf;
	// Compute flags.
	// Carry has two parts: normal carry for subtraction
	// (computed above) OR'ed with normal carry for
	// subtraction with corf:
	// (sm & ~dm) | (rm & ~dm) | (sm & rm)
	// but simplified because sm = 0 and ~sm = 1 for corf:
	cpsr.C = (bc | (~dd & rr)) >> 7;
	// Normal overflow computation for subtraction with corf:
	// (~sm & dm & ~rm) | (sm & ~dm & rm)
	// but simplified because sm = 0 and ~sm = 1 for corf:
	cpsr.V = (dd & ~rr) >> 7;
	// Accumulate zero flag:
	cpsr.Z &= (rr == 0);
	cpsr.N = rr >> 7;

    __set_cpsr_x(cpsr.word);
	return rr;
}

unsigned handle_nbcd(uint8_t xx) {
    cpsr_t cpsr = { .word = __get_cpsr_x(0) };

	// Equivalent to
	//return sbcd(ctx, 0, xx);
	// but slightly optimized.
	unsigned dd = - xx - cpsr.C;
	// Normal carry computation for subtraction:
	// (sm & ~dm) | (rm & ~dm) | (sm & rm)
	// but simplified because dm = 0 and ~dm = 1 for 0:
	unsigned bc = (xx | dd) & 0x88;
	unsigned corf = bc - (bc >> 2);
	unsigned rr = dd - corf;
	// Compute flags.
	// Carry has two parts: normal carry for subtraction
	// (computed above) OR'ed with normal carry for
	// subtraction with corf:
	// (sm & ~dm) | (rm & ~dm) | (sm & rm)
	// but simplified because sm = 0 and ~sm = 1 for corf:
	cpsr.C = (bc | (~dd & rr)) >> 7;
	// Normal overflow computation for subtraction with corf:
	// (~sm & dm & ~rm) | (sm & ~dm & rm)
	// but simplified because sm = 0 and ~sm = 1 for corf:
	cpsr.V = (dd & ~rr) >> 7;
	// Accumulate zero flag:
	cpsr.Z &= (rr == 0);
	cpsr.N = rr >> 7;
    
    __set_cpsr_x(cpsr.word);
	return rr;
}

unsigned handle_roxl_immd(uint8_t tR2, uint16_t amt, uint8_t size) { }
unsigned handle_roxr_immd(uint8_t tR2, uint16_t amt, uint8_t size) { }
unsigned handle_roxl_reg(uint8_t tR2, uint16_t amt, uint8_t size) { }
unsigned handle_roxr_reg(uint8_t tR2, uint16_t amt, uint8_t size) { }
