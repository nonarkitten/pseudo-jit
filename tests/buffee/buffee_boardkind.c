/// @file buffee_boardkind.c
/// @brief Writes the board-kind record in the EEPROM (bootloader/hal/config.c: 8 bytes at $0100, 'BKND', the kind and its complement) with KIND (0 through the byte swapper,
///        1 straight; build with -DKIND=n), to test the bootloader's board-kind rules: a record that an Amiga's Kickstart contradicts must make the GreenPAK supply test run
///        again at the next boot, which stores the right kind. Prints `BOARDKIND written <kind>` or the I2C failure; touches nothing else.
#include <stdint.h>
#include "pjit.h"
#include "bbb.h"
#include "buffee.h"

#ifndef KIND
#define KIND 0
#endif
#define R32(a)      (*(volatile uint32_t *)(a))
#define I2C(off)    R32(0x44E0B000u + (off))
#define EEPROM_I2C  0x50
#define TIMEOUT     2000000u            /* cycles (2 ms) */

/// One write to I2C0: the bytes in b, from a START to a STOP. 0 when the slave took them all.
static int i2c_write(uint8_t sa, const uint8_t *b, int n)
{
    uint32_t t = bbb_pmu_cycles(), s;
    int k = 0, rc = 0;

    I2C(0xAC) = sa; I2C(0x98) = n; I2C(0x28) = 0x7FFF; I2C(0xA4) = 0x8600;
    while (I2C(0x24) & (1u << 12)) if (bbb_pmu_cycles() - t > TIMEOUT) return -1;
    I2C(0xA4) |= 3;                                                                     /* START, STOP after n */
    while (k < n) {
        s = I2C(0x24);
        if ((s & 3) || bbb_pmu_cycles() - t > TIMEOUT) { rc = -1; break; }
        if (s & (1u << 4)) { I2C(0x9C) = b[k++]; I2C(0x28) = 1u << 4; }
    }
    while (!(I2C(0x24) & (1u << 8)) && bbb_pmu_cycles() - t < TIMEOUT) ;
    I2C(0x94) |= (1u << 14) | (1u << 6); I2C(0x28) = 0x7FFF; I2C(0x98) = 0;
    return rc;
}

int main(void)
{
    const uint8_t rec[10] = { 0x01, 0x00, 0x42, 0x4B, 0x4E, 0x44, KIND, (uint8_t)~KIND, 0, 0 };     /* address $0100, then 'BKND' (little-endian word), kind, ~kind */
    uint32_t t;

    if (i2c_write(EEPROM_I2C, rec, sizeof rec)) { printf("BOARDKIND: the EEPROM did not take the record\n"); return 0; }
    t = bbb_pmu_cycles(); while (bbb_pmu_cycles() - t < 10000000u) ;                 /* the write cycle */
    printf("BOARDKIND written %d (%s)\n", KIND, KIND ? "straight" : "through the byte swapper");
    return 0;
}
