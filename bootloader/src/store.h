/* Images kept in the SPI flash next to the bootloader (which sits at offset 0, where the AM335x ROM looks): two PJIT bundles and a MapROM image.
 * Each slot is a 256-byte header page (magic, length, CRC-32, sequence number) followed by the data at +0x100. The header is written last: a slot interrupted
 * by a power failure has no valid header and is simply absent. A word after the header, left erased, is programmed to zero to mark a slot bad (no erase needed).
 * The PJIT bundle has two slots: the one the UART and JTAG write (the backup) and the one an update from the Amiga writes; the boot takes the good one with the
 * higher sequence number, so the bundle written last wins and the other is the fallback. */
#ifndef STORE_H
#define STORE_H
#include <stdint.h>
#define STORE_PJIT_OFF  0x020000u               /* the backup slot (UART, JTAG): up to 448 KB (the bundle is ~240 KB) */
#define STORE_PJIT_UPD  0x090000u               /* the update slot (written on the payload's request) */
#define STORE_PJIT_MAX  0x070000u
#define STORE_ROM_OFF   0x100000u               /* up to 1 MB */
#define STORE_ROM_MAX   0x100000u
#define STORE_MAGIC_PJIT 0x544A4A50u            /* 'PJJT' */
#define STORE_MAGIC_ROM  0x4D4F5250u            /* 'PROM' */
enum { STORE_OK = 0, STORE_ABSENT = -1, STORE_BAD_CRC = -2, STORE_TOO_BIG = -3, STORE_VERIFY = -4, STORE_MARKED_BAD = -5 };
uint32_t crc32_update(uint32_t crc, const void *buf, uint32_t len);
int StoreWrite(uint32_t off, uint32_t max, uint32_t magic, const void *src, uint32_t len, uint32_t seq);
int StoreLoad(uint32_t off, uint32_t max, uint32_t magic, void *dst, uint32_t *len, uint32_t *seq);   /* dst may be 0: check only */
int StoreHeader(uint32_t off, uint32_t max, uint32_t magic, uint32_t *len, uint32_t *seq);            /* the header alone (no CRC): STORE_OK, STORE_ABSENT or STORE_MARKED_BAD */
int StoreInfo(uint32_t off, uint32_t max, uint32_t magic, uint32_t *len, uint32_t *crc, uint32_t *seq);    /* the same, with the CRC the header gives (not checked) */
void StoreMarkBad(uint32_t off);                /* the slot is skipped from now on, until it is written again */
void StoreErase(uint32_t off);                  /* empties the slot at off */
int StoreWipe(void *scratch, uint32_t scratch_len);    /* erases the whole chip, the bootloader too, and reads every address back: 0 = all erased */
#define STORE_CHECK_SPAN 0x1000000u             /* what StoreWipe reads back: the 24-bit address space (a smaller part repeats itself in it) */
#endif
