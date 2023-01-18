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
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#ifndef __HW_FLASH_H__
#define __HW_FLASH_H__

typedef enum {
    ERASE_4K,
    ERASE_32K,
    ERASE_64K,
    ERASE_ALL
} ERASE_SIZE_t;

/*Wait for last action to complete*/
extern void WaitSPI(void);
/*Perform a block/chip erase*/
extern void EraseSPI(uint32_t addr, ERASE_SIZE_t size);
/*Read one byte on SPI*/
extern uint8_t ReadSPI(uint32_t addr);
/*Read Device ID from SPI*/
extern uint16_t ReadSPIID(void);
/*Write one byte on SPI*/
extern void WriteSPI(uint32_t addr, uint8_t data);
/*Read a whole block from SPI*/
extern void ReadSPIBlock(uint32_t addr, void* buffer, uint8_t len);
/*Write a whole block to SPI*/
extern void WriteSPIBlock(uint32_t addr, const void* buffer, uint8_t len);

extern void SPIInit(int bps);
extern void SPITest(void);
extern void SPIDump(void);

#endif