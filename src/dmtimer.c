/**
 * Copyright 2019 University of Applied Sciences Western Switzerland / Fribourg
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Project: HEIA-FR†/ Embedded Systems 1+2 Laboratory
 *
 * Abstract: AM335x DTTimer1
 *
 * Purpose: This module implements basic services to drive the AM335x DMTimer1
 *
 * Author:  Daniel Gachet
 * Date:    23.03.2019
 */

#include "clock.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "timer.h" 

#define printf(...)
 
/* -- Internal types and constant definition -------------------------------- */

// DMTimer TIOCP_CFG register bit definition
#define TIOCP_CFG_SOFTRESET (1 << 1)

// DMTimer TISTAT register bit definition
#define TISTAT_RESETDONE (1 << 0)

// DMTimer TCLR register bit definition
#define TCLR_ST (1 << 0)
#define TCLR_AR (1 << 1)

// DMTimer input clock frequency
#define FREQUENCY 24000000

/**
 * DMTimer1 Register Definition
 */   
struct timer1_ctrl {
    uint32_t tidr;       // 00
    uint32_t res1[3];    // 04-0c
    uint32_t tiocp_cfg;  // 10
    uint32_t tistat;     // 14
    uint32_t tisr;       // 18
    uint32_t tier;       //†1c
    uint32_t twer;       // 20
    uint32_t tclr;       // 24
    uint32_t tcrr;       // 28
    uint32_t tldr;       // 2c
    uint32_t ttgr;       // 30
    uint32_t twps;       //†34
    uint32_t tmar;       // 38
    uint32_t tcar1;      //†3c
    uint32_t tsicr;      //†40
    uint32_t tcar2;      // 44
    uint32_t tpir;       // 48
    uint32_t tnir;       // 4c
    uint32_t tcvr;       // 50
    uint32_t tocr;       // 54
    uint32_t towr;       // 5c
};

/**
 * DMTimer1 registers
 */
static volatile struct timer1_ctrl* timer1 =
    (volatile struct timer1_ctrl*)0x44e31000;

// -- Public methods definition -----------------------------------------------

void am335x_dmtimer1_init()
{
    static bool is_initialized = false;
    printf("...Entering %s\n", __func__);
    printf("...timer1 @ 0x%p\n", (uint32_t)timer1);
    if (!is_initialized) {
        printf("...Configure DMTimer1 for internal use (resolution 24MHz)\n");
        am335x_clock_enable_timer_module(AM335X_CLOCK_TIMER1);

        printf("...Forcing timer reset\n");
        timer1->tiocp_cfg = __builtin_bswap32(TIOCP_CFG_SOFTRESET);
        
        printf("...Waiting for software reset to complete\n");
        while ((timer1->tistat & __builtin_bswap32(TISTAT_RESETDONE)) == 0)
            continue;
        
        printf("...Completing initialization\n");
        timer1->tldr = 0;
        timer1->tcrr = 0;
        timer1->ttgr = 0;
        timer1->tclr = __builtin_bswap32(TCLR_AR | TCLR_ST);

        is_initialized = true;
    }
    printf("...Exiting %s\n", __func__);
}

// ----------------------------------------------------------------------------

uint32_t am335x_dmtimer1_get_counter() {
    return __builtin_bswap32(timer1->tcrr); 
}

// ----------------------------------------------------------------------------

uint32_t am335x_dmtimer1_get_frequency() { return FREQUENCY; }

// ----------------------------------------------------------------------------

void am335x_dmtimer1_wait_us(uint32_t us)
{
    uint32_t clocks = us * (FREQUENCY / 1000000) + 1;
    uint32_t st     = __builtin_bswap32(timer1->tcrr);
    uint32_t sp     = st;
    while (((sp - st) < clocks)) {
        sp = __builtin_bswap32(timer1->tcrr);
    }
}

// ----------------------------------------------------------------------------

void am335x_dmtimer1_wait_ms(uint32_t ms)
{
    uint32_t clocks = ms * (FREQUENCY / 1000) + 1;
    uint32_t st     = __builtin_bswap32(timer1->tcrr);
    uint32_t sp     = st;
    while (((sp - st) < clocks)) {
        sp = __builtin_bswap32(timer1->tcrr);
    }
}

// ----------------------------------------------------------------------------

uint64_t am335x_dmtimer1_get_uptime()
{
    static uint32_t st     = 0;
    static uint64_t uptime = 0;
    uint32_t sp            = __builtin_bswap32(timer1->tcrr);
    uptime += sp - st;
    st = sp;
    return uptime;
}