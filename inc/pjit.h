#include <arm_neon.h>

#include "makeops.h"

#ifndef __PJIT_H__
#define __PJIT_H__

typedef union { 
    uint32_t address;
    void (*invoke)(void);    
} caller_t;

typedef union {
    uint32_t ul; int32_t sl;
    uint16_t uw; int16_t sw;
    uint8_t ub;  int8_t sb;
} data_reg_t;

typedef union {
    uint32_t* ul; int32_t* sl;
    uint16_t* uw; int16_t* sw;
    uint8_t* ub;  int8_t* sb;
    signed long int raw;
} addr_reg_t;

typedef union {
    uint16_t sr;
    uint8_t ccr;
    struct {
        int t : 1; // trace
        int   : 1; // reserved
        int s : 1; // supervisor
        int   : 2; // reserved
        int i : 3; // interrupt level
        int   : 3; // reserved
        int x : 1; // extra carry flag
        int n : 1; // negative result
        int z : 1; // zero result
        int v : 1; // result overflow
        int c : 1; // carry flag
    };
} sr_ccr_t;

#if defined(__aarch64__)
// ARM64
#define DATAREG(X,Y) register data_reg_t X asm(#Y);
DATAREG(d0,w2) DATAREG(d1,w3) DATAREG(d2,w4) DATAREG(d3,w5)
DATAREG(d4,w6) DATAREG(d5,w7) DATAREG(d6,w8) DATAREG(d7,w9)


#elif defined(__arm__)
// ARM32

// r0~r1 sratch registers
// r2~r9 for a0~a7
// d0~d7 (NEON regs) for d0~d7
// r10 pc
// r11 ip
#define DATAREG(X) register data_reg_t X asm(#X);
DATAREG(d0) DATAREG(d1) DATAREG(d2) DATAREG(d3)
DATAREG(d4) DATAREG(d5) DATAREG(d6) DATAREG(d7)

#define ADDRREG(X,Y) register addr_reg_t X asm(#Y)
ADDRREG(a0,r2) ADDRREG(a1,r3) ADDRREG(a2,r4) ADDRREG(a3,r5) 
ADDRREG(a4,r6) ADDRREG(a5,r7) ADDRREG(a6,r8) ADDRREG(a7,r9) 

register uint32_t lr asm("lr");

#define SET_NZ_B(R) asm volatile("rors %0, %0, #8 @ set nz flags" :: "r"(R))
#define SET_NZ_W(R) asm volatile("rors %0, %0, #16 @ set nz flags" :: "r"(R))
#define SET_NZ_L(R) asm volatile("teq %0, #0 @ set nz flags" :: "r"(R))

register uint32_t sr_ccr asm("r11");
extern uint8_t arm_sr_ccr[16];
#define m68_get_ccr() do { uint32_t t;\
    asm volatile("mrs %0, cpsr" : "=r" (t)); \
    sr_ccr.ccr = arm_sr_ccr[t >> 28]; } while(0)

register addr_reg_t pc asm("r10");
#define m68_syncpc() pc.raw = (pc.raw & 0xFFFFF000) | ((lr >> 1) & 0xFFF)

#else
// GENERIC

#endif

#define EXP_ADD_CVNZ(TYPE,OP,BITS) \
static inline TYPE add_##TYPE(TYPE d, TYPE s) { \
    TYPE x = 0; \
    if(OP(d << BITS, s << BITS, &x)) \
        asm volatile("" ::: "memory"); \
    else \
        asm volatile("" ::: "memory"); \
    return x >> BITS; }
  
EXP_ADD_CVNZ(uint32_t,__builtin_uaddl_overflow,32)
EXP_ADD_CVNZ(uint16_t,__builtin_uaddl_overflow,16)
EXP_ADD_CVNZ(uint8_t, __builtin_uaddl_overflow,8)

EXP_ADD_CVNZ(int32_t,__builtin_saddl_overflow,32)
EXP_ADD_CVNZ(int16_t,__builtin_saddl_overflow,16)
EXP_ADD_CVNZ(int8_t, __builtin_saddl_overflow,8)

#define sub_int32_t(X,Y) add_int32_t(X,-(Y))
#define sub_int16_t(X,Y) add_int16_t(X,-(Y))
#define sub_int8_t(X,Y)  add_int8_t(X,-(Y))

extern void vec_0000(void);
extern void vec_0001(void);
extern void vec_0002(void);
extern void vec_0003(void);
extern void vec_0004(void);
extern void vec_0005(void);
extern void vec_0006(void);
extern void vec_0007(void);
extern void vec_0008(void);
extern void vec_0009(void);
extern void vec_000A(void);
extern void vec_000B(void);
extern void vec_000C(void);
extern void vec_000D(void);
extern void vec_000E(void);
extern void vec_000F(void);
extern void vec_0010(void);
extern void vec_0011(void);
extern void vec_0012(void);
extern void vec_0013(void);
extern void vec_0014(void);
extern void vec_0015(void);
extern void vec_0016(void);
extern void vec_0017(void);
extern void vec_0018(void);
extern void vec_0019(void);
extern void vec_001A(void);
extern void vec_001B(void);
extern void vec_001C(void);
extern void vec_001D(void);
extern void vec_001E(void);
extern void vec_001F(void);

extern void m68_enter(uint32_t addr);
extern void m68_start(uint32_t m68_address);
extern int32_t m68_addrext(uint16_t* m68_address);
extern void m68_exception(uint8_t);

#endif __PJIT_H__
