#ifndef __GPAK_H__
#define __GPAK_H__

#include <stdint.h>

extern int  DetectGP(void);             /* finds the GreenPAK on I2C0 and writes the bus-logic design into it */
extern void GPEnableASn(void);          /* lets the GreenPAK drive ASn */
extern void GPReloadImage(void);        /* writes the design again (as at boot) */
extern void GPUseImage(int noswap);     /* writes the design for a board with (0) or without (1) the byte swapper */
extern int  GPPowerTest(void);          /* 1: the board powers the GreenPAK (ball M1, a byte-swapper board), 0: it does not (straight), -1: cannot tell */
extern void GPSetPhase(uint8_t table);  /* selects the parity gate's truth table */
extern void ASNCalibrate(void);         /* hal/gpmc.c: chooses the table that puts ASn on the right clock phase */

#endif
