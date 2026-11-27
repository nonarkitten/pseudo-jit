
#ifndef __HW_GPMC_H__
#define __HW_GPMC_H__

#include "main.h"

extern void InitGPMC(float);
extern void EnableRomOverlay(void);         /* CIA-A OVL: the ROM at $000000 (as after a reset) */
extern void DisableRomOverlay(void);        /* chip RAM at $000000 */
extern int GPMCUnits(double bus_khz);       /* the GPMC units that stand for 4 bus clocks */

#endif