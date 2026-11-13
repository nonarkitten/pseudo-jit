#ifndef __INIT_POWER_H__
#define __INIT_POWER_H__

#include "main.h"

typedef enum {
    RAIL_DCDC1,
    RAIL_DCDC2,
    RAIL_DCDC3,
} PowerRail_t;

extern int InitPower(PowerRail_t rail, double voltage);
extern int DetectPMIC(void);        /* the PMIC answers and the last boot was good: the core voltage and settings.dpll_mul MHz are set (1); 300 MHz otherwise (0) */

#endif
