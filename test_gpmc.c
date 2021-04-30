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
#include "am335x_gpmc.h"

//#define GPMC_BASE 		   ((volatile uint32_t*)0x50000000)
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
#define CONF_MMC0_CMD       0x904   // Address bit 25   MODE(1)

#define CONF_GPMC_WAIT0     0x870   // MODE(0)
#define CONF_GPMC_NWP		0x874   // MODE(0)
#define CONF_GPMC_NBE1      0x878   // MODE(0)
#define CONF_GPMC_NCS0		0x87C   // MODE(0)
#define CONF_GPMC_NADV_ALE  0x890   // MODE(0)
#define CONF_GPMC_NOE		0x894   // MODE(0)
#define CONF_GPMC_NWE		0x898   // MODE(0)
#define CONF_GPMC_NBE0_CLE	0x89C   // MODE(0)




/*
 * IEN  - Input Enable
 * IDIS - Input Disable
 * PTD  - Pull type Down
 * PTU  - Pull type Up
 * DIS  - Pull type selection is inactive
 * EN   - Pull type selection is active
 * M0   - Mode 0
 * The commented string gives the final mux configuration for that pin
 */
#define IEN		(1 << 5)
#define IDIS	(0 << 5)
#define PTU		(1 << 4)
#define PTD		(0 << 4)
#define EN		(1 << 3)
#define DIS		(0 << 3)

#define M0		(0 << 0)
#define M1		(1 << 0)
#define M2		(2 << 0)
#define M3		(3 << 0)
#define M4		(4 << 0)
#define M5		(5 << 0)
#define M6		(6 << 0)
#define M7		(7 << 0)

/* Pin mux for m68k bus module */
static struct pin_muxing {
	uint32_t offset;
	uint32_t val;
} m68k_pin_mux[] = {
		{ CONF_GPMC_A1,       (IDIS | PTU | EN  | M0) }, /*GPMC_A1        */
		{ CONF_GPMC_A2,       (IDIS | PTU | EN  | M0) }, /*GPMC_A2        */
		{ CONF_GPMC_A3,       (IDIS | PTU | EN  | M0) }, /*GPMC_A3        */
		{ CONF_GPMC_A4,       (IDIS | PTU | EN  | M0) }, /*GPMC_A4        */
		{ CONF_GPMC_A5,       (IDIS | PTU | EN  | M0) }, /*GPMC_A5        */
		{ CONF_GPMC_A6,       (IDIS | PTU | EN  | M0) }, /*GPMC_A6        */
		{ CONF_GPMC_A7,       (IDIS | PTU | EN  | M0) }, /*GPMC_A7        */
		{ CONF_GPMC_A8,       (IDIS | PTU | EN  | M0) }, /*GPMC_A8        */
		{ CONF_GPMC_A9,       (IDIS | PTU | EN  | M0) }, /*GPMC_A9        */
		{ CONF_GPMC_A10,      (IDIS | PTU | EN  | M0) }, /*GPMC_A10       */
		{ CONF_GPMC_A11,      (IDIS | PTU | EN  | M0) }, /*GPMC_A11       */

		{ CONF_LCD_DATA8,     (IDIS | PTU | EN  | M1) }, /* GPMC_A12      */
		{ CONF_LCD_DATA9,     (IDIS | PTU | EN  | M1) }, /* GPMC_A13      */
		{ CONF_LCD_DATA10,    (IDIS | PTU | EN  | M1) }, /* GPMC_A14      */
		{ CONF_LCD_DATA11,    (IDIS | PTU | EN  | M1) }, /* GPMC_A15      */
		{ CONF_LCD_DATA12,    (IDIS | PTU | EN  | M1) }, /* GPMC_A16      */
		{ CONF_LCD_DATA13,    (IDIS | PTU | EN  | M1) }, /* GPMC_A17      */
		{ CONF_LCD_DATA14,    (IDIS | PTU | EN  | M1) }, /* GPMC_A18      */
		{ CONF_LCD_DATA15,    (IDIS | PTU | EN  | M1) }, /* GPMC_A19      */

		{ CONF_MMC0_DAT3,     (IDIS | PTU | EN  | M1) }, /* GPMC_A20      */
		{ CONF_MMC0_DAT2,     (IDIS | PTU | EN  | M1) }, /* GPMC_A21      */
		{ CONF_MMC0_DAT1,     (IDIS | PTU | EN  | M1) }, /* GPMC_A22      */
		{ CONF_MMC0_DAT0,     (IDIS | PTU | EN  | M1) }, /* GPMC_A23      */
		{ CONF_MMC0_CLK,      (IDIS | PTU | EN  | M1) }, /* GPMC_A24      */
		{ CONF_MMC0_CMD,      (IDIS | PTU | EN  | M1) }, /* GPMC_A25/FCx  */

		{ CONF_GPMC_AD0,      (IEN  | PTU | EN  | M0) }, /* GPMC_D0       */
		{ CONF_GPMC_AD1,      (IEN  | PTU | EN  | M0) }, /* GPMC_D1       */
		{ CONF_GPMC_AD2,      (IEN  | PTU | EN  | M0) }, /* GPMC_D2       */
		{ CONF_GPMC_AD3,      (IEN  | PTU | EN  | M0) }, /* GPMC_D3       */
		{ CONF_GPMC_AD4,      (IEN  | PTU | EN  | M0) }, /* GPMC_D4       */
		{ CONF_GPMC_AD5,      (IEN  | PTU | EN  | M0) }, /* GPMC_D5       */
		{ CONF_GPMC_AD6,      (IEN  | PTU | EN  | M0) }, /* GPMC_D6       */
		{ CONF_GPMC_AD7,      (IEN  | PTU | EN  | M0) }, /* GPMC_D7       */
		{ CONF_GPMC_AD8,      (IEN  | PTU | EN  | M0) }, /* GPMC_D8       */
		{ CONF_GPMC_AD9,      (IEN  | PTU | EN  | M0) }, /* GPMC_D9       */
		{ CONF_GPMC_AD10,     (IEN  | PTU | EN  | M0) }, /* GPMC_D10      */
		{ CONF_GPMC_AD11,     (IEN  | PTU | EN  | M0) }, /* GPMC_D11      */
		{ CONF_GPMC_AD12,     (IEN  | PTU | EN  | M0) }, /* GPMC_D12      */
		{ CONF_GPMC_AD13,     (IEN  | PTU | EN  | M0) }, /* GPMC_D13      */
		{ CONF_GPMC_AD14,     (IEN  | PTU | EN  | M0) }, /* GPMC_D14      */
		{ CONF_GPMC_AD15,     (IEN  | PTU | EN  | M0) }, /* GPMC_D15      */

		{ CONF_GPMC_NCS0,     (IDIS | PTU | EN  | M0) }, /* GPMC_nCS0     */
		{ CONF_GPMC_NADV_ALE, (IDIS | PTD | DIS | M0) }, /* GPMC_nADV_ALE */
		{ CONF_GPMC_NOE,      (IDIS | PTD | DIS | M0) }, /* GPMC_nOE      */
		{ CONF_GPMC_NWE,      (IDIS | PTD | DIS | M0) }, /* GPMC_nWE      */
		{ CONF_GPMC_NBE0_CLE, (IDIS | PTU | EN  | M0) }, /* GPMC_nBE0_CLE */
		{ CONF_GPMC_NBE1,     (IEN  | PTU | EN  | M0) }, /* GPMC_nBE1     */
		{ CONF_GPMC_NWP,      (IEN  | PTD | DIS | M0) }, /* GPMC_nWP      */
		{ CONF_GPMC_WAIT0,    (IEN  | PTU | EN  | M0) }, /* GPMC_WAIT0    */

	//	{ CONF_GPMC_NCS1,     (IDIS | PTU | EN  | M0) }, /* GPMC_nCS1     */
	//	{ CONF_GPMC_NCS2,     (IDIS | PTU | EN  | M0) }, /* GPMC_nCS2     */
	//	{ CONF_GPMC_NCS3,     (IDIS | PTU | EN  | M0) }, /* GPMC_nCS3     */
	//	{ CONF_GPMC_NCS4,     (IEN  | PTU | EN  | M0) }, /* GPMC_nCS4     */
	//	{ CONF_GPMC_NCS5,     (IDIS | PTU | EN  | M0) }, /* GPMC_nCS5     */
	//	{ CONF_GPMC_NCS6,     (IEN  | PTU | EN  | M0) }, /* GPMC_nCS6     */
	//	{ CONF_GPMC_NCS7,     (IEN  | PTU | EN  | M0) }, /* GPMC_nCS7     */
	//	{ CONF_GPMC_CLK,      (IDIS | PTU | EN  | M0) }, /* GPMC_CLK      */
	//	{ CONF_GPMC_WAIT1,    (IEN  | PTU | EN  | M0) }, /* GPMC_WAIT1    */
	//	{ CONF_GPMC_WAIT2,    (IEN  | PTU | EN  | M4) }, /* GPIO_64       */
	//	{ CONF_GPMC_WAIT3,    (IEN  | PTU | EN  | M0) }, /* GPMC_WAIT3    */
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

static am335x_gpmc_cs_config_t sram_config = {
		/* CONFIG1 */ ( 1 << 12),
		/* CONFIG2 */ (26 << 16) | (26 <<  8) | ( 1 <<  0),
		/* CONFIG3 */ (26 << 16) | (26 <<  8) | ( 7 <<  0),
		/* CONFIG4 */ (26 << 24) | ( 1 << 20),
		/* CONFIG5 */ (15 << 16) | (26 <<  8) | (26 <<  0),
		/* CONFIG6 */ (15 << 24),
		/* CONFIG7 */ (14 <<  8) | ( 1 <<  6) | ( 1 <<  0)
};

void init_gpmc(void) {
	// Enable and wait for clock
	//am335x_clock_enable_gpmc();
	am335x_gpmc_init();

	// Enable pins
	config_mux(m68k_pin_mux);

	// Set CONFIGx pins
	am335x_gpmc_enable_cs_config(
			&sram_config,	// config
			0,				// chip select
			0x01000000,		// base
			0x02000000		// size
	);
}

void test_gpmc(void) {
	uint32_t* d_base = (uint32_t*)0x01000000;
	//uint32_t* i_base = (uint32_t*)0x02000000;

	printf("[GPMC] Starting bus test\n");
	for(uint32_t i=0; i<0x10; i++) {
		uint32_t mask = i * 0x11111111;
		//printf("Read %08X at %08X\n", d_base[i], (uint32_t)&d_base[i]);
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


