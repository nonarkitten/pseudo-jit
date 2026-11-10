/// @file buffee_peekrom.c
/// @brief Prints ROM words through the Amiga window: `ROM <addr>: <words>` for the ranges in R[] (to read the code a sampler PC points at).
#include <stdint.h>
#include "pjit.h"
#include "bbb.h"
#include "buffee.h"
#include "mem_be.h"
int main(void)
{
    static const struct { uint32_t a, n; } R[] = { { 0xF96EE0, 128 } };
    for (unsigned r = 0; r < sizeof R / sizeof R[0]; r++) for (uint32_t i = 0; i < R[r].n; i += 8) {
        printf("ROM %06X:", (unsigned)(R[r].a + 2 * i)); for (int k = 0; k < 8; k++) printf(" %04X", read_BE_word(R[r].a + 2 * (i + k))); printf("\n"); }
    printf("ROM done\n");
    return 0;
}
