/*
 * test_gpmc.c
 *
 *  Created on: Apr. 18, 2021
 *      Author: renee.cousins
 */
#include "main.h"

#include "ioam3358.h"

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
		{ PINMUX_CONF_MMC0_CLK,      (PIN_CFG_PDIS | PIN_CFG_M1) }, /* GPMC_A24      */
		{ PINMUX_CONF_MMC0_CMD,      (PIN_CFG_PDIS | PIN_CFG_M1) }, /* GPMC_A25/FCx  */

		{ PINMUX_CONF_GPMC_AD0,      (PIN_CFG_INEN  | PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_D0       */
		{ PINMUX_CONF_GPMC_AD1,      (PIN_CFG_INEN  | PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_D1       */
		{ PINMUX_CONF_GPMC_AD2,      (PIN_CFG_INEN  | PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_D2       */
		{ PINMUX_CONF_GPMC_AD3,      (PIN_CFG_INEN  | PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_D3       */
		{ PINMUX_CONF_GPMC_AD4,      (PIN_CFG_INEN  | PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_D4       */
		{ PINMUX_CONF_GPMC_AD5,      (PIN_CFG_INEN  | PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_D5       */
		{ PINMUX_CONF_GPMC_AD6,      (PIN_CFG_INEN  | PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_D6       */
		{ PINMUX_CONF_GPMC_AD7,      (PIN_CFG_INEN  | PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_D7       */
		{ PINMUX_CONF_GPMC_AD8,      (PIN_CFG_INEN  | PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_D8       */
		{ PINMUX_CONF_GPMC_AD9,      (PIN_CFG_INEN  | PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_D9       */
		{ PINMUX_CONF_GPMC_AD10,     (PIN_CFG_INEN  | PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_D10      */
		{ PINMUX_CONF_GPMC_AD11,     (PIN_CFG_INEN  | PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_D11      */
		{ PINMUX_CONF_GPMC_AD12,     (PIN_CFG_INEN  | PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_D12      */
		{ PINMUX_CONF_GPMC_AD13,     (PIN_CFG_INEN  | PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_D13      */
		{ PINMUX_CONF_GPMC_AD14,     (PIN_CFG_INEN  | PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_D14      */
		{ PINMUX_CONF_GPMC_AD15,     (PIN_CFG_INEN  | PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_D15      */

		{ PINMUX_CONF_GPMC_CSN2,     (PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_nCS0     */
//		{ PINMUX_CONF_GPMC_CSN3,     (PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_nCS0     */
//		{ PINMUX_CONF_GPMC_ADVN_ALE, (PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_nADV_ALE */
		{ PINMUX_CONF_GPMC_OEN_REN,  (PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_nOE      */
		{ PINMUX_CONF_GPMC_WEN,      (PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_nWE      */
		{ PINMUX_CONF_GPMC_BEN0_CLE, (PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_nBE0_CLE */
		{ PINMUX_CONF_GPMC_BEN1,     (PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_nBE1     */
		{ PINMUX_CONF_GPMC_WPN,      (PIN_CFG_INEN  | PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_nWP      */
		{ PINMUX_CONF_GPMC_WAIT0,    (PIN_CFG_INEN  | PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_WAIT0    */

	//	{ PINMUX_CONF_GPMC_NCS1,     (PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_nCS1     */
	//	{ PINMUX_CONF_GPMC_NCS2,     (PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_nCS2     */
	//	{ PINMUX_CONF_GPMC_NCS3,     (PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_nCS3     */
	//	{ PINMUX_CONF_GPMC_NCS4,     (PIN_CFG_IEN  | PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_nCS4     */
	//	{ PINMUX_CONF_GPMC_NCS5,     (PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_nCS5     */
	//	{ PINMUX_CONF_GPMC_NCS6,     (PIN_CFG_IEN  | PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_nCS6     */
	//	{ PINMUX_CONF_GPMC_NCS7,     (PIN_CFG_IEN  | PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_nCS7     */
	//	{ PINMUX_CONF_GPMC_CLK,      (PIN_CFG_PDIS | PIN_CFG_M0) }, /* GPMC_CLK      */
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
     * Disable the GPMC0 config set by ROM code
     * It conflicts with our MPDB (both at 0x08000000)
     */
    GPMC_CONFIG7_0->LONG = 0;
}

void GPMCConfig(const uint32_t config[6], uint32_t cs, uint32_t base, uint32_t size) {
    volatile __gpmc_config7_bits config7;

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

    GPMC_TIMEOUT_CONTROL->BIT.TIMEOUTENABLE = 1;
    GPMC_TIMEOUT_CONTROL->BIT.TIMEOUTSTARTVALUE = 200;
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

#define ASSERT(TRUTH) if(!(TRUTH)) printf("[GPMC] SYNC VIOLATION: %s\n", #TRUTH)

// STOCK 68000 BUSTEST
// rom       $00F80000  readw     577.1 ns   normal       3.5 * 10^6 byte/s
// rom       $00F80000  readl    1011.1 ns   normal       4.0 * 10^6 byte/s
// rom       $00F80000  readm     904.0 ns   normal       4.4 * 10^6 byte/s

typedef struct {
    uint8_t CYCLETIME, ACCESSTIME;
    uint8_t CSONTIME, CSOFFTIME;
    uint8_t OEONTIME, OEOFFTIME;
    uint8_t WEONTIME, WEOFFTIME;
} Timing_t;

Timing_t default_timing = { 0 };
Timing_t current_timing = { 0 };
static bool enableRAM = false;

static void PrintCycle(uint8_t on, uint8_t off, uint8_t max) {
    int i;
    for(i=0; i<on; i++) putchar('_');
    for(; i<=off; i++) putchar(' ');
    for(; i<max; i++) putchar('_');
    putchar('\n');

    for(i=0; i<on; i++) putchar(' ');
    putchar('|');
    for(; i<off-1; i++) putchar('_');
    putchar('|');
    for(; i<max; i++) putchar(' ');
}

static void PrintMarker(uint8_t off, uint8_t max) {
    int i;
    for(i=0; i<off; i++) putchar(' ');
    putchar('*');
    for(;i<=max;i++) putchar(' ');
}

static void PrintTiming(Timing_t *t) {
    int i;
    printf("Timing Diagram:\n");

    for(i=0; i<t->CYCLETIME; i++) putchar(" 123"[i / 10]); putchar('\n');
    for(i=0; i<t->CYCLETIME; i++) putchar('0' + (i % 10)); putchar('\n');
    //PrintCycle(0, t->CYCLETIME); printf("  Cycle\n");
    //PrintCycle(0, t->ACCESSTIME); printf("  Access\n");
    PrintMarker(t->ACCESSTIME, t->CYCLETIME); printf("  Access\n");
    PrintCycle(t->CSONTIME, t->CSOFFTIME, t->CYCLETIME); printf("  AS\n");
    PrintMarker(t->CSONTIME+5, t->CYCLETIME); printf("  S4 Sync\n");
    PrintCycle(t->OEONTIME, t->OEOFFTIME, t->CYCLETIME); printf("  DS (Read)\n");
    PrintCycle(t->WEONTIME, t->WEOFFTIME, t->CYCLETIME); printf("  DS (Write)\n\n");
}

static void SetGPMCTiming(Timing_t *t) {
    // All ON times must be <= 15
    ASSERT(t->CSONTIME < 16);
    ASSERT(t->OEONTIME < 16);
    ASSERT(t->WEONTIME < 16);

    // All OFF times must be <= 31
    ASSERT(t->CSOFFTIME < 32);
    ASSERT(t->OEOFFTIME < 32);
    ASSERT(t->WEOFFTIME < 32);

    ASSERT(t->CSOFFTIME < t->CYCLETIME);

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

    uint32_t gpmc_config[] = { 0
        /* CONFIG1 */
                    | WAITPINMONITORING_RW
                    | WAITPINSEL_WAIT0
                    | WAITMONITORTIME_1CLKS 
                    | DEVICESIZE_16b 
                    | TIMEPARAGRANULARITY_x2
                    ,
        /* CONFIG2 */ (t->CSOFFTIME  << 16) | (t->CSOFFTIME  <<  8) | (CSEXTRATIME << 7)  | (t->CSONTIME  <<  0),
        /* CONFIG3 */ (ADVWROFFTIME << 16)  | (ADVRDOFFTIME <<  8)  | (ADVEXTRATIME << 7) | (ADVONTIME <<  0),
        /* CONFIG4 */ (t->WEOFFTIME << 24)  | (WEEXTRATIME << 23)   | (t->WEONTIME << 16) |
                      (t->OEOFFTIME << 8)   | (OEEXTRATIME << 7)    | (t->OEONTIME << 0),
        /* CONFIG5 */ (t->ACCESSTIME << 16) | (t->CYCLETIME <<  8)  | (t->CYCLETIME << 0),
        /* CONFIG6 */ (t->ACCESSTIME << 24),
    };

	// Set CONFIGx bits
	GPMCConfig(gpmc_config, 2, 0x02000000, 0x01000000);

    // printf("[GPMC] Cycle Time: %d\n", t->CYCLETIME);
    // printf("[GPMC] Access Time: %d\n", t->ACCESSTIME);
    // printf("[GPMC] nCS Timing (ON/OFF): %d/%d\n", t->CSONTIME, t->CSOFFTIME);
    // printf("[GPMC] nRE Timing (ON/OFF): %d/%d\n", t->OEONTIME, t->OEOFFTIME);
    // printf("[GPMC] nWE Timing (ON/OFF): %d/%d\n", t->WEONTIME, t->WEOFFTIME);

    // PrintTiming(&default_timing);
}

int core_pll = 1000;

void InitGPMC(float bus_clock) {
    int cycle_time = 200.0f / bus_clock; // always round down here

    // int _pll = 0.5f + (1000.0f * bus_clock) / (200.0f / (float)cycle_time);
    // if(_pll > 1000) _pll = 1000;
    // if(_pll != core_pll) {
    //     core_pll = _pll;
    //     printf("[GPMC] Trimming Core PLL to: %d\n", core_pll);
    //     InitCorePLL(core_pll);
    // }


    // Total Cycle Time
    default_timing.CYCLETIME  = cycle_time;

    // Point at which data should be valid
    default_timing.ACCESSTIME   = (22 * cycle_time + 13) /27;

    // Address Strobe
    default_timing.CSONTIME   = (8 * cycle_time + 13) /27;
    default_timing.CSOFFTIME  = (23 * cycle_time + 13) /27;

    // Read Cycle
    default_timing.OEONTIME     = (6 * cycle_time + 13) /27;
    default_timing.OEOFFTIME    = (22 * cycle_time + 13) /27;

    // Write Cycle
    default_timing.WEONTIME     = (6 * cycle_time + 13) /27;
    default_timing.WEOFFTIME    = (22 * cycle_time + 13) /27;

    current_timing = default_timing;

	// Enable pins
	config_mux(m68k_pin_mux);

	// Enable and wait for clock
	GPMCInit();

    // Set CS
    SetGPMCTiming(&default_timing);

}

static int Prompt(const char* out, uint8_t* value) {
    char option[4] = { 0 };
    int n = *value;
    printf(out, n);
    gets(option);
    if(option[0] == 0) return true;
    else if(option[0] < '0' || option[0] > '9') return false;
    else { *value = atoi(option); return true; }
}

static void ChangeGPMCTiming(void) {
    Timing_t t = current_timing;

    PrintTiming(&t);

    printf("[GPMC] Cycle Time: %d\n", t.CYCLETIME);
    printf("[GPMC] Access Time: %d\n", t.ACCESSTIME);
    printf("[GPMC] nCS Timing (ON/OFF): %d/%d\n", t.CSONTIME, t.CSOFFTIME);
    printf("[GPMC] nRE Timing (ON/OFF): %d/%d\n", t.OEONTIME, t.OEOFFTIME);
    printf("[GPMC] nWE Timing (ON/OFF): %d/%d\n", t.WEONTIME, t.WEOFFTIME);

    if(!Prompt("Set Cycle Time     (0-31) [%2d]: ", &t.CYCLETIME)) return;
    if(!Prompt("Set Access Time    (0-31) [%2d]: ", &t.ACCESSTIME)) return;
    if(!Prompt("Set AS On-Time     (0-15) [%2d]: ", &t.CSONTIME)) return;
    if(!Prompt("Set AS Off-Time    (0-31) [%2d]: ", &t.CSOFFTIME)) return;
    if(!Prompt("Set Read On-Time   (0-15) [%2d]: ", &t.OEONTIME)) return;
    if(!Prompt("Set Read Off-Time  (0-31) [%2d]: ", &t.OEOFFTIME)) return;
    if(!Prompt("Set Write On-Time  (0-15) [%2d]: ", &t.WEONTIME)) return;
    if(!Prompt("Set Write Off-Time (0-31) [%2d]: ", &t.WEOFFTIME)) return;

    current_timing = t;
    SetGPMCTiming(&current_timing);
    PrintTiming(&t);
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
            "D. Set GPMC timing to default\n"
            "T. Set GPMC timing\n"
            "X. Exit to main menu\n"
            "] "
        );
        gets(option);
        bool all = (option[0] == 'a' || option[0] == 'A');
        bool allpassed = true;

        if(option[0] == 'd' || option[0] == 'D') {
            current_timing = default_timing;
            SetGPMCTiming(&default_timing);
            continue;
        }

        if(option[0] == 't' || option[0] == 'T') {
            ChangeGPMCTiming();
            continue;
        }

        if(option[0] == '1' || all) {
            char buffer[65] = { 0 };

            if(enableRAM) {
                enableRAM = false;
                *(uint8_t*)0xBFE201 |= 0x03; // Set low two bits for output.
                *(uint8_t*)0xBFE001 |= 0x01; // Set boot ROM on
                WaitMSDMTimer(2);
            }

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

            if(enableRAM) {
                enableRAM = false;
                *(uint8_t*)0xBFE201 |= 0x03; // Set low two bits for output.
                *(uint8_t*)0xBFE001 |= 0x01; // Set boot ROM on
                WaitMSDMTimer(2);
            }

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

            if(enableRAM) {
                enableRAM = false;
                *(uint8_t*)0xBFE201 |= 0x03; // Set low two bits for output.
                *(uint8_t*)0xBFE001 |= 0x01; // Set boot ROM on
                WaitMSDMTimer(2);
            }

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
            if(!enableRAM) {
                enableRAM = true;
                (void)*(volatile uint16_t*)0xD00000;
                *(uint8_t*)0xBFE201 |= 0x03; // Set low two bits for output.
                *(uint8_t*)0xBFE001 &= 0xFC; // Set boot ROM off, power light.
                WaitMSDMTimer(2);
            }

            volatile int errors = 1;

            #define TEST_LEN 64
            uint16_t buffer[TEST_LEN] = { 0 };
            errors = 0;

            printf("[GPMC] Performing RAM test, pass 1");
            for(i=0; i<TEST_LEN ; i++) {
                write_BE_word(131072 + i * 2, 0x0101 * (i & 0xFF));
                asm("nop");
                buffer[i] = read_BE_word(131072 + i * 2);
            }
            for(i=0; i<TEST_LEN; i++) {
                bool match = buffer[i] == (0x0101 * (i & 0xFF));
                if((i & 7) == 0) printf("\n[GMPC] $%08lX:", 131072 + i * 2);
                printf(" %04hx%c", buffer[i], match ? ' ' : '*');
                if(!match) errors++;
            }

            if((i & 7) == 0) printf("\n");
            printf("[GPMC] Performing RAM test, pass 2 (re-read pass 1)");
            for(i=0; i<TEST_LEN ; i++) {
                buffer[i] = read_BE_word(131072 + i * 2);
            }
            for(i=0; i<TEST_LEN; i++) {
                bool match = buffer[i] == (0x0101 * (i & 0xFF));
                if((i & 7) == 0) printf("\n[GMPC] $%08lX:", 131072 + i * 2);
                printf(" %04hx%c", buffer[i], match ? ' ' : '*');
                if(!match) errors++;
            }

            if((i & 7) == 0) printf("\n");
            printf("[GPMC] Performing RAM test, pass 3 (clear, two-pass)");
            for(i=0; i<TEST_LEN ; i++) {
                write_BE_word(131072 + i * 2, 0x0000);
            }
            for(i=0; i<TEST_LEN ; i++) {
                buffer[i] = read_BE_word(131072 + i * 2);
            }
            for(i=0; i<TEST_LEN; i++) {
                bool match = buffer[i] == 0;
                if((i & 7) == 0) printf("\n[GMPC] $%08lX:", 131072 + i * 2);
                printf(" %04hx%c", buffer[i], match ? ' ' : '*');
                if(!match) errors++;
            }

            if((i & 7) == 0) printf("\n");
            printf("[GPMC] Performing RAM test, pass 4 (fill, one-pass)");
            for(i=0; i<TEST_LEN ; i++) {
                write_BE_word(131072 + i * 2, 0xFFFF);
                asm("nop");
                buffer[i] = read_BE_word(131072 + i * 2);
            }
            for(i=0; i<TEST_LEN; i++) {
                bool match = buffer[i] == 0xFFFF;
                if((i & 7) == 0) printf("\n[GMPC] $%08lX:", 131072 + i * 2);
                printf(" %04hx%c", buffer[i], match ? ' ' : '*');
                if(!match) errors++;
            }

            if((i & 7) == 0) printf("\n");
            printf("[GPMC] Test %sed, %d errors, %0.1f%%\n",  (errors == 0) ? "pass" : "fail", errors, (33.333f * errors) / TEST_LEN);
            allpassed &= (errors == 0);
        }

        if(all) printf("[GPMC] All tests %sed\n", allpassed ? "pass" : "fail");
    }
}


