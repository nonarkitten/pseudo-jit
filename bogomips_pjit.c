/*
 * bogomips_pjit.c
 *
 *  Created on: Apr. 13, 2021
 *      Author: renee.cousins
 */
#include "main.h"
#include "pjit.h"
#include "am335x_dmtimer1.h"

#include "cpu/pjit.h"

uint16_t bogomips[] = {
	// 		TST.L	D0
	0x4A80,
	// 		BEQ.S	EXIT
	0x6704,
	// LOOP:
	//		SUBQ	D0, #1
	0x5380,
	//		BNE.S	LOOP
	0x66FC,
	// EXIT:
	//		RTS
	0x4E75,

	// INVALID
	0xFFFF
};

//static uint32_t cache[PJIT_CACHE_SIZE + PJIT_TAG_SIZE];
extern const uint32_t __HeapBase;
static uint32_t *cache = (uint32_t *)&__HeapBase;

static double duration = 0;
static int loops = 0;

cpu_t cpu_state;

__attribute__((noinline)) double dopjit(void) {
    double begin, end;


    do {
        loops = loops * 2;

        begin = am335x_dmtimer1_get_time();
    	cpu_start(bogomips);
        end = am335x_dmtimer1_get_time();

        duration = end - begin;

    } while(duration < 1.0);

    return 2.0*((double) loops)/(((double)duration))/1.0e6;
}

void test_pjit_bogomips(void) {
    static int pass = 0;

	cache_init(cache);
	printf("[PJIT] Cache initialized.\n");
	cpu = &cpu_state;
	cpu_dump_state();

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

