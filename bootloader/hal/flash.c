/*
 * Copyright (c) 2020-2023 Renee Cousins, the Buffee Project - http://www.buffee.ca
 *
 * Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (see LICENSE).
 *
 * InitSPI() below is derived from the McSPI initialisation in src/bbb/am335x_spi.c of libbbb, and modified:
 *
 *     libbbb, Apache License, Version 2.0 (the text is in LICENSES/Apache-2.0.txt)
 *     Copyright 2019 University of Applied Sciences Western Switzerland / Fribourg
 *     https://gitlab.forge.hefr.ch/embsys/libbbb
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "init.h"
#include "flash.h"
#include "pinmux.h"

static uint8_t spi_xfer[256];
static uint32_t spi_conf_normal;           /* CH0CONF as InitSPI wrote it: 8-bit words at bus_speed */
int spi_bulk_div = 0;                      /* the bulk reads' clock: 0 = 48 MHz (the McSPI maximum), 1 = 24 MHz, ... (CH0CONF CLKD, granularity of powers of two) */

#include "am335x_regs.h"

#define SYSTEM_CLOCK 48000000

void InitSPI(int bus_speed) {
    /*Enable Clocks*/
    CM_PER_SPI0_CLKCTRL->BIT.MODULEMODE = 2;
    while(CM_PER_SPI0_CLKCTRL->BIT.IDLEST);

    /*Enable SPI*/
    MCSPI0_SYSCONFIG->BIT.SOFTRESET = 1;
    while(!MCSPI0_SYSSTATUS->BIT.RESETDONE);

    /*Configure clock activity and idle mode*/
    MCSPI0_SYSCONFIG->BIT.SIDLEMODE = 0; // no-idle
    MCSPI0_SYSCONFIG->BIT.CLOCKACTIVITY = 3; // both

    /*Configure module contoller*/
    MCSPI0_MODULCTRL->LONG = (
          (0 << 8)               /* fifo managed with ctrl register */
        | (0 << 7)               /* multiword disabled */
        | (1 << 4)               /* initial spi delay = 4 spi clock */
        | (0 << 3)               /* functional mode */
        | (0 << 2)               /* master mode */
        | (0 << 1)               /* use SPIEN as chip select */
        | (1 << 0)               /* multi channel mode */
        );
    /*Set fifo level to 0*/
    MCSPI0_XFERLEVEL->LONG = 0;

    /*Compute frequency flags*/
    uint32_t clkg   = 0;
    uint32_t clkd   = 0;
    uint32_t extclk = 0;

    /*calculate the frequency ratio*/
    uint32_t ratio = SYSTEM_CLOCK / bus_speed;

    /*If ratio is not a power of 2, set granularity of 1 clock cycle*/
    if ((ratio & (ratio - 1)) != 0) {
        clkg   = 1;
        clkd   = (ratio - 1) & 0xf;
        extclk = (ratio - 1) >> 4;
    } else {
        /* Compute log2 (ratio) */
        while (ratio != 1) {
            ratio /= 2;
            clkd++;
        }
    }

    spi_conf_normal = (
          (clkg << 29)           /* clock granularity */
        | (3 << 25)              /* chip select time control 2.5 cycles */
        | (0 << 21)              /* spienslv = 0 */
        | (0 << 20)              /* force = 0 */
        | (0 << 19)              /* turbo = 0 */
        | (1 << 16)              /* TX on D1, RX on D0 */
        | (0 << 14)              /* DMA transfer disabled */
        | (0 << 12)              /* TX + RX mode */
        | ((8 - 1) << 7)         /* spi word len */
        | (1 << 6)               /* spien polarity = low */
        | (clkd << 2)            /* frequency diviver */
        | (0 << 1)               /* spiclk polarity = high */
        | (0 << 0)               /* spiclk phase = odd */
        );
    MCSPI0_CH0CONF->LONG = spi_conf_normal;

    /*Configure clock ratio extender*/
    MCSPI0_CH0CTRL->BIT.EXTCLK = extclk;

    /*Configure pins*/
    CONF_SPI0_SCLK->BIT.MMODE = 0;
    CONF_SPI0_SCLK->BIT.RXACTIVE = 1;

    CONF_SPI0_D0->BIT.MMODE = 0;
    CONF_SPI0_D0->BIT.RXACTIVE = 1;
    CONF_SPI0_D0->BIT.PUTYPESEL = 1;

    CONF_SPI0_D1->BIT.MMODE = 0;
    CONF_SPI0_D1->BIT.RXACTIVE = 1;
    CONF_SPI0_D1->BIT.PUTYPESEL = 1;

    CONF_SPI0_CS0->BIT.MMODE = 0;
    CONF_SPI0_CS0->BIT.RXACTIVE = 1;
}

static uint8_t TransferSPI(uint8_t *io_buffer, uint8_t len) {
    MCSPI0_CH0CTRL->BIT.EN = 1;
    MCSPI0_CH0CONF->BIT.FORCE = 1;
    while(len--) {
        while(!MCSPI0_CH0STAT->BIT.TXS);
        MCSPI0_TX0->LONG = *io_buffer;
        while(!MCSPI0_CH0STAT->BIT.RXS);
        *io_buffer++ = MCSPI0_RX0->LONG;
    }
    MCSPI0_CH0CONF->BIT.FORCE = 0;
    MCSPI0_CH0CTRL->BIT.EN = 0;
    return 0;
}

void WaitSPI(void) {
    while(1) {
        spi_xfer[0] = 0x05;
        spi_xfer[1] = 0x00;
        TransferSPI( spi_xfer, 2 );
        if(spi_xfer[1] & 1) {
            int i=10000;
            while(i--) asm("    nop");
        } else {
            break;
        }
    }
}

void WriteAllowSPI(void) {
    uint8_t unlock = 0x06;
    TransferSPI(&unlock, 1);
    WaitSPI();
}

void EraseSPI(uint32_t addr, ERASE_SIZE_t size) {
    uint8_t cmd, len;

    switch(size) {
    default: return;
    case ERASE_4K:  cmd = 0x20; addr &= 0xFFFFF000; len = 4; break;
    case ERASE_32K: cmd = 0x52; addr &= 0xFFFF8000; len = 4; break;
    case ERASE_64K: cmd = 0xD8; addr &= 0xFFFF0000; len = 4; break;
    case ERASE_ALL: cmd = 0x60; addr = 0; len = 1; break;
    }

    WriteAllowSPI();

    spi_xfer[0] = cmd;
    spi_xfer[1] = (addr >> 16) & 0xFF;
    spi_xfer[2] = (addr >>  8) & 0xFF;
    spi_xfer[3] = (addr >>  0) & 0xFF;
    TransferSPI( spi_xfer, len );
    WaitSPI();
}

uint16_t ReadSPIID(void) {
    spi_xfer[0] = 0x90;
    spi_xfer[1] = 0;
    spi_xfer[2] = 0;
    spi_xfer[3] = 0;
    spi_xfer[4] = 0;
    TransferSPI( spi_xfer, 6 );
    return (spi_xfer[4] << 8) | spi_xfer[5];
}


void ReadSPIBlock(uint32_t addr, void* buffer, uint8_t len) {
    spi_xfer[0] = 0x03;
    spi_xfer[1] = (addr >> 16) & 0xFF;
    spi_xfer[2] = (addr >>  8) & 0xFF;
    spi_xfer[3] = (addr >>  0) & 0xFF;
    memset(&spi_xfer[4], 0, len);
    TransferSPI( spi_xfer, len + 4 );
    memcpy(buffer, &spi_xfer[4], len);
    return;
}

/* Bulk read at the full McSPI rate. The transfer is made of 32-bit SPI words (WL = 31): the bits go out MSB first, so the fast-read command 0x0B and the 24-bit address are ONE word and the
   data arrives four bytes per register access. The fast read (0x0B, one dummy byte) is the command every SPI flash accepts at 48 MHz and beyond (0x03 stops at 33-50 MHz); the dummy byte shifts
   the data by one byte in the word stream. len is a multiple of 4; the CS stays asserted for the whole read (FORCE).
   (Default 1: mode 0 stalled on the board, both as receive-only and with both FIFOs; the registers need a look with a test payload before it is the default.)
   spi_bulk_mode 0: both FIFOs (16 words each), 16-word blocks: one status poll and 32 back-to-back register accesses per 64 bytes (the byte-wise path spent 4 slow peripheral accesses per
                    word and starved the clock). (A receive-only version stalled: the master does not free-run in TRM = 1 here.) Bounded by a spin count: a stuck FIFO returns with spi_bulk_err.
   spi_bulk_mode 1: word at a time (queue the next TX word while this one shifts, poll RXS, read): the fallback, ~2.3 MB/s. */
int spi_bulk_mode = 1, spi_bulk_err = 0;                /* spi_bulk_err: a bounded wait ran out (the caller falls back to the next mode) */
#define SPI_STAT_RXS   (1u << 0)
#define SPI_STAT_TXS   (1u << 1)
#define SPI_STAT_RXFFF (1u << 6)
#define SPI_CONF_FORCE (1u << 20)
#define SPI_CONF_TURBO (1u << 19)
#define SPI_CONF_FFER  (1u << 28)
#define SPI_CONF_FFEW  (1u << 27)
#define SPI_CONF_TRM_RX (1u << 12)                               /* TRM = 1: receive only */
void ReadSPIBulk(uint32_t addr, void* buffer, uint32_t len) {
    uint32_t words = len >> 2, *out = (uint32_t*)buffer;
    spi_bulk_err = 0;
    if (!words) return;
    uint32_t base = (spi_conf_normal & ~((0x1Fu << 7) | (0xFu << 2) | (1u << 29) | (3u << 12))) | (31u << 7) | ((uint32_t)spi_bulk_div << 2) | SPI_CONF_FORCE;
    uint32_t prev = 0, w;
    if (spi_bulk_mode != 0) {
        MCSPI0_CH0CTRL->BIT.EN = 0;
        MCSPI0_CH0CONF->LONG = base | SPI_CONF_TURBO;            /* transmit + receive, the command word */
        MCSPI0_CH0CTRL->BIT.EN = 1;
        while (!(MCSPI0_CH0STAT->LONG & SPI_STAT_TXS)) ;
        MCSPI0_TX0->LONG = 0x0B000000u | (addr & 0xFFFFFFu);
        while (!(MCSPI0_CH0STAT->LONG & SPI_STAT_RXS)) ;
        (void)MCSPI0_RX0->LONG;                                  /* r0: what the flash sent while it took the command */
    }
    if (spi_bulk_mode == 0) {
        /* both FIFOs on (FFER, FFEW), transmit + receive: a block is 16 words = the 64-byte FIFO. Write the block's 16 dummy TX words (the clock starts with the first), wait for the RX FIFO to
           fill (one poll), read 16 words. The next block's TX words go in BEFORE the current RX words are read, so the clock keeps running while the CPU drains the FIFO. The stream is
           s0 (junk) s1 .. : the command word was sent by the first block's first TX word. */
        MCSPI0_CH0CTRL->BIT.EN = 0;
        MCSPI0_XFERLEVEL->LONG = 0;
        MCSPI0_CH0CONF->LONG = base | SPI_CONF_FFER | SPI_CONF_FFEW;
        MCSPI0_CH0CTRL->BIT.EN = 1;
        uint32_t T = (words + 2 + 15) & ~15u, nb = T >> 4, idx = 0;     /* stream words needed: s0 .. s(words+1); rounded up to whole blocks */
        MCSPI0_TX0->LONG = 0x0B000000u | (addr & 0xFFFFFFu);
        for (uint32_t i = 1; i < 16; i++) MCSPI0_TX0->LONG = 0;
        for (uint32_t b = 0; b < nb; b++) {
            uint32_t spin = 0;
            while (!(MCSPI0_CH0STAT->LONG & SPI_STAT_RXFFF)) if (++spin > 20000u) { spi_bulk_err = 1; goto done; }
            if (b + 1 < nb) for (uint32_t i = 0; i < 16; i++) MCSPI0_TX0->LONG = 0;
            for (uint32_t i = 0; i < 16; i++, idx++) {
                w = MCSPI0_RX0->LONG;
                if (idx >= 2 && idx - 2 < words) out[idx - 2] = __builtin_bswap32((prev << 8) | (w >> 24));
                prev = w;
            }
        }
    } else {
        uint32_t total = words + 1;                              /* r1 .. r(words+1): the data words and one more for the byte the dummy pushed along */
        while (!(MCSPI0_CH0STAT->LONG & SPI_STAT_TXS)) ;
        MCSPI0_TX0->LONG = 0;                                    /* the first data word's clocks */
        for (uint32_t k = 0; k < total; k++) {
            if (k + 1 < total) { while (!(MCSPI0_CH0STAT->LONG & SPI_STAT_TXS)) ; MCSPI0_TX0->LONG = 0; }     /* queue the next while this one shifts */
            while (!(MCSPI0_CH0STAT->LONG & SPI_STAT_RXS)) ;
            w = MCSPI0_RX0->LONG;
            if (k >= 1) out[k - 1] = __builtin_bswap32((prev << 8) | (w >> 24));
            prev = w;
        }
    }
done:
    MCSPI0_CH0CTRL->BIT.EN = 0;
    MCSPI0_CH0CONF->BIT.FORCE = 0;
    MCSPI0_XFERLEVEL->LONG = 0;
    MCSPI0_CH0CONF->LONG = spi_conf_normal;
}


void WriteSPIBlock(uint32_t addr, const void* buffer, uint8_t len) {
    WriteAllowSPI();

    spi_xfer[0] = 0x02;
    spi_xfer[1] = (addr >> 16) & 0xFF;
    spi_xfer[2] = (addr >>  8) & 0xFF;
    spi_xfer[3] = (addr >>  0) & 0xFF;
    memcpy(&spi_xfer[4], buffer, len);
    TransferSPI( spi_xfer, len + 4 );
    WaitSPI();
    return;
}

void SPITest(void) {
    const uint32_t write = 0xAA55AA55;
    const uint32_t clear = 0x00000000;
    uint32_t read = 0;
	bool all_good = true;

	uint16_t devid = ReadSPIID();
	all_good &= devid == 0x1F16;
	printf("[FLASH] Flash Device ID: %04x\n", devid);
	printf("[FLASH] Erasing page 0\n");
	EraseSPI(0, ERASE_4K);

    printf("[FLASH] Writing pattern block\n");
    for(int i = 0; i < 4; i++) WriteSPIBlock(i * 4, &write, sizeof(write));
    printf("[FLASH] Read:");
    for(int i = 0; i < 4; i++) {
        ReadSPIBlock(i * 4, &read, sizeof(read));
		all_good &= read == write;
		printf(" %08lX", read);
	}
    printf("\n");

    printf("[FLASH] Writing zero block\n");
    for(int i = 0; i < 4; i++) WriteSPIBlock(i * 4, &clear, sizeof(clear));
    printf("[FLASH] Read:");
    for(int i = 0; i < 4; i++) {
        ReadSPIBlock(i * 4, &read, sizeof(read));
		all_good &= read == clear;
		printf(" %08lX", read);
	}
    printf("\n");
	printf("[FLASH] Tests %s\n", (all_good ? "passed" : "failed"));
}

void SPIDump(void) {
    const uint32_t len = 4096;
    uint32_t data;
    uint32_t pos = 0;

    printf("[FLASH] Reading first 4KB:\n");
    while(pos < len) {
        ReadSPIBlock(pos, &data, sizeof(data));
        if((pos & 15) == 0) printf("\n%08lX:", pos);
        printf(" %08lX", data);
        pos += 4;
    }
    printf("\n");
}

void ManageSPI(void) {
    char option[4] = { 0 };
    while(option[0] != 'x' && option[0] != 'X') {
        printf("%s",
            "1. Erase SPI flash\n"
            "2. Program SPI flash\n"
            "X. Exit to main menu\n"
            "] "
        );
        gets(option);
        switch(option[0]) {
        case '1': if (confirm()) EraseSPI(0, ERASE_ALL); break;
        case '2': if (confirm()) WriteImage(&_image_start, &_image_end); break;
        }
    }
}
