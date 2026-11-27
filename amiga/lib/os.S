/* os.S -- the library calls of amiga/include/amiga_os.h: each takes its C arguments from the stack, puts them in the library's registers, loads the library base into A6
 * and calls the function's offset. D2-D7 and A2-A6 are preserved (D2, D3 and A6 are saved here; the libraries keep the rest).
 *
 * Copyright (c) 2026 the Buffee Project - http://www.buffee.ca
 * Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (see LICENSE).
 */
	.text

/* exec.library: arguments in A1 / D0 / D1 as each function wants them */
	.globl	OpenLibrary, CloseLibrary, AllocMem, FreeMem
OpenLibrary:	| (name, version)
	move.l	%a6,-(%sp)
	move.l	SysBase(%pc),%a6
	move.l	8(%sp),%a1
	move.l	12(%sp),%d0
	jsr	-552(%a6)
	move.l	(%sp)+,%a6
	rts
CloseLibrary:	| (lib)
	move.l	%a6,-(%sp)
	move.l	SysBase(%pc),%a6
	move.l	8(%sp),%a1
	jsr	-414(%a6)
	move.l	(%sp)+,%a6
	rts
AllocMem:	| (size, flags)
	move.l	%a6,-(%sp)
	move.l	SysBase(%pc),%a6
	move.l	8(%sp),%d0
	move.l	12(%sp),%d1
	jsr	-198(%a6)
	move.l	(%sp)+,%a6
	rts
FreeMem:	| (mem, size)
	move.l	%a6,-(%sp)
	move.l	SysBase(%pc),%a6
	move.l	8(%sp),%a1
	move.l	12(%sp),%d0
	jsr	-210(%a6)
	move.l	(%sp)+,%a6
	rts

/* dos.library: arguments in D1, D2, D3 */
.macro	DOSCALL name, off
	.globl	\name
\name:
	movem.l	%d2-%d3/%a6,-(%sp)
	move.l	DOSBase(%pc),%a6
	movem.l	16(%sp),%d1-%d3
	jsr	\off(%a6)
	movem.l	(%sp)+,%d2-%d3/%a6
	rts
.endm
	DOSCALL	Open, -30
	DOSCALL	Close, -36
	DOSCALL	Read, -42
	DOSCALL	Write, -48
	DOSCALL	Input, -54
	DOSCALL	Output, -60
	DOSCALL	Seek, -66
	DOSCALL	IoErr, -132
	DOSCALL	Delay, -198
	DOSCALL	PrintFault, -474		| dos 36 and later from here on
	DOSCALL	ReadArgs, -798
	DOSCALL	FreeArgs, -858
	DOSCALL	FindSegment, -780
