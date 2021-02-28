#include <stdlib.h>
#include <stdint.h>

#if 0

typedef uint32_t uint32x2_t __attribute__ ((vector_size (8)));
typedef  int32_t  int32x2_t __attribute__ ((vector_size (8)));

uint32_t __aeabi_uidiv(uint32_t numerator, uint32_t denominator) {
	return (uint32_t)((double)numerator / (double)denominator);
}

uint32x2_t __aeabi_uidivmod(uint32_t numerator, uint32_t denominator) {
    double d = (double)denominator;
    uint32_t q = (uint32_t)((double)numerator / d);
    uint32_t r = numerator - ((double)q * d);
    uint32x2_t ret = { q, r };
    return ret;
}

int32_t __aeabi_idiv(int32_t numerator, int32_t denominator) {
	return (int32_t)((double)numerator / (double)denominator);
}

int32x2_t __aeabi_idivmod(int32_t numerator, int32_t denominator) {
    double d = (double)denominator;
    int32_t q = (int32_t)((double)numerator / d);
    int32_t r = numerator - ((double)q * d);
    int32x2_t ret = { q, r };
    return ret;
}

#endif

div_t div (int num, int denom) {
	div_t r;

	r.quot = num / denom;
	r.rem = num % denom;
	/*
	 * The ANSI standard says that |r.quot| <= |n/d|, where
	 * n/d is to be computed in infinite precision.  In other
	 * words, we should always truncate the quotient towards
	 * 0, never -infinity or +infinity.
	 *
	 * Machine division and remainer may work either way when
	 * one or both of n or d is negative.  If only one is
	 * negative and r.quot has been truncated towards -inf,
	 * r.rem will have the same sign as denom and the opposite
	 * sign of num; if both are negative and r.quot has been
	 * truncated towards -inf, r.rem will be positive (will
	 * have the opposite sign of num).  These are considered
	 * `wrong'.
	 *
	 * If both are num and denom are positive, r will always
	 * be positive.
	 *
	 * This all boils down to:
	 *	if num >= 0, but r.rem < 0, we got the wrong answer.
	 * In that case, to get the right answer, add 1 to r.quot and
	 * subtract denom from r.rem.
	 *      if num < 0, but r.rem > 0, we also have the wrong answer.
	 * In this case, to get the right answer, subtract 1 from r.quot and
	 * add denom to r.rem.
	 */
	if (num >= 0 && r.rem < 0) {
		++r.quot;
		r.rem -= denom;
	}
	else if (num < 0 && r.rem > 0) {
		--r.quot;
		r.rem += denom;
	}
	return (r);
}

#include "main.h"
#include "support.h"

void arm_flush_cache(uintptr_t addr, uint32_t length) {
    length = (length + 31) & ~31;
    while (length) {
        asm volatile("mcr p15, 0, %0, c7, c14, 1"::"r"(addr));
        addr += 32;
        length -= 32;
    }
    asm volatile("mcr p15, 0, %0, c7, c10, 4"::"r"(addr));
}

void arm_icache_invalidate(uintptr_t addr, uint32_t length) {
    length = (length + 31) & ~31;
    while (length) {
        asm volatile("mcr p15, 0, %0, c7, c5, 1"::"r"(addr));
        addr += 32;
        length -= 32;
    }
    asm volatile("mcr p15, 0, %0, c7, c10, 4"::"r"(addr));
}

void arm_dcache_invalidate(uintptr_t addr, uint32_t length) {
    length = (length + 31) & ~31;
    while (length) {
        asm volatile("mcr p15, 0, %0, c7, c6, 1"::"r"(addr));
        addr += 32;
        length -= 32;
    }
    asm volatile("mcr p15, 0, %0, c7, c10, 4"::"r"(addr));
}

const char *remove_path(const char *in) {
    const char *p = &in[strlen(in)-1];
    while (p > in && p[-1] != '/' && p[-1] != ':') p--;
    return p;
}

size_t strlen(const char *c) {
    size_t result = 0;
    while (*c++) result++;
    return result;
}

int strcmp(const char *s1, const char *s2) {
	while (*s1 == *s2++)
		if (*s1++ == '\0')
			return (0);
        
	return (*(const unsigned char *)s1 - *(const unsigned char *)(s2 - 1));
}


void *memmove(void *dst, const void *src, size_t sz) {
    uint8_t *d = dst;
    const uint8_t *s = src;

    if (d > s) {
        d += sz;
        s += sz;

        while(sz--)
            *--d = *--s;
    } else {
        while(sz--)
            *d++ = *s++;
    }

    return dst;
}

void memcpy(void *dst, const void *src, size_t sz)
{
    uint8_t *d = dst;
    const uint8_t *s = src;

    while(sz--)
	*d++ = *s++;
}

void memset(void *ptr, uint8_t fill, size_t sz)
{
    uint8_t *p = ptr;
    if (p)
        while(sz--)
            *p++ = fill;
}

void bzero(void *ptr, size_t sz)
{
    char *p = ptr;
    if (p)
        while(sz--)
            *p++ = 0;
}