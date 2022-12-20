#ifndef __M68K_CPU_H__
#define __M68K_CPU_H__
/* Supported CPU types
 * CPU                  CPU_TYPE
 * MC68000              68000
 * MC68030              68030
 */
#define CPU_TYPE        68000

/***
 *    ___      __   ___  __   ___  ___  __
 *     |  \ / |__) |__  |  \ |__  |__  (__
 *     |   |  |    |___ |__/ |___ |     __)
 *
 */

// TODO add this to the emitter (currently, it's just emitting size)
// TODO extend this for 68020 addressing modes

#ifndef EXT_WORD_DEFINED
#define EXT_WORD_DEFINED
typedef enum {
	EXT_WORD_SRC_EXT = 0x0100,
	EXT_WORD_SRC_16B = 0x0200,
	EXT_WORD_SRC_32B = 0x0300,
	EXT_WORD_DST_EXT = 0x1000,
	EXT_WORD_DST_16B = 0x2000,
	EXT_WORD_DST_32B = 0x3000,
} ext_word_t;
#endif

typedef enum {
	ALU_OP_OR,
	ALU_OP_AND,
	ALU_OP_SUB,
	ALU_OP_ADD,
	ALU_OP_EOR,
	ALU_OP_CMP,
	ALU_OP_MOVE,
} ALU_OP_t;

#define NO_BX_LR 0x7700

/* Exception vector numbers (trap numbers). */
typedef enum {
    RESET_SP,       // 0  Initial stack pointer
    RESET_PC,       // 1  Inital program counter
    BUSERR,         // 2  Bus error
    ADDRESSERR,     // 3  Address error
    ILLINSTR,       // 4  Illegal instruction
    DIVZ,           // 5  Division by zero
    TRAPCHK,        // 6  CHK, CHK2 instruction
    TRAPV,          // 7  cpTRAPcc, TRAPcc, TRAPV instructions
    PRIV,           // 8  Privillege violation
    TRACE,          // 9  Trace emulation
    LINE_A,         // 10 Unhandled Line 1010 Emulator
    LINE_F,         // 11 Unhandled Line 1111 Emulation
    RESERVED_12,    // 12 Unassigned
    #if CPU_TYPE != 68000
    COPROPV,        // 13 Coprocessor protocol violation
    FORMATERR,      // 14 Format error
    UNINITIRQ,      // 15 Uninitialized interrupt
    #else
    RESERVED_13,    // 13 Unassigned
    RESERVED_14,    // 14 Unassigned
    RESERVED_15,    // 15 Unassigned
    #endif
    RESERVED_16,    // 16
    RESERVED_17,    // 17
    RESERVED_18,    // 18
    RESERVED_19,    // 19
    RESERVED_20,    // 20
    RESERVED_21,    // 21
    RESERVED_22,    // 22
    RESERVED_23,    // 23
    #if CPU_TYPE != 68000
    SPURIOUS,       // 24 Spurious interrupt
    #else    
    RESERVED_24,    // 24
    #endif
    AUTOINT1,       // 25 Level 1 Autovector (IPL)
    AUTOINT2,       // 26 Level 2 Autovector (IPL)
    AUTOINT3,       // 27 Level 3 Autovector (IPL)
    AUTOINT4,       // 28 Level 4 Autovector (IPL)
    AUTOINT5,       // 29 Level 5 Autovector (IPL)
    AUTOINT6,       // 30 Level 6 Autovector (IPL)
    AUTOINT7,       // 31 Level 7 Autovector (IPL)
    TRAP0,          // 32 TRAP #0 instruction
    TRAP1,          // 33 TRAP #1 instruction
    TRAP2,          // 34 TRAP #2 instruction
    TRAP3,          // 35 TRAP #3 instruction
    TRAP4,          // 36 TRAP #4 instruction
    TRAP5,          // 37 TRAP #5 instruction
    TRAP6,          // 38 TRAP #6 instruction
    TRAP7,          // 39 TRAP #7 instruction
    TRAP8,          // 40 TRAP #8 instruction
    TRAP9,          // 41 TRAP #9 instruction
    TRAP10,         // 42 TRAP #10 instruction
    TRAP11,         // 43 TRAP #11 instruction
    TRAP12,         // 44 TRAP #12 instruction
    TRAP13,         // 45 TRAP #13 instruction
    TRAP14,         // 46 TRAP #14 instruction
    TRAP15,         // 47 TRAP #15 instruction
    #if CPU_TYPE != 68000
    FPUNORDERED,    // 48 FPCP Branch or Set on Unordered
    FPINEXACT,      // 49 FPCP Inexact result
    FPDIVZ,         // 50 FPCP Division by zero
    FPUNDERFLOW,    // 51 FPCP Underflow
    FPOPERROR,      // 52 FPCP Operand error
    FPOVERFLOW,     // 53 FPCP Overflow
    FPSNAN,         // 54 FPCP Signalling NAN
    RESERVED_55,    // 55 Unassigned
    PMMUCONFIG,     // 56 PMMU Configuration
    PMMUILLEGALOP,  // 57 PMMU Illegal operation
    PMMUACCESSV,    // 58 PMMU Access level violation
    #endif
} vector_t;

typedef struct {
	union {
		uint32_t reg[32];
		struct {
			// offsets 0 thru 7
			union {
				uint32_t d[8];
				struct {
					uint32_t    d0;
					uint32_t    d1;
					uint32_t    d2;
					uint32_t    d3;
					uint32_t    d4;
					uint32_t    d5;
					uint32_t    d6;
					uint32_t    d7;
				};
			};
			// offsets 8 thru 15
			union {
				uint32_t a[8];
				struct {
					uint32_t    a0;
					uint32_t    a1;
					uint32_t    a2;
					uint32_t    a3;
					uint32_t    a4;
					uint32_t    a5;
					uint32_t    a6;
					uint32_t    a7;
				};
			};
			uint32_t sfc;    // 16 Source Function Code
			uint32_t usp;    // 17 User Stack Pointer
			uint32_t dfc;    // 18 Destination Function Code
			uint32_t vbr;    // 19 Vector Base Register
			uint32_t cacr;   // 20 Cache Control Register
			uint32_t caar;   // 21 Cache Address Register
			uint32_t tc;     // 22 MMU Translation Control Register
			uint32_t msp;    // 23 Master Stack Pointer
			uint32_t itt0;   // 24 Instruction Transparent Translation Register 0
			uint32_t isp;    // 25 Interrupt Stack Pointer
			uint32_t itt1;   // 26 Instruction Transparent Translation Register 1
			uint32_t mmusr;  // 27 MMU Status Register
			uint32_t dtt0;   // 28 Data Transparent Translation Register 0
			uint32_t urp;    // 29 User Root Pointer
			uint32_t dtt1;   // 30 Data Transparent Translation Register 1
			uint32_t srp;    // 31 Supervisor Root Pointer	
		};
	};
		
	// Status register
	union {
		uint16_t sr;
		struct {
			uint8_t user_byte;
			uint8_t system_byte;
		};
		struct {
			uint16_t C : 1;
			uint16_t V : 1;
			uint16_t Z : 1;
			uint16_t N : 1;
			uint16_t X : 1;
			uint16_t   : 3;
			uint16_t I : 3;			
			uint16_t   : 2;
			uint16_t S : 1;
			uint16_t   : 1;
			uint16_t T : 1;
		};
	};
	// Padding for 32-bit alignment
	uint16_t pad1;
	
	// backup for ARM
	uint32_t cpsr;
	uint32_t lr;

	// Program counter
	uint32_t pc;

	// Program counter
	uint32_t x;

	// Conversion tables for 68K to ARM condition codes
	uint32_t cc2arm[16];
	uint8_t arm2cc[16];

	// Base address cache
	void        *base_ptr;

	// The M68K address of the current cache page
	uint32_t 	m68k_page;
		
#ifdef MC68881
	/* 68881 regs: */
	double      fp0;
	double      fp1;
	double      fp2;
	double      fp3;
	double      fp4;
	double      fp5;
	double      fp6;
	double      fp7;
	uint32_t	fpsr;
	uint16_t	fpcr;
	// Ensure whole thing is multiple of 8 bytes
	uint16_t    padding;
#endif
} cpu_t;

typedef union {
    uint32_t 	word;     //  Type used for word access. 
    struct {
        uint32_t   M:5;   // bit: 0.. 4 Mode field  
        uint32_t   T:1;   // bit: 5 Thumb execution state bit  
        uint32_t   F:1;   // bit: 6 FIQ mask bit  
        uint32_t   I:1;   // bit: 7 IRQ mask bit 
        uint32_t   A:1;   // bit: 8 Asynchronous abort mask bit 
        uint32_t   E:1;   // bit: 9 Endianness execution state bit 
        uint32_t   IT1:6; // bit: 10..15 If-Then execution state bits 2-7  
        uint32_t   GE:4;  // bit: 16..19 Greater than or Equal flags 
        uint32_t   J:1;   // bit: 24 Jazelle bit  
        uint32_t   IT0:2; // bit: 25..26 If-Then execution state bits 0-1 
        uint32_t   Q:1;   // bit: 27 Saturation condition flag  
        uint32_t   V:1;   // bit: 28 Overflow condition code flag 
        uint32_t   C:1;   // bit: 29 Carry condition code flag 
        uint32_t   Z:1;   // bit: 30 Zero condition code flag 
        uint32_t   N:1;   // bit: 31 Negative condition code flag 
    };
} cpsr_t;

#endif
