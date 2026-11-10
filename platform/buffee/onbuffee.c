/*
 * onbuffee.c — lets the BeagleBone benchmarks (tests/bbb) run on the Buffee board (`make ONBUFFEE=1 VARIANT=onbuffee bundle-bbb_dhrystone`).
 * They call pjit_cache_init() with the top of a static pool of their own; on Buffee the cache must be where the platform expects it
 * (BUFFEE_CACHE_TOP), and the platform's translate map and write detection, which know nothing of guest code in static arrays, are switched off. Linked with -Wl,--wrap=pjit_cache_init.
 */
#include <stdint.h>
#include "pjit.h"
#include "buffee.h"

extern void __real_pjit_cache_init(uint32_t top);

void __wrap_pjit_cache_init(uint32_t top)
{
    (void)top;
    __real_pjit_cache_init(BUFFEE_CACHE_TOP);
    cpu->xlat = 0; cpu->codebits = 0; cpu->codeexact = 0;                                   // no translate map: as on the BeagleBone, the test's own dr_threshold decides what is cached
    cpu->dr_threshold = DR_THRESHOLD_DEFAULT;
}
