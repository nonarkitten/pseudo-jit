/****************************************************************************/
/*  AM335x.lds                                                              */
/*  Copyright (c) 2014  Texas Instruments Incorporated                      */
/*  Author: Rafael de Souza                                                 */
/*                                                                          */
/*    Description: This file is a sample linker command file that can be    */
/*                 used for linking programs built with the GCC compiler    */
/*                 and running the resulting .out file on an AM335x device. */
/*                 Use it as a guideline.  You will want to                 */
/*                 change the memory layout to match your specific          */
/*                 target system.  You may want to change the allocation    */
/*                 scheme according to the size of your program.            */
/*                                                                          */
/****************************************************************************/

MEMORY
{
    IRAM_MEM : o = 0x402F0400,  l = 0x0001FC00  /* 127k internal Memory */
    M3SHUMEM : o = 0x44D00000,  l = 0x00004000  /* 16kB M3 Shared Unified Code Space */
    M3SHDMEM : o = 0x44D80000,  l = 0x00002000  /* 8kB M3 Shared Data Memory */
    DDR0 :     o = 0x80000000,  l = 0x20000000  /* 1GB external DDR Bank 0 */

}

/* Linker script to place sections and symbol values. Should be used together
 * with other linker script that defines memory region DDR0.
 * It references following symbols, which must be defined in code:
 *   Entry : Entry of reset handler
 * 
 * It defines following symbols, which code can use without definition:
 *   __exidx_start
 *   __exidx_end
 *   __etext
 *   __data_start__
 *   __preinit_array_start
 *   __preinit_array_end
 *   __init_array_start
 *   __init_array_end
 *   __fini_array_start
 *   __fini_array_end
 *   __data_end__
 *   __bss_start__
 *   __bss_end__
 *   __end__
 *   end
 *   __HeapLimit
 *   __HeapBase        - To be compatible with Linaro's semihosting support 
 *   __StackLimit
 *   __StackTop
 *   __StackBase    - To be compatible with Linaro's semihosting support 
 *   __stack
 */
ENTRY(Entry)

SECTIONS
{
        .rsthand :
        {
            . = ALIGN(4);
            *startup.o      (.isr_vector)
            *startup.o      (.text)
        } >DDR0

        .text :
        {
            . = ALIGN(4);
            *(.text*)
            *(.rodata*)
        } >DDR0

        .data :
        {
            . = ALIGN(4);
            *(.data*)
        } >DDR0

        .bss :
        {
            . = ALIGN(4);
            _bss_start = .;
            *(.bss*)
            *(COMMON)
            _bss_end = .;
        } >DDR0

	    .data :
	    {
	        . = ALIGN(4);
	        __data_start__ = .;
	        *(vtable)
	        *(.data*)

	        . = ALIGN(4);
	        /* preinit data */
	        PROVIDE_HIDDEN (__preinit_array_start = .);
	        KEEP(*(.preinit_array))
	        PROVIDE_HIDDEN (__preinit_array_end = .);

	        . = ALIGN(4);
	        /* init data */
	        PROVIDE_HIDDEN (__init_array_start = .);
	        KEEP(*(SORT(.init_array.*)))
	        KEEP(*(.init_array))
	        PROVIDE_HIDDEN (__init_array_end = .);


	        . = ALIGN(4);
	        /* finit data */
	        PROVIDE_HIDDEN (__fini_array_start = .);
	        KEEP(*(SORT(.fini_array.*)))
	        KEEP(*(.fini_array))
	        PROVIDE_HIDDEN (__fini_array_end = .);

	        . = ALIGN(4);
	        /* All data end */
	        __data_end__ = .;

	    } > DDR0

        .heap :
        {
            . = ALIGN(4);
            __end__ = .;
            end = __end__;
            __HeapBase = __end__;
            *(.heap*)
            . = . + 0x800;
            __HeapLimit = .;
        } >DDR0

        .stack :
        {
            . = ALIGN(256);
            __StackLimit = . ;
            *(.stack*)
            . = . + 0x800;
            __StackTop = .;
        } >DDR0
        _stack = __StackTop;

}
/**************************************************************************/
