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
#include "am335x_gpio.h"

#include "pinmux.h"

/* Pin mux for m68k bus module */
static pin_muxing_t m68k_pin_mux[] = {
		{ CONF_GPMC_A1,       (IDIS | PTD | DIS | M0) }, /*GPMC_A1        */
		{ CONF_GPMC_A2,       (IDIS | PTD | DIS | M0) }, /*GPMC_A2        */
		{ CONF_GPMC_A3,       (IDIS | PTD | DIS | M0) }, /*GPMC_A3        */
		{ CONF_GPMC_A4,       (IDIS | PTD | DIS | M0) }, /*GPMC_A4        */
		{ CONF_GPMC_A5,       (IDIS | PTD | DIS | M0) }, /*GPMC_A5        */
		{ CONF_GPMC_A6,       (IDIS | PTD | DIS | M0) }, /*GPMC_A6        */
		{ CONF_GPMC_A7,       (IDIS | PTD | DIS | M0) }, /*GPMC_A7        */
		{ CONF_GPMC_A8,       (IDIS | PTD | DIS | M0) }, /*GPMC_A8        */
		{ CONF_GPMC_A9,       (IDIS | PTD | DIS | M0) }, /*GPMC_A9        */
		{ CONF_GPMC_A10,      (IDIS | PTD | DIS | M0) }, /*GPMC_A10       */
		{ CONF_GPMC_A11,      (IDIS | PTD | DIS | M0) }, /*GPMC_A11       */

		{ CONF_LCD_DATA8,     (IDIS | PTD | DIS | M1) }, /* GPMC_A12      */
		{ CONF_LCD_DATA9,     (IDIS | PTD | DIS | M1) }, /* GPMC_A13      */
		{ CONF_LCD_DATA10,    (IDIS | PTD | DIS | M1) }, /* GPMC_A14      */
		{ CONF_LCD_DATA11,    (IDIS | PTD | DIS | M1) }, /* GPMC_A15      */
		{ CONF_LCD_DATA12,    (IDIS | PTD | DIS | M1) }, /* GPMC_A16      */
		{ CONF_LCD_DATA13,    (IDIS | PTD | DIS | M1) }, /* GPMC_A17      */
		{ CONF_LCD_DATA14,    (IDIS | PTD | DIS | M1) }, /* GPMC_A18      */
		{ CONF_LCD_DATA15,    (IDIS | PTD | DIS | M1) }, /* GPMC_A19      */

		{ CONF_MMC0_DAT3,     (IDIS | PTD | DIS | M1) }, /* GPMC_A20      */
		{ CONF_MMC0_DAT2,     (IDIS | PTD | DIS | M1) }, /* GPMC_A21      */
		{ CONF_MMC0_DAT1,     (IDIS | PTD | DIS | M1) }, /* GPMC_A22      */
		{ CONF_MMC0_DAT0,     (IDIS | PTD | DIS | M1) }, /* GPMC_A23      */
		{ CONF_MMC0_CLK,      (IDIS | PTD | DIS | M1) }, /* GPMC_A24      */
		{ CONF_MMC0_CMD,      (IDIS | PTD | DIS | M1) }, /* GPMC_A25/FCx  */

		{ CONF_GPMC_AD0,      (IEN  | PTU | EN | M0) }, /* GPMC_D0       */
		{ CONF_GPMC_AD1,      (IEN  | PTU | EN | M0) }, /* GPMC_D1       */
		{ CONF_GPMC_AD2,      (IEN  | PTU | EN | M0) }, /* GPMC_D2       */
		{ CONF_GPMC_AD3,      (IEN  | PTU | EN | M0) }, /* GPMC_D3       */
		{ CONF_GPMC_AD4,      (IEN  | PTU | EN | M0) }, /* GPMC_D4       */
		{ CONF_GPMC_AD5,      (IEN  | PTU | EN | M0) }, /* GPMC_D5       */
		{ CONF_GPMC_AD6,      (IEN  | PTU | EN | M0) }, /* GPMC_D6       */
		{ CONF_GPMC_AD7,      (IEN  | PTU | EN | M0) }, /* GPMC_D7       */
		{ CONF_GPMC_AD8,      (IEN  | PTU | EN | M0) }, /* GPMC_D8       */
		{ CONF_GPMC_AD9,      (IEN  | PTU | EN | M0) }, /* GPMC_D9       */
		{ CONF_GPMC_AD10,     (IEN  | PTU | EN | M0) }, /* GPMC_D10      */
		{ CONF_GPMC_AD11,     (IEN  | PTU | EN | M0) }, /* GPMC_D11      */
		{ CONF_GPMC_AD12,     (IEN  | PTU | EN | M0) }, /* GPMC_D12      */
		{ CONF_GPMC_AD13,     (IEN  | PTU | EN | M0) }, /* GPMC_D13      */
		{ CONF_GPMC_AD14,     (IEN  | PTU | EN | M0) }, /* GPMC_D14      */
		{ CONF_GPMC_AD15,     (IEN  | PTU | EN | M0) }, /* GPMC_D15      */

		{ CONF_GPMC_CSN2,     (IDIS | PTD | DIS | M0) }, /* GPMC_nCS0     */
//		{ CONF_GPMC_CSN3,     (IDIS | PTD | DIS | M0) }, /* GPMC_nCS0     */
//		{ CONF_GPMC_ADVN_ALE, (IDIS | PTD | DIS | M0) }, /* GPMC_nADV_ALE */
		{ CONF_GPMC_OEN_REN,  (IDIS | PTD | DIS | M0) }, /* GPMC_nOE      */
		{ CONF_GPMC_WEN,      (IDIS | PTD | DIS | M0) }, /* GPMC_nWE      */
		{ CONF_GPMC_BEN0_CLE, (IDIS | PTD | DIS | M0) }, /* GPMC_nBE0_CLE */
		{ CONF_GPMC_BEN1,     (IDIS | PTD | DIS | M0) }, /* GPMC_nBE1     */
		{ CONF_GPMC_WPN,      (IEN  | PTD | DIS | M0) }, /* GPMC_nWP      */
		{ CONF_GPMC_WAIT0,    (IEN  | PTD | DIS | M0) }, /* GPMC_WAIT0    */

	//	{ CONF_GPMC_NCS1,     (IDIS | PTD | DIS | M0) }, /* GPMC_nCS1     */
	//	{ CONF_GPMC_NCS2,     (IDIS | PTD | DIS | M0) }, /* GPMC_nCS2     */
	//	{ CONF_GPMC_NCS3,     (IDIS | PTD | DIS | M0) }, /* GPMC_nCS3     */
	//	{ CONF_GPMC_NCS4,     (IEN  | PTD | DIS | M0) }, /* GPMC_nCS4     */
	//	{ CONF_GPMC_NCS5,     (IDIS | PTD | DIS | M0) }, /* GPMC_nCS5     */
	//	{ CONF_GPMC_NCS6,     (IEN  | PTD | DIS | M0) }, /* GPMC_nCS6     */
	//	{ CONF_GPMC_NCS7,     (IEN  | PTD | DIS | M0) }, /* GPMC_nCS7     */
	//	{ CONF_GPMC_CLK,      (IDIS | PTD | DIS | M0) }, /* GPMC_CLK      */
	//	{ CONF_GPMC_WAIT1,    (IEN  | PTD | DIS | M0) }, /* GPMC_WAIT1    */
	//	{ CONF_GPMC_WAIT2,    (IEN  | PTD | DIS | M4) }, /* GPIO_64       */
	//	{ CONF_GPMC_WAIT3,    (IEN  | PTD | DIS | M0) }, /* GPMC_WAIT3    */
		{ 0xFFFFFFFF },
};



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

/*

	HSDIVIDER   GPMC  TIMEPARA    WINDOW   68000 BUS
	CLKOUT1_DIV	FCLK  GRANULARITY MAX (ns) CLOCK (MHz) TARGET SYSTEM
	=========== ===== =========== ======== =========== ================
		16       62.5	  0.0       496	      16.13     ATARIx2**
		18       55.6	  0.0       558	      14.34     AMIGAx2**
		10      100.0	  1.0       620	      12.90     NEOGEO
		12       83.3	  1.0       744	      10.75     X68000
		16       62.5	  1.0       992	       8.06     ATARI/MAC/SEGA*
		18       55.6	  1.0      1116	       7.17     AMIGA
		25       40.0	  1.0      1550	       5.16     LISA

	NOTES:
	  * Use this one as a generic "we don't know" setting
	 ** When using basic clock doublers like AdSpeed

*/

/* General 68K bus timing:
 *
 * AS on 1/8
 *
 *
 */

#define ACCESSTIME		(0x19)		//   AccessTime

#define CSONTIME		(0x04)		// 	 tCAS
#define ADVONTIME		(0x00)		//   tAAVDS
#define OEONTIME		(0x13)		// 	 OeOnTime >= AdvRdOffTime
#define OEOFFTIME		(0x1C)	// = AccessTime + 1cycle

#define RDCYCLETIME		(0x1F)	// = AccessTime + 1cycle + tOEZ
#define CSRDOFFTIME		(0x1C)	// = AccessTime + 1 cycle
#define ADVRDOFFTIME	(0x00)		// = tAAVDS + tAVDP

#define WRCYCLETIME		(0x1F)		// = WeOffTime + AccessCompletion
#define CSWROFFTIME		(0x16)		// = WeOffTime + 1
#define ADVWROFFTIME	(0x00)		// = tAVSC + tAVDP
#define WEONTIME		(0x0A)		//   tCS
#define WEOFFTIME		(0x1C)		//   WeOffTime

static am335x_gpmc_cs_config_t sram_config = {
		// ( 1 << 22) | ( 1 << 21) |
		/* CONFIG1 */ /*( 3 << 21) |*/ ( 2 << 18) | ( 1 << 12) | (1 << 4),
		/* CONFIG2 */ (CSWROFFTIME  << 16) | (CSRDOFFTIME  <<  8) | (CSONTIME  <<  0),
		/* CONFIG3 */ (ADVWROFFTIME << 16) | (ADVRDOFFTIME <<  8) | (ADVONTIME <<  0),
		/* CONFIG4 */ (WEOFFTIME << 24) | (WEONTIME << 20),
		/* CONFIG5 */ (ACCESSTIME << 16) | (WRCYCLETIME <<  8) | (RDCYCLETIME <<  0),
		/* CONFIG6 */ (ACCESSTIME << 24),
		/* CONFIG7 */ (14 <<  8) | ( 1 <<  6) | ( 1 <<  0)
};

typedef struct {
	char* name;
	uint32_t clk_div;

} gpmc_system_config_t;

//static gpmc_system_config_t configs[] = {
//
//};

void init_gpmc(void) {

	// bit bang ALE for test, gpio2_2
	//am335x_gpio_set_pin_dir(AM335X_GPIO2, 2, AM335X_GPIO_PIN_OUT);
	//am335x_gpio_change_state(AM335X_GPIO2, 2, 1);
	//am335x_gpio_change_state(AM335X_GPIO2, 2, 0);
	//am335x_gpio_change_state(AM335X_GPIO2, 2, 1);
	//am335x_gpio_change_state(AM335X_GPIO2, 2, 0);

	// Enable and wait for clock
	//am335x_clock_enable_gpmc();
	am335x_gpmc_init(16);

	// Enable pins
	config_mux(m68k_pin_mux);

	// Set CONFIGx pins
	am335x_gpmc_enable_cs_config(
			&sram_config,	// config
			2,				// chip select
			0x01000000,		// base
			0x02000000		// size
	);
}

void test_gpmc(void) {
	static uint16_t test_colors[] = {
			0x0fff, 0x0f0f, 0x0555, 0x0505,
			0x0aaa, 0x0a0a, 0x00a0, 0x0050
	};
	static uint16_t should_be[] = {
			0x1111, 0x4EF9, 0x00FC, 0x00D2,
			0x0000, 0xFFFF, 0x0022, 0x0005,
			0x0022, 0x0002, 0xFFFF, 0xFFFF,
			0x6578, 0x6563, 0x2033, 0x342E
	};
	static uint16_t read[16] = {0};
	volatile uint16_t* d_base = (uint32_t*)0x00000000;
	volatile uint8_t* cia_base = (uint8_t*)0x00BFE001;
	volatile uint16_t* custom = (uint32_t*)0x00DFF000;
	//uint32_t* i_base = (uint32_t*)0x02000000;

	//uint8_t buffer[128];
	//strncpy(buffer, &d_base[19], 127);
	//buffer[127] = 0;

	//printf("[GPMC] ROM Check: %s", buffer);

	printf("[GPMC] Starting bus test\n");

	for(uint32_t i=0; i<16; i++) read[i] = d_base[i];

	for(uint32_t i=0; i<16; i++)
		printf("[GPMC] At %08X, read %04X, expected %04x\n", (uint32_t)&d_base[i], read[i], should_be[i]);

//	for(int x=0; x<16; x++) {
//		uint8_t read = cia_base[x << 8];
//		printf("[GPMC] At %08X, read %02X\n", (uint32_t)&cia_base[x << 8], read);
//	}

	//printf("[GPMC] DENISEID %04X", custom[0x7C >> 1]);
	//printf("[GPMC] DENISEID %04X", custom[0x7C >> 1]);
	//printf("[GPMC] DENISEID %04X", custom[0x7C >> 1]);

//	for(int c=0; c<8; c++) custom[0xC0 + c] = test_colors[c];
//
//	for(int c=0; c<8; c++) {
//		uint16_t read_back = custom[0xC0 + c];
//		printf("[GPMC] At dff180, read %04X, expected %04x\n", read_back, test_colors[c]);
//	}


}


