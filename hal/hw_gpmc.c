/*
 * test_gpmc.c
 *
 *  Created on: Apr. 18, 2021
 *      Author: renee.cousins
 */
#include "main.h"

/* Pin mux for m68k bus module */
static const pin_muxing_t m68k_pin_mux[] = {
		{ CONF_GPMC_A1,       (PIN_CFG_PDIS | PIN_CFG_M0) }, /*GPMC_A1        */
		{ CONF_GPMC_A2,       (PIN_CFG_PDIS | PIN_CFG_M0) }, /*GPMC_A2        */
		{ CONF_GPMC_A3,       (PIN_CFG_PDIS | PIN_CFG_M0) }, /*GPMC_A3        */
		{ CONF_GPMC_A4,       (PIN_CFG_PDIS | PIN_CFG_M0) }, /*GPMC_A4        */
		{ CONF_GPMC_A5,       (PIN_CFG_PDIS | PIN_CFG_M0) }, /*GPMC_A5        */
		{ CONF_GPMC_A6,       (PIN_CFG_PDIS | PIN_CFG_M0) }, /*GPMC_A6        */
		{ CONF_GPMC_A7,       (PIN_CFG_PDIS | PIN_CFG_M0) }, /*GPMC_A7        */
		{ CONF_GPMC_A8,       (PIN_CFG_PDIS | PIN_CFG_M0) }, /*GPMC_A8        */
		{ CONF_GPMC_A9,       (PIN_CFG_PDIS | PIN_CFG_M0) }, /*GPMC_A9        */
		{ CONF_GPMC_A10,      (PIN_CFG_PDIS | PIN_CFG_M0) }, /*GPMC_A10       */
		{ CONF_GPMC_A11,      (PIN_CFG_PDIS | PIN_CFG_M0) }, /*GPMC_A11       */

		{ CONF_LCD_DATA8,     (PIN_CFG_PDIS | PIN_CFG_M1) }, /* GPMC_A12      */
		{ CONF_LCD_DATA9,     (PIN_CFG_PDIS | PIN_CFG_M1) }, /* GPMC_A13      */
		{ CONF_LCD_DATA10,    (PIN_CFG_PDIS | PIN_CFG_M1) }, /* GPMC_A14      */
		{ CONF_LCD_DATA11,    (PIN_CFG_PDIS | PIN_CFG_M1) }, /* GPMC_A15      */
		{ CONF_LCD_DATA12,    (PIN_CFG_PDIS | PIN_CFG_M1) }, /* GPMC_A16      */
		{ CONF_LCD_DATA13,    (PIN_CFG_PDIS | PIN_CFG_M1) }, /* GPMC_A17      */
		{ CONF_LCD_DATA14,    (PIN_CFG_PDIS | PIN_CFG_M1) }, /* GPMC_A18      */
		{ CONF_LCD_DATA15,    (PIN_CFG_PDIS | PIN_CFG_M1) }, /* GPMC_A19      */

		{ CONF_MMC0_DAT3,     (PIN_CFG_PDIS | PIN_CFG_M1) }, /* GPMC_A20      */
		{ CONF_MMC0_DAT2,     (PIN_CFG_PDIS | PIN_CFG_M1) }, /* GPMC_A21      */
		{ CONF_MMC0_DAT1,     (PIN_CFG_PDIS | PIN_CFG_M1) }, /* GPMC_A22      */
		{ CONF_MMC0_DAT0,     (PIN_CFG_PDIS | PIN_CFG_M1) }, /* GPMC_A23      */
		{ CONF_MMC0_CLK,      (PIN_CFG_PDIS | PIN_CFG_M1) }, /* GPMC_A24      */
		{ CONF_MMC0_CMD,      (PIN_CFG_PDIS | PIN_CFG_M1) }, /* GPMC_A25/FCx  */

		{ CONF_GPMC_AD0,      (PIN_CFG_INEN  | PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_D0       */
		{ CONF_GPMC_AD1,      (PIN_CFG_INEN  | PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_D1       */
		{ CONF_GPMC_AD2,      (PIN_CFG_INEN  | PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_D2       */
		{ CONF_GPMC_AD3,      (PIN_CFG_INEN  | PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_D3       */
		{ CONF_GPMC_AD4,      (PIN_CFG_INEN  | PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_D4       */
		{ CONF_GPMC_AD5,      (PIN_CFG_INEN  | PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_D5       */
		{ CONF_GPMC_AD6,      (PIN_CFG_INEN  | PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_D6       */
		{ CONF_GPMC_AD7,      (PIN_CFG_INEN  | PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_D7       */
		{ CONF_GPMC_AD8,      (PIN_CFG_INEN  | PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_D8       */
		{ CONF_GPMC_AD9,      (PIN_CFG_INEN  | PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_D9       */
		{ CONF_GPMC_AD10,     (PIN_CFG_INEN  | PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_D10      */
		{ CONF_GPMC_AD11,     (PIN_CFG_INEN  | PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_D11      */
		{ CONF_GPMC_AD12,     (PIN_CFG_INEN  | PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_D12      */
		{ CONF_GPMC_AD13,     (PIN_CFG_INEN  | PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_D13      */
		{ CONF_GPMC_AD14,     (PIN_CFG_INEN  | PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_D14      */
		{ CONF_GPMC_AD15,     (PIN_CFG_INEN  | PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_D15      */

		{ CONF_GPMC_CSN2,     (PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_nCS0     */
//		{ CONF_GPMC_CSN3,     (PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_nCS0     */
//		{ CONF_GPMC_ADVN_ALE, (PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_nADV_ALE */
		{ CONF_GPMC_OEN_REN,  (PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_nOE      */
		{ CONF_GPMC_WEN,      (PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_nWE      */
		{ CONF_GPMC_BEN0_CLE, (PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_nBE0_CLE */
		{ CONF_GPMC_BEN1,     (PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_nBE1     */
		{ CONF_GPMC_WPN,      (PIN_CFG_INEN  | PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_nWP      */
		{ CONF_GPMC_WAIT0,    (PIN_CFG_INEN  | PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_WAIT0    */

	//	{ CONF_GPMC_NCS1,     (PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_nCS1     */
	//	{ CONF_GPMC_NCS2,     (PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_nCS2     */
	//	{ CONF_GPMC_NCS3,     (PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_nCS3     */
	//	{ CONF_GPMC_NCS4,     (PIN_CFG_IEN  | PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_nCS4     */
	//	{ CONF_GPMC_NCS5,     (PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_nCS5     */
	//	{ CONF_GPMC_NCS6,     (PIN_CFG_IEN  | PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_nCS6     */
	//	{ CONF_GPMC_NCS7,     (PIN_CFG_IEN  | PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_nCS7     */
	//	{ CONF_GPMC_CLK,      (PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_CLK      */
	//	{ CONF_GPMC_WAIT1,    (PIN_CFG_IEN  | PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_WAIT1    */
	//	{ CONF_GPMC_WAIT2,    (PIN_CFG_IEN  | PIN_CFG_PDIS | M4) }, /* GPIO_64       */
	//	{ CONF_GPMC_WAIT3,    (PIN_CFG_IEN  | PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_WAIT3    */
		{ 0xFFFFFFFF, 0xFFFFFFFF },
};

/**
                     REAL   CYCLE  EFFECTIVE MAIN CLOCK  FINAL  
    SYSTEM           CLOCK  TIME*   CLOCK     ADJUSTMENT  CLOCK  PRECISION
    ---------------- -----  ------  --------- ----------  -----  ---------
    Atari AdSPEED    16.04  12.00   16.67     96.3%       16.05  1.0005
    Amiga AdSPEED    14.32  13.00   15.38     93.1%       14.32  1.0003
    Amiga AdSPEED    14.19  14.00   14.29     99.3%       14.19  0.9999
    NEOGEO           12.00  16.00   12.50     96.0%       12.00  1.0000
    X68000           10.00  20.00   10.00     100.0%      10.00  1.0000
    Atari STe (NTSC) 8.05   24.00   8.33      96.6%       8.05   0.9995
    Atari (PAL)      8.02   24.00   8.33      96.3%       8.03   1.0005
    Atari ST (NTSC)  8.01   24.00   8.33      96.1%       8.01   0.9997
    Apple Mac        7.83   25.00   8.00      97.9%       7.83   0.9998
    SEGA Genesis     7.67   26.00   7.69      99.7%       7.67   0.9999
    SEGA Mega Drive  7.60   26.00   7.69      98.8%       7.60   1.0000
    Amiga (NTSC)     7.16   27.00   7.41      96.6%       7.16   0.9995
    Amiga (PAL)      7.09   28.00   7.14      99.3%       7.09   0.9999
    Apple Lisa       5.00   31.00   6.45      77.5%       5.00   1.0000

    * This is the basic "window" that we need to use for the whole 68000
      bus cycle and encompases four natural bus clocks and the eight
      internal states. Cycle times of less than 16 may be imprecise.

    Each state centre (when we want to ideally transition a signal) is:

    For S0 to S7 (Sn), time is CYCLE_TIME * (n + 0.5) / 8

          |<--------------------->| Cycle Time
  
           S0 S1 S2 S3 S4 S5 S6 S7 S0 ...
          +--+--+--+--+--+--+--+--+--+

          |--------------------|          ACCESS TIME
          |-----------------------|       CYCLE TIME
              __    __    __    __    __
    CLK   |__|  |__|  |__|  |__|  |__|  |
           _______________________ ______
    A     X_______________________X______

    INPUT TO GREENPAK
          _______                ________
    nCS          \______________/         (ADDRESS STROBE)
          _______                ________
    nOE          \______________/         (READ INDICATION, HIGH DURING WRITE)
          +--+--+--+--+--+--+--+--+
           1  1  0  0  0  0  0  1
          _____________          ________
    nWE                \________/         (WRITE INDICATION, HIGH DURING READ)
          +--+--+--+--+--+--+--+--+
           1  1  1  1  0  0  0  1
           _______________________ ______
    nBEx  X_______________________X______ (BYTE ENABLES)

    OUTPUT FROM GREENPAK
          _______                ______
    DSn          \______________/       (DURING READ, UDS = BE0, LDS = BE1) 
          _____________          ______
    DSn                \________/       (DURING WRITE, UDS = BE0, LDS = BE1) 
          _____________________________
    RnW                                 (DURING READ)
          _______                ______
    RnW          \______________/       (DURING WRITE)


    RnW = !nOE    ______________
          _______/              \______ 


          READ CYCLE              WRITE CYCLE
          _______________________________                _ 
                  ______________         \______________/  DESIRED RnW
          _______/              \________________________  OUR RnW
                  *                       *                RnW SAMPLE TIMES

    UDS = nBEx | (nOE & nWE)
          READ CYCLE              WRITE CYCLE
          ____________________________________          _
                                              \________/   nWE
          _______                ________________________
                 \______________/                          nOE
          _______                _____________          _
                 \______________/             \________/   nWE & nOE

*/

#define WAITPINMONITORING_RW   (3 << 21)  // Monitor both reading & writing
#define WAITPINSEL_WAIT0       (0 << 16)  // Use WAIT0 pin
#define WAITMONITORTIME_0CLKS  (0 << 18)  // No offset cycles for WAIT
#define WAITMONITORTIME_1CLKS  (1 << 18)  // 
#define WAITMONITORTIME_2CLKS  (2 << 18)  // 
#define DEVICESIZE_16b         (1 << 12)  // We're using 16-bit bus
#define TIMEPARAGRANULARITY_x2 (1 << 4)   // CLK / 2

#define _STR(X) #X
#define STR(X) _STR(X)
#define ASSERT(TRUTH) if(!(TRUTH)) printf("GPMC SYNC VIOLATION: %s\n", #TRUTH)

// STOCK 68000 BUSTEST
// rom       $00F80000  readw     577.1 ns   normal       3.5 * 10^6 byte/s
// rom       $00F80000  readl    1011.1 ns   normal       4.0 * 10^6 byte/s
// rom       $00F80000  readm     904.0 ns   normal       4.4 * 10^6 byte/s

void InitGPMC(float bus_clock) {
    int cycle_time = 200.0f / bus_clock; // always round down here

    int core_pll = 0.5f + (1000.0f * bus_clock) / (200.0f / (float)cycle_time);
    InitCorePLL(core_pll);
    printf("[GPMC] Trimming Core PLL to: %d\n", core_pll);

    int state[8], extra[8];

    for(int i=0; i<8; i++) {
        int mid_point = (int)(cycle_time * (i + 0.5f) / 4.0f);
        state[i] = mid_point / 2;
        extra[i] = mid_point & 1;
    }

    // OLD
    // #define RDCYCLETIME		(0x1F)		// = AccessTime + 1cycle + tOEZ
    // #define WRCYCLETIME		(0x1F)		// = WeOffTime + AccessCompletion
    // #define ACCESSTIME		(0x19)		//   AccessTime
    // #define CSONTIME		    (0x04)		// 	 tCAS
    // #define CSRDOFFTIME		(0x1C)		// = AccessTime + 1 cycle
    // #define CSWROFFTIME		(0x16)		// = WeOffTime + 1
    // #define OEONTIME		    (0x13)		// 	 OeOnTime >= AdvRdOffTime
    // #define OEOFFTIME		(0x1C)		// = AccessTime + 1cycle
    // #define WEONTIME		    (0x0A)		//   tCS
    // #define WEOFFTIME		(0x1C)		//   WeOffTime
    // #define ADVONTIME		(0x00)		//   tAAVDS
    // #define ADVRDOFFTIME	    (0x00)		// = tAAVDS + tAVDP
    // #define ADVWROFFTIME	    (0x00)		// = tAVSC + tAVDP

    // NEW
    // [GPMC] Cycle Time: 27
    // [GPMC] Access Time: 25
    // [GPMC] nCS Timing (ON/OFF): 7/25
    // [GPMC] nRE Timing (ON/OFF): 7/26
    // [GPMC] nWE Timing (ON/OFF): 15/26
    // [GPMC] ADV Timing (ON/OFF): 0/0

    // Total Cycle Time
    uint32_t RDCYCLETIME  = cycle_time;
    uint32_t WRCYCLETIME  = cycle_time;
    printf("[GPMC] Cycle Time: %d\n", RDCYCLETIME);

    // Point at which data should be valid
    uint32_t ACCESSTIME   = 5 + (cycle_time * 6) / 8;
    printf("[GPMC] Access Time: %d\n", ACCESSTIME);

    // Address Strobe
    uint32_t CSONTIME     = state[1] - 2;
    uint32_t CSRDOFFTIME  = state[7]; // CSRDOFFTIME < RDCYCLETIME
    uint32_t CSWROFFTIME  = state[7];
    uint32_t CSEXTRATIME  = 0; //extra[2];
    printf("[GPMC] nCS Timing (ON/OFF): %d/%d\n", CSONTIME, CSRDOFFTIME);

    // Read Cycle
    uint32_t OEONTIME     = 0;
    uint32_t OEOFFTIME    = state[7] + 1;
    uint32_t OEEXTRATIME  = 0; //extra[2];
    printf("[GPMC] nRE Timing (ON/OFF): %d/%d\n", OEONTIME, OEOFFTIME);

    // Write Cycle
    uint32_t WEONTIME     = state[1];
    uint32_t WEOFFTIME    = state[7] + 1;
    uint32_t WEEXTRATIME  = 0; //extra[4];
    printf("[GPMC] nWE Timing (ON/OFF): %d/%d\n", WEONTIME, WEOFFTIME);

    // Read and Write should mostly be the same here
    uint32_t ADVONTIME    = 0;
    uint32_t ADVRDOFFTIME = 0;
    uint32_t ADVWROFFTIME = 0;
    uint32_t ADVEXTRATIME = 0;
    printf("[GPMC] ADV Timing (ON/OFF): %d/%d\n", ADVONTIME, ADVRDOFFTIME);

    // All ON times must be <= 15
    ASSERT(CSONTIME < 16);
    ASSERT(OEONTIME < 16);
    ASSERT(WEONTIME < 16);

    // All OFF times must be <= 31
    ASSERT(CSRDOFFTIME < 32);
    ASSERT(CSWROFFTIME < 32);
    ASSERT(OEOFFTIME < 32);
    ASSERT(WEOFFTIME < 32);
    ASSERT(ADVRDOFFTIME < 32);
    ASSERT(ADVWROFFTIME < 32);



    // CSRDOFFTIME < RDCYCLETIME
    ASSERT(CSRDOFFTIME < RDCYCLETIME);
    // Sync Read: Rule 3. (RDACCESSTIME – CLKACTIVATIONTIME) modulus (GPMCFCLKDIVIDER + 1) 
    // must be different from GPMCFCLKDIVIDER
    // ASSERT(((ACCESSTIME) % (GMPC_CLK_DIV + 1)) != GMPC_CLK_DIV);
    // Sync Read: Rule 8. RdCycleTime must be strictly greater than all the Off times 
    // of the control signals (OeOffTime CsRdOffTime, CsWrOffTime, AdvRdOffTime, AdvWrOffTime, WeOffTime),
    // plus the possible extra delays added (CSExtraDelay, AdvExtraDelay, WeExtraDelay, OeExtraDelay, CsExtraDelay).
    ASSERT(RDCYCLETIME > OEOFFTIME);
    ASSERT(RDCYCLETIME > CSRDOFFTIME);
    ASSERT(RDCYCLETIME > ADVRDOFFTIME);
    ASSERT(RDCYCLETIME > CSWROFFTIME);
    ASSERT(RDCYCLETIME > ADVWROFFTIME);
    ASSERT(RDCYCLETIME > WEOFFTIME);

    // CSWROFFTIME < WRCYCLETIME
    ASSERT(CSWROFFTIME < WRCYCLETIME);
    // Sync Write: Rule 6. If GPMCFCLKDIVIDER is greater than 0 and WAITWRITEMONITORING is enabled,
    // WEOFFTIME must be greater or equal to WRACCESSTIME+2. If GPMCFCLKDIVIDER is 0 and 
    // WAITWRITEMONITORING is enabled, WEOFFTIME must be greater than or equal to WRACCESSTIME+1
    ASSERT(WEOFFTIME >= (ACCESSTIME+1));
    
    // Sync Write: Rule 7. Regardless of WAITWRITEMONITORING and 
    // GPMCFCLKDIVIDER, WEOFFTIME and CSWROFFTIME must be greater than or equal to WRACCESSTIME+1
    ASSERT(WEOFFTIME >= (ACCESSTIME+1));
    ASSERT(CSWROFFTIME >= (ACCESSTIME+1));

    // Sync Write: Rule 8. WrCycleTime must be strictly greater than all the Off times of the control
    // signals (OeOffTime CsRdOffTime, CsWrOffTime, AdvRdOffTime, AdvWrOffTime, WeOffTime), 
    // plus the possible extra delays added (CSExtraDelay, AdvExtraDelay, WeExtraDelay, OeExtraDelay, CsExtraDelay).
    ASSERT(WRCYCLETIME > OEOFFTIME);
    ASSERT(WRCYCLETIME > CSRDOFFTIME);
    ASSERT(WRCYCLETIME > ADVRDOFFTIME);
    ASSERT(WRCYCLETIME > CSWROFFTIME);
    ASSERT(WRCYCLETIME > ADVWROFFTIME);
    ASSERT(WRCYCLETIME > WEOFFTIME);    

    uint32_t gpmc_config[] = { 0
        /* CONFIG1 */
                    | WAITPINMONITORING_RW
                    | WAITPINSEL_WAIT0
                    | WAITMONITORTIME_0CLKS 
                    | DEVICESIZE_16b 
                    | TIMEPARAGRANULARITY_x2
                    ,
        /* CONFIG2 */ (CSWROFFTIME  << 16) | (CSRDOFFTIME  <<  8) | (CSEXTRATIME << 7)  | (CSONTIME  <<  0),
        /* CONFIG3 */ (ADVWROFFTIME << 16) | (ADVRDOFFTIME <<  8) | (ADVEXTRATIME << 7) | (ADVONTIME <<  0),
        /* CONFIG4 */ (WEOFFTIME << 24)    | (WEEXTRATIME << 23)  | (WEONTIME << 16)    |
                      (OEOFFTIME << 8)     | (OEEXTRATIME << 7)   | (OEONTIME << 0),
        /* CONFIG5 */ (ACCESSTIME << 16)   | (WRCYCLETIME <<  8)  | (RDCYCLETIME << 0),
        /* CONFIG6 */ (ACCESSTIME << 24),
    };

	// Enable pins
	config_mux(m68k_pin_mux);

	// Enable and wait for clock
	GPMCInit();

	// Set CONFIGx bits
	GPMCConfig(gpmc_config, 2, 0x02000000, 0x01000000);
}

uint32_t count_diff_bits(uint16_t a, uint16_t b) {
    uint16_t c     = a ^ b;
    uint32_t count = 0;
    while (c) {
        count++;
        c &= c - 1;  // clear least significant bit
    }
    return count;
 }

void TestGPMC(void) {
	const uint32_t base = (MMUEnabled) ? 0 : 0x2000000;
    char option[4] = { 0 };
    uint32_t i;

    while(option[0] != 'a' && option[0] != 'A' && option[0] != 'x' && option[0] != 'X') {
        printf("%s",
            "1. Endian test\n"
            "2. Dump first 16 words of ROM\n"
            "3. ROM speed test\n"
            "4. CIA speed test\n"
            "5. Agnus read/write\n"
            "6. CIA read/write\n"
            "7. Chip RAM read/write\n"
            "A. Perform all tests and exit\n"
            "X. Exit to main menu\n"
            "] "
        );
        gets(option);
        bool all = (option[0] == 'a' || option[0] == 'A');
        bool allpassed = true;

        if(option[0] == '1' || all) {
            char buffer[65] = { 0 };

            (void)*(volatile uint16_t*)0xD00000;

            *(uint8_t*)0xBFE201 |= 0x03; // Set low two bits for output.
            *(uint8_t*)0xBFE001 |= 0x01; // Set boot ROM on

            memcpy(buffer, (void*)(base + 24), 64);
            uint8_t len = 1 + strlen(buffer);
            bool byte_swapt = false;

            if(memcmp(buffer, "exec", 4)) {
                for(i=0; i<len; i+=2) {
                    char t = buffer[i];
                    buffer[i] = buffer[i + 1];
                    buffer[i + 1] = t;
                }
                byte_swapt = true;
            }

            printf("[GPMC] Endian Check: %s", buffer);
            if(byte_swapt) printf("[GPMC] Bytes swapped\n");
            else printf("[GPMC] Bytes NOT swapped\n");
            allpassed &= !byte_swapt;
        }

        if(option[0] == '2' || all) {
            static uint16_t should_be[] = {
                    0x1111, 0x4EF9, 0x00FC, 0x00D2,
                    0x0000, 0xFFFF, 0x0022, 0x0005,
                    0x0022, 0x0002, 0xFFFF, 0xFFFF,
                    0x6578, 0x6563, 0x2033, 0x342E
            };	

            printf("[GPMC] Read words (LE:BE)");

            (void)*(volatile uint16_t*)0xD00000;

            *(uint8_t*)0xBFE201 |= 0x03; // Set low two bits for output.
            *(uint8_t*)0xBFE001 |= 0x01; // Set boot ROM on

            bool passed = true;
            bool match = true;
            for(i=0; i<16; i++) {
                uint32_t addr = base + (i << 1);
                uint16_t read = *(uint16_t*)addr;
                uint16_t read_be = read_BE_word(addr);
                if((i & 3) == 0) printf("\n[GMPC] $%08lX:", addr);
                printf(" %04hx:%04hx", read, read_be);
                if(should_be[i] == read_be) printf(" "); else printf("*");
                match &= should_be[i] == read_be;
                passed &= __builtin_bswap16(read_be) == read;
            }
            printf("\n[GPMC] Test %sed and %s\n", passed ? "pass" : "fail", match ? "matches" : "differs");
            allpassed &= passed && match;
        }

        if(option[0] == '3' || all) {
            // Speed check
            // ROM operates at 1/4 bus cycles, speed should be 1.79MHz
            // This should mean about 3.58MB/s or that 2MB should be read in 0.559s
            printf("[GPMC] Performing benchmark with ROM\n");

            (void)*(volatile uint16_t*)0xD00000;

            *(uint8_t*)0xBFE201 |= 0x03; // Set low two bits for output.
            *(uint8_t*)0xBFE001 |= 0x01; // Set boot ROM on

            double startTimer = ReadDMTimerSeconds();
            volatile uint16_t r;
            for(i=0; i<1048576; i++) r = *(uint16_t*)(base + ((i & 0xF) << 1));
            double endTimer = ReadDMTimerSeconds();
            double timeTaken = endTimer - startTimer;
            printf("[GPMC] Read %ld words in %0.2f s, expecting 0.559s\n", i, timeTaken);
            printf("[GPMC] Read ROM at %0.2f MB/s, expecting 3.58MB/s\n", 2.0 / timeTaken);
            bool passed = !(timeTaken < 0.447 || timeTaken > 0.999);
            printf("[GPMC] Test %sed\n",  passed ? "pass" : "fail");
            (void)r;
            allpassed &= passed;
        }

        if(option[0] == '4' || all) {
            // Speed check
            // CIA operates at 1/10 bus cycles +/- a lot more variability
            // This should mean about 716kB/s or that 256kB should should be read in 0.358s
            printf("[GPMC] Performing benchmark with CIA\n");
            double startTimer = ReadDMTimerSeconds();
            volatile uint16_t r;
            for (i = 0; i < 262144; i++) r = *(uint8_t*)(base + 0xBFE801 + ((i & 0xF) << 8));
            double endTimer  = ReadDMTimerSeconds();
            double timeTaken = endTimer - startTimer;
            printf("[GPMC] Read %ld words in %0.2f s, expecting 0.358 s\n", i, timeTaken);
            printf("[GPMC] Read CIA at %0.2f kB/s, expecting 716kB/s\n", 256.0 / timeTaken);
            bool passed = ~(timeTaken < 0.286 || timeTaken > 0.448);
            printf("[GPMC] Test %sed\n", passed ? "pass" : "fail");
            (void)r;
            allpassed &= passed;
        }

        if(option[0] == '5' || all) {
            uint16_t joy1, joy2, bitdiff = 5;
            bool passed;

            uint16_t deniseID = read_BE_word(base + 0xDFF07C);
            printf("[GPMC] DeniseID: 0x%04X (%s)\n", deniseID, (deniseID == 0xFC) ? "ECS" : "OCS");

            for (i = 0; i < 10; i++) {
                if (i) WaitMSDMTimer(1);
                write_BE_word(base + 0xDFF036, 0xA0A0);
                joy1 = read_BE_word(base + 0xDFF00A);
                joy2 = read_BE_word(base + 0xDFF00C);
                bitdiff += count_diff_bits(joy1, 0xA0A0) + count_diff_bits(joy2, 0xA0A0);
            }
            bitdiff /= 10, passed = bitdiff < 12;
            printf("[GPMC] JOYxDAT Check: 0x%04hx 0x%04hx (%d avg bit errors)\n", joy1, joy2, bitdiff);
            printf("[GPMC] Test %sed\n", passed ? "pass" : "fail");

            bitdiff = 5;
            for (i = 0; i < 10; i++) {
                if (i) WaitMSDMTimer(1);
                write_BE_word(base + 0xDFF036, 0x5050);
                joy1 = read_BE_word(base + 0xDFF00A);
                joy2 = read_BE_word(base + 0xDFF00C);
                bitdiff += count_diff_bits(joy1, 0x5050) + count_diff_bits(joy2, 0x5050);
            }
            bitdiff /= 10, passed = bitdiff < 12;
            printf("[GPMC] JOYxDAT Check: 0x%04hx 0x%04hx (%d avg bit errors)\n", joy1, joy2, bitdiff);
            printf("[GPMC] Test %sed\n", passed ? "pass" : "fail");
            allpassed &= passed;
        }

        if(option[0] == '6' || all) {
            uint32_t start = 0, end = 0;
            uint32_t state = 1;
            uint32_t tod = 0;
            int elapsed;
            bool passed = true;

            *(volatile uint8_t*)0xBFE201 = 3; // Set low two bits for output
            uint8_t readback = *(volatile uint8_t*)0xBFE201;
            printf("[GPMC] CIAA Read %s (%02X == 03)\n", (readback == 3) ? "ok" : "bad", readback);
            passed &= readback == 3;

            *(volatile uint8_t*)0xBFE201 = 0; // Set low two bits for output
            readback = *(volatile uint8_t*)0xBFE201;
            printf("[GPMC] CIAA Read %s (%02X == 00)\n", (readback == 0) ? "ok" : "bad", readback);
            passed &= readback == 0;

            // *(uint8_t*)BFE001 = 2; // Set boot ROM off, power light dim.

            // bchg      #1,BFE001         Flip the power light to bright.
            // and.b     #$FE,BFE001           Set power light to bright.

            printf("[GPMC] CIAA Timer test (should be 180 ticks)\n");
            *(uint8_t*)0xBFEA01 = 0x00;
            *(uint8_t*)0xBFE901 = 0x00;
            *(uint8_t*)0xBFE801 = 0x00;

            WaitUSDMTimer(100);

            while(state) {
                switch(state) {
                    case 1: case 3:
                        tod  = read_BE_byte(base + 0xBFEA01); tod <<= 8; // odd CIA
                        tod |= read_BE_byte(base + 0xBFE901); tod <<= 8;
                        tod |= read_BE_byte(base + 0xBFE801);
                        state++;
                        break;

                    case 2:
                        start = tod;
                        printf("[GPMC] CIAA timer A Start: 0x%08lx\n", start);
                        WaitMSDMTimer(3000);
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
        
            passed &= (elapsed > 177) && (elapsed < 183);
            printf("[GPMC] Test %sed\n", passed ? "pass" : "fail");
            allpassed &= passed;
        }

        if(option[0] == '7' || all) {
            //volatile uint8_t confirm;
            (void)*(volatile uint16_t*)0xD00000;

            *(uint8_t*)0xBFE201 |= 0x03; // Set low two bits for output.
            *(uint8_t*)0xBFE001 &= 0xFC; // Set boot ROM off, power light.

            WaitMSDMTimer(2);
            volatile int errors = 1;
            printf("[GPMC] Performing RAM test");
            #define TEST_LEN 200

            uint16_t buffer[TEST_LEN] = { 0 };
            for(i=0; i<TEST_LEN ; i++) {
                write_BE_word(131072 + i * 2, 0x0000);
                write_BE_word(131072 + i * 2, 0xFFFF);
            }

            errors = 0;

            for(i=0; i<TEST_LEN ; i++) {
                write_BE_word(131072 + i * 2, 0x0101 * (i & 0xFF));
                volatile uint16_t data[3] = { 0 };
                buffer[i] = read_BE_word(131072 + i * 2);
            }

            for(i=0; i<TEST_LEN; i++) {
                bool match = buffer[i] == (0x0101 * (i & 0xFF));
                if((i & 7) == 0) printf("\n[GMPC] $%08lX:", 131072 + i);
                printf(" %04hx%c", buffer[i], match ? ' ' : '*');
                if(!match) errors++;
            }

            if((i & 7) == 0) printf("\n");
            printf("[GPMC] Test %sed, %d errors, %0.1f%%\n",  (errors == 0) ? "pass" : "fail", errors, (100.0f * errors) / TEST_LEN);
            allpassed &= (errors == 0);
        }

        if(all) printf("[GPMC] All tests %sed\n", allpassed ? "pass" : "fail");
    }
}


