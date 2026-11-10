/* declarations only: the bootloader brings its own printf / gets / putchar (clib/printf.c, clib/cstd.c); the toolchain here has no newlib headers */
#ifndef SHIM_STDIO_H
#define SHIM_STDIO_H
#include <stddef.h>
#include <stdarg.h>
#ifndef NULL
#define NULL ((void*)0)
#endif
#define EOF (-1)
#define __offsetof(t, m) __builtin_offsetof(t, m)
int printf(const char *fmt, ...);
int sprintf(char *buf, const char *fmt, ...);
int putchar(int c);
char *gets(char *s);
#endif
