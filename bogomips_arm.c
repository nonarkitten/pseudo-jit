/*
 * arm_bogomips.c
 *
 *  Created on: Apr. 13, 2021
 *      Author: renee.cousins
 */

#include "main.h"
#include "am335x_dmtimer1.h"

static int loops;
static double duration;

extern uint32_t bogo_delay(uint32_t loops);

__attribute__((noinline)) double measure(void) {
    double begin, end;
    loops = 1;

    do {
        loops = loops * 2;

        begin = am335x_dmtimer1_get_time();
        bogo_delay(loops);
        end = am335x_dmtimer1_get_time();
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
    double bogomips = measure();

    volatile int bmWhole = (int)bogomips;
    volatile int bmDeci = '0' + ((int)(bogomips * 10.0) % 10);
    volatile int bmCenti = '0' + ((int)(bogomips * 100.0) % 10);

    printf("[TEST] Native BogoMIPS %d.%c%c\n", bmWhole, bmDeci, bmCenti);
    printf("[TEST] Loops: %d, duration %d ms\n", loops, (int)(duration * 1000.0));
}


