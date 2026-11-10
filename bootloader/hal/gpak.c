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
#include "gpak.h"
#include "../greenpak/asn_image.h"

static uint8_t gp_i2c = 1;      /* the detected GreenPAK's I2C address (0-7) */

static void gpak_write_reg(uint8_t addr, uint8_t reg, uint8_t value) {
    uint8_t data[1] = { value };
    I2C0SendCmd(addr << 3, &reg, 1, data, 1);
}

/* Writes a register image into the GreenPAK RAM (the NVM is not touched) in 16-byte pages: 0x00-0x73 and 0x80-0xC7 (0x74-0x7F are read-only: the live matrix inputs).
   The RAM survives a reset of Buffee, so every build writes its own design at boot: an image another firmware left behind is gone before anything uses the bus.
   hold_oe: write the image with the pin 20 output enable (matrix OUT91, 0x44 bits 7:2) off; GPEnableASn() writes it last. */
static void GPWriteImage(uint8_t addr, const uint8_t* img, int hold_oe) {
    uint8_t buf[16];
    for (int off = 0; off < 0xC8; off += 16) {
        int n = (off == 0x70) ? 4 : (off == 0xC0) ? 8 : 16;
        for (int k = 0; k < n; k++) buf[k] = img[off + k];
        if (hold_oe && off == 0x40) buf[4] &= 0x03;
        uint8_t o = (uint8_t)off;
        I2C0SendCmd(addr << 3, &o, 1, buf, n);
    }
}

/* The GreenPAK drives ASn: the pin 20 output is enabled once the GPMC chip-select pad has been released, and the parity gate (which clock phase ASn may fall on) is a truth table
   that is random per boot (ASNCalibrate in hal/gpmc.c chooses it). */
void GPEnableASn(void)         { gpak_write_reg(gp_i2c, ASN_REG_OE, asn_image[ASN_REG_OE]); }
/* re-initialise the GreenPAK as at boot (tests/buffee/buffee_reroll.c): the image is rewritten with pin 20's output enable off, then enabled */
void GPReloadImage(void)       { GPWriteImage(gp_i2c, asn_image, 1); GPEnableASn(); }
void GPSetPhase(uint8_t table) { gpak_write_reg(gp_i2c, ASN_REG_PHASE, table); }

int DetectGP(void) {
    for (int i = 1; i < 8; i++) {
        if (I2C0Probe(i << 3)) {
            gp_i2c = i;
            printf("GreenPAK Detected ($%02X~$%02X) \n", (i << 3), ((i << 3) + 3));
            GPWriteImage(i, asn_image, 1);     /* its output enable (GPEnableASn) follows once the GPMC CS pad is released in InitGPMC */
            printf("GreenPAK RAM: ASn design loaded\n");
            return 1;
        }
    }
    printf("GreenPAK Not Detected\n");
    return 0;
}
