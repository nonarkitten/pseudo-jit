/* PJIT image for Buffee: code, tables and runtime state in the last 6 MB of the 512 MB DDR (see docs/buffee/BUFFEE_MAP.md).
 * tools/pack/lzt_elf.py --image packs this image; the loader (pjit/lzt.h) unpacks it to its link addresses, zeroes the bss and calls the entry point. */
ENTRY(_start)

MEMORY {
    PJIT (rwx) : ORIGIN = 0x9F600000, LENGTH = 0x00600000       /* 0x9F600000 - 0x9FBFFFFF */
}

SECTIONS {
    .text : {
        *(.text.boot)                       /* the exception vectors first: VBAR = 0x9F600000 */
        *(.text*)
    } > PJIT
    .rodata : { *(.rodata*) } > PJIT
    .data   : { *(.data*)   } > PJIT
    .bss : {
        __bss_start = .;
        *(.bss*)
        *(COMMON)
        __bss_end = .;
    } > PJIT
    . = ALIGN(16);
    .stack (NOLOAD) : { . += 0x10000; _stack_top = .; } > PJIT
    ASSERT(_stack_top <= 0x9FC00000, "the PJIT image does not fit in its 6 MB")
}
