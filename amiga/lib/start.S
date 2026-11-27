/* start.S -- the entry of an Amiga Shell program built with m68k-elf-gcc (amiga/amiga.ld, tools/amiga/elf2hunk.py): exec's base from address 4, then
 * amiga_main(length, arguments) with the argument line the Shell passes in D0 / A0 (not terminated; it ends with a newline). Its result is the return code.
 * The program is started from a Shell only: it has no Workbench icon, so no Workbench start-up message to answer. Everything is PC-relative (-mpcrel): the program has no
 * relocations and runs wherever it is loaded.
 *
 * A resident program is one copy in memory that every Shell runs (amiga/buffee/resident.S). Each run therefore starts by zeroing the bss, the program's only writable
 * data (amiga.ld refuses any with an initial value), and runs alone: a lock byte outside the bss, taken under Forbid, makes a run that finds it taken wait (Delay, a
 * fifth of a second at a time) until the other run has finished.
 *
 * Copyright (c) 2026 the Buffee Project - http://www.buffee.ca
 * Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (see LICENSE).
 */
	.section .text.start,"ax"
	.globl	_start
_start:
	movem.l	%d2-%d7/%a2-%a6,-(%sp)
	move.l	%d0,%d2				| the argument line
	move.l	%a0,%a2
	move.l	4.w,%a6
	lea	dosname(%pc),%a1
	moveq	#0,%d0
	jsr	-552(%a6)			| OpenLibrary("dos.library", 0): for Delay
	move.l	%d0,%a5
	tst.l	%d0
	beq.s	9f
1:	jsr	-132(%a6)			| Forbid
	lea	lock(%pc),%a1
	tst.b	(%a1)
	beq.s	2f
	jsr	-138(%a6)			| Permit: another Shell is running this copy
	move.l	%a5,%a6
	moveq	#10,%d1
	jsr	-198(%a6)			| Delay(10)
	move.l	4.w,%a6
	bra.s	1b
2:	st	(%a1)
	jsr	-138(%a6)			| Permit
	lea	__bss_start(%pc),%a0
	lea	__bss_end(%pc),%a1
3:	cmp.l	%a1,%a0
	bhs.s	4f
	clr.l	(%a0)+
	bra.s	3b
4:	lea	SysBase(%pc),%a1
	move.l	%a6,(%a1)
	move.l	%a2,-(%sp)
	move.l	%d2,-(%sp)
	bsr.w	amiga_main
	addq.l	#8,%sp
	move.l	%d0,%d2
	lea	lock(%pc),%a1
	sf	(%a1)
	move.l	4.w,%a6
	move.l	%a5,%a1
	jsr	-414(%a6)			| CloseLibrary
	move.l	%d2,%d0
	movem.l	(%sp)+,%d2-%d7/%a2-%a6
	rts
9:	moveq	#20,%d0				| (no dos.library)
	movem.l	(%sp)+,%d2-%d7/%a2-%a6
	rts

dosname:
	.asciz	"dos.library"

	.section .lock,"aw"
lock:	.byte	0				| a run is in progress (zero in the file and in the resident tag's copy)

	.bss
	.globl	SysBase
	.even
SysBase:
	.long	0
