/* Images kept in the SPI flash next to the bootloader (which sits at offset 0, where the AM335x ROM looks): the PJIT bundle and a MapROM image.
 * Each slot is a 256-byte header page (magic, length, CRC-32, aux) followed by the data at +0x100. The header is written last: a slot interrupted
 * by a power failure has no valid header and is simply absent. */
#ifndef STORE_H
#define STORE_H
#include <stdint.h>
#define STORE_PJIT_OFF  0x020000u               /* up to 896 KB (the bundle is ~220 KB) */
#define STORE_PJIT_MAX  0x0E0000u
#define STORE_ROM_OFF   0x100000u               /* up to 1 MB */
#define STORE_ROM_MAX   0x100000u
#define STORE_MAGIC_PJIT 0x544A4A50u            /* 'PJJT' */
#define STORE_MAGIC_ROM  0x4D4F5250u            /* 'PROM' */
enum { STORE_OK = 0, STORE_ABSENT = -1, STORE_BAD_CRC = -2, STORE_TOO_BIG = -3, STORE_VERIFY = -4 };
uint32_t crc32_update(uint32_t crc, const void *buf, uint32_t len);
int StoreWrite(uint32_t off, uint32_t max, uint32_t magic, const void *src, uint32_t len, uint32_t aux);
int StoreLoad(uint32_t off, uint32_t max, uint32_t magic, void *dst, uint32_t *len, uint32_t *aux);   /* dst may be 0: check only */
void ManageStore(void);                         /* menu F */
extern int uart_break;                          /* set by the UART interrupt after three ESC in a row; cleared by whoever shows the menu */
#endif
