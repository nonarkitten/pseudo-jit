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

#include <arm_neon.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef __PJIT_H__
#define __PJIT_H__

// PJIT Better Register Allocation
// --ARM-- 68K USE
// ------- --- ------
// R0      T0  68K PC, temp
// R1      T1  Src Ext Addr, temp
// R2      T2  Dest Ext Addr, temp

// R3      D0  Data registers
// R4      D1   "

// R5      CPU CPU state pointer

// R6      A0  Address registers
// R7      A1   "
// R8      A2   "
// R9      A3   "
// R10     A4   "
// R11     A5   "
// R12     A6   "

// R13 SP  SP* Stack Pointer
// R14 LR  N/A Link Register
// R15 PC  N/A ARM Program Counter

// D0      FP0
// D1      FP1
// D2      FP2
// D3      FP3
// D4      FP4
// D5      FP5
// D6      FP6
// D7      FP7

// D8-D15  Unused/Spare

typedef enum {
    RESET_SP,       // 0  Initial stack pointer
    RESET_PC,       // 1  Inital program counter
    BUSERR,         // 2  Bus error
    ADDRESSERR,     // 3  Address error
    ILLINSTR,       // 4  Illegal instruction
    DIVZ,           // 5  Division by zero
    TRAPCHK,        // 6  CHK, CHK2 instruction
    TRAPV,          // 7  cpTRAPcc, TRAPcc, TRAPV instructions
    PRIV,           // 8  Privillege violation
    TRACE,          // 9  Trace emulation
    LINE_A,         // 10 Unhandled Line 1010 Emulator
    LINE_F,         // 11 Unhandled Line 1111 Emulation
    RESERVED_12,    // 12 Unassigned
    COPROPV,        // 13 Coprocessor protocol violation
    FORMATERR,      // 14 Format error
    UNINITIRQ,      // 15 Uninitialized interrupt
    RESERVED_16,    // 16 Unassigned
    RESERVED_17,    // 17 Unassigned
    RESERVED_18,    // 18 Unassigned
    RESERVED_19,    // 19 Unassigned
    RESERVED_20,    // 20 Unassigned
    RESERVED_21,    // 21 Unassigned
    RESERVED_22,    // 22 Unassigned
    RESERVED_23,    // 23 Unassigned
    SPURIOUS,       // 24 Spurious interrupt
    AUTOINT1,       // 25 Level 1 Autovector (IPL)
    AUTOINT2,       // 26 Level 2 Autovector (IPL)
    AUTOINT3,       // 27 Level 3 Autovector (IPL)
    AUTOINT4,       // 28 Level 4 Autovector (IPL)
    AUTOINT5,       // 29 Level 5 Autovector (IPL)
    AUTOINT6,       // 30 Level 6 Autovector (IPL)
    AUTOINT7,       // 31 Level 7 Autovector (IPL)
    TRAP0,          // 32 TRAP #0 instruction
    TRAP1,          // 33 TRAP #1 instruction
    TRAP2,          // 34 TRAP #2 instruction
    TRAP3,          // 35 TRAP #3 instruction
    TRAP4,          // 36 TRAP #4 instruction
    TRAP5,          // 37 TRAP #5 instruction
    TRAP6,          // 38 TRAP #6 instruction
    TRAP7,          // 39 TRAP #7 instruction
    TRAP8,          // 40 TRAP #8 instruction
    TRAP9,          // 41 TRAP #9 instruction
    TRAP10,         // 42 TRAP #10 instruction
    TRAP11,         // 43 TRAP #11 instruction
    TRAP12,         // 44 TRAP #12 instruction
    TRAP13,         // 45 TRAP #13 instruction
    TRAP14,         // 46 TRAP #14 instruction
    TRAP15,         // 47 TRAP #15 instruction
    FPUNORDERED,    // 48 FPCP Branch or Set on Unordered
    FPINEXACT,      // 49 FPCP Inexact result
    FPDIVZ,         // 50 FPCP Division by zero
    FPUNDERFLOW,    // 51 FPCP Underflow
    FPOPERROR,      // 52 FPCP Operand error
    FPOVERFLOW,     // 53 FPCP Overflow
    FPSNAN,         // 54 FPCP Signalling NAN
    RESERVED_55,    // 55 Unassigned
    PMMUCONFIG,     // 56 PMMU Configuration
    PMMUILLEGALOP,  // 57 PMMU Illegal operation
    PMMUACCESSV,    // 58 PMMU Access level violation
} vector_t;

typedef enum {
    cpu_enable_68020  = 0x01,  // Enable 68020/030 Instructions
    cpu_enable_32bits = 0x02,  // Enable 32-bit Addressing
    cpu_enable_68882  = 0x04,  // Enable 68882 FPU Instructions
    cpu_enable_040fpu = 0x08,  // Enable 68040 FPU Instructions
    cpu_enable_040mmu = 0x10,  // Enable MMU Emulation
    cpu_enable_icache = 0x40,  // Enable Instruction Cache
    cpu_enable_dcache = 0x80,  // Enable Data Cache
} cpu_feature_t;

typedef enum {
    post_enable_long_mem = 0x01,  // Enable Long Memory Test
    post_enable_gpack_ok = 0x04,  // Enable GreenPAK Integrity Test
    post_enable_checkclk = 0x10,  // Enable Host CLK Test
    post_enable_autommap = 0x40,  // Enable Auto MemMap for Host Type
} post_enable_t;

typedef struct {
    // Header for EEPROM integrity check
    uint32_t ident;  // must be 0x704A4954
    uint16_t index;  // must increment from last
    uint16_t crc16;  // must check rest of config

    // BOOTLOADER SECTION

    // see cpu_feature_t
    uint8_t cpu_features;

    // see post_enable_t
    uint8_t post_enable_t;

    // CPU clock = 24 * dpll_mul / dpll_div
    uint16_t dpll_mul;
    uint8_t  dpll_div;

    // Decivolts
    uint8_t pmic_voltage;

    // Each bit indicates 1MB
    uint16_t dcache_mask_24b;  // 1 to allow data caching of region
    uint16_t icache_mask_24b;  // 1 to allow instruction caching

    uint8_t last_boot_good;

    // PJIT SECTION

    // Cache size = 8 << (cache_index_bits + cache_block_bits)
    uint8_t cache_index_bits;
    uint8_t cache_block_bits;

    // MapROM page from 24-bit RAM (single 512KB), 0xFF to disable
    uint8_t maprom_page;

} config_t;

typedef struct {
    // this is replace with a branch to the lookup routine
    uint32_t b_lookup;

    // offsets 0 thru 7
    uint32_t d0;
    uint32_t d1;
    uint32_t d2;
    uint32_t d3;
    uint32_t d4;
    uint32_t d5;
    uint32_t d6;
    uint32_t d7;

    // offsets 8 thru 15
    uint32_t a0;
    uint32_t a1;
    uint32_t a2;
    uint32_t a3;
    uint32_t a4;
    uint32_t a5;
    uint32_t a6;
    uint32_t a7;

    uint32_t sfc;    // 16 Source Function Code
    uint32_t usp;    // 17 User Stack Pointer
    uint32_t dfc;    // 18 Destination Function Code
    uint32_t vbr;    // 19 Vector Base Register
    uint32_t cacr;   // 20 Cache Control Register
    uint32_t caar;   // 21 Cache Address Register
    uint32_t tc;     // 22 MMU Translation Control Register
    uint32_t msp;    // 23 Master Stack Pointer
    uint32_t itt0;   // 24 Instruction Transparent Translation Register 0
    uint32_t isp;    // 25 Interrupt Stack Pointer
    uint32_t itt1;   // 26 Instruction Transparent Translation Register 1
    uint32_t mmusr;  // 27 MMU Status Register
    uint32_t dtt0;   // 28 Data Transparent Translation Register 0
    uint32_t urp;    // 29 User Root Pointer
    uint32_t dtt1;   // 30 Data Transparent Translation Register 1
    uint32_t srp;    // 31 Supervisor Root Pointer

    // Status register
    uint16_t sr;

    // 68K Program Counter
    uint16_t* pc;

    // System configuration
    config_t* config;

    // Pointers to everything
    uint32_t* cache_data;
    uint16_t* cache_tags;
    uint32_t* maprom_data;
    uint32_t* opcode_table;
    uint32_t* opcode_stubs;
    void*     pjit_core;

    uint16_t free_memory;  // in MB

    // The M68K address of the current cache page
    uint32_t m68k_page;

    /* FPU regs: */
    double   fp0;
    double   fp1;
    double   fp2;
    double   fp3;
    double   fp4;
    double   fp5;
    double   fp6;
    double   fp7;
    uint32_t fpsr;
    uint16_t fpcr;
} cpu_t;

#define OFFSETOF(TYPE, ELEMENT) ((size_t) & (((TYPE*)0)->ELEMENT))

extern config_t config;
extern cpu_t    cpu_state;
register cpu_t* cpu asm("r5");

extern void cpu_start(void* base);

#endif