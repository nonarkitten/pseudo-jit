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

extern void SetEcho(int);
extern bool console_verbose;    /* info() prints (clib/printf.c) */
extern bool printTimeStamp;     /* every line starts with the time since power-on */

/* What the start-up found (main.c), for the boot report and the board summary of the menu. */
typedef struct {
    int pmic;                   /* the PMIC answered and the ARM runs at settings.dpll_mul MHz (300 MHz otherwise) */
    int greenpak;
    int bus;                    /* the Amiga's bus runs: there is a GreenPAK, and the host let RESET and HALT go */
    int eeprom;                 /* CFG_* (config.h) */
    double bus_mhz;             /* the bus clock the GPMC timing was made for */
    int post;                   /* POST_* (post.h) */
} boot_t;
extern boot_t boot;
extern const char* Version;

extern char _image_start, _image_end;       /* from linker.lds */

extern uint32_t LoadXMODEM(uint32_t start);                 /* xmodem.c: the bytes received (a multiple of 128), 0 on failure */
extern int UpdateBootloader(const void* image, uint32_t len);   /* writeimage.c: 0 when written and verified */
extern void Menu(void);                                     /* menu.c */
extern void SettingsMenu(void);                             /* settings.c */

#ifndef _DEBUG
#define printf(...)
#endif

#define _STR(s) #s
#define STR(s) _STR(s)

#endif /* MAIN_H_ */
