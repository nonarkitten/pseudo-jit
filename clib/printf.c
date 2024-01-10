/*
 * printf.c
 *
 *  Created on: Apr. 14, 2021
 *      Author: renee.cousins
 */

#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#include "init.h"

static const char* l_base_chars = "0123456789abcdef";
static const char* u_base_chars = "0123456789ABCDEF";

extern void _putchar(uint8_t);

static inline void printUART(int c) {
    if(c == '\n') _putchar('\r');
    _putchar(c);
}

static int printchar(char **str, int c) {
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
    int len = _strlen(str);
    int pc = 0;

    if(len > width) width = 0; else width -= len;
    if(!(pad&PAD_RIGHT)) for(;width;--width) pc += printchar(out, pad_char);
    /* dump string    */ for(;*str; ++str)   pc += printchar(out, *str);
    /* left padding   */ for(;width;--width) pc += printchar(out, pad_char);

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

    int pad_char = (pad&PAD_ZERO)?'0':' ';

    if(pad & PAD_DECIMAL) {
        while (len < 9) *--b = '0', len++;
        if (width < len) b[width] = 0;
    }

    return prints(out, b, width, pad);
}

static inline int printi(char **out, int32_t x, int base, const char* base_chars, int width, int pad) {
    int pc = 0;

    if((x < 0) && (base == 10)) {
        pc += printchar(out, '-');
        if(width) width--;
        x = -x;
    } else if((pad & PAD_SIGN) && (base == 10)) {
        pc += printchar(out, '+');
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

static int print(char **out, const char *format, va_list vl) {
    int width, pad;
    int pc = 0;
    int decimals;

    for (; *format != 0; ++format) {
        if (*format == '%') {
            ++format;
            width = pad = 0;
            decimals = 6;
            if (*format == '\0') break;
            if (*format == '-') {
                ++format;
                pad = PAD_RIGHT;
            }
            if (*format == '+') {
            	++format;
            	pad |= PAD_SIGN;
            }
            while (*format == '0') {
                ++format;
                pad |= PAD_ZERO;
            }
            for ( ; *format >= '0' && *format <= '9'; ++format) {
                width = (width * 10) + (*format - '0');
            }
            if (*format == '.') {
                ++format;
                decimals = 0;
                for ( ; *format >= '0' && *format <= '9'; ++format) {
                    decimals = (decimals * 10) + (*format - '0');
                }
            }
            if (*format == 'h') {
                ++format;
                if (*format == 'h') {
                    ++format;
                    pad |= PAD_BYTE;
                } else {
                    pad |= PAD_WORD;
                }
            } else if(*format == 'l') {
                ++format;
                pad |= PAD_LONG;
            }

            /**/ if( *format == 's' ) pc += prints (out, va_arg(vl,char *), width, pad);
            else if( *format == 'd' ) pc += printi (out, va_arg(vl,int32_t),  10, l_base_chars, width, pad);
            else if( *format == 'o' ) pc += printu (out, va_arg(vl,uint32_t),  8, l_base_chars, width, pad);
            else if( *format == 'x' ) pc += printu (out, va_arg(vl,uint32_t), 16, l_base_chars, width, pad);
            else if( *format == 'X' ) pc += printu (out, va_arg(vl,uint32_t), 16, u_base_chars, width, pad);
            else if((*format >= 'e' && *format <= 'g' ) || (*format >= 'E' && *format <= 'G' ))
                                      pc += printd (out, va_arg(vl, double), width, pad, decimals, *format);
            else if( *format == 'p' ) pc += printu (out, va_arg(vl,uint32_t), 16, u_base_chars, 8, pad | PAD_ZERO);
            else if( *format == 'u' ) pc += printu (out, va_arg(vl,uint32_t), 10, l_base_chars, width, pad);
            else if( *format == 'c' ) pc += printchar (out, va_arg(vl,int32_t));
            else /*                */ pc += printchar (out, *format);

        } else {
            pc += printchar (out, *format);
        }
    }
    if (out) **out = '\0';
    return pc;
}

volatile double printf_overhead = 0.0;

int vprintf(const char *format, va_list vl) {
    double t1 = ReadDMTimerSeconds();
    int r = print(0, format, vl);
    double t2 = ReadDMTimerSeconds();
    printf_overhead += (t2 - t1);
    return r;
}

int printf(const char *format, ...) {
    double t1 = ReadDMTimerSeconds();
    va_list vl;
    va_start (vl, format);
    int r = print(0, format, vl);
    va_end (vl);
    double t2 = ReadDMTimerSeconds();
    printf_overhead += (t2 - t1);
    return r;
}

int sprintf(char *out, const char *format, ...) {
    double t1 = ReadDMTimerSeconds();
    va_list vl;
    va_start (vl, format);
    int r = print(&out, format, vl);
    va_end (vl);
    double t2 = ReadDMTimerSeconds();
    printf_overhead += (t2 - t1);
    return r;
}


