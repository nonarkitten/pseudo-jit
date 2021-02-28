#pragma once
#ifndef AM335X_DMTIMER1_H
#define AM335X_DMTIMER1_H
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

#include <stdint.h>
 
/**
 * method to initialize the AM335x Timer resources.
 * should be called prior any other methods.
 */
extern void am335x_dmtimer1_init();

/**
 * method to get the current AM335x DMTimer1 counter value in tick
 *
 * @return counter value in ticks
 */
extern uint32_t am335x_dmtimer1_get_counter();

/**
 * method to get the frequency of the timer source clock
 * @return timer clock frequency in Hz
 */
extern uint32_t am335x_dmtimer1_get_frequency();

/**
 * method to wait specified amount of microseconds
 * waiting timing is done in busywait mode
 * @param us number of microseconds to wait
 */
extern void am335x_dmtimer1_wait_us(uint32_t us);

/**
 * method to wait specified amount of milliseconds
 * waiting timing is done in busywait mode
 * @param ms number of milliseconds to wait
 */
extern void am335x_dmtimer1_wait_ms(uint32_t ms);

/**
 * method to get uptime, this method must be called a
 * little quickly that every 3 minutes
 * @return current uptime in ticks
 */
extern uint64_t am335x_dmtimer1_get_uptime();

#endif
