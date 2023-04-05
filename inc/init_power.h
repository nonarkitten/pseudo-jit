#ifndef __INIT_POWER_H__
#define __INIT_POWER_H__

#include "main.h"

typedef enum {
    RAIL_DCDC1,
    RAIL_DCDC2,
    RAIL_DCDC3,
} PowerRail_t;

extern void InitPower(PowerRail_t rail, double voltage);

#endif
