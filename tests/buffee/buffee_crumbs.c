/// @file buffee_crumbs.c
/// @brief Prints the hang breadcrumbs that a -DCRUMBS payload left in DDR (0x9F5FFC00: counter, then 16 x {tag 1 = slot-line translation / 2 = block translation / 3 = write fault, address, length or PC, sequence}) after a watchdog reset.
#include <stdint.h>
#include "pjit.h"
#include "bbb.h"
#include "buffee.h"
int main(void)
{
    volatile uint32_t *c = (volatile uint32_t *)0x9F5FFC00u;
    printf("CRUMBS count %u\n", (unsigned)c[0]);
    for (uint32_t k = 0; k < 16; k++) { volatile uint32_t *e = c + 4 + 4 * k; printf("CRUMB slot %2u: seq %u tag %u a %08X b %08X\n", (unsigned)k, (unsigned)e[3], (unsigned)e[0], (unsigned)e[1], (unsigned)e[2]); }
    return 0;
}
