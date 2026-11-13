/*
 * printf.c -- the bootloader's printf family: integers, strings, characters and doubles to the UART (or into a buffer), with width, zero and left padding,
 * '+', precision and the h / hh / l size letters.
 *
 *  Created on: Apr. 14, 2021
 *      Author: renee.cousins
 *
 * Copyright (c) 2021-2026 the Buffee Project - http://www.buffee.ca
 * Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (see LICENSE).
 */

#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#include "init.h"

static const char* l_base_chars = "0123456789abcdef";
static const char* u_base_chars = "0123456789ABCDEF";

extern void _putchar(uint8_t);

static bool line_start = true;          /* the last character sent was a newline: the next printf gets the time stamp */

static inline void printUART(int c) {
    if(c == '\n') _putchar('\r');
    _putchar(c);
    line_start = (c == '\n');
}

static int emit_char(char **str, int c) {
    if(str) *(*str)++ = c;
    else printUART(c);
    return 1;
}

static inline int _strlen(char *buf) {
    if (buf) {
        char *end = buf++;
        while(*end++);
        return end - buf;
    }
    else return 0;
}

#define PAD_RIGHT 1
#define PAD_ZERO 2
#define PAD_SIGN 4
#define PAD_DECIMAL 8
#define PAD_BYTE 16
#define PAD_WORD 32
#define PAD_LONG 64

static int prints(char **out, char *str, int width, int pad) {
    if(str == NULL) return prints(out, "(null)", width, pad);

    int pad_char = (pad&PAD_ZERO)?'0':' ';
    int fill = width > _strlen(str) ? width - _strlen(str) : 0;      /* how many pad characters the string needs */
    int pc = 0;

    if(!(pad&PAD_RIGHT)) for(; fill; --fill) pc += emit_char(out, pad_char);      /* padding in front */
    for(; *str; ++str) pc += emit_char(out, *str);
    for(; fill; --fill) pc += emit_char(out, pad_char);                           /* or behind, when left-justified */

    return pc;
}

static int printu(char **out, uint32_t x, uint32_t base, const char* base_chars, int width, int pad) {
    static char buffer[33];
    char *b = &buffer[32];
    int len = 0;

    if(pad & PAD_LONG) x &= 0xFFFFFFFFul;
    else if(pad & PAD_WORD) x &= 0xFFFFul;
    else if(pad & PAD_BYTE) x &= 0xFFul;

    *b = 0;
    do {
        uint32_t q = 0, r = 0;
        if(base == 16)      q = x >> 4, r = x & 15;
        else if(base == 2)  q = x >> 1, r = x & 1;
        else if(base == 10) q = x / 10, r = x % 10;
        *--b = base_chars[r];
        len += 1;
        x = q;
    } while(x);

    if(pad & PAD_DECIMAL) {
        while (len < 9) *--b = '0', len++;
        if (width < len) b[width] = 0;
    }

    return prints(out, b, width, pad);
}

static inline int printi(char **out, int32_t x, int base, const char* base_chars, int width, int pad) {
    int pc = 0;

    if((x < 0) && (base == 10)) {
        pc += emit_char(out, '-');
        if(width) width--;
        x = -x;
    } else if((pad & PAD_SIGN) && (base == 10)) {
        pc += emit_char(out, '+');
        if(width) width--;
    }

    return pc + printu(out, (uint64_t)x, base, base_chars, width, pad);
}

static int printd(char **out, double num, int width, int pad, int decimals, int exp) {
    char buffer1[64], *b1 = buffer1;
    char buffer2[64], *b2 = buffer2;
    char expbuf[8], *expptr = expbuf;
    int l1=0, l2=0, pow10=0;
    int32_t whole=0;
    uint32_t frac=0;

    switch(exp) {
    case 'e': case 'E':
        while(num >= 10.0) num /= 10.0, pow10++;
        while(num < 0.1) num *= 10.0, pow10--;
        *expptr++ = exp;
        printi(&expptr, pow10, 10, u_base_chars, 0, PAD_SIGN);
        // fallthru
    default:
    case 'f': case 'F':
        whole = (int32_t)num;
        frac = (uint32_t)(1000000000.0 * (num - (double)whole));
        printi(&b1, (int)num, 10, u_base_chars, 0, pad & PAD_SIGN);
        if(decimals) {
            *b1++ = '.';
            printu(&b1, frac, 10, u_base_chars, decimals, PAD_DECIMAL);
        }
        if(expptr != expbuf) prints(&b1, expbuf, 0, 0);
        *b1 = 0;
        return prints(out, buffer1, width, pad);
    case 'g': case 'G':
        l1 = printd(&b1, num, width, pad, decimals, exp - 2); // e/E
        l2 = printd(&b2, num, width, pad, decimals, exp - 1); // f/F
        if(l1 < l2) return prints(out, buffer1, width, pad);
        else        return prints(out, buffer2, width, pad);
    }
}

/* One conversion specification, %[flags][width][.precision][size]conversion, as read by parse_spec() */
typedef struct {
    int pad;            /* PAD_* flags: from '-', '+', '0' and the size letters */
    int width;
    int decimals;       /* the precision; 6 when there is none */
    char conv;          /* the conversion letter, or 0 when the format ends inside the specification */
} fmt_spec_t;

/* the unsigned decimal number at *p; *p is left after its digits */
static int read_number(const char **p) {
    int n = 0;
    while (**p >= '0' && **p <= '9') {
        n = n * 10 + (**p - '0');
        (*p)++;
    }
    return n;
}

/* fills in *s from the text after a '%' and returns the position of the conversion letter */
static const char *parse_spec(const char *p, fmt_spec_t *s) {
    s->pad = 0;
    s->decimals = 6;
    for (;; p++) {
        if      (*p == '-') s->pad |= PAD_RIGHT;
        else if (*p == '+') s->pad |= PAD_SIGN;
        else if (*p == '0') s->pad |= PAD_ZERO;
        else break;
    }
    s->width = read_number(&p);
    if (*p == '.') {
        p++;
        s->decimals = read_number(&p);
    }
    if (*p == 'h') {
        p++;
        if (*p == 'h') { p++; s->pad |= PAD_BYTE; }
        else s->pad |= PAD_WORD;
    } else if (*p == 'l') {
        p++;
        s->pad |= PAD_LONG;
    }
    s->conv = *p;
    return p;
}

/* formats into *out when it is not NULL (and terminates it), else to the UART; returns the number of characters */
static int print(char **out, const char *format, va_list vl) {
    int pc = 0;

    for (; *format != 0; format++) {
        fmt_spec_t s;

        if (*format != '%') {
            pc += emit_char(out, *format);
            continue;
        }
        format = parse_spec(format + 1, &s);
        if (s.conv == 0) break;

        switch (s.conv) {
        case 's': pc += prints(out, va_arg(vl, char *), s.width, s.pad); break;
        case 'd': pc += printi(out, va_arg(vl, int32_t), 10, l_base_chars, s.width, s.pad); break;
        case 'u': pc += printu(out, va_arg(vl, uint32_t), 10, l_base_chars, s.width, s.pad); break;
        case 'o': pc += printu(out, va_arg(vl, uint32_t), 8, l_base_chars, s.width, s.pad); break;
        case 'x': pc += printu(out, va_arg(vl, uint32_t), 16, l_base_chars, s.width, s.pad); break;
        case 'X': pc += printu(out, va_arg(vl, uint32_t), 16, u_base_chars, s.width, s.pad); break;
        case 'p': pc += printu(out, va_arg(vl, uint32_t), 16, u_base_chars, 8, s.pad | PAD_ZERO); break;
        case 'c': pc += emit_char(out, va_arg(vl, int32_t)); break;
        case 'e': case 'E': case 'f': case 'F': case 'g': case 'G':
            pc += printd(out, va_arg(vl, double), s.width, s.pad, s.decimals, s.conv);
            break;
        default:  pc += emit_char(out, s.conv); break;      /* "%%", and whatever else follows a '%' */
        }
    }
    if (out) **out = '\0';
    return pc;
}

bool printTimeStamp = false;
bool console_verbose = true;


static int printproxy(const char *format, ...) {
    va_list vl;
    va_start (vl, format);
    int r = print(0, format, vl);
    va_end (vl);
    return r;
}

static void stamp(void) {
    if(printTimeStamp && line_start) {
        static double lastTime = 0.0;
        double thisTime = ReadDMTimerSeconds();
        printproxy("%4.5f (+%4dms): ", thisTime, (int)((thisTime - lastTime) * 1000));
        lastTime = thisTime;
    }
}

int printf(const char *format, ...) {
    stamp();
    va_list vl;
    va_start (vl, format);
    int r = print(0, format, vl);
    va_end (vl);
    return r;
}

/* A progress or status line: printed only when the boot report is verbose (errors use printf). */
int info(const char *format, ...) {
    if(!console_verbose) return 0;
    stamp();
    va_list vl;
    va_start (vl, format);
    int r = print(0, format, vl);
    va_end (vl);
    return r;
}

int sprintf(char *out, const char *format, ...) {
    va_list vl;
    va_start (vl, format);
    int r = print(&out, format, vl);
    va_end (vl);
    return r;
}

