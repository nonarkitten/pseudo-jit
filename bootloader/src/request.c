/*
 * request.c -- serving the request page (request.h): the JTAG installer's commands while the bootloader waits for them, and the one request a payload returns with.
 * Every request with data is checked before the flash is touched: the data lies in the DRAM's free pool, has the CRC the request gives and looks like what it claims.
 *
 * Copyright (c) 2026 the Buffee Project - http://www.buffee.ca
 * Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (see LICENSE).
 */
#include "cp15.h"
#include "main.h"
#include "store.h"
#include "payload.h"
#include "request.h"

#define POOL_START 0x80000000u          /* the guest's pool (docs/buffee/BUFFEE_MAP.md): free while the bootloader runs */
#define POOL_END   0x9F000000u
#define BUF_MAX    0x01000000u          /* RX_BUF up to the unpacker's scratch */

static const char* const cmd_name[] = { "none", "store PJIT", "store a ROM image", "store the bootloader", "copy the Amiga's ROM", "settings", "erase the update",
                                        "reboot", "status", "run a PJIT bundle", "wipe the flash and the EEPROM" };

static int DataOK(uint32_t addr, uint32_t len, uint32_t crc, uint32_t max) {
    if (len == 0 || len > max || addr < POOL_START || addr >= POOL_END || len > POOL_END - addr) { printf("Request: %lu bytes at %08lX: not in the free DRAM, or too big\n",
                                                                                                  (unsigned long)len, (unsigned long)addr); return REQ_E_RANGE; }
    if (~crc32_update(0xFFFFFFFFu, (const void*)addr, len) != crc) { printf("Request: the data does not have CRC %08lX\n", (unsigned long)crc); return REQ_E_CRC; }
    return 0;
}

int ServeRequest(volatile request_t* r, int from) {
    uint32_t cmd = r->cmd, addr = r->addr, len = r->len, crc = r->crc;
    char text[sizeof r->text];
    int rc, i;

    if (r->magic != REQ_MAGIC) { printf("Request: no request page\n"); return REQ_E_COMMAND; }
    if (cmd >= sizeof cmd_name / sizeof cmd_name[0] || (from == REQ_FROM_PAYLOAD && (cmd == REQ_STORE_BOOT || cmd == REQ_REBOOT || cmd == REQ_WIPE)) ||
        (from == REQ_FROM_JTAG && cmd == REQ_RUN_PJIT)) {
        printf("Request %lu: not served here\n", (unsigned long)cmd);
        return REQ_E_COMMAND;
    }
    printf("Request: %s\n", cmd_name[cmd]);
    switch (cmd) {
    case REQ_STORE_PJIT:
        if ((rc = DataOK(addr, len, crc, STORE_PJIT_MAX - 0x100u))) return rc;
        return PJITStore(from == REQ_FROM_PAYLOAD, (const void*)addr, len);
    case REQ_STORE_ROM:
        if ((rc = DataOK(addr, len, crc, STORE_ROM_MAX - 0x100u))) return rc;
        if (len < 0x10000u || (len & (len - 1))) { printf("A ROM image is 64 KB to 1 MB, a power of two\n"); return REQ_E_FORMAT; }
        return StoreWrite(STORE_ROM_OFF, STORE_ROM_MAX, STORE_MAGIC_ROM, (const void*)addr, len, 0);
    case REQ_STORE_BOOT:
        if ((rc = DataOK(addr, len, crc, BUF_MAX))) return rc;
        if (BootloaderInFlash((const void*)addr, len)) { printf("The flash holds this bootloader already: left alone\n"); return 0; }
        return UpdateBootloader((const void*)addr, len) ? REQ_E_FORMAT : 0;
    case REQ_COPY_ROM:
        if (!boot.bus) { printf("The Amiga's bus is not running\n"); return REQ_E_BUS; }
        return StoreAmigaROM();
    case REQ_SETTINGS:
        for (i = 0; i < (int)sizeof text - 1 && r->text[i]; i++) text[i] = r->text[i];
        text[i] = 0;
        if (SettingsApply(text) < 0) return REQ_E_SETTING;
        return SaveConfigEEPROM() ? REQ_E_SETTING : 0;
    case REQ_ERASE_UPDATE:
        PJITEraseSlot(1);
        printf("The update slot is empty: the backup boots\n");
        return 0;
    case REQ_REBOOT:
        UART0Flush();
        Reset();
        return 0;
    case REQ_STATUS:
        FlashStatus();
        SettingsShow();
        return 0;
    case REQ_RUN_PJIT:
        if ((rc = DataOK(addr, len, crc, BUF_MAX))) return rc;
        return PJITFromMemory((const void*)addr, len);
    case REQ_WIPE:
        rc = StoreWipe((void*)RX_BUF, 0x10000u);
        if (WipeEEPROM()) rc = REQ_E_WIPE;
        if (rc) return REQ_E_WIPE;
        printf("The flash and the EEPROM are blank: the board starts only from a debugger or the UART until it is installed again\n");
        return 0;
    default:
        return 0;
    }
}

/* The debugger writes the page and the data with the CPU halted, possibly past the caches (through the system bus): the data cache is off while this runs. */
void JTAGService(void) {
    volatile request_t* r = (volatile request_t*)REQ_PAGE;
    uint32_t seq;
    int i;

    CP15DCacheCleanFlush();
    CP15DCacheDisable();
    for (i = 0; i < (int)(sizeof *r / 4); i++) ((volatile uint32_t*)r)[i] = 0;
    r->buf = RX_BUF;
    r->buf_max = BUF_MAX;
    r->log = (uint32_t)&console_log;
    r->version = REQ_VERSION;
    r->magic = REQ_MAGIC;
    printf("JTAG: waiting for requests (ESC for the menu)\n");
    for (;;) {
        r->state = REQ_READY;
        while ((seq = r->seq) == r->done) { UART0Ready(); if (uart_esc_seen || uart_break) { r->state = REQ_IDLE; CP15DCacheEnable(); return; } }
        r->state = REQ_BUSY;
        r->result = ServeRequest(r, REQ_FROM_JTAG);
        printf("JTAG: request %lu %s (%ld)\n", (unsigned long)seq, r->result ? "failed" : "done", (long)r->result);
        r->done = seq;
    }
}
