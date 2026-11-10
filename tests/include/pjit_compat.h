/// @file pjit_compat.h
/// @brief Lets the smoke tests, written as "dispatch, then call into the cache", run through pjit_run().
///
/// pjit_cache_dispatch(pc) returns the guest PC (non-zero, so the tests' `if (!arm_entry)` checks pass). PJIT_TEST_RUN(entry) runs the guest until its TRAP #15 and then
/// calls the test's own pjit_test_exit() hook with D0. Guest code above 16MB runs from the direct-mapped cache (the default threshold).
#ifndef PJIT_COMPAT_H
#define PJIT_COMPAT_H
#include <stdint.h>
#include "pjit.h"

extern void pjit_test_exit(uint32_t d0);

static inline uint32_t pjit_compat_dispatch(uint32_t guest_pc) { return guest_pc; }
#define pjit_cache_dispatch(pc) pjit_compat_dispatch(pc)

/// Stacks for a harness that sets them from C: the shadows are big-endian.
#define PJIT_SET_STACKS(x) (cpu->ssp = cpu->usp = __builtin_bswap32(x))

/// Runs the guest at `pc` until its TRAP #15. A7 is the guest's stack (the ARM stack pointer); if the
/// harness never set one, point it at a private area.
static inline uint32_t pjit_compat_run(uint16_t *pc)
{
    static uint8_t pjit_compat_stack[4096] __attribute__((aligned(16)));
    if (!pjit_get_an(7)) pjit_set_an(7, (uint32_t)(pjit_compat_stack + sizeof(pjit_compat_stack)));
    pjit_cache_flush();         // harnesses reuse guest addresses between runs
    return pjit_run(pc);
}

#define PJIT_TEST_RUN(entry) do { \
        (void)pjit_compat_run((uint16_t *)(entry)); \
        pjit_test_exit(pjit_get_dn(0)); \
    } while (0)
#endif
