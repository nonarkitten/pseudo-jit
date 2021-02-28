/*
    Copyright © 2019 Michal Schulz <michal.schulz@gmx.de>
    https://github.com/michalsc

    This Source Code Form is subject to the terms of the
    Mozilla Public License, v. 2.0. If a copy of the MPL was not distributed
    with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef _SUPPORT_H
#define _SUPPORT_H

#include <stdint.h>
#include <stdarg.h>

#include "arm.h"

#define TRUE 1
#define FALSE 0

#define LE16(x) __builtin_bswap16(x)
#define LE32(x) __builtin_bswap32(x)
#define LE64(x) __builtin_bswap64(x)
//(((L32(x)) << 32) | L32(((x) >> 32) & 0xffffffff))

#define BE16(x) (x)
#define BE32(x) (x)
#define BE64(x) (x)

static inline uint32_t rd32le(uintptr_t iobase) {
    return LE32(*(volatile uint32_t *)(iobase));
}

static inline uint32_t rd32be(uintptr_t iobase) {
    return BE32(*(volatile uint32_t *)(iobase));
}

static inline uint16_t rd16le(uintptr_t iobase) {
    return LE16(*(volatile uint16_t *)(iobase));
}

static inline uint16_t rd16be(uintptr_t iobase) {
    return BE16(*(volatile uint16_t *)(iobase));
}

static inline uint8_t rd8(uintptr_t iobase) {
    return *(volatile uint8_t *)(iobase);
}

static inline void wr32le(uintptr_t iobase, uint32_t value) {
    *(volatile uint32_t *)(iobase) = LE32(value);
}

static inline void wr32be(uintptr_t iobase, uint32_t value) {
    *(volatile uint32_t *)(iobase) = BE32(value);
}

static inline void wr16le(uintptr_t iobase, uint16_t value) {
    *(volatile uint16_t *)(iobase) = LE16(value);
}

static inline void wr16be(uintptr_t iobase, uint16_t value) {
    *(volatile uint16_t *)(iobase) = BE16(value);
}

static inline void wr8(uintptr_t iobase, uint8_t value) {
    *(volatile uint8_t *)(iobase) = value;
}

#define kprintf printf

void arm_flush_cache(uintptr_t addr, uint32_t length);
void arm_icache_invalidate(uintptr_t addr, uint32_t length);
void arm_dcache_invalidate(uintptr_t addr, uint32_t length);
const char *remove_path(const char *in);
size_t strlen(const char *c);
int strcmp(const char *s1, const char *s2);
void *memmove(void *dst, const void *src, size_t sz);
void memcpy(void *dst, const void *src, size_t sz);
void memset(void *ptr, uint8_t fill, size_t sz);
void bzero(void *ptr, size_t sz);

extern void * tlsf;
extern void * jit_tlsf;

#endif /* _SUPPORT_H */