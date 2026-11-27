/*
 * console.c -- reading what the user types at the menus: the choice, a line, a confirmation (the numbers in it: settings_set.c).
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
