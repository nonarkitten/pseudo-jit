/*
 * busclock.c -- the Amiga's bus clock. From its start the PRU counts CLK7 edges against its own cycle counter (L3F, 200 MHz) in
 * 5 ms windows (pru/main_gp4.c); the clock sets the GPMC timing and the pace governor's rate, and names the host. Then the PRU's main loop takes over and makes the E clock (CLK7 / 10) and the 6800-cycle handshake.
 *
 * Copyright (c) 2020-2026 Renee Cousins, the Buffee Project - http://www.buffee.ca
 * Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (see LICENSE).
 */
#include <string.h>
#include "main.h"
#include "busclock.h"

#define PRU_WINDOWS 8
#define E_DIVIDER   10
#define TERM_AT     8               /* PRU_TERMn is asserted when the E counter is this (6800 cycles): 5-9 read reliably, but a write needs its data and CS to last until E falls, which only 8 gives */
#define NSYSTEMS    (int)(sizeof systems / sizeof systems[0])

/* The PRU's shared RAM: [0] the E divider the ARM asks for (non-zero ends the measurement), [1] the divider the PRU runs with, [3] the E count at which PRU_TERMn is
   asserted, [6] the windows measured, [8 + 2i] / [9 + 2i] the cumulative CLK7 edges and PRU cycles at the end of window i. */
static volatile uint32_t* const smem = (volatile uint32_t*)0x4A310000;

/* Hosts by bus clock. The clock alone cannot tell a CDTV or a MiniMig from an A500 of the same video standard. */
static const struct { uint16_t kHz; const char* name; } systems[] = {
    { 16050, "Atari AdSPEED"    }, { 14320, "Amiga AdSPEED"    }, { 14190, "Amiga AdSPEED"   }, { 12000, "NEOGEO"       },
    { 10000, "X68000"           }, {  8050, "Atari STe (NTSC)" }, {  8030, "Atari (PAL)"     }, {  8010, "Atari ST (NTSC)" },
    {  7830, "Apple Mac"        }, {  7670, "SEGA Genesis"     }, {  7600, "SEGA Mega Drive" }, {  7160, "Amiga (NTSC)"  },
    {  7090, "Amiga (PAL)"      }, {  5000, "Apple Lisa"       },
};

/* The mean kHz over the first n windows (0 = no clock / not ready): n = 1 is the 5 ms look, 8 the 40 ms measurement. The PRU's cycles are L3F's, whatever the core PLL
   was set to when it counted them (the test builds of main.c change it only after the measurement). */
static double PRUClockKHz(int n) {
    uint32_t edges, cycles;
    int t;

    for (t = 0; smem[6] < (uint32_t)n && t < 100; t++) WaitMSDMTimer(1);
    if (smem[6] < (uint32_t)n) return 0.0;
    edges = smem[8 + 2 * (n - 1)];
    cycles = smem[9 + 2 * (n - 1)];
    return cycles ? edges * CoreM4kHz() / cycles : 0.0;
}

/* The spread of the per-window frequency over the 8 windows in ppm: a crystal is a few tens, a PLL-driven FPGA clock or a loose connection shows more. */
static int PRUClockSpreadPPM(void) {
    double lo = 1e9, hi = 0.0, f;
    uint32_t pe = 0, pc = 0, e, c;
    int i;

    if (smem[6] < PRU_WINDOWS) return -1;
    for (i = 0; i < PRU_WINDOWS; i++) {
        e = smem[8 + 2 * i];
        c = smem[9 + 2 * i];
        f = (e - pe) * 200000.0 / (c - pc);
        if (f < lo) lo = f;
        if (f > hi) hi = f;
        pe = e; pc = c;
    }
    return (int)((hi - lo) / lo * 1e6);
}

/* The nearest entry by clock, 1-based (0 = below the table): stored in the EEPROM as host_type next to the measured kHz. */
static int SystemIndex(uint16_t kHz) {
    int i;

    for (i = 0; i < NSYSTEMS - 1; i++) if (kHz > (systems[i].kHz + systems[i + 1].kHz) / 2) return i + 1;
    return 0;
}

const char* HostName(void) {
    int i = SystemIndex(settings.kHz);

    return i ? systems[i - 1].name : "Unknown";
}

int HostIsAmiga(void) {
    return memcmp(HostName(), "Amiga", 5) == 0;                     /* (every name has at least five characters) */
}

/* A host may start its clock late, and not cleanly: a MiniMig's CPU clock comes from its FPGA, which its controller loads from the SD card a second or two after
   power-on, and it can stop again for a moment while the controller sets the FPGA up. The PRU starts its windows at the first edge and stalls inside a window when
   the edges stop, so a clock that has just come is measured over all eight windows and taken only when it is in range and steady (spread under 2000 ppm); otherwise
   the PRU starts its measurement again. All of it within CLOCK_WAIT_S of the first look; stop() (ESC) gives up. 0: no usable clock. */
#define CLOCK_WAIT_S  10.0
#define STEADY_PPM    2000

static double SteadyBusClockKHz(int (*stop)(void), double t0) {
    double kHz;
    int spread, tries = 0;

    for (;;) {
        while (smem[6] == 0) {
            if (stop()) return 0.0;
            if (ReadDMTimerSeconds() - t0 > CLOCK_WAIT_S) { printf("No steady bus clock after %d s\n", (int)CLOCK_WAIT_S); return 0.0; }
        }
        kHz = PRUClockKHz(PRU_WINDOWS);
        spread = PRUClockSpreadPPM();
        if (kHz >= 5000.0 && kHz <= 16000.0 && spread >= 0 && spread < STEADY_PPM) {
            printf("Bus clock after %d ms%s\n", (int)((ReadDMTimerSeconds() - t0) * 1000.0), tries ? " (once it was steady)" : "");
            return kHz;
        }
        if (ReadDMTimerSeconds() - t0 > CLOCK_WAIT_S) { printf("No steady bus clock after %d s\n", (int)CLOCK_WAIT_S); return 0.0; }
        tries++;
        smem[0] = 0;                                                    /* (the PRU measures until the ARM asks for the E clock) */
        smem[6] = 0;
        PRUFullReset(PRU0);                                             /* the measurement starts again at the next first edge */
    }
}

/* With a stored host the 5 ms look decides: within 0.5% of the stored clock it is the same host and the stored value stays (no EEPROM write). Anything else (no host
   yet, a different clock, a full self-test, a clock that came late) takes the 40 ms measurement. The EEPROM keeps whole kHz, too coarse for the pace governor (one kHz is
   141 ppm at 7 MHz): the clock returned is the 40 ms measurement whenever it is there and agrees, the stored value otherwise. */
double BusClockMHz(int full, int (*stop)(void)) {
    double kHz, now5, t0 = ReadDMTimerSeconds();

    if (!settings.post_checkclk) {
        kHz = settings.kHz;
        info("Bus clock set to %0.3f MHz\n", kHz * 0.001);
    } else {
        while (smem[6] == 0 && ReadDMTimerSeconds() - t0 < 0.1) ;
        if (smem[6] == 0) {
            printf("No bus clock from the host yet: waiting up to %d s (ESC: the menu)\n", (int)CLOCK_WAIT_S);
            kHz = SteadyBusClockKHz(stop, t0);
            if (kHz == 0.0) return 0.0;
            info("Bus clock measured at %0.3f MHz (spread %d ppm)\n", kHz * 0.001, PRUClockSpreadPPM());
        } else if (now5 = PRUClockKHz(1), !full && settings.host_type && now5 > settings.kHz * 0.995 && now5 < settings.kHz * 1.005) {
            kHz = settings.kHz;
            info("Bus clock %0.3f MHz (5 ms check %0.3f, host unchanged)\n", kHz * 0.001, now5 * 0.001);
        } else {
            kHz = PRUClockKHz(PRU_WINDOWS);
            info("Bus clock measured at %0.3f MHz (spread %d ppm)\n", kHz * 0.001, PRUClockSpreadPPM());
        }
        if (kHz >= 5000.0 && kHz <= 16000.0 && (uint16_t)(kHz + 0.5) != settings.kHz) {
            settings.kHz = kHz + 0.5;
            settings.host_type = SystemIndex(settings.kHz);
            settings.is_dirty = 1;
        }
    }
    if (settings.post_checkclk && smem[6] >= PRU_WINDOWS) {
        double fine = PRUClockKHz(PRU_WINDOWS);
        if (fine > kHz * 0.995 && fine < kHz * 1.005) kHz = fine;
    }
    if (kHz < 5000.0 || kHz > 16000.0) {
        printf("Bus clock %0.3f MHz is outside 5-16 MHz\n", kHz * 0.001);
        return 0.0;
    }
    return kHz * 0.001;
}

void StartEClock(void) {
    int timeout;

    smem[3] = TERM_AT;
    smem[0] = E_DIVIDER;
    for (timeout = 5; smem[1] == 0 && timeout--; ) WaitMSDMTimer(100);
    if (smem[1] == 0) printf("E clock not running (PRU hung?)\n");
    else info("E clock divider %lu\n", (unsigned long)smem[1]);
}

void RestartPRU(void) {
    PRUFullReset(PRU0);
    WaitMSDMTimer(2);
    StartEClock();
}
