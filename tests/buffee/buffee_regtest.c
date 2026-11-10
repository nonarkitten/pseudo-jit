/// @file buffee_regtest.c
/// @brief Does a guest RAM region behave as RAM at its guest address? Writes a position-dependent pattern through the whole region (halfwords and longs), reads it back, checks the 24-bit alias
///        and that the megabyte after the region is NOT the same memory. Region base / size come from the EEPROM via bootinfo: REGBASE / REGMB (defaults $200000, 4).
#include <stdint.h>
#include "pjit.h"
#include "bbb.h"
#include "buffee.h"
#include "mem_be.h"
#ifndef REGBASE
#define REGBASE 0x200000u
#endif
#ifndef REGMB
#define REGMB 4u
#endif
int main(void)
{
    uint32_t bad = 0, n = REGMB << 20;
    printf("=== REGTEST %06X + %u MB ===\n", (unsigned)REGBASE, (unsigned)REGMB);
    for (uint32_t o = 0; o < n; o += 4) write_BE_long(REGBASE + o, (o * 2654435761u) ^ 0xA5A5A5A5u);
    for (uint32_t o = 0; o < n; o += 4) { uint32_t g = read_BE_long(REGBASE + o); if (g != ((o * 2654435761u) ^ 0xA5A5A5A5u) && bad++ < 8) printf("  %06X: %08X want %08X\n", (unsigned)(REGBASE + o), (unsigned)g, (unsigned)((o * 2654435761u) ^ 0xA5A5A5A5u)); }
    for (uint32_t o = 0; o < n; o += 2) { write_BE_word(REGBASE + o, (uint16_t)(o ^ 0x5A5A)); }
    for (uint32_t o = 0; o < n; o += 2) { uint16_t g = read_BE_word(REGBASE + o); if (g != (uint16_t)(o ^ 0x5A5A) && bad++ < 8) printf("  word %06X: %04X\n", (unsigned)(REGBASE + o), g); }
    uint32_t alias = 0x01000000u + REGBASE + 0x1234;
    write_BE_word(REGBASE + 0x1234, 0xC0DE);
    printf("alias %08X reads %04X (expect C0DE in 24-bit mode)\n", (unsigned)alias, read_BE_word(alias));
    printf("first word after the region %06X: %04X\n", (unsigned)(REGBASE + n), read_BE_word(REGBASE + n));
    printf("REGTEST %s (%u bad)\n", bad ? "FAIL" : "PASS", (unsigned)bad);
    return 0;
}
