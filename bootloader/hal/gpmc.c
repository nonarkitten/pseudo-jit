/*
 * gpmc.c -- the GPMC as the Amiga's bus: the 16 MB window, its timing, and the clock-phase calibration at boot.
 *
 * Copyright (c) 2021-2026 the Buffee Project - http://www.buffee.ca
 * Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (see LICENSE).
 */
#include "main.h"
#include "gpak.h"
#include "../greenpak/asn_regs.h"

#include "am335x_regs.h"

#define GPMC_SIZE_256M  0x0
#define GPMC_SIZE_128M  0x8
#define GPMC_SIZE_64M   0xC
#define GPMC_SIZE_32M   0xE
#define GPMC_SIZE_16M   0xF

/* Pin mux for m68k bus module */
static const pin_muxing_t m68k_pin_mux[] = {
		{ PINMUX_CONF_GPMC_A1,       (PIN_CFG_PDIS | PIN_CFG_M0) }, /*GPMC_A1        */
		{ PINMUX_CONF_GPMC_A2,       (PIN_CFG_PDIS | PIN_CFG_M0) }, /*GPMC_A2        */
		{ PINMUX_CONF_GPMC_A3,       (PIN_CFG_PDIS | PIN_CFG_M0) }, /*GPMC_A3        */
		{ PINMUX_CONF_GPMC_A4,       (PIN_CFG_PDIS | PIN_CFG_M0) }, /*GPMC_A4        */
		{ PINMUX_CONF_GPMC_A5,       (PIN_CFG_PDIS | PIN_CFG_M0) }, /*GPMC_A5        */
		{ PINMUX_CONF_GPMC_A6,       (PIN_CFG_PDIS | PIN_CFG_M0) }, /*GPMC_A6        */
		{ PINMUX_CONF_GPMC_A7,       (PIN_CFG_PDIS | PIN_CFG_M0) }, /*GPMC_A7        */
		{ PINMUX_CONF_GPMC_A8,       (PIN_CFG_PDIS | PIN_CFG_M0) }, /*GPMC_A8        */
		{ PINMUX_CONF_GPMC_A9,       (PIN_CFG_PDIS | PIN_CFG_M0) }, /*GPMC_A9        */
		{ PINMUX_CONF_GPMC_A10,      (PIN_CFG_PDIS | PIN_CFG_M0) }, /*GPMC_A10       */
		{ PINMUX_CONF_GPMC_A11,      (PIN_CFG_PDIS | PIN_CFG_M0) }, /*GPMC_A11       */

		{ PINMUX_CONF_LCD_DATA8,     (PIN_CFG_PDIS | PIN_CFG_M1) }, /* GPMC_A12      */
		{ PINMUX_CONF_LCD_DATA9,     (PIN_CFG_PDIS | PIN_CFG_M1) }, /* GPMC_A13      */
		{ PINMUX_CONF_LCD_DATA10,    (PIN_CFG_PDIS | PIN_CFG_M1) }, /* GPMC_A14      */
		{ PINMUX_CONF_LCD_DATA11,    (PIN_CFG_PDIS | PIN_CFG_M1) }, /* GPMC_A15      */
		{ PINMUX_CONF_LCD_DATA12,    (PIN_CFG_PDIS | PIN_CFG_M1) }, /* GPMC_A16      */
		{ PINMUX_CONF_LCD_DATA13,    (PIN_CFG_PDIS | PIN_CFG_M1) }, /* GPMC_A17      */
		{ PINMUX_CONF_LCD_DATA14,    (PIN_CFG_PDIS | PIN_CFG_M1) }, /* GPMC_A18      */
		{ PINMUX_CONF_LCD_DATA15,    (PIN_CFG_PDIS | PIN_CFG_M1) }, /* GPMC_A19      */

		{ PINMUX_CONF_MMC0_DAT3,     (PIN_CFG_PDIS | PIN_CFG_M1) }, /* GPMC_A20      */
		{ PINMUX_CONF_MMC0_DAT2,     (PIN_CFG_PDIS | PIN_CFG_M1) }, /* GPMC_A21      */
		{ PINMUX_CONF_MMC0_DAT1,     (PIN_CFG_PDIS | PIN_CFG_M1) }, /* GPMC_A22      */
		{ PINMUX_CONF_MMC0_DAT0,     (PIN_CFG_PDIS | PIN_CFG_M1) }, /* GPMC_A23      */

		{ PINMUX_CONF_GPMC_AD0,      (PIN_CFG_INEN  | PIN_CFG_PTUP | PIN_CFG_M0) }, /* GPMC_D0       */
		{ PINMUX_CONF_GPMC_AD1,      (PIN_CFG_INEN  | PIN_CFG_PTUP | PIN_CFG_M0) }, /* GPMC_D1       */
		{ PINMUX_CONF_GPMC_AD2,      (PIN_CFG_INEN  | PIN_CFG_PTUP | PIN_CFG_M0) }, /* GPMC_D2       */
		{ PINMUX_CONF_GPMC_AD3,      (PIN_CFG_INEN  | PIN_CFG_PTUP | PIN_CFG_M0) }, /* GPMC_D3       */
		{ PINMUX_CONF_GPMC_AD4,      (PIN_CFG_INEN  | PIN_CFG_PTUP | PIN_CFG_M0) }, /* GPMC_D4       */
		{ PINMUX_CONF_GPMC_AD5,      (PIN_CFG_INEN  | PIN_CFG_PTUP | PIN_CFG_M0) }, /* GPMC_D5       */
		{ PINMUX_CONF_GPMC_AD6,      (PIN_CFG_INEN  | PIN_CFG_PTUP | PIN_CFG_M0) }, /* GPMC_D6       */
		{ PINMUX_CONF_GPMC_AD7,      (PIN_CFG_INEN  | PIN_CFG_PTUP | PIN_CFG_M0) }, /* GPMC_D7       */
		{ PINMUX_CONF_GPMC_AD8,      (PIN_CFG_INEN  | PIN_CFG_PTUP | PIN_CFG_M0) }, /* GPMC_D8       */
		{ PINMUX_CONF_GPMC_AD9,      (PIN_CFG_INEN  | PIN_CFG_PTUP | PIN_CFG_M0) }, /* GPMC_D9       */
		{ PINMUX_CONF_GPMC_AD10,     (PIN_CFG_INEN  | PIN_CFG_PTUP | PIN_CFG_M0) }, /* GPMC_D10      */
		{ PINMUX_CONF_GPMC_AD11,     (PIN_CFG_INEN  | PIN_CFG_PTUP | PIN_CFG_M0) }, /* GPMC_D11      */
		{ PINMUX_CONF_GPMC_AD12,     (PIN_CFG_INEN  | PIN_CFG_PTUP | PIN_CFG_M0) }, /* GPMC_D12      */
		{ PINMUX_CONF_GPMC_AD13,     (PIN_CFG_INEN  | PIN_CFG_PTUP | PIN_CFG_M0) }, /* GPMC_D13      */
		{ PINMUX_CONF_GPMC_AD14,     (PIN_CFG_INEN  | PIN_CFG_PTUP | PIN_CFG_M0) }, /* GPMC_D14      */
		{ PINMUX_CONF_GPMC_AD15,     (PIN_CFG_INEN  | PIN_CFG_PTUP | PIN_CFG_M0) }, /* GPMC_D15      */

		{ PINMUX_CONF_GPMC_CSN2,     (PIN_CFG_INEN | PIN_CFG_PDIS | PIN_CFG_M7) }, /* GPMC_nCS: released, the GreenPAK drives ASn (pin 20) */
//		{ PINMUX_CONF_GPMC_CSN3,     (PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_nCS0     */
//		{ PINMUX_CONF_GPMC_ADVN_ALE, (PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_nADV_ALE */
		{ PINMUX_CONF_GPMC_OEN_REN,  (PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_nOE      */
		{ PINMUX_CONF_GPMC_WEN,      (PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_nWE      */
		{ PINMUX_CONF_GPMC_BEN0_CLE, (PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_nBE0_CLE */
		{ PINMUX_CONF_GPMC_BEN1,     (PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_nBE1     */
		{ PINMUX_CONF_GPMC_WPN,      (PIN_CFG_INEN | PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_nWP      */
		{ PINMUX_CONF_GPMC_WAIT0,    (PIN_CFG_INEN | PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_WAIT0    */

	//	{ PINMUX_CONF_GPMC_NCS1,     (PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_nCS1     */
	//	{ PINMUX_CONF_GPMC_NCS2,     (PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_nCS2     */
	//	{ PINMUX_CONF_GPMC_NCS3,     (PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_nCS3     */
	//	{ PINMUX_CONF_GPMC_NCS4,     (PIN_CFG_IEN  | PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_nCS4     */
	//	{ PINMUX_CONF_GPMC_NCS5,     (PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_nCS5     */
	//	{ PINMUX_CONF_GPMC_NCS6,     (PIN_CFG_IEN  | PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_nCS6     */
	//	{ PINMUX_CONF_GPMC_NCS7,     (PIN_CFG_IEN  | PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_nCS7     */
	// { PINMUX_CONF_GPMC_CLK,      (PIN_CFG_INEN | PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_CLK      */
	//	{ PINMUX_CONF_GPMC_WAIT1,    (PIN_CFG_IEN  | PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_WAIT1    */
	//	{ PINMUX_CONF_GPMC_WAIT2,    (PIN_CFG_IEN  | PIN_CFG_PDIS | M4) }, /* GPIO_64       */
	//	{ PINMUX_CONF_GPMC_WAIT3,    (PIN_CFG_IEN  | PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_WAIT3    */
		{ 0xFFFFFFFF, 0xFFFFFFFF },
};

void GPMCInit(void) {
    // CM_DIV_M4_DPLL_CORE->BIT.HSDIVIDER_CLKOUT1_DIV = clk_div;
    CM_PER_GPMC_CLKCTRL->BIT.MODULEMODE = 2;
    while(CM_PER_GPMC_CLKCTRL->BIT.IDLEST);
    CM_PER_ELM_CLKCTRL->BIT.MODULEMODE = 2;
    while(CM_PER_ELM_CLKCTRL->BIT.IDLEST);

    /* Reset GPMC */
    GPMC_SYSCONFIG->BIT.SOFTRESET = 1;
    while(!GPMC_SYSSTATUS->BIT.RESETDONE);

    /* isr's sources masked */
    GPMC_IRQENABLE->LONG = 0;
    /* timeout disable */
    GPMC_TIMEOUT_CONTROL->LONG = 0;
    /* wait polarity */
    GPMC_CONFIG->BIT.WAIT0PINPOLARITY = 1;
    /* disable idle */
    GPMC_SYSCONFIG->BIT.SIDLEMODE = 1;

    /*
     * Chip select 0 is left set up by the boot ROM, and its window overlaps our MPDB (both at 0x08000000): switch it off
     */
    GPMC_CONFIG7_0->LONG = 0;

    INIT_PRIORITY_0->BIT.HOST_ARM = 2;
}

void GPMCConfig(const uint32_t config[6], uint32_t cs, uint32_t base, uint32_t size) {
    volatile gpmc_config7_bits config7;

    // Take an actual size and convert it to an address mask
    config7.MASKADDRESS = (size > 0x08000000) ? GPMC_SIZE_256M :
                         ((size > 0x04000000) ? GPMC_SIZE_128M :
                         ((size > 0x02000000) ? GPMC_SIZE_64M  :
                         ((size > 0x01000000) ? GPMC_SIZE_32M  :
                                                GPMC_SIZE_16M)));

    // Ensure base aligns to the boundary
    config7.BASEADDRESS = (base >> 24) & (0x30 | config7.MASKADDRESS);

    // Prepare for enablement
    config7.CSVALID = 1;

    if(cs == 2) {
        GPMC_CONFIG7_2->LONG = 0;

        WaitUSDMTimer(100);

        GPMC_CONFIG1_2->LONG = config[0];
        GPMC_CONFIG2_2->LONG = config[1];
        GPMC_CONFIG3_2->LONG = config[2];
        GPMC_CONFIG4_2->LONG = config[3];
        GPMC_CONFIG5_2->LONG = config[4];
        GPMC_CONFIG6_2->LONG = config[5];
        GPMC_CONFIG7_2->BIT = config7;
    }

    GPMC_CONFIG->BIT.WAIT0PINPOLARITY = 0;

    GPMC_TIMEOUT_CONTROL->LONG = 0;         /* no timeout: it did not recover the GPMC state machine anyway, and a long DMA burst may hold an access longer than any value (docs/buffee/BUS_ARBITRATION.md) */
}

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

#define ASSERT(TRUTH) if(!(TRUTH)) printf("SYNC VIOLATION: %s\n", #TRUTH)

// STOCK 68000 BUSTEST
// rom       $00F80000  readw     577.1 ns   normal       3.5 * 10^6 byte/s
// rom       $00F80000  readl    1011.1 ns   normal       4.0 * 10^6 byte/s
// rom       $00F80000  readm     904.0 ns   normal       4.4 * 10^6 byte/s

typedef union {
    uint8_t data[8];
    struct {
        uint8_t CYCLETIME, ACCESSTIME;
        uint8_t CSONTIME, CSOFFTIME;
        uint8_t OEONTIME, OEOFFTIME;
        uint8_t WEONTIME, WEOFFTIME;
    };
} Timing_t __attribute__((aligned(1)));


Timing_t default_timing = { 0 };
Timing_t current_timing = { 0 };
// static bool enableRAM = false;

static uint8_t gpmc_c2c = 0;

static void SetGPMCTiming(Timing_t *t) {
    // Note CLKACTIVATIONTIME == 0
    // GPMCFCLKDIVIDER == 0

    // Rule 1. PAGEBURSTACCESSTIME must be a multiple of GPMCFCLKDIVIDER+1.
    // Since we don't use either, I don't think this is relevant
    // GPMCFCLKDIVIDER+1 is only 1 so there's no issue anyway

    // Rule 2. (RDCYCLETIME – CLKACTIVATIONTIME) must be a multiple of GPMCFCLKDIVIDER+1.
    // GPMCFCLKDIVIDER+1 is only 1 so there's no issue anyway

    // Rule 3. (RDACCESSTIME – CLKACTIVATIONTIME) modulus (GPMCFCLKDIVIDER + 1)
    //         must be different from GPMCFCLKDIVIDER
    // I'm not sure how this is satisfied. Anything / 1 has a remainder of 0 which
    // is equal to GPMCFCLKDIVIDER.


    uint32_t RDACCESSTIME = t->ACCESSTIME - t->CSONTIME + t->OEONTIME;
    uint32_t WRACCESSTIME = t->ACCESSTIME - t->CSONTIME + t->WEONTIME + 2;

    ASSERT(RDACCESSTIME < 32);
    ASSERT(WRACCESSTIME < 32);
    ASSERT(RDACCESSTIME < (t->OEOFFTIME));
    ASSERT(WRACCESSTIME < (t->WEOFFTIME));

    // All ON times must be <= 15
    ASSERT(t->CSONTIME < 16);
    ASSERT(t->OEONTIME < 16);
    ASSERT(t->WEONTIME < 16);

    // All OFF times must be <= 31
    ASSERT(t->CSOFFTIME < 32);
    ASSERT(t->OEOFFTIME < 32);
    ASSERT(t->WEOFFTIME < 32);

    ASSERT(t->CYCLETIME > t->CSOFFTIME);
    ASSERT(t->CYCLETIME > t->OEOFFTIME);
    ASSERT(t->CYCLETIME > t->WEOFFTIME);

    ASSERT(t->CSOFFTIME >= (t->ACCESSTIME+1));
    ASSERT(t->OEOFFTIME >= (t->ACCESSTIME+1));
    ASSERT(t->WEOFFTIME >= (t->ACCESSTIME+1));

    ASSERT(t->CYCLETIME > t->OEOFFTIME);

    uint32_t OEEXTRATIME = 0;
    uint32_t WEEXTRATIME = 0;
    uint32_t CSEXTRATIME = 0;

    uint32_t ADVWROFFTIME = 0;
    uint32_t ADVRDOFFTIME = 0;
    uint32_t ADVEXTRATIME = 0;
    uint32_t ADVONTIME = 0;

    uint32_t CYCLE2CYCLEDELAY = gpmc_c2c;   // 0 = off; else GPMC_FCLK cycles (0-15) between successive accesses (the stress test's 11th parameter)

    /* the GreenPAK ends write cycles, so the GPMC write cycle only has to outlast WE: a cycle time of WEOFF + 1 gives back-to-back writes of 6 CLK7 */
#define WRCYCLE(t) ((t)->WEOFFTIME + 1)
    uint32_t gpmc_config[] = { 0
        /* CONFIG1 */
                    | WAITPINMONITORING_RW
                    | WAITPINSEL_WAIT0
                    | WAITMONITORTIME_1CLKS 
                    | DEVICESIZE_16b 
                    | TIMEPARAGRANULARITY_x2
                    ,
        /* CONFIG2 */ (t->CSOFFTIME << 16) | (t->CSOFFTIME <<  8) | (CSEXTRATIME  <<  7) | (t->CSONTIME <<  0),
        /* CONFIG3 */ (ADVWROFFTIME << 16) | (ADVRDOFFTIME <<  8) | (ADVEXTRATIME <<  7) | (ADVONTIME   <<  0),
        /* CONFIG4 */ (t->WEOFFTIME << 24) | (WEEXTRATIME  << 23) | (t->WEONTIME  << 16) |
                      (t->OEOFFTIME <<  8) | (OEEXTRATIME  <<  7) | (t->OEONTIME  <<  0),
        /* CONFIG5 */ (RDACCESSTIME << 16) | (WRCYCLE(t) <<  8) | (t->CYCLETIME <<  0),
        /* CONFIG6 */ (WRACCESSTIME << 24) | (CYCLE2CYCLEDELAY ? ((CYCLE2CYCLEDELAY << 8) | (3 << 6)) : 0),
    };

	// Set CONFIGx bits
	GPMCConfig(gpmc_config, 2, 0x02000000, 0x01000000);

    // printf("Cycle Time: %d\n", t->CYCLETIME);
    // printf("Access Time: %d\n", t->ACCESSTIME);
    // printf("nCS Timing (ON/OFF): %d/%d\n", t->CSONTIME, t->CSOFFTIME);
    // printf("nRE Timing (ON/OFF): %d/%d\n", t->OEONTIME, t->OEOFFTIME);
    // printf("nWE Timing (ON/OFF): %d/%d\n", t->WEONTIME, t->WEOFFTIME);

    // PrintTiming(&default_timing);
}


void InitGPMC(float bus_clock) {
    int cycle_time = 200.0f / bus_clock; // + 0.5f;

    // Total Cycle Time
    default_timing.CYCLETIME  = cycle_time;

    // Point at which data should be valid
    default_timing.ACCESSTIME   = (14 * cycle_time + 14) / 28;

    // Address Strobe
    default_timing.CSONTIME     = ( 2 * cycle_time + 14) / 28;
    default_timing.CSOFFTIME    = (19 * cycle_time + 14) / 28;

    // Read Cycle
    default_timing.OEONTIME     = ( 0 * cycle_time + 14) / 28;
    default_timing.OEOFFTIME    = (17 * cycle_time + 14) / 28;

    // Write Cycle
    default_timing.WEONTIME     = ( 2 * cycle_time + 14) / 28;
    default_timing.WEOFFTIME    = (19 * cycle_time + 14) / 28;

    current_timing = default_timing;

	// Enable pins
	config_mux(m68k_pin_mux);
	GPEnableASn();     // the CS pad is released: the GreenPAK may drive ASn (idle high)

	// Enable and wait for clock
	GPMCInit();

    // Set CS
    SetGPMCTiming(&default_timing);

}


/* ASn clock phase: the GreenPAK lets ASn fall only on one phase of CLK7 (a toggle flop gates it); which phase is the one Agnus answers in like it does a 68000 is random per boot.
   Two independent signals decide, both read under each truth table: (1) a GreenPAK flop that records whether DTACKn was still high one clock after ASn fell (in the right phase every
   chip RAM read says yes, in the wrong one almost all say no: GPPhaseClass, 256 reads), (2) a pattern written to 512 words of chip RAM and read back (a few percent errors in the wrong phase).
   Score = flop 'no' count + data errors; the lower wins (A on a tie); if even the best is not clean the gate is switched off and the delay made long (slow, but right).
   (With the PRU loop that echoed ~70 ns late the flop alone was enough; a tight echo that is not inverted moves ASn half a clock early and the flop says 'right' in both phases: see pru/main_gp4.c.) */
void EnableRomOverlay();
void DisableRomOverlay();
#define CAL_WORDS 512
#ifndef CAL_FLOP
#define CAL_FLOP 32                                       /* class-flop samples per phase: each is an I2C read (0.42 ms); the wrong phase says "no" on ~98% of them, 256 cost 107 ms per phase */
#endif
static void ASNPhaseScore(uint8_t table, int* flop, int* data) {
    /* With the 68000 bus timing (gen_asn_image.py: AS on the clock's rising edge, DTACK ignored until S4) BOTH parity tables give correct data; the wrong one costs 8 CLK7 per bus cycle
       instead of 6. So the score is the time of CAL_READS chip RAM reads (in us, written to *flop) and the data errors. */
    volatile uint16_t* ram = (volatile uint16_t*)0x20000;
    GPSetPhase(table); WaitUSDMTimer(50);
    *flop = 0; *data = 0;
    for (int pass = 0; pass < 2; pass++) {                       /* two patterns: every bit goes both ways */
        uint16_t x = pass ? 0x5A3C : 0xA5C3;
        for (int k = 0; k < CAL_WORDS; k++) ram[k] = (uint16_t)(k * 0x9E37u) ^ x;
        for (int k = 0; k < CAL_WORDS; k++) if (ram[k] != (uint16_t)((uint16_t)(k * 0x9E37u) ^ x)) (*data)++;
    }
    double t0 = ReadDMTimerSeconds();
    for (int r = 0; r < 4; r++) for (int k = 0; k < CAL_WORDS; k++) (void)ram[k];
    *flop = (int)((ReadDMTimerSeconds() - t0) * 1e6);
}
/* Calibrating right after the power is switched on can fail on every word (scores 1024 / 1280: chip RAM does not answer, 9 s of waiting did not help) while the same calibration after the
   bootloader has pulsed the Amiga RESET (RunPJIT) is clean. A failed try therefore pulses RESET (masked: our own pulse), waits, disables the overlay again (a reset turns it on) and scores
   again, up to ASN_CAL_TRIES times, before the slow fallback; a clean result always puts the image's tap back (the GreenPAK RAM keeps the fallback's tap 3 across resets). */
#ifndef ASN_CAL_TRIES
#define ASN_CAL_TRIES 50                                  /* retries (20 ms apart, no RESET pulse) before the slow fallback */
#endif
void ASNCalibrate(void) {
    static const uint8_t phase[2] = { ASN_PHASE_A, ASN_PHASE_B };
    int flop[2], data[2], score[2], chosen = 0, clean = 0, tries = 0;
    /* For ~200 ms after the Amiga's reset is released the CIAs ignore writes (power-on: PRA reads FF, DDRA 00 whatever is written): the overlay would stay on, $20000 would read the ROM and every
       word of the data check would fail in BOTH phases. Wait until the overlay-off write sticks (read back), then score once. */
    for (double t0 = ReadDMTimerSeconds(); ; WaitMSDMTimer(5)) {
        DisableRomOverlay();
        if ((*(volatile uint8_t*)0xBFE201 & 3) == 3 && (*(volatile uint8_t*)0xBFE001 & 3) == 0) break;
        if (ReadDMTimerSeconds() - t0 > 1.0) break;              /* never: score anyway, the retries and the slow fallback follow */
    }
    do {
        if (tries) {
#ifdef ASN_CAL_PULSE
            AssertReset(); WaitMSDMTimer(100); ReleaseReset(); WaitMSDMTimer(100);       /* a retry with a RESET pulse (a 68000 sees its RESET once, at power-on): for experiments only */
#else
            WaitMSDMTimer(20);
#endif
        }
        tries++;
        DisableRomOverlay();                                     /* chip RAM at $20000 */
#ifdef ASN_CAL_TRYDIAG
        double tt0 = ReadDMTimerSeconds();
#endif
        ASNPhaseScore(phase[0], &flop[0], &data[0]);
#ifdef ASN_CAL_TRYDIAG
        double tt1 = ReadDMTimerSeconds();
#endif
        ASNPhaseScore(phase[1], &flop[1], &data[1]);
#ifdef ASN_CAL_TRYDIAG
        printf("cal try %d: starts @%.3fs, phase A scored in %.1f ms, phase B in %.1f ms\n", tries, tt0, (tt1 - tt0) * 1000.0, (ReadDMTimerSeconds() - tt1) * 1000.0);
#endif
#ifdef ASN_CAL_TRYDIAG
        { volatile uint16_t *r = (volatile uint16_t*)0x20000, *rom = (volatile uint16_t*)0xFA0000, *v = (volatile uint16_t*)0xDFF004;
          printf("cal try %d @%.3fs: A flop %d data %d, B flop %d data %d; CIAA PRA %02X DDRA %02X; $20000: %04X %04X, ROM@$FA0000: %04X %04X; VPOSR %04X %04X\n", tries, ReadDMTimerSeconds(), flop[0], data[0], flop[1], data[1],
                 *(volatile uint8_t*)0xBFE001, *(volatile uint8_t*)0xBFE201, r[0], r[1], rom[0], rom[1], v[0], v[0]); }
#endif
        score[0] = data[0] * 100000 + flop[0]; score[1] = data[1] * 100000 + flop[1];       /* errors first, then the faster table (us for 4 x CAL_WORDS reads) */
        chosen = score[1] < score[0] ? 1 : 0;
        clean = (data[chosen] == 0);
    } while (!clean && tries < ASN_CAL_TRIES);
#ifdef ASN_CAL_DIAG
    printf("ASn calibration: chosen %c after %d tries; read time of %d reads A %d us, B %d us; data errors A %d, B %d of %d\n", 'A' + chosen, tries, 4 * CAL_WORDS, flop[0], flop[1], data[0], data[1], 2 * CAL_WORDS);
#endif
    if (clean) GPSetPhase(phase[chosen]);
    WaitUSDMTimer(50);
    EnableRomOverlay();                                          /* the machine boots with the overlay on */
    if (clean) printf("ASn: GreenPAK drives ASn, clock phase %c%s\n", 'A' + chosen, tries > 1 ? " (after retries)" : "");
    else { GPSetPhase(ASN_PHASE_OFF); printf("ASn: no clean clock phase (data errors A %d, B %d) after %d tries, parity gate off\n", data[0], data[1], tries); }
}

void EnableRomOverlay() {
    *(uint8_t*)0xBFE201 |= 0x03; // Set low two bits for output.
    *(uint8_t*)0xBFE001 |= 0x01; // Set boot ROM on
}

void DisableRomOverlay() {
    *(uint8_t*)0xBFE201 |= 0x03; // Set low two bits for output.
    *(uint8_t*)0xBFE001 &= 0xFC; // Set boot ROM off, power light.
}

