/*
 * buffee_rt.c -- what Dhrystone needs around it on the bare 68000 of the PJIT tests: start-up, a console, malloc, strcpy, strcmp, printf, scanf,
 * times, and the 32-bit divide its timed loop uses.
 *
 * Copyright (c) 2026 Renee Cousins, the Buffee Project - http://www.buffee.ca
 * Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (see LICENSE).
 *
 * Weicker's dhry.h, dhry_1.c and dhry_2.c are compiled exactly as they are; this file supplies the library calls they make. The harness
 * (tests/qemu/smoke_dhrystone.c) times the run from outside, so times() returns 0 and the program takes its "measured time too small" path; the
 * floating-point reporting code that follows it in main() is never reached, and the soft-float helpers it calls are stubs that only satisfy the linker.
 *
 * The "BIOS" is four fixed guest addresses the harness fills in. No libc, no libgcc: tools/data/dhrystone.py checks that nothing is left undefined.
 */
#include <stdarg.h>

#define BIOS_PUTC 0x100                                     /* D0.B = character to print      (TRAP #1 ; RTS in the harness) */
#define BIOS_EXIT 0x110                                     /* does not return                (TRAP #15 ; RTS)               */
#define QUIET (*(volatile unsigned char *)0x118)            /* nonzero: printf prints nothing (the timed runs)               */
#define RUNS (*(volatile long *)0x11C)                      /* the number of runs scanf hands to main()                      */

__asm__(".pushsection .text.start,\"ax\"\n.globl _start\n_start:\n\tlea 0xF0000,%sp\n\tjsr main\n\tjsr 0x110\n1:\tbra.s 1b\n.popsection\n");

static void put(int c)
{
    register int d0 __asm__("d0") = c;
    __asm__ volatile("jsr 0x100" : "+d"(d0) : : "cc", "memory");
}

/* ---- the C library calls of dhry_1.c and dhry_2.c ---- */

char *strcpy(char *to, const char *from)
{
    char *start = to;
    while ((*to++ = *from++) != 0)
        ;
    return start;
}

int strcmp(const char *a, const char *b)
{
    while (*a != 0 && *a == *b) {
        a++;
        b++;
    }
    return (unsigned char)*a - (unsigned char)*b;
}

static char heap[512];                                      /* two records of Rec_Type are all Dhrystone asks for */
static unsigned heap_used;

char *malloc(unsigned n)
{
    char *p = heap + heap_used;
    heap_used += (n + 3) & ~3u;
    return p;
}

struct tms { long tms_utime, tms_stime, tms_cutime, tms_cstime; };

int times(struct tms *t)
{
    t->tms_utime = t->tms_stime = t->tms_cutime = t->tms_cstime = 0;
    return 0;
}

int scanf(const char *fmt, int *n)
{
    (void)fmt;
    *n = (int)RUNS;
    return 1;
}

/* ---- printf: %d %u %x %c %s and a width; precision is skipped; a double (%f) is read and printed as "?" ---- */

static int emit_number(unsigned long v, int negative, int width, int zero, int base)
{
    static const unsigned long dec[] = { 1000000000UL, 100000000UL, 10000000UL, 1000000UL, 100000UL, 10000UL, 1000UL, 100UL, 10UL, 1UL };
    char text[12];
    int n = 0, i, count = 0;

    if (base == 10) {
        int started = 0;
        for (i = 0; i < 10; i++) {                           /* subtract the powers of ten: no division needed */
            int digit = 0;
            while (v >= dec[i]) {
                v -= dec[i];
                digit++;
            }
            if (digit || started || i == 9) {
                text[n++] = (char)('0' + digit);
                started = 1;
            }
        }
    } else {
        for (i = 28; i >= 0; i -= 4) {
            int digit = (int)((v >> i) & 15);
            if (digit || n || i == 0)
                text[n++] = "0123456789abcdef"[digit];
        }
    }
    if (negative)
        width--;
    if (negative && zero) {                                  /* -0005 */
        put('-');
        count++;
    }
    for (i = n; i < width; i++) {
        put(zero ? '0' : ' ');
        count++;
    }
    if (negative && !zero) {                                 /*    -5 */
        put('-');
        count++;
    }
    for (i = 0; i < n; i++) {
        put(text[i]);
        count++;
    }
    return count;
}

int printf(const char *f, ...)
{
    va_list ap;
    int count = 0;

    if (QUIET)
        return 0;
    va_start(ap, f);
    for (; *f; f++) {
        int width = 0, zero = 0;
        if (*f != '%') {
            put(*f);
            count++;
            continue;
        }
        f++;
        if (*f == '0') {
            zero = 1;
            f++;
        }
        while (*f >= '0' && *f <= '9')
            width = (width << 3) + (width << 1) + (*f++ - '0');
        if (*f == '.')
            for (f++; *f >= '0' && *f <= '9'; f++)
                ;
        switch (*f) {
        case 'd': {
            long v = va_arg(ap, int);
            count += emit_number(v < 0 ? -(unsigned long)v : (unsigned long)v, v < 0, width, zero, 10);
            break;
        }
        case 'u':
            count += emit_number(va_arg(ap, unsigned), 0, width, zero, 10);
            break;
        case 'x':
            count += emit_number(va_arg(ap, unsigned), 0, width, zero, 16);
            break;
        case 'c':
            put(va_arg(ap, int));
            count++;
            break;
        case 's': {
            const char *s = va_arg(ap, const char *);
            while (*s) {
                put(*s++);
                count++;
            }
            break;
        }
        case 'f':
            (void)va_arg(ap, double);
            put('?');
            count++;
            break;
        case '%':
            put('%');
            count++;
            break;
        default:
            break;
        }
    }
    va_end(ap);
    return count;
}

/* ---- the 32-bit divide of the timed loop (Int_1_Loc = Int_2_Loc / Int_3_Loc): DIVU.W when the divisor fits in 16 bits, else shift and subtract ---- */

static inline unsigned divu16(unsigned n, unsigned d, unsigned *rem)   /* n / d for d < 65536 when the quotient fits in 16 bits */
{
    __asm__("divu.w %1,%0" : "+d"(n) : "d"(d) : "cc");
    *rem = n >> 16;
    return n & 0xFFFF;
}

int __divsi3(int a, int b)
{
    unsigned ua = a < 0 ? -(unsigned)a : (unsigned)a, ub = b < 0 ? -(unsigned)b : (unsigned)b, q = 0, r;

    if (ub == 0)
        return 0;
    if (ub < 0x10000) {
        q = divu16(ua >> 16, ub, &r) << 16;
        q |= divu16((r << 16) | (ua & 0xFFFF), ub, &r);
    } else {
        int i;
        for (i = 16; i >= 0; i--) {
            if ((ua >> i) >= ub) {
                ua -= ub << i;
                q |= 1u << i;
            }
        }
    }
    return (a < 0) != (b < 0) ? -(int)q : (int)q;
}

/* ---- never executed (see the top): they satisfy the linker for the float reporting code at the end of main() ---- */

#define NEVER(name) void name(void) { for (;;) ; }
NEVER(__divsf3)
NEVER(__divdf3)
NEVER(__mulsf3)
NEVER(__muldf3)
NEVER(__floatsisf)
NEVER(__extendsfdf2)
NEVER(__truncdfsf2)
