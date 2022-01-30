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

/* Address of the BBB primary UART */
//#define  UART            (0x44E09000)

static const char* l_base_chars = "0123456789abcdef";
static const char* u_base_chars = "0123456789ABCDEF";

#define SYS_WRITEC      0x03

//static inline void writeUART(int c) {
//    while(*((volatile uint8_t*)(UART + 0x68)) >= 63) continue;
//   *(volatile uint8_t*)UART = (uint8_t)c;
//}

static char buffer[33] = { 0 };
static uint32_t buffer_index = 0;

//extern void sh_printch(int c);

extern void sh_printstr(char *string);
static inline void printUART(int c) {
//    if(c == '\n') writeUART('\r');
//    writeUART(c);
//    if(c == '\n') sh_printch('\r');
//    sh_printch(c);
    buffer[buffer_index++] = c;
    if(buffer_index == 32) {
        sh_printstr(buffer);
        buffer_index = 0;
    }
}

static inline void flushUART() {
    buffer[buffer_index++] = 0;
    sh_printstr(buffer);
    buffer_index = 0;
}

static int printchar(char **str, int c) {
    if(str) *(*str)++ = c;
    else printUART(c);
    return 1;
}

static inline int __attribute__ ((optimize("Os"))) _strlen(char *buf) {
    if (buf) {
        char *end = buf++;
        while(*end++);
        return end - buf;
    }
    else return 0;
}

#define PAD_RIGHT 1
#define PAD_ZERO 2

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

    *b = 0;
    do {
        uint32_t q = x / base;
        uint32_t r = x % base;

        *--b = base_chars[r];
        x = q;
    } while(x);

    return prints(out, b, width, pad);
}

static inline int printi(char **out, int32_t x, int base, const char* base_chars, int width, int pad) {
    int pc = 0;

    if((x < 0) && (base == 10)) {
        pc += printchar(out, '-');
        x = -x;
    }

    return pc + printu(out, (uint32_t)x, base, base_chars, width, pad);
}

static int print(char **out, int *varg) {
    int width, pad;
    int pc = 0;
    char *format = (char *)(*varg++);

    for (; *format != 0; ++format) {
        if (*format == '%') {
            ++format;
            width = pad = 0;
            if (*format == '\0') break;
            if (*format == '-') {
                ++format;
                pad = PAD_RIGHT;
            }
            while (*format == '0') {
                ++format;
                pad |= PAD_ZERO;
            }
            for ( ; *format >= '0' && *format <= '9'; ++format) {
                width = (width * 10) + (*format - '0');
            }

            /**/ if( *format == 's' ) pc += prints (out, *((char **)varg++), width, pad);
            else if( *format == 'd' ) pc += printi (out, *(int32_t*)varg++,  10, l_base_chars, width, pad);
            else if( *format == 'o' ) pc += printu (out, *(uint32_t*)varg++,  8, l_base_chars, width, pad);
            else if( *format == 'x' ) pc += printu (out, *(uint32_t*)varg++, 16, l_base_chars, width, pad);
            else if( *format == 'X' ) pc += printu (out, *(uint32_t*)varg++, 16, u_base_chars, width, pad);
            else if( *format == 'p' ) pc += printu (out, *(uint32_t*)varg++, 16, u_base_chars, 8, pad | PAD_ZERO);
            else if( *format == 'u' ) pc += printu (out, *(uint32_t*)varg++, 10, l_base_chars, width, pad);
            else if( *format == 'c' ) pc += printchar (out, *varg++);
            else /*                */ pc += printchar (out, *format);

        } else {
            pc += printchar (out, *format);
        }
    }
    if (out) **out = '\0';
    return pc;
}

int printf(const char *format, ...) {
    int r = print(0, (char *)(&format));
    flushUART();
    return r;
}

int sprintf(char *out, const char *format, ...) {
    return print(&out, (char *)(&format));
}

int puts ( const char * str ) {
    int r = prints(0, (char*)str, 0, 0) + printchar(0, '\n');
    flushUART();
    return r;
}

int putchar ( int c ) {
    printUART( c );
    flushUART();
    return c;
}

void exit(int status) {
	while(1) ;
}


