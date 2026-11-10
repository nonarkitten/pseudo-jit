/*
 * support.h — what the sources in pjit/ expect from the platform: the cache maintenance after
 * code has been written (platform/arm_cache.c, over the CP15 routines in platform/cp15.S) and the
 * freestanding libc subset (platform/pjit_libc.h; the toolchain has no newlib).
 */
#ifndef _SUPPORT_H
#define _SUPPORT_H

#include <stdint.h>

extern void arm_flush_cache(uintptr_t addr, uint32_t length);

#include "pjit_libc.h"

#endif /* _SUPPORT_H */
