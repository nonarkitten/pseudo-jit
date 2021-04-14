		.globl	sh_printstr

sh_printstr:
		mov		r1, r0
		mov		r0, #0x04		@ SYS_WRITE0
		svc		#0x123456
		mov	pc, lr

		.end
