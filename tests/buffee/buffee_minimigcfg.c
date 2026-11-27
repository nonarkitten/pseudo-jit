/// @file buffee_minimigcfg.c
/// @brief Configures a MiniMig whose FPGA was loaded over JTAG, which its ARM never configures. Needs a MiniMig core with the CPU command register: a write of $B5cc to
///        $DFF1F0 acts as the ARM's OSD command byte cc (Userio.v in Renee's MiniMig tree). Sends the memory, IDE, chipset, floppy, scanline and filter commands (MEM, IDE,
///        CHIPSET, FLOPPY, SCAN, FILTER; build with -DMEM=0xF5 and so on), then gives the boot ROM's "done" (both CIAs selected at once, which takes the boot ROM away) and the reset command 80, in
///        which the memory and IDE settings take effect. Prints `MINIMIGCFG` lines: what it found, the bytes it sent, and the
///        register read back before and after (a core with the read-back: the commands taken, the memory, chipset and IDE config in force,
///        and at $DFF1F2 the last command word the register saw).
#include <stdint.h>
#include "pjit.h"
#include "bbb.h"
#include "buffee.h"
#include "mem_be.h"

#ifndef MEM
#define MEM     0xF5        /* 1111SSCC: 1 MB chip, 512 KB slow */
#endif
#ifndef IDE
#define IDE     0xB3        /* 1011-SMC: the controller and the master */
#endif
#ifndef CHIPSET
#define CHIPSET 0xD0        /* 1101EANT: OCS, PAL, no turbo */
#endif
#ifndef FLOPPY
#define FLOPPY  0xC0        /* 1100FF-S: normal speed, one drive */
#endif
#ifndef SCAN
#define SCAN    0xA0        /* 1010--SS: no scanlines */
#endif
#ifndef FILTER
#define FILTER  0xE0        /* 1110HHLL: no interpolation */
#endif
#define CPUCMD  0xDFF1F0u

/// 1 when a word at a stores two patterns and does not show up at alias (both are written before either is read, so a bus that only holds the last value fails).
static int ram_at(uint32_t a, uint32_t alias)
{
    uint16_t sa = read_BE_word(a), sl = read_BE_word(alias);
    int ok = 1;
    for (int k = 0; k < 2; k++) {
        uint16_t v = k ? 0xA55A : 0x5AA5;
        write_BE_word(a, v); write_BE_word(alias, (uint16_t)~v);
        if (read_BE_word(a) != v || read_BE_word(alias) != (uint16_t)~v) ok = 0;
    }
    write_BE_word(alias, sl); write_BE_word(a, sa);
    return ok;
}

/// The register read back: bits 15-12 the commands it took (mod 16), 11-8 the memory config, 7-4 the chipset config, 3-1 the IDE config, bit 0 always 1.
static void show(const char *when)
{
    uint16_t r = read_BE_word(CPUCMD);
    if (!(r & 1)) { printf("MINIMIGCFG %s: register reads %04X (a core without the read-back)\n", when, r); return; }
    printf("MINIMIGCFG %s: %u commands taken, memory %X, chipset %X, IDE %X; the last command word seen %04X\n", when, r >> 12, (r >> 8) & 15, (r >> 4) & 15, (r >> 1) & 7,
           read_BE_word(CPUCMD + 2));
}

int main(void)
{
    static const uint8_t cmds[] = { MEM, IDE, CHIPSET, FLOPPY, SCAN, FILTER };
    uint8_t id = 0;
    write_BE_word(0xDE1000, 0);                                                     /* the Gayle ID: a write restarts it, four reads give bit 7 = 1, 1, 0, 1 */
    for (int i = 0; i < 4; i++) id = (uint8_t)(id << 1 | read_BE_word(0xDE1000) >> 15);
    *(volatile uint8_t *)0xBFE201 |= 1; *(volatile uint8_t *)0xBFE001 &= 0xFE;    /* OVL off (CIA-A PA0), so that $000000 is chip RAM unless the boot ROM is there */
    int boot = !ram_at(0x000100, 0x040100);                                        /* the boot ROM is mapped over $000000-$003FFF */
    printf("MINIMIGCFG before: boot ROM %s, chip RAM at $080000 %s, slow RAM at $C00000 %s, Gayle ID %X (D = IDE on)\n", boot ? "mapped" : "gone",
           ram_at(0x080000, 0x000000) ? "yes" : "no (512 KB)", ram_at(0xC00000, 0x040000) ? "yes" : "no", id);
    show("before");
    printf("MINIMIGCFG sent");
    for (unsigned i = 0; i < sizeof cmds; i++) {
        write_BE_word(CPUCMD, 0xB500 | cmds[i]);
        printf(" %02X", cmds[i]);
    }
    printf("\n");
    show("after");
    printf("MINIMIGCFG boot ROM done (both CIAs selected), then the reset command 80\n");
    (void)*(volatile uint8_t *)0xBFC001;          /* A12 and A13 low: CIA-A and CIA-B selected together, the boot ROM's "done" (it resets the machine only while the boot ROM is mapped) */
    write_BE_word(CPUCMD, 0xB580);                /* the memory and IDE settings are taken in a reset */
    return 0;
}
