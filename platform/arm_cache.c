/*
 * arm_cache.c — arm_flush_cache, declared in pjit/support.h, over the CP15 routines in cp15.S.
 *
 * After code has been written to [addr, addr + length): clean and invalidate the data cache over
 * the range (so that no pending write is lost and the instruction side sees the new words), then
 * invalidate the instruction cache over it.
 */
#include <stdint.h>
#include "cp15.h"

void arm_flush_cache(uintptr_t addr, uint32_t length)
{
    CP15DCacheCleanFlushBuff((uint32_t)addr, length);
    CP15ICacheFlushBuff((uint32_t)addr, (uint32_t)addr + length);
}
