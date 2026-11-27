/// @file buffee_ide.c
/// @brief Does a Gayle IDE port (an A600 / A1200, a MiniMig's hard file) deliver its sectors? Without Kickstart: the Gayle ID, IDENTIFY DEVICE of the master, then SECTORS
///        (default 64) sectors from LBA 0 read with READ SECTORS one at a time (CHS from the drive's geometry), each read twice and compared, and the same range again as
///        commands of MULTI (default 8) sectors. Prints `IDE` lines: per sector a checksum (a, b: a += word, b += a, both mod 65521, over the 256 big-endian words) and
///        its first four words, so that a host tool can compare them with the image. Rigid Disk Blocks among them are decoded (the geometry, the
///        partitions, the filesystems to load) and their checksums verified. `IDE RESULT` counts the sectors that differed between two reads, the commands that
///        failed and the blocks with a wrong checksum.
#include <stdint.h>
#include "pjit.h"
#include "bbb.h"
#include "buffee.h"
#include "mem_be.h"

#ifndef SECTORS
#define SECTORS 64
#endif
#ifndef MULTI
#define MULTI 8
#endif
#define REG(n)      ((volatile uint8_t *)(0xDA2000u + 4u * (n)))    /* the task file: 0 data (16 bits), 1 error / features, 2 sector count, 3 sector, 4 / 5 cylinder, 6 device / head, 7 status / command */
#define DATA        0xDA2000u
#define ST_BSY      0x80
#define ST_DRQ      0x08
#define ST_ERR      0x01

static uint16_t buf[MULTI > 1 ? MULTI * 256 : 256], again[256];
static uint32_t cyls, heads, spt;

/// Waits until BSY is clear and (want_drq) DRQ is set; the status, or -1 after ms milliseconds.
static int wait_status(int want_drq, uint32_t ms)
{
    uint32_t t = bbb_pmu_cycles();
    for (;;) {
        uint8_t s = *REG(7);
        if (!(s & ST_BSY) && (!want_drq || (s & (ST_DRQ | ST_ERR)))) return s;
        if (bbb_pmu_cycles() - t > ms * 1000000u) return -1;
    }
}

static uint32_t L(const uint16_t *p, unsigned i) { return (uint32_t)p[2 * i] << 16 | p[2 * i + 1]; }

/// A Rigid Disk Block (RDSK, PART, FSHD, LSEG, BADB)? Prints what it says and whether its checksum holds (the longs of the block add up to 0). 1 when the checksum is wrong.
static int rdb_block(uint32_t lba, const uint16_t *p)
{
    uint32_t id = L(p, 0), n = L(p, 1), sum = 0;
    if (id != 0x5244534Bu && id != 0x50415254u && id != 0x46534844u && id != 0x4C534547u && id != 0x42414442u) return 0;
    if (n > 128) n = 128;
    for (uint32_t i = 0; i < n; i++) sum += L(p, i);
    if (id == 0x5244534Bu)
        printf("IDE   RDSK at %u: %u cylinders, %u sectors, %u heads; partitions from block %u, filesystems from block %u\n", (unsigned)lba, (unsigned)L(p, 16), (unsigned)L(p, 17),
               (unsigned)L(p, 18), (unsigned)L(p, 7), (unsigned)L(p, 8));
    else if (id == 0x50415254u) {
        uint32_t dt = L(p, 48), len = p[18] >> 8;
        printf("IDE   PART at %u: \"", (unsigned)lba);
        for (uint32_t i = 0; i < len && i < 30; i++) { uint16_t w = p[18 + (i + 1) / 2]; printf("%c", (i + 1) & 1 ? w & 255 : w >> 8); }
        printf("\", DOS type %c%c%c\\%u, cylinders %u-%u, boot priority %d, flags %X\n", (int)(dt >> 24), (int)(dt >> 16 & 255), (int)(dt >> 8 & 255), (unsigned)(dt & 255),
               (unsigned)L(p, 41), (unsigned)L(p, 42), (int)L(p, 47), (unsigned)L(p, 5));
    } else if (id == 0x46534844u) {
        uint32_t dt = L(p, 8), v = L(p, 9);
        printf("IDE   FSHD at %u: a filesystem for DOS type %c%c%c\\%u, version %u.%u, its code from block %u\n", (unsigned)lba, (int)(dt >> 24), (int)(dt >> 16 & 255),
               (int)(dt >> 8 & 255), (unsigned)(dt & 255), (unsigned)(v >> 16), (unsigned)(v & 0xFFFF), (unsigned)L(p, 18));
    }
    if (sum) printf("IDE   block %u: CHECKSUM WRONG (the longs add up to %08X)\n", (unsigned)lba, (unsigned)sum);
    return sum != 0;
}

static void checksum(const uint16_t *p, uint32_t *a, uint32_t *b)
{
    uint32_t x = 0, y = 0;
    for (int i = 0; i < 256; i++) { x = (x + p[i]) % 65521u; y = (y + x) % 65521u; }
    *a = x; *b = y;
}

/// READ SECTORS of n sectors from lba into p. 0, or the step that failed (1 not ready, 2 no DRQ for a sector, 3 an error status).
static int read_sectors(uint32_t lba, unsigned n, uint16_t *p)
{
    uint32_t c = lba / (heads * spt), h = lba / spt % heads, s = lba % spt + 1;

    if (wait_status(0, 1000) < 0) return 1;
    *REG(6) = (uint8_t)(0xA0 | h);
    *REG(2) = (uint8_t)n; *REG(3) = (uint8_t)s; *REG(4) = (uint8_t)c; *REG(5) = (uint8_t)(c >> 8);
    *REG(7) = 0x20;
    for (unsigned k = 0; k < n; k++) {
        int st = wait_status(1, 2000);
        if (st < 0) return 2;
        if (st & ST_ERR) return 3;
        for (int i = 0; i < 256; i++) p[k * 256 + i] = read_BE_word(DATA);
    }
    return 0;
}

int main(void)
{
    uint32_t a, b, bad = 0, timeouts = 0, t0, rdb = 0, rdbbad = 0;
    uint8_t id = 0;
    int st;

    write_BE_word(0xDE1000, 0);
    for (int i = 0; i < 4; i++) id = (uint8_t)(id << 1 | read_BE_word(0xDE1000) >> 15);
    printf("IDE Gayle ID %X (D = a Gayle that is read once per access), status %02X\n", id, *REG(7));
    *(volatile uint8_t *)0xDAA000 = 0;                                  /* Gayle INTENA off: polled */
    *REG(6) = 0xA0;
    if (wait_status(0, 1000) < 0) { printf("IDE RESULT: the drive stays busy (status %02X)\n", *REG(7)); return 0; }
    *REG(7) = 0xEC;
    st = wait_status(1, 2000);
    if (st < 0 || (st & ST_ERR)) { printf("IDE RESULT: IDENTIFY gave status %02X error %02X\n", *REG(7), *REG(1)); return 0; }
    for (int i = 0; i < 256; i++) buf[i] = read_BE_word(DATA);
    for (int i = 0; i < 256; i++) buf[i] = (uint16_t)(buf[i] << 8 | buf[i] >> 8);        /* ATA words arrive low byte first */
    cyls = buf[1]; heads = buf[3]; spt = buf[6];
    printf("IDE identify: %u cylinders, %u heads, %u sectors (%u MB), model \"", (unsigned)cyls, (unsigned)heads, (unsigned)spt, (unsigned)(cyls * heads * spt / 2048));
    for (int i = 27; i < 47; i++) printf("%c%c", buf[i] >> 8 >= 32 && buf[i] >> 8 < 127 ? buf[i] >> 8 : '.', (buf[i] & 255) >= 32 && (buf[i] & 255) < 127 ? buf[i] & 255 : '.');
    printf("\", status after %02X\n", *REG(7));
    if (!heads || !spt) { printf("IDE RESULT: no geometry\n"); return 0; }

    t0 = bbb_pmu_cycles();
    for (uint32_t lba = 0; lba < SECTORS; lba++) {
        int r1 = read_sectors(lba, 1, buf), r2 = read_sectors(lba, 1, again), same = 1;
        if (r1 || r2) { timeouts++; printf("IDE sector %u: failed (%d, %d), status %02X error %02X\n", (unsigned)lba, r1, r2, *REG(7), *REG(1)); continue; }
        for (int i = 0; i < 256; i++) if (buf[i] != again[i]) same = 0;
        checksum(buf, &a, &b);
        printf("IDE sector %u: %04X %04X  %04X %04X %04X %04X%s\n", (unsigned)lba, (unsigned)a, (unsigned)b, buf[0], buf[1], buf[2], buf[3], same ? "" : "  DIFFERS on the second read");
        if (!same) bad++;
        { uint32_t id = L(buf, 0); if (id == 0x5244534Bu || id == 0x50415254u || id == 0x46534844u || id == 0x4C534547u || id == 0x42414442u) rdb++; }
        rdbbad += rdb_block(lba, buf);
#ifdef DUMP
        printf("IDE dump %u:", (unsigned)lba); for (int i = 0; i < 256; i++) printf(" %04X", buf[i]); printf("\n");     /* -DDUMP: every sector's 256 words, for a host tool */
#endif
    }
    printf("IDE single-sector reads: %u ms for %u reads\n", (unsigned)((bbb_pmu_cycles() - t0) / 1000000u), 2u * SECTORS);
    for (uint32_t lba = 0; lba + MULTI <= SECTORS; lba += MULTI) {
        int r = read_sectors(lba, MULTI, buf);
        if (r) { timeouts++; printf("IDE multi %u+%u: failed (%d), status %02X error %02X\n", (unsigned)lba, MULTI, r, *REG(7), *REG(1)); continue; }
        printf("IDE multi %u+%u:", (unsigned)lba, MULTI);
        for (unsigned k = 0; k < MULTI; k++) { checksum(buf + k * 256, &a, &b); printf(" %04X%04X", (unsigned)a, (unsigned)b); }
        printf("\n");
    }
    printf("IDE RESULT: %u sectors differed between two reads, %u commands failed, %u of %u Rigid Disk Blocks have a wrong checksum; status %02X\n", (unsigned)bad, (unsigned)timeouts,
           (unsigned)rdbbad, (unsigned)rdb, *REG(7));
    return 0;
}
