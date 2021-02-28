#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include "timer.h"
#include "clock.h"

#ifndef __MAIN_H__
#define __MAIN_H__
 
extern void init_led_output(void);
extern void heartbeat_forever(void);
extern void disable_watchdog(void);


#endif