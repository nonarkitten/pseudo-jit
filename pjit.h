#include <arm_neon.h>

#include "makeops.h"

#ifndef __PJIT_H__
#define __PJIT_H__

// r0~r12, r14 are available for abuse
// r0~r1 sratch registers
// r2~r9 for a0~a7
// r10 pc
// r11 ip

// 68000 STATE
#define DATAREG(X) \
register uint32x2_t X##_l asm(#X); \
register uint16x4_t X##_w asm(#X); \
register uint8x8_t X##_b asm(#X);
DATAREG(d0) DATAREG(d1) DATAREG(d2) DATAREG(d3)
DATAREG(d4) DATAREG(d5) DATAREG(d6) DATAREG(d7)

#define FPUREG(X,Y) register double fp##X asm(#Y);
FPUREG(0,d8)  FPUREG(1,d9)  FPUREG(2,d10) FPUREG(3,d11)
FPUREG(4,d12) FPUREG(5,d13) FPUREG(6,d14) FPUREG(7,d15)

#define ADDRREG(X,Y) \
register uint16_t X##_w asm(#Y); \
register uint32_t X##_l asm(#Y); \
register uint8_t* X##_bp asm(#Y); \
register uint16_t* X##_wp asm(#Y); \
register uint32_t* X##_lp asm(#Y);
ADDRREG(a0,r2) ADDRREG(a1,r3) ADDRREG(a2,r4) ADDRREG(a3,r5) 
ADDRREG(a4,r6) ADDRREG(a5,r7) ADDRREG(a6,r8) ADDRREG(a7,r9) 

#define SET_NZ_B(R) asm volatile("rors %0, %0, #8 @ set nz flags" :: "r"(R))
#define SET_NZ_W(R) asm volatile("rors %0, %0, #16 @ set nz flags" :: "r"(R))
#define SET_NZ_L(R) asm volatile("teq %0, #0 @ set nz flags" :: "r"(R))

register uint8_t*  pc_bp asm("r10"); 
register uint16_t* pc_wp asm("r10"); 
register uint32_t* pc_lp asm("r10");
register uint32_t  pc_l  asm("r10");

register uint32_t  sr_ccr  asm("r11");

register uint32_t lr asm("lr");

extern uint8_t arm_sr_ccr[16];

#define m68_syncpc() pc_l = (pc_l & 0xFFFFF000) | ((lr >> 1) & 0xFFF)
#define m68_get_ccr() do { \
    asm volatile("mrs r0, cpsr"); \
    sr_ccr = (sr_ccr & 0xFF00) | arm_sr_ccr[r0 >> 28]; \
} while(0)

extern void m68_enter(uint32_t addr);
extern void m68_start(uint32_t m68_address);
extern int32_t m68_addrext(void);
extern void m68_exception(uint8_t);

#endif __PJIT_H__
