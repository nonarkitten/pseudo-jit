@ amiga_hook.S -- the interpreter's prefetch hook of tests/qemu/amiga_diff.c (cpu->prefetch_hook): called before every instruction with the guest's flags and D0 live.
@ diff_state (two BIG-endian words: the instructions completed so far, the count at which the C side wants control) is read and written here under E=BE.
@ Below the deadline: count the instruction and go on. At it: `svc #0x104` (runtime.s ends pjit_run() with the guest parked); the C side handles the event, counts the
@ instruction itself and calls pjit_run() again at the same PC. No flag is touched.
	.syntax	unified
	.arm
	.text
	.global	pjit_prefetch_diff
	.type	pjit_prefetch_diff, %function
pjit_prefetch_diff:
	movw	r0, #:lower16:diff_state
	movt	r0, #:upper16:diff_state
	ldr	r1, [r0]                        @ completed (a BE load of a word the C side stored byte-swapped)
	ldr	r2, [r0, #4]                    @ deadline
	sub	r3, r1, r2
	lsr	r3, r3, #31                     @ 1 while completed < deadline
	add	pc, pc, r3, lsl #2              @ pc reads as .+8: the entry below the nop
	nop
	svc	#0x104                          @ at the deadline: the C side takes over
	b	diff_go
diff_go:
	add	r1, r1, #1
	str	r1, [r0]
	b	pjit_prefetch
