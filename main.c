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

#include "main.h"

uint32_t xm_start = 0, xm_end = 0;
extern char _image_start, _image_end;

// Slave Address for GreenPAK
#define SLAVE_ADDRESS 1
#define NVM_CONFIG 0x02
#define NVM_BYTES  240

#define NAK     0x15
#define CANCEL  0x18

uint16_t i2c_address = 0;

// Mask determines all the settable (not reserved) bits
//                      ↓↓ 0 1 2 3 4 5 6 7 8 9 a b c d e f
const char nvmMask0[]  = "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF";
const char nvmMask1[]  = "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF";
const char nvmMask2[]  = "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF";
const char nvmMask3[]  = "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF";
const char nvmMask4[]  = "FFFFFFFFFFFFFFFF0000000000000000";
const char nvmMask5[]  = "00000000000000000000000000000000";
const char nvmMask6[]  = "03FF7F00FFFF7F7F1E1EFEFE7F007F7F";
const char nvmMask7[]  = "7F7F7F7FFFFFFFFFFFFFFFFFFFFFFFFF";
const char nvmMask8[]  = "FFFFFF7E7F00000C130000FFFF380800";
const char nvmMask9[]  = "FFFFFFFFFFFFFFFFFFFFFF7FFF7F0000";
const char nvmMask10[] = "FFFFFFFFE7FFFFFFFFFFFFFFFFFF6FFF";
const char nvmMask11[] = "FEFFFFFFFFFFFF6BFFFEFFFFFFFFFFFF";
const char nvmMask12[] = "6BFFFFFFFF03FFFF06FFFFFF00000000";
const char nvmMask13[] = "00000000000000000000000000000000";
const char nvmMask14[] = "00000000000000000000000000000000";
//                        ↑↑ 0 1 2 3 4 5 6 7 8 9 a b c d e f

const char* const nvmMaskString[15] = {
    nvmMask0,  nvmMask1,  nvmMask2,  nvmMask3,
    nvmMask4,  nvmMask5,  nvmMask6,  nvmMask7,
    nvmMask8,  nvmMask9,  nvmMask10, nvmMask11,
    nvmMask12, nvmMask13, nvmMask14, //nvmMask15
};

// Store nvmData in to save on RAM
//                        ↓↓ 0 1 2 3 4 5 6 7 8 9 a b c d e f
const char nvmString0[]  = "00702007D230CAC2240B030000000000";
const char nvmString1[]  = "00000000000000000090FE24FABB0000";
const char nvmString2[]  = "000000000D0018401900000000000000";
const char nvmString3[]  = "000000D23F3DD00F00110A0000000000";
const char nvmString4[]  = "00000000000000000000000000000000";
const char nvmString5[]  = "00000000000000000000000000000000";
const char nvmString6[]  = "00010000808000710000808000000000";
const char nvmString7[]  = "00000000000000000000000000000000";
const char nvmString8[]  = "00400000001422300C00000000000000";
const char nvmString9[]  = "D0EB0000EAEA00000000000000000000";
const char nvmString10[] = "0000002000010008D0020100C0020401";
const char nvmString11[] = "0000020100000200010200A0011000A0";
const char nvmString12[] = "000102FE000001000000010100000000";
const char nvmString13[] = "00000000000000000000000000000000";
const char nvmString14[] = "00000100010000000000000000000000";
//                        ↑↑ 0 1 2 3 4 5 6 7 8 9 a b c d e f

const char* const nvmString[15] = {
    nvmString0,  nvmString1,  nvmString2,  nvmString3,
    nvmString4,  nvmString5,  nvmString6,  nvmString7,
    nvmString8,  nvmString9,  nvmString10, nvmString11,
    nvmString12, nvmString13, nvmString14, //nvmString15
};

static uint8_t nvmData[NVM_BYTES] = { 0 };
static uint8_t nvmMask[NVM_BYTES] = { 0 };

static uint8_t toHex2(char*n) {
    static const char TOHEX[32] = {
        0, 10, 11, 12, 13, 14, 15,
        0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
        0,  0,  0,  0,  0,  0, 
    };
    return (TOHEX[n[0] & 0x1F] << 4) | TOHEX[n[1] & 0x1F];
}

static void initNvm(void) {
    for(int i=0; i<(NVM_BYTES/16); i++) {
        for(int b=0; b<16; b++) {
            nvmData[(i<<4) + b] = toHex2(nvmString[i] + (b<<1));
            nvmMask[(i<<4) + b] = toHex2(nvmMask[i] + (b<<1));
        }
    }
    nvmData[0xCA] = SLAVE_ADDRESS;
}

////////////////////////////////////////////////////////////////////////////////
// readChip 
////////////////////////////////////////////////////////////////////////////////
static int readChip(int dump) {
    uint8_t data[NVM_BYTES];
    int same = 1;
    gpak_read(NVM_CONFIG, 0, data, NVM_BYTES);
    for(int page=0; page<(NVM_BYTES/16); page++) {
        for(int b=0; b<16; b++) {
            uint8_t m = nvmMask[(page << 4) | b];
            same &= (nvmData[(page << 4) | b] & m)
                == (data[(page << 4) | b] & m);
        }
    }
    return same;
}

static void programChip(int dump) {
    gpak_erase(NVM_CONFIG);
    gpak_write_reg(0xC8, 0x02); // Reset
    am335x_dmtimer1_wait_us(300);

    gpak_write(NVM_CONFIG, 0, nvmData, NVM_BYTES);
    gpak_write_reg(0xC8, 0x02); // Reset
    am335x_dmtimer1_wait_us(300);
}

static void get_i2c_config(void) {
	uint32_t ident = 0;
	int16_t last_index = -1;
	int16_t index = 0;
	int save_required = 1;

	printf("[I2C0] 101_0000 ($50) EEPROM Detected");

	// fast scan for last incermenting address
	while(1) {
		am335x_i2c_read( AM335X_I2C0, 0x50, i2c_address, &ident, sizeof(ident));
		if(ident != 0x704A4954) break;
		am335x_i2c_read( AM335X_I2C0, 0x50, i2c_address + sizeof(ident), &index, sizeof(index));
		if(index != (last_index + 1)) break;
		last_index = index;
		i2c_address += 0xFF;
	}

	// found the last page with an incrementing index
	// verify this page, and if it fails, rewind
	while(i2c_address) {
		//am335x_i2c_read( AM335X_I2C0, 0x50, i2c_address, &config, sizeof(config_t));
		uint16_t offset = 8;
		uint16_t file_crc;
		uint16_t calc_crc = 0xFFFF;
		am335x_i2c_read( AM335X_I2C0, 0x50, i2c_address, &file_crc, sizeof(file_crc));
		while(offset < sizeof(config_t)) {
			uint16_t data;
			am335x_i2c_read( AM335X_I2C0, 0x50, i2c_address, &data, sizeof(data));
			calc_crc = crc16_ccitt(calc_crc, &data, sizeof(data));
		}
		if(calc_crc == file_crc) { save_required = 0; break; }
		i2c_address -= 0xFF;
	}

	if(save_required) {
		config.last_boot_good = 0;
		i2c_address = 0; // sanity
		am335x_i2c_write( AM335X_I2C0, 0x50, i2c_address, &config, sizeof(config_t));
		printf(", defaults loaded.\n");
	} else {
		am335x_i2c_read( AM335X_I2C0, 0x50, i2c_address, &config, sizeof(config_t));
		printf(", settings loaded.\n");
	}
	if(config.last_boot_good) {
		printf("[I2C0] Last boot was good.");
	}
}

static void check_greenpak(void) {
	gpak_init(0, 1);
	printf("[I2C0] 000_10xx ($8~$A) GreenPAK Detected");
	if (config.post_enable_t & post_enable_gpack_ok) {
		initNvm();
		
		if(!readChip(0)) {
			programChip(0);
			if(!readChip(0)) printf(", bad chip!\n");
		}
		printf(", image OK.\n");
	} else {
		printf(".\n");
	}
}

static void make_i2c_good(void) {
	uint8_t good = 0x5A;
	uint16_t addr = i2c_address + OFFSETOF(config_t, last_boot_good);
	am335x_i2c_write( AM335X_I2C0, 0x50, addr, &good, 1);
}

static void ddr_test(int long_test) {
	uint32_t* ddr = (uint32_t*)0x80000000;
	int passed = 1;

	if(long_test) {
		while(ddr < (uint32_t*)0xA0000000) {
			for(int i=0; i<16; i++) {
				*ddr = (0x11111111 * i);
				asm("dsb" ::: "memory");
				asm("isb" ::: "memory");
				if(*ddr != (0x11111111 * i)) passed = 0;
			}
			ddr += 1;
		}
	} else {
		while(ddr < (uint32_t*)0xA0000000) {
			for(int i=0; i<16; i++) {
				ddr[i] = (0x11111111 * i);
			}
			asm("dsb" ::: "memory");
			asm("isb" ::: "memory");
			for(int i=0; i<16; i++) {
				if(ddr[i] != (0x11111111 * i)) passed = 0;
			}
			ddr += 32;
		}
	}
}

static void check_clock(void) {
	am335x_dmtimer1_wait_ms(10);
	uint32_t* pru_timer = am335x_pru_mem_base(PRU0_DRAM) + PRU_TIMER_OFFSET;
	uint32_t pru_counts = *pru_timer;
	double time_since_boot = am335x_dmtimer1_get_time();
	double ticks_per_s = (double)pru_counts / time_since_boot;
	printf("[CLKC] Clock read at %0.3fMHz\n", ticks_per_s);
}

static void print_menu(void) {
    printf(
		"MENU\n"
		"E. Erase all SPI flash.\n"
		"P. Write SRAM to SPI flash.\n"
		"X. Reboot.\n"
		"Ready.\n] "
	);
}

static int confirm(void) {
    char query[32];
    printf("Are you sure [y/N]? ");
    gets(query);
    return (query[0] == 'y' || query[0] == 'Y');
}

// make this a UART interrupt...?
void monitor_break(void) {
	static char option[32];
	switch(gets(option)[0]) {
	case 'E': case 'e':
		if(confirm()) EraseSPI(0, ERASE_ALL);
		else printf("Erase cancelled.\n");
		break;
	case 'P': case 'p':
		if(confirm()) {
			uint32_t a = (uint32_t)&_image_start;
			uint32_t l = (uint32_t)(&_image_end - &_image_start);
			WriteLoaderImage(a, l);
		} else printf("Write cancelled.\n");
		break;
	case 'X': case 'x':
		if(confirm()) {
			*(volatile uint32_t*)0x44E00F00 = __builtin_bswap32(2);
		}
		else printf("Reset cancelled.\n");
		break;
	default:
		printf("Command not understood...\n");
		print_menu();
	}
}	

int main(void) {
	// 1. initialize all hardware including:
	//    I2C, PMIC, clocks, EEPROM, DRAM, GPMC, GreenPAK, SPI and UART
	// 2. verify basic operating state of the system 
	//    i.e. Power-On Self Test or POST
    am335x_clock_init_core_pll();
    am335x_clock_init_per_pll();

	// Todo, assert RESET

    am335x_dmtimer1_init();
    InitPRU();

    InitUART( 115200 );

	// flush XMODEM
	printf("%c%c%c%c", NAK, CANCEL, CANCEL, CANCEL);
	setecho(1);

    printf("\n\n              ____ ______________\n");
    printf("  ______     |    |   \\__    ___/\n");
    printf("  \\  __ \\    |    |   | |    |\n");
    printf("  |  |_) )\\__|    |   | |    |\n");
    printf("  |   __/\\________|___| |____|\n");
    printf("  |__|\n\n");
    
    printf("[BOOT] Build Date %s, Time %s\n", __DATE__, __TIME__);
    printf("[BOOT] Image %p ~ %p (%d bytes)\n", &_image_start, &_image_end, (&_image_end - &_image_start));

    am335x_clock_enable_i2c_module(AM335X_CLOCK_I2C0);
    I2C0Init( 400000 );
    SPIInit( 12000000 );

    cpu = &cpu_state;
    cpu->config = &config;

	if(am335x_i2c_probe( AM335X_I2C0, 0x50)) get_i2c_config();

	if(am335x_i2c_probe( AM335X_I2C0, 0x8) 
	&& am335x_i2c_probe( AM335X_I2C0, 0x9) 
	&& am335x_i2c_probe( AM335X_I2C0, 0xA) 
	&& am335x_i2c_probe( AM335X_I2C0, 0xB)) check_greenpak();

    if(am335x_i2c_probe( AM335X_I2C0, 0x24) && config.last_boot_good) {
		double v = 0.01 * config.pmic_voltage;
        InitPower(RAIL_DCDC2, v);
        am335x_clock_init_mpu_pll(config.dpll_mul, config.dpll_div);
    } else {
        am335x_clock_init_mpu_pll(300, 24);
    }

    InitGPMC();

	DDRInit();	
	ddr_test(config.post_enable_t & post_enable_long_mem);

	// 3. based on EEPROM settings, initialize PJIT cache and opcode jump tables
	pjit_cache_init(0xA0000000); // has to come after DDR init, before MMU init

	if(config.post_enable_t & post_enable_checkclk) check_clock();

    InitMMU();

	make_i2c_good();
	printf("[BOOT] Completed in %0.5f seconds.\n", am335x_dmtimer1_get_time());

	// TODO Release RESET

	// 4. finally, start PJIT
	print_menu();
}

