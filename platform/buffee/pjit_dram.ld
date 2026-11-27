/* PJIT image for Buffee: code, tables and runtime state in the last 6 MB of the 512 MB DDR (see docs/buffee/BUFFEE_MAP.md).
 * Linked at its virtual address (BUFFEE_DDR() of the physical one: platform/buffee/buffee.h) and loaded at the physical one (AT: 0x80000000 higher), where the bootloader calls it.
 * tools/pack/lzt_elf.py --image packs this image at its load addresses; the loader (pjit/lzt.h) unpacks it there; boot.S zeroes the bss. */
ENTRY(_start)

MEMORY {
    PJIT (rwx) : ORIGIN = 0x1F600000, LENGTH = 0x00600000       /* virtual 0x1F600000 - 0x1FBFFFFF */
}

SECTIONS {
    .text : AT(ADDR(.text) + 0x80000000) {
        *(.text.boot)                       /* the exception vectors first: VBAR = 0x1F600000 */
        *(.text*)
    } > PJIT
    .rodata : AT(ADDR(.rodata) + 0x80000000) { *(.rodata*) } > PJIT
    .data   : AT(ADDR(.data) + 0x80000000) { *(.data*) } > PJIT
    .bss : AT(ADDR(.bss) + 0x80000000) {
        __bss_start = .;
        *(.bss*)
        *(COMMON)
        __bss_end = .;
    } > PJIT
    . = ALIGN(16);
    .stack (NOLOAD) : AT(ADDR(.stack) + 0x80000000) { . += 0x10000; _stack_top = .; } > PJIT
    .bridge (NOLOAD) : AT(ADDR(.bridge) + 0x80000000) { . = ALIGN(16384); *(.bridge) } > PJIT
    ASSERT(. <= 0x1FC00000, "the PJIT image does not fit in its 6 MB")
}
