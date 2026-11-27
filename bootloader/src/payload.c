/*
 * payload.c -- PJIT, the payload the bootloader starts (platform/buffee, make bundle-<test>): an LZTB bundle read from the flash slot or received over XMODEM into DRAM
 * and unpacked to its link addresses at the top of the DDR (docs/buffee/BUFFEE_MAP.md; the image starts with its exception vectors at 0x9F600000, whose first word
 * branches to _start), then called like a function. It hands the CPU back when the Amiga is reset (it is started again after a recalibration) or on ESC x3.
 *
 * Copyright (c) 2020-2026 Renee Cousins, the Buffee Project - http://www.buffee.ca
 * Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (see LICENSE).
 */
#include <string.h>
#include "cp15.h"
#include "main.h"
#include "lzt.h"
#include "store.h"
#include "gpak.h"
#include "busclock.h"
#include "amiga.h"
#include "payload.h"
#include "request.h"

#define PJIT_SCRATCH  0x91000000u       /* the unpacker's scratch: the largest part (1 MB) + slack */
#define PJIT_IMAGE    0x9F600000u
#define PJIT_MAILBOX  ((volatile uint32_t*)0x9F5FFFF0u)  /* to the payload: 1 = this is a restart after an Amiga reset */

/* What the bootloader tells the payload (read by platform/buffee/buffee.c): the boot settings and whether the MapROM image is already in the DRAM. */
#define BOOTINFO ((volatile bootinfo_t*)0x9F5FFF00u)
#define BOOTINFO_MAGIC 0x544F4F42u      /* 'BOOT' */
#define MAPROM_STAGE   0x9FE00000u      /* the 1 MB behind guest $F00000-$FFFFFF (BUFFEE_MAPROM_PHYS); the image sits at its offset inside that megabyte */
typedef struct { uint32_t magic; uint16_t maprom_base_64k; uint8_t maprom_src, maprom_size_log2, ram_mode, boot_flags, rom_staged, pace; uint32_t rom_size; struct { uint16_t base_mb; uint8_t size_log2, flags; } region[4]; uint32_t bus_hz;
                 uint8_t pjit_slot, slot_state[2], req_version; uint32_t slot_seq[2]; } bootinfo_t;
/* region[] at byte 16, bus_hz at 32; at 36 the PJIT slot that was started (0 backup, 1 update), each slot's state (SLOT_*) and the version of the request page (request.h)
   this bootloader serves when the payload returns RET_REQUEST; at 40 the slots' sequence numbers */

/* The payload's return codes besides 0: 0x100 the Amiga was reset; 0x110 + k a test asks for one Buffee-side piece to be set up again (bit 0 the PRU, bit 1 the
   GreenPAK image, bit 2 the GPMC; tests/buffee/buffee_reroll.c, buffee_bus.c), then a recalibration. Either way the payload is started again. */
#define RET_AMIGA_RESET 0x100
#define RET_REROLL      0x110
#define RET_REQUEST     0x120           /* the payload filled the request page (request.h): the bootloader serves it and resets the board */

static int pjit_loaded;
static const uint32_t slot_off[2] = { STORE_PJIT_OFF, STORE_PJIT_UPD };
static const char* const slot_name[2] = { "backup", "update" };
pjit_slots_t pjit_slots = { { SLOT_EMPTY, SLOT_EMPTY }, SLOT_NONE, { 0, 0 } };

/* Each PJIT slot's header (no CRC check): empty, good or marked bad, and its sequence number. */
void PJITScanSlots(void) {
    uint32_t seq;
    int k, rc;

    for (k = 0; k < 2; k++) {
        rc = StoreHeader(slot_off[k], STORE_PJIT_MAX, STORE_MAGIC_PJIT, 0, &seq);
        pjit_slots.state[k] = rc == STORE_OK ? SLOT_GOOD : rc == STORE_MARKED_BAD ? SLOT_BAD : SLOT_EMPTY;
        pjit_slots.seq[k] = rc == STORE_ABSENT ? 0 : seq;
    }
}

uint32_t PJITNextSeq(void) {
    PJITScanSlots();
    return (pjit_slots.seq[0] > pjit_slots.seq[1] ? pjit_slots.seq[0] : pjit_slots.seq[1]) + 1;
}

const char* PJITSlotName(int slot) { return slot_name[slot & 1]; }

int PJITStore(int slot, const void* src, uint32_t len) {
    uint32_t seq;

    if (len < 8 || *(const uint32_t*)src != 0x42545A4Cu) { printf("That is not a PJIT bundle (no LZTB header)\n"); return REQ_E_FORMAT; }
    seq = PJITNextSeq();
    printf("PJIT: into the %s slot as #%lu\n", slot_name[slot], (unsigned long)seq);
    return StoreWrite(slot_off[slot], STORE_PJIT_MAX, STORE_MAGIC_PJIT, src, len, seq);
}

void PJITEraseSlot(int slot) { StoreErase(slot_off[slot]); }

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

    bi.bus_hz = (uint32_t)(boot.bus_mhz * 1e6 + 0.5);
    bi.pjit_slot = pjit_slots.booted; bi.slot_state[0] = pjit_slots.state[0]; bi.slot_state[1] = pjit_slots.state[1]; bi.req_version = REQ_VERSION;
    bi.slot_seq[0] = pjit_slots.seq[0]; bi.slot_seq[1] = pjit_slots.seq[1];
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
            if (AmigaCopyROM(dst, base, size)) return -1;
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

/* The good slot with the higher sequence number first (the bundle written last), the other if that one fails. A slot whose bundle fails its CRC or does not unpack is
   marked bad, so that later boots go straight to the other. */
int PJITLoad(void) {
    uint32_t n = 0;
    int first, i, k, rc;

    pjit_loaded = 0;
    pjit_slots.booted = SLOT_NONE;
    PJITScanSlots();
    first = pjit_slots.state[1] == SLOT_GOOD && (pjit_slots.state[0] != SLOT_GOOD || pjit_slots.seq[1] > pjit_slots.seq[0]);
    for (i = 0; i < 2; i++) {
        k = i ? !first : first;
        if (pjit_slots.state[k] != SLOT_GOOD) continue;
        rc = StoreLoad(slot_off[k], STORE_PJIT_MAX, STORE_MAGIC_PJIT, (void*)RX_BUF, &n, 0);
        if (rc == STORE_ABSENT) { if (uart_break) return -1; continue; }
        if (rc == STORE_OK) {
            info("PJIT: the %s slot, #%lu\n", slot_name[k], (unsigned long)pjit_slots.seq[k]);
            if (Unpack(n) == 0) { pjit_slots.booted = k; return 0; }
        }
        StoreMarkBad(slot_off[k]);
        pjit_slots.state[k] = SLOT_BAD;
        printf("PJIT: the bundle in the %s slot is damaged (%s): marked bad\n", slot_name[k], rc == STORE_BAD_CRC ? "CRC" : "it does not unpack");
    }
    printf("PJIT: no good bundle in the flash\n");
    return -1;
}

int PJITReceive(void) {
    uint32_t n;

    pjit_loaded = 0;
    n = LoadXMODEM(RX_BUF);
    return n ? Unpack(n) : -1;
}

int PJITFromMemory(const void* src, uint32_t len) {
    if (len > PJIT_SCRATCH - RX_BUF) return REQ_E_RANGE;
    memmove((void*)RX_BUF, src, len);
    pjit_slots.booted = SLOT_NONE;
    return Unpack(len) ? REQ_E_FORMAT : 0;
}

void PJITRun(int clean_reset) {
    volatile request_t* r = (volatile request_t*)REQ_PAGE;
    int ret, k;

again:
    if (!pjit_loaded) { printf("PJIT is not loaded\n"); return; }
    *PJIT_MAILBOX = 0;
    uart_break = 0;                                         /* (a break from before this run is not one for its restarts) */
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
            if (k & 4) InitGPMC((float)boot.bus_mhz);
            printf("\nre-roll %d\n", k);
        }
        ASNCalibrate();
        if (uart_break) break;                              /* ESC x3 while the bus was being set up again: the menu, not another run */
        *PJIT_MAILBOX = 1;
    }
    printf("PJIT returned %d\n", ret);
    if (ret != RET_REQUEST) return;
    ret = ServeRequest(r, REQ_FROM_PAYLOAD);                /* a request from the Amiga */
    if (r->cmd == REQ_RUN_PJIT && ret == 0 && StageROM() == 0) { clean_reset = 1; goto again; }      /* a bundle to try: it runs from a clean Amiga reset */
    printf("Request %s (%d): resetting the board\n", ret ? "failed" : "done", ret);                  /* anything else ends with a start from the flash */
    UART0Flush();
    Reset();
}

void PJITBoot(void) {
    if (PJITLoad() || StageROM()) return;
    PJITRun(1);
}
