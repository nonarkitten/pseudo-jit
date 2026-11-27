/* resident.S -- the `buffee` command made resident from the board's own memory: a RomTag that PJIT puts at $F01000, in the half of the MapROM megabyte that is always
 * the board's DRAM (platform/buffee/buffee.c, buffee_cmd.h). Kickstart 3.0 and later (exec 39) find it in their resident scan and, once dos.library is up
 * (RTF_AFTERDOS), call its init, which copies the program into RAM as a segment of its own and adds it to the Shell's resident list (AddSegment): `buffee` then runs
 * from any Shell with no disk. The program is PC-relative, so the copy runs where it lands. RTF_AFTERDOS is exec 39's: Kickstart 2.0x lists the tag but never runs
 * it, and 1.3 has no resident list for commands; there the command comes from the floppy.
 * Linked at $F01000 (mk/amiga.mk); the program is the code hunk of build/amiga/buffee.
 *
 * Copyright (c) 2026 the Buffee Project - http://www.buffee.ca
 * Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (see LICENSE).
 */
	.text
	.globl	tag
tag:
	.word	0x4AFC				| rt_MatchWord
	.long	tag				| rt_MatchTag
	.long	tag_end				| rt_EndSkip
	.byte	0x04				| rt_Flags: RTF_AFTERDOS
	.byte	1				| rt_Version
	.byte	0				| rt_Type
	.byte	-100				| rt_Pri
	.long	tagname				| rt_Name
	.long	idstring			| rt_IdString
	.long	init				| rt_Init

/* init(d0 = 0, a0 = 0, a6 = SysBase): copies the program into a segment, adds it as `buffee`, and tells the board what happened; the result is not looked at */
init:
	movem.l	%d2-%d4/%a2-%a3/%a6,-(%sp)
	move.l	#0x80000000,%d4			| the entry's use count as the Shell finds it (not found yet)
	lea	dosname(%pc),%a1
	moveq	#36,%d0
	jsr	-552(%a6)			| OpenLibrary("dos.library", 36)
	move.l	%d0,%a2
	moveq	#1,%d2				| BFC_RES_NO_DOS
	tst.l	%d0
	beq.s	9f
	move.l	#prog_end-prog+8,%d0
	moveq	#1,%d1				| MEMF_PUBLIC
	jsr	-198(%a6)			| AllocMem
	move.l	%d0,%a3
	moveq	#2,%d2				| BFC_RES_NO_MEMORY
	tst.l	%d0
	beq.s	8f
	move.l	%a3,%a0
	move.l	#prog_end-prog+8,(%a0)+		| a segment as LoadSeg makes one: its allocation size, the next segment (none), the code
	clr.l	(%a0)+
	lea	prog(%pc),%a1
	move.w	#(prog_end-prog)/4-1,%d1
1:	move.l	(%a1)+,(%a0)+
	dbf	%d1,1b
	lea	cmdname(%pc),%a0
	move.l	%a0,%d1				| the name
	move.l	%a3,%d2
	addq.l	#4,%d2
	lsr.l	#2,%d2				| the segment list: a BPTR to the next-segment long
	moveq	#1,%d3				| the use count of an unused entry, as the Resident command makes it: Kickstart 3.1's FindSegment(name, 0, FALSE),
	move.l	%a2,%a6				| which the Shell looks commands up with, passes over an entry whose count is 0 (its Resident lists the count less one)
	jsr	-774(%a6)			| AddSegment
	tst.l	%d0
	beq.s	7f
	lea	cmdname(%pc),%a0		| the entry as the Shell finds it, for the report
	move.l	%a0,%d1
	moveq	#0,%d2
	moveq	#0,%d3
	jsr	-780(%a6)			| FindSegment
	tst.l	%d0
	beq.s	6f
	move.l	%d0,%a0
	move.l	4(%a0),%d4			| seg_UC
6:	move.l	4.w,%a6
	moveq	#4,%d2				| BFC_RES_ADDED
	bra.s	8f
7:	move.l	4.w,%a6
	moveq	#3,%d2				| BFC_RES_NOT_ADDED
	move.l	%a3,%a1				| (not added: the memory goes back)
	move.l	#prog_end-prog+8,%d0
	jsr	-210(%a6)			| FreeMem
8:	move.l	%a2,%a1
	jsr	-414(%a6)			| CloseLibrary
9:	lea	-64(%sp),%sp			| what happened, to the board (platform/buffee/control.h: BFC_RESIDENT, d2 = BFC_RES_*), for the command's status:
	move.l	%sp,%a0				| a request block on the stack, its address written to the control page; the write faults and PJIT serves it
	moveq	#15,%d1
1:	clr.l	(%a0)+
	dbf	%d1,1b
	move.l	#0x42465251,(%sp)		| BFC_REQ_MAGIC
	moveq	#11,%d1
	move.l	%d1,4(%sp)			| BFC_RESIDENT
	move.l	%d2,8(%sp)			| arg[0]
	move.l	%d4,12(%sp)			| arg[1]
	move.l	%sp,0xF7F000			| BFC_PAGE
	lea	64(%sp),%sp
	movem.l	(%sp)+,%d2-%d4/%a2-%a3/%a6
	moveq	#0,%d0
	rts

tagname:	.asciz	"buffee.cmd"
idstring:	.asciz	"buffee.cmd 1.0 (the buffee command, resident)"
dosname:	.asciz	"dos.library"
cmdname:	.asciz	"buffee"
	.balign	4
prog:
	.incbin	"buffee.bin"
	.balign	4
prog_end:
tag_end:
