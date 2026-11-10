/*
 * Copyright (c) 2020-2023 Renee Cousins, the Buffee Project - http://www.buffee.ca
 *
 * Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (see LICENSE).
 */

#ifndef MAIN_H_
#define MAIN_H_

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "power.h"
#include "ddr.h"
#include "flash.h"
#include "gpmc.h"
#include "init.h"
#include "pru.h"
#include "mmu.h"
#include "mem_be.h"
#include "pinmux.h"
#include "config.h"

extern void test_native_bogomips(void);
extern void test_printf(void);
extern void SetEcho(int);

extern int pmic_detected;       /* what the start-up probes found */
extern int gpak_detected;
extern int prom_detected;

extern char _image_start, _image_end;       /* from linker.lds */
extern char _stack_end, _stack_top;

extern void WriteImage(void* start, void* end);

#ifndef _DEBUG
#define printf(...)
#endif

#define _STR(s) #s
#define STR(s) _STR(s)

#endif /* MAIN_H_ */
