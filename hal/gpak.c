/*
 * Copyright (c) 2020-2023 Renee Cousins, the Buffee Project - http://www.buffee.ca
 *
 * This is part of PJIT the Pseudo-JIT 68K emulator.
 *
 * PJIT is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * PJIT is licensed under a Creative Commons
 * Attribution-NonCommercial-ShareAlike 4.0 International License.
 *
 * Under the terms of this license you are free copy and redistribute
 * the material in any medium or format as well as remix, transform,
 * and build upon the material.
 *
 * You must give appropriate credit, provide a link to the license,
 * and indicate if changes were made. You may do so in any reasonable
 * manner, but not in any way that suggests the licensor endorses you
 * or your use.
 *
 * You may not use the material for commercial purposes.
 *
 * If you remix, transform, or build upon the material, you must
 * distribute your contributions under the same license as the original.
 *
 * You may not apply legal terms or technological measures that legally
 * restrict others from doing anything the license permits.
 *
 * Portions of PJIT have been derived from the following:
 *
 *     Castaway (formerly FAST), GPL version 2 License
 *     Copyright (c) 1994-2002 Martin Döring, Joachim Hönig
 *    
 *     Cyclone 68K, GPL version 2 License
 *     Copyright (c) 2004,2011 Dave "FinalDave" Haywood
 *     Copyright (c) 2005-2011 Graûvydas "notaz" Ignotas
 *    
 *     TI StarterWare, modified BSD 3-Clause License
 *     Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/
 *
 *     libbbb, Apache License, Version 2.0
 *     Copyright 2015 University of Applied Sciences Western Switzerland / Fribourg
 */

#include "gpak.h"

#define PAGE_LIMIT 0x0E
#define PAGE_DELAY 25

static uint8_t _bus;
static uint8_t _slave_address;

void gpak_init(uint8_t bus, uint8_t slave_address) {
    _bus = bus;
    _slave_address = slave_address;
}

uint8_t gpak_i2caddr(uint8_t area) {
    if(!_slave_address) return 0x88;
    else return (_slave_address << 3) + area;
}

void gpak_read(uint8_t area, uint8_t offset, uint8_t* data, uint16_t length) {
    uint8_t addr = gpak_i2caddr(area);
    while(length > 0) {
        uint8_t xferlen = (length > 16) ? 16 : length;
        am335x_i2c_read( AM335X_I2C0, addr, offset, data, xferlen);
        offset += xferlen;
        length -= xferlen;
        data += xferlen;
    }
}

void gpak_write(uint8_t area, uint8_t offset, uint8_t* data, uint16_t length) {
    uint8_t addr = gpak_i2caddr(area);
    //if((offset + length) >= (PAGE_LIMIT << 4)) return;
    while(length > 0) {
        uint8_t xferlen = (length > 16) ? 16 : length;
        am335x_i2c_write( AM335X_I2C0, addr, offset, data, xferlen);
        am335x_dmtimer1_wait_ms(PAGE_DELAY);
        offset += xferlen;
        length -= xferlen;
        data += xferlen;
    }
}

void gpak_write_reg(uint8_t reg, uint8_t value) {
    uint8_t addr = gpak_i2caddr(REG_CONFIG);
    uint8_t data[1] = { value };
    am335x_i2c_write( AM335X_I2C0, addr, reg, data, 1);
}

uint8_t gpak_read_reg(uint8_t reg) {
    uint8_t addr = gpak_i2caddr(REG_CONFIG);
    uint8_t data[1] = { 0 };
    am335x_i2c_read( AM335X_I2C0, addr, reg, data, 1);
    return data[0];
}

void gpak_erase(uint8_t area) {
    uint8_t addr = gpak_i2caddr(area);
    uint8_t data[1] = { 0 };
    for(int i=0; i<16; i++) {
        //if(i >= PAGE_LIMIT) break;
        if(area == NVM_CONFIG) data[0] = 0x80 + i;
        else if(area == EEPROM) data[0] = 0x90 + i;
        am335x_i2c_write( AM335X_I2C0, addr, 0xE3, data, 1);
        am335x_dmtimer1_wait_ms(PAGE_DELAY);
    }
}
