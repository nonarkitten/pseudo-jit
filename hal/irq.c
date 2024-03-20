#include <stdint.h>

#include "ioam3358.h"

#define NUM_INTERRUPTS                          (128u)

static void (*fnRAMVectors[NUM_INTERRUPTS])(void);

/* The Default Interrupt Handler.
 * This is the default interrupt handler for all interrupts.  It simply loops
 * forever so that the system state is preserved for observation by a
 * debugger.  Since interrupts should be disabled before unregistering the
 * corresponding handler, this should never be called. */
static void IntDefaultHandler(void) {
    while (1) continue;
}

void IntRegister(uint32_t intrNum, void (*fnHandler)(void)) {
    fnRAMVectors[intrNum] = fnHandler;
}

void IntUnRegister(uint32_t intrNum) {
    fnRAMVectors[intrNum] = IntDefaultHandler;
}