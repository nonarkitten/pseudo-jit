#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "flash.h"
#include "store.h"
#include "init.h"
#include "config.h"
#include "am335x_regs.h"

/* CRC-32 (reflected, 0xEDB88320), slicing by 4: one word load and four table lookups per 4 bytes (about 10x fewer instructions than a bytewise loop).
   The 4 KB table is built on first use. */
static uint32_t crc_t[4][256];
static int crc_ready;
static void crc_init(void) {
    for (uint32_t i = 0; i < 256; i++) { uint32_t c = i; for (int k = 0; k < 8; k++) c = (c & 1) ? (c >> 1) ^ 0xEDB88320u : c >> 1; crc_t[0][i] = c; }
    for (uint32_t i = 0; i < 256; i++) for (int k = 1; k < 4; k++) crc_t[k][i] = (crc_t[k - 1][i] >> 8) ^ crc_t[0][crc_t[k - 1][i] & 0xFF];
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

typedef struct { uint32_t magic, len, crc, aux; } hdr_t;
#define CHUNK 128u

int StoreWrite(uint32_t off, uint32_t max, uint32_t magic, const void *src, uint32_t len, uint32_t aux) {
    if (len == 0 || len > max - 0x100u) return STORE_TOO_BIG;
    uint32_t crc = ~crc32_update(0xFFFFFFFFu, src, len), end = off + 0x100u + len;
    for (uint32_t a = off; a < end; a += 0x10000u) { EraseSPI(a, ERASE_64K); if (((a - off) >> 16 & 3) == 3) putchar('.'); }
    printf("\nErased, programming %lu bytes\n", (unsigned long)len);
    const uint8_t *s = (const uint8_t *)src;
    for (uint32_t i = 0; i < len; i += CHUNK) {
        WriteSPIBlock(off + 0x100u + i, s + i, (len - i < CHUNK) ? len - i : CHUNK);
        if ((i & 0x7FFF) == 0) putchar('.');
    }
    uint32_t back = 0xFFFFFFFFu; uint8_t b[CHUNK];
    for (uint32_t i = 0; i < len; i += CHUNK) { uint32_t n = (len - i < CHUNK) ? len - i : CHUNK; ReadSPIBlock(off + 0x100u + i, b, n); back = crc32_update(back, b, n); }
    if (~back != crc) { printf("\nVerify failed\n"); return STORE_VERIFY; }
    hdr_t h = { magic, len, crc, aux };
    WriteSPIBlock(off, &h, sizeof h);
    printf("\nStored %lu bytes, CRC %08lX\n", (unsigned long)len, (unsigned long)crc);
    return STORE_OK;
}

#define BULK 4096u
static uint8_t bulk_scratch[BULK] __attribute__((aligned(4)));
/* Reads the stored image through ReadSPIBulk (32-bit words, 48 MHz, fast read: 0.5 -> ~5 MB/s) straight into dst (or into a scratch buffer when only the check is wanted); the CRC
   in the header decides. A mismatch at 48 MHz is retried once at 24 MHz (a flash or a board that cannot take the fast clock): spi_bulk_div stays at 1 after that. */
int StoreLoad(uint32_t off, uint32_t max, uint32_t magic, void *dst, uint32_t *len, uint32_t *aux) {
    hdr_t h;
    ReadSPIBlock(off, &h, sizeof h);
    if (h.magic != magic || h.len == 0 || h.len > max - 0x100u) return STORE_ABSENT;
    for (int attempt = 0; ; attempt++) {
        uint32_t crc = 0xFFFFFFFFu; uint8_t *d = (uint8_t *)dst; int err = 0;
        for (uint32_t i = 0; i < h.len && !err; i += BULK) {
            uint32_t n = (h.len - i < BULK) ? h.len - i : BULK, n4 = n & ~3u;
            uint8_t *p = d ? d + i : bulk_scratch;
            ReadSPIBulk(off + 0x100u + i, p, n4);
            err = spi_bulk_err;
            if (n & 3u) ReadSPIBlock(off + 0x100u + i + n4, p + n4, (uint8_t)(n & 3u));
            crc = crc32_update(crc, p, n);
            if (uart_break) return STORE_ABSENT;
        }
        if (!err && ~crc == h.crc) break;
        if (spi_bulk_mode == 0 && attempt == 0)      { spi_bulk_mode = 1; printf("(flash FIFO read %s: retrying word by word)\n", err ? "stalled" : "failed the CRC"); continue; }
        if (spi_bulk_div == 0 && attempt <= 1)       { spi_bulk_div = 1;  printf("(flash read at 48 MHz failed: retrying at 24 MHz)\n"); continue; }
        return STORE_BAD_CRC;
    }
    if (len) *len = h.len;
    if (aux) *aux = h.aux;
    return STORE_OK;
}

static const char *why(int rc) { return rc == STORE_OK ? "ok" : rc == STORE_ABSENT ? "absent" : rc == STORE_BAD_CRC ? "CRC mismatch" : "bad"; }

extern uint32_t LoadXMODEM(uint32_t start);
#define RX_BUF 0x90000000u

void ManageStore(void) {
    char o[4] = { 0 };
    uint32_t n, aux;
    while (o[0] != 'x' && o[0] != 'X') {
        printf("Stored images:\n  PJIT: %s\n  ROM:  %s\n", why(StoreLoad(STORE_PJIT_OFF, STORE_PJIT_MAX, STORE_MAGIC_PJIT, 0, &n, &aux)), why(StoreLoad(STORE_ROM_OFF, STORE_ROM_MAX, STORE_MAGIC_ROM, 0, &n, &aux)));
        printf("1. Store a PJIT bundle (xmodem)\n2. Store a ROM image (xmodem)\n3. Erase the PJIT slot\n4. Erase the ROM slot\n5. Store a copy of the Amiga's own ROM (EEPROM base and size)\nX. Exit\n] ");
        gets(o);
        switch (o[0]) {
        case '1': if ((n = LoadXMODEM(RX_BUF))) { StoreWrite(STORE_PJIT_OFF, STORE_PJIT_MAX, STORE_MAGIC_PJIT, (void *)RX_BUF, n, 0); } break;
        case '2': if ((n = LoadXMODEM(RX_BUF))) { StoreWrite(STORE_ROM_OFF, STORE_ROM_MAX, STORE_MAGIC_ROM, (void *)RX_BUF, n, 0); } break;
        case '3': EraseSPI(STORE_PJIT_OFF, ERASE_64K); break;
        case '4': EraseSPI(STORE_ROM_OFF, ERASE_64K); break;
        case '5': {
            uint32_t base = (uint32_t)settings.maprom_base_64k << 16, size = 1u << settings.maprom_size_log2;
            volatile uint16_t *src = (volatile uint16_t *)base; uint16_t *dst = (uint16_t *)RX_BUF;     /* halfword reads through the window keep the Amiga's byte order */
            printf("Reading %lu KB at %06lX\n", (unsigned long)(size >> 10), (unsigned long)base);
            for (uint32_t i = 0; i < size / 2; i++) dst[i] = src[i];
            StoreWrite(STORE_ROM_OFF, STORE_ROM_MAX, STORE_MAGIC_ROM, (void *)RX_BUF, size, 0);
        } break;
        }
    }
}
