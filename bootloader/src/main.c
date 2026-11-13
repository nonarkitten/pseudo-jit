/*
 * main.c -- the boot: bring the hardware up, run the self-test, start PJIT from the flash. Anything that stops the boot (ESC, a failed self-test, no good PJIT
 * bundle in the flash) leads to the menu (menu.c) instead. docs/buffee/BOOT_SEQUENCE.md describes the steps.
 *
 * A first boot (the EEPROM had no settings, or the menu asked for it) runs the full self-test and prints the whole report; the first boot that passes switches
 * both off and saves that. Later boots run the short test and print one line.
 *
 * Copyright (c) 2020-2026 Renee Cousins, the Buffee Project - http://www.buffee.ca
 * Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (see LICENSE).
 */
#include "cp15.h"
#include "main.h"
#include "irq.h"
#include "gpak.h"
#include "post.h"
#include "busclock.h"
#include "payload.h"
#include "amiga.h"

#define NAK     0x15
#define CANCEL  0x18

/* Core DPLL: DCO = 24 * mult / div MHz; 28 GPMC units (20 ns at 1000/24) must equal 4 CLK7 = 563.9 ns, so 993.4 MHz (952/23) measured to zero drift against the
   PAL bus clock on the bench (869/21 left +0.001 clk per cycle). -DCORE_PLL_MULT=1000 -DCORE_PLL_DIV=24 selects the plain 1 GHz clock. */
#ifndef CORE_PLL_MULT
#define CORE_PLL_MULT 952
#define CORE_PLL_DIV  23
#endif

const char* Version = STR(VERSION);
boot_t boot;

static const char* const eeprom_why[] = { "", "blank", "damaged (CRC)", "not found" };     /* by CFG_* */

static const pin_muxing_t reset_pins[] = {
    { PINMUX_CONF_UART1_RTSN,  (PIN_CFG_INEN | PIN_CFG_PTUP | PIN_CFG_M7) }, /* HALT gpio0_13 */
    { PINMUX_CONF_MCASP0_AXR0, (PIN_CFG_INEN | PIN_CFG_PTUP | PIN_CFG_M7) }, /* RESET gpio3_16 */
    { 0xFFFFFFFF, 0xFFFFFFFF },
};

static const pin_muxing_t i2c_pins[] = {
    { PINMUX_CONF_I2C0_SCL, (PIN_CFG_INEN | PIN_CFG_PTUP | PIN_CFG_M0) },
    { PINMUX_CONF_I2C0_SDA, (PIN_CFG_INEN | PIN_CFG_PTUP | PIN_CFG_M0) },
    { 0xFFFFFFFF, 0xFFFFFFFF },
};

static const char* banner =
"              ____ ______________\n"
"  ______     |    |   \\__    ___/\n"
"  \\  __ \\    |    |   |#|    |###\n"
"  |  |_) )\\__|    |   |#|    |#\n"
"  |   __/\\________|___|#|____|#\n"
"  |__|###  #############  #####\n"
"    ###\n\n";

uint8_t _getchar(void) { return UART0Read(); }
void _putchar(uint8_t ch) { UART0Write(ch); }

static void InitResetHalt(void) {
    InitGPIO();
    GPSupplyOn();                                               /* the GreenPAK's supply on a v0.8 (ball M1): driven, not the pad's pull-up */
    AssertReset();
    config_mux(reset_pins);
}

static void InitI2C(int bps) {
    I2C0Init(bps);
    config_mux(i2c_pins);
}

/* ESC (once is enough) at any point of the boot stops it at the menu. */
static int Interrupted(void) { UART0Ready(); return uart_esc_seen || uart_break; }

static void Fail(const char* what) {
    printf("Self-test: %s\n", what);
    boot.post = POST_FAILED;
}

int main(void) {
    int first, full, ready, stopped, good;

    InitResetHalt();
    InitDMTimer();
    InitPERPLL();
    InitL3L4Wakeup();
    InitUART0(115200);
    InitPRU();                                                  /* the PRU starts measuring the bus clock now */
    InitSPI(24000000);
    SetEcho(1);
    printf("%c%c%c%c\n", NAK, CANCEL, CANCEL, CANCEL);          /* ends an XMODEM transfer a terminal may still be offering */

    InitI2C(100000);
    boot.eeprom = LoadConfigEEPROM();
    first = settings.first_boot;
    full = first || settings.post_full;
    console_verbose = first || settings.boot_verbose;
    printTimeStamp = console_verbose;
    if (console_verbose) printf("%s", banner);
    printf("Buffee %s (%s %s): press ESC for the menu\n", Version, __DATE__, __TIME__);
    if (boot.eeprom != CFG_LOADED) info("EEPROM %s: default settings\n", eeprom_why[boot.eeprom & 3]);
    if (first) info("First boot: the full self-test and this report; later boots are short and quiet\n");

    boot.greenpak = DetectGP();
    if (!boot.greenpak) Fail("no GreenPAK");
    else AmigaFindLanes();                                      /* the board kind: the EEPROM's record, or (first boot) the GreenPAK supply test; the right design before the bus runs */
    boot.pmic = DetectPMIC();                                   /* the ARM's clock: settings.dpll_mul MHz with the PMIC and a good last boot, 300 MHz otherwise */
    if (boot.pmic) InitCorePLL(CORE_PLL_MULT, CORE_PLL_DIV);
    info("ARM at %d MHz\n", boot.pmic ? settings.dpll_mul : 300);

    InitDDR();
    if (DDRLineTest()) Fail("DDR");
    if (FlashIDTest()) Fail("SPI flash");

    boot.bus_mhz = BusClockMHz(full);
    if (boot.bus_mhz == 0.0) { Fail("bus clock"); boot.bus_mhz = 8.0; }      /* the menu still gets a bus to work with */
    else info("Hello %s!\n", HostName());

    StartEClock();
    boot.bus = boot.greenpak;                                   /* without the GreenPAK no bus cycle ends (nothing releases WAIT): the Amiga stays in reset */
    if (boot.bus) {
        InitGPMC((float)boot.bus_mhz);
        ReleaseReset();                                         /* the GreenPAK image, the PRU (E clock) and the bus pads are up: the Amiga's reset ends (its CIAs take writes ~200 ms later) */
    }
    InitMMU();
    if (boot.bus && WaitResetReleased(RESET_SETTLE_MS, Interrupted) < 0) boot.bus = 0;    /* a host may hold RESET longer (a MiniMig loads its Kickstart first) */
    if (boot.bus) AmigaCheckLanes();                            /* before the first write to the Amiga: the Kickstart's word decides when nothing else did */

    if (full && boot.post == POST_OK && !Interrupted() && DDRFullTest() == POST_FAILED) Fail("DDR");

    /* Load PJIT (and a ROM image from the flash) while the Amiga comes out of reset; a ROM copied through the Amiga's bus has to wait for the calibration. */
    ready = boot.post == POST_OK && !Interrupted() && PJITLoad() == 0;
    if (ready && settings.maprom_src != MAPROM_RAM) ready = StageROM() == 0;
    if (boot.bus) ASNCalibrate();
    InitIRQ();
    if (boot.bus) irq_reset_cb = ASNCalibrate;
    if (ready && settings.maprom_src == MAPROM_RAM) ready = !Interrupted() && StageROM() == 0;

    /* A boot that passed ends a first boot; the result decides the ARM's clock next time. The EEPROM is written only when something changed. */
    stopped = Interrupted();
    if (!stopped) {
        good = boot.post == POST_OK;
        if (good && first) { settings.first_boot = 0; settings.boot_verbose = 0; settings.is_dirty = 1; }
        if (settings.last_boot_good != good) { settings.last_boot_good = good; settings.is_dirty = 1; }
        if (settings.is_dirty && boot.eeprom != CFG_NO_EEPROM) SaveConfigEEPROM();
    }
    info("Boot took %0.3f s\n", ReadDMTimerSeconds());

    if (stopped) printf("ESC: stopped at the menu\n");
    else if (boot.post != POST_OK) printf("The self-test failed: stopped at the menu\n");
    else if (ready) {
        printf("Starting PJIT\n");
        printTimeStamp = false;
        PJITRun(0);                                             /* the machine is as the calibration left it (overlay on, DMA off): no second RESET */
    }
    Menu();
    return 0;
}
