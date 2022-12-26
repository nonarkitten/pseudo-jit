/* entry point of the program, first instruction executed */
ENTRY(pjit_entry) 

SECTIONS
{
	. = 0x80000000;
		
	.text : {
		*main.o(.text*)
		*(.text*)
        *(.rodata*)
	}
	
	/* non-zero global and static variables initialized by the application code */
	.data : { 
		. = ALIGN(4);
		*(.data)
	}
	
	/* global and static variable that are not initialized by the application code. */
	/* the compiler (hopefully) initialize this data to 0. */
	.bss :  {
		. = ALIGN(4);
		_bss_start = .;
		*(.bss)
		. = ALIGN(4);
		_bss_end = .;
	}
}