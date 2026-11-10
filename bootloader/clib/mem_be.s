

		.global read_BE_byte
		.global read_BE_word
		.global read_BE_long
		.global write_BE_byte
		.global write_BE_word
		.global write_BE_long
        .sect ".text"
        
read_BE_byte:
		setend  BE
		ldrb	r0, [r0]
		setend  LE
		mov		pc, lr

read_BE_word:
		setend  BE
		ldrh	r0, [r0]
		setend  LE
		mov		pc, lr

read_BE_long:
		setend  BE
		ldr 	r0, [r0]
		setend  LE
		mov		pc, lr

write_BE_byte:
		setend  BE
		strb	r1, [r0]
		setend  LE
		mov		pc, lr

write_BE_word:
		setend  BE
		strh	r1, [r0]
		setend  LE
		mov		pc, lr

write_BE_long:
		setend  BE
		str 	r1, [r0]
		setend  LE
		mov		pc, lr
