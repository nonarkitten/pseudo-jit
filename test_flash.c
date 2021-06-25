/*
 * test_flash.c
 *
 *  Created on: Apr. 17, 2021
 *      Author: renee.cousins
 */

#include <stdbool.h>
#include <stdio.h>

#include "am335x_dmtimer1.h"
#include "am335x_gpio.h"
#include "am335x_spi.h"

#define PRCM_BASE            (0x44e00000)
#define CM_PER_SPI0_CLKCTRL  (PRCM_BASE + 0x4c)
#define CM_PER_SPI1_CLKCTRL  (PRCM_BASE + 0x50)
#define CM_PER_I2C0_CLKCTRL (PRCM_BASE + 0x4B8)
#define PRCM_MOD_EN          (0x2)

#define CFG_MOD_BASE       (0x44E10000)      //REVISIT
#define SPI0_SCLK_OFF      (0x950)
#define SPI0_D0_OFF        (0x954)
#define SPI0_D1_OFF        (0x958)
#define SPI0_CS0_OFF       (0x95c)
#define SPI1_SCLK_OFF      (0x990)
#define SPI1_D0_OFF        (0x994)
#define SPI1_D1_OFF        (0x998)
#define SPI1_CS0_OFF       (0x99c)
#define I2C0_SD0_OFF       (0x988)
#define I2C0_SCL_OFF       (0x98c)

#define MODE(val)        (val << 0)

#define PULL_UD_DISABLE  (1 << 3) /* PULL UP/DOWN disabled */
#define PULL_TYPESEL     (1 << 4) /* PULL UP Select */
#define RXACTIVE         (1 << 5)
#define SLEW_CTRL        (1 << 7)
#define PULL_UP_EN       (PULL_TYPESEL) /* PUL UP Select, PULL_UD_EN = 0 */

static struct {
	int freq;
	enum am335x_spi_controllers ctrl;
	enum am335x_spi_channels chan;
} flash_spi = {
		12000000,
		AM335X_SPI0,
		AM335X_CHAN0,
};

struct pin_muxing {
	uint32_t offset;
	uint32_t val;
} SPI0_flash_mux[] = {
	{ SPI0_SCLK_OFF, MODE(0) | 				RXACTIVE },
	{ SPI0_D0_OFF,   MODE(0) | PULL_UP_EN | RXACTIVE },
	{ SPI0_D1_OFF,   MODE(0) | 				RXACTIVE },
	{ SPI0_CS0_OFF,  MODE(0) | PULL_UP_EN | RXACTIVE },
	{ 0xFFFFFFFF },
};

static void config_mux(struct pin_muxing* pin_mux) {
	while(pin_mux->offset != 0xffffffff) {
		*(volatile uint32_t*)(CFG_MOD_BASE + pin_mux->offset) = pin_mux->val;
		pin_mux++;
	}
}

uint16_t flash_read_id(void) {
	static uint8_t xfer[6];
	xfer[0] = 0x90;
	xfer[1] = xfer[2] = xfer[3] = xfer[4] = xfer[5] = 0;

	am335x_spi_xfer(
			flash_spi.ctrl,
			flash_spi.chan,
			xfer,
			sizeof(xfer));

	return (xfer[4] << 8) | xfer[5];
}

uint8_t flash_read_b(uint32_t address) {
	static uint8_t xfer[5];
	xfer[0] = 0x03;
	xfer[1] = (address >> 16) & 0xFF;
	xfer[2] = (address >>  8) & 0xFF;
	xfer[3] = (address >>  0) & 0xFF;
	xfer[4] = 0;

	am335x_spi_xfer(
			flash_spi.ctrl,
			flash_spi.chan,
			xfer,
			sizeof(xfer));

	return xfer[4];

}

void flash_write_b(uint32_t address, uint8_t data) {
	static uint8_t xfer[5];
	xfer[0] = 0x02;
	xfer[1] = (address >> 16) & 0xFF;
	xfer[2] = (address >>  8) & 0xFF;
	xfer[3] = (address >>  0) & 0xFF;
	xfer[4] = data;

	am335x_spi_xfer(
			flash_spi.ctrl,
			flash_spi.chan,
			xfer,
			sizeof(xfer));
	return;
}
// SVC_Handler
void flash_init(void) {
	/* Configure GPIO
	 * ROLE          OSD    ZCZ     GPIO
	 * SPI0_SCLK     A13    A17     gpio0_2, mode 0
	 * SPI0_D0       B13    B17     gpio0_3, mode 0
	 * SPI0_D1       B14    B16     gpio0_4, mode 0
	 * SPI0_CS0      A14    A16     gpio0_5, mode 0
	 */

	config_mux(SPI0_flash_mux);
//	am335x_gpio_init(flash_spi.sclk.gpio);
//	am335x_gpio_init(flash_spi.d0.gpio);
//	am335x_gpio_init(flash_spi.d1.gpio);
//	am335x_gpio_init(flash_spi.cs0.gpio);

	am335x_spi_init(flash_spi.ctrl, flash_spi.chan, flash_spi.freq, 8);
}

void test_flash(void) {
	flash_init();
	uint16_t devid = flash_read_id();
	//printf("[FLASH] Flash Device ID: %04x (%s)\n", devid, ((devid == 0xEF17) ? "Good" : "Bad"));
}

