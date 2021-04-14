/*
 * bogomips_pjit.c
 *
 *  Created on: Apr. 13, 2021
 *      Author: renee.cousins
 */
#include "main.h"
#include "pjit.h"
#include "am335x_dmtimer1.h"

#define D0 __d0
#define D1 __d1
#define D2 cpu->d[2]
#define D3 cpu->d[3]
#define D4 cpu->d[4]
#define D5 cpu->d[5]
#define D6 cpu->d[6]
#define D7 cpu->d[7]

#define A0 __a0
#define A1 __a1
#define A2 cpu->a[2]
#define A3 cpu->a[3]
#define A4 __a4
#define A5 __a5
#define A6 __a6
#define A7 __a7

static cpu_t _cpu = { 0 };
static int loops;
static double duration;

/* Run BogoMIPS loop without having loaded the whole pjit table
** This does the best job of imitating what PJIT ought to emit
*/
static inline void tst_d0_l(void) {
    uint32_t t0 = D0;
    ASM("rors\t%0, %1, %2" : "=r"(t0) : "r"(t0), "n"(0));
}

static inline void subq_l_1_d0(void) {
    uint32_t t0 = D0;
    ASM("subs\t%0, %1, %2" : "=r"(t0) : "r"(t0), "n"(0x01));
    D0 = t0;
}

__attribute__((noinline)) void pjit_bogomips(int loops) {
    cpu = &_cpu;
    D0 = loops;
    tst_d0_l();
    ASM_GOTO("beq\t%l0" :::: exit);
loop:
    subq_l_1_d0();
    ASM_GOTO("bne\t%l0" :::: loop);
exit:
    ASM("");
}


__attribute__((noinline)) double dopjit(void) {
    double begin, end;
    loops = 1;

    do {
        loops = loops * 2;

        begin = am335x_dmtimer1_get_time();
        pjit_bogomips(loops);
        end = am335x_dmtimer1_get_time();
        duration = end - begin;

    } while(duration < 1.0);

    return 2.0*((double) loops)/(((double)duration))/1.0e6;
}

void test_pjit_bogomips(void) {
    static int pass = 0;
    pass++;
    printf("[PJIT] Performing PJIT BogoMIPS benchmark, warming up\n");
    dopjit();
    printf("[PJIT] Warm up complete, starting pass %d\n", pass);
    double bogomips = dopjit();

    volatile int bmWhole = (int)bogomips;
    volatile int bmDeci = '0' + ((int)(bogomips * 10.0) % 10);
    volatile int bmCenti = '0' + ((int)(bogomips * 100.0) % 10);

    printf("[PJIT] PJIT BogoMIPS %d.%c%c\n", bmWhole, bmDeci, bmCenti);
    printf("[PJIT] loops: %d, duration %d ms\n", loops, (int)(duration * 1000.0));
}

