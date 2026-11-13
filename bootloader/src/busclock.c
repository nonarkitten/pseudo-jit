/*
 * busclock.c -- the Amiga's bus clock. From its start the PRU counts CLK7 edges against its own 200 MHz cycle counter in 5 ms windows (pru/main_gp4.c); the clock
 * sets the GPMC timing and names the host. Then the PRU's main loop takes over and makes the E clock (CLK7 / 10) and the 6800-cycle handshake.
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

/* The mean kHz over the first n windows (0 = no clock / not ready): n = 1 is the 5 ms look, 8 the 40 ms measurement. */
static double PRUClockKHz(int n) {
    uint32_t edges, cycles;
    int t;

    for (t = 0; smem[6] < (uint32_t)n && t < 100; t++) WaitMSDMTimer(1);
    if (smem[6] < (uint32_t)n) return 0.0;
    edges = smem[8 + 2 * (n - 1)];
    cycles = smem[9 + 2 * (n - 1)];
    return cycles ? edges * 200000.0 / cycles : 0.0;
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

/* With a stored host the 5 ms look decides: within 0.5% of the stored clock it is the same host and the stored value stays (no EEPROM write, the same GPMC timing every
   boot). Anything else (no host yet, a different clock, a full self-test) takes the 40 ms measurement. */
double BusClockMHz(int full) {
    double kHz, now5;

    if (!settings.post_checkclk) {
        kHz = settings.kHz;
        info("Bus clock set to %0.3f MHz\n", kHz * 0.001);
    } else {
        now5 = PRUClockKHz(1);
        if (!full && settings.host_type && now5 > settings.kHz * 0.995 && now5 < settings.kHz * 1.005) {
            kHz = settings.kHz;
            info("Bus clock %0.3f MHz (5 ms check %0.3f, host unchanged)\n", kHz * 0.001, now5 * 0.001);
        } else {
            kHz = PRUClockKHz(PRU_WINDOWS);
            info("Bus clock measured at %0.3f MHz (spread %d ppm)\n", kHz * 0.001, PRUClockSpreadPPM());
            if (kHz >= 5000.0 && kHz <= 16000.0 && (uint16_t)(kHz + 0.5) != settings.kHz) {
                settings.kHz = kHz + 0.5;
                settings.host_type = SystemIndex(settings.kHz);
                settings.is_dirty = 1;
            }
        }
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
