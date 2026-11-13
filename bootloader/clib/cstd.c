/*
 * cstd.c -- the few C library functions the bootloader needs: memcpy, memmove, memset, memcmp, strcmp, atoi, and a console getsn / putchar.
 *
 *  Created on: Apr. 14, 2021
 *      Author: renee.cousins
 *
 * Copyright (c) 2021-2026 the Buffee Project - http://www.buffee.ca
 * Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (see LICENSE).
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>

#include <arm_neon.h>


__attribute__((optimize("no-tree-loop-distribute-patterns")))

int memcmp ( const void * ptr1, const void * ptr2, size_t num ) {
	const unsigned char *s1 = ptr1;
	const unsigned char *s2 = ptr2;
	while (num--) {
		if (*s1 == *s2) s1++, s2++;
		else return *s1 - *s2;
	}
	return 0;
}

int strcmp(const char *s1, const char *s2) {
    const unsigned char *a = (const unsigned char *)s1, *b = (const unsigned char *)s2;

    while (*a != 0 && *a == *b) {
        a++;
        b++;
    }
    return *a - *b;
}

/* (the attribute keeps the compiler from turning these loops back into calls to memmove / memcpy / memset: these ARE those) */
__attribute__((optimize("no-tree-loop-distribute-patterns")))
void *memmove(void *dest, const void *source, size_t n) {
    uint8_t *to = dest;
    const uint8_t *from = source;
    size_t i;

    if (to < from) {                                /* the destination starts below the source: copy upwards */
        for (i = 0; i < n; i++) to[i] = from[i];
    } else if (to > from) {                         /* it may overlap the end of the source: copy downwards */
        for (i = n; i > 0; i--) to[i - 1] = from[i - 1];
    }
    return dest;
}

__attribute__((optimize("no-tree-loop-distribute-patterns")))
void *memcpy(void *dest, const void *source, size_t n) {
    uint8_t *to = dest;
    const uint8_t *from = source;
    size_t i;

    for (i = 0; i < n; i++) to[i] = from[i];
    return dest;
}

__attribute__((optimize("no-tree-loop-distribute-patterns")))
void *memset(void *ptr, int fill, size_t n) {
    uint8_t *p = ptr;
    size_t i;

    if (p == NULL) return ptr;
    for (i = 0; i < n; i++) p[i] = (uint8_t)fill;
    return ptr;
}



int echo_on = 0;
void SetEcho(int on) {
    echo_on = on;
}

extern uint8_t _getchar(void);
extern void _putchar(uint8_t);


int putchar ( int character ) {
    _putchar(character);
    return character;
}


/* A line from the console into str (at most size - 1 characters, the rest of a longer line is dropped), echoed when SetEcho(1); backspace edits. */
char * getsn (char * str, int size) {
    int i = 0;
    char c;
    for (;;) {
        c = _getchar();
        if(c == '\b' || c == 0x7F) {
            if(i) {
                if(echo_on) printf("\b \b");
                i--;
            }
        } else if(c == '\n' || c == '\r') {
            putchar('\n');
            break;
        } else if(i < size - 1) {
            if(echo_on) putchar(c);
            str[i++] = c;
        }
    }
    str[i] = 0;
    return str;
}

int atoi(const char *c) {
    uint32_t u = 0, s = 0;
    if(*c == '-') s = 1, c++;
    else if(*c == '+') c++;
    while(*c == ' ') c++;
    while(*c >= '0' && *c <= '9') {
        u = (u * 10) + *c++ - '0';
    }
    if(s) return -u; else return u;
}