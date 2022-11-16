#include "main.h"

static uint16_t bogomips_ROM[] = {
	// Place holder for entry
	0x0000, 0x0000,
	// Place holder for stack pointer
	0x0000, 0x0000,
	// 		TST.L	D0
	0x4A80,
	// 		BEQ.S	EXIT
	0x6704,
	// LOOP:
	//		SUBQ	D0, #1
	0x5380,
	//		BNE.S	LOOP
	0x66FC,
	// EXIT:
	//		RTS
	0x4E75,
	// INVALID
	0xFFFF
};

__attribute__((naked)) void main(void) {
    asm volatile (                                                                      
    "           mov r0, #0                      \n"
    "           ldr r1, =_bss_start             \n"
    "           ldr r2, =_bss_end               \n"
    "1:         cmp r1, r2                      \n"
    "           strlo   r0, [r1], #4            \n"
    "           blo     1b                      \n"
    );
	
	uint32_t pc = (uint32_t)&bogomips_ROM[4];
	bogomips_ROM[0] = pc >> 16;
	bogomips_ROM[1] = pc & 0xFFFF;
	cpu_start(bogomips_ROM);
	while(1);
}

