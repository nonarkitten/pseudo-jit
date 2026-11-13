/*
 * settings.c -- the settings page of the menu: the EEPROM settings (config.h) as a numbered list, one value changed at a time. Changes are used from then on (by the
 * next PJIT started from the menu); leaving the page offers to save them in the EEPROM, otherwise they last until the board is reset.
 *
 * Copyright (c) 2020-2026 Renee Cousins, the Buffee Project - http://www.buffee.ca
 * Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (see LICENSE).
 */
#include "main.h"
#include "console.h"

#define MAX_REGIONS   4
#define ITEM_DEFAULTS 11                /* the entry that restores the defaults; 1 .. ITEM_DEFAULTS - 1 are values */

static const char* const maprom_names[] = { "off", "flash", "copy" };      /* by MAPROM_* */

static void ShowRegions(void) {
    uint8_t f;
    int k;

    if (!settings.mem_region[0].size_log2) { printf("none"); return; }
    for (k = 0; k < MAX_REGIONS && settings.mem_region[k].size_log2; k++) {
        f = settings.mem_region[k].flags;
        printf("%s%lX/%lu%s%s%s", k ? "," : "", (unsigned long)settings.mem_region[k].base_mb << 20, (unsigned long)(1ul << (settings.mem_region[k].size_log2 - 20)),
               (f & 2) ? "B" : "", (f & 1) ? "I" : "", (f & 4) ? "F" : "");
    }
}

static void Show(void) {
    uint32_t base = (uint32_t)settings.maprom_base_64k << 16, size = 1u << settings.maprom_size_log2;

    printf("\nSETTINGS\n");
    printf(" 1. MapROM              %-10s off, flash (the ROM image in the flash), copy (of the Amiga's own ROM)\n", maprom_names[settings.maprom_src % 3]);
    printf(" 2. MapROM address      %06lX\n", (unsigned long)base);
    printf(" 3. MapROM size         %lu KB\n", (unsigned long)(size >> 10));
    printf(" 4. Fast RAM            "); ShowRegions();
    printf("  BASE/MB,... e.g. 200000/8, or 0 for none; after MB: B on the Amiga bus, I interpret only, F unpaced\n");
    printf(" 5. Pace                %u %%  of an MC68000 at 7.09 MHz, for code in chip RAM; 0: off\n", (unsigned)settings.pace);
    if (settings.post_checkclk) printf(" 6. Bus clock           measured (%0.3f MHz)  0: measure at every boot, or the clock in MHz\n", settings.kHz * 0.001);
    else                        printf(" 6. Bus clock           %0.3f MHz  0: measure at every boot, or the clock in MHz\n", settings.kHz * 0.001);
    printf(" 7. ARM clock           %u MHz\n", (unsigned)settings.dpll_mul);
    printf(" 8. ARM core voltage    %u.%02u V\n", settings.pmic_voltage / 100, settings.pmic_voltage % 100);
    printf(" 9. Boot report         %-10s quiet, verbose (the first boot is always verbose)\n", settings.boot_verbose ? "verbose" : "quiet");
    printf("10. Self-test           %-10s short, full (the first boot is always full)\n", settings.post_full ? "full" : "short");
    printf("11. Restore the defaults\n");
    printf(" 0. Back\n");
}

/* BASE(hex bytes, 1 MB aligned)/SIZE(decimal MB, a power of two)[B][I][F], up to four, comma separated; anything else that is not a region clears the list. */
static void SetRegions(const char* c) {
    uint32_t base, mb, l;
    int k, n, digits, flags;
    char f;

    for (k = 0; k < MAX_REGIONS; k++) { settings.mem_region[k].base_mb = 0; settings.mem_region[k].size_log2 = 0; settings.mem_region[k].flags = 0; }
    for (n = 0; n < MAX_REGIONS; n++) {
        base = 0; mb = 0; digits = 0;
        while (*c == ' ') c++;
        if (*c == '$') c++;
        for (; (*c >= '0' && *c <= '9') || (*c >= 'A' && *c <= 'F') || (*c >= 'a' && *c <= 'f'); c++, digits++) base = base << 4 | (*c <= '9' ? *c - '0' : (*c | 0x20) - 'a' + 10);
        if (!digits || *c != '/') return;
        for (c++; *c >= '0' && *c <= '9'; c++) mb = mb * 10 + (*c - '0');
        if (!mb || (mb & (mb - 1)) || (base & 0xFFFFF) || mb > 512) { printf("Region %d rejected: it needs a 1 MB aligned base and a power-of-two size in MB\n", n + 1); return; }
        for (l = 20; (1u << (l - 20)) < mb; l++) ;
        for (flags = 0;; c++) { f = *c | 0x20; if (f == 'i') flags |= 1; else if (f == 'b') flags |= 2; else if (f == 'f') flags |= 4; else break; }
        settings.mem_region[n].base_mb = base >> 20; settings.mem_region[n].size_log2 = l; settings.mem_region[n].flags = flags;
        if (*c != ',') return;
        c++;
    }
}

/* Changes one setting from what was typed; 1 if it changed. */
static int Set(int item, const char* v) {
    config_t before;
    uint32_t x, l;
    int32_t d;
    char c = *v | 0x20;

    memcpy(&before, &settings, sizeof before);
    switch (item) {
    case 1: if (c == 'o' && (v[1] | 0x20) == 'f') settings.maprom_src = MAPROM_OFF; else if (c == 'f') settings.maprom_src = MAPROM_FLASH; else if (c == 'c') settings.maprom_src = MAPROM_RAM; break;
    case 2: if (ParseHex(v, &x)) settings.maprom_base_64k = x > 0xFFFF ? x >> 16 : x; break;      /* F80000 or F8 */
    case 3: if (ParseFixed(v, 1, &d) && d >= 64 && d <= 1024 && !(d & (d - 1))) { for (l = 10; d > 1; d >>= 1) l++; settings.maprom_size_log2 = l; } break;
    case 4: SetRegions(v); break;
    case 5: if (ParseFixed(v, 1, &d)) settings.pace = d < 0 ? 0 : d > 255 ? 255 : d; break;
    case 6: if (ParseFixed(v, 1000, &d)) { if (d == 0) settings.post_checkclk = 1; else if (d >= 5000 && d <= 16000) { settings.post_checkclk = 0; settings.kHz = d; } } break;
    case 7: if (ParseFixed(v, 1, &d) && d >= 300 && d <= 1100) settings.dpll_mul = d; break;
    case 8: if (ParseFixed(v, 100, &d) && d >= 90 && d <= 145) settings.pmic_voltage = d; break;
    case 9: if (c == 'q') settings.boot_verbose = 0; else if (c == 'v') settings.boot_verbose = 1; break;
    case 10: if (c == 's') settings.post_full = 0; else if (c == 'f') settings.post_full = 1; break;
    default: break;
    }
    if (memcmp(&before, &settings, sizeof settings) == 0) { printf("Unchanged\n"); return 0; }
    return 1;
}

void SettingsMenu(void) {
    char v[48];
    int changed = 0, item;

    for (;;) {
        Show();
        item = MenuChoice();
        if (item == 0) break;
        if (item == ITEM_DEFAULTS) {
            if (Confirm("Restore the default settings")) { config_set_defaults(&settings); changed = 1; }
        } else if (item >= 1 && item < ITEM_DEFAULTS) {
            changed |= Set(item, Ask("New value: ", v, sizeof v));
        }
    }
    if (!changed) return;
    if (Confirm("Save the settings in the EEPROM")) SaveConfigEEPROM();
    else printf("Kept until the board is reset\n");
}
