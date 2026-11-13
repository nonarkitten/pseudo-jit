/*
 * writeimage.c -- puts a new bootloader into the SPI flash at offset 0, where the AM335x ROM looks: the TI boot header (the image's length and its load address)
 * and the image, in 4 KB sectors, then reads all of it back.
 *
 * Copyright (c) 2021-2026 the Buffee Project - http://www.buffee.ca
 * Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (see LICENSE).
 */
#include "main.h"

#define LOAD_ADDRESS 0x402F0400u        /* where the ROM puts the image in the on-chip RAM */
#define MAX_IMAGE    0x1C000u           /* what fits below the ROM's own data in the on-chip RAM */

/* The flash holds each word byte-swapped: the ROM reads the header big-endian. */
static uint32_t WriteWord(uint32_t addr, uint32_t data) {
    uint32_t w = rev32(data);

    WriteSPIBlock(addr, (uint8_t*)&w, sizeof w);
    WaitUSDMTimer(60);
    return addr + 4;
}

static uint32_t ReadWord(uint32_t addr) {
    uint32_t w;

    ReadSPIBlock(addr, (uint8_t*)&w, sizeof w);
    return rev32(w);
}

/* The header (the image's length, its load address) in the first 8 bytes, the image from offset 8. */
static int WriteImage(const uint32_t* src, uint32_t len) {
    uint32_t pos = 0, end = 8 + len;
    const uint32_t* p;

    printf("Programming %lu bytes", (unsigned long)len);
    for (p = src; pos < end; ) {
        if ((pos & 0xFFF) == 0) { EraseSPI(pos, ERASE_4K); WaitMSDMTimer(400); putchar('.'); }
        pos = WriteWord(pos, pos == 0 ? len : pos == 4 ? LOAD_ADDRESS : *p++);
    }
    printf("\nVerifying\n");
    if (ReadWord(0) != len || ReadWord(4) != LOAD_ADDRESS) { printf("Verify failed: the boot header\n"); return -1; }
    for (pos = 8, p = src; pos < end; pos += 4, p++) if (ReadWord(pos) != *p) { printf("Verify failed at %08lX\n", (unsigned long)pos); return -1; }
    printf("Verify passed\n");
    return 0;
}

int UpdateBootloader(const void* image, uint32_t len) {
    uint32_t first = *(const uint32_t*)image;

    if ((first >> 24) != 0xEA || len + 8 > MAX_IMAGE || (len & 3)) {        /* a bootloader image starts with its vector table: a branch */
        printf("That does not look like a bootloader image (first word %08lX, %lu bytes)\n", (unsigned long)first, (unsigned long)len);
        return -1;
    }
    return WriteImage((const uint32_t*)image, len);
}
