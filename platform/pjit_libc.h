/*
 * pjit_libc.h — tiny freestanding libc subset for PJIT.
 *
 * PJIT's core (pjit/) is written against a handful of familiar libc calls
 * (memcpy, memset, memcmp, bzero, printf, sprintf) but the project is
 * explicitly bare-metal/no-OS: it must link and run with no host C library
 * present. The Homebrew arm-none-eabi-gcc toolchain used for this project
 * ships GCC only (no newlib/libgloss), which is actually the right fit for
 * that goal -- it just means these few calls need real, tiny, freestanding
 * implementations instead of assuming a hosted libc.
 *
 * These implementations are deliberately small and self-contained: no heap,
 * no locale, no wide chars. printf()/sprintf() support the conversions PJIT
 * actually uses: %d %i %u %x %X %p %s %c %f %% with zero-padded width
 * (e.g. %04X, %08X) and one case of a precision on %f (e.g. %0.1f).
 *
 * Output routing: printf() calls pjit_console_putc() once per character.
 * That hook has a weak no-op default here so the core links standalone;
 * the boot/test harness overrides it (UART MMIO on bare metal, semihosting
 * or plain stdio when hosted under QEMU). See platform/console_*.c.
 */
#ifndef PJIT_LIBC_H
#define PJIT_LIBC_H

#include <stddef.h>
#include <stdarg.h>

void   *memcpy(void *dst, const void *src, size_t n);
void   *memset(void *dst, int c, size_t n);
int     memcmp(const void *a, const void *b, size_t n);
void    bzero(void *dst, size_t n);
size_t  strlen(const char *s);
char   *strstr(const char *haystack, const char *needle);
char   *strcpy(char *dst, const char *src);

/* Weak hook implemented by the platform (UART/semihosting/stdio). */
void pjit_console_putc(char c);

int vsnprintf(char *buf, size_t bufsz, const char *fmt, va_list ap);
int snprintf(char *buf, size_t bufsz, const char *fmt, ...);
int sprintf(char *buf, const char *fmt, ...);
int printf(const char *fmt, ...);

#endif /* PJIT_LIBC_H */
