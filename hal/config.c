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
 * 
 *     emu68 (https://github.com/michalsc), Mozilla Public License, v. 2.0
 *     Copyright © 2019 Michal Schulz <michal.schulz@gmx.de>
 */
#include <stdio.h>
#include <stdint.h>
#include "pjit.h"

static unsigned short crc16_ccitt(const void *buf, uint32_t len, uint16_t init) {
    static const unsigned short crc16_table[256]= {
        0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
        0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
        0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
        0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
        0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
        0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
        0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
        0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
        0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
        0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
        0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
        0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
        0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
        0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
        0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
        0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
        0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
        0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
        0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
        0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
        0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
        0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
        0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
        0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
        0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
        0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
        0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
        0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
        0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
        0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
        0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
        0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
    };
    uint8_t *_b = (uint8_t *)buf;
    uint16_t crc = init;
    while (len--) {
        crc = (crc << 8) ^ crc16_table[((crc >> 8) ^ *_b++) & 0x00FF];
    }
    return crc;
}

void SaveConfigEEPROM(int boot_good) {
    uint8_t addr[2] = { 0 };
    const uint8_t* config = 6 + (uint8_t*)&cpu_state.config;
    config_t i2c_config;

    cpu_state.config.is_dirty = 0;
    cpu_state.config.last_boot_good = boot_good;
    cpu_state.config.crc16 = crc16_ccitt(config, sizeof(config_t) - 8, 0xFFFF);    
    printf("[I2C0] Saving and verifying settings (CRC=%04X)\n", cpu_state.config.crc16);
    I2C0SendCmd( 0x50, addr, 2, &cpu_state.config, sizeof(config_t));
    WaitMSDMTimer(10);
    I2C0ReadCmd( 0x50, addr, 2, &i2c_config, sizeof(config_t));
    if(memcmp(&i2c_config, &cpu_state.config, sizeof(config_t))) {
        printf("[I2C0] Verify failed! EEPROM may be bad\n");
    }
}
static const char zero = 0;
static const char one = 1;

void MakeGoodEEPROM(void) {
    uint8_t addr[2] = { 0 };
    addr[1] = __offsetof(config_t, last_boot_good);
    I2C0SendCmd( 0x50, addr, 2, &zero, 1);
}

void LoadConfigEEPROM(void) {
    static config_t i2c_config;
    const uint8_t* config = 6 + (uint8_t*)&i2c_config;
    uint8_t addr[2] = { 0 };
	int err = 0;

    I2C0ReadCmd( 0x50, addr, 2, &i2c_config, sizeof(config_t));
    if(i2c_config.ident == 0x704A4954) {
        uint16_t calc_crc = crc16_ccitt(config, sizeof(config_t) - 8, 0xFFFF);
        if(calc_crc == i2c_config.crc16) {
            printf("[I2C0] Settings loaded, last boot was %s\n", i2c_config.last_boot_good ? "good" : "bad");
            // MakeGoodEEPROM();
            cpu_state.config = i2c_config;
            return;
        } else {
            printf("[I2C0] CRC fail, %04X != %04X\n", calc_crc, i2c_config.crc16);
        }
    } else {
        printf("[I2C0] Ident wrong, 0x704A4954 != 0x%08X\n", i2c_config.ident);
    }
    cpu_state.config = default_config;
    cpu_state.config.is_dirty = 1;
    printf("[I2C0] Default settings loaded\n");
}

int DetectEEPROM(void) {
    if (I2C0Probe(0x50)) {
        printf("[I2C0] EEPROM Detected ($50)\n");
        LoadConfigEEPROM();
        return 1;
    } else {
        printf("[I2C0] Default settings loaded\n");
        cpu_state.config = default_config;
        return 0;
    }
}

static const char* GetConfigCPU(void) {
    static char buffer[32] = { 0 };
    static const char *cpu;
    static const char *end = "";

    if(cpu_state.config.cpu_enable_68000) cpu = "68000";
    else if(cpu_state.config.cpu_enable_68020) {
        if(cpu_state.config.cpu_enable_32bits) {
            cpu = (cpu_state.config.cpu_enable_fpu) ? "68020/68882" : "68020";
        } else {
            cpu = (cpu_state.config.cpu_enable_fpu) ? "68EC020/68882" : "68EC020";
        }
    }
    else if(cpu_state.config.cpu_enable_68030) {
        if(!cpu_state.config.cpu_enable_mmu) cpu = "68EC030";
        else if(!cpu_state.config.cpu_enable_fpu) cpu = "68030";
        else cpu = "68030/68882";
    }
    else if(cpu_state.config.cpu_enable_68040) {
        if(!cpu_state.config.cpu_enable_mmu) cpu = "68EC040";
        else if(!cpu_state.config.cpu_enable_fpu) cpu = "68LC040";
        else cpu = "68040";
    }

    if(cpu_state.config.cpu_little_endian) end = "-LE";

    sprintf(buffer, "%s%s", cpu, end);
    return buffer;
}

static int streq(const char* l, const char* r) {
    while(*l && *r) {
        if(*l != *r) return 0;
        l++; r++;
    }
    return 1;
}

static void SetConfigCPU(const char* CPU) {
    if(streq(CPU, "68000")) {
        cpu_state.config.cpu_enable_68000 = 1;
        cpu_state.config.cpu_enable_68020 = 0;
        cpu_state.config.cpu_enable_68030 = 0;
        cpu_state.config.cpu_enable_68040 = 0;
        cpu_state.config.cpu_enable_32bits = 0;
        cpu_state.config.cpu_enable_icache = 0;
        cpu_state.config.cpu_enable_dcache = 0;
        cpu_state.config.cpu_enable_fpu = 0;
        cpu_state.config.cpu_enable_mmu = 0;
        cpu_state.config.cpu_little_endian = streq(&CPU[7 + 6 * cpu_state.config.cpu_enable_fpu], "-LE");
    }
    if(streq(CPU, "68EC020")) {
        cpu_state.config.cpu_enable_68000 = 0;
        cpu_state.config.cpu_enable_68020 = 1;
        cpu_state.config.cpu_enable_68030 = 0;
        cpu_state.config.cpu_enable_68040 = 0;
        cpu_state.config.cpu_enable_32bits = 0;
        cpu_state.config.cpu_enable_icache = 1;
        cpu_state.config.cpu_enable_dcache = 0;
        cpu_state.config.cpu_enable_fpu = streq(&CPU[7], "/68882");
        cpu_state.config.cpu_enable_mmu = 0;
        cpu_state.config.cpu_little_endian = streq(&CPU[7 + 6 * cpu_state.config.cpu_enable_fpu], "-LE");
    }
    if(streq(CPU, "68020")) {
        cpu_state.config.cpu_enable_68000 = 0;
        cpu_state.config.cpu_enable_68020 = 1;
        cpu_state.config.cpu_enable_68030 = 0;
        cpu_state.config.cpu_enable_68040 = 0;
        cpu_state.config.cpu_enable_32bits = 1;
        cpu_state.config.cpu_enable_icache = 1;
        cpu_state.config.cpu_enable_dcache = 0;
        cpu_state.config.cpu_enable_fpu = streq(&CPU[7], "/68882");
        cpu_state.config.cpu_enable_mmu = 0;
        cpu_state.config.cpu_little_endian = 0;
    }
    if(streq(CPU, "68EC030")) {
        cpu_state.config.cpu_enable_68000 = 0;
        cpu_state.config.cpu_enable_68020 = 0;
        cpu_state.config.cpu_enable_68030 = 1;
        cpu_state.config.cpu_enable_68040 = 0;
        cpu_state.config.cpu_enable_32bits = 1;
        cpu_state.config.cpu_enable_icache = 1;
        cpu_state.config.cpu_enable_dcache = 1;
        cpu_state.config.cpu_enable_fpu = streq(&CPU[7], "/68882");
        cpu_state.config.cpu_enable_mmu = 0;
        cpu_state.config.cpu_little_endian = 0;
    }
    if(streq(CPU, "68030")) {
        cpu_state.config.cpu_enable_68000 = 0;
        cpu_state.config.cpu_enable_68020 = 0;
        cpu_state.config.cpu_enable_68030 = 1;
        cpu_state.config.cpu_enable_68040 = 0;
        cpu_state.config.cpu_enable_32bits = 1;
        cpu_state.config.cpu_enable_icache = 1;
        cpu_state.config.cpu_enable_dcache = 1;
        cpu_state.config.cpu_enable_fpu = streq(&CPU[7], "/68882");
        cpu_state.config.cpu_enable_mmu = 1;
        cpu_state.config.cpu_little_endian = 0;
    }
    if(streq(CPU, "68EC040")) {
        cpu_state.config.cpu_enable_68000 = 0;
        cpu_state.config.cpu_enable_68020 = 0;
        cpu_state.config.cpu_enable_68030 = 0;
        cpu_state.config.cpu_enable_68040 = 1;
        cpu_state.config.cpu_enable_32bits = 1;
        cpu_state.config.cpu_enable_icache = 1;
        cpu_state.config.cpu_enable_dcache = 1;
        cpu_state.config.cpu_enable_fpu = 0;
        cpu_state.config.cpu_enable_mmu = 0;
        cpu_state.config.cpu_little_endian = 0;
    }
    if(streq(CPU, "68LC030")) {
        cpu_state.config.cpu_enable_68000 = 0;
        cpu_state.config.cpu_enable_68020 = 0;
        cpu_state.config.cpu_enable_68030 = 0;
        cpu_state.config.cpu_enable_68040 = 1;
        cpu_state.config.cpu_enable_32bits = 1;
        cpu_state.config.cpu_enable_icache = 1;
        cpu_state.config.cpu_enable_dcache = 1;
        cpu_state.config.cpu_enable_fpu = 0;
        cpu_state.config.cpu_enable_mmu = 1;
        cpu_state.config.cpu_little_endian = 0;
    }
    if(streq(CPU, "68030")) {
        cpu_state.config.cpu_enable_68000 = 0;
        cpu_state.config.cpu_enable_68020 = 0;
        cpu_state.config.cpu_enable_68030 = 0;
        cpu_state.config.cpu_enable_68040 = 1;
        cpu_state.config.cpu_enable_32bits = 1;
        cpu_state.config.cpu_enable_icache = 1;
        cpu_state.config.cpu_enable_dcache = 1;
        cpu_state.config.cpu_enable_fpu = 1;
        cpu_state.config.cpu_enable_mmu = 1;
        cpu_state.config.cpu_little_endian = 0;
    }

}

extern uint32_t toHex2(char* n, int len);
void ManageConfig(void) {
    char option[32] = { 0 };
    while(option[0] != 'x' && option[0] != 'X') {
        printf("%s",
            "1. Reload config from EEPROM\n"
            "2. Save config to EEPROM\n"
            "3. Display & Change config\n"
            "X. Exit to main menu\n"
            "] "
        );
        gets(option);
        switch(option[0]) {
        default: break;
        case '1': LoadConfigEEPROM(); break;
        case '2': SaveConfigEEPROM(1); break;
        case '3':
            while(1) {
                printf("Config:\n");
                printf("A.  CPU: %s\n", GetConfigCPU());
                printf("B.  Clock: %0.3f MHz\n", cpu_state.config.kHz * 0.001);
                printf("C.  I$: %s\n", cpu_state.config.cpu_enable_icache ? "Enabled" : "Disabled");
                printf("D.  I$ Mask: %04X\n", cpu_state.config.icache_mask_24b);
                printf("E.  D$: %s\n", cpu_state.config.cpu_enable_dcache ? "Enabled" : "Disabled");
                printf("F.  D$ Mask: %04X\n", cpu_state.config.dcache_mask_24b);
                printf("G.  MAPROM %s\n", cpu_state.config.maprom_page != 0xFF ? "Enabled" : "Disabled");
                if(cpu_state.config.maprom_page != 0xFF) {
                    uint32_t page = (cpu_state.config.maprom_page & 0xF8) << 16;
                    printf("    %06X~%06X\n", page, (page | 0x7FFFF));
                }
                printf("    POST:\n");
                printf("H.    Long Mem Test %s\n", cpu_state.config.post_enable_long_mem ? "Enabled" : "Disabled");
                printf("I.    GreenPAK Test %s\n", cpu_state.config.post_enable_gpack_ok ? "Enabled" : "Disabled");
                printf("J.    Bus Clock Test %s\n", cpu_state.config.post_enable_checkclk ? "Enabled" : "Disabled");
                printf("K.    Automap %s\n", cpu_state.config.post_enable_autommap ? "Enabled" : "Disabled");
                printf("L.  MCU CLK: %d\n" , cpu_state.config.dpll_mul);
                printf("M.  PMIC Voltage: %0.2f\n" , cpu_state.config.pmic_voltage * 0.01);
                printf("    PJIT Cache:\n");
                printf("N.    Block Size: %d bytes\n", 8 << cpu_state.config.cache_block_bits);
                printf("O.    Block Count: %d\n", 1 << cpu_state.config.cache_index_bits);
                uint32_t cs = 8 << (cpu_state.config.cache_index_bits + cpu_state.config.cache_block_bits);
                char* suffix = "bytes";
                if(cs > 1024 & ((cs & 1023)==0)) cs = cs / 1024, suffix = "kB";
                if(cs > 1024 & ((cs & 1023)==0)) cs = cs / 1024, suffix = "MB";
                printf("      Cache Size: %d %s\n", cs, suffix);
                printf("X.  Return to previous menu\n");
                printf("] ");
                gets(option);
                uint8_t choice = toupper(option[0]);
                if(choice == 'X') { option[0] = '?'; break; }

                printf("New value? ");
                gets(option);
                option[0] = toupper(option[0]);

                switch(choice) {
                case 'A': SetConfigCPU(option); break;
                case 'B': cpu_state.config.kHz = (int)(1000.0 * strtod(option, NULL)); break;
                case 'C': cpu_state.config.cpu_enable_icache = option[0] == 'E'; break;
                case 'D': cpu_state.config.icache_mask_24b = toHex2(option, 4); break;
                case 'E': cpu_state.config.cpu_enable_dcache = option[0] == 'E'; break;
                case 'F': cpu_state.config.dcache_mask_24b = toHex2(option, 4); break;
                case 'G': cpu_state.config.maprom_page = toHex2(option, 2); break;
                case 'H': cpu_state.config.post_enable_long_mem = option[0] == 'E'; break;
                case 'I': cpu_state.config.post_enable_gpack_ok = option[0] == 'E'; break;
                case 'J': cpu_state.config.post_enable_checkclk = option[0] == 'E'; break;
                case 'K': cpu_state.config.post_enable_autommap = option[0] == 'E'; break;
                case 'L': cpu_state.config.dpll_mul = (int)strtod(option, NULL); break;
                case 'M': cpu_state.config.pmic_voltage = (int)(100.0 * strtod(option, NULL)); break;
                case 'N': cpu_state.config.cache_block_bits = toHex2(option, 1); break;
                case 'O': cpu_state.config.cache_index_bits = toHex2(option, 1); break;
                }
            } ;
            break;
        }
    }
}