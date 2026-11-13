/*
 * payload.c -- PJIT, the payload the bootloader starts (platform/buffee, make bundle-<test>): an LZTB bundle read from the flash slot or received over XMODEM into DRAM
 * and unpacked to its link addresses at the top of the DDR (docs/buffee/BUFFEE_MAP.md; the image starts with its exception vectors at 0x9F600000, whose first word
 * branches to _start), then called like a function. It hands the CPU back when the Amiga is reset (it is started again after a recalibration) or on ESC x3.
 *
 * Copyright (c) 2020-2026 Renee Cousins, the Buffee Project - http://www.buffee.ca
 * Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (see LICENSE).
 */
#include "cp15.h"
#include "main.h"
#include "lzt.h"
#include "store.h"
#include "gpak.h"
#include "busclock.h"
#include "amiga.h"
#include "payload.h"

#define PJIT_SCRATCH  0x91000000u       /* the unpacker's scratch: the largest part (1 MB) + slack */
#define PJIT_IMAGE    0x9F600000u
#define PJIT_MAILBOX  ((volatile uint32_t*)0x9F5FFFF0u)  /* to the payload: 1 = this is a restart after an Amiga reset */
#define COPY_STEP     0x2000u           /* bytes of the Amiga's ROM copied between looks at ESC */

/* What the bootloader tells the payload (read by platform/buffee/buffee.c): the boot settings and whether the MapROM image is already in the DRAM. */
#define BOOTINFO ((volatile bootinfo_t*)0x9F5FFF00u)
#define BOOTINFO_MAGIC 0x544F4F42u      /* 'BOOT' */
#define MAPROM_STAGE   0x9FE00000u      /* the 1 MB behind guest $F00000-$FFFFFF (BUFFEE_MAPROM_PHYS); the image sits at its offset inside that megabyte */
typedef struct { uint32_t magic; uint16_t maprom_base_64k; uint8_t maprom_src, maprom_size_log2, ram_mode, boot_flags, rom_staged, pace; uint32_t rom_size; struct { uint16_t base_mb; uint8_t size_log2, flags; } region[4]; } bootinfo_t;     /* region[] at byte 16 */

/* The payload's return codes besides 0: 0x100 the Amiga was reset; 0x110 + k a test asks for one Buffee-side piece to be set up again (bit 0 the PRU, bit 1 the
   GreenPAK image, bit 2 the GPMC; tests/buffee/buffee_reroll.c, buffee_bus.c), then a recalibration. Either way the payload is started again. */
#define RET_AMIGA_RESET 0x100
#define RET_REROLL      0x110

static int pjit_loaded;

int StageROM(void) {
    config_t *c = &settings;
    bootinfo_t bi = { BOOTINFO_MAGIC, c->maprom_base_64k, c->maprom_src, c->maprom_size_log2, c->ram_mode,
                      c->boot_flags | BOOT_24BIT | (amiga_lanes == LANES_STRAIGHT ? BOOT_NOSWAP : 0), 0, c->pace, 0 };
    uint32_t base = (uint32_t)c->maprom_base_64k << 16, size = 1u << c->maprom_size_log2;
    uint32_t *sect = (uint32_t*)MAPROM_STAGE;
    uint8_t *dst = (uint8_t*)(MAPROM_STAGE + (base & 0xFFFFFu));
    uint32_t i, n = 0;
    double t0;
    int k, rc;

    for (k = 0; k < 4; k++) { bi.region[k].base_mb = c->mem_region[k].base_mb; bi.region[k].size_log2 = c->mem_region[k].size_log2; bi.region[k].flags = c->mem_region[k].flags; }
    if (c->maprom_src != MAPROM_OFF) {
        if ((base & 0xFFFFFu) + size > 0x100000u || size < 0x10000u) { printf("MapROM: base %06lX size %lu KB does not fit one megabyte\n", (unsigned long)base, (unsigned long)(size >> 10)); return -1; }
        t0 = ReadDMTimerSeconds();
        for (i = 0; i < 0x100000u / 4; i++) sect[i] = 0xFFFFFFFFu;                       /* an empty bus reads 0xFF */
        if (c->maprom_src == MAPROM_FLASH) {
            rc = StoreLoad(STORE_ROM_OFF, STORE_ROM_MAX, STORE_MAGIC_ROM, dst, &n, 0);
            if (rc) { printf("MapROM: the ROM image in the flash is %s\n", rc == STORE_BAD_CRC ? "damaged (CRC)" : "not there"); return -1; }
            bi.rom_size = n;
            info("MapROM: %lu bytes from the flash at %06lX", (unsigned long)n, (unsigned long)base);
        } else {
            for (i = 0; i < size; i += COPY_STEP) { AmigaCopy(dst + i, base + i, COPY_STEP); if (uart_break) return -1; }
            bi.rom_size = size;
            info("MapROM: %lu KB copied from the Amiga's ROM at %06lX", (unsigned long)(size >> 10), (unsigned long)base);
        }
        bi.rom_staged = 1;
        info(" in %0.3f s\n", ReadDMTimerSeconds() - t0);
    }
    CP15DCacheCleanFlush();
    *BOOTINFO = bi;
    CP15DCacheCleanFlush();
    return 0;
}

static int Unpack(uint32_t n) {
    double t0;
    int rc;

    lzt_neon_enable();
    t0 = ReadDMTimerSeconds();
    rc = lztb_load((const uint8_t*)RX_BUF, (uint8_t*)0, (uint8_t*)PJIT_SCRATCH);
    CP15DCacheCleanFlush();
    CP15ICacheFlush();
    if (rc) printf("PJIT: the bundle (%lu bytes) does not unpack (rc %d)\n", (unsigned long)n, rc);
    else info("PJIT: unpacked %lu bytes in %0.3f s\n", (unsigned long)n, ReadDMTimerSeconds() - t0);
    pjit_loaded = (rc == 0);
    return rc;
}

int PJITLoad(void) {
    uint32_t n = 0;
    int rc;

    pjit_loaded = 0;
    rc = StoreLoad(STORE_PJIT_OFF, STORE_PJIT_MAX, STORE_MAGIC_PJIT, (void*)RX_BUF, &n, 0);
    if (rc == STORE_ABSENT && uart_break) return -1;
    if (rc) { printf("PJIT: %s\n", rc == STORE_BAD_CRC ? "the bundle in the flash is damaged (CRC)" : "no bundle in the flash"); return -1; }
    return Unpack(n);
}

int PJITReceive(void) {
    uint32_t n;

    pjit_loaded = 0;
    n = LoadXMODEM(RX_BUF);
    return n ? Unpack(n) : -1;
}

void PJITRun(int clean_reset) {
    int ret, k;

    if (!pjit_loaded) { printf("PJIT is not loaded\n"); return; }
    *PJIT_MAILBOX = 0;
    if (clean_reset) {                                      /* the machine starts clean: Agnus, Paula, the CIAs and the overlay (it is on after a reset) */
        AssertReset(); WaitMSDMTimer(100); ReleaseReset();
        if (WaitResetReleased(100, NULL) < 0) return;
        ASNCalibrate();
    }
    while ((ret = ((int(*)(void))PJIT_IMAGE)()) == RET_AMIGA_RESET || (ret & ~0xF) == RET_REROLL) {
        if (ret == RET_AMIGA_RESET) {                       /* wait for the host to let RESET go, then recalibrate: the colour clock may have re-phased */
            if (WaitResetReleased(RESET_SETTLE_MS, NULL) < 0) break;
            info("\nAmiga reset: recalibrating\n");
        } else {
            k = ret & 0xF;
            if (k & 1) RestartPRU();
            if (k & 2) GPReloadImage();
            if (k & 4) InitGPMC((float)(settings.kHz * 0.001));
            printf("\nre-roll %d\n", k);
        }
        ASNCalibrate();
        *PJIT_MAILBOX = 1;
    }
    printf("PJIT returned %d\n", ret);
}

void PJITBoot(void) {
    if (PJITLoad() || StageROM()) return;
    PJITRun(1);
}
