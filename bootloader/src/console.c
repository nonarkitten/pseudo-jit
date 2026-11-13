/*
 * console.c -- reading what the user types at the menus: the choice, a line, a confirmation, hex and decimal numbers.
 *
 * Copyright (c) 2021-2026 the Buffee Project - http://www.buffee.ca
 * Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (see LICENSE).
 */
#include <stdio.h>
#include <stdlib.h>
#include "console.h"

#define CHOICE_LEN 8                    /* the longest menu choice that is read */

static const char* skip_blanks(const char* s) { while (*s == ' ' || *s == '\t') s++; return s; }

int MenuChoice(void) {
    char line[CHOICE_LEN];
    const char* s;

    printf("Ready\n] ");
    getsn(line, sizeof line);
    s = skip_blanks(line);
    if (*s < '0' || *s > '9') return -1;
    return atoi(s);
}

char* Ask(const char* prompt, char* buf, int size) {
    printf("%s", prompt);
    return getsn(buf, size);
}

int Confirm(const char* question) {
    char a[4];

    printf("%s [y/N]? ", question);
    getsn(a, sizeof a);
    if (a[0] == 'y' || a[0] == 'Y') return 1;
    printf("Cancelled\n");
    return 0;
}

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
