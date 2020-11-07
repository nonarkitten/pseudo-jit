#include <arm_neon.h>

#include "makeops.h"

#ifndef __PJIT_H__
#define __PJIT_H__

typedef union { 
    uint32_t address;
    void (*invoke)(void);    
} caller_t;


#ifdef __arm__
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

static inline uint32_t add_uint32_t(uint32_t d, uint32_t s) {
    uint32_t x = 0;
    if(__builtin_uaddl_overflow(d, s, &x))
        asm volatile("" ::: "memory");
    else
        asm volatile("" ::: "memory");
    return x;
}

static inline uint16_t add_uint16_t(uint16_t d, uint16_t s) {
    uint32_t x = 0;
    if(__builtin_uaddl_overflow(d << 16, s << 16, &x))
        asm volatile("" ::: "memory");
    else
        asm volatile("" ::: "memory");
    return (x >> 16);
}

static inline uint8_t add_uint8_t(uint8_t d, uint8_t s) {
    uint32_t x = 0;
    if(__builtin_uaddl_overflow(d << 24, s << 24, &x))
        asm volatile("" ::: "memory");
    else
        asm volatile("" ::: "memory");
    return (x >> 24);
}

#define sub_uint32_t(X,Y) add_uint32_t(X,-(Y))
#define sub_uint16_t(X,Y) add_uint16_t(X,-(Y))
#define sub_uint8_t(X,Y) add_uint8_t(X,-(Y))

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

#else
/*

Generic Accessor Form
D(N,W)  N=0..7, W=BWL
A(N,W)  N=0..7, W= WL
PC, PC_BPTR, PC_WPTR, PC_LPTR
SR

*/
typedef struct { 
    uint32_t L; 
    uint16_t W; 
    uint8_t B; 
    uint32_t* L_PTR; 
    uint16_t* W_PTR; 
    uint8_t* B_PTR; 
} reg_t;

typedef struct {
    int x : 1;
    int v : 1;
    int c : 1;
    int z : 1;
    int n : 1;
} sr_ccr_t;

inline sr_ccr_t sr_ccr;
#define FLAG_X sr_ccr.z
#define FLAG_V sr_ccr.v
#define FLAG_C sr_ccr.c
#define FLAG_Z sr_ccr.z
#define FLAG_N sr_ccr.n

#define D(N,W) (__regs[N].W)
#define A(N,W) (__regs[(N)+8].W)
#define APTR(N,W) __regs[(N)+8].W##_PTR
#define PC_BPTR __regs[16].B_PTR
#define PC_WPTR __regs[16].W_PTR
#define PC_LPTR __regs[16].L_PTR
#define PC      __regs[16].L

inline reg_t __regs[17] = { 0 };
inline uint32_t SR;

#endif


extern void m68_enter(uint32_t addr);
extern void m68_start(uint32_t m68_address);
extern int32_t m68_addrext(uint16_t* m68_address);
extern void m68_exception(uint8_t);

#endif __PJIT_H__
