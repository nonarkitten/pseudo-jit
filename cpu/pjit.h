#include <setjmp.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#ifndef __PJIT_H__
#define __PJIT_H__

#include "ops/m68k_cpu.h"
#include "ops/m68k_registers.h"
#include "ops/pjit_extword.h"

#define           D2 cpu->d[2]
#define           D3 cpu->d[3]
#define           D4 cpu->d[4]
#define           D5 cpu->d[5]
#define           D6 cpu->d[6]
#define           D7 cpu->d[7]

// PJIT Better Register Allocation
// --ARM-- 68K USE
// ------- --- ------
// R0      T0  Src Ext Addr, temp
// R1      T1  Dest Ext Addr, temp
// R2      T2  Temp

// R3      D0  Data registers
// R4      D1   "

// R5      CPU CPU state pointer

// R6      A0  Address registers
// R7      A1   "
// R8      A2   "
// R9      A3   "
// R10     A4   "
// R11     A5   "
// R12     A6   "

// R13 SP  SP* Stack Pointer
// R14 LR  N/A Link Register
// R15 PC  N/A ARM Program Counter

// pedantic doesn't like global register allocation
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
register uint32_t D0 asm("r3");
register uint32_t D1 asm("r4");

register cpu_t* cpu asm("r5");

register uint32_t A0 asm("r6");
register uint32_t A1 asm("r7");
register uint32_t A2 asm("r8");
register uint32_t A3 asm("r9");
register uint32_t A4 asm("r10");
register uint32_t A5 asm("r11");
register uint32_t A6 asm("r12");
register uint32_t A7 asm("r13");

register uint32_t lr asm("r14");
#pragma GCC diagnostic pop

#include "pjit_cache.h"
#include "pjit_ops.h"

extern uint16_t oplen[65536];
extern uint32_t optab[65536];


#define dmb(option) __asm__ __volatile__ ("dmb\t" #option : : : "memory")
#define dsb(option) __asm__ __volatile__ ("dsb\t" #option : : : "memory")
#define isb(option) __asm__ __volatile__ ("isb\t" #option : : : "memory")

#endif

