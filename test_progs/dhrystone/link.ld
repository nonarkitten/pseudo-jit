/* Linker script of the Dhrystone guest: a flat image at guest address 0x1000 (see tools/data/dhrystone.py). The start-up code is first. */
ENTRY(_start)
SECTIONS
{
    . = 0x1000;
    .text   : { *(.text.start) *(.text*) *(.rodata*) }
    .data   : { *(.data*) }
    .bss    : { *(.bss*) *(COMMON) }
    /DISCARD/ : { *(.comment) *(.note*) *(.eh_frame*) *(.m68k*) }
}
