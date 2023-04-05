extern void arm_flush_cache(uintptr_t addr, uint32_t length);
extern void arm_icache_invalidate(uintptr_t addr, uint32_t length);
extern void arm_dcache_invalidate(uintptr_t addr, uint32_t length);