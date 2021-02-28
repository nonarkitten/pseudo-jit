#pragma once
#ifndef AM335X_CLOCK_H
#define AM335X_CLOCK_H
/**
 * Copyright 2015 University of Applied Sciences Western Switzerland / Fribourg
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
 * Abstract: AM335x Clocking Driver
 *
 * Purpose: This module implements basic services to drive the AM335x
 *          clocking module.
 *
 * Author:  Daniel Gachet
 * Date:    28.11.2016
 */

/** 
 * am335x uart clock modules
 */
enum am335x_clock_uart_modules {
    AM335X_CLOCK_UART0,
    AM335X_CLOCK_UART1,
    AM335X_CLOCK_UART2,
    AM335X_CLOCK_UART3,
    AM335X_CLOCK_UART4,
    AM335X_CLOCK_UART5,
};

/**
 * am335x gpio clock modules
 */
enum am335x_clock_gpio_modules {
    AM335X_CLOCK_GPIO0,
    AM335X_CLOCK_GPIO1,
    AM335X_CLOCK_GPIO2,
    AM335X_CLOCK_GPIO3,
};

/**
 * am335x i2c clock modules
 */
enum am335x_clock_i2c_modules {
    AM335X_CLOCK_I2C0,
    AM335X_CLOCK_I2C1,
    AM335X_CLOCK_I2C2,
};

/**
 * am335x i2c clock modules
 */
enum am335x_clock_spi_modules {
    AM335X_CLOCK_SPI0,
    AM335X_CLOCK_SPI1,
};

/**
 * am335x timer clock modules
 */
enum am335x_clock_timer_modules {
    AM335X_CLOCK_TIMER0,
    AM335X_CLOCK_TIMER1,
    AM335X_CLOCK_TIMER2,
    AM335X_CLOCK_TIMER3,
    AM335X_CLOCK_TIMER4,
    AM335X_CLOCK_TIMER5,
    AM335X_CLOCK_TIMER6,
    AM335X_CLOCK_TIMER7
};

/**
 * am335x mmc clock modules
 */
enum am335x_clock_mmc_modules {
    AM335X_CLOCK_MMC0,
    AM335X_CLOCK_MMC1,
    AM335X_CLOCK_MMC2
};

/**
 * am335x epwm clock modules
 */
enum am335x_clock_epwm_modules {
    AM335X_CLOCK_EPWM0,
    AM335X_CLOCK_EPWM1,
    AM335X_CLOCK_EPWM2
};

/**
 * method to enable the system L3 and system L4_WKUP clocks.
 */
extern void am335x_clock_enable_l3_l4wkup(void);

/**
 * method to enable the clock instance for a specific uart.
 */
extern void am335x_clock_enable_uart_module(
    enum am335x_clock_uart_modules module);

/**
 * method to enable the clock instance for a specific gpio module.
 */
extern void am335x_clock_enable_gpio_module(
    enum am335x_clock_gpio_modules module);

/**
 * method to enable the clock instance for a specific i2c module.
 */
extern void am335x_clock_enable_i2c_module(
    enum am335x_clock_i2c_modules module);

/**
 * method to enable the clock instance for a specific spi module.
 */
extern void am335x_clock_enable_spi_module(
    enum am335x_clock_spi_modules module);

/**
 * method to enable the clock instance for a specific timer module.
 */
extern void am335x_clock_enable_timer_module(
    enum am335x_clock_timer_modules module);

/**
 * method to enable the clock instance for a specific mmc module.
 */
extern void am335x_clock_enable_mmc_module(
    enum am335x_clock_mmc_modules module);

/**
 * method to enable the clock instance for a specific epwm module.
 */
extern void am335x_clock_enable_epwm_module(
    enum am335x_clock_epwm_modules module);

/**
 * method to enable the clock instance for EDMA
 */
extern void am335x_clock_enable_edma_module();

/**
 * method to enable the clock instance for CPSW (Ethernet) module
 */
extern void am335x_clock_enable_cpsw_module();

#endif
