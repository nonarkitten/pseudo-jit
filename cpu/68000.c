
/*
 * Castaway
 *  (C) 1994 - 2002 Martin Doering, Joachim Hoenig
 *
 * 68000.c - 68000 emulator jump table and misc subroutines
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 *
 * revision history
 *  23.05.2002  JH  FAST1.0.1 code import: KR -> ANSI, restructuring
 *  09.06.2002  JH  Use of mmap for memory access discontinued
 *  12.06.2002  JH  Correct bus error/address error exception stack frame
 *  13.06.2002  JH  Merged in Martin's BCD implementation (untested),
 *                  completed jump table. STOP is now the only
 *                  unimplemented instruction.
 *  14.06.2002  JH  Implemented STOP, shutdown CPU after multiple bus errors.
 *                  Removed inst parameter from CPU opcode functions.
 *  19.06.2002  JH  CPURun() returns # of executed instructions.
 *  20.06.2002  JH  added yet another SR implementation variant.
 *  02.07.2002  JH  Support different CPU types. Removed MOVE CCR,<EA> from
 *                  68000 jump table.
 *  20.08.2002  JH  Fixed CPU shutdown.
 *  27.08.2002  JH  Bugfix: S and T flag incorrectly reported for group 0 exceptions
 *                  Implemented additional 68010 registers and instructions.
 *  31.08.2002  JH  Implemented M68010 exception stack frames.
 *  08.10.2002  JH  Implemented Trace exception
 *  25.10.2002  JH  CHKTRACE no longer optional, improved implementation.
 */

static char sccsid[] = "$Id: 68000.c,v 1.20 2002/10/30 16:23:04 jhoenig Exp $";

#include "config.h"

#include <stdio.h>
#include <assert.h>
#include <setjmp.h>

#include "68000.h"
#include <pjit_ea_modes.h>
#include "proto.h"

#define INCLUDE_C
#if (CPU_TYPE == 68000 || CPU_TYPE == 68008)
#include "op68000.c"
#elif (CPU_TYPE == 68010) // broken
#include "op68010.c"
#endif
#undef INCLUDE_C

#include <pjit_cache.h>


int intmask, intpri;
char cpu_state;
unsigned cpu_type;
jmp_buf trap_buf;



void ExceptionGroup0(int number, unsigned long address, int ReadWrite) {
	short sr = GetSRW(), context = 0;
#ifdef DEBUG
    ON_TRAP(number);
    assert(cpu_state != -3);
#endif
	if (cpu_state == 0) {
		cpu_state = -3;
		longjmp(trap_buf, 1);
	}
#if CPU_TYPE == 68000
	else if (cpu_state > 0) {
		context |= 0x8;
	}
	if (ReadWrite)
		context |= 0x10;
#else
    if (ReadWrite) context |= 0x100;
#endif
	if (GetS())
		context |= 0x4;
	if (ReadWrite && address == pc)
		context |= 0x2;
	else
		context |= 0x1;
	cpu_state = 0; /* begin group 0 exception processing */
	SetS(1);
	SetT(0);
#if CPU_TYPE == 68010
    A7 -= 44; /* Rerun info */
    A7 -= 4; SetMemL(A7, address); /* fault address */
    A7 -= 2; SetMemW(A7, context);
    A7 -= 2; SetMemW(A7, 0x8000 | (number * 4));
    A7 -= 4; SetMemL(A7, GetPC());
    A7 -= 2; SetMemW(A7, sr);
#else
	A7 = A7 - 14;
	SetMemW(A7, context);
	SetMemL(A7 + 2, address);
	SetMemW(A7 + 6, inst);
	SetMemW(A7 + 8, sr);
	SetMemL(A7 + 10, GetPC());
#endif

	SetPC(GetMemL((long ) number * 4) + vbr);
	/* end exception processing */
	cpu_state = -1;
}

void ExceptionGroup1(int number) {
	short sr = GetSRW();
#ifdef DEBUG
    ON_TRAP(number);
#endif
	cpu_state = 1; /* begin group 1 exception processing */
	switch (number) {
	case LINE_A:
	case LINE_F:
	case PRIV:
	case ILLINSTR:
		SetPC(GetPC() - 2);
		break;
	}
	SetS(1);
	SetT(0);
#if CPU_TYPE == 68010
    A7 -= 2; SetMemW(A7, number * 4);
#endif
	A7 -= 4;
	SetMemL(A7, GetPC());
	A7 -= 2;
	SetMemW(A7, sr);
	SetPC(GetMemL((long ) number * 4) + vbr);
	/* end exception processing */
	cpu_state = -1;
}

void Interrupt() {
	short sr = GetSRW();
	int number = QueryIRQ(intpri);
#ifdef DEBUG
    ON_TRAP(number);
#endif
	cpu_state = 1; /* begin group 1 exception processing */
	SetI(intpri);
	SetS(1);
	SetT(0);
#if CPU_TYPE == 68010
    A7 -= 2; SetMemW(A7, number * 4);
#endif
	A7 -= 4;
	SetMemL(A7, GetPC());
	A7 -= 2;
	SetMemW(A7, sr);
	SetPC(GetMemL((long ) number * 4) + vbr);
	/* end exception processing */
	cpu_state = -1;
}

void ExceptionGroup2(int number) {
	short sr = GetSRW();
#ifdef DEBUG
    ON_TRAP(number);
#endif
	cpu_state = 2; /* begin group 2 exception processing */
	SetS(1);
	SetT(0);
#if CPU_TYPE == 68010
    A7 -= 2; SetMemW(A7, number * 4);
#endif
	A7 -= 4;
	SetMemL(A7, GetPC());
	A7 -= 2;
	SetMemW(A7, sr);
	SetPC(GetMemL((long ) number * 4) + vbr);
	/* end exception processing */
	cpu_state = -1;
}

void Trace() {
	int number = TRACE;
	short sr = GetSRW();
#ifdef DEBUG
    ON_TRAP(number);
#endif
	cpu_state = 1; /* begin group 1 exception processing */
	SetS(1);
	SetT(0);
#if CPU_TYPE == 68010
    A7 -= 2; SetMemW(A7, number * 4);
#endif
	A7 -= 4;
	SetMemL(A7, GetPC());
	A7 -= 2;
	SetMemW(A7, sr);
	SetPC(GetMemL((long ) number * 4) + vbr);
	/* end exception processing */
	cpu_state = -1;
}

void IllIns(void) {
	ExceptionGroup1(ILLINSTR);
}

void Line_A(void) {
	ExceptionGroup1(LINE_A);
}

void Line_F(void) {
	ExceptionGroup1(LINE_F);
}

void Stop(void) {
	if (!GetS()) ExceptionGroup1(PRIV);
	SetSRW(GetMPCW());
	pc += 2;
	cpu_state = -2; /* stopped */
	longjmp(trap_buf, 1);
}

void pjit_trace(void) {
	longjmp(trap_buf, 1);
}

void pjit_reset(void) {
	static cpu_t cpu_static;
	cpu = &cpu_static;

	cache_init();

	A7 = GetMemL(0l);
	SetPC(GetMemL(4l));
	SetI(7);
	SetT(0);
	SetS(1);
	cpu_state = -1; /* running */
	cpu_type = CPU_TYPE;
}

//void CPUEvent() {
//	// force the CPURun loop to end
//	//cpu_count = 1;
//}

__attribute__((noreturn)) void pjit_run(void) {
	if (setjmp(trap_buf)) ; // debug?
	if (GetT()) {
		pc = (uint16_t*)cache_reverse(lr);
		do {
			inst = GetInst(pc);
			pc += len_table[inst >> 3] << 1;
			*(jmp_table[inst >> 3])();

			if (intpri > GetI()) Interrupt();
	//#ifdef TRACEBACK
	//		SaveState(inst);
	//#endif
			Trace();
		} while(GetT());
	}
	goto *cache_find_entry(pc);
}

