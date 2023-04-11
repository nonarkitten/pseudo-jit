/*
 * arm_bogomips.c
 *
 *  Created on: Apr. 13, 2021
 *      Author: renee.cousins
 */

#include "main.h"

static int loops;
static double duration;

extern uint32_t bogo_delay(uint32_t loops);

__attribute__((noinline)) double measure(void) {
    double begin, end;
    loops = 1;

    do {
        loops = loops * 2;

        begin = ReadDMTimerSeconds();
        bogo_delay(loops);
        end = ReadDMTimerSeconds();
        duration = end - begin;

    } while(duration < 1.0);

    return 2.0*((double) loops)/(((double)duration))/1.0e6;
}

void test_native_bogomips(void) {
    static int pass = 0;
    pass++;
    printf("[TEST] Performing Native BogoMIPS benchmark, warming up\n");

    measure(); // warm up
    printf("[TEST] Warm up complete, starting pass %d\n", pass);
    printf("[TEST] Native BogoMIPS %0.2f\n", measure());
    printf("[TEST] Loops: %d, duration %d ms\n", loops, (int)(duration * 1000.0));
}
