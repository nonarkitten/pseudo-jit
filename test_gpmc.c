/*
 * test_gpmc.c
 *
 *  Created on: Apr. 18, 2021
 *      Author: renee.cousins
 */

#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>

#include "am335x_clock.h"

#define GPMC_BASE 		   ((volatile uint32_t*)0x50000000)
#define CFG_MOD_BASE       (0x44E10000)      //REVISIT

#define CONF_GPMC_AD0       0x800   // Data bit 0       MODE(0)
#define CONF_GPMC_AD1       0x804   // Data bit 1       MODE(0)
#define CONF_GPMC_AD2       0x808   // Data bit 2       MODE(0)
#define CONF_GPMC_AD3       0x80C   // Data bit 3       MODE(0)
#define CONF_GPMC_AD4       0x810   // Data bit 4       MODE(0)
#define CONF_GPMC_AD5       0x814   // Data bit 5       MODE(0)
#define CONF_GPMC_AD6       0x818   // Data bit 6       MODE(0)
#define CONF_GPMC_AD7       0x81C   // Data bit 7       MODE(0)
#define CONF_GPMC_AD8       0x820   // Data bit 8       MODE(0)
#define CONF_GPMC_AD9       0x824   // Data bit 9       MODE(0)
#define CONF_GPMC_AD10      0x828   // Data bit 10      MODE(0)
#define CONF_GPMC_AD11      0x82C   // Data bit 11      MODE(0)
#define CONF_GPMC_AD12      0x830   // Data bit 12      MODE(0)
#define CONF_GPMC_AD13      0x834   // Data bit 13      MODE(0)
#define CONF_GPMC_AD14      0x838   // Data bit 14      MODE(0)
#define CONF_GPMC_AD15      0x83C   // Data bit 15      MODE(0)
#define CONF_GPMC_A1        0x844   // Address bit 1    MODE(0)
#define CONF_GPMC_A2        0x848   // Address bit 2    MODE(0)
#define CONF_GPMC_A3        0x84C   // Address bit 3    MODE(0)
#define CONF_GPMC_A4        0x850   // Address bit 4    MODE(0)
#define CONF_GPMC_A5        0x854   // Address bit 5    MODE(0)
#define CONF_GPMC_A6        0x858   // Address bit 6    MODE(0)
#define CONF_GPMC_A7        0x85C   // Address bit 7    MODE(0)
#define CONF_GPMC_A8        0x860   // Address bit 8    MODE(0)
#define CONF_GPMC_A9        0x864   // Address bit 9    MODE(0)
#define CONF_GPMC_A10       0x868   // Address bit 10   MODE(0)
#define CONF_GPMC_A11       0x86C   // Address bit 11   MODE(0)

#define CONF_LCD_DATA8      0x8C0   // Address bit 12   MODE(1)
#define CONF_LCD_DATA9      0x8C4   // Address bit 13   MODE(1)
#define CONF_LCD_DATA10     0x8C8   // Address bit 14   MODE(1)
#define CONF_LCD_DATA11     0x8CC   // Address bit 15   MODE(1)
#define CONF_LCD_DATA12     0x8D0   // Address bit 16   MODE(1)
#define CONF_LCD_DATA13     0x8D4   // Address bit 17   MODE(1)
#define CONF_LCD_DATA14     0x8D8   // Address bit 18   MODE(1)
#define CONF_LCD_DATA15     0x8DC   // Address bit 19   MODE(1)
#define CONF_MMC0_DAT3      0x8F0   // Address bit 20   MODE(1)
#define CONF_MMC0_DAT2      0x8F4   // Address bit 21   MODE(1)
#define CONF_MMC0_DAT1      0x8F8   // Address bit 22   MODE(1)
#define CONF_MMC0_DAT0      0x8FC   // Address bit 23   MODE(1)
#define CONF_MMC0_CLK       0x900   // Address bit 24   MODE(1)

#define CONF_GPMC_WAIT0     0x870   // MODE(0)
#define CONF_GPMC_BEN1      0x878   // MODE(0)
#define CONF_GPMC_ADVN_ALE  0x890   // MODE(0)
#define CONF_GPMC_WEN		0x898   // MODE(0)
#define CONF_GPMC_BEN0_CLE	0x89C   // MODE(0)

#define MODE(val)        (val << 0)

#define PULL_UD_DISABLE  (1 << 3) /* PULL UP/DOWN disabled */
#define PULL_TYPESEL     (1 << 4) /* PULL UP Select */
#define RXACTIVE         (1 << 5)
#define SLEW_CTRL        (1 << 7)
#define PULL_UP_EN       (PULL_TYPESEL) /* PUL UP Select, PULL_UD_EN = 0 */

/* Pin mux for m68k bus module */
static struct pin_muxing {
	uint32_t offset;
	uint32_t val;
} m68k_pin_mux[] = {
		{ CONF_GPMC_AD0,        MODE(0) | PULL_UP_EN | RXACTIVE },
		{ CONF_GPMC_AD1,        MODE(0) | PULL_UP_EN | RXACTIVE },
		{ CONF_GPMC_AD2,        MODE(0) | PULL_UP_EN | RXACTIVE },
		{ CONF_GPMC_AD3,        MODE(0) | PULL_UP_EN | RXACTIVE },
		{ CONF_GPMC_AD4,        MODE(0) | PULL_UP_EN | RXACTIVE },
		{ CONF_GPMC_AD5,        MODE(0) | PULL_UP_EN | RXACTIVE },
		{ CONF_GPMC_AD6,        MODE(0) | PULL_UP_EN | RXACTIVE },
		{ CONF_GPMC_AD7,        MODE(0) | PULL_UP_EN | RXACTIVE },
		{ CONF_GPMC_AD8,        MODE(0) | PULL_UP_EN | RXACTIVE },
		{ CONF_GPMC_AD9,        MODE(0) | PULL_UP_EN | RXACTIVE },
		{ CONF_GPMC_AD10,       MODE(0) | PULL_UP_EN | RXACTIVE },
		{ CONF_GPMC_AD11,       MODE(0) | PULL_UP_EN | RXACTIVE },
		{ CONF_GPMC_AD12,       MODE(0) | PULL_UP_EN | RXACTIVE },
		{ CONF_GPMC_AD13,       MODE(0) | PULL_UP_EN | RXACTIVE },
		{ CONF_GPMC_AD14,       MODE(0) | PULL_UP_EN | RXACTIVE },
		{ CONF_GPMC_AD15,       MODE(0) | PULL_UP_EN | RXACTIVE },
		{ CONF_GPMC_A1,         MODE(0) | PULL_UD_DISABLE },
		{ CONF_GPMC_A2,         MODE(0) | PULL_UD_DISABLE },
		{ CONF_GPMC_A3,         MODE(0) | PULL_UD_DISABLE },
		{ CONF_GPMC_A4,         MODE(0) | PULL_UD_DISABLE },
		{ CONF_GPMC_A5,         MODE(0) | PULL_UD_DISABLE },
		{ CONF_GPMC_A6,         MODE(0) | PULL_UD_DISABLE },
		{ CONF_GPMC_A7,         MODE(0) | PULL_UD_DISABLE },
		{ CONF_GPMC_A8,         MODE(0) | PULL_UD_DISABLE },
		{ CONF_GPMC_A9,         MODE(0) | PULL_UD_DISABLE },
		{ CONF_GPMC_A10,        MODE(0) | PULL_UD_DISABLE },
		{ CONF_GPMC_A11,        MODE(0) | PULL_UD_DISABLE },
		{ CONF_LCD_DATA8,       MODE(1) | PULL_UD_DISABLE },
		{ CONF_LCD_DATA8,       MODE(1) | PULL_UD_DISABLE },
		{ CONF_LCD_DATA8,       MODE(1) | PULL_UD_DISABLE },
		{ CONF_LCD_DATA8,       MODE(1) | PULL_UD_DISABLE },
		{ CONF_LCD_DATA8,       MODE(1) | PULL_UD_DISABLE },
		{ CONF_LCD_DATA8,       MODE(1) | PULL_UD_DISABLE },
		{ CONF_LCD_DATA8,       MODE(1) | PULL_UD_DISABLE },
		{ CONF_LCD_DATA8,       MODE(1) | PULL_UD_DISABLE },
		{ CONF_MMC0_DAT3,       MODE(1) | PULL_UD_DISABLE },
		{ CONF_MMC0_DAT2,       MODE(1) | PULL_UD_DISABLE },
		{ CONF_MMC0_DAT1,       MODE(1) | PULL_UD_DISABLE },
		{ CONF_MMC0_DAT0,       MODE(1) | PULL_UD_DISABLE },
		{ CONF_MMC0_CLK,        MODE(1) | PULL_UD_DISABLE },
		{ CONF_GPMC_WAIT0,      MODE(0) | PULL_UP_EN | RXACTIVE },
		{ CONF_GPMC_BEN1,       MODE(0) | PULL_UD_DISABLE },
		{ CONF_GPMC_ADVN_ALE,   MODE(0) | PULL_UD_DISABLE },
		{ CONF_GPMC_WEN		,   MODE(0) | PULL_UD_DISABLE },
		{ CONF_GPMC_BEN0_CLE,   MODE(0) | PULL_UD_DISABLE },
		{ 0xFFFFFFFF },
};

void config_mux(struct pin_muxing* pin_mux) {
	while(pin_mux->offset != 0xffffffff) {
		*(volatile uint32_t*)(CFG_MOD_BASE + pin_mux->offset) = pin_mux->val;
		pin_mux++;
	}
}

/**
 * Amiga Uses 41256-15
 *
 * nCS	Not observed on 68K bus
 * nADV	Used as 68K AS signal
 * 		ADVWROFFTIME
 * nWE	Used at 68K RW signal
 * 		WEONETIME
 *
 * 150ns access time (read or write)
 * 260ns cycle time
 *  70ns AS assert
 * 260ns AS deassert
 *
 * GPMC has the following times
 *
 * CONFIG1			Value		Sh	68K		Notes
 * 	DEVICESIZE		8/16		12	1		16=1, be sure to set this
 *
 * CONFIG2
 * 	CSWROFFTIME		0-310ns		16			Should match ADV
 * 	CSRDOFFTIME		0-310ns		8			"
 * 	CSEXTRADELAY	+5ns		7			"
 * 	CSONTIME		0-150ns		0			Should be zero or one
 *
 * CONFIG3			Value		Sh	68K		Notes
 * 	ADVWROFFTIME	0-310ns		16			#ADV de-assert time from cycle start for writes
 * 	ADVRDOFFTIME	0-310ns		8			#ADV de-assert time from cycle start for reads
 * 	ADVEXTRADELAY	+5ns		7			#ADV timing shifted by 5ns
 * 	ADVONTIME		0-150ns		0	7		#ADV assertion time from the cycle start
 *
 * CONFIG4
 * 	WEOFFTIME		0-310ns		24			#WE de-assert time from cycle start
 * 	WEEXTRADELAY	+5ns		23			#WE timing delayed 5ns
 * 	WEONTIME		0-150ns		20			#WE assert time from cycle start
 *
 * CONFIG5
 * 	RDACCESSTIME	0-310ns		16			Delay from cycle start to data valid
 * 	WRCYCLETIME		0-310ns		8			Total write cycle time
 * 	RDCYCLETIME		0-310ns		0			Total read cycle time
 *
 * CONFIG6
 * 	WRACCESSTIME	0-310ns		24			Delay from cycle start to when RAM should latch
 *
 * CONFIG7
 * 	MASKADDRESS		0/8/C/E/F	8			Chip-select address mask
 * 											0=256M, 8=128M, C=64M, E=32M, F=16M
 * 	CSENABLE					6			Chip-select enable
 * 	BASEADDRESS					0			Chip-Select base address
 * 											Bits 5-0 correspond to A29~A24
 *
 */

void _set_gpmc_conf(volatile uint32_t *conf)
{
	conf += 0x30 / 4;

	conf[0x1E] = 0;

	/* CONFIG1 */ conf[0x18] = ( 1 << 12);
	/* CONFIG2 */ conf[0x19] = (26 << 16) | (26 <<  8) | ( 1 <<  0);
	/* CONFIG3 */ conf[0x1A] = (26 << 16) | (26 <<  8) | ( 7 <<  0);
	/* CONFIG4 */ conf[0x1B] = (26 << 24) | ( 1 << 20);
	/* CONFIG5 */ conf[0x1C] = (26 << 16) | (15 <<  8) | (15 <<  0);
	/* CONFIG6 */ conf[0x1D] = (26 << 24);
	/* CONFIG7 */ conf[0x1E] = (14 <<  8) | ( 1 <<  6) | ( 1 <<  0);
}

void test_gpmc(void) {
	// Enable and wait for clock
	am335x_clock_enable_gpmc();

	// Enable pins
	config_mux(m68k_pin_mux);

	// GPMC Soft reset
	GPMC_BASE[0x04] |= 2;
	while ((GPMC_BASE[0x05] & 1) == 0);

	// Configure to no idle
	GPMC_BASE[0x04] |= 8;

	// Clear IRQ
	GPMC_BASE[0x07] = 0;
	// Clear timeout
	GPMC_BASE[0x10] = 0;
	// Clear config
	GPMC_BASE[0x14] = 0;

	// Set CONFIGx pins
	_set_gpmc_conf(GPMC_BASE);

	// 7.1.3.2/7.1.3.3 GPMC Configuration
	//	Memory Type
	//	Chip-Select Configuration
	//	Configure Timings
	//	Wait Pin Configuration
	//	Enable Chip-Select

	// 7.1.3.4 Set memory access

	// 7.1.3.5 GPMC Timing Parameters


	uint32_t* d_base = (uint32_t*)0x01000000;
	uint32_t* i_base = (uint32_t*)0x02000000;

	printf("[GPMC] Starting bus test\n");
	for(uint32_t i=0; i<0x10; i++) {
		uint32_t mask = i * 0x11111111;
		printf("Read %08X at %08X\n", d_base[i], (uint32_t)&d_base[i]);
		d_base[i] = mask;
//		i_base[i] = mask;
	}

	for(uint32_t i=0; i<0x10; i++) {
		uint32_t mask = i * 0x11111111;
		uint32_t read;
		if((read = d_base[i]) != mask) {
			printf("[GPMC] Error at %08X, read %08X, expected %08X\n",
					(uint32_t)&d_base[i], read, mask
					);
		}
//		if((read = i_base[i]) != mask) {
//			printf("[GPMC] Error at %08X, read %08X, expected %08X\n",
//					(uint32_t)&i_base[i], read, mask
//					);
//		}
	}
}


