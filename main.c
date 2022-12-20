#include "main.h"

__attribute__((naked)) void pjit_entry(void) {
	asm __volatile(
	"entry:                             \n"		
	"   mov r0, r0                      \n"		
	"   mov r0, #0                      \n"
	"   ldr r1, =_bss_start             \n"
	"   ldr r2, =_bss_end               \n"
	"1: cmp r1, r2                      \n"
	"   strlo   r0, [r1], #4            \n"
	"   blo     1b                      \n"
	);
	cpu_start(0);
	while(1); 
}

