/*
 * Copyright (c) 2020-2023 Renee Cousins, the Buffee Project - http://www.buffee.ca
 *
 * Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (see LICENSE).
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "config.h"

config_t settings;

/* The settings of a board with nothing stored in its EEPROM. */
void config_set_defaults(config_t *c) {
    memset(c, 0, sizeof *c);
    c->ident = 0x704A4954;
    c->cpu_enable_68000 = 1;
    c->cpu_enable_icache = 1;
    c->cpu_enable_dcache = 1;
    c->post_enable_long_mem = 0;            /* 512 MB takes a while: off unless asked */
    c->post_enable_gpack_ok = 1;
    c->post_enable_checkclk = 1;
    c->post_enable_autommap = 1;
    c->dpll_mul = 1000;
    c->pmic_voltage = 135;                  /* 1.35 V for 1 GHz */
    c->cache_index_bits = 11;
    c->cache_block_bits = 7;
    c->maprom_page = 0xFF;
    c->maprom_base_64k = 0xF8;              /* $F80000 */
    c->maprom_size_log2 = 19;               /* 512 KB */
    c->maprom_src = MAPROM_RAM;             /* copy the Amiga's own Kickstart into DRAM (works with nothing in the flash) */
    c->ram_mode = RAM_128M;
    c->mem_region[0].base_mb = 0;           /* no fast RAM: whatever RAM the Amiga has is left alone; menu E, R adds e.g. 200000/8 */
    c->mem_region[0].size_log2 = 0;
    c->pace = 100;                          /* interpreted code (chip RAM) runs at an MC68000's pace; fast RAM and the MapROM megabyte are translated at full speed */
    c->boot_flags = 0;                      /* a fresh board stops at the menu until the payload is stored; storing it turns both on */
    c->kHz = 8000;
    c->is_dirty = 1;
}

/* The value of the first `len` hex digits of n. */
static uint32_t toHex2(char* n, int len) {
    static const char TOHEX[32] = {
        0, 10, 11, 12, 13, 14, 15,
        0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
        0,  0,  0,  0,  0,  0,
    };
    uint32_t value = 0;
    while(len--) value = (value << 4) | TOHEX[(*n++) & 0x1f];
    return value;
}

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
    const uint8_t* config = 6 + (uint8_t*)&settings;
    config_t i2c_config;

    settings.is_dirty = 0;
    settings.last_boot_good = boot_good;
    settings.crc16 = crc16_ccitt(config, sizeof(config_t) - 8, 0xFFFF);    
    printf("Saving and verifying settings (CRC=%04X)\n", settings.crc16);
    /* A page write that crosses a page boundary wraps inside the page (the part is 8 / 16 / 32 bytes per page): write aligned chunks of 8 bytes, wait out the write cycle after each. */
    for (uint32_t off = 0; off < sizeof(config_t); off += 8) {
        uint32_t n = sizeof(config_t) - off < 8 ? sizeof(config_t) - off : 8;
        uint8_t a[2] = { (uint8_t)(off >> 8), (uint8_t)off };
        I2C0SendCmd( 0x50, a, 2, (const uint8_t*)&settings + off, n);
        WaitMSDMTimer(10);
    }
    I2C0ReadCmd( 0x50, addr, 2, &i2c_config, sizeof(config_t));
    if(memcmp(&i2c_config, &settings, sizeof(config_t))) {
        printf("Verify failed! EEPROM may be bad\n");
    }
}
static const char zero = 0;
static const char one = 1;

void MakeGoodEEPROM(void) {
    uint8_t addr[2] = { 0 };
    addr[1] = __offsetof(config_t, last_boot_good);
    I2C0SendCmd( 0x50, addr, 2, &one, 1);
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
            printf("Settings loaded, last boot was %s\n", i2c_config.last_boot_good ? "good" : "bad");
            // MakeGoodEEPROM();
            settings = i2c_config;
            return;
        } else {
            printf("CRC fail, %04X != %04X\n", calc_crc, i2c_config.crc16);
        }
    } else {
        printf("Ident wrong, 0x704A4954 != 0x%08X\n", i2c_config.ident);
    }
    config_set_defaults(&settings);
    settings.is_dirty = 1;
    printf("Default settings loaded\n");
}

int DetectEEPROM(void) {
    if (I2C0Probe(0x50)) {
        printf("EEPROM Detected ($50)\n");
        LoadConfigEEPROM();
        return 1;
    } else {
        printf("Default settings loaded\n");
        config_set_defaults(&settings);
        return 0;
    }
}

static const char* GetConfigCPU(void) {
    if(settings.cpu_enable_68000) return "68000";
    if(settings.cpu_enable_68020) {
        if(settings.cpu_enable_32bits) {
            return (settings.cpu_enable_fpu) ? "68020/68882" : "68020";
        } else {
            return (settings.cpu_enable_fpu) ? "68EC020/68882" : "68EC020";
        }
    }
    if(settings.cpu_enable_68030) {
        if(!settings.cpu_enable_mmu) return "68EC030";
        if(!settings.cpu_enable_fpu) return "68030";
        return "68030/68882";
    }
    if(settings.cpu_enable_68040) {
        if(!settings.cpu_enable_mmu) return "68EC040";
        if(!settings.cpu_enable_fpu) return "68LC040";
        return "68040";
    }
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
        settings.cpu_enable_68000 = 1;
        settings.cpu_enable_68020 = 0;
        settings.cpu_enable_68030 = 0;
        settings.cpu_enable_68040 = 0;
        settings.cpu_enable_32bits = 0;
        settings.cpu_enable_icache = 0;
        settings.cpu_enable_dcache = 0;
        settings.cpu_enable_fpu = 0;
        settings.cpu_enable_mmu = 0;
    }
    if(streq(CPU, "68EC020")) {
        settings.cpu_enable_68000 = 0;
        settings.cpu_enable_68020 = 1;
        settings.cpu_enable_68030 = 0;
        settings.cpu_enable_68040 = 0;
        settings.cpu_enable_32bits = 0;
        settings.cpu_enable_icache = 1;
        settings.cpu_enable_dcache = 0;
        settings.cpu_enable_fpu = streq(&CPU[7], "/68882");
        settings.cpu_enable_mmu = 0;
    }
    if(streq(CPU, "68020")) {
        settings.cpu_enable_68000 = 0;
        settings.cpu_enable_68020 = 1;
        settings.cpu_enable_68030 = 0;
        settings.cpu_enable_68040 = 0;
        settings.cpu_enable_32bits = 1;
        settings.cpu_enable_icache = 1;
        settings.cpu_enable_dcache = 0;
        settings.cpu_enable_fpu = streq(&CPU[7], "/68882");
        settings.cpu_enable_mmu = 0;
    }
    if(streq(CPU, "68EC030")) {
        settings.cpu_enable_68000 = 0;
        settings.cpu_enable_68020 = 0;
        settings.cpu_enable_68030 = 1;
        settings.cpu_enable_68040 = 0;
        settings.cpu_enable_32bits = 1;
        settings.cpu_enable_icache = 1;
        settings.cpu_enable_dcache = 1;
        settings.cpu_enable_fpu = streq(&CPU[7], "/68882");
        settings.cpu_enable_mmu = 0;
    }
    if(streq(CPU, "68030")) {
        settings.cpu_enable_68000 = 0;
        settings.cpu_enable_68020 = 0;
        settings.cpu_enable_68030 = 1;
        settings.cpu_enable_68040 = 0;
        settings.cpu_enable_32bits = 1;
        settings.cpu_enable_icache = 1;
        settings.cpu_enable_dcache = 1;
        settings.cpu_enable_fpu = streq(&CPU[7], "/68882");
        settings.cpu_enable_mmu = 1;
    }
    if(streq(CPU, "68EC040")) {
        settings.cpu_enable_68000 = 0;
        settings.cpu_enable_68020 = 0;
        settings.cpu_enable_68030 = 0;
        settings.cpu_enable_68040 = 1;
        settings.cpu_enable_32bits = 1;
        settings.cpu_enable_icache = 1;
        settings.cpu_enable_dcache = 1;
        settings.cpu_enable_fpu = 0;
        settings.cpu_enable_mmu = 0;
    }
    if(streq(CPU, "68LC030")) {
        settings.cpu_enable_68000 = 0;
        settings.cpu_enable_68020 = 0;
        settings.cpu_enable_68030 = 0;
        settings.cpu_enable_68040 = 1;
        settings.cpu_enable_32bits = 1;
        settings.cpu_enable_icache = 1;
        settings.cpu_enable_dcache = 1;
        settings.cpu_enable_fpu = 0;
        settings.cpu_enable_mmu = 1;
    }
    if(streq(CPU, "68030")) {
        settings.cpu_enable_68000 = 0;
        settings.cpu_enable_68020 = 0;
        settings.cpu_enable_68030 = 0;
        settings.cpu_enable_68040 = 1;
        settings.cpu_enable_32bits = 1;
        settings.cpu_enable_icache = 1;
        settings.cpu_enable_dcache = 1;
        settings.cpu_enable_fpu = 1;
        settings.cpu_enable_mmu = 1;
    }
}

int toupper(int c) { 
    if(c < 'a' || c > 'z') return c;
    else return c - 'a' + 'A';
}
double strtod(const char* in, char** out) {
    int v = 0;
    int d = 0;
    int div = 1;
    int c;

    while(c = *in++) {
        if(c == 0) break;
        else if(c == '.') { d = 1; continue; }
        else if(c >= '0' && c <= '9') {
            v = (v * 10) + (c - '0');
            if(d) div *= 10;
        }
        else break;
    }

    return (double)v / (double)div;
}

void ManageConfig(void) {
    char option[12] = { 0 };
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
                printf("    CPU: 68000 (68020 and up: not offered yet)\n");
                printf("B.  Clock: %0.3f MHz\n", settings.kHz * 0.001);
                printf("C.  I$: %s\n", settings.cpu_enable_icache ? "Enabled" : "Disabled");
                printf("D.  I$ Mask: %04X\n", settings.icache_mask_24b);
                printf("E.  D$: %s\n", settings.cpu_enable_dcache ? "Enabled" : "Disabled");
                printf("F.  D$ Mask: %04X\n", settings.dcache_mask_24b);
                static const char* const msrc[] = { "Off (ROM stays where the Amiga has it)", "SPI flash image", "Copy of the Amiga's ROM" };
                printf("G.  MapROM: %s\n", msrc[settings.maprom_src % 3]);
                if(settings.maprom_src) {
                    uint32_t base = (uint32_t)settings.maprom_base_64k << 16, sz = 1u << settings.maprom_size_log2;
                    printf("P.    Base %06lX, Q. Size %lu KB (%06lX~%06lX)\n", (unsigned long)base, (unsigned long)(sz >> 10), (unsigned long)base, (unsigned long)(base + sz - 1));
                }
                printf("R.  Guest RAM regions (host DRAM, translated; enter BASE/SIZEMB[I|B],... e.g. 200000/8 or 200000/8I (I = interpret only, paced) or 200000/8B (B = RAM on the Amiga bus, e.g. a Zorro II board: no host DRAM, its code is translated and cached) or 0 for none):");
                for (int k = 0; k < 4 && settings.mem_region[k].size_log2; k++) printf(" %lX/%luMB%s%s%s", (unsigned long)settings.mem_region[k].base_mb << 20, (unsigned long)(1ul << (settings.mem_region[k].size_log2 - 20)), (settings.mem_region[k].flags & 2) ? "B" : "", (settings.mem_region[k].flags & 1) ? "I" : "", (settings.mem_region[k].flags & 4) ? "F" : "");
                if (!settings.mem_region[0].size_log2) printf(" none");
                printf("\n");
                printf("S.  Autoload PJIT from the SPI flash: %s\n", (settings.boot_flags & BOOT_AUTOLOAD) ? "Enabled" : "Disabled");
                printf("T.  Autorun PJIT after loading: %s\n", (settings.boot_flags & BOOT_AUTORUN) ? "Enabled" : "Disabled");
                printf("U.  24-bit guest address space: always on (68000; A24-A31 ignored, RAM regions must lie below 16 MB)\n");
                printf("V.  Pace interpreted code (chip RAM) to percent of an MC68000 at 7.09 MHz, 0 = off: %u\n", (unsigned)settings.pace);
                printf("    POST:\n");
                printf("H.    Long Mem Test %s\n", settings.post_enable_long_mem ? "Enabled" : "Disabled");
                printf("I.    GreenPAK Test %s\n", settings.post_enable_gpack_ok ? "Enabled" : "Disabled");
                printf("J.    Bus Clock Test %s\n", settings.post_enable_checkclk ? "Enabled" : "Disabled");
                printf("K.    Automap %s\n", settings.post_enable_autommap ? "Enabled" : "Disabled");
                printf("L.  MCU CLK: %d\n" , settings.dpll_mul);
                printf("M.  PMIC Voltage: %0.2f\n" , settings.pmic_voltage * 0.01);
                printf("    PJIT Cache:\n");
                printf("N.    Block Size: %d bytes\n", 8 << settings.cache_block_bits);
                printf("O.    Block Count: %d\n", 1 << settings.cache_index_bits);
                uint32_t cs = 8 << (settings.cache_index_bits + settings.cache_block_bits);
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
                case 'B': settings.kHz = (int)(1000.0 * strtod(option, NULL)); break;
                case 'C': settings.cpu_enable_icache = option[0] == 'E'; break;
                case 'D': settings.icache_mask_24b = toHex2(option, 4); break;
                case 'E': settings.cpu_enable_dcache = option[0] == 'E'; break;
                case 'F': settings.dcache_mask_24b = toHex2(option, 4); break;
                case 'G': settings.maprom_src = option[0] == 'F' ? MAPROM_FLASH : (option[0] == 'R' || option[0] == 'C') ? MAPROM_RAM : MAPROM_OFF; break;   /* Flash / Ram (Copy) / Off */
                case 'P': { uint32_t v = 0; for (const char* c = option; (*c >= '0' && *c <= '9') || (*c >= 'A' && *c <= 'F'); c++) v = v << 4 | (*c <= '9' ? *c - '0' : *c - 'A' + 10); settings.maprom_base_64k = v > 0xFFFF ? v >> 16 : v; } break;   /* F80000 or F8 */
                case 'Q': { uint32_t kb = (uint32_t)strtod(option, NULL), l = 10; while (kb > 1) { kb >>= 1; l++; } settings.maprom_size_log2 = l; } break;
                case 'R': {                                      /* BASE(hex bytes, 1 MB aligned)/SIZE(decimal MB, a power of two), up to four, comma separated; anything else clears */
                    int n = 0; const char* c = option;
                    for (int k = 0; k < 4; k++) { settings.mem_region[k].base_mb = 0; settings.mem_region[k].size_log2 = 0; settings.mem_region[k].flags = 0; }
                    while (n < 4) {
                        if (*c == '$') c++;
                        uint32_t base = 0, mb = 0; int digits = 0;
                        for (; (*c >= '0' && *c <= '9') || (*c >= 'A' && *c <= 'F'); c++, digits++) base = base << 4 | (*c <= '9' ? *c - '0' : *c - 'A' + 10);
                        if (!digits || *c != '/') break;
                        for (c++; *c >= '0' && *c <= '9'; c++) mb = mb * 10 + (*c - '0');
                        if (!mb || (mb & (mb - 1)) || (base & 0xFFFFF) || mb > 512) { printf("region %d rejected (needs a 1 MB aligned base and a power-of-two size in MB)\n", n + 1); break; }
                        uint32_t l = 20; while ((1u << (l - 20)) < mb) l++;
                        int interp = 0; for (;; c++) { if (*c == 'I' || *c == 'i') interp |= 1; else if (*c == 'B' || *c == 'b') interp |= 2; else if (*c == 'F' || *c == 'f') interp |= 4; else break; }          /* a trailing I: interpret only (never translated, paced like chip RAM) */
                        settings.mem_region[n].base_mb = base >> 20; settings.mem_region[n].size_log2 = l; settings.mem_region[n].flags = interp; n++;
                        if (*c != ',') break;
                        c++;
                    }
                } break;
                case 'S': if (option[0] == 'E') settings.boot_flags |= BOOT_AUTOLOAD; else settings.boot_flags &= ~BOOT_AUTOLOAD; break;
                case 'T': if (option[0] == 'E') settings.boot_flags |= BOOT_AUTORUN; else settings.boot_flags &= ~BOOT_AUTORUN; break;
                case 'U': if (option[0] == 'E') settings.boot_flags |= BOOT_24BIT; else settings.boot_flags &= ~BOOT_24BIT; break;
                case 'V': { int v = (int)strtod(option, NULL); settings.pace = v < 0 ? 0 : v > 255 ? 255 : v; } break;
                case 'H': settings.post_enable_long_mem = option[0] == 'E'; break;
                case 'I': settings.post_enable_gpack_ok = option[0] == 'E'; break;
                case 'J': settings.post_enable_checkclk = option[0] == 'E'; break;
                case 'K': settings.post_enable_autommap = option[0] == 'E'; break;
                case 'L': settings.dpll_mul = (int)strtod(option, NULL); break;
                case 'M': settings.pmic_voltage = (int)(100.0 * strtod(option, NULL)); break;
                case 'N': settings.cache_block_bits = toHex2(option, 1); break;
                case 'O': settings.cache_index_bits = toHex2(option, 1); break;
                }
            } ;
            break;
        }
    }
}