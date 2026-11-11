/* Linker script for QEMU's realview-pb-a8 (a Cortex-A8 core model; RAM at 0x70000000), loaded via -kernel. */
ENTRY(_start)

MEMORY {
    RAM (rwx) : ORIGIN = 0x70000000, LENGTH = 128M
}

SECTIONS {
    . = 0x70000000;
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
