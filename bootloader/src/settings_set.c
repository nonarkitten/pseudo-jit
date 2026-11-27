/*
 * settings_set.c -- the settings (config.h) changed from text: one value as typed on the settings page, or "item=value" pairs. The menu (settings.c), the request page
 * (request.c) and the PJIT payload (platform/buffee/control.c) all go through here, so a value means the same wherever it is typed.
 *
 * Copyright (c) 2020-2026 Renee Cousins, the Buffee Project - http://www.buffee.ca
 * Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (see LICENSE).
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "config.h"
#include "console.h"

#define MAX_REGIONS   4

static const char* skip_blanks(const char* s) { while (*s == ' ' || *s == '\t') s++; return s; }

int ParseHex(const char* s, uint32_t* v) {
    int digits = 0, d;
    uint32_t x = 0;

    s = skip_blanks(s);
    if (*s == '$') s++;
    else if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) s += 2;
    for (;; s++, digits++) {
        d = (*s >= '0' && *s <= '9') ? *s - '0' : (*s >= 'a' && *s <= 'f') ? *s - 'a' + 10 : (*s >= 'A' && *s <= 'F') ? *s - 'A' + 10 : -1;
        if (d < 0) break;
        x = x << 4 | d;
    }
    if (digits) *v = x;
    return digits > 0;
}

int ParseFixed(const char* s, int scale, int32_t* v) {
    int digits = 0, neg = 0;
    int32_t whole = 0, part = 0, unit = scale;

    s = skip_blanks(s);
    if (*s == '-') { neg = 1; s++; }
    for (; *s >= '0' && *s <= '9'; s++, digits++) whole = whole * 10 + (*s - '0');
    if (*s == '.') for (s++; *s >= '0' && *s <= '9'; s++, digits++) if (unit > 1) { unit /= 10; part += (*s - '0') * unit; }
    if (digits) *v = (neg ? -1 : 1) * (whole * scale + part);
    return digits > 0;
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
        if (!mb || (mb & (mb - 1)) || (base & 0xFFFFF) || mb > 512) { printf("Region %d rejected: needs a 1 MB aligned base, a power-of-two MB\n", n + 1); return; }
        for (l = 20; (1u << (l - 20)) < mb; l++) ;
        for (flags = 0;; c++) { f = *c | 0x20; if (f == 'i') flags |= 1; else if (f == 'b') flags |= 2; else if (f == 'f') flags |= 4; else break; }
        settings.mem_region[n].base_mb = base >> 20; settings.mem_region[n].size_log2 = l; settings.mem_region[n].flags = flags;
        if (*c != ',') return;
        c++;
    }
}

/* Changes one setting from what was typed; 1 if it changed. */
int SettingsSet(int item, const char* v) {
    config_t before;
    uint32_t x, l;
    int32_t d;
    char c = *v | 0x20;

    const char* before_v = v;

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
    case 10: for (; *v; c = *++v | 0x20) {                       /* words: short / full, bus / nobus */
                 if (v != before_v && v[-1] != ' ' && v[-1] != ',' && v[-1] != '+') continue;
                 if (c == 's') settings.post_full = 0; else if (c == 'f') settings.post_full = 1;
                 else if (c == 'b') settings.post_nobus = 0; else if (c == 'n') settings.post_nobus = 1;
             }
             break;
    default: break;
    }
    if (memcmp(&before, &settings, sizeof settings) == 0) { printf("Unchanged: %s\n", before_v); return 0; }
    return 1;
}

/* "item=value" pairs as typed on the page, ';' or a line end between them; item 11 restores the defaults (before the pairs after it). */
int SettingsApply(const char* spec) {
    const char* part;
    char v[48];
    int changed = 0, item, n;

    for (;;) {
        while (*spec == ' ' || *spec == ';' || *spec == '\n' || *spec == '\r') spec++;
        if (!*spec) return changed;
        part = spec;
        for (item = 0; *spec >= '0' && *spec <= '9'; spec++) item = item * 10 + (*spec - '0');
        if (*spec++ != '=' || item < 1 || item > ITEM_DEFAULTS) { printf("Settings: '%s' is not item=value\n", part); return -1; }
        for (n = 0; *spec && *spec != ';' && *spec != '\n' && *spec != '\r'; spec++) if (n < (int)sizeof v - 1) v[n++] = *spec;
        v[n] = 0;
        if (item == ITEM_DEFAULTS) { config_set_defaults(&settings); changed++; }
        else if (SettingsSet(item, v)) changed++;
    }
}
