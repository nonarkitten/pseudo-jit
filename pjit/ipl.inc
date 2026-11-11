@ IRQ_VERIFY rl, ta, tb -- at the points where the router decides to deliver an interrupt: rl = cpu->irq_level as the last host IRQ left it. On a platform whose interrupt lines are
@ a level (Buffee: IPL2-0 on GPIO2_6..8, active low; -Wa,--defsym,PJIT_IPL_GPIO=<GPIO2 DATAIN address>) the cached level can be stale, because a guest's acknowledge of the
@ device is a posted write that reaches the pins after the RTE (and the edge interrupt after that): read the pins now (twice, until two reads agree), store the result back.
@ Level 7 (an edge) and 0 are left alone. E=LE; clobbers ta, tb and the flags.
	.macro	IRQ_VERIFY rl, ta, tb
	.ifdef	PJIT_IPL_GPIO
	cmp	\rl, #0
	beq	99f
	cmp	\rl, #7
	bhs	99f
	cpsid	i                               @ the edge interrupt must not rewrite the level between this check and the delivery (the exits UNMASK again)
	dsb
	mov	\ta, #0xF80000                 @ a read of the Kickstart ROM: the GPMC does not pass a read over a posted write, so the guest's acknowledge has reached Paula when it returns
	ldrh	\tb, [\ta]
	movw	\ta, #:lower16:PJIT_IPL_GPIO
	movt	\ta, #:upper16:PJIT_IPL_GPIO
98:	ldr	\rl, [\ta]
	ldr	\tb, [\ta]
	eor	\tb, \tb, \rl
	tst	\tb, #0x1C0
	bne	98b
	mvn	\rl, \rl
	ubfx	\rl, \rl, #6, #3                @ bit 0 = IPL2, bit 1 = IPL1, bit 2 = IPL0
	and	\tb, \rl, #2
	tst	\rl, #1
	orrne	\tb, \tb, #4
	tst	\rl, #4
	orrne	\tb, \tb, #1
	mov	\rl, \tb
	str	\rl, [r5, #CPU_IRQ_LEVEL]
99:
	.endif
	.endm
