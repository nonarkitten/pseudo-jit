/* The BeagleBone benchmarks on Buffee (make ONBUFFEE=1): the image as in pjit_dram.ld, but the benchmarks' large static arrays (guest memory, pools)
 * are in the guest pool of the DDR just below the cache, outside the 6 MB image (and within the reach of a bl from the blocks that lie in it). The loader zeroes the bss wherever it is. */
ENTRY(_start)

MEMORY {
    PJIT (rwx) : ORIGIN = 0x9F600000, LENGTH = 0x00600000       /* 0x9F600000 - 0x9FBFFFFF */
    BSS  (rw)  : ORIGIN = 0x9D700000, LENGTH = 0x01800000       /* 0x9D700000 - 0x9EEFFFFF: within 32 MB of the code, which the translated blocks' bl needs */
}

SECTIONS {
    .text : {
        *(.text.boot)
        *(.text*)
    } > PJIT
    .rodata : { *(.rodata*) } > PJIT
    .data   : { *(.data*)   } > PJIT
    .bss : {
        __bss_start = .;
        *(.bss*)
        *(COMMON)
        __bss_end = .;
    } > BSS
    .stack (NOLOAD) : { . = ALIGN(16); . += 0x10000; _stack_top = .; } > PJIT
    ASSERT(_stack_top <= 0x9FC00000, "the PJIT image does not fit in its 6 MB")
    /* the block ring is in the bss, so no block is further from the code than the start of the bss */
    ASSERT(ADDR(.text) + SIZEOF(.text) - __bss_start <= 0x02000000, "the bss starts more than 32 MB before the end of the code: a block's bl to the runtime would wrap")
}
