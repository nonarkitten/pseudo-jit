/*
 * amiga.c -- the bootloader's accesses to the Amiga's bus (the GPMC window, 16-bit device memory at guest address 0). The Beta boards meet the Amiga's data lines
 * straight; the byte-swapper board crosses D7:D0 with D15:D8. Through the swapper a little-endian halfword load sees the 68000's word byte-reversed and a byte
 * keeps its address; straight, the load sees the word as it is and a byte travels on the other lane, so it is written at address ^ 1 and the GreenPAK raises
 * the strobe of that lane (asn_image_noswap). Reads take the whole word, which needs no strobe choice.
 *
 * Which board this is (the board kind) is found once and kept in the EEPROM (LoadBoardKind, in its own record): at the first boot by the GreenPAK supply test
 * (GPPowerTest: a v0.8 or later powers the GreenPAK from ball M1, a v0.6 does not), before the bus runs and whatever the host. The Kickstart's JMP word, read once
 * the host lets RESET go, decides only when there is no answer yet, and questions the record only in an Amiga (by its clock) whose ROM reads the other way round.
 *
 * Copyright (c) 2026 the Buffee Project - http://www.buffee.ca
 * Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (see LICENSE).
 */
#include "main.h"
#include "gpak.h"
#include "amiga.h"
#include "../src/busclock.h"
#include "../src/request.h"

#define ROM_JMP        0x4EF9u          /* JMP abs.L: the second word of every Kickstart ROM, after the 0x1111 / 0x1114 marker */
#define DETECT_TRIES   50               /* 1 ms apart: the ROM answers as soon as the bus runs, the tries only cover a slow start */

static const uint32_t rom_probe[] = { 0xF80002u, 0xFC0002u };    /* a 512 KB ROM starts at $F80000, a 256 KB one at $FC0000 */

int amiga_lanes = LANES_SWAPPER;
int amiga_lanes_from = LANES_FROM_NONE;

static const char* const lanes_name[] = { "through the byte swapper", "straight (no byte swapper): bytes swapped in software" };
static const char* const lanes_from_name[] = { "assumed", "the board kind stored in the EEPROM", "the GreenPAK supply test", "the Kickstart ROM, this boot only" };

static inline uint16_t raw16(uint32_t a) { return *(volatile uint16_t*)(a & ~1u); }

uint16_t AmigaRead16(uint32_t a) {
    uint16_t w = raw16(a);
    return amiga_lanes == LANES_STRAIGHT ? w : __builtin_bswap16(w);
}

void AmigaWrite16(uint32_t a, uint16_t v) {
    *(volatile uint16_t*)(a & ~1u) = amiga_lanes == LANES_STRAIGHT ? v : __builtin_bswap16(v);
}

uint8_t AmigaRead8(uint32_t a) {
    uint16_t w = AmigaRead16(a);
    return (a & 1) ? (uint8_t)w : (uint8_t)(w >> 8);
}

void AmigaWrite8(uint32_t a, uint8_t v) {
    *(volatile uint8_t*)(amiga_lanes == LANES_STRAIGHT ? a ^ 1u : a) = v;
}

/* A Kickstart's 32-bit words, added with each carry folded back in, come to FFFFFFFF (a 256 KB one seen twice in 512 KB too). The sum does not change when words
   change places, so it misses a read from the wrong address; AmigaCopyROM compares two reads as well. An image that is not a Kickstart is not checked. */
int KickstartSumOK(const void* image, uint32_t bytes) {
    const uint8_t* p = (const uint8_t*)image;
    uint32_t sum = 0, w, i;

    if (bytes < 0x40000u || p[0] != 0x11 || (p[1] != 0x11 && p[1] != 0x14)) return 1;
    for (i = 0; i < bytes; i += 4) {
        w = (uint32_t)p[i] << 24 | (uint32_t)p[i + 1] << 16 | (uint32_t)p[i + 2] << 8 | p[i + 3];
        sum += w;
        if (sum < w) sum++;
    }
    return sum == 0xFFFFFFFFu;
}

uint32_t AmigaCompare(const void* copy, uint32_t src, uint32_t bytes) {
    const uint16_t* c = (const uint16_t*)copy;
    uint32_t i, bad = 0;
    uint16_t w;

    for (i = 0; i < bytes / 2; i++) {
        w = raw16(src + 2 * i);
        if (amiga_lanes == LANES_STRAIGHT) w = __builtin_bswap16(w);
        bad += w != c[i];
    }
    return bad;
}

/* A ROM read through the bus for MapROM or the flash: read, read again and compare, and check a Kickstart's sum; a few tries, each line of the console saying why. */
int AmigaCopyROM(void* dst, uint32_t src, uint32_t bytes) {
    const uint32_t step = 0x2000u;                                 /* between looks at ESC */
    uint32_t i, bad;
    int k;

    for (k = 0; k < ROM_COPY_TRIES; k++) {
        for (i = 0; i < bytes; i += step) { AmigaCopy((uint8_t*)dst + i, src + i, step); if (uart_break) return -2; }
        for (i = 0, bad = 0; i < bytes; i += step) { bad += AmigaCompare((uint8_t*)dst + i, src + i, step); if (uart_break) return -2; }
        if (bad) printf("ROM at %06lX: %lu words read differently twice: reading it again\n", (unsigned long)src, (unsigned long)bad);
        else if (!KickstartSumOK(dst, bytes)) printf("ROM at %06lX: a bad Kickstart checksum: reading it again\n", (unsigned long)src);
        else return 0;
    }
    printf("ROM at %06lX: no good copy in %d tries: the bus misreads\n", (unsigned long)src, ROM_COPY_TRIES);
    return -1;
}

void AmigaCopy(void* dst, uint32_t src, uint32_t bytes) {
    uint16_t* d = (uint16_t*)dst;
    uint32_t i;

    if (amiga_lanes == LANES_STRAIGHT) for (i = 0; i < bytes / 2; i++) d[i] = __builtin_bswap16(raw16(src + 2 * i));
    else for (i = 0; i < bytes / 2; i++) d[i] = raw16(src + 2 * i);        /* through the swapper the halfword is already in memory order */
}

/* The Kickstart's JMP word as the lanes deliver it: LANES_SWAPPER, LANES_STRAIGHT, or LANES_UNKNOWN (no Kickstart ROM at $F80000 or $FC0000). */
static int RomLanes(void) {
    uint16_t w;
    int t, k;

    for (t = 0; t < DETECT_TRIES; t++) {
        for (k = 0; k < (int)(sizeof rom_probe / sizeof rom_probe[0]); k++) {
            w = raw16(rom_probe[k]);
            if (w == __builtin_bswap16(ROM_JMP)) return LANES_SWAPPER;
            if (w == ROM_JMP) return LANES_STRAIGHT;
        }
        WaitMSDMTimer(1);
    }
    return LANES_UNKNOWN;
}

static void UseLanes(int lanes, int from) {
    amiga_lanes = lanes;
    amiga_lanes_from = from;
    GPUseImage(lanes == LANES_STRAIGHT);
}

static int StoreKind(int kind) {                                /* not under the debugger, where only a request writes: the next boot from the flash stores it */
    if (boot.eeprom == CFG_NO_EEPROM || jtag_entry[0] == JTAG_MAGIC || SaveBoardKind(kind)) return 0;
    info("Board kind stored in the EEPROM\n");
    return 1;
}

void AmigaFindLanes(void) {
    int kind = LoadBoardKind(), p;

    if (kind >= 0) UseLanes(kind, LANES_FROM_RECORD);
    else if ((p = GPPowerTest()) >= 0) {
        UseLanes(p ? LANES_SWAPPER : LANES_STRAIGHT, LANES_FROM_TEST);
        StoreKind(amiga_lanes);
    } else return;
    info("Data lines: %s (%s)\n", lanes_name[amiga_lanes], lanes_from_name[amiga_lanes_from]);
}

/* A record contradicted by an Amiga's own Kickstart makes the supply test run again, and its answer is stored. This boot follows the ROM, which shows how the lanes
   behave in this machine; a test that still disagrees with the ROM leaves that disagreement on the console. */
void AmigaCheckLanes(void) {
    int rom = RomLanes(), was = amiga_lanes, p;

    if (amiga_lanes_from == LANES_FROM_NONE) {
        if (rom == LANES_UNKNOWN) { printf("Data lines: no board kind and no Kickstart ROM at $F80000 or $FC0000; assuming the byte swapper\n"); return; }
        UseLanes(rom, LANES_FROM_ROM);
        info("Data lines: %s (%s)\n", lanes_name[rom], lanes_from_name[LANES_FROM_ROM]);
        return;
    }
    if (rom == LANES_UNKNOWN || rom == amiga_lanes || !HostIsAmiga()) return;
    printf("Data lines: the Kickstart reads %s, the board kind says %s: the GreenPAK supply test again\n", lanes_name[rom], lanes_name[was]);
    p = GPPowerTest();
    if (p >= 0 && (p ? LANES_SWAPPER : LANES_STRAIGHT) != was) StoreKind(p ? LANES_SWAPPER : LANES_STRAIGHT);
    if (p < 0 || (p ? LANES_SWAPPER : LANES_STRAIGHT) != rom) printf("Data lines: the supply test %s; this boot follows the ROM\n", p < 0 ? "cannot tell" : "agrees with the old record");
    UseLanes(rom, LANES_FROM_ROM);
}

int AmigaRetestLanes(void) {
    int p = GPPowerTest();

    if (p < 0) return -1;
    UseLanes(p ? LANES_SWAPPER : LANES_STRAIGHT, LANES_FROM_TEST);
    StoreKind(amiga_lanes);
    return amiga_lanes;
}

const char* AmigaLanesName(void) {
    return amiga_lanes_from == LANES_FROM_NONE ? "not known yet" : lanes_name[amiga_lanes];
}

const char* AmigaLanesFrom(void) {
    return lanes_from_name[amiga_lanes_from];
}
