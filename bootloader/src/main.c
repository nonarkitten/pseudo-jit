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
#include "request.h"

#define NAK     0x15
#define CANCEL  0x18

const char* Version = STR(VERSION);
boot_t boot;
log_t console_log;                                              /* everything printed, for a debugger: clib/init.s puts its address at 0x402F0424 */
uint32_t jtag_entry[2] = { 1, 0 };                              /* r0, r1 at the entry (clib/init.s; initialised, so it is in .data and not cleared with .bss) */

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
void _putchar(uint8_t ch) { console_log.buf[console_log.head++ % LOG_SIZE] = ch; UART0Write(ch); }

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
    int first, full, ready, stopped, good, jtag = jtag_entry[0] == JTAG_MAGIC;

    console_log.magic = LOG_MAGIC;
    console_log.size = LOG_SIZE;
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
    console_verbose = first || settings.boot_verbose || jtag;
    printTimeStamp = console_verbose;
    if (console_verbose) printf("%s", banner);
    printf("Buffee %s (%s %s): press ESC for the menu\n", Version, __DATE__, __TIME__);
    if (boot.eeprom != CFG_LOADED) info("EEPROM %s: default settings\n", eeprom_why[boot.eeprom & 3]);
    if (first) info("First boot: the full self-test and this report; later boots are short and quiet\n");
    if (jtag) info("Started by a debugger: the flash's PJIT is not started\n");

    boot.greenpak = DetectGP();
    if (!boot.greenpak) Fail("no GreenPAK");
    else AmigaFindLanes();                                      /* the board kind: the EEPROM's record, or (first boot) the GreenPAK supply test; the right design before the bus runs */
    boot.pmic = DetectPMIC();                                   /* the ARM's clock: settings.dpll_mul MHz with the PMIC and a good last boot, 300 MHz otherwise */
    info("ARM at %d MHz\n", boot.pmic ? settings.dpll_mul : 300);

    /* The bus clock. The core PLL stays as the ROM set it: L3F 200 MHz, which the PRU and (halved) the GPMC run on, and the GPMC takes as many 20 ns units as fit in the
       host's 4 clocks (GPMCUnits). Every other L3F tried on an A500+ (the PLL set for the host, 198.6-199.3 MHz; 222 MHz through M4; docs/HISTORY.md) made the Kickstart's
       start hang in a bus access on a third to three quarters of the boots, against about 1 in 40 at 200 MHz. The switches below set them again for the tests. */
    boot.bus_mhz = BusClockMHz(full, Interrupted);
    if (boot.bus_mhz == 0.0) Fail("bus clock");
    else {
        info("Hello %s!\n", HostName());
#if defined(CORE_M4)                                            /* -DCORE_M4=9: L3F = 2000 / CORE_M4 MHz, the divider changed on the fly with the DPLL locked */
#ifdef CORE_M4_VIA                                              /* -DCORE_M4_VIA=9: through that divider first (the same end state reached by two changes) */
        if (SetCoreM4(CORE_M4_VIA)) printf("M4 %d not acknowledged\n", CORE_M4_VIA);
#endif
        if (SetCoreM4(CORE_M4)) printf("M4 %d not acknowledged\n", CORE_M4);
#elif defined(CORE_PLL_HOST)                                    /* -DCORE_PLL_HOST: the DPLL relocked so that the units last the host's 4 clocks (CorePLLFor), after a good last boot */
        if (boot.pmic) {
            int units = GPMCUnits(boot.bus_mhz * 1000.0), mult, div;
            CorePLLFor(boot.bus_mhz * 1000.0, units, &mult, &div);
#ifdef CORE_PLL_M                                               /* -DCORE_PLL_M=952 -DCORE_PLL_N=23: a fixed M / N instead */
            mult = CORE_PLL_M; div = CORE_PLL_N;
#endif
            InitCorePLL(mult, div, 10);
        }
#endif
        {
            int units = GPMCUnits(boot.bus_mhz * 1000.0);
            info("L3F %.1f MHz: %d GPMC units = 4 bus clocks %+.2f%%\n", CoreM4kHz() / 1000.0, units, (1.0 - units * 4.0 / CoreM4kHz() * boot.bus_mhz * 1000.0 / 4.0) * 100.0);
        }
    }

    InitDDR();
    if (DDRLineTest()) Fail("DDR");
    if (FlashIDTest()) Fail("SPI flash");

    StartEClock();
    boot.bus = boot.greenpak && boot.bus_mhz != 0.0;            /* without the GreenPAK or the clock no bus cycle ends (nothing releases WAIT): the Amiga stays in reset */
    if (boot.bus) {
        InitGPMC((float)boot.bus_mhz);
        ReleaseReset();                                         /* the GreenPAK image, the PRU (E clock) and the bus pads are up: the Amiga's reset ends (its CIAs take writes ~200 ms later) */
    } else if (boot.greenpak) ReleaseReset();                   /* no usable clock: the bus stays off, but the host is not held in reset behind the menu (a MiniMig's controller gives up) */
    InitMMU();
    if (boot.bus && WaitResetReleased(RESET_SETTLE_MS, Interrupted) < 0) boot.bus = 0;    /* a host may hold RESET longer (a MiniMig loads its Kickstart first) */
    if (boot.bus) AmigaCheckLanes();                            /* before the first write to the Amiga: the Kickstart's word decides when nothing else did */

    if (full && boot.post == POST_OK && !Interrupted() && DDRFullTest() == POST_FAILED) Fail("DDR");

    /* Load PJIT (and a ROM image from the flash) while the Amiga comes out of reset; a ROM copied through the Amiga's bus has to wait for the calibration. */
    ready = boot.post == POST_OK && !Interrupted() && !jtag && PJITLoad() == 0;
    if (ready && settings.maprom_src != MAPROM_RAM) ready = StageROM() == 0;
    if (boot.bus) ASNCalibrate();
    InitIRQ();
    if (boot.bus) irq_reset_cb = ASNCalibrate;
    if (boot.bus && !settings.post_nobus && boot.post == POST_OK && !Interrupted() && AmigaBusTest()) Fail("the lines to the Amiga");
    if (ready && settings.maprom_src == MAPROM_RAM) ready = boot.post == POST_OK && !Interrupted() && StageROM() == 0;

    /* A boot that passed ends a first boot; the result decides the ARM's clock next time. The EEPROM is written only when something changed. */
    stopped = Interrupted();
    if (!stopped) {
        good = boot.post == POST_OK;
        if (good && first) { settings.first_boot = 0; settings.boot_verbose = 0; settings.is_dirty = 1; }
        if (settings.last_boot_good != good) { settings.last_boot_good = good; settings.is_dirty = 1; }
        if (settings.is_dirty && boot.eeprom != CFG_NO_EEPROM && !jtag) SaveConfigEEPROM();     /* under the debugger only a request writes */
    }
    info("Boot took %0.3f s\n", ReadDMTimerSeconds());

    if (jtag) JTAGService();
    else if (stopped) printf("ESC: stopped at the menu\n");
    else if (boot.post != POST_OK) printf("The self-test failed: stopped at the menu\n");
    else if (ready) {
        printf("Starting PJIT\n");
        printTimeStamp = false;
        PJITRun(0);                                             /* the machine is as the calibration left it (overlay on, DMA off): no second RESET */
    }
    Menu();
    return 0;
}
