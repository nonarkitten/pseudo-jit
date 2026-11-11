SECTIONS { . = 0x1000; .text : { *(.text.entry) *(.text*) *(.rodata*) } .data : { *(.data*) } }
