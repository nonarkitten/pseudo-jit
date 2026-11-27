/* request.h -- flash and settings work the bootloader does for someone other than the person at the menu: the JTAG installer (tools/buffee/buffee_jtag.py),
 * which starts the bootloader with r0 = JTAG_MAGIC and then writes requests into the request page while it waits, and the payload, which fills the page and
 * returns RET_REQUEST (an update of PJIT or the settings from the Amiga). The page sits at REQ_PAGE in the DRAM; the data a request names is elsewhere in the
 * DRAM. Every field is a little-endian word. The layout is an interface to programs built apart from the bootloader: fields are only ever added at the end.
 *
 * Copyright (c) 2026 the Buffee Project - http://www.buffee.ca
 * Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (see LICENSE).
 */
#ifndef REQUEST_H
#define REQUEST_H
#include <stdint.h>

#define JTAG_MAGIC  0x4A544147u         /* r0 at the entry when a debugger started the bootloader */
#define REQ_PAGE    0x9F5F0000u
#define REQ_MAGIC   0x51524642u         /* 'BFRQ' */
#define LOG_MAGIC   0x474F4C42u         /* 'BLOG': the console log, whose address is the word after it at 0x402F0424 */
#define LOG_SIZE    0x2000u

enum {                                  /* request_t.cmd */
    REQ_NONE,
    REQ_STORE_PJIT,                     /* addr/len/crc: a PJIT bundle. From JTAG into the backup slot, from the payload into the update slot. */
    REQ_STORE_ROM,                      /* addr/len/crc: a MapROM image (64 KB .. 1 MB, a power of two) into the ROM slot */
    REQ_STORE_BOOT,                     /* addr/len/crc: a bootloader image into the start of the flash (JTAG only; left alone when the flash holds the same) */
    REQ_COPY_ROM,                       /* the Amiga's own ROM (the MapROM address and size of the settings) into the ROM slot */
    REQ_SETTINGS,                       /* text: "item=value" pairs of the settings page (menu 4), ';' between them, "11=" restores the defaults; saved in the EEPROM */
    REQ_ERASE_UPDATE,                   /* empties the update slot: the backup boots */
    REQ_REBOOT,                         /* resets the board (JTAG only; a payload's request always ends with one) */
    REQ_STATUS,                         /* prints the flash slots and the settings to the console */
    REQ_RUN_PJIT,                       /* addr/len/crc: a PJIT bundle to run from a clean Amiga reset without storing it (the payload only; the next reset boots the flash) */
    REQ_WIPE,                           /* erases all of the flash (the bootloader too) and the EEPROM and reads both back blank (JTAG only): the board then starts only
                                           from a debugger or the UART, as one never programmed */
};

enum {                                  /* request_t.result besides 0 and the STORE_* codes */
    REQ_E_COMMAND = -10,                /* unknown, or not allowed from where it came */
    REQ_E_RANGE   = -11,                /* the data is not inside the DRAM's free pool, or too big */
    REQ_E_CRC     = -12,                /* the data does not have the CRC the request gives */
    REQ_E_FORMAT  = -13,                /* not a bundle, a ROM image or a bootloader */
    REQ_E_SETTING = -14,                /* the settings text did not parse, or the EEPROM write failed */
    REQ_E_BUS     = -15,                /* needs the Amiga's bus, which is not running */
    REQ_E_WIPE    = -16,                /* the flash or the EEPROM did not read back blank */
};

enum { REQ_IDLE, REQ_READY, REQ_BUSY }; /* request_t.state */

typedef struct {
    uint32_t magic;                     /* REQ_MAGIC (the asker) */
    uint32_t cmd;                       /* REQ_* (the asker) */
    uint32_t addr, len, crc;            /* the data: DRAM address, bytes, CRC-32 as zlib computes it (the asker) */
    uint32_t seq;                       /* JTAG: the host increments it to start the command in cmd */
    uint32_t done;                      /* the bootloader: the seq of the last command finished */
    int32_t  result;                    /* the bootloader: 0 or an error */
    uint32_t state;                     /* the bootloader: REQ_READY while it waits for a command */
    uint32_t buf, buf_max;              /* the bootloader: where the data may go */
    uint32_t log;                       /* the bootloader: the console log (log_t) */
    uint32_t version;                   /* the bootloader: REQ_VERSION */
    uint32_t reserved[3];
    char     text[192];                 /* REQ_SETTINGS */
} request_t;

#define REQ_VERSION 1

typedef struct { uint32_t magic, size, head; char buf[LOG_SIZE]; } log_t;     /* buf[i % size] for the last `size` of `head` bytes written */

enum { REQ_FROM_JTAG, REQ_FROM_PAYLOAD };

extern log_t console_log;
extern uint32_t jtag_entry[2];          /* r0, r1 at the entry (clib/init.s) */
int  ServeRequest(volatile request_t* r, int from);
void JTAGService(void);                 /* waits for requests from the debugger; returns on ESC */

#endif
