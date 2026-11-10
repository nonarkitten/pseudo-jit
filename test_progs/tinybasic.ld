/* Linker script of test_progs/tinybasic.c: a flat image at guest address 0x1000 (see tools/data/tinybasic.py). The code is first, `_start` leads. */
ENTRY(_start)
SECTIONS
{
    . = 0x1000;
    .text   : { *(.text.start) *(.text*) *(.rodata*) }
    .data   : { *(.data*) }
    .bss    : { *(.bss*) *(COMMON) }
    /DISCARD/ : { *(.comment) *(.note*) *(.eh_frame*) *(.m68k*) }
}
