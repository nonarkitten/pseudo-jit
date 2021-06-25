/*
 * proto.h - FAST 68000 emulator instruction prototypes
 * Version 1.0
 * Copyright (C) 1994, 1995 Joachim Hönig
 * (hoenig@informatik.uni-erlangen.de)
 *
 * This file is part of FAST, the Fine Atari ST Emulator.
 *
 * FAST is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * FAST is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FAST; see the file COPYING.  If not, write to
 * the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 */
/*
 * Prototypes of all 68000 opcodes defined in op68k*.c
 */

#undef Oper
#define Oper(a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11) \
    extern void a1(void);

#define PROTOH

#include "op68kadd.c"
#include "op68karith.c"
#include "op68ksub.c"
#include "op68klogop.c"
#include "op68kmisc.c"
#include "op68kmove.c"
#include "op68kshift.c"

extern void     Op4840(void);   /* SWAP */
extern void     Op4848(void);   /* BKPT */
extern void     Op4880(void);   /* EXT.W */
extern void     Op4890(void);   /* MOVEM.W to (ax) */
extern void     Op48a0(void);   /* MOVEM.W to -(ax) */
extern void     Op48a8(void);   /* MOVEM.W to d(ax) */
extern void     Op48b0(void);   /* MOVEM.W to d(ax,rx) */
extern void     Op48b8(void);   /* MOVEM.W to w l */
extern void     Op48c0(void);   /* EXT.L */
extern void     Op48d0(void);   /* MOVEM.L to (ax) */
extern void     Op48e0(void);   /* MOVEM.L to -(ax) */
extern void     Op48e8(void);   /* MOVEM.L to d(ax) */
extern void     Op48f0(void);   /* MOVEM.L to d(ax,rx) */
extern void     Op48f8(void);   /* MOVEM.L to w l */
extern void     Op4c90(void);   /* MOVEM.W from (ax) */
extern void     Op4c98(void);   /* MOVEM.W from (ax)+ */
extern void     Op4ca8(void);   /* MOVEM.W from d(ax) */
extern void     Op4cb0(void);   /* MOVEM.W from d(ax,rx) */
extern void     Op4cb8(void);   /* MOVEM.W from ea */
extern void     Op4cd0(void);   /* MOVEM.L from (ax) */
extern void     Op4cd8(void);   /* MOVEM.L from (ax)+ */
extern void     Op4ce8(void);   /* MOVEM.L from d(ax) */
extern void     Op4cf0(void);   /* MOVEM.L from d(ax,rx) */
extern void     Op4cf8(void);   /* MOVEM.L from ea */
extern void     Op4e40(void);   /* TRAP */
extern void     Op4e50(void);   /* LINK */
extern void     Op4e58(void);   /* UNLK */
extern void     Op4e60(void);   /* MOVE ax,usp */
extern void     Op4e68(void);   /* MOVE usp,ax */
extern void     Op4e70(void);   /* RESET, NOP, STOP, RTE,
                                 * RTD, RTS, TRAPV, RTR */
extern void     Op4e78(void);   /* MOVEC */

extern void     Opc140(void);   /* EXG */
extern void     Opc148(void);   /* EXG */
extern void     Opc188(void);   /* EXG */
extern void     Opc340(void);   /* EXG */
extern void     Opc348(void);   /* EXG */
extern void     Opc388(void);   /* EXG */
extern void     Opc540(void);   /* EXG */
extern void     Opc548(void);   /* EXG */
extern void     Opc588(void);   /* EXG */
extern void     Opc740(void);   /* EXG */
extern void     Opc748(void);   /* EXG */
extern void     Opc788(void);   /* EXG */
extern void     Opc940(void);   /* EXG */
extern void     Opc948(void);   /* EXG */
extern void     Opc988(void);   /* EXG */
extern void     Opcb40(void);   /* EXG */
extern void     Opcb48(void);   /* EXG */
extern void     Opcb88(void);   /* EXG */
extern void     Opcd40(void);   /* EXG */
extern void     Opcd48(void);   /* EXG */
extern void     Opcd88(void);   /* EXG */
extern void     Opcf40(void);   /* EXG */
extern void     Opcf48(void);   /* EXG */
extern void     Opcf88(void);   /* EXG */
