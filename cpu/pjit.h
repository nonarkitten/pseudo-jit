#include <setjmp.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#ifndef __PJIT_H__
#define __PJIT_H__

#include "m68k_cpu.h"

register uint32_t D0 asm("r4");
register uint32_t D1 asm("r5");
register uint32_t D2 asm("r6");
register uint32_t D3 asm("r7");
#define           D4 cpu->d[4]
#define           D5 cpu->d[5]
#define           D6 cpu->d[6]
#define           D7 cpu->d[7]

register uint32_t A0 asm("r8");
register uint32_t A1 asm("r9");
register uint32_t A2 asm("r10");
#define           A3 cpu->a[3]
#define           A4 cpu->a[4]
#define           A5 cpu->a[5]
#define           A6 cpu->a[6]
register uint32_t A7 asm("r11");

register cpu_t* cpu asm("r12");

register uint32_t lr asm("r14");

#include "pjit_cache.h"
#include "pjit_extword.h"
#include "pjit_ops.h"

extern uint16_t oplen[65536];
extern uint32_t optab[65536];


#define dmb(option) __asm__ __volatile__ ("dmb\t" #option : : : "memory")
#define dsb(option) __asm__ __volatile__ ("dsb\t" #option : : : "memory")
#define isb(option) __asm__ __volatile__ ("isb\t" #option : : : "memory")

#endif

