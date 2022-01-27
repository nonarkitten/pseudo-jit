		.globl	bogo_delay
bogo_delay:
		subs	r0, r0, #1
		bcs		bogo_delay
		bx		lr

		.end
