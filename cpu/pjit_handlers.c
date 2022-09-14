#include "ops/m68k_common.h"
#include "ops/m68k_cpu.h"
#include "ops/m68k_emit_ea.h"
#include "ops/m68k_registers.h"

__attribute__((always_inline))
static inline unsigned GET_X(void) {
    uint32_t x;
    asm volatile(
        "ldr\t%0, [" CPU ", %1]"
        : "=r"(x) : "i"(offsetof(cpu_t, x))
    );
	return x;
}

__attribute__((always_inline))
static inline unsigned SET_X(int bcd) {
    uint32_t temp;
    asm volatile(
        "lsls\t%0, %1, #24\n\t"
        "movcc\t%0, #0\n\t"
        "movcs\t%0, #1\n\t"
        "str\t%0, [" CPU ", %2]"
        : "=r"(temp) : "r"(bcd),"i"(offsetof(cpu_t, x))
    );
    return bcd;
}

unsigned handle_abcd(uint8_t xx, uint8_t yy) {
	unsigned ss = xx + yy + GET_X();
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
    return SET_X(rr);
}

unsigned handle_sbcd(uint8_t xx, uint8_t yy) {
	unsigned dd = xx - yy - GET_X();
	// Normal carry computation for subtraction:
	// (sm & ~dm) | (rm & ~dm) | (sm & rm)
	unsigned bc = ((~xx & yy) | (dd & ~xx) | (dd & yy)) & 0x88;
	unsigned corf = bc - (bc >> 2);
	unsigned rr = dd - corf;
	// Compute flags.
	return SET_X(rr);
}

unsigned handle_nbcd(uint8_t xx) {
	// Equivalent to sbcd(ctx, 0, xx);
	// but slightly optimized.
	unsigned dd = - xx - GET_X();
	// Normal carry computation for subtraction:
	// (sm & ~dm) | (rm & ~dm) | (sm & rm)
	// but simplified because dm = 0 and ~dm = 1 for 0:
	unsigned bc = (xx | dd) & 0x88;
	unsigned corf = bc - (bc >> 2);
	unsigned rr = dd - corf;
	// Compute flags.
	return SET_X(rr);
}

// TODO: Finish me!!
unsigned handle_roxl_immd(uint8_t tR2, uint16_t amt, uint8_t size) { (void)tR2; (void)amt; (void)size; return 0; }
unsigned handle_roxr_immd(uint8_t tR2, uint16_t amt, uint8_t size) { (void)tR2; (void)amt; (void)size; return 0; }
unsigned handle_roxl_reg(uint8_t tR2, uint16_t amt, uint8_t size) {  (void)tR2; (void)amt; (void)size; return 0; }
unsigned handle_roxr_reg(uint8_t tR2, uint16_t amt, uint8_t size) {  (void)tR2; (void)amt; (void)size; return 0; }
