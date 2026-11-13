/*
 * config.c -- the boot settings in the EEPROM (I2C address $50): the defaults, and loading and saving them with a CRC.
 *
 * Copyright (c) 2020-2026 Renee Cousins, the Buffee Project - http://www.buffee.ca
 * Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (see LICENSE).
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "config.h"
#include "init.h"

#define EEPROM_I2C  0x50
#define CONFIG_IDENT 0x704A4954u

/* The board kind (whether the data lines go through the byte swapper) is a property of the board, not of the host or the settings: a record of its own, outside
   config_t, so that the defaults, a bad settings CRC or a new settings layout never touch it. Written once (hal/amiga.c: the GreenPAK supply test), read at every boot. */
#define BOARD_REC_ADDR  0x0100u         /* 8-byte aligned: one page write on any part */
#define BOARD_REC_MAGIC 0x444E4B42u     /* 'BKND' */
typedef struct { uint32_t magic; uint8_t kind, check, unused[2]; } board_rec_t;

config_t settings;

/* The settings of a board with nothing stored in its EEPROM. */
void config_set_defaults(config_t *c) {
    memset(c, 0, sizeof *c);
    c->ident = CONFIG_IDENT;
    c->cpu_enable_68000 = 1;
    c->first_boot = 1;                      /* the full self-test and the boot report, until a boot passes */
    c->post_checkclk = 1;
    c->dpll_mul = 1000;
    c->pmic_voltage = 135;                  /* 1.35 V for 1 GHz */
    c->cache_index_bits = 11;
    c->cache_block_bits = 7;
    c->maprom_page = 0xFF;
    c->maprom_base_64k = 0xF8;              /* $F80000 */
    c->maprom_size_log2 = 19;               /* 512 KB */
    c->maprom_src = MAPROM_RAM;             /* copy the Amiga's own Kickstart into DRAM (works with nothing in the flash) */
    c->ram_mode = RAM_128M;
    c->pace = 100;                          /* interpreted code (chip RAM) runs at an MC68000's pace; fast RAM and the MapROM megabyte are translated at full speed */
    c->kHz = 8000;
    c->is_dirty = 1;
}

/* The CRC covers the bytes after it up to and including is_dirty (pace, in the tail padding, is outside). */
static unsigned short crc16_ccitt(const void *buf, uint32_t len, uint16_t init) {
    static const unsigned short crc16_table[256]= {
        0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
        0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
        0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
        0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
        0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
        0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
        0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
        0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
        0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
        0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
        0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
        0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
        0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
        0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
        0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
        0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
        0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
        0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
        0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
        0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
        0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
        0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
        0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
        0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
        0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
        0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
        0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
        0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
        0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
        0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
        0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
        0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
    };
    uint8_t *_b = (uint8_t *)buf;
    uint16_t crc = init;
    while (len--) {
        crc = (crc << 8) ^ crc16_table[((crc >> 8) ^ *_b++) & 0x00FF];
    }
    return crc;
}

static uint16_t config_crc(const config_t *c) {
    return crc16_ccitt(6 + (const uint8_t*)c, sizeof(config_t) - 8, 0xFFFF);
}

int SaveConfigEEPROM(void) {
    static config_t back;
    uint8_t addr[2] = { 0 }, a[2];
    uint32_t off, n;

    settings.is_dirty = 0;
    settings.crc16 = config_crc(&settings);
    /* A page write that crosses a page boundary wraps inside the page (the part is 8 / 16 / 32 bytes per page): write aligned chunks of 8 bytes, wait out the write cycle after each. */
    for (off = 0; off < sizeof(config_t); off += 8) {
        n = sizeof(config_t) - off < 8 ? sizeof(config_t) - off : 8;
        a[0] = (uint8_t)(off >> 8);
        a[1] = (uint8_t)off;
        I2C0SendCmd(EEPROM_I2C, a, 2, (const uint8_t*)&settings + off, n);
        WaitMSDMTimer(10);
    }
    I2C0ReadCmd(EEPROM_I2C, addr, 2, (uint8_t*)&back, sizeof(config_t));
    if (memcmp(&back, &settings, sizeof(config_t))) { printf("Settings: the EEPROM did not read back what was written\n"); return -1; }
    info("Settings saved (CRC %04X)\n", settings.crc16);
    return 0;
}

int LoadConfigEEPROM(void) {
    static config_t stored;
    uint8_t addr[2] = { 0 };
    int why;

    if (!I2C0Probe(EEPROM_I2C)) { config_set_defaults(&settings); return CFG_NO_EEPROM; }
    I2C0ReadCmd(EEPROM_I2C, addr, 2, (uint8_t*)&stored, sizeof(config_t));
    if (stored.ident == CONFIG_IDENT && config_crc(&stored) == stored.crc16) { settings = stored; return CFG_LOADED; }
    why = (stored.ident == CONFIG_IDENT) ? CFG_BAD_CRC : CFG_BLANK;
    config_set_defaults(&settings);
    return why;
}

int LoadBoardKind(void) {
    board_rec_t r;
    uint8_t a[2] = { BOARD_REC_ADDR >> 8, BOARD_REC_ADDR & 0xFF };

    if (!I2C0Probe(EEPROM_I2C) || !I2C0ReadCmd(EEPROM_I2C, a, 2, (uint8_t*)&r, sizeof r)) return -1;
    if (r.magic != BOARD_REC_MAGIC || r.check != (uint8_t)~r.kind || r.kind > 1) return -1;
    return r.kind;
}

int SaveBoardKind(int kind) {
    board_rec_t r = { BOARD_REC_MAGIC, (uint8_t)kind, (uint8_t)~kind, { 0, 0 } }, back;
    uint8_t a[2] = { BOARD_REC_ADDR >> 8, BOARD_REC_ADDR & 0xFF };

    if (!I2C0Probe(EEPROM_I2C)) return -1;
    I2C0SendCmd(EEPROM_I2C, a, 2, (const uint8_t*)&r, sizeof r);
    WaitMSDMTimer(10);
    if (!I2C0ReadCmd(EEPROM_I2C, a, 2, (uint8_t*)&back, sizeof back) || memcmp(&back, &r, sizeof r)) { printf("Board kind: the EEPROM did not read back what was written\n"); return -1; }
    return 0;
}
