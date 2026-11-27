/* The BeagleBone benchmarks on Buffee (make ONBUFFEE=1): the image as in pjit_dram.ld, but the benchmarks' large static arrays (guest memory, pools)
 * are in the guest pool of the DDR just below the cache, outside the 6 MB image (and within the reach of a bl from the blocks that lie in it). boot.S zeroes the bss.
 * Linked at virtual addresses (BUFFEE_DDR() of the physical ones: platform/buffee/buffee.h), loaded at the physical ones. */
ENTRY(_start)

MEMORY {
    PJIT (rwx) : ORIGIN = 0x1F600000, LENGTH = 0x00600000       /* virtual 0x1F600000 - 0x1FBFFFFF */
    BSS  (rw)  : ORIGIN = 0x1D700000, LENGTH = 0x01800000       /* virtual 0x1D700000 - 0x1EEFFFFF (physical 0x9D700000 -): within 32 MB of the code, which the translated blocks' bl needs */
}

SECTIONS {
    .text : AT(ADDR(.text) + 0x80000000) {
        *(.text.boot)
        *(.text*)
    } > PJIT
    .rodata : AT(ADDR(.rodata) + 0x80000000) { *(.rodata*) } > PJIT
    .data   : AT(ADDR(.data) + 0x80000000) { *(.data*) } > PJIT
    .bss : AT(ADDR(.bss) + 0x80000000) {
        __bss_start = .;
        *(.bss*)
        *(COMMON)
        __bss_end = .;
    } > BSS
    .stack (NOLOAD) : AT(ADDR(.stack) + 0x80000000) { . = ALIGN(16); . += 0x10000; _stack_top = .; } > PJIT
    .bridge (NOLOAD) : AT(ADDR(.bridge) + 0x80000000) { . = ALIGN(16384); *(.bridge) } > PJIT
    ASSERT(. <= 0x1FC00000, "the PJIT image does not fit in its 6 MB")
    /* the block ring is in the bss, so no block is further from the code than the start of the bss */
    ASSERT(ADDR(.text) + SIZEOF(.text) - __bss_start <= 0x02000000, "the bss starts more than 32 MB before the end of the code: a block's bl to the runtime would wrap")
    ASSERT(__bss_start >= 0x1D000000, "the bss is below the bridge table's mapping of the DDR (BUFFEE_BRIDGE_DDR)")
}
