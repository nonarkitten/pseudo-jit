/*
 * store.c -- the images kept in the SPI flash next to the bootloader (store.h): writing a slot, reading and checking one, erasing one.
 *
 * Copyright (c) 2021-2026 the Buffee Project - http://www.buffee.ca
 * Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (see LICENSE).
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "flash.h"
#include "store.h"
#include "init.h"
#include "config.h"
#include "am335x_regs.h"

#define CHUNK     128u                  /* bytes per page program and per read-back */
#define BULK      4096u                 /* bytes per bulk read */
#define HDR_SIZE  0x100u                /* the header page in front of a slot's data */
#define CRC_POLY  0xEDB88320u

typedef struct { uint32_t magic, len, crc, aux; } hdr_t;

/* CRC-32 (reflected), slicing by 4: one word load and four table lookups per 4 bytes (about 10x fewer instructions than a bytewise loop). The 4 KB table is built on first use. */
static uint32_t crc_t[4][256];
static int crc_ready;
static uint8_t bulk_scratch[BULK] __attribute__((aligned(4)));

static void crc_init(void) {
    uint32_t i, c;
    int k;

    for (i = 0; i < 256; i++) { c = i; for (k = 0; k < 8; k++) c = (c & 1) ? (c >> 1) ^ CRC_POLY : c >> 1; crc_t[0][i] = c; }
    for (i = 0; i < 256; i++) for (k = 1; k < 4; k++) crc_t[k][i] = (crc_t[k - 1][i] >> 8) ^ crc_t[0][crc_t[k - 1][i] & 0xFF];
    crc_ready = 1;
}

uint32_t crc32_update(uint32_t crc, const void *buf, uint32_t len) {
    const uint8_t *p = (const uint8_t *)buf;

    if (!crc_ready) crc_init();
    while (len && ((uintptr_t)p & 3)) { crc = (crc >> 8) ^ crc_t[0][(crc ^ *p++) & 0xFF]; len--; }
    for (; len >= 4; len -= 4, p += 4) {
        crc ^= *(const uint32_t *)p;                                                       /* the bootloader runs little-endian */
        crc = crc_t[3][crc & 0xFF] ^ crc_t[2][(crc >> 8) & 0xFF] ^ crc_t[1][(crc >> 16) & 0xFF] ^ crc_t[0][crc >> 24];
    }
    while (len--) crc = (crc >> 8) ^ crc_t[0][(crc ^ *p++) & 0xFF];
    return crc;
}

int StoreWrite(uint32_t off, uint32_t max, uint32_t magic, const void *src, uint32_t len, uint32_t aux) {
    const uint8_t *s = (const uint8_t *)src;
    uint32_t crc, end, a, i, n, back = 0xFFFFFFFFu;
    uint8_t b[CHUNK];
    hdr_t h;

    if (len == 0 || len > max - HDR_SIZE) return STORE_TOO_BIG;
    crc = ~crc32_update(0xFFFFFFFFu, src, len);
    end = off + HDR_SIZE + len;
    for (a = off; a < end; a += 0x10000u) { EraseSPI(a, ERASE_64K); if (((a - off) >> 16 & 3) == 3) putchar('.'); }
    printf("\nErased, programming %lu bytes\n", (unsigned long)len);
    for (i = 0; i < len; i += CHUNK) {
        WriteSPIBlock(off + HDR_SIZE + i, s + i, (len - i < CHUNK) ? len - i : CHUNK);
        if ((i & 0x7FFF) == 0) putchar('.');
    }
    for (i = 0; i < len; i += CHUNK) { n = (len - i < CHUNK) ? len - i : CHUNK; ReadSPIBlock(off + HDR_SIZE + i, b, n); back = crc32_update(back, b, n); }
    if (~back != crc) { printf("\nVerify failed\n"); return STORE_VERIFY; }
    h.magic = magic; h.len = len; h.crc = crc; h.aux = aux;
    WriteSPIBlock(off, &h, sizeof h);
    printf("\nStored %lu bytes, CRC %08lX\n", (unsigned long)len, (unsigned long)crc);
    return STORE_OK;
}

/* Reads the stored image through ReadSPIBulk (32-bit words, 48 MHz, fast read: 0.5 -> ~5 MB/s) straight into dst (or into a scratch buffer when only the check is wanted); the CRC
   in the header decides. A mismatch at 48 MHz is retried once at 24 MHz (a flash or a board that cannot take the fast clock): spi_bulk_div stays at 1 after that. */
int StoreLoad(uint32_t off, uint32_t max, uint32_t magic, void *dst, uint32_t *len, uint32_t *aux) {
    uint8_t *d = (uint8_t *)dst, *p;
    uint32_t crc, i, n, n4;
    int attempt, err;
    hdr_t h;

    ReadSPIBlock(off, &h, sizeof h);
    if (h.magic != magic || h.len == 0 || h.len > max - HDR_SIZE) return STORE_ABSENT;
    for (attempt = 0; ; attempt++) {
        crc = 0xFFFFFFFFu;
        err = 0;
        for (i = 0; i < h.len && !err; i += BULK) {
            n = (h.len - i < BULK) ? h.len - i : BULK;
            n4 = n & ~3u;
            p = d ? d + i : bulk_scratch;
            ReadSPIBulk(off + HDR_SIZE + i, p, n4);
            err = spi_bulk_err;
            if (n & 3u) ReadSPIBlock(off + HDR_SIZE + i + n4, p + n4, (uint8_t)(n & 3u));
            crc = crc32_update(crc, p, n);
            if (uart_break) return STORE_ABSENT;
        }
        if (!err && ~crc == h.crc) break;
        if (spi_bulk_mode == 0 && attempt == 0)      { spi_bulk_mode = 1; info("(flash FIFO read %s: retrying word by word)\n", err ? "stalled" : "failed the CRC"); continue; }
        if (spi_bulk_div == 0 && attempt <= 1)       { spi_bulk_div = 1;  info("(flash read at 48 MHz failed: retrying at 24 MHz)\n"); continue; }
        return STORE_BAD_CRC;
    }
    if (len) *len = h.len;
    if (aux) *aux = h.aux;
    return STORE_OK;
}

void StoreErase(uint32_t off) { EraseSPI(off, ERASE_64K); }       /* the header page goes with the first 64 KB: the slot is empty */
