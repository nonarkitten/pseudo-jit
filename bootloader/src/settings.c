/*
 * settings.c -- the settings page of the menu: the EEPROM settings (config.h) as a numbered list, one value changed at a time. Changes are used from then on (by the
 * next PJIT started from the menu); leaving the page offers to save them in the EEPROM, otherwise they last until the board is reset. What a typed value means:
 * settings_set.c.
 *
 * Copyright (c) 2020-2026 Renee Cousins, the Buffee Project - http://www.buffee.ca
 * Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (see LICENSE).
 */
#include "main.h"
#include "console.h"

#define MAX_REGIONS   4

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

void SettingsShow(void) {
    uint32_t base = (uint32_t)settings.maprom_base_64k << 16, size = 1u << settings.maprom_size_log2;

    printf("\nSETTINGS\n");
    printf(" 1. MapROM              %-10s off, flash (the ROM image in the flash), copy (of the Amiga's own ROM)\n", maprom_names[settings.maprom_src % 3]);
    printf(" 2. MapROM address      %06lX\n", (unsigned long)base);
    printf(" 3. MapROM size         %lu KB\n", (unsigned long)(size >> 10));
    printf(" 4. Fast RAM            "); ShowRegions();
    printf("  BASE/MB,... e.g. 200000/8, or 0 for none; after MB: B on the Amiga bus, I interpret only, F unpaced\n");
    printf(" 5. Pace                %u %%  of an MC68000 at the host's bus clock, for code in chip RAM; 0: off\n", (unsigned)settings.pace);
    if (settings.post_checkclk) printf(" 6. Bus clock           measured (%0.3f MHz)  0: measure at every boot, or the clock in MHz\n", settings.kHz * 0.001);
    else                        printf(" 6. Bus clock           %0.3f MHz  0: measure at every boot, or the clock in MHz\n", settings.kHz * 0.001);
    printf(" 7. ARM clock           %u MHz\n", (unsigned)settings.dpll_mul);
    printf(" 8. ARM core voltage    %u.%02u V\n", settings.pmic_voltage / 100, settings.pmic_voltage % 100);
    printf(" 9. Boot report         %-10s quiet, verbose (the first boot is always verbose)\n", settings.boot_verbose ? "verbose" : "quiet");
    printf("10. Self-test           %-5s %-4s short, full (the first boot is always full); bus, nobus: the test of the lines to the Amiga\n",
           settings.post_full ? "full" : "short", settings.post_nobus ? "" : "bus");
    printf("11. Restore the defaults\n");
    printf(" 0. Back\n");
}

void SettingsMenu(void) {
    char v[48];
    int changed = 0, item;

    for (;;) {
        SettingsShow();
        item = MenuChoice();
        if (item == 0) break;
        if (item == ITEM_DEFAULTS) {
            if (Confirm("Restore the default settings")) { config_set_defaults(&settings); changed = 1; }
        } else if (item >= 1 && item < ITEM_DEFAULTS) {
            changed |= SettingsSet(item, Ask("New value: ", v, sizeof v));
        }
    }
    if (!changed) return;
    if (Confirm("Save the settings in the EEPROM")) SaveConfigEEPROM();
    else printf("Kept until the board is reset\n");
}
