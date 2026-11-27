/* mock_board.c -- the board's side of platform/buffee/control.c on the host (amiga_host.c): guest memory is the harness's array, the SPI flash a 16 MB array that only
 * clears bits when programmed and fills 64 KB with 0xFF when erased, the EEPROM a 32 KB array on a pretend I2C bus. The bootloader's own store.c, config.c and settings_set.c
 * run on top of them unchanged.
 *
 * Copyright (c) 2026 the Buffee Project - http://www.buffee.ca
 * Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (see LICENSE).
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include "control.h"
#include "flash.h"

#define FLASH_SIZE  0x1000000u
#define EEPROM_SIZE 0x8000u
extern uint8_t mem[];
static uint8_t flash[FLASH_SIZE], eeprom[EEPROM_SIZE], amiga_rom[0x100000], stage[BFC_STAGE_MAX];
static const char *flash_file, *eeprom_file;
static int has_rom;
static uint32_t rom_size;
jmp_buf mock_reboot;
volatile int uart_break;
int spi_bulk_div, spi_bulk_mode, spi_bulk_err;
static int pace = 100, cache = BFC_CACHE_L1I | BFC_CACHE_L1D | BFC_CACHE_L2 | BFC_CACHE_SLOTS | BFC_CACHE_BLOCKS, mhz = 1000;
static uint8_t bootinfo[64] = { 'B', 'O', 'O', 'T' };

static void load(const char *f, uint8_t *dst, uint32_t n, uint8_t fill) { memset(dst, fill, n); FILE *h = f ? fopen(f, "rb") : 0; if (h) { size_t r = fread(dst, 1, n, h); (void)r; fclose(h); } }
static void save(const char *f, const uint8_t *src, uint32_t n) { FILE *h = f ? fopen(f, "wb") : 0; if (h) { fwrite(src, 1, n, h); fclose(h); } }

void mock_board_load(const char *f, const char *e, const char *rom)
{
    flash_file = f; eeprom_file = e;
    load(f, flash, FLASH_SIZE, 0xFF);
    load(e, eeprom, EEPROM_SIZE, 0xFF);
    if (rom) { FILE *h = fopen(rom, "rb"); if (h) { rom_size = (uint32_t)fread(amiga_rom, 1, sizeof amiga_rom, h); fclose(h); has_rom = 1; } }
    bootinfo[36] = 0;                                                   /* booted from the backup slot */
}
void mock_board_save(void) { save(flash_file, flash, FLASH_SIZE); save(eeprom_file, eeprom, EEPROM_SIZE); }

/* ---- control.c's board side ---- */
void ctl_guest_read(uint32_t a, void *dst, uint32_t n) { for (uint32_t i = 0; i < n; i++) ((uint8_t *)dst)[i] = mem[(a + i) & 0xFFFFFF]; }
void ctl_guest_write(uint32_t a, const void *src, uint32_t n) { for (uint32_t i = 0; i < n; i++) mem[(a + i) & 0xFFFFFF] = ((const uint8_t *)src)[i]; }
void ctl_console(char c) { if (c != '\r') fputc(c, stderr); }
uint8_t *ctl_stage(void) { return stage; }
const volatile uint8_t *ctl_bootinfo(void) { return bootinfo; }
void ctl_hw_init(void) { }
int ctl_amiga_rom(uint32_t base, uint32_t size, uint8_t *dst)
{
    (void)base;
    if (!has_rom) { memset(dst, 0xFF, size); return 0; }
    for (uint32_t i = 0; i < size; i++) dst[i] = amiga_rom[i % (rom_size ? rom_size : 1)];
    return 0;
}
void ctl_reboot(void) { longjmp(mock_reboot, 1); }
int ctl_pace(int p) { if (p >= 0) pace = p; return pace; }
int ctl_clock(int m) { if (m > 1000) { int ctl_printf(const char *, ...); ctl_printf("At most 1000 MHz\n"); return -2; } if (m > 0) mhz = m; return mhz; }
int ctl_cache(unsigned s, unsigned c) { cache = (int)((((unsigned)cache | s) & ~c) | BFC_CACHE_SLOTS | BFC_CACHE_BLOCKS); return cache; }   /* (as on the board: translation stays) */
void ctl_board_status(void) { int ctl_printf(const char *, ...); ctl_printf("Bus       simulated (tests/amiga/mock_board.c)\n"); }

/* ---- the SPI flash (bootloader/inc/flash.h) ---- */
void InitSPI(int bps) { (void)bps; }
void WaitSPI(void) { }
uint16_t ReadSPIID(void) { return 0xEF17; }
void EraseSPI(uint32_t addr, ERASE_SIZE_t size)
{
    uint32_t n = size == ERASE_4K ? 0x1000u : size == ERASE_32K ? 0x8000u : size == ERASE_64K ? 0x10000u : FLASH_SIZE;
    memset(flash + ((addr & (FLASH_SIZE - 1)) & ~(n - 1)), 0xFF, n);
}
void ReadSPIBlock(uint32_t addr, void *buf, uint8_t len) { for (uint32_t i = 0; i < len; i++) ((uint8_t *)buf)[i] = flash[(addr + i) & (FLASH_SIZE - 1)]; }
void ReadSPIBulk(uint32_t addr, void *buf, uint32_t len) { for (uint32_t i = 0; i < len; i++) ((uint8_t *)buf)[i] = flash[(addr + i) & (FLASH_SIZE - 1)]; spi_bulk_err = 0; }
void WriteSPIBlock(uint32_t addr, const void *buf, uint8_t len)       /* a page program: bits only go from 1 to 0, within one 256-byte page */
{
    for (uint32_t i = 0; i < len; i++) { uint32_t a = (addr & ~0xFFu) | ((addr + i) & 0xFFu); flash[a & (FLASH_SIZE - 1)] &= ((const uint8_t *)buf)[i]; }
}

/* ---- the I2C bus with the EEPROM at $50 (bootloader/inc/init.h) ---- */
int32_t I2C0Probe(uint32_t slave) { return slave == 0x50; }
int32_t I2C0SendCmd(uint32_t slave, const uint8_t *cmd, uint32_t cmd_len, const uint8_t *data, uint32_t n)
{
    if (slave != 0x50 || cmd_len != 2) return 0;
    uint32_t a = (uint32_t)cmd[0] << 8 | cmd[1];
    for (uint32_t i = 0; i < n; i++) eeprom[(a + i) % EEPROM_SIZE] = data[i];
    return 1;
}
int32_t I2C0ReadCmd(uint32_t slave, const uint8_t *cmd, uint32_t cmd_len, uint8_t *data, uint32_t n)
{
    if (slave != 0x50 || cmd_len != 2) return 0;
    uint32_t a = (uint32_t)cmd[0] << 8 | cmd[1];
    for (uint32_t i = 0; i < n; i++) data[i] = eeprom[(a + i) % EEPROM_SIZE];
    return 1;
}
void WaitMSDMTimer(uint32_t ms) { (void)ms; }
