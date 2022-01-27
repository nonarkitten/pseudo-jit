/* This is a simple version of setjmp and longjmp.
   Nick Clifton, Cygnus Solutions, 13 June 1997.  */

/* Arm/Thumb interworking support:
   The interworking scheme expects functions to use a BX instruction
   to return control to their parent.  Since we need this code to work
   in both interworked and non-interworked environments as well as with
   older processors which do not have the BX instruction we do the
   following:
	Test the return address.
	If the bottom bit is clear perform an "old style" function exit.
	(We know that we are in ARM mode and returning to an ARM mode caller).
	Otherwise use the BX instruction to perform the function exit.
   We know that we will never attempt to perform the BX instruction on
   an older processor, because that kind of processor will never be
   interworked, and a return address with the bottom bit set will never
   be generated.
   In addition, we do not actually assemble the BX instruction as this would
   require us to tell the assembler that the processor is an ARM7TDMI and
   it would store this information in the binary.  We want this binary to be
   able to be linked with binaries compiled for older processors however, so
   we do not want such information stored there.
   If we are running using the APCS-26 convention however, then we never
   test the bottom bit, because this is part of the processor status.
   Instead we just do a normal return, since we know that we cannot be
   returning to a Thumb caller - the Thumb does not support APCS-26.
   Function entry is much simpler.  If we are compiling for the Thumb we
   just switch into ARM mode and then drop through into the rest of the
   function.  The function exit code will take care of the restore to
   Thumb mode.
   For Thumb-2 do everything in Thumb mode.  */
#if defined(__ARM_ARCH_6M__)

/* ARMv6-M has to be implemented in Thumb mode.  */
	.thumb
	.thumb_func
	.globl setjmp

setjmp:
	/* Save registers in jump buffer.  */
	stmia	r0!, {r4, r5, r6, r7}
	mov	r1, r8
	mov	r2, r9
	mov	r3, r10
	mov	r4, fp
	mov	r5, sp
	mov	r6, lr
	stmia	r0!, {r1, r2, r3, r4, r5, r6}
	sub	r0, r0, #40
	/* Restore callee-saved low regs.  */
	ldmia	r0!, {r4, r5, r6, r7}
	/* Return zero.  */
	mov	r0, #0
	bx lr

.thumb_func
	.globl longjmp
longjmp:
	/* Restore High regs.  */
	add	r0, r0, #16
	ldmia	r0!, {r2, r3, r4, r5, r6}
	mov	r8, r2
	mov	r9, r3
	mov	r10, r4
	mov	fp, r5
	mov	sp, r6
	ldmia	r0!, {r3} /* lr */
	/* Restore low regs.  */
	sub	r0, r0, #40
	ldmia	r0!, {r4, r5, r6, r7}
	/* Return the result argument, or 1 if it is zero.  */
	mov	r0, r1
	bne	1f
	mov	r0, #1
1:
	bx	r3
