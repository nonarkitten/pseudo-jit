#include <setjmp.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#ifndef __PJIT_H__
#define __PJIT_H__

#include "ops/m68k_common.h"
#include "ops/m68k_cpu.h"
#include "ops/m68k_emit_ea.h"
#include "ops/m68k_emitters.h"
#include "ops/m68k_registers.h"
#include "ops/pjit_extword.h"

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

register cpu_t* cpu asm("r5");

// must be <= 32MB minus overhead
#define PAGE_SIZE       10		/* 2 ^ 10 = 1KB PJIT PAGE SIZE */
#define PAGE_COUNT		10		/* 2 ^ 10 * 1KB = 1MB PJIT CACHE*/
#define SET_BITS        0       /* 2 ^ 0 = Number of associated sets */

extern void cpu_start(void* base);

#endif

