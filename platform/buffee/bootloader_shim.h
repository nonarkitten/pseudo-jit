/* bootloader_shim.h -- force-included (-include) when the bootloader's flash, EEPROM and settings code (bootloader/src/store.c, settings_set.c, bootloader/hal/config.c,
 * flash.c, i2c0.c, dmtimer.c) is built into the payload (mk/board.mk) or into the host test of the control page (tests/amiga): what that code prints goes into the
 * request's reply (control.c), and in the payload its peripheral registers are where the payload maps them (BUFFEE_IO), not at their physical addresses.
 *
 * Copyright (c) 2026 the Buffee Project - http://www.buffee.ca
 * Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (see LICENSE).
 */
#ifndef BOOTLOADER_SHIM_H
#define BOOTLOADER_SHIM_H
#include <stdio.h>
int ctl_printf(const char *fmt, ...);
int ctl_putchar(int c);
#define printf  ctl_printf
#define info    ctl_printf
#define putchar ctl_putchar
#ifndef _DEBUG
#define _DEBUG                          /* (bootloader/inc/main.h drops every printf without it) */
#endif
#ifdef BUFFEE_PAYLOAD
#include "buffee.h"
#define AM335X_VA(pa) BUFFEE_IO(pa)
#endif
#endif
