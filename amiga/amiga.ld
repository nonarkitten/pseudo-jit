/* amiga.ld -- an Amiga program as one code hunk: everything (code, constants, the run lock and the zeroed data) in one section linked at 0, start.S first. Linked with
 * --emit-relocs, so tools/amiga/elf2hunk.py finds every absolute address in it and writes them as the hunk's relocations.
 * A resident program is one copy that every run shares (amiga/buffee/resident.S), so a run cannot rely on what the file held: start.S zeroes the bss at each start, and
 * writable data with an initial value is refused here (a static with a non-zero initialiser would keep what the previous run left in it). */
ENTRY(_start)
SECTIONS {
    . = 0;
    .hunk : {
        *(.text.start)
        *(.text*)
        *(.rodata*)
        __data_start = .;
        *(.data*)
        __data_end = .;
        *(.lock)
        . = ALIGN(4);
        __bss_start = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        __bss_end = .;
    }
    /DISCARD/ : { *(.comment) *(.note*) *(.debug*) }
}
ASSERT(__data_end == __data_start, "amiga.ld: writable data with an initial value (it would keep the previous run's value in a resident copy): make it const, or zero")
