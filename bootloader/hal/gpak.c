/*
 * gpak.c -- the GreenPAK (SLG46826), the programmable logic that shapes the Amiga bus signals. Its register image (greenpak/asn_image.h, made from the design by
 * tools/buffee/gen_asn_image.py) is written into the chip's RAM at every boot, so the design that runs is always the one in this build.
 *
 * Copyright (c) 2020-2026 Renee Cousins, the Buffee Project - http://www.buffee.ca
 * Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (see LICENSE).
 */
#include <stdio.h>
#include <stdint.h>

#include "main.h"
#include "init.h"
#include "gpak.h"
#include "../greenpak/asn_image.h"

#define GP_NVM_BLOCK 2           /* the I2C address of a GreenPAK is its control code (bits 6:3) and a block (bits 2:0: 0 the registers, 2 the NVM, 3 the EEPROM).
                                   Detection probes the NVM block, which answers for every control code; the registers of control code 0 are at $00, the general call. */

#define GP_DESIGN    0xC8        /* the design's registers: 0x00-0x73 and 0x80-0xC7 (0x74-0x7F are the live matrix inputs, read only) */

static uint8_t gp_i2c = 1;      /* the detected GreenPAK's control code (0-7) */
static int gp_found;            /* DetectGP found one */
static int gp_asn_on;           /* its ASn output is enabled (GPEnableASn: the GPMC let go of the pad) */

static void gpak_write_reg(uint8_t addr, uint8_t reg, uint8_t value) {
    uint8_t data[1] = { value };
    I2C0SendCmd(addr << 3, &reg, 1, data, 1);
}

/* Writes a register image into the GreenPAK RAM (the NVM is not touched) in 16-byte pages: 0x00-0x73 and 0x80-0xC7 (0x74-0x7F are read-only: the live matrix inputs).
   The RAM survives a reset of Buffee, so every build writes its own design at boot: an image another firmware left behind is gone before anything uses the bus.
   hold_oe: write the image with the pin 20 output enable (matrix OUT91, 0x44 bits 7:2) off; GPEnableASn() writes it last.
   Returns the number of pages the GreenPAK refused (0: all written). */
static int GPWriteImage(uint8_t addr, const uint8_t* img, int hold_oe) {
    uint8_t buf[16], o;
    int off, n, k, refused = 0;

    for (off = 0; off < 0xC8; off += 16) {
        n = (off == 0x70) ? 4 : (off == 0xC0) ? 8 : 16;
        for (k = 0; k < n; k++) buf[k] = img[off + k];
        if (hold_oe && off == 0x40) buf[4] &= 0x03;
        o = (uint8_t)off;
        if (!I2C0SendCmd(addr << 3, &o, 1, buf, n)) refused++;
    }
    return refused;
}

/* The GreenPAK drives ASn: the pin 20 output is enabled once the GPMC chip-select pad has been released, and the parity gate (which clock phase ASn may fall on) is a truth table
   that is random per boot (ASNCalibrate in hal/gpmc.c chooses it). */
void GPEnableASn(void)         { gpak_write_reg(gp_i2c, ASN_REG_OE, asn_image[ASN_REG_OE]); gp_asn_on = 1; }     /* (the same in both images) */
static const uint8_t* gp_image = asn_image;     /* the image in use: asn_image_noswap on a board without the byte swapper */

/* re-initialise the GreenPAK as at boot (tests/buffee/buffee_reroll.c): the image is rewritten with pin 20's output enable off, then enabled */
void GPReloadImage(void)       { (void)GPWriteImage(gp_i2c, gp_image, 1); GPEnableASn(); }
void GPSetPhase(uint8_t table) { gpak_write_reg(gp_i2c, ASN_REG_PHASE, table); }

/* The design for a board with (0) or without (1) the byte swapper; ASn's output goes back on only if it was on (before InitGPMC the GPMC still drives the pad). */
void GPUseImage(int noswap) {
    const uint8_t* img = noswap ? asn_image_noswap : asn_image;

    if (img == gp_image) return;                                    /* already in the registers (DetectGP, GPPowerTest and GPReloadImage write gp_image) */
    gp_image = img;
    (void)GPWriteImage(gp_i2c, gp_image, 1);
    if (gp_asn_on) GPEnableASn();
}

static int GPReadBlock(uint8_t slave, uint8_t* b) {
    uint8_t o;
    int off;

    for (off = 0; off < GP_DESIGN; off += 8) {
        o = (uint8_t)off;
        if (!I2C0ReadCmd(slave, &o, 1, b + off, 8)) return 0;
    }
    return 1;
}

/* The board kind from the GreenPAK's supply: a v0.8 or later powers it from ball M1, so a 1 ms dip (GPSupplyDip) makes it load its NVM again, and every register
   that our design sets differently from the NVM goes back to the NVM's value; a v0.6 has nothing on M1 and keeps them all. The chip answers I2C during the dip either
   way (its inputs hold it up through their protection diodes), so only the read-back tells; after a reload it is silent for ~1.5 ms. The design is written again
   afterwards. Returns 1 (the board powers the GreenPAK: a byte-swapper board), 0 (it does not: straight), -1 (no answer: no GreenPAK, an I2C failure, nothing
   to compare because the registers hold what the NVM holds, or a mixed result). About 0.1 s at 100 kHz. */
int GPPowerTest(void) {
    static uint8_t before[GP_DESIGN], nvm[GP_DESIGN], after[GP_DESIGN];
    uint8_t regs = (uint8_t)(gp_i2c << 3), nvm_block = (uint8_t)((gp_i2c << 3) | GP_NVM_BLOCK);
    int a, differ = 0, back = 0, kept = 0, ms, ok;

    if (!gp_found || !GPReadBlock(regs, before) || !GPReadBlock(nvm_block, nvm)) return -1;
    for (a = 0; a < GP_DESIGN; a++) if ((a < 0x74 || a >= 0x80) && before[a] != nvm[a]) differ++;
    if (!differ) return -1;
    GPSupplyDip(1);
    for (ms = 0; ms < 50 && !I2C0Probe(nvm_block); ms++) WaitMSDMTimer(1);
    WaitMSDMTimer(2);
    ok = GPReadBlock(regs, after);
    for (a = 0; ok && a < GP_DESIGN; a++) {
        if (!(a < 0x74 || a >= 0x80) || before[a] == nvm[a]) continue;
        if (after[a] == nvm[a]) back++;
        else if (after[a] == before[a]) kept++;
    }
    (void)GPWriteImage(gp_i2c, gp_image, 1);
    if (gp_asn_on) GPEnableASn();
    info("GreenPAK supply test: a 1 ms dip put %d of %d registers back to the NVM's values, %d kept\n", back, differ, kept);
    return !ok ? -1 : back == differ ? 1 : kept == differ ? 0 : -1;
}

int DetectGP(void) {
    int i, a, refused;

    for (i = 0; i < 8; i++) {
        if (I2C0Probe((i << 3) | GP_NVM_BLOCK)) {
            gp_i2c = i;
            gp_found = 1;
            info("GreenPAK detected ($%02X~$%02X) \n", (i << 3), ((i << 3) + 3));
            refused = GPWriteImage(i, asn_image, 1);     /* its output enable (GPEnableASn) follows once the GPMC CS pad is released in InitGPMC */
            if (refused) { printf("GreenPAK at $%02X refused %d of 13 pages of the design\n", i << 3, refused); return 0; }
            info("GreenPAK RAM: ASn design loaded\n");
            return 1;
        }
    }
    printf("GreenPAK not detected; I2C devices answer at:");      /* the EEPROM is $50, the PMIC $24 */
    for (a = 1; a < 0x78; a++) if (I2C0Probe(a)) printf(" $%02X", a);
    printf("\n");
    return 0;
}
