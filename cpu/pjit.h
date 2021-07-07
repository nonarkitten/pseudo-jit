/*
 * file: pjit_ops.h
 * author: Renee Cousins
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 *
 * This is the global header for everything PJIT commonly needs
 *
 */

#include <stdint.h>

#include "pjit_ops.h"
#include "pjit_cache.h"
#include "pjit_ea_modes.h"

#ifndef CPU_PJIT_H_
#define CPU_PJIT_H_

/***
 *     __   ___  ___         ___  __
 *    |  \ |__  |__  | |\ | |__  /__'
 *    |__/ |___ |    | | \| |___ .__/
 *
 */

#define ASM __asm__ __volatile__

/* Interrupt Requests may be delayed for up
** to 5 opcode fetches */
#define DELAY_QIRQ

#define D0 (cpu->d[0])
#define D1 (cpu->d[1])
#define D2 (cpu->d[2])
#define D3 (cpu->d[3])
#define D4 (cpu->d[4])
#define D5 (cpu->d[5])
#define D6 (cpu->d[6])
#define D7 (cpu->d[7])

#define A0 (cpu->a[0])
#define A1 (cpu->a[1])
#define A2 (cpu->a[2])
#define A3 (cpu->a[3])
#define A4 (cpu->a[4])
#define A5 (cpu->a[5])
#define A6 (cpu->a[6])
#define A7 (cpu->a[7])

/***
 *    ___      __   ___  __   ___  ___  __
 *     |  \ / |__) |__  |  \ |__  |__  /__'
 *     |   |  |    |___ |__/ |___ |    .__/
 *
 */

/* Allowed CPU types */
typedef enum {
	MC68000,
	MC68030
} cpu_type_t;

const inline cpu_type_t CPU_TYPE = MC68000;

/* Exception vector numbers (trap numbers). */
typedef enum {
	RESET_SP,
	RESET_PC,
	BUSERR,
	ADDRESSERR,
	ILLINSTR,
	DIVZ,
	TRAPCHK,
	TRAPV,
	PRIV,
	TRACE,
	LINE_A,
	LINE_F,

	FORMATERR = 14,

	AUTOINT1 = 25,
	AUTOINT2,
	AUTOINT3,
	AUTOINT4,
	AUTOINT5,
	AUTOINT6,
	AUTOINT7,

	TRAP0,
	TRAP1,
	TRAP2,
	TRAP3,
	TRAP4,
	TRAP5,
	TRAP6,
	TRAP7,
	TRAP8,
	TRAP9,
	TRAP10,
	TRAP11,
	TRAP12,
	TRAP13,
	TRAP14,
	TRAP15

} vector_t;

typedef struct {
	union {
		uint32_t reg[32];
		struct {
			uint32_t d[8];   // Data Registers
			uint32_t a[8];	 // Address Registers
			uint32_t sfc;    // Source Function Code
			uint32_t usp;    // User Stack Pointer
			uint32_t dfc;    // Destination Function Code
			uint32_t vbr;    // Vector Base Register
			uint32_t cacr;   // Cache Control Register
			uint32_t caar;   // Cache Address Register
			uint32_t tc;     // MMU Translation Control Register
			uint32_t msp;    // Master Stack Pointer
			uint32_t itt0;   // Instruction Transparent Translation Register 0
			uint32_t isp;    // Interrupt Stack Pointer
			uint32_t itt1;   // Instruction Transparent Translation Register 1
			uint32_t mmusr;  // MMU Status Register
			uint32_t dtt0;   // Data Transparent Translation Register 0
			uint32_t urp;    // User Root Pointer
			uint32_t dtt1;   // Data Transparent Translation Register 1
			uint32_t srp;    // Supervisor Root Pointer
		};
	};
    uint32_t page;			 // Current cache page we're in
    uint32_t x_flag;		 // Quick-n-Dirty x-flag copy
} cpu_t;

typedef struct {
	uint32_t opcode_fn[8192];
	uint8_t operand_len[8192];
} jump_t;

typedef union {
	uint16_t status_register;
	struct {
		uint8_t system_byte;
		uint8_t user_byte;
	};
	struct {
		uint16_t T : 1;
		uint16_t   : 1;
		uint16_t S : 1;
		uint16_t   : 2;
		uint16_t I : 3;
		uint16_t   : 3;
		uint16_t X : 1;
		uint16_t N : 1;
		uint16_t Z : 1;
		uint16_t V : 1;
		uint16_t C : 1;
	};
} sr_ccr_t;

/***
 *     __   __                          __        ___  __   __
 *    /  ' |__) |  |    |__|  /\  |\ | |  \ |    |__  |__) /__'
 *    \__, |    \__/    |  | /--\ | \| |__/ |___ |___ |  \ .__/
 *
 */

/* Group 0 exceptions (address error / bus error) */
extern void pjit_exception_group_0(int32_t, uint32_t, int32_t);

/* Group 1 exceptions: illegal instruction, privilege violation, interrupts */
extern void pjit_exception_group_1(int32_t);
extern void pjit_interrupt(void);

/* Group 2 exceptions: traps, divide by zero */
extern void pjit_exception_group_2(int32_t number);

/* Hardware Reset */
extern void pjit_HW_reset(void);
/* Illegal instruction */
extern void pjit_illegal_inst(void);
/* Axxx instruction opcode  */
extern void pjit_line_a_emu(void);
/* Fxxx instruction opcode */
extern void pjit_line_f_emu(void);
/* Stop instruction */
extern void pjit_stop(void);
/* Called whenever trace mode changes */
extern void pjit_trace(void);
/* Start the main emulator */
extern void pjit_run(void);


/***
 *               __          __        ___  __
 *    \  /  /\  |__) |  /\  |__) |    |__  /__'
 *     \/  /--\ |  \ | /--\ |__) |___ |___ .__/
 *
 */

register cpu_t*   cpu  asm("r4");
register sr_ccr_t sr   asm("r5");
register uint32_t pc   asm("r6");
register uint16_t inst asm("r7");
register uint32_t lr   asm("lr");
register jump_t*  jt   asm("r8");
/***
 *                        ___  __
 *    | |\ | |    | |\ | |__  /__'
 *    | | \| |___ | | \| |___ .__/
 *
 */
#define MEMADDRMASK 0x00fffffful

static inline void pjit_check_address(uint32_t addr) {
	if (addr & 0x1) ExceptionGroup0(ADDRESSERR, addr, 1);
}

/* Fetch byte from address */
static inline uint8_t pjit_get_mem_byte(uint32_t addr) {
    return *(uint8_t*)((addr ^ 1) & MEMADDRMASK);
}
/* Fetch word, address may not be word-aligned */
static inline uint16_t pjit_get_mem_word(uint32_t addr) {
	pjit_check_address(addr);
    return *(uint16_t*)(addr & MEMADDRMASK);
}
/* Fetch long, address may not be word-aligned */
static inline uint32_t pjit_get_mem_long(uint32_t addr) {
	pjit_check_address(addr);
    uint32_t value = *(long*)(addr & MEMADDRMASK);
    asm("ror %0, %0, #16" : "+r"(value));
    return value;
}

/* Write byte to address */
static inline void pjit_set_mem_byte (uint32_t addr, uint8_t value) {
    *(uint8_t*)((addr ^ 1) & MEMADDRMASK) = value;
}
/* Write word, address may not be word-aligned */
static inline void pjit_set_mem_byte(uint32_t addr, uint16_t value) {
	pjit_check_address(addr);
    *(uint16_t*)(addr & MEMADDRMASK) = value;
}
/* Write long, address may not be word-aligned */
static inline void pjit_set_mem_byte(uint32_t addr, uint32_t value) {
	pjit_check_address(addr);
    asm("ror %0, %0, #16" : "+r"(value));
    *(uint32_t*)(addr & MEMADDRMASK) = value;
}

static inline uint16_t pjit_get_instr(uint32_t addr) {
	pjit_check_address(addr);
    return *(uint16_t*)(0x01000000 | (addr & MEMADDRMASK));
}

static inline void pjit_set_ccr(uint8_t value) {
	sr.user_byte = value;
}

static inline void pjit_set_sr(uint16_t value) {
	// TODO very different behaviour for 68020!
	if(sr.S) ssp = cpu->a[7];
	else     usp = cpu->a[7];

	sr.status_register = value;

	if(sr.S) cpu->a[7] = ssp;
	else     cpu->a[7] = usp;
}

static inline uint8_t pjit_get_ccr(void) {
	return sr.user_byte;
}

static inline uint16_t pjit_get_sr(void) {
	return sr.status_register;
}

static inline uint8_t convert_ccr[] = {
	0x0, 0x2, 0x1, 0x3, 0x4, 0x6, 0x5, 0x7, 0x8, 0xA, 0x9, 0xB, 0xC, 0xE, 0xD, 0xF
};

static inline void pjit_save_ccr(void) {
	uint32_t ccr;
	ASM("mrs apsr, %0" : "=r"(ccr));
	sr.user_byte = convert_ccr[ccr];
	sr.X = cpu->x_flag;
}

static inline void pjit_restore_ccr(void) {
	uint32_t ccr;
	ASM("mrs apsr, %0" : "=r"(ccr));
	ASM("ror %0, %0, #24" : "+r"(ccr));
	ccr &= 0xFFFFFFF0;
	ccr |= convert_ccr(sr.user_byte & 0xF);
	ASM("msr apsr, %0" :: "r"(ccr));
}

#endif /* CPU_PJIT_H_ */
