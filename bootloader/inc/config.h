/*
 * config.h -- the boot settings: what the bootloader does at start-up and what it tells the payload. They live in the EEPROM (menu 4, Settings) with a CRC.
 *
 * Copyright (c) 2020-2026 Renee Cousins, the Buffee Project - http://www.buffee.ca
 * Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (see LICENSE).
 *
 * The layout is what is stored on the board: change it only together with `ident`. Fields marked "unused" are kept so that stored settings stay valid.
 */
#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <stdint.h>

enum { MAPROM_OFF = 0, MAPROM_FLASH = 1, MAPROM_RAM = 2 };     /* where the ROM image comes from: nowhere (the Amiga's own ROM stays), the SPI flash, a copy of the Amiga's ROM */
enum { RAM_OFF = 0, RAM_128M = 1, RAM_AUTOCONFIG = 2 };        /* unused: the guest RAM is described by mem_region[] */
enum { BOOT_24BIT = 4, BOOT_NOSWAP = 8 };   /* BOOT_24BIT: the guest has 24 address lines (A24-A31 ignored, as on a 68000): the page table repeats the 16 MB space over the 4 GB.
                                                BOOT_NOSWAP (the payload only, never stored): the board has no byte swapper. Bits 0 and 1 are unused. */

typedef struct {
    /* integrity check of the EEPROM contents */
    uint32_t ident;                 /* 0x704A4954 */
    uint16_t crc16;                 /* over everything after it */

    /* the guest CPU: unused (the 68000 is the only model) */
    struct {
        int cpu_enable_68000  : 1;
        int cpu_enable_68020  : 1;
        int cpu_enable_68030  : 1;
        int cpu_enable_68040  : 1;
        int cpu_enable_32bits : 1;
        int cpu_enable_fpu    : 1;
        int cpu_enable_icache : 1;
        int cpu_enable_dcache : 1;
        int cpu_enable_mmu    : 1;
    };

    /* the power-on self-test and the boot report. A first boot (the defaults were just loaded) runs the full test with the report on; the first boot that passes clears
       first_boot and boot_verbose, so later boots run the short test quietly unless post_full or boot_verbose is set again. */
    struct {
        unsigned post_full     : 1; /* the full self-test at every boot: all of the DDR, the 40 ms bus-clock measurement */
        unsigned boot_verbose  : 1; /* the boot report at every boot */
        unsigned post_checkclk : 1; /* measure the Amiga's bus clock (or check it against the stored host) at every boot; 0: kHz as set */
        unsigned first_boot    : 1; /* set with the defaults: the next boot is a first boot */
    };

    uint16_t dpll_mul;              /* the ARM's clock in MHz when the PMIC is there and the last boot was good (300 MHz otherwise) */
    uint8_t  pmic_voltage;          /* the core voltage in hundredths of a volt (135 = 1.35 V, for 1 GHz) */
    uint16_t dcache_mask_24b;       /* unused: one bit per megabyte of the 24-bit space */
    uint16_t icache_mask_24b;       /* unused */

    uint8_t  cache_index_bits;      /* unused: translation cache size = 8 << (index + block bits) */
    uint8_t  cache_block_bits;      /* unused */
    uint8_t  maprom_page;           /* unused */
    uint8_t  host_type;             /* the host recognised at the last clock measurement: an index into the system table in main.c, 0 = unknown */

    /* the MapROM image: guest base in 64 KB units, size as a power of two, where it comes from */
    uint16_t maprom_base_64k;       /* 0xF8 = $F80000 */
    uint8_t  maprom_size_log2;      /* 19 = 512 KB */
    uint8_t  maprom_src;            /* MAPROM_* */
    uint8_t  ram_mode;              /* RAM_*, unused */
    uint8_t  boot_flags;            /* BOOT_24BIT (always set when the payload is told) */

    /* the guest's RAM: host DRAM that the payload maps into the guest address space in 1 MB sections, backed by consecutive megabytes of a pool (the total with the payload's own needs
       stays under about 495 MB: the first region that does not fit ends the list). Code in a region is translated; the Amiga bus below 24 bits is interpreted; everything else faults.
       base_mb = guest address >> 20; size = 1 << size_log2 bytes (at least 20); size_log2 = 0 ends the list. */
    struct { uint16_t base_mb; uint8_t size_log2; uint8_t flags; } mem_region[4];

    uint16_t kHz;                   /* the Amiga's bus clock, for the GPMC timing */

    uint8_t  last_boot_good;        /* the last boot passed its self-test */
    uint8_t  is_dirty;              /* RAM only: the settings changed and are to be written back */
    uint8_t  pace;                  /* percent of an MC68000's speed for interpreted code (chip RAM, the Amiga window), 0 = off; in the struct's tail padding, so size and CRC do not change */
} config_t;

enum { CFG_LOADED, CFG_BLANK, CFG_BAD_CRC, CFG_NO_EEPROM };      /* what LoadConfigEEPROM found */

extern config_t settings;
extern void config_set_defaults(config_t *c);
extern int  LoadConfigEEPROM(void);         /* the settings from the EEPROM, or the defaults: CFG_* */
extern int  SaveConfigEEPROM(void);         /* writes and reads back: 0 = verified */
extern int  LoadBoardKind(void);            /* the board kind from its own EEPROM record: LANES_SWAPPER / LANES_STRAIGHT (amiga.h), -1 = none */
extern int  SaveBoardKind(int kind);        /* writes the record and reads it back: 0 = verified */

#endif
