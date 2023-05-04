/*
 * cstd.c
 *
 *  Created on: Apr. 14, 2021
 *      Author: renee.cousins
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>

#include <arm_neon.h>

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
     *  if num >= 0, but r.rem < 0, we got the wrong answer.
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

size_t strlen(const char *c) {
    size_t result = 0;
    while (*c++) result++;
    return result;
}

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

void* memcpy(void *dst, const void *src, size_t sz) {
    uint8_t *d = dst;
    const uint8_t *s = src;

    while(sz--) *d++ = *s++;
    return dst;
}

void* memset(void *ptr, int fill, size_t sz) {
    uint8_t *p = ptr;
    if (p) while(sz--) *p++ = (uint8_t)fill;
    return ptr;
}

void bzero(void *ptr, size_t sz) {
    char *p = ptr;
    if (p) while(sz--) *p++ = 0;
}

// Returns a pointer to the first occurrence of str2 in str1, or a null pointer if str2 is not part of str1.
const char * strstr ( const char * str1, const char * str2 ) {
    while(*str1) {
        int i = 0;
        while(str1[i] && (str1[i] == str2[i])) i++;
        if(str2[i] == 0) return str1;
        str1++;
    }
    return NULL;
}

// Copy string
char * strcpy ( char * destination, const char * source ) {
    char c, *d = destination;
    const char *s = source;
    while((c = *s++)) *d++ = c;
    *d = 0;
    return destination;
}

// Locate last occurrence of character in string
const char * strrchr ( const char * str, int character ) {
    const char *s = str, *e = NULL;
    char c;
    while((c = *s)) {
        if(c == character) e = s;
        s++;
    }
    return e;
}

// Copy characters from string
char * strncpy ( char * destination, const char * source, size_t num ) {
    char c = 0, *d = destination;
    const char *s = source;
    while(num-- && (c = *s++)) *d++ = c;
    *d = c;
    return destination;
}

// Append characters from string
char * strncat ( char * destination, const char * source, size_t num ) {
    char *d = destination + strlen(source);
    return strncpy(d, source, num);
}

int echo_on = 0;
void SetEcho(int on) {
    echo_on = on;
}

extern uint8_t _getchar(void);
extern void _putchar(uint8_t);

int puts ( const char * str ) {
    return printf("%s\r\n", str);
}

int putchar ( int character ) {
    _putchar(character);
    return character;
}

int getchar ( void ) {
    return _getchar();
}

char * gets (char * str) {
    int i = 0;
    char c;
    do {
        c = _getchar();
        if(c == '\b') {
            if(i) {
                if(echo_on) printf("\b \b");
                i--;
            }
        } else {
            if(c == '\n' || c == '\r') {
                putchar('\n');
                c = 0;
            }
            if(c && echo_on) putchar(c);
            str[i++] = c;
        }
    } while(c);
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

int toupper(int c) { 
    if(c < 'a' || c > 'z') return c;
    else return c - 'a' + 'A';
}

double strtod(const char* in, char** out) {
    int v = 0;
    int d = 0;
    int div = 1;
    int c;

    while(c = *in++) {
        if(c == 0) break;
        else if(c == '.') { d = 1; continue; }
        else if(c >= '0' && c <= '9') {
            v = (v * 10) + (c - '0');
            if(d) div *= 10;
        }
        else break;
    }

    return (double)v / (double)div;
}