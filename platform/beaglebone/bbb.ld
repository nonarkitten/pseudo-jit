/* Linker script for the BeagleBone (AM335x): a flat image U-Boot loads at 0x82000000 and starts with `go 0x82000000`. */
ENTRY(_start)

MEMORY {
    RAM (rwx) : ORIGIN = 0x82000000, LENGTH = 0x05F00000      /* up to the RAM log at 0x87F00000 */
}

SECTIONS {
    . = 0x82000000;
    .text : {
        *(.text.boot)
        *(.text*)
    } > RAM

    .rodata : { *(.rodata*) } > RAM
    .data   : { *(.data*)   } > RAM

    .bss : {
        __bss_start = .;
        *(.bss*)
        *(COMMON)
        __bss_end = .;
    } > RAM

    . = ALIGN(16);
    . = . + 0x10000; /* 64KB stack */
    _stack_top = .;
}
