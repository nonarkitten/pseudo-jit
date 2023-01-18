/*
 * Copyright (c) 2020-2023 Renee Cousins, the Buffee Project - http://www.buffee.ca
 *
 * This is part of PJIT the Pseudo-JIT 68K emulator.
 *
 * PJIT is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * PJIT is licensed under a Creative Commons
 * Attribution-NonCommercial-ShareAlike 4.0 International License.
 *
 * Under the terms of this license you are free copy and redistribute
 * the material in any medium or format as well as remix, transform,
 * and build upon the material.
 *
 * You must give appropriate credit, provide a link to the license,
 * and indicate if changes were made. You may do so in any reasonable
 * manner, but not in any way that suggests the licensor endorses you
 * or your use.
 *
 * You may not use the material for commercial purposes.
 *
 * If you remix, transform, or build upon the material, you must
 * distribute your contributions under the same license as the original.
 *
 * You may not apply legal terms or technological measures that legally
 * restrict others from doing anything the license permits.
 *
 * Portions of PJIT have been derived from the following:
 *
 *     Castaway (formerly FAST), GPL version 2 License
 *     Copyright (c) 1994-2002 Martin Döring, Joachim Hönig
 *    
 *     Cyclone 68K, GPL version 2 License
 *     Copyright (c) 2004,2011 Dave "FinalDave" Haywood
 *     Copyright (c) 2005-2011 Graûvydas "notaz" Ignotas
 *    
 *     TI StarterWare, modified BSD 3-Clause License
 *     Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/
 *
 *     libbbb, Apache License, Version 2.0
 *     Copyright 2015 University of Applied Sciences Western Switzerland / Fribourg
 */
#include "main.h"

/* Pin mux for m68k bus module */
static const pin_muxing_t m68k_pin_mux[] = {
		{ CONF_GPMC_A1,       (PIN_CFG_IDIS | PIN_CFG_PTD | PIN_CFG_DIS | PIN_CFG_M0) }, /*GPMC_A1        */
		{ CONF_GPMC_A2,       (PIN_CFG_IDIS | PIN_CFG_PTD | PIN_CFG_DIS | PIN_CFG_M0) }, /*GPMC_A2        */
		{ CONF_GPMC_A3,       (PIN_CFG_IDIS | PIN_CFG_PTD | PIN_CFG_DIS | PIN_CFG_M0) }, /*GPMC_A3        */
		{ CONF_GPMC_A4,       (PIN_CFG_IDIS | PIN_CFG_PTD | PIN_CFG_DIS | PIN_CFG_M0) }, /*GPMC_A4        */
		{ CONF_GPMC_A5,       (PIN_CFG_IDIS | PIN_CFG_PTD | PIN_CFG_DIS | PIN_CFG_M0) }, /*GPMC_A5        */
		{ CONF_GPMC_A6,       (PIN_CFG_IDIS | PIN_CFG_PTD | PIN_CFG_DIS | PIN_CFG_M0) }, /*GPMC_A6        */
		{ CONF_GPMC_A7,       (PIN_CFG_IDIS | PIN_CFG_PTD | PIN_CFG_DIS | PIN_CFG_M0) }, /*GPMC_A7        */
		{ CONF_GPMC_A8,       (PIN_CFG_IDIS | PIN_CFG_PTD | PIN_CFG_DIS | PIN_CFG_M0) }, /*GPMC_A8        */
		{ CONF_GPMC_A9,       (PIN_CFG_IDIS | PIN_CFG_PTD | PIN_CFG_DIS | PIN_CFG_M0) }, /*GPMC_A9        */
		{ CONF_GPMC_A10,      (PIN_CFG_IDIS | PIN_CFG_PTD | PIN_CFG_DIS | PIN_CFG_M0) }, /*GPMC_A10       */
		{ CONF_GPMC_A11,      (PIN_CFG_IDIS | PIN_CFG_PTD | PIN_CFG_DIS | PIN_CFG_M0) }, /*GPMC_A11       */

		{ CONF_LCD_DATA8,     (PIN_CFG_IDIS | PIN_CFG_PTD | PIN_CFG_DIS | PIN_CFG_M1) }, /* GPMC_A12      */
		{ CONF_LCD_DATA9,     (PIN_CFG_IDIS | PIN_CFG_PTD | PIN_CFG_DIS | PIN_CFG_M1) }, /* GPMC_A13      */
		{ CONF_LCD_DATA10,    (PIN_CFG_IDIS | PIN_CFG_PTD | PIN_CFG_DIS | PIN_CFG_M1) }, /* GPMC_A14      */
		{ CONF_LCD_DATA11,    (PIN_CFG_IDIS | PIN_CFG_PTD | PIN_CFG_DIS | PIN_CFG_M1) }, /* GPMC_A15      */
		{ CONF_LCD_DATA12,    (PIN_CFG_IDIS | PIN_CFG_PTD | PIN_CFG_DIS | PIN_CFG_M1) }, /* GPMC_A16      */
		{ CONF_LCD_DATA13,    (PIN_CFG_IDIS | PIN_CFG_PTD | PIN_CFG_DIS | PIN_CFG_M1) }, /* GPMC_A17      */
		{ CONF_LCD_DATA14,    (PIN_CFG_IDIS | PIN_CFG_PTD | PIN_CFG_DIS | PIN_CFG_M1) }, /* GPMC_A18      */
		{ CONF_LCD_DATA15,    (PIN_CFG_IDIS | PIN_CFG_PTD | PIN_CFG_DIS | PIN_CFG_M1) }, /* GPMC_A19      */

		{ CONF_MMC0_DAT3,     (PIN_CFG_IDIS | PIN_CFG_PTD | PIN_CFG_DIS | PIN_CFG_M1) }, /* GPMC_A20      */
		{ CONF_MMC0_DAT2,     (PIN_CFG_IDIS | PIN_CFG_PTD | PIN_CFG_DIS | PIN_CFG_M1) }, /* GPMC_A21      */
		{ CONF_MMC0_DAT1,     (PIN_CFG_IDIS | PIN_CFG_PTD | PIN_CFG_DIS | PIN_CFG_M1) }, /* GPMC_A22      */
		{ CONF_MMC0_DAT0,     (PIN_CFG_IDIS | PIN_CFG_PTD | PIN_CFG_DIS | PIN_CFG_M1) }, /* GPMC_A23      */
		{ CONF_MMC0_CLK,      (PIN_CFG_IDIS | PIN_CFG_PTD | PIN_CFG_DIS | PIN_CFG_M1) }, /* GPMC_A24      */
		{ CONF_MMC0_CMD,      (PIN_CFG_IDIS | PIN_CFG_PTD | PIN_CFG_DIS | PIN_CFG_M1) }, /* GPMC_A25/FCx  */

		{ CONF_GPMC_AD0,      (PIN_CFG_IEN  | PIN_CFG_PTU | PIN_CFG_EN | PIN_CFG_M0) }, /* GPMC_D0       */
		{ CONF_GPMC_AD1,      (PIN_CFG_IEN  | PIN_CFG_PTU | PIN_CFG_EN | PIN_CFG_M0) }, /* GPMC_D1       */
		{ CONF_GPMC_AD2,      (PIN_CFG_IEN  | PIN_CFG_PTU | PIN_CFG_EN | PIN_CFG_M0) }, /* GPMC_D2       */
		{ CONF_GPMC_AD3,      (PIN_CFG_IEN  | PIN_CFG_PTU | PIN_CFG_EN | PIN_CFG_M0) }, /* GPMC_D3       */
		{ CONF_GPMC_AD4,      (PIN_CFG_IEN  | PIN_CFG_PTU | PIN_CFG_EN | PIN_CFG_M0) }, /* GPMC_D4       */
		{ CONF_GPMC_AD5,      (PIN_CFG_IEN  | PIN_CFG_PTU | PIN_CFG_EN | PIN_CFG_M0) }, /* GPMC_D5       */
		{ CONF_GPMC_AD6,      (PIN_CFG_IEN  | PIN_CFG_PTU | PIN_CFG_EN | PIN_CFG_M0) }, /* GPMC_D6       */
		{ CONF_GPMC_AD7,      (PIN_CFG_IEN  | PIN_CFG_PTU | PIN_CFG_EN | PIN_CFG_M0) }, /* GPMC_D7       */
		{ CONF_GPMC_AD8,      (PIN_CFG_IEN  | PIN_CFG_PTU | PIN_CFG_EN | PIN_CFG_M0) }, /* GPMC_D8       */
		{ CONF_GPMC_AD9,      (PIN_CFG_IEN  | PIN_CFG_PTU | PIN_CFG_EN | PIN_CFG_M0) }, /* GPMC_D9       */
		{ CONF_GPMC_AD10,     (PIN_CFG_IEN  | PIN_CFG_PTU | PIN_CFG_EN | PIN_CFG_M0) }, /* GPMC_D10      */
		{ CONF_GPMC_AD11,     (PIN_CFG_IEN  | PIN_CFG_PTU | PIN_CFG_EN | PIN_CFG_M0) }, /* GPMC_D11      */
		{ CONF_GPMC_AD12,     (PIN_CFG_IEN  | PIN_CFG_PTU | PIN_CFG_EN | PIN_CFG_M0) }, /* GPMC_D12      */
		{ CONF_GPMC_AD13,     (PIN_CFG_IEN  | PIN_CFG_PTU | PIN_CFG_EN | PIN_CFG_M0) }, /* GPMC_D13      */
		{ CONF_GPMC_AD14,     (PIN_CFG_IEN  | PIN_CFG_PTU | PIN_CFG_EN | PIN_CFG_M0) }, /* GPMC_D14      */
		{ CONF_GPMC_AD15,     (PIN_CFG_IEN  | PIN_CFG_PTU | PIN_CFG_EN | PIN_CFG_M0) }, /* GPMC_D15      */

		{ CONF_GPMC_CSN2,     (PIN_CFG_IDIS | PIN_CFG_PTD | PIN_CFG_DIS | PIN_CFG_M0) }, /* GPMC_nCS0     */
//		{ CONF_GPMC_CSN3,     (PIN_CFG_IDIS | PIN_CFG_PTD | PIN_CFG_DIS | PIN_CFG_M0) }, /* GPMC_nCS0     */
//		{ CONF_GPMC_ADVN_ALE, (PIN_CFG_IDIS | PIN_CFG_PTD | PIN_CFG_DIS | PIN_CFG_M0) }, /* GPMC_nADV_ALE */
		{ CONF_GPMC_OEN_REN,  (PIN_CFG_IDIS | PIN_CFG_PTD | PIN_CFG_DIS | PIN_CFG_M0) }, /* GPMC_nOE      */
		{ CONF_GPMC_WEN,      (PIN_CFG_IDIS | PIN_CFG_PTD | PIN_CFG_DIS | PIN_CFG_M0) }, /* GPMC_nWE      */
		{ CONF_GPMC_BEN0_CLE, (PIN_CFG_IDIS | PIN_CFG_PTD | PIN_CFG_DIS | PIN_CFG_M0) }, /* GPMC_nBE0_CLE */
		{ CONF_GPMC_BEN1,     (PIN_CFG_IDIS | PIN_CFG_PTD | PIN_CFG_DIS | PIN_CFG_M0) }, /* GPMC_nBE1     */
		{ CONF_GPMC_WPN,      (PIN_CFG_IEN  | PIN_CFG_PTD | PIN_CFG_DIS | PIN_CFG_M0) }, /* GPMC_nWP      */
		{ CONF_GPMC_WAIT0,    (PIN_CFG_IEN  | PIN_CFG_PTD | PIN_CFG_DIS | PIN_CFG_M0) }, /* GPMC_WAIT0    */

	//	{ CONF_GPMC_NCS1,     (PIN_CFG_IDIS | PIN_CFG_PTD | PIN_CFG_DIS | PIN_CFG_M0) }, /* GPMC_nCS1     */
	//	{ CONF_GPMC_NCS2,     (PIN_CFG_IDIS | PIN_CFG_PTD | PIN_CFG_DIS | PIN_CFG_M0) }, /* GPMC_nCS2     */
	//	{ CONF_GPMC_NCS3,     (PIN_CFG_IDIS | PIN_CFG_PTD | PIN_CFG_DIS | PIN_CFG_M0) }, /* GPMC_nCS3     */
	//	{ CONF_GPMC_NCS4,     (PIN_CFG_IEN  | PIN_CFG_PTD | PIN_CFG_DIS | PIN_CFG_M0) }, /* GPMC_nCS4     */
	//	{ CONF_GPMC_NCS5,     (PIN_CFG_IDIS | PIN_CFG_PTD | PIN_CFG_DIS | PIN_CFG_M0) }, /* GPMC_nCS5     */
	//	{ CONF_GPMC_NCS6,     (PIN_CFG_IEN  | PIN_CFG_PTD | PIN_CFG_DIS | PIN_CFG_M0) }, /* GPMC_nCS6     */
	//	{ CONF_GPMC_NCS7,     (PIN_CFG_IEN  | PIN_CFG_PTD | PIN_CFG_DIS | PIN_CFG_M0) }, /* GPMC_nCS7     */
	//	{ CONF_GPMC_CLK,      (PIN_CFG_IDIS | PIN_CFG_PTD | PIN_CFG_DIS | PIN_CFG_M0) }, /* GPMC_CLK      */
	//	{ CONF_GPMC_WAIT1,    (PIN_CFG_IEN  | PIN_CFG_PTD | PIN_CFG_DIS | PIN_CFG_M0) }, /* GPMC_WAIT1    */
	//	{ CONF_GPMC_WAIT2,    (PIN_CFG_IEN  | PIN_CFG_PTD | PIN_CFG_DIS | M4) }, /* GPIO_64       */
	//	{ CONF_GPMC_WAIT3,    (PIN_CFG_IEN  | PIN_CFG_PTD | PIN_CFG_DIS | PIN_CFG_M0) }, /* GPMC_WAIT3    */
		{ 0xFFFFFFFF, 0xFFFFFFFF },
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
#define OEOFFTIME		(0x1C)		// = AccessTime + 1cycle

#define RDCYCLETIME		(0x1F)		// = AccessTime + 1cycle + tOEZ
#define CSRDOFFTIME		(0x1C)		// = AccessTime + 1 cycle
#define ADVRDOFFTIME	(0x00)		// = tAAVDS + tAVDP

#define WRCYCLETIME		(0x1F)		// = WeOffTime + AccessCompletion
#define CSWROFFTIME		(0x16)		// = WeOffTime + 1
#define ADVWROFFTIME	(0x00)		// = tAVSC + tAVDP
#define WEONTIME		(0x0A)		//   tCS
#define WEOFFTIME		(0x1C)		//   WeOffTime

/* CONFIG1 OPTIONS */
/* Enable Wait monitoring on reads, writes or both */
#define WAITREADMONITORING (1<<22)
#define WAITWRITEMONITORING (1<<21)
#define WAITMONITORING (WAITREADMONITORING|WAITWRITEMONITORING)
/* Wait monitoring time of 0, 1 or 2 extra cycles */
#define WAITMONITORINGTIME(N) (((N)&3)<<18)
/* Device size may be 16 or 8 */
#define DEVICESIZE(N) ((((N)/8)-1)<<12)
/* Double clock counts (halve effective rate) */
#define TIMEPARAGRANULARITY (1<<4)
/* Divides the GPMC main FCLK */
#define GPMCFCLKDIVIDER(N) ((N)&3)

static const uint32_t gpmc_config[] = {
	/* CONFIG1 */ WAITMONITORING | WAITMONITORINGTIME(2) | DEVICESIZE(16),
	/* CONFIG2 */ (CSWROFFTIME  << 16) | (CSRDOFFTIME  <<  8) | (CSONTIME  <<  0),
	/* CONFIG3 */ (ADVWROFFTIME << 16) | (ADVRDOFFTIME <<  8) | (ADVONTIME <<  0),
	/* CONFIG4 */ (WEOFFTIME << 24) | (WEONTIME << 20),
	/* CONFIG5 */ (ACCESSTIME << 16) | (WRCYCLETIME <<  8) | (RDCYCLETIME <<  0),
	/* CONFIG6 */ (ACCESSTIME << 24),
};

void InitGPMC(void) {
	// Enable pins
	config_mux(m68k_pin_mux);

	// Enable and wait for clock
	am335x_gpmc_init(16);

	// Set CONFIGx bits
	am335x_gpmc_enable_cs_config(gpmc_config, 2, 0x02000000, 0x01000000);
}

void TestGPMC(void) {
    const uint32_t base = (MMUEnabled) ? 0 : 0x2000000;
	
// [GMPC] $00000000: 1111:1111 f94e:4ef9 fc00:00fc d200:00d2
// [GMPC] $00000008: 0000:0000 ffff:ffff 2200:0022 0500:0005
// [GMPC] $00000010: 2200:0022 0200:0002 ffff:ffff ffff:ffff
// [GMPC] $00000018: 7865:6578 6365:6563 3320:2033 2e34:342e
	static uint16_t should_be[] = {
			0x1111, 0x4EF9, 0x00FC, 0x00D2,
			0x0000, 0xFFFF, 0x0022, 0x0005,
			0x0022, 0x0002, 0xFFFF, 0xFFFF,
			0x6578, 0x6563, 0x2033, 0x342E
	};

	char buffer[65] = { 0 };
	memcpy(buffer, (void*)(base + 24), 64);
	uint8_t len = 1 + strlen(buffer);
	bool byte_swapt = false;

	if(memcmp(buffer, "exec", 4)) {
		for(int i=0; i<len; i+=2) {
			char t = buffer[i];
			buffer[i] = buffer[i + 1];
			buffer[i + 1] = t;
		}
		byte_swapt = true;
	}

	printf("[GPMC] ROM Check: %s", buffer);
	if(byte_swapt) printf("[GPMC] Bytes swapped\n");
    else printf("[GPMC] Bytes NOT swapped\n");

    uint32_t i;
    printf("[GPMC] Raw bytes:");
    for(i=0; i<32; i++) {
		uint32_t addr = base + i;
        uint8_t read = *(uint8_t*)addr;
        if((i & 7) == 0) printf("\n[GMPC] $%08lX:", addr);
        printf(" %02x", read);
    }
    printf("\n[GPMC] Read words (LE:BE)");
    for(i=0; i<16; i++) {
		uint32_t addr = base + (i << 1);
        uint16_t read = *(uint16_t*)addr;
		uint16_t read_be = read_BE_word(addr);
        if((i & 3) == 0) printf("\n[GMPC] $%08lX:", addr);
        printf(" %04hx:%04hx", read, read_be);
		if(should_be[i] == read_be) printf(" "); else printf("*");
    }
    printf("\n[GPMC] Read longs (LE:BE)");
    for(i=0; i<8; i++) {
		uint32_t addr = base + (i << 2);
        uint32_t read = *(uint32_t*)addr;
        if((i & 1) == 0) printf("\n[GMPC] $%08lX:", addr);
        printf(" %08lx:%08lx", read, read_BE_long(addr));
    }
    printf("\n");

	// Speed check
    printf("[GPMC] Performing benchmark with ROM (~1.5s)\n");
    double startTimer = am335x_dmtimer1_get_time();
	volatile uint16_t r;
	for(i=0; i<1048576; i++) {
		r = *(uint16_t*)(base + ((i & 0xF) << 1));
	}
    double endTimer = am335x_dmtimer1_get_time();
    printf("[GPMC] Read %d words in %0.2f s (%0.2f MB/s)\n", 
		i, (endTimer - startTimer), 2.0 / (endTimer - startTimer));
	(void)r;

	// Speed check
    printf("[GPMC] Performing benchmark with CIA (~1.5s)\n");
    startTimer = am335x_dmtimer1_get_time();
	for(i=0; i<131072; i++) {
		r = *(uint16_t*)(base + ((i & 0xF) << 1));
	}
	endTimer = am335x_dmtimer1_get_time();
    printf("[GPMC] Read %d words in %0.2f s (%0.2f MB/s)\n", 
		i, (endTimer - startTimer), 0.25 / (endTimer - startTimer));
	(void)r;

    write_BE_word(base + 0xDFF036, 0xF0F0);
	am335x_dmtimer1_wait_us(2);
    printf("[GPMC] JOYxDAT Check: 0x%04hx 0x%04hx (should be close to 0xF0F0)\n",
		read_BE_word(base + 0xDFF00A), read_BE_word(base + 0xDFF00C));

    write_BE_word(base + 0xDFF036, 0xA5A5);
	am335x_dmtimer1_wait_us(2);
    printf("[GPMC] JOYxDAT Check: 0x%04hx 0x%04hx (should be close to 0xA5A5)\n",
		read_BE_word(base + 0xDFF00A), read_BE_word(base + 0xDFF00C));

    uint32_t start = 0, end = 0;
    uint32_t state = 1;
    uint32_t tod = 0;
    int elapsed;

    while(state) {
        switch(state) {
            case 1: case 3:
                tod  = read_BE_byte(base + 0xBFE801); tod <<= 8;
                tod |= read_BE_byte(base + 0xBFE901); tod <<= 8;
                tod |= read_BE_byte(base + 0xBFEA01);
                state++;
                break;

            case 2:
                start = tod;
                printf("[GPMC] CIAA timer A Start: 0x%08lx\n", start);
                am335x_dmtimer1_wait_us(100);
                state = 3;
                break;

            case 4:
                end = tod;
                printf("[GPMC] CIAA timer A End: 0x%08lx\n", end);
                elapsed = (int)end - (int)start;
                printf("[GPMC] CIAA timer A Elapsed: %d\n", elapsed);
                state = 0;
                break;
        } // end switch
    } // end while
}


