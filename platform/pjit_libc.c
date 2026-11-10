/*
 * pjit_libc.c — implementation of the tiny freestanding libc subset.
 * See pjit_libc.h for why this exists.
 */
#include "pjit_libc.h"
#include <stdint.h>

void *memcpy(void *dst, const void *src, size_t n)
{
    uint8_t *d = (uint8_t *)dst;
    const uint8_t *s = (const uint8_t *)src;
    while (n--) *d++ = *s++;
    return dst;
}

void *memset(void *dst, int c, size_t n)
{
    uint8_t *d = (uint8_t *)dst;
    while (n--) *d++ = (uint8_t)c;
    return dst;
}

int memcmp(const void *a, const void *b, size_t n)
{
    const uint8_t *pa = (const uint8_t *)a, *pb = (const uint8_t *)b;
    while (n--) {
        if (*pa != *pb) return (int)*pa - (int)*pb;
        pa++; pb++;
    }
    return 0;
}

void bzero(void *dst, size_t n)
{
    memset(dst, 0, n);
}

size_t strlen(const char *s)
{
    size_t n = 0;
    while (s[n]) n++;
    return n;
}

char *strstr(const char *haystack, const char *needle)
{
    size_t nlen = strlen(needle);
    if (nlen == 0) return (char *)haystack;
    for (; *haystack; haystack++) {
        if (memcmp(haystack, needle, nlen) == 0) return (char *)haystack;
    }
    return NULL;
}

char *strcpy(char *dst, const char *src)
{
    char *d = dst;
    while ((*d++ = *src++) != '\0') { }
    return dst;
}

/*
 * Weak default: swallow output silently so the core links even when no
 * platform console has been wired up yet. Real targets provide a strong
 * definition (platform/qemu_virt/console_uart.c, platform/beaglebone/bbb.c, platform/buffee/buffee.c).
 */
__attribute__((weak)) void pjit_console_putc(char c)
{
    (void)c;
}

/* ---- minimal printf/sprintf core ------------------------------------- */

struct sink {
    char   *buf;      /* NULL => write via pjit_console_putc() instead */
    size_t  cap;      /* buffer capacity, including the NUL terminator */
    size_t  written;  /* characters produced so far (like real *printf) */
};

static void sink_putc(struct sink *s, char c)
{
    if (s->buf) {
        if (s->written + 1 < s->cap) s->buf[s->written] = c;
    } else {
        pjit_console_putc(c);
    }
    s->written++;
}

static void sink_puts(struct sink *s, const char *str)
{
    while (*str) sink_putc(s, *str++);
}

/* Pads with spaces on the right up to `width`, counting from `mark`
 * (an earlier s->written snapshot) to however much has been written since. */
static void pad_right_from(struct sink *s, size_t mark, int width)
{
    int printed = (int)(s->written - mark);
    for (int pad = width - printed; pad > 0; pad--) sink_putc(s, ' ');
}

static void out_unsigned(struct sink *s, unsigned long v, unsigned base,
                          int width, int zero_pad, int upper, int left_justify)
{
    char digits[32];
    const char *set = upper ? "0123456789ABCDEF" : "0123456789abcdef";
    int n = 0;

    do {
        digits[n++] = set[v % base];
        v /= base;
    } while (v && n < (int)sizeof(digits));

    if (!left_justify) {
        for (int pad = width - n; pad > 0; pad--) sink_putc(s, zero_pad ? '0' : ' ');
        while (n) sink_putc(s, digits[--n]);
    } else {
        size_t mark = s->written;
        while (n) sink_putc(s, digits[--n]);
        pad_right_from(s, mark, width);
    }
}

static void out_signed(struct sink *s, long v, int width, int zero_pad, int left_justify)
{
    if (v < 0) {
        sink_putc(s, '-');
        if (width > 0) width--;
        out_unsigned(s, (unsigned long)(-(v + 1)) + 1UL, 10, width, zero_pad, 0, left_justify);
    } else {
        out_unsigned(s, (unsigned long)v, 10, width, zero_pad, 0, left_justify);
    }
}

/* Fixed-point float printer: good enough for PJIT's one "%0.1f%%" report. */
static void out_float(struct sink *s, double v, int precision)
{
    if (v < 0) { sink_putc(s, '-'); v = -v; }
    if (precision < 0) precision = 6;

    unsigned long scale = 1;
    for (int i = 0; i < precision; i++) scale *= 10;

    unsigned long scaled = (unsigned long)(v * (double)scale + 0.5);
    unsigned long ip = scaled / scale;
    unsigned long fp = scaled % scale;

    out_unsigned(s, ip, 10, 0, 0, 0, 0);
    if (precision > 0) {
        sink_putc(s, '.');
        /* zero-pad the fractional part out to `precision` digits */
        unsigned long p = scale / 10;
        while (p) {
            sink_putc(s, (char)('0' + (fp / p) % 10));
            p /= 10;
        }
    }
}

static int vformat(struct sink *s, const char *fmt, va_list ap)
{
    while (*fmt) {
        if (*fmt != '%') { sink_putc(s, *fmt++); continue; }
        fmt++; /* skip '%' */

        int zero_pad = 0, left_justify = 0, width = 0, precision = -1;

        /* flags: '-' (left-justify) and '0' (zero-pad) in either order */
        for (;;) {
            if (*fmt == '-') { left_justify = 1; fmt++; }
            else if (*fmt == '0') { zero_pad = 1; fmt++; }
            else break;
        }
        while (*fmt >= '0' && *fmt <= '9') { width = width * 10 + (*fmt - '0'); fmt++; }
        if (*fmt == '.') {
            fmt++;
            precision = 0;
            while (*fmt >= '0' && *fmt <= '9') { precision = precision * 10 + (*fmt - '0'); fmt++; }
        }
        if (left_justify) zero_pad = 0; /* '-' overrides '0', as in real printf */

        switch (*fmt) {
        case 'd': case 'i':
            out_signed(s, va_arg(ap, int), width, zero_pad, left_justify);
            break;
        case 'u':
            out_unsigned(s, va_arg(ap, unsigned int), 10, width, zero_pad, 0, left_justify);
            break;
        case 'x':
            out_unsigned(s, va_arg(ap, unsigned int), 16, width, zero_pad, 0, left_justify);
            break;
        case 'X':
            out_unsigned(s, va_arg(ap, unsigned int), 16, width, zero_pad, 1, left_justify);
            break;
        case 'p':
            sink_puts(s, "0x");
            out_unsigned(s, (unsigned long)(uintptr_t)va_arg(ap, void *), 16, 8, 1, 0, 0);
            break;
        case 'c':
            sink_putc(s, (char)va_arg(ap, int));
            break;
        case 's': {
            const char *str = va_arg(ap, const char *);
            if (!str) str = "(null)";
            if (left_justify) {
                size_t mark = s->written;
                sink_puts(s, str);
                pad_right_from(s, mark, width);
            } else {
                int pad = width - (int)strlen(str);
                for (; pad > 0; pad--) sink_putc(s, ' ');
                sink_puts(s, str);
            }
            break;
        }
        case 'f':
            out_float(s, va_arg(ap, double), precision);
            break;
        case '%':
            sink_putc(s, '%');
            break;
        default:
            /* Unknown conversion: emit verbatim so mistakes are visible. */
            sink_putc(s, '%');
            sink_putc(s, *fmt);
            break;
        }
        fmt++;
    }
    return (int)s->written;
}

int vsnprintf(char *buf, size_t bufsz, const char *fmt, va_list ap)
{
    struct sink s = { buf, bufsz, 0 };
    int n = vformat(&s, fmt, ap);
    if (buf && bufsz) buf[s.written < bufsz ? s.written : bufsz - 1] = '\0';
    return n;
}

int snprintf(char *buf, size_t bufsz, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(buf, bufsz, fmt, ap);
    va_end(ap);
    return n;
}

/*
 * True unbounded sprintf(), matching the (unsafe) calls already in PJIT's
 * source. Capped generously rather than truly unbounded to avoid silently
 * corrupting memory if a call site's assumptions are ever wrong.
 */
int sprintf(char *buf, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(buf, (size_t)4096, fmt, ap);
    va_end(ap);
    return n;
}

int printf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    struct sink s = { NULL, 0, 0 };
    int n = vformat(&s, fmt, ap);
    va_end(ap);
    return n;
}
