/*
 * Castaway
 *  (C) 1994 - 2002 Martin Doering, Joachim Hoenig
 *
 * $File$ - 68k miscellaneous instructions
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 *
 * revision history
 *  23.05.2002  JH  FAST1.0.1 code import: KR -> ANSI, restructuring
 *  12.06.2002  JH  Correct bus error/address error exception stack frame
 *  14.06.2002  JH  Implemented STOP, shutdown CPU after multiple bus errors.
 *                  Removed inst parameter from CPU opcode functions.
 *  09.07.2002  JH  STOP should work now as expected.
 *  10.07.2002  JH  Fixed X, C and Z-Flag for NEGX, X and C for NEG.
 *  27.08.2002  JH  Some clarifications in op68k.h
 *                  Implemented additional 68010 registers and instructions.
 *  31.08.2002  JH  Implemented M68010 exception stack frames.
 *  10.09.2002  JH  Fixed MOVE to/from CCR: word size
 *  15.09.2002  JH  Minor SR/Bcc/Scc optimization.
 *  25.10.2002  JH  CHKTRACE no longer optional, improved implementation.
 *                  PC correct now if MOVE to SR raises privilege violation.
 *  30.10.2002  JH  Replaced "% 8" with "& 7". Helps lesser compilers to generate faster code.
 *  11.11.2002  JH  Fixed BTST Dx,Dy etc. for negative Dx values (thx to schtruck@users.sourceforge.net)
 */
#ifndef PROTOH
static char     sccsid[] = "$Id: op68kmisc.c,v 1.13 2002/11/13 01:04:30 jhoenig Exp $";
#include "68000.h"
#include "op68k.h"

/*
 * Opfuncs.
 */
#define OMoves(Code, Decl, CalcEA, GetReg, SetReg, GetMem, SetMem) \
void            Code(void)\
{\
    Decl(source)\
    unsigned long address;\
    unsigned short ext = GetMPCW();\
    unsigned short index = ext >> 12;\
    if (!GetS()) { ExceptionGroup1(PRIV); };\
    pc += 2;\
    CalcEA(address, (inst & 7))\
    if (ext & 0x800) { \
        GetReg(index, source)\
        SetMem(address, source)\
    } else {\
        GetMem(address, source)\
        SetReg(index, source)\
    }\
}

#define OMoveToSR(Code, Get1)\
void            Code(void)\
{\
    DW(source)\
    if (!GetS()) ExceptionGroup1(PRIV);\
    Get1(source, (inst & 7))\
    SetSRW(source);\
    TraceChanged();\
}

#define DoBtst8(target,source) SetZ (0 == (target & (1 << (source & 7))));
#define DoBtst32(target,source) SetZ (0 == (target & (1 << (source & 31))));
#define DoBchg8(target,source) DoBtst8(target,source); target ^= (1 << (source & 7));
#define DoBchg32(target,source) DoBtst32(target,source); target ^= (1 << (source & 31));
#define DoBclr8(target,source) DoBtst8(target,source); target &= ~(1 << (source & 7));
#define DoBclr32(target,source) DoBtst32(target,source); target &= ~(1 << (source & 31));
#define DoBset8(target,source) DoBtst8(target,source); target |= (1 << (source & 7));
#define DoBset32(target,source) DoBtst32(target,source); target |= (1 << (source & 31));

#define DoBra(target,source) if (!source) source = GetMPCW (); pc += source;
#define DoBras(target,source) pc += source;
#define DoBsr(target,source) areg[7] -= 4; if (!source) { source = GetMPCW (); SetMemL (areg[7], GetPC() + 2);} else {SetMemL (areg[7], GetPC());} pc += source;
#define DoBsrs(target,source) areg[7] -= 4; SetMemL (areg[7], GetPC()); pc += source;
#define DoBhi(target,source) if (!source) {source = GetMPCW (); if (CChi) {pc += source;} else { pc += 2;}} else {if (CChi) pc += source;};
#define DoBhis(target,source) if (CChi) pc += source;
#define DoBls(target,source) if (!source) {source = GetMPCW (); if (CCls) {pc += source;} else { pc += 2;}} else {if (CCls) pc += source;};
#define DoBlss(target,source) if (CCls) pc += source;
#define DoBcc(target,source) if (!source) {source = GetMPCW (); if (CCcc) {pc += source;} else { pc += 2;}} else {if (CCcc) pc += source;};
#define DoBccs(target,source) if (CCcc) pc += source;
#define DoBcs(target,source) if (!source) {source = GetMPCW (); if (CCcs) {pc += source;} else { pc += 2;}} else {if (CCcs) pc += source;};
#define DoBcss(target,source) if (CCcs) pc += source;
#define DoBne(target,source) if (!source) {source = GetMPCW (); if (CCne) {pc += source;} else { pc += 2;}} else {if (CCne) pc += source;};
#define DoBnes(target,source) if (CCne) pc += source;
#define DoBeq(target,source) if (!source) {source = GetMPCW (); if (CCeq) {pc += source;} else { pc += 2;}} else {if (CCeq) pc += source;};
#define DoBeqs(target,source) if (CCeq) pc += source;
#define DoBvc(target,source) if (!source) {source = GetMPCW (); if (CCvc) {pc += source;} else { pc += 2;}} else {if (CCvc) pc += source;};
#define DoBvcs(target,source) if (CCvc) pc += source;
#define DoBvs(target,source) if (!source) {source = GetMPCW (); if (CCvs) {pc += source;} else { pc += 2;}} else {if (CCvs) pc += source;};
#define DoBvss(target,source) if (CCvs) pc += source;
#define DoBpl(target,source) if (!source) {source = GetMPCW (); if (CCpl) {pc += source;} else { pc += 2;}} else {if (CCpl) pc += source;};
#define DoBpls(target,source) if (CCpl) pc += source;
#define DoBmi(target,source) if (!source) {source = GetMPCW (); if (CCmi) {pc += source;} else { pc += 2;}} else {if (CCmi) pc += source;};
#define DoBmis(target,source) if (CCmi) pc += source;
#define DoBge(target,source) if (!source) {source = GetMPCW (); if (CCge) {pc += source;} else { pc += 2;}} else {if (CCge) pc += source;};
#define DoBges(target,source) if (CCge) pc += source;
#define DoBlt(target,source) if (!source) {source = GetMPCW (); if (CClt) {pc += source;} else { pc += 2;}} else {if (CClt) pc += source;};
#define DoBlts(target,source) if (CClt) pc += source;
#define DoBgt(target,source) if (!source) {source = GetMPCW (); if (CCgt) {pc += source;} else { pc += 2;}} else {if (CCgt) pc += source;};
#define DoBgts(target,source) if (CCgt) pc += source;
#define DoBle(target,source) if (!source) {source = GetMPCW (); if (CCle) {pc += source;} else { pc += 2;}} else {if (CCle) pc += source;};
#define DoBles(target,source) if (CCle) pc += source;

#define DoChk(target,source) \
    SetN (source < 0); if (source < 0 || source > target) ExceptionGroup2(TRAPCHK);

#define DoClr(target,source) target = 0; ClrCVSetNZ(target);

#define DoDbt(target,source) if (!CCt) {if (--target != -1) pc += GetMPCW(); else pc += 2;} else pc += 2;
#define DoDbf(target,source) if (!CCf) {if (--target != -1) pc += GetMPCW(); else pc += 2;} else pc += 2;
#define DoDbhi(target,source) if (!CChi) {if (--target != -1) pc += GetMPCW(); else pc += 2;} else pc += 2;
#define DoDbls(target,source) if (!CCls) {if (--target != -1) pc += GetMPCW(); else pc += 2;} else pc += 2;
#define DoDbcc(target,source) if (!CCcc) {if (--target != -1) pc += GetMPCW(); else pc += 2;} else pc += 2;
#define DoDbcs(target,source) if (!CCcs) {if (--target != -1) pc += GetMPCW(); else pc += 2;} else pc += 2;
#define DoDbne(target,source) if (!CCne) {if (--target != -1) pc += GetMPCW(); else pc += 2;} else pc += 2;
#define DoDbeq(target,source) if (!CCeq) {if (--target != -1) pc += GetMPCW(); else pc += 2;} else pc += 2;
#define DoDbvc(target,source) if (!CCvc) {if (--target != -1) pc += GetMPCW(); else pc += 2;} else pc += 2;
#define DoDbvs(target,source) if (!CCvs) {if (--target != -1) pc += GetMPCW(); else pc += 2;} else pc += 2;
#define DoDbpl(target,source) if (!CCpl) {if (--target != -1) pc += GetMPCW(); else pc += 2;} else pc += 2;
#define DoDbmi(target,source) if (!CCmi) {if (--target != -1) pc += GetMPCW(); else pc += 2;} else pc += 2;
#define DoDbge(target,source) if (!CCge) {if (--target != -1) pc += GetMPCW(); else pc += 2;} else pc += 2;
#define DoDblt(target,source) if (!CClt) {if (--target != -1) pc += GetMPCW(); else pc += 2;} else pc += 2;
#define DoDbgt(target,source) if (!CCgt) {if (--target != -1) pc += GetMPCW(); else pc += 2;} else pc += 2;
#define DoDble(target,source) if (!CCle) {if (--target != -1) pc += GetMPCW(); else pc += 2;} else pc += 2;

#define DoJmp(target,source) SetPC (source);

#define DoJsr(target,source) areg[7] -= 4; SetMemL (areg[7], GetPC()); SetPC(source);

#define DoLea(target,source) target = source;

#define DoMovecc(target,source) target = source;

#define DoMovep(target,source) target = source;

#define DoMoveFromSR(target,source) \
    if (!GetS()) ExceptionGroup1(PRIV); \
    target = source;

#define DoNeg(target,source) \
    SetV (target < 0 && -target < 0);\
    target = -target;\
    SetXC (target != 0);\
    SetNZ(target);

/* simplified DoSubx */
#define DoNegx(target,source) \
    SetV (target < 0);\
    if (GetX()) {\
        SetXC(1);\
        target = -target;\
        target -= 1;\
    } else {\
        SetXC (target != 0);\
        target = -target;\
    }\
    SetV (GetV() && (target < 0));\
    SetZ(target == 0 ? GetZ() : 0);\
    SetN(target < 0);

#define DoNot(target,source) target = ~target; ClrCVSetNZ(target);

#define DoPea(target,source) areg[7] -= 4; SetMemL (areg[7],source);

#define DoSt(target,source) target = CCt ? 0xff : 0x00;
#define DoSf(target,source) target = CCf ? 0xff : 0x00;
#define DoShi(target,source) target = CChi ? 0xff : 0x00;
#define DoSls(target,source) target = CCls ? 0xff : 0x00;
#define DoScc(target,source) target = CCcc ? 0xff : 0x00;
#define DoScs(target,source) target = CCcs ? 0xff : 0x00;
#define DoSne(target,source) target = CCne ? 0xff : 0x00;
#define DoSeq(target,source) target = CCeq ? 0xff : 0x00;
#define DoSvc(target,source) target = CCvc ? 0xff : 0x00;
#define DoSvs(target,source) target = CCvs ? 0xff : 0x00;
#define DoSpl(target,source) target = CCpl ? 0xff : 0x00;
#define DoSmi(target,source) target = CCmi ? 0xff : 0x00;
#define DoSge(target,source) target = CCge ? 0xff : 0x00;
#define DoSlt(target,source) target = CClt ? 0xff : 0x00;
#define DoSgt(target,source) target = CCgt ? 0xff : 0x00;
#define DoSle(target,source) target = CCle ? 0xff : 0x00;

#define DoTst(target,source) ClrCVSetNZ(target);

#define DoTas(target,source) ClrCVSetNZ(target); target |= 0x80;

void            Opc140(void)    /* EXG */
{
    register long var;
    var = reg[0];
    reg[0] = reg[inst & 15];
    reg[inst & 15] = var;
}

void            Opc148(void)    /* EXG */
{
    register long var;
    var = reg[8];
    reg[8] = reg[inst & 15];
    reg[inst & 15] = var;
}

void            Opc188(void)    /* EXG */
{
    register long var;
    var = reg[0];
    reg[0] = reg[inst & 15];
    reg[inst & 15] = var;
}

void            Opc340(void)    /* EXG */
{
    register long var;
    var = reg[1];
    reg[1] = reg[inst & 15];
    reg[inst & 15] = var;
}

void            Opc348(void)    /* EXG */
{
    register long var;
    var = reg[9];
    reg[9] = reg[inst & 15];
    reg[inst & 15] = var;
}

void            Opc388(void)    /* EXG */
{
    register long var;
    var = reg[1];
    reg[1] = reg[inst & 15];
    reg[inst & 15] = var;
}

void            Opc540(void)    /* EXG */
{
    register long var;
    var = reg[2];
    reg[2] = reg[inst & 15];
    reg[inst & 15] = var;
}

void            Opc548(void)    /* EXG */
{
    register long var;
    var = reg[10];
    reg[10] = reg[inst & 15];
    reg[inst & 15] = var;
}

void            Opc588 (void)   /* EXG */
{
    register long var;
    var = reg[2];
    reg[2] = reg[inst & 15];
    reg[inst & 15] = var;
}

void            Opc740(void)    /* EXG */
{
    register long var;
    var = reg[3];
    reg[3] = reg[inst & 15];
    reg[inst & 15] = var;
}

void            Opc748(void)    /* EXG */
{
    register long var;
    var = reg[11];
    reg[11] = reg[inst & 15];
    reg[inst & 15] = var;
}

void            Opc788(void)    /* EXG */
{
    register long var;
    var = reg[3];
    reg[3] = reg[inst & 15];
    reg[inst & 15] = var;
}

void            Opc940(void)    /* EXG */
{
    register long var;
    var = reg[4];
    reg[4] = reg[inst & 15];
    reg[inst & 15] = var;
}

void            Opc948(void)    /* EXG */
{
    register long var;
    var = reg[12];
    reg[12] = reg[inst & 15];
    reg[inst & 15] = var;
}

void            Opc988(void)    /* EXG */
{
    register long var;
    var = reg[4];
    reg[4] = reg[inst & 15];
    reg[inst & 15] = var;
}

void            Opcb40(void)    /* EXG */
{
    register long var;
    var = reg[5];
    reg[5] = reg[inst & 15];
    reg[inst & 15] = var;
}

void            Opcb48(void)    /* EXG */
{
    register long var;
    var = reg[13];
    reg[13] = reg[inst & 15];
    reg[inst & 15] = var;
}

void            Opcb88(void)    /* EXG */
{
    register long var;
    var = reg[5];
    reg[5] = reg[inst & 15];
    reg[inst & 15] = var;
}

void            Opcd40(void)    /* EXG */
{
    register long var;
    var = reg[6];
    reg[6] = reg[inst & 15];
    reg[inst & 15] = var;
}

void            Opcd48(void)    /* EXG */
{
    register long var;
    var = reg[14];
    reg[14] = reg[inst & 15];
    reg[inst & 15] = var;
}

void            Opcd88(void)    /* EXG */
{
    register long var;
    var = reg[6];
    reg[6] = reg[inst & 15];
    reg[inst & 15] = var;
}

void            Opcf40(void)    /* EXG */
{
    register long var;
    var = reg[7];
    reg[7] = reg[inst & 15];
    reg[inst & 15] = var;
}

void            Opcf48(void)    /* EXG */
{
    register long var;
    var = areg[7];
    areg[7] = reg[inst & 15];
    reg[inst & 15] = var;
}

void            Opcf88(void)    /* EXG */
{
    register long var;
    var = reg[7];
    reg[7] = reg[inst & 15];
    reg[inst & 15] = var;
}

void            Op4840(void)    /* SWAP */
{
    long            source, target;
    source = GetRegL(inst & 7);
    target = ((unsigned) source) >> 16;
    source <<= 16;
    target |= source;
    SetRegL(inst & 7, target);
    ClrCVSetNZ (target);
}

void            Op4848(void)    /* BKPT */
{
    ExceptionGroup1(ILLINSTR);
}

void            Op4880(void)    /* EXT.W */
{
    char            source;
    short           target;
    source = GetRegB(inst & 7);
    target = source;
    SetRegW(inst & 7, target);
    ClrCVSetNZ (target);
}

void            Op4890(void)    /* MOVEM.W to (ax) */
{
    short           source;
    int             index;
    long            address;
    source = GetMPCW();
    pc += 2;
    Cain (address, (inst & 7));
    index = 0;
    while (source) {
        if (source & 0x1) {
            SetMemW (address, GetRegW (index));
            address += 2;
        }
        index++;
        source = ((unsigned short) source) >> 1;
    }
}

/* TODO: if ax is written, use initial ax content (68000, 68010)
 *       if ax is written, use final ax content (other CPUs) */
void            Op48a0(void)    /* MOVEM.W to -(ax) */
{
    short           source;
    int             index;
    long            address;
    source = GetMPCW();
    pc += 2;
    address = reg[8 + (inst & 7)];
    index = 15;
    while (source) {
        if (source & 0x1) {
            address -= 2;
            SetMemW (address, GetRegW (index));
        }
        index--;
        source = ((unsigned short) source) >> 1;
    }
    SetRegL (8 + (inst & 7), address);
}

void            Op48a8(void)    /* MOVEM.W to d(ax) */
{
    short           source;
    int             index;
    long            address;
    source = GetMPCW ();
    pc += 2;
    Cdai (address, (inst & 7));
    index = 0;
    while (source) {
        if (source & 0x1) {
            SetMemW (address, GetRegW (index));
            address += 2;
        }
        index++;
        source = ((unsigned short) source) >> 1;
    }
}

void            Op48b0(void)    /* MOVEM.W to d(ax,rx) */
{
    short           source;
    int             index;
    long            address;
    source = GetMPCW ();
    pc += 2;
    Caix (address, (inst & 7));
    index = 0;
    while (source) {
        if (source & 0x1) {
            SetMemW (address, GetRegW (index));
            address += 2;
        }
        index++;
        source = ((unsigned short) source) >> 1;
    }
}

void            Op48b8(void)    /* MOVEM.W to w l */
{
    short           source;
    int             index;
    long            address;
    source = GetMPCW ();
    pc += 2;
    Ceaw (address, (inst & 7));
    index = 0;
    while (source) {
        if (source & 0x1) {
            SetMemW (address, GetRegW (index));
            address += 2;
        }
        index++;
        source = ((unsigned short) source) >> 1;
    }
}

void            Op48c0(void)    /* EXT.L */
{
    short           source;
    long            target;
    source = GetRegW (inst & 7);
    target = source;
    SetRegL ((inst & 7), target);
    ClrCVSetNZ (target);
}

void            Op48d0(void)    /* MOVEM.L to (ax) */
{
    short           source;
    int             index;
    long            address;
    source = GetMPCW();
    pc += 2;
    Cain (address, (inst & 7));
    index = 0;
    while (source) {
        if (source & 0x1) {
            SetMemL (address, GetRegL (index));
            address += 4;
        }
        index++;
        source = ((unsigned short) source) >> 1;
    }
}

/* TODO: if ax is written, use initial ax content (68000, 68010)
 *       if ax is written, use final ax content (other CPUs) */
void            Op48e0(void)    /* MOVEM.L to -(ax) */
{
    short           source;
    int             index;
    long            address;
    source = GetMPCW ();
    pc += 2;
    address = reg[8 + (inst & 7)];
    index = 15;
    while (source) {
        if (source & 0x1) {
            address -= 4;
            SetMemL (address, GetRegL (index));
        }
        index--;
        source = ((unsigned short) source) >> 1;
    }
    SetRegL (8 + (inst & 7), address);
}

void            Op48e8(void)    /* MOVEM.L to d(ax) */
{
    short           source;
    int             index;
    long            address;
    source = GetMPCW ();
    pc += 2;
    Cdai (address, (inst & 7));
    index = 0;
    while (source) {
        if (source & 0x1) {
            SetMemL (address, GetRegL (index));
            address += 4;
        }
        index++;
        source = ((unsigned short) source) >> 1;
    }
}

void            Op48f0(void)    /* MOVEM.L to d(ax,rx) */
{
    short           source;
    int             index;
    long            address;
    source = GetMPCW ();
    pc += 2;
    Caix (address, (inst & 7));
    index = 0;
    while (source) {
        if (source & 0x1) {
            SetMemL (address, GetRegL (index));
            address += 4;
        }
        index++;
        source = ((unsigned short) source) >> 1;
    }
}

void            Op48f8(void)    /* MOVEM.L to w l */
{
    short           source;
    int             index;
    long            address;
    source = GetMPCW ();
    pc += 2;
    Ceaw (address, (inst & 7));
    index = 0;
    while (source) {
        if (source & 0x1) {
            SetMemL (address, GetRegL (index));
            address += 4;
        }
        index++;
        source = ((unsigned short) source) >> 1;
    }
}

void            Op4c90(void)    /* MOVEM.W from (ax) */
{
    short           source;
    int             index;
    long            address;
    source = GetMPCW ();
    pc += 2;
    Cain (address, (inst & 7));
    index = 0;
    while (source) {
        if (source & 0x1) {
            SetRegL (index, (long) GetMemW (address));
            address += 2;
        }
        index++;
        source = ((unsigned short) source) >> 1;
    }
}

void            Op4c98(void)    /* MOVEM.W from (ax)+ */
{
    short           source;
    int             index;
    long            address;
    source = GetMPCW ();
    pc += 2;
    Cain (address, (inst & 7));   /* no increment here! */
    index = 0;
    while (source) {
        if (source & 0x1) {
            SetRegL (index, (long) GetMemW (address));
            address += 2;
        }
        index++;
        source = ((unsigned short) source) >> 1;
    }
    SetRegL (8 + (inst & 7), address);
}

void            Op4ca8(void)    /* MOVEM.W from d(ax) */
{
    short           source;
    int             index;
    long            address;
    source = GetMPCW ();
    pc += 2;
    Cdai (address, (inst & 7));
    index = 0;
    while (source) {
        if (source & 0x1) {
            SetRegL (index, (long) GetMemW (address));
            address += 2;
        }
        index++;
        source = ((unsigned short) source) >> 1;
    }
}

void            Op4cb0(void)    /* MOVEM.W from d(ax,rx) */
{
    short           source;
    int             index;
    long            address;
    source = GetMPCW ();
    pc += 2;
    Caix (address, (inst & 7));
    index = 0;
    while (source) {
        if (source & 0x1) {
            SetRegL (index, (long) GetMemW (address));
            address += 2;
        }
        index++;
        source = ((unsigned short) source) >> 1;
    }
}

void            Op4cb8(void)    /* MOVEM.W from ea */
{
    short           source;
    int             index;
    long            address;
    source = GetMPCW ();
    pc += 2;
    Cear (address, (inst & 7));
    index = 0;
    while (source) {
        if (source & 0x1) {
            SetRegL (index, (long) GetMemW (address));
            address += 2;
        }
        index++;
        source = ((unsigned short) source) >> 1;
    }
}

void            Op4cd0(void)    /* MOVEM.L from (ax) */
{
    short           source;
    int             index;
    long            address;
    source = GetMPCW ();
    pc += 2;
    Cain (address, (inst & 7));
    index = 0;
    while (source) {
        if (source & 0x1) {
            SetRegL (index, (long) GetMemL (address));
            address += 4;
        }
        index++;
        source = ((unsigned short) source) >> 1;
    }
}

void            Op4cd8(void)    /* MOVEM.L from (ax)+ */
{
    short           source;
    int             index;
    long            address;
    source = GetMPCW ();
    pc += 2;
    Cain (address, (inst & 7));   /* no increment here! */
    index = 0;
    while (source) {
        if (source & 0x1) {
            SetRegL (index, (long) GetMemL (address));
            address += 4;
        }
        index++;
        source = ((unsigned short) source) >> 1;
    }
    SetRegL (8 + (inst & 7), address);
}

void            Op4ce8(void)    /* MOVEM.L from d(ax) */
{
    short           source;
    int             index;
    long            address;
    source = GetMPCW ();
    pc += 2;
    Cdai (address, (inst & 7));
    index = 0;
    while (source) {
        if (source & 0x1) {
            SetRegL (index, (long) GetMemL (address));
            address += 4;
        }
        index++;
        source = ((unsigned short) source) >> 1;
    }
}

void            Op4cf0(void)    /* MOVEM.L from d(ax,rx) */
{
    short           source;
    int             index;
    long            address;
    source = GetMPCW ();
    pc += 2;
    Caix (address, (inst & 7));
    index = 0;
    while (source) {
        if (source & 0x1) {
            SetRegL (index, (long) GetMemL (address));
            address += 4;
        }
        index++;
        source = ((unsigned short) source) >> 1;
    }
}

void            Op4cf8(void)    /* MOVEM.L from ea */
{
    short           source;
    int             index;
    long            address;
    source = GetMPCW ();
    pc += 2;
    Cear (address, (inst & 7));
    index = 0;
    while (source) {
        if (source & 0x1) {
            SetRegL (index, (long) GetMemL (address));
            address += 4;
        }
        index++;
        source = ((unsigned short) source) >> 1;
    }
}

void            Op4e40(void)    /* TRAP */
{
    ExceptionGroup2(TRAP0 + (inst & 15));
}

void            Op4e50(void)    /* LINK */
{
    long            source;
    source = GetRegL (8 + (inst & 7));
    areg[7] -= 4;
    SetMemL (areg[7], source);
    source = GetRegL (15);
    SetRegL (8 + (inst & 7), source);
    source += GetMPCW ();
    pc += 2;
    SetRegL (15, source);
}

void            Op4e58(void)    /* UNLK */
{
    long            source;
    source = GetRegL (8 + (inst & 7));
    SetRegL (15, source);
    source = GetMemL (areg[7]);
    areg[7] += 4;
    SetRegL (8 + (inst & 7), source);
}

void            Op4e60(void)    /* MOVE ax,usp */
{
    if (!GetS ())
    ExceptionGroup1(PRIV);
    usp = reg[8 + (inst & 7)];
}

void            Op4e68(void)    /* MOVE usp,ax */
{
    if (!GetS ())
    ExceptionGroup1(PRIV);
    reg[8 + (inst & 7)] = usp;
}

void            Op4e70(void)    /* RESET, NOP, STOP, RTE, RTD, RTS,
                                 * TRAPV, RTR */
{
    switch (inst & 0x7) {
    case 0:         /* RESET */
        if (!GetS ())
            ExceptionGroup1(PRIV);
        break;
    case 1:         /* NOP */
        break;
    case 2:         /* STOP */
        Stop();
        break;
    case 3:         /* RTE */
        if (!GetS ())
            ExceptionGroup1(PRIV);
        {
            unsigned short  sr;
#if CPU_TYPE == 68000
            sr = GetMemW (areg[7]); areg[7] += 2;
            SetPC (GetMemL (areg[7])); areg[7] += 4;
            SetSRW(sr);
#else
            switch (GetMemW(areg[7] + 6) >> 12) {
            case 0x0:
                break;
            case 0x8:
                /* FIXME: this shouldn't be a format error */
#ifdef DEBUG
                DBG_STOP();
#endif
            default:
                ExceptionGroup1(FORMATERR);
                break;
            }
            sr = GetMemW (areg[7]); areg[7] += 2;
            SetPC (GetMemL (areg[7])); areg[7] += 4;
            SetSRW(sr);
            areg[7] += 2;
#endif
            TraceChanged(); /* trace bit may have changed */
        }
        break;
    case 4:         /* RTD */
#if CPU_TYPE == 68000
        ExceptionGroup1(ILLINSTR);
#else
        {
            short displacement = GetMPCW();
            SetPC(GetMemL(areg[7]));
            areg[7] = areg[7] + 4 + displacement;
        }
#endif
        break;
    case 5:         /* RTS */
        SetPC (GetMemL (areg[7]));
        areg[7] += 4;
        break;
    case 6:         /* TRAPV */
        if (GetV ())
            ExceptionGroup1(TRAPV);
        break;
    case 7:         /* RTR */
        SetSRB ( GetMemW (areg[7]));
        areg[7] += 2;
        SetPC (GetMemL (areg[7]));
        areg[7] += 4;
        break;
    }
}

void            Op4e78(void)    /* MOVEC */
{
    switch (inst & 0x7) {
    case 2:
        if (!GetS ()) {
            ExceptionGroup1(PRIV);
        } else {
            short           source;
            int             index;
            source = GetMPCW ();
            pc += 2;
            index = source >> 12;
            switch (source & 0xfff) {
            case 0x000:
                reg[index] = sfc & 7;
                break;
            case 0x001:
                reg[index] = dfc & 7;
                break;
            case 0x800:
                reg[index] = usp;
                break;
            case 0x801:
                reg[index] = vbr;
                break;
#if CPU_TYPE == 68020
            case 0x002:
                reg[index] = cacr & 0x3;
                break;
            case 0x802:
                reg[index] = caar & 0xfc;
                break;
            case 0x803:
                reg[index] = msp;
                break;
            case 0x804:
                reg[index] = isp;
                break;
#endif
            default:
                /* as specified by Motorola */
                ExceptionGroup1(ILLINSTR);
                break;
            }
        }
        break;
    case 3:
        if (!GetS ()) {
            ExceptionGroup1(PRIV);
        } else {
            short           source;
            int             index;
            source = GetMPCW ();
            pc += 2;
            index = source >> 12;
            switch (source & 0xfff) {
            case 0x000:
                sfc = reg[index] & 7;
                break;
            case 0x001:
                dfc = reg[index] & 7;
                break;
            case 0x800:
                usp = reg[index];
                break;
            case 0x801:
                vbr = reg[index];
                break;
#if CPU_TYPE == 68020
            case 0x002:
                cacr = reg[index] & 0x3;
                break;
            case 0x802:
                caar = reg[index] & 0xfc;
                break;
            case 0x803:
                msp = reg[index];
                break;
            case 0x804:
                isp = reg[index];
                break;
#endif
            default:
                /* as specified by Motorola */
                ExceptionGroup1(ILLINSTR);
                break;
            }
        }
        break;
    default:
        ExceptionGroup1(ILLINSTR);
        break;
    }
}
#else

#define OMoves(a1,a2,a3,a4,a5,a6,a7) \
    extern void a1(void);
#define OMoveToSR(a1, a2)\
    extern void a1(void);
#endif

Oper (Op4000, DoNegx, DN, SNN, 0, DB, DR, Cd, (inst & 7), GRB, SRB)
Oper (Op4010, DoNegx, DN, SNN, 0, DB, DA, Cain, (inst & 7), GMB, SMB)
Oper (Op4018, DoNegx, DN, SNN, 0, DB, DA, CaipB, (inst & 7), GMB, SMB)
Oper (Op4020, DoNegx, DN, SNN, 0, DB, DA, CmaiB, (inst & 7), GMB, SMB)
Oper (Op4028, DoNegx, DN, SNN, 0, DB, DA, Cdai, (inst & 7), GMB, SMB)
Oper (Op4030, DoNegx, DN, SNN, 0, DB, DA, Caix, (inst & 7), GMB, SMB)
Oper (Op4038, DoNegx, DN, SNN, 0, DB, DA, Ceaw, (inst & 7), GMB, SMB)
Oper (Op4040, DoNegx, DN, SNN, 0, DW, DR, Cd, (inst & 7), GRW, SRW)
Oper (Op4050, DoNegx, DN, SNN, 0, DW, DA, Cain, (inst & 7), GMW, SMW)
Oper (Op4058, DoNegx, DN, SNN, 0, DW, DA, CaipW, (inst & 7), GMW, SMW)
Oper (Op4060, DoNegx, DN, SNN, 0, DW, DA, CmaiW, (inst & 7), GMW, SMW)
Oper (Op4068, DoNegx, DN, SNN, 0, DW, DA, Cdai, (inst & 7), GMW, SMW)
Oper (Op4070, DoNegx, DN, SNN, 0, DW, DA, Caix, (inst & 7), GMW, SMW)
Oper (Op4078, DoNegx, DN, SNN, 0, DW, DA, Ceaw, (inst & 7), GMW, SMW)
Oper (Op4080, DoNegx, DN, SNN, 0, DL, DR, Cd, (inst & 7), GRL, SRL)
Oper (Op4090, DoNegx, DN, SNN, 0, DL, DA, Cain, (inst & 7), GML, SML)
Oper (Op4098, DoNegx, DN, SNN, 0, DL, DA, CaipL, (inst & 7), GML, SML)
Oper (Op40a0, DoNegx, DN, SNN, 0, DL, DA, CmaiL, (inst & 7), GML, SML)
Oper (Op40a8, DoNegx, DN, SNN, 0, DL, DA, Cdai, (inst & 7), GML, SML)
Oper (Op40b0, DoNegx, DN, SNN, 0, DL, DA, Caix, (inst & 7), GML, SML)
Oper (Op40b8, DoNegx, DN, SNN, 0, DL, DA, Ceaw, (inst & 7), GML, SML)
#if CPU_TYPE == 68000 // 68000: MOVE from SR not privileged
Oper (Op40c0, DoMovecc, DW, ScW, 0, DW, DR, Cd, (inst & 7), GN, SRW)
Oper (Op40d0, DoMovecc, DW, ScW, 0, DW, DA, Cain, (inst & 7), GN, SMW)
Oper (Op40d8, DoMovecc, DW, ScW, 0, DW, DA, CaipW, (inst & 7), GN, SMW)
Oper (Op40e0, DoMovecc, DW, ScW, 0, DW, DA, CmaiW, (inst & 7), GN, SMW)
Oper (Op40e8, DoMovecc, DW, ScW, 0, DW, DA, Cdai, (inst & 7), GN, SMW)
Oper (Op40f0, DoMovecc, DW, ScW, 0, DW, DA, Caix, (inst & 7), GN, SMW)
Oper (Op40f8, DoMovecc, DW, ScW, 0, DW, DA, Ceaw, (inst & 7), GN, SMW)
#else
Oper (Op40c0, DoMoveFromSR, DW, ScW, 0, DW, DR, Cd, (inst & 7), GN, SRW)
Oper (Op40d0, DoMoveFromSR, DW, ScW, 0, DW, DA, Cain, (inst & 7), GN, SMW)
Oper (Op40d8, DoMoveFromSR, DW, ScW, 0, DW, DA, CaipW, (inst & 7), GN, SMW)
Oper (Op40e0, DoMoveFromSR, DW, ScW, 0, DW, DA, CmaiW, (inst & 7), GN, SMW)
Oper (Op40e8, DoMoveFromSR, DW, ScW, 0, DW, DA, Cdai, (inst & 7), GN, SMW)
Oper (Op40f0, DoMoveFromSR, DW, ScW, 0, DW, DA, Caix, (inst & 7), GN, SMW)
Oper (Op40f8, DoMoveFromSR, DW, ScW, 0, DW, DA, Ceaw, (inst & 7), GN, SMW)
#endif

Oper (Op4200, DoClr, DN, SNN, 0, DB, DR, Cd, (inst & 7), GRB, SRB)
Oper (Op4210, DoClr, DN, SNN, 0, DB, DA, Cain, (inst & 7), GMB, SMB)
Oper (Op4218, DoClr, DN, SNN, 0, DB, DA, CaipB, (inst & 7), GMB, SMB)
Oper (Op4220, DoClr, DN, SNN, 0, DB, DA, CmaiB, (inst & 7), GMB, SMB)
Oper (Op4228, DoClr, DN, SNN, 0, DB, DA, Cdai, (inst & 7), GMB, SMB)
Oper (Op4230, DoClr, DN, SNN, 0, DB, DA, Caix, (inst & 7), GMB, SMB)
Oper (Op4238, DoClr, DN, SNN, 0, DB, DA, Ceaw, (inst & 7), GMB, SMB)
Oper (Op4240, DoClr, DN, SNN, 0, DW, DR, Cd, (inst & 7), GRW, SRW)
Oper (Op4250, DoClr, DN, SNN, 0, DW, DA, Cain, (inst & 7), GMW, SMW)
Oper (Op4258, DoClr, DN, SNN, 0, DW, DA, CaipW, (inst & 7), GMW, SMW)
Oper (Op4260, DoClr, DN, SNN, 0, DW, DA, CmaiW, (inst & 7), GMW, SMW)
Oper (Op4268, DoClr, DN, SNN, 0, DW, DA, Cdai, (inst & 7), GMW, SMW)
Oper (Op4270, DoClr, DN, SNN, 0, DW, DA, Caix, (inst & 7), GMW, SMW)
Oper (Op4278, DoClr, DN, SNN, 0, DW, DA, Ceaw, (inst & 7), GMW, SMW)
Oper (Op4280, DoClr, DN, SNN, 0, DL, DR, Cd, (inst & 7), GRL, SRL)
Oper (Op4290, DoClr, DN, SNN, 0, DL, DA, Cain, (inst & 7), GML, SML)
Oper (Op4298, DoClr, DN, SNN, 0, DL, DA, CaipL, (inst & 7), GML, SML)
Oper (Op42a0, DoClr, DN, SNN, 0, DL, DA, CmaiL, (inst & 7), GML, SML)
Oper (Op42a8, DoClr, DN, SNN, 0, DL, DA, Cdai, (inst & 7), GML, SML)
Oper (Op42b0, DoClr, DN, SNN, 0, DL, DA, Caix, (inst & 7), GML, SML)
Oper (Op42b8, DoClr, DN, SNN, 0, DL, DA, Ceaw, (inst & 7), GML, SML)
Oper (Op42c0, DoMovecc, DW, ScB, 0, DW, DR, Cd, (inst & 7), GN, SRW)
Oper (Op42d0, DoMovecc, DW, ScB, 0, DW, DA, Cain, (inst & 7), GN, SMW)
Oper (Op42d8, DoMovecc, DW, ScB, 0, DW, DA, CaipW, (inst & 7), GN, SMW)
Oper (Op42e0, DoMovecc, DW, ScB, 0, DW, DA, CmaiW, (inst & 7), GN, SMW)
Oper (Op42e8, DoMovecc, DW, ScB, 0, DW, DA, Cdai, (inst & 7), GN, SMW)
Oper (Op42f0, DoMovecc, DW, ScB, 0, DW, DA, Caix, (inst & 7), GN, SMW)
Oper (Op42f8, DoMovecc, DW, ScB, 0, DW, DA, Ceaw, (inst & 7), GN, SMW)

Oper (Op4400, DoNeg, DN, SNN, 0, DB, DR, Cd, (inst & 7), GRB, SRB)
Oper (Op4410, DoNeg, DN, SNN, 0, DB, DA, Cain, (inst & 7), GMB, SMB)
Oper (Op4418, DoNeg, DN, SNN, 0, DB, DA, CaipB, (inst & 7), GMB, SMB)
Oper (Op4420, DoNeg, DN, SNN, 0, DB, DA, CmaiB, (inst & 7), GMB, SMB)
Oper (Op4428, DoNeg, DN, SNN, 0, DB, DA, Cdai, (inst & 7), GMB, SMB)
Oper (Op4430, DoNeg, DN, SNN, 0, DB, DA, Caix, (inst & 7), GMB, SMB)
Oper (Op4438, DoNeg, DN, SNN, 0, DB, DA, Ceaw, (inst & 7), GMB, SMB)
Oper (Op4440, DoNeg, DN, SNN, 0, DW, DR, Cd, (inst & 7), GRW, SRW)
Oper (Op4450, DoNeg, DN, SNN, 0, DW, DA, Cain, (inst & 7), GMW, SMW)
Oper (Op4458, DoNeg, DN, SNN, 0, DW, DA, CaipW, (inst & 7), GMW, SMW)
Oper (Op4460, DoNeg, DN, SNN, 0, DW, DA, CmaiW, (inst & 7), GMW, SMW)
Oper (Op4468, DoNeg, DN, SNN, 0, DW, DA, Cdai, (inst & 7), GMW, SMW)
Oper (Op4470, DoNeg, DN, SNN, 0, DW, DA, Caix, (inst & 7), GMW, SMW)
Oper (Op4478, DoNeg, DN, SNN, 0, DW, DA, Ceaw, (inst & 7), GMW, SMW)
Oper (Op4480, DoNeg, DN, SNN, 0, DL, DR, Cd, (inst & 7), GRL, SRL)
Oper (Op4490, DoNeg, DN, SNN, 0, DL, DA, Cain, (inst & 7), GML, SML)
Oper (Op4498, DoNeg, DN, SNN, 0, DL, DA, CaipL, (inst & 7), GML, SML)
Oper (Op44a0, DoNeg, DN, SNN, 0, DL, DA, CmaiL, (inst & 7), GML, SML)
Oper (Op44a8, DoNeg, DN, SNN, 0, DL, DA, Cdai, (inst & 7), GML, SML)
Oper (Op44b0, DoNeg, DN, SNN, 0, DL, DA, Caix, (inst & 7), GML, SML)
Oper (Op44b8, DoNeg, DN, SNN, 0, DL, DA, Ceaw, (inst & 7), GML, SML)
Oper (Op44c0, DoMovecc, DW, SdW, (inst & 7), DB, DN, CN, 0, GN, SCB)
Oper (Op44d0, DoMovecc, DW, SainW, (inst & 7), DB, DN, CN, 0, GN, SCB)
Oper (Op44d8, DoMovecc, DW, SaipW, (inst & 7), DB, DN, CN, 0, GN, SCB)
Oper (Op44e0, DoMovecc, DW, SmaiW, (inst & 7), DB, DN, CN, 0, GN, SCB)
Oper (Op44e8, DoMovecc, DW, SdaiW, (inst & 7), DB, DN, CN, 0, GN, SCB)
Oper (Op44f0, DoMovecc, DW, SaixW, (inst & 7), DB, DN, CN, 0, GN, SCB)
Oper (Op44f8, DoMovecc, DW, SearW, (inst & 7), DB, DN, CN, 0, GN, SCB)

Oper (Op4600, DoNot, DN, SNN, 0, DB, DR, Cd, (inst & 7), GRB, SRB)
Oper (Op4610, DoNot, DN, SNN, 0, DB, DA, Cain, (inst & 7), GMB, SMB)
Oper (Op4618, DoNot, DN, SNN, 0, DB, DA, CaipB, (inst & 7), GMB, SMB)
Oper (Op4620, DoNot, DN, SNN, 0, DB, DA, CmaiB, (inst & 7), GMB, SMB)
Oper (Op4628, DoNot, DN, SNN, 0, DB, DA, Cdai, (inst & 7), GMB, SMB)
Oper (Op4630, DoNot, DN, SNN, 0, DB, DA, Caix, (inst & 7), GMB, SMB)
Oper (Op4638, DoNot, DN, SNN, 0, DB, DA, Ceaw, (inst & 7), GMB, SMB)
Oper (Op4640, DoNot, DN, SNN, 0, DW, DR, Cd, (inst & 7), GRW, SRW)
Oper (Op4650, DoNot, DN, SNN, 0, DW, DA, Cain, (inst & 7), GMW, SMW)
Oper (Op4658, DoNot, DN, SNN, 0, DW, DA, CaipW, (inst & 7), GMW, SMW)
Oper (Op4660, DoNot, DN, SNN, 0, DW, DA, CmaiW, (inst & 7), GMW, SMW)
Oper (Op4668, DoNot, DN, SNN, 0, DW, DA, Cdai, (inst & 7), GMW, SMW)
Oper (Op4670, DoNot, DN, SNN, 0, DW, DA, Caix, (inst & 7), GMW, SMW)
Oper (Op4678, DoNot, DN, SNN, 0, DW, DA, Ceaw, (inst & 7), GMW, SMW)
Oper (Op4680, DoNot, DN, SNN, 0, DL, DR, Cd, (inst & 7), GRL, SRL)
Oper (Op4690, DoNot, DN, SNN, 0, DL, DA, Cain, (inst & 7), GML, SML)
Oper (Op4698, DoNot, DN, SNN, 0, DL, DA, CaipL, (inst & 7), GML, SML)
Oper (Op46a0, DoNot, DN, SNN, 0, DL, DA, CmaiL, (inst & 7), GML, SML)
Oper (Op46a8, DoNot, DN, SNN, 0, DL, DA, Cdai, (inst & 7), GML, SML)
Oper (Op46b0, DoNot, DN, SNN, 0, DL, DA, Caix, (inst & 7), GML, SML)
Oper (Op46b8, DoNot, DN, SNN, 0, DL, DA, Ceaw, (inst & 7), GML, SML)
OMoveToSR (Op46c0, SdW)
OMoveToSR (Op46d0, SainW)
OMoveToSR (Op46d8, SaipW)
OMoveToSR (Op46e0, SmaiW)
OMoveToSR (Op46e8, SdaiW)
OMoveToSR (Op46f0, SaixW)
OMoveToSR (Op46f8, SearW)

Oper (Op4850, DoPea, DL, SainA, (inst & 7), DN, DN, CN, 0, GN, SN)
Oper (Op4868, DoPea, DL, SdaiA, (inst & 7), DN, DN, CN, 0, GN, SN)
Oper (Op4870, DoPea, DL, SaixA, (inst & 7), DN, DN, CN, 0, GN, SN)
Oper (Op4878, DoPea, DL, SearA, (inst & 7), DN, DN, CN, 0, GN, SN)

Oper (Op4a00, DoTst, DN, SNN, 0, DB, DR, Cd, (inst & 7), GRB, SN)
Oper (Op4a10, DoTst, DN, SNN, 0, DB, DA, Cain, (inst & 7), GMB, SN)
Oper (Op4a18, DoTst, DN, SNN, 0, DB, DA, CaipB, (inst & 7), GMB, SN)
Oper (Op4a20, DoTst, DN, SNN, 0, DB, DA, CmaiB, (inst & 7), GMB, SN)
Oper (Op4a28, DoTst, DN, SNN, 0, DB, DA, Cdai, (inst & 7), GMB, SN)
Oper (Op4a30, DoTst, DN, SNN, 0, DB, DA, Caix, (inst & 7), GMB, SN)
Oper (Op4a38, DoTst, DN, SNN, 0, DB, DA, Ceaw, (inst & 7), GMB, SN)
Oper (Op4a40, DoTst, DN, SNN, 0, DW, DR, Cd, (inst & 7), GRW, SN)
Oper (Op4a50, DoTst, DN, SNN, 0, DW, DA, Cain, (inst & 7), GMW, SN)
Oper (Op4a58, DoTst, DN, SNN, 0, DW, DA, CaipW, (inst & 7), GMW, SN)
Oper (Op4a60, DoTst, DN, SNN, 0, DW, DA, CmaiW, (inst & 7), GMW, SN)
Oper (Op4a68, DoTst, DN, SNN, 0, DW, DA, Cdai, (inst & 7), GMW, SN)
Oper (Op4a70, DoTst, DN, SNN, 0, DW, DA, Caix, (inst & 7), GMW, SN)
Oper (Op4a78, DoTst, DN, SNN, 0, DW, DA, Ceaw, (inst & 7), GMW, SN)
Oper (Op4a80, DoTst, DN, SNN, 0, DL, DR, Cd, (inst & 7), GRL, SN)
Oper (Op4a90, DoTst, DN, SNN, 0, DL, DA, Cain, (inst & 7), GML, SN)
Oper (Op4a98, DoTst, DN, SNN, 0, DL, DA, CaipL, (inst & 7), GML, SN)
Oper (Op4aa0, DoTst, DN, SNN, 0, DL, DA, CmaiL, (inst & 7), GML, SN)
Oper (Op4aa8, DoTst, DN, SNN, 0, DL, DA, Cdai, (inst & 7), GML, SN)
Oper (Op4ab0, DoTst, DN, SNN, 0, DL, DA, Caix, (inst & 7), GML, SN)
Oper (Op4ab8, DoTst, DN, SNN, 0, DL, DA, Ceaw, (inst & 7), GML, SN)
Oper (Op4ac0, DoTas, DN, SNN, 0, DB, DR, Cd, (inst & 7), GRB, SRB)
Oper (Op4ad0, DoTas, DN, SNN, 0, DB, DA, Cain, (inst & 7), GMB, SMB)
Oper (Op4ad8, DoTas, DN, SNN, 0, DB, DA, CaipB, (inst & 7), GMB, SMB)
Oper (Op4ae0, DoTas, DN, SNN, 0, DB, DA, CmaiB, (inst & 7), GMB, SMB)
Oper (Op4ae8, DoTas, DN, SNN, 0, DB, DA, Cdai, (inst & 7), GMB, SMB)
Oper (Op4af0, DoTas, DN, SNN, 0, DB, DA, Caix, (inst & 7), GMB, SMB)
Oper (Op4af8, DoTas, DN, SNN, 0, DB, DA, Ceaw, (inst & 7), GMB, SMB)

Oper (Op4e90, DoJsr, DL, SainA, (inst & 7), DN, DN, CN, 0, GN, SN)
Oper (Op4ea8, DoJsr, DL, SdaiA, (inst & 7), DN, DN, CN, 0, GN, SN)
Oper (Op4eb0, DoJsr, DL, SaixA, (inst & 7), DN, DN, CN, 0, GN, SN)
Oper (Op4eb8, DoJsr, DL, SearA, (inst & 7), DN, DN, CN, 0, GN, SN)

Oper (Op4ed0, DoJmp, DL, SainA, (inst & 7), DN, DN, CN, 0, GN, SN)
Oper (Op4ee8, DoJmp, DL, SdaiA, (inst & 7), DN, DN, CN, 0, GN, SN)
Oper (Op4ef0, DoJmp, DL, SaixA, (inst & 7), DN, DN, CN, 0, GN, SN)
Oper (Op4ef8, DoJmp, DL, SearA, (inst & 7), DN, DN, CN, 0, GN, SN)

Oper (Op4180, DoChk, DW, SdW, 0, DW, DR, Cd, (inst & 7), GRW, SN)
Oper (Op4190, DoChk, DW, SdW, 0, DW, DA, Cain, (inst & 7), GMW, SN)
Oper (Op4198, DoChk, DW, SdW, 0, DW, DA, CaipB, (inst & 7), GMW, SN)
Oper (Op41a0, DoChk, DW, SdW, 0, DW, DA, CmaiB, (inst & 7), GMW, SN)
Oper (Op41a8, DoChk, DW, SdW, 0, DW, DA, Cdai, (inst & 7), GMW, SN)
Oper (Op41b0, DoChk, DW, SdW, 0, DW, DA, Caix, (inst & 7), GMW, SN)
Oper (Op41b8, DoChk, DW, SdW, 0, DW, DA, Cear, (inst & 7), GMW, SN)

Oper (Op4380, DoChk, DW, SdW, 1, DW, DR, Cd, (inst & 7), GRW, SN)
Oper (Op4390, DoChk, DW, SdW, 1, DW, DA, Cain, (inst & 7), GMW, SN)
Oper (Op4398, DoChk, DW, SdW, 1, DW, DA, CaipB, (inst & 7), GMW, SN)
Oper (Op43a0, DoChk, DW, SdW, 1, DW, DA, CmaiB, (inst & 7), GMW, SN)
Oper (Op43a8, DoChk, DW, SdW, 1, DW, DA, Cdai, (inst & 7), GMW, SN)
Oper (Op43b0, DoChk, DW, SdW, 1, DW, DA, Caix, (inst & 7), GMW, SN)
Oper (Op43b8, DoChk, DW, SdW, 1, DW, DA, Cear, (inst & 7), GMW, SN)

Oper (Op4580, DoChk, DW, SdW, 2, DW, DR, Cd, (inst & 7), GRW, SN)
Oper (Op4590, DoChk, DW, SdW, 2, DW, DA, Cain, (inst & 7), GMW, SN)
Oper (Op4598, DoChk, DW, SdW, 2, DW, DA, CaipB, (inst & 7), GMW, SN)
Oper (Op45a0, DoChk, DW, SdW, 2, DW, DA, CmaiB, (inst & 7), GMW, SN)
Oper (Op45a8, DoChk, DW, SdW, 2, DW, DA, Cdai, (inst & 7), GMW, SN)
Oper (Op45b0, DoChk, DW, SdW, 2, DW, DA, Caix, (inst & 7), GMW, SN)
Oper (Op45b8, DoChk, DW, SdW, 2, DW, DA, Cear, (inst & 7), GMW, SN)

Oper (Op4780, DoChk, DW, SdW, 3, DW, DR, Cd, (inst & 7), GRW, SN)
Oper (Op4790, DoChk, DW, SdW, 3, DW, DA, Cain, (inst & 7), GMW, SN)
Oper (Op4798, DoChk, DW, SdW, 3, DW, DA, CaipB, (inst & 7), GMW, SN)
Oper (Op47a0, DoChk, DW, SdW, 3, DW, DA, CmaiB, (inst & 7), GMW, SN)
Oper (Op47a8, DoChk, DW, SdW, 3, DW, DA, Cdai, (inst & 7), GMW, SN)
Oper (Op47b0, DoChk, DW, SdW, 3, DW, DA, Caix, (inst & 7), GMW, SN)
Oper (Op47b8, DoChk, DW, SdW, 3, DW, DA, Cear, (inst & 7), GMW, SN)

Oper (Op4980, DoChk, DW, SdW, 4, DW, DR, Cd, (inst & 7), GRW, SN)
Oper (Op4990, DoChk, DW, SdW, 4, DW, DA, Cain, (inst & 7), GMW, SN)
Oper (Op4998, DoChk, DW, SdW, 4, DW, DA, CaipB, (inst & 7), GMW, SN)
Oper (Op49a0, DoChk, DW, SdW, 4, DW, DA, CmaiB, (inst & 7), GMW, SN)
Oper (Op49a8, DoChk, DW, SdW, 4, DW, DA, Cdai, (inst & 7), GMW, SN)
Oper (Op49b0, DoChk, DW, SdW, 4, DW, DA, Caix, (inst & 7), GMW, SN)
Oper (Op49b8, DoChk, DW, SdW, 4, DW, DA, Cear, (inst & 7), GMW, SN)

Oper (Op4b80, DoChk, DW, SdW, 5, DW, DR, Cd, (inst & 7), GRW, SN)
Oper (Op4b90, DoChk, DW, SdW, 5, DW, DA, Cain, (inst & 7), GMW, SN)
Oper (Op4b98, DoChk, DW, SdW, 5, DW, DA, CaipB, (inst & 7), GMW, SN)
Oper (Op4ba0, DoChk, DW, SdW, 5, DW, DA, CmaiB, (inst & 7), GMW, SN)
Oper (Op4ba8, DoChk, DW, SdW, 5, DW, DA, Cdai, (inst & 7), GMW, SN)
Oper (Op4bb0, DoChk, DW, SdW, 5, DW, DA, Caix, (inst & 7), GMW, SN)
Oper (Op4bb8, DoChk, DW, SdW, 5, DW, DA, Cear, (inst & 7), GMW, SN)

Oper (Op4d80, DoChk, DW, SdW, 6, DW, DR, Cd, (inst & 7), GRW, SN)
Oper (Op4d90, DoChk, DW, SdW, 6, DW, DA, Cain, (inst & 7), GMW, SN)
Oper (Op4d98, DoChk, DW, SdW, 6, DW, DA, CaipB, (inst & 7), GMW, SN)
Oper (Op4da0, DoChk, DW, SdW, 6, DW, DA, CmaiB, (inst & 7), GMW, SN)
Oper (Op4da8, DoChk, DW, SdW, 6, DW, DA, Cdai, (inst & 7), GMW, SN)
Oper (Op4db0, DoChk, DW, SdW, 6, DW, DA, Caix, (inst & 7), GMW, SN)
Oper (Op4db8, DoChk, DW, SdW, 6, DW, DA, Cear, (inst & 7), GMW, SN)

Oper (Op4f80, DoChk, DW, SdW, 7, DW, DR, Cd, (inst & 7), GRW, SN)
Oper (Op4f90, DoChk, DW, SdW, 7, DW, DA, Cain, (inst & 7), GMW, SN)
Oper (Op4f98, DoChk, DW, SdW, 7, DW, DA, CaipB, (inst & 7), GMW, SN)
Oper (Op4fa0, DoChk, DW, SdW, 7, DW, DA, CmaiB, (inst & 7), GMW, SN)
Oper (Op4fa8, DoChk, DW, SdW, 7, DW, DA, Cdai, (inst & 7), GMW, SN)
Oper (Op4fb0, DoChk, DW, SdW, 7, DW, DA, Caix, (inst & 7), GMW, SN)
Oper (Op4fb8, DoChk, DW, SdW, 7, DW, DA, Cear, (inst & 7), GMW, SN)

Oper (Op41d0, DoLea, DL, SainA, (inst & 7), DL, DR, Ca, 0, GN, SRL)
Oper (Op41e8, DoLea, DL, SdaiA, (inst & 7), DL, DR, Ca, 0, GN, SRL)
Oper (Op41f0, DoLea, DL, SaixA, (inst & 7), DL, DR, Ca, 0, GN, SRL)
Oper (Op41f8, DoLea, DL, SearA, (inst & 7), DL, DR, Ca, 0, GN, SRL)

Oper (Op43d0, DoLea, DL, SainA, (inst & 7), DL, DR, Ca, 1, GN, SRL)
Oper (Op43e8, DoLea, DL, SdaiA, (inst & 7), DL, DR, Ca, 1, GN, SRL)
Oper (Op43f0, DoLea, DL, SaixA, (inst & 7), DL, DR, Ca, 1, GN, SRL)
Oper (Op43f8, DoLea, DL, SearA, (inst & 7), DL, DR, Ca, 1, GN, SRL)

Oper (Op45d0, DoLea, DL, SainA, (inst & 7), DL, DR, Ca, 2, GN, SRL)
Oper (Op45e8, DoLea, DL, SdaiA, (inst & 7), DL, DR, Ca, 2, GN, SRL)
Oper (Op45f0, DoLea, DL, SaixA, (inst & 7), DL, DR, Ca, 2, GN, SRL)
Oper (Op45f8, DoLea, DL, SearA, (inst & 7), DL, DR, Ca, 2, GN, SRL)

Oper (Op47d0, DoLea, DL, SainA, (inst & 7), DL, DR, Ca, 3, GN, SRL)
Oper (Op47e8, DoLea, DL, SdaiA, (inst & 7), DL, DR, Ca, 3, GN, SRL)
Oper (Op47f0, DoLea, DL, SaixA, (inst & 7), DL, DR, Ca, 3, GN, SRL)
Oper (Op47f8, DoLea, DL, SearA, (inst & 7), DL, DR, Ca, 3, GN, SRL)

Oper (Op49d0, DoLea, DL, SainA, (inst & 7), DL, DR, Ca, 4, GN, SRL)
Oper (Op49e8, DoLea, DL, SdaiA, (inst & 7), DL, DR, Ca, 4, GN, SRL)
Oper (Op49f0, DoLea, DL, SaixA, (inst & 7), DL, DR, Ca, 4, GN, SRL)
Oper (Op49f8, DoLea, DL, SearA, (inst & 7), DL, DR, Ca, 4, GN, SRL)

Oper (Op4bd0, DoLea, DL, SainA, (inst & 7), DL, DR, Ca, 5, GN, SRL)
Oper (Op4be8, DoLea, DL, SdaiA, (inst & 7), DL, DR, Ca, 5, GN, SRL)
Oper (Op4bf0, DoLea, DL, SaixA, (inst & 7), DL, DR, Ca, 5, GN, SRL)
Oper (Op4bf8, DoLea, DL, SearA, (inst & 7), DL, DR, Ca, 5, GN, SRL)

Oper (Op4dd0, DoLea, DL, SainA, (inst & 7), DL, DR, Ca, 6, GN, SRL)
Oper (Op4de8, DoLea, DL, SdaiA, (inst & 7), DL, DR, Ca, 6, GN, SRL)
Oper (Op4df0, DoLea, DL, SaixA, (inst & 7), DL, DR, Ca, 6, GN, SRL)
Oper (Op4df8, DoLea, DL, SearA, (inst & 7), DL, DR, Ca, 6, GN, SRL)

Oper (Op4fd0, DoLea, DL, SainA, (inst & 7), DL, DR, Ca, 7, GN, SRL)
Oper (Op4fe8, DoLea, DL, SdaiA, (inst & 7), DL, DR, Ca, 7, GN, SRL)
Oper (Op4ff0, DoLea, DL, SaixA, (inst & 7), DL, DR, Ca, 7, GN, SRL)
Oper (Op4ff8, DoLea, DL, SearA, (inst & 7), DL, DR, Ca, 7, GN, SRL)

Oper (Op50c0, DoSt, DN, SNN, 0, DB, DR, Cd, (inst & 7), GN, SRB)
Oper (Op50c8, DoDbt, DN, SNN, 0, DW, DR, Cd, (inst & 7), GRW, SRW)
Oper (Op50d0, DoSt, DN, SNN, 0, DB, DA, Cain, (inst & 7), GN, SMB)
Oper (Op50d8, DoSt, DN, SNN, 0, DB, DA, CaipB, (inst & 7), GN, SMB)
Oper (Op50e0, DoSt, DN, SNN, 0, DB, DA, CmaiB, (inst & 7), GN, SMB)
Oper (Op50e8, DoSt, DN, SNN, 0, DB, DA, Cdai, (inst & 7), GN, SMB)
Oper (Op50f0, DoSt, DN, SNN, 0, DB, DA, Caix, (inst & 7), GN, SMB)
Oper (Op50f8, DoSt, DN, SNN, 0, DB, DA, Ceaw, (inst & 7), GN, SMB)

Oper (Op51c0, DoSf, DN, SNN, 0, DB, DR, Cd, (inst & 7), GN, SRB)
Oper (Op51c8, DoDbf, DN, SNN, 0, DW, DR, Cd, (inst & 7), GRW, SRW)
Oper (Op51d0, DoSf, DN, SNN, 0, DB, DA, Cain, (inst & 7), GN, SMB)
Oper (Op51d8, DoSf, DN, SNN, 0, DB, DA, CaipB, (inst & 7), GN, SMB)
Oper (Op51e0, DoSf, DN, SNN, 0, DB, DA, CmaiB, (inst & 7), GN, SMB)
Oper (Op51e8, DoSf, DN, SNN, 0, DB, DA, Cdai, (inst & 7), GN, SMB)
Oper (Op51f0, DoSf, DN, SNN, 0, DB, DA, Caix, (inst & 7), GN, SMB)
Oper (Op51f8, DoSf, DN, SNN, 0, DB, DA, Ceaw, (inst & 7), GN, SMB)

Oper (Op52c0, DoShi, DN, SNN, 0, DB, DR, Cd, (inst & 7), GN, SRB)
Oper (Op52c8, DoDbhi, DN, SNN, 0, DW, DR, Cd, (inst & 7), GRW, SRW)
Oper (Op52d0, DoShi, DN, SNN, 0, DB, DA, Cain, (inst & 7), GN, SMB)
Oper (Op52d8, DoShi, DN, SNN, 0, DB, DA, CaipB, (inst & 7), GN, SMB)
Oper (Op52e0, DoShi, DN, SNN, 0, DB, DA, CmaiB, (inst & 7), GN, SMB)
Oper (Op52e8, DoShi, DN, SNN, 0, DB, DA, Cdai, (inst & 7), GN, SMB)
Oper (Op52f0, DoShi, DN, SNN, 0, DB, DA, Caix, (inst & 7), GN, SMB)
Oper (Op52f8, DoShi, DN, SNN, 0, DB, DA, Ceaw, (inst & 7), GN, SMB)

Oper (Op53c0, DoSls, DN, SNN, 0, DB, DR, Cd, (inst & 7), GN, SRB)
Oper (Op53c8, DoDbls, DN, SNN, 0, DW, DR, Cd, (inst & 7), GRW, SRW)
Oper (Op53d0, DoSls, DN, SNN, 0, DB, DA, Cain, (inst & 7), GN, SMB)
Oper (Op53d8, DoSls, DN, SNN, 0, DB, DA, CaipB, (inst & 7), GN, SMB)
Oper (Op53e0, DoSls, DN, SNN, 0, DB, DA, CmaiB, (inst & 7), GN, SMB)
Oper (Op53e8, DoSls, DN, SNN, 0, DB, DA, Cdai, (inst & 7), GN, SMB)
Oper (Op53f0, DoSls, DN, SNN, 0, DB, DA, Caix, (inst & 7), GN, SMB)
Oper (Op53f8, DoSls, DN, SNN, 0, DB, DA, Ceaw, (inst & 7), GN, SMB)

Oper (Op54c0, DoScc, DN, SNN, 0, DB, DR, Cd, (inst & 7), GN, SRB)
Oper (Op54c8, DoDbcc, DN, SNN, 0, DW, DR, Cd, (inst & 7), GRW, SRW)
Oper (Op54d0, DoScc, DN, SNN, 0, DB, DA, Cain, (inst & 7), GN, SMB)
Oper (Op54d8, DoScc, DN, SNN, 0, DB, DA, CaipB, (inst & 7), GN, SMB)
Oper (Op54e0, DoScc, DN, SNN, 0, DB, DA, CmaiB, (inst & 7), GN, SMB)
Oper (Op54e8, DoScc, DN, SNN, 0, DB, DA, Cdai, (inst & 7), GN, SMB)
Oper (Op54f0, DoScc, DN, SNN, 0, DB, DA, Caix, (inst & 7), GN, SMB)
Oper (Op54f8, DoScc, DN, SNN, 0, DB, DA, Ceaw, (inst & 7), GN, SMB)

Oper (Op55c0, DoScs, DN, SNN, 0, DB, DR, Cd, (inst & 7), GN, SRB)
Oper (Op55c8, DoDbcs, DN, SNN, 0, DW, DR, Cd, (inst & 7), GRW, SRW)
Oper (Op55d0, DoScs, DN, SNN, 0, DB, DA, Cain, (inst & 7), GN, SMB)
Oper (Op55d8, DoScs, DN, SNN, 0, DB, DA, CaipB, (inst & 7), GN, SMB)
Oper (Op55e0, DoScs, DN, SNN, 0, DB, DA, CmaiB, (inst & 7), GN, SMB)
Oper (Op55e8, DoScs, DN, SNN, 0, DB, DA, Cdai, (inst & 7), GN, SMB)
Oper (Op55f0, DoScs, DN, SNN, 0, DB, DA, Caix, (inst & 7), GN, SMB)
Oper (Op55f8, DoScs, DN, SNN, 0, DB, DA, Ceaw, (inst & 7), GN, SMB)

Oper (Op56c0, DoSne, DN, SNN, 0, DB, DR, Cd, (inst & 7), GN, SRB)
Oper (Op56c8, DoDbne, DN, SNN, 0, DW, DR, Cd, (inst & 7), GRW, SRW)
Oper (Op56d0, DoSne, DN, SNN, 0, DB, DA, Cain, (inst & 7), GN, SMB)
Oper (Op56d8, DoSne, DN, SNN, 0, DB, DA, CaipB, (inst & 7), GN, SMB)
Oper (Op56e0, DoSne, DN, SNN, 0, DB, DA, CmaiB, (inst & 7), GN, SMB)
Oper (Op56e8, DoSne, DN, SNN, 0, DB, DA, Cdai, (inst & 7), GN, SMB)
Oper (Op56f0, DoSne, DN, SNN, 0, DB, DA, Caix, (inst & 7), GN, SMB)
Oper (Op56f8, DoSne, DN, SNN, 0, DB, DA, Ceaw, (inst & 7), GN, SMB)

Oper (Op57c0, DoSeq, DN, SNN, 0, DB, DR, Cd, (inst & 7), GN, SRB)
Oper (Op57c8, DoDbeq, DN, SNN, 0, DW, DR, Cd, (inst & 7), GRW, SRW)
Oper (Op57d0, DoSeq, DN, SNN, 0, DB, DA, Cain, (inst & 7), GN, SMB)
Oper (Op57d8, DoSeq, DN, SNN, 0, DB, DA, CaipB, (inst & 7), GN, SMB)
Oper (Op57e0, DoSeq, DN, SNN, 0, DB, DA, CmaiB, (inst & 7), GN, SMB)
Oper (Op57e8, DoSeq, DN, SNN, 0, DB, DA, Cdai, (inst & 7), GN, SMB)
Oper (Op57f0, DoSeq, DN, SNN, 0, DB, DA, Caix, (inst & 7), GN, SMB)
Oper (Op57f8, DoSeq, DN, SNN, 0, DB, DA, Ceaw, (inst & 7), GN, SMB)

Oper (Op58c0, DoSvc, DN, SNN, 0, DB, DR, Cd, (inst & 7), GN, SRB)
Oper (Op58c8, DoDbvc, DN, SNN, 0, DW, DR, Cd, (inst & 7), GRW, SRW)
Oper (Op58d0, DoSvc, DN, SNN, 0, DB, DA, Cain, (inst & 7), GN, SMB)
Oper (Op58d8, DoSvc, DN, SNN, 0, DB, DA, CaipB, (inst & 7), GN, SMB)
Oper (Op58e0, DoSvc, DN, SNN, 0, DB, DA, CmaiB, (inst & 7), GN, SMB)
Oper (Op58e8, DoSvc, DN, SNN, 0, DB, DA, Cdai, (inst & 7), GN, SMB)
Oper (Op58f0, DoSvc, DN, SNN, 0, DB, DA, Caix, (inst & 7), GN, SMB)
Oper (Op58f8, DoSvc, DN, SNN, 0, DB, DA, Ceaw, (inst & 7), GN, SMB)

Oper (Op59c0, DoSvs, DN, SNN, 0, DB, DR, Cd, (inst & 7), GN, SRB)
Oper (Op59c8, DoDbvs, DN, SNN, 0, DW, DR, Cd, (inst & 7), GRW, SRW)
Oper (Op59d0, DoSvs, DN, SNN, 0, DB, DA, Cain, (inst & 7), GN, SMB)
Oper (Op59d8, DoSvs, DN, SNN, 0, DB, DA, CaipB, (inst & 7), GN, SMB)
Oper (Op59e0, DoSvs, DN, SNN, 0, DB, DA, CmaiB, (inst & 7), GN, SMB)
Oper (Op59e8, DoSvs, DN, SNN, 0, DB, DA, Cdai, (inst & 7), GN, SMB)
Oper (Op59f0, DoSvs, DN, SNN, 0, DB, DA, Caix, (inst & 7), GN, SMB)
Oper (Op59f8, DoSvs, DN, SNN, 0, DB, DA, Ceaw, (inst & 7), GN, SMB)

Oper (Op5ac0, DoSpl, DN, SNN, 0, DB, DR, Cd, (inst & 7), GN, SRB)
Oper (Op5ac8, DoDbpl, DN, SNN, 0, DW, DR, Cd, (inst & 7), GRW, SRW)
Oper (Op5ad0, DoSpl, DN, SNN, 0, DB, DA, Cain, (inst & 7), GN, SMB)
Oper (Op5ad8, DoSpl, DN, SNN, 0, DB, DA, CaipB, (inst & 7), GN, SMB)
Oper (Op5ae0, DoSpl, DN, SNN, 0, DB, DA, CmaiB, (inst & 7), GN, SMB)
Oper (Op5ae8, DoSpl, DN, SNN, 0, DB, DA, Cdai, (inst & 7), GN, SMB)
Oper (Op5af0, DoSpl, DN, SNN, 0, DB, DA, Caix, (inst & 7), GN, SMB)
Oper (Op5af8, DoSpl, DN, SNN, 0, DB, DA, Ceaw, (inst & 7), GN, SMB)

Oper (Op5bc0, DoSmi, DN, SNN, 0, DB, DR, Cd, (inst & 7), GN, SRB)
Oper (Op5bc8, DoDbmi, DN, SNN, 0, DW, DR, Cd, (inst & 7), GRW, SRW)
Oper (Op5bd0, DoSmi, DN, SNN, 0, DB, DA, Cain, (inst & 7), GN, SMB)
Oper (Op5bd8, DoSmi, DN, SNN, 0, DB, DA, CaipB, (inst & 7), GN, SMB)
Oper (Op5be0, DoSmi, DN, SNN, 0, DB, DA, CmaiB, (inst & 7), GN, SMB)
Oper (Op5be8, DoSmi, DN, SNN, 0, DB, DA, Cdai, (inst & 7), GN, SMB)
Oper (Op5bf0, DoSmi, DN, SNN, 0, DB, DA, Caix, (inst & 7), GN, SMB)
Oper (Op5bf8, DoSmi, DN, SNN, 0, DB, DA, Ceaw, (inst & 7), GN, SMB)

Oper (Op5cc0, DoSge, DN, SNN, 0, DB, DR, Cd, (inst & 7), GN, SRB)
Oper (Op5cc8, DoDbge, DN, SNN, 0, DW, DR, Cd, (inst & 7), GRW, SRW)
Oper (Op5cd0, DoSge, DN, SNN, 0, DB, DA, Cain, (inst & 7), GN, SMB)
Oper (Op5cd8, DoSge, DN, SNN, 0, DB, DA, CaipB, (inst & 7), GN, SMB)
Oper (Op5ce0, DoSge, DN, SNN, 0, DB, DA, CmaiB, (inst & 7), GN, SMB)
Oper (Op5ce8, DoSge, DN, SNN, 0, DB, DA, Cdai, (inst & 7), GN, SMB)
Oper (Op5cf0, DoSge, DN, SNN, 0, DB, DA, Caix, (inst & 7), GN, SMB)
Oper (Op5cf8, DoSge, DN, SNN, 0, DB, DA, Ceaw, (inst & 7), GN, SMB)

Oper (Op5dc0, DoSlt, DN, SNN, 0, DB, DR, Cd, (inst & 7), GN, SRB)
Oper (Op5dc8, DoDblt, DN, SNN, 0, DW, DR, Cd, (inst & 7), GRW, SRW)
Oper (Op5dd0, DoSlt, DN, SNN, 0, DB, DA, Cain, (inst & 7), GN, SMB)
Oper (Op5dd8, DoSlt, DN, SNN, 0, DB, DA, CaipB, (inst & 7), GN, SMB)
Oper (Op5de0, DoSlt, DN, SNN, 0, DB, DA, CmaiB, (inst & 7), GN, SMB)
Oper (Op5de8, DoSlt, DN, SNN, 0, DB, DA, Cdai, (inst & 7), GN, SMB)
Oper (Op5df0, DoSlt, DN, SNN, 0, DB, DA, Caix, (inst & 7), GN, SMB)
Oper (Op5df8, DoSlt, DN, SNN, 0, DB, DA, Ceaw, (inst & 7), GN, SMB)

Oper (Op5ec0, DoSgt, DN, SNN, 0, DB, DR, Cd, (inst & 7), GN, SRB)
Oper (Op5ec8, DoDbgt, DN, SNN, 0, DW, DR, Cd, (inst & 7), GRW, SRW)
Oper (Op5ed0, DoSgt, DN, SNN, 0, DB, DA, Cain, (inst & 7), GN, SMB)
Oper (Op5ed8, DoSgt, DN, SNN, 0, DB, DA, CaipB, (inst & 7), GN, SMB)
Oper (Op5ee0, DoSgt, DN, SNN, 0, DB, DA, CmaiB, (inst & 7), GN, SMB)
Oper (Op5ee8, DoSgt, DN, SNN, 0, DB, DA, Cdai, (inst & 7), GN, SMB)
Oper (Op5ef0, DoSgt, DN, SNN, 0, DB, DA, Caix, (inst & 7), GN, SMB)
Oper (Op5ef8, DoSgt, DN, SNN, 0, DB, DA, Ceaw, (inst & 7), GN, SMB)

Oper (Op5fc0, DoSle, DN, SNN, 0, DB, DR, Cd, (inst & 7), GN, SRB)
Oper (Op5fc8, DoDble, DN, SNN, 0, DW, DR, Cd, (inst & 7), GRW, SRW)
Oper (Op5fd0, DoSle, DN, SNN, 0, DB, DA, Cain, (inst & 7), GN, SMB)
Oper (Op5fd8, DoSle, DN, SNN, 0, DB, DA, CaipB, (inst & 7), GN, SMB)
Oper (Op5fe0, DoSle, DN, SNN, 0, DB, DA, CmaiB, (inst & 7), GN, SMB)
Oper (Op5fe8, DoSle, DN, SNN, 0, DB, DA, Cdai, (inst & 7), GN, SMB)
Oper (Op5ff0, DoSle, DN, SNN, 0, DB, DA, Caix, (inst & 7), GN, SMB)
Oper (Op5ff8, DoSle, DN, SNN, 0, DB, DA, Ceaw, (inst & 7), GN, SMB)

Oper (Op6000, DoBra, DW, SNQ, 0, DN, DN, CN, 0, GN, SN)
Oper (Op6008, DoBras, DW, SNQ, 0, DN, DN, CN, 0, GN, SN)
Oper (Op6100, DoBsr, DW, SNQ, 0, DN, DN, CN, 0, GN, SN)
Oper (Op6108, DoBsrs, DW, SNQ, 0, DN, DN, CN, 0, GN, SN)
Oper (Op6200, DoBhi, DW, SNQ, 0, DN, DN, CN, 0, GN, SN)
Oper (Op6208, DoBhis, DW, SNQ, 0, DN, DN, CN, 0, GN, SN)
Oper (Op6300, DoBls, DW, SNQ, 0, DN, DN, CN, 0, GN, SN)
Oper (Op6308, DoBlss, DW, SNQ, 0, DN, DN, CN, 0, GN, SN)
Oper (Op6400, DoBcc, DW, SNQ, 0, DN, DN, CN, 0, GN, SN)
Oper (Op6408, DoBccs, DW, SNQ, 0, DN, DN, CN, 0, GN, SN)
Oper (Op6500, DoBcs, DW, SNQ, 0, DN, DN, CN, 0, GN, SN)
Oper (Op6508, DoBcss, DW, SNQ, 0, DN, DN, CN, 0, GN, SN)
Oper (Op6600, DoBne, DW, SNQ, 0, DN, DN, CN, 0, GN, SN)
Oper (Op6608, DoBnes, DW, SNQ, 0, DN, DN, CN, 0, GN, SN)
Oper (Op6700, DoBeq, DW, SNQ, 0, DN, DN, CN, 0, GN, SN)
Oper (Op6708, DoBeqs, DW, SNQ, 0, DN, DN, CN, 0, GN, SN)
Oper (Op6800, DoBvc, DW, SNQ, 0, DN, DN, CN, 0, GN, SN)
Oper (Op6808, DoBvcs, DW, SNQ, 0, DN, DN, CN, 0, GN, SN)
Oper (Op6900, DoBvs, DW, SNQ, 0, DN, DN, CN, 0, GN, SN)
Oper (Op6908, DoBvss, DW, SNQ, 0, DN, DN, CN, 0, GN, SN)
Oper (Op6a00, DoBpl, DW, SNQ, 0, DN, DN, CN, 0, GN, SN)
Oper (Op6a08, DoBpls, DW, SNQ, 0, DN, DN, CN, 0, GN, SN)
Oper (Op6b00, DoBmi, DW, SNQ, 0, DN, DN, CN, 0, GN, SN)
Oper (Op6b08, DoBmis, DW, SNQ, 0, DN, DN, CN, 0, GN, SN)
Oper (Op6c00, DoBge, DW, SNQ, 0, DN, DN, CN, 0, GN, SN)
Oper (Op6c08, DoBges, DW, SNQ, 0, DN, DN, CN, 0, GN, SN)
Oper (Op6d00, DoBlt, DW, SNQ, 0, DN, DN, CN, 0, GN, SN)
Oper (Op6d08, DoBlts, DW, SNQ, 0, DN, DN, CN, 0, GN, SN)
Oper (Op6e00, DoBgt, DW, SNQ, 0, DN, DN, CN, 0, GN, SN)
Oper (Op6e08, DoBgts, DW, SNQ, 0, DN, DN, CN, 0, GN, SN)
Oper (Op6f00, DoBle, DW, SNQ, 0, DN, DN, CN, 0, GN, SN)
Oper (Op6f08, DoBles, DW, SNQ, 0, DN, DN, CN, 0, GN, SN)

Oper (Op0100, DoBtst32, DL, SdL, 0, DL, DR, Cd, (inst & 7), GRL, SN)
Oper (Op0108, DoMovep, DW, SdaiPW, (inst & 7), DW, DR, Cd, 0, GN, SRW)
Oper (Op0110, DoBtst8, DB, SdB, 0, DB, DA, Cain, (inst & 7), GMB, SN)
Oper (Op0118, DoBtst8, DB, SdB, 0, DB, DA, CaipB, (inst & 7), GMB, SN)
Oper (Op0120, DoBtst8, DB, SdB, 0, DB, DA, CmaiB, (inst & 7), GMB, SN)
Oper (Op0128, DoBtst8, DB, SdB, 0, DB, DA, Cdai, (inst & 7), GMB, SN)
Oper (Op0130, DoBtst8, DB, SdB, 0, DB, DA, Caix, (inst & 7), GMB, SN)
Oper (Op0138, DoBtst8, DB, SdB, 0, DB, DA, Cear, (inst & 7), GMB, SN)
Oper (Op0140, DoBchg32, DL, SdL, 0, DL, DR, Cd, (inst & 7), GRL, SRL)
Oper (Op0148, DoMovep, DL, SdaiPL, (inst & 7), DL, DR, Cd, 0, GN, SRL)
Oper (Op0150, DoBchg8, DB, SdB, 0, DB, DA, Cain, (inst & 7), GMB, SMB)
Oper (Op0158, DoBchg8, DB, SdB, 0, DB, DA, CaipB, (inst & 7), GMB, SMB)
Oper (Op0160, DoBchg8, DB, SdB, 0, DB, DA, CmaiB, (inst & 7), GMB, SMB)
Oper (Op0168, DoBchg8, DB, SdB, 0, DB, DA, Cdai, (inst & 7), GMB, SMB)
Oper (Op0170, DoBchg8, DB, SdB, 0, DB, DA, Caix, (inst & 7), GMB, SMB)
Oper (Op0178, DoBchg8, DB, SdB, 0, DB, DA, Cear, (inst & 7), GMB, SMB)
Oper (Op0180, DoBclr32, DL, SdL, 0, DL, DR, Cd, (inst & 7), GRL, SRL)
Oper (Op0188, DoMovep, DW, SdW, 0, DW, DA, Cdai, (inst & 7), GN, SPW)
Oper (Op0190, DoBclr8, DB, SdB, 0, DB, DA, Cain, (inst & 7), GMB, SMB)
Oper (Op0198, DoBclr8, DB, SdB, 0, DB, DA, CaipB, (inst & 7), GMB, SMB)
Oper (Op01a0, DoBclr8, DB, SdB, 0, DB, DA, CmaiB, (inst & 7), GMB, SMB)
Oper (Op01a8, DoBclr8, DB, SdB, 0, DB, DA, Cdai, (inst & 7), GMB, SMB)
Oper (Op01b0, DoBclr8, DB, SdB, 0, DB, DA, Caix, (inst & 7), GMB, SMB)
Oper (Op01b8, DoBclr8, DB, SdB, 0, DB, DA, Cear, (inst & 7), GMB, SMB)
Oper (Op01c0, DoBset32, DL, SdL, 0, DL, DR, Cd, (inst & 7), GRL, SRL)
Oper (Op01c8, DoMovep, DL, SdL, 0, DL, DA, Cdai, (inst & 7), GN, SPL)
Oper (Op01d0, DoBset8, DB, SdB, 0, DB, DA, Cain, (inst & 7), GMB, SMB)
Oper (Op01d8, DoBset8, DB, SdB, 0, DB, DA, CaipB, (inst & 7), GMB, SMB)
Oper (Op01e0, DoBset8, DB, SdB, 0, DB, DA, CmaiB, (inst & 7), GMB, SMB)
Oper (Op01e8, DoBset8, DB, SdB, 0, DB, DA, Cdai, (inst & 7), GMB, SMB)
Oper (Op01f0, DoBset8, DB, SdB, 0, DB, DA, Caix, (inst & 7), GMB, SMB)
Oper (Op01f8, DoBset8, DB, SdB, 0, DB, DA, Cear, (inst & 7), GMB, SMB)

Oper (Op0300, DoBtst32, DL, SdL, 1, DL, DR, Cd, (inst & 7), GRL, SN)
Oper (Op0308, DoMovep, DW, SdaiPW, (inst & 7), DW, DR, Cd, 1, GN, SRW)
Oper (Op0310, DoBtst8, DB, SdB, 1, DB, DA, Cain, (inst & 7), GMB, SN)
Oper (Op0318, DoBtst8, DB, SdB, 1, DB, DA, CaipB, (inst & 7), GMB, SN)
Oper (Op0320, DoBtst8, DB, SdB, 1, DB, DA, CmaiB, (inst & 7), GMB, SN)
Oper (Op0328, DoBtst8, DB, SdB, 1, DB, DA, Cdai, (inst & 7), GMB, SN)
Oper (Op0330, DoBtst8, DB, SdB, 1, DB, DA, Caix, (inst & 7), GMB, SN)
Oper (Op0338, DoBtst8, DB, SdB, 1, DB, DA, Cear, (inst & 7), GMB, SN)
Oper (Op0340, DoBchg32, DL, SdL, 1, DL, DR, Cd, (inst & 7), GRL, SRL)
Oper (Op0348, DoMovep, DL, SdaiPL, (inst & 7), DL, DR, Cd, 1, GN, SRL)
Oper (Op0350, DoBchg8, DB, SdB, 1, DB, DA, Cain, (inst & 7), GMB, SMB)
Oper (Op0358, DoBchg8, DB, SdB, 1, DB, DA, CaipB, (inst & 7), GMB, SMB)
Oper (Op0360, DoBchg8, DB, SdB, 1, DB, DA, CmaiB, (inst & 7), GMB, SMB)
Oper (Op0368, DoBchg8, DB, SdB, 1, DB, DA, Cdai, (inst & 7), GMB, SMB)
Oper (Op0370, DoBchg8, DB, SdB, 1, DB, DA, Caix, (inst & 7), GMB, SMB)
Oper (Op0378, DoBchg8, DB, SdB, 1, DB, DA, Cear, (inst & 7), GMB, SMB)
Oper (Op0380, DoBclr32, DL, SdL, 1, DL, DR, Cd, (inst & 7), GRL, SRL)
Oper (Op0388, DoMovep, DW, SdW, 1, DW, DA, Cdai, (inst & 7), GN, SPW)
Oper (Op0390, DoBclr8, DB, SdB, 1, DB, DA, Cain, (inst & 7), GMB, SMB)
Oper (Op0398, DoBclr8, DB, SdB, 1, DB, DA, CaipB, (inst & 7), GMB, SMB)
Oper (Op03a0, DoBclr8, DB, SdB, 1, DB, DA, CmaiB, (inst & 7), GMB, SMB)
Oper (Op03a8, DoBclr8, DB, SdB, 1, DB, DA, Cdai, (inst & 7), GMB, SMB)
Oper (Op03b0, DoBclr8, DB, SdB, 1, DB, DA, Caix, (inst & 7), GMB, SMB)
Oper (Op03b8, DoBclr8, DB, SdB, 1, DB, DA, Cear, (inst & 7), GMB, SMB)
Oper (Op03c0, DoBset32, DL, SdL, 1, DL, DR, Cd, (inst & 7), GRL, SRL)
Oper (Op03c8, DoMovep, DL, SdL, 1, DL, DA, Cdai, (inst & 7), GN, SPL)
Oper (Op03d0, DoBset8, DB, SdB, 1, DB, DA, Cain, (inst & 7), GMB, SMB)
Oper (Op03d8, DoBset8, DB, SdB, 1, DB, DA, CaipB, (inst & 7), GMB, SMB)
Oper (Op03e0, DoBset8, DB, SdB, 1, DB, DA, CmaiB, (inst & 7), GMB, SMB)
Oper (Op03e8, DoBset8, DB, SdB, 1, DB, DA, Cdai, (inst & 7), GMB, SMB)
Oper (Op03f0, DoBset8, DB, SdB, 1, DB, DA, Caix, (inst & 7), GMB, SMB)
Oper (Op03f8, DoBset8, DB, SdB, 1, DB, DA, Cear, (inst & 7), GMB, SMB)

Oper (Op0500, DoBtst32, DL, SdL, 2, DL, DR, Cd, (inst & 7), GRL, SN)
Oper (Op0508, DoMovep, DW, SdaiPW, (inst & 7), DW, DR, Cd, 2, GN, SRW)
Oper (Op0510, DoBtst8, DB, SdB, 2, DB, DA, Cain, (inst & 7), GMB, SN)
Oper (Op0518, DoBtst8, DB, SdB, 2, DB, DA, CaipB, (inst & 7), GMB, SN)
Oper (Op0520, DoBtst8, DB, SdB, 2, DB, DA, CmaiB, (inst & 7), GMB, SN)
Oper (Op0528, DoBtst8, DB, SdB, 2, DB, DA, Cdai, (inst & 7), GMB, SN)
Oper (Op0530, DoBtst8, DB, SdB, 2, DB, DA, Caix, (inst & 7), GMB, SN)
Oper (Op0538, DoBtst8, DB, SdB, 2, DB, DA, Cear, (inst & 7), GMB, SN)
Oper (Op0540, DoBchg32, DL, SdL, 2, DL, DR, Cd, (inst & 7), GRL, SRL)
Oper (Op0548, DoMovep, DL, SdaiPL, (inst & 7), DL, DR, Cd, 2, GN, SRL)
Oper (Op0550, DoBchg8, DB, SdB, 2, DB, DA, Cain, (inst & 7), GMB, SMB)
Oper (Op0558, DoBchg8, DB, SdB, 2, DB, DA, CaipB, (inst & 7), GMB, SMB)
Oper (Op0560, DoBchg8, DB, SdB, 2, DB, DA, CmaiB, (inst & 7), GMB, SMB)
Oper (Op0568, DoBchg8, DB, SdB, 2, DB, DA, Cdai, (inst & 7), GMB, SMB)
Oper (Op0570, DoBchg8, DB, SdB, 2, DB, DA, Caix, (inst & 7), GMB, SMB)
Oper (Op0578, DoBchg8, DB, SdB, 2, DB, DA, Cear, (inst & 7), GMB, SMB)
Oper (Op0580, DoBclr32, DL, SdL, 2, DL, DR, Cd, (inst & 7), GRL, SRL)
Oper (Op0588, DoMovep, DW, SdW, 2, DW, DA, Cdai, (inst & 7), GN, SPW)
Oper (Op0590, DoBclr8, DB, SdB, 2, DB, DA, Cain, (inst & 7), GMB, SMB)
Oper (Op0598, DoBclr8, DB, SdB, 2, DB, DA, CaipB, (inst & 7), GMB, SMB)
Oper (Op05a0, DoBclr8, DB, SdB, 2, DB, DA, CmaiB, (inst & 7), GMB, SMB)
Oper (Op05a8, DoBclr8, DB, SdB, 2, DB, DA, Cdai, (inst & 7), GMB, SMB)
Oper (Op05b0, DoBclr8, DB, SdB, 2, DB, DA, Caix, (inst & 7), GMB, SMB)
Oper (Op05b8, DoBclr8, DB, SdB, 2, DB, DA, Cear, (inst & 7), GMB, SMB)
Oper (Op05c0, DoBset32, DL, SdL, 2, DL, DR, Cd, (inst & 7), GRL, SRL)
Oper (Op05c8, DoMovep, DL, SdL, 2, DL, DA, Cdai, (inst & 7), GN, SPL)
Oper (Op05d0, DoBset8, DB, SdB, 2, DB, DA, Cain, (inst & 7), GMB, SMB)
Oper (Op05d8, DoBset8, DB, SdB, 2, DB, DA, CaipB, (inst & 7), GMB, SMB)
Oper (Op05e0, DoBset8, DB, SdB, 2, DB, DA, CmaiB, (inst & 7), GMB, SMB)
Oper (Op05e8, DoBset8, DB, SdB, 2, DB, DA, Cdai, (inst & 7), GMB, SMB)
Oper (Op05f0, DoBset8, DB, SdB, 2, DB, DA, Caix, (inst & 7), GMB, SMB)
Oper (Op05f8, DoBset8, DB, SdB, 2, DB, DA, Cear, (inst & 7), GMB, SMB)

Oper (Op0700, DoBtst32, DL, SdL, 3, DL, DR, Cd, (inst & 7), GRL, SN)
Oper (Op0708, DoMovep, DW, SdaiPW, (inst & 7), DW, DR, Cd, 3, GN, SRW)
Oper (Op0710, DoBtst8, DB, SdB, 3, DB, DA, Cain, (inst & 7), GMB, SN)
Oper (Op0718, DoBtst8, DB, SdB, 3, DB, DA, CaipB, (inst & 7), GMB, SN)
Oper (Op0720, DoBtst8, DB, SdB, 3, DB, DA, CmaiB, (inst & 7), GMB, SN)
Oper (Op0728, DoBtst8, DB, SdB, 3, DB, DA, Cdai, (inst & 7), GMB, SN)
Oper (Op0730, DoBtst8, DB, SdB, 3, DB, DA, Caix, (inst & 7), GMB, SN)
Oper (Op0738, DoBtst8, DB, SdB, 3, DB, DA, Cear, (inst & 7), GMB, SN)
Oper (Op0740, DoBchg32, DL, SdL, 3, DL, DR, Cd, (inst & 7), GRL, SRL)
Oper (Op0748, DoMovep, DL, SdaiPL, (inst & 7), DL, DR, Cd, 3, GN, SRL)
Oper (Op0750, DoBchg8, DB, SdB, 3, DB, DA, Cain, (inst & 7), GMB, SMB)
Oper (Op0758, DoBchg8, DB, SdB, 3, DB, DA, CaipB, (inst & 7), GMB, SMB)
Oper (Op0760, DoBchg8, DB, SdB, 3, DB, DA, CmaiB, (inst & 7), GMB, SMB)
Oper (Op0768, DoBchg8, DB, SdB, 3, DB, DA, Cdai, (inst & 7), GMB, SMB)
Oper (Op0770, DoBchg8, DB, SdB, 3, DB, DA, Caix, (inst & 7), GMB, SMB)
Oper (Op0778, DoBchg8, DB, SdB, 3, DB, DA, Cear, (inst & 7), GMB, SMB)
Oper (Op0780, DoBclr32, DL, SdL, 3, DL, DR, Cd, (inst & 7), GRL, SRL)
Oper (Op0788, DoMovep, DW, SdW, 3, DW, DA, Cdai, (inst & 7), GN, SPW)
Oper (Op0790, DoBclr8, DB, SdB, 3, DB, DA, Cain, (inst & 7), GMB, SMB)
Oper (Op0798, DoBclr8, DB, SdB, 3, DB, DA, CaipB, (inst & 7), GMB, SMB)
Oper (Op07a0, DoBclr8, DB, SdB, 3, DB, DA, CmaiB, (inst & 7), GMB, SMB)
Oper (Op07a8, DoBclr8, DB, SdB, 3, DB, DA, Cdai, (inst & 7), GMB, SMB)
Oper (Op07b0, DoBclr8, DB, SdB, 3, DB, DA, Caix, (inst & 7), GMB, SMB)
Oper (Op07b8, DoBclr8, DB, SdB, 3, DB, DA, Cear, (inst & 7), GMB, SMB)
Oper (Op07c0, DoBset32, DL, SdL, 3, DL, DR, Cd, (inst & 7), GRL, SRL)
Oper (Op07c8, DoMovep, DL, SdL, 3, DL, DA, Cdai, (inst & 7), GN, SPL)
Oper (Op07d0, DoBset8, DB, SdB, 3, DB, DA, Cain, (inst & 7), GMB, SMB)
Oper (Op07d8, DoBset8, DB, SdB, 3, DB, DA, CaipB, (inst & 7), GMB, SMB)
Oper (Op07e0, DoBset8, DB, SdB, 3, DB, DA, CmaiB, (inst & 7), GMB, SMB)
Oper (Op07e8, DoBset8, DB, SdB, 3, DB, DA, Cdai, (inst & 7), GMB, SMB)
Oper (Op07f0, DoBset8, DB, SdB, 3, DB, DA, Caix, (inst & 7), GMB, SMB)
Oper (Op07f8, DoBset8, DB, SdB, 3, DB, DA, Cear, (inst & 7), GMB, SMB)

Oper (Op0900, DoBtst32, DL, SdL, 4, DL, DR, Cd, (inst & 7), GRL, SN)
Oper (Op0908, DoMovep, DW, SdaiPW, (inst & 7), DW, DR, Cd, 4, GN, SRW)
Oper (Op0910, DoBtst8, DB, SdB, 4, DB, DA, Cain, (inst & 7), GMB, SN)
Oper (Op0918, DoBtst8, DB, SdB, 4, DB, DA, CaipB, (inst & 7), GMB, SN)
Oper (Op0920, DoBtst8, DB, SdB, 4, DB, DA, CmaiB, (inst & 7), GMB, SN)
Oper (Op0928, DoBtst8, DB, SdB, 4, DB, DA, Cdai, (inst & 7), GMB, SN)
Oper (Op0930, DoBtst8, DB, SdB, 4, DB, DA, Caix, (inst & 7), GMB, SN)
Oper (Op0938, DoBtst8, DB, SdB, 4, DB, DA, Cear, (inst & 7), GMB, SN)
Oper (Op0940, DoBchg32, DL, SdL, 4, DL, DR, Cd, (inst & 7), GRL, SRL)
Oper (Op0948, DoMovep, DL, SdaiPL, (inst & 7), DL, DR, Cd, 4, GN, SRL)
Oper (Op0950, DoBchg8, DB, SdB, 4, DB, DA, Cain, (inst & 7), GMB, SMB)
Oper (Op0958, DoBchg8, DB, SdB, 4, DB, DA, CaipB, (inst & 7), GMB, SMB)
Oper (Op0960, DoBchg8, DB, SdB, 4, DB, DA, CmaiB, (inst & 7), GMB, SMB)
Oper (Op0968, DoBchg8, DB, SdB, 4, DB, DA, Cdai, (inst & 7), GMB, SMB)
Oper (Op0970, DoBchg8, DB, SdB, 4, DB, DA, Caix, (inst & 7), GMB, SMB)
Oper (Op0978, DoBchg8, DB, SdB, 4, DB, DA, Cear, (inst & 7), GMB, SMB)
Oper (Op0980, DoBclr32, DL, SdL, 4, DL, DR, Cd, (inst & 7), GRL, SRL)
Oper (Op0988, DoMovep, DW, SdW, 4, DW, DA, Cdai, (inst & 7), GN, SPW)
Oper (Op0990, DoBclr8, DB, SdB, 4, DB, DA, Cain, (inst & 7), GMB, SMB)
Oper (Op0998, DoBclr8, DB, SdB, 4, DB, DA, CaipB, (inst & 7), GMB, SMB)
Oper (Op09a0, DoBclr8, DB, SdB, 4, DB, DA, CmaiB, (inst & 7), GMB, SMB)
Oper (Op09a8, DoBclr8, DB, SdB, 4, DB, DA, Cdai, (inst & 7), GMB, SMB)
Oper (Op09b0, DoBclr8, DB, SdB, 4, DB, DA, Caix, (inst & 7), GMB, SMB)
Oper (Op09b8, DoBclr8, DB, SdB, 4, DB, DA, Cear, (inst & 7), GMB, SMB)
Oper (Op09c0, DoBset32, DL, SdL, 4, DL, DR, Cd, (inst & 7), GRL, SRL)
Oper (Op09c8, DoMovep, DL, SdL, 4, DL, DA, Cdai, (inst & 7), GN, SPL)
Oper (Op09d0, DoBset8, DB, SdB, 4, DB, DA, Cain, (inst & 7), GMB, SMB)
Oper (Op09d8, DoBset8, DB, SdB, 4, DB, DA, CaipB, (inst & 7), GMB, SMB)
Oper (Op09e0, DoBset8, DB, SdB, 4, DB, DA, CmaiB, (inst & 7), GMB, SMB)
Oper (Op09e8, DoBset8, DB, SdB, 4, DB, DA, Cdai, (inst & 7), GMB, SMB)
Oper (Op09f0, DoBset8, DB, SdB, 4, DB, DA, Caix, (inst & 7), GMB, SMB)
Oper (Op09f8, DoBset8, DB, SdB, 4, DB, DA, Cear, (inst & 7), GMB, SMB)

Oper (Op0b00, DoBtst32, DL, SdL, 5, DL, DR, Cd, (inst & 7), GRL, SN)
Oper (Op0b08, DoMovep, DW, SdaiPW, (inst & 7), DW, DR, Cd, 5, GN, SRW)
Oper (Op0b10, DoBtst8, DB, SdB, 5, DB, DA, Cain, (inst & 7), GMB, SN)
Oper (Op0b18, DoBtst8, DB, SdB, 5, DB, DA, CaipB, (inst & 7), GMB, SN)
Oper (Op0b20, DoBtst8, DB, SdB, 5, DB, DA, CmaiB, (inst & 7), GMB, SN)
Oper (Op0b28, DoBtst8, DB, SdB, 5, DB, DA, Cdai, (inst & 7), GMB, SN)
Oper (Op0b30, DoBtst8, DB, SdB, 5, DB, DA, Caix, (inst & 7), GMB, SN)
Oper (Op0b38, DoBtst8, DB, SdB, 5, DB, DA, Cear, (inst & 7), GMB, SN)
Oper (Op0b40, DoBchg32, DL, SdL, 5, DL, DR, Cd, (inst & 7), GRL, SRL)
Oper (Op0b48, DoMovep, DL, SdaiPL, (inst & 7), DL, DR, Cd, 5, GN, SRL)
Oper (Op0b50, DoBchg8, DB, SdB, 5, DB, DA, Cain, (inst & 7), GMB, SMB)
Oper (Op0b58, DoBchg8, DB, SdB, 5, DB, DA, CaipB, (inst & 7), GMB, SMB)
Oper (Op0b60, DoBchg8, DB, SdB, 5, DB, DA, CmaiB, (inst & 7), GMB, SMB)
Oper (Op0b68, DoBchg8, DB, SdB, 5, DB, DA, Cdai, (inst & 7), GMB, SMB)
Oper (Op0b70, DoBchg8, DB, SdB, 5, DB, DA, Caix, (inst & 7), GMB, SMB)
Oper (Op0b78, DoBchg8, DB, SdB, 5, DB, DA, Cear, (inst & 7), GMB, SMB)
Oper (Op0b80, DoBclr32, DL, SdL, 5, DL, DR, Cd, (inst & 7), GRL, SRL)
Oper (Op0b88, DoMovep, DW, SdW, 5, DW, DA, Cdai, (inst & 7), GN, SPW)
Oper (Op0b90, DoBclr8, DB, SdB, 5, DB, DA, Cain, (inst & 7), GMB, SMB)
Oper (Op0b98, DoBclr8, DB, SdB, 5, DB, DA, CaipB, (inst & 7), GMB, SMB)
Oper (Op0ba0, DoBclr8, DB, SdB, 5, DB, DA, CmaiB, (inst & 7), GMB, SMB)
Oper (Op0ba8, DoBclr8, DB, SdB, 5, DB, DA, Cdai, (inst & 7), GMB, SMB)
Oper (Op0bb0, DoBclr8, DB, SdB, 5, DB, DA, Caix, (inst & 7), GMB, SMB)
Oper (Op0bb8, DoBclr8, DB, SdB, 5, DB, DA, Cear, (inst & 7), GMB, SMB)
Oper (Op0bc0, DoBset32, DL, SdL, 5, DL, DR, Cd, (inst & 7), GRL, SRL)
Oper (Op0bc8, DoMovep, DL, SdL, 5, DL, DA, Cdai, (inst & 7), GN, SPL)
Oper (Op0bd0, DoBset8, DB, SdB, 5, DB, DA, Cain, (inst & 7), GMB, SMB)
Oper (Op0bd8, DoBset8, DB, SdB, 5, DB, DA, CaipB, (inst & 7), GMB, SMB)
Oper (Op0be0, DoBset8, DB, SdB, 5, DB, DA, CmaiB, (inst & 7), GMB, SMB)
Oper (Op0be8, DoBset8, DB, SdB, 5, DB, DA, Cdai, (inst & 7), GMB, SMB)
Oper (Op0bf0, DoBset8, DB, SdB, 5, DB, DA, Caix, (inst & 7), GMB, SMB)
Oper (Op0bf8, DoBset8, DB, SdB, 5, DB, DA, Cear, (inst & 7), GMB, SMB)

Oper (Op0d00, DoBtst32, DL, SdL, 6, DL, DR, Cd, (inst & 7), GRL, SN)
Oper (Op0d08, DoMovep, DW, SdaiPW, (inst & 7), DW, DR, Cd, 6, GN, SRW)
Oper (Op0d10, DoBtst8, DB, SdB, 6, DB, DA, Cain, (inst & 7), GMB, SN)
Oper (Op0d18, DoBtst8, DB, SdB, 6, DB, DA, CaipB, (inst & 7), GMB, SN)
Oper (Op0d20, DoBtst8, DB, SdB, 6, DB, DA, CmaiB, (inst & 7), GMB, SN)
Oper (Op0d28, DoBtst8, DB, SdB, 6, DB, DA, Cdai, (inst & 7), GMB, SN)
Oper (Op0d30, DoBtst8, DB, SdB, 6, DB, DA, Caix, (inst & 7), GMB, SN)
Oper (Op0d38, DoBtst8, DB, SdB, 6, DB, DA, Cear, (inst & 7), GMB, SN)
Oper (Op0d40, DoBchg32, DL, SdL, 6, DL, DR, Cd, (inst & 7), GRL, SRL)
Oper (Op0d48, DoMovep, DL, SdaiPL, (inst & 7), DL, DR, Cd, 6, GN, SRL)
Oper (Op0d50, DoBchg8, DB, SdB, 6, DB, DA, Cain, (inst & 7), GMB, SMB)
Oper (Op0d58, DoBchg8, DB, SdB, 6, DB, DA, CaipB, (inst & 7), GMB, SMB)
Oper (Op0d60, DoBchg8, DB, SdB, 6, DB, DA, CmaiB, (inst & 7), GMB, SMB)
Oper (Op0d68, DoBchg8, DB, SdB, 6, DB, DA, Cdai, (inst & 7), GMB, SMB)
Oper (Op0d70, DoBchg8, DB, SdB, 6, DB, DA, Caix, (inst & 7), GMB, SMB)
Oper (Op0d78, DoBchg8, DB, SdB, 6, DB, DA, Cear, (inst & 7), GMB, SMB)
Oper (Op0d80, DoBclr32, DL, SdL, 6, DL, DR, Cd, (inst & 7), GRL, SRL)
Oper (Op0d88, DoMovep, DW, SdW, 6, DW, DA, Cdai, (inst & 7), GN, SPW)
Oper (Op0d90, DoBclr8, DB, SdB, 6, DB, DA, Cain, (inst & 7), GMB, SMB)
Oper (Op0d98, DoBclr8, DB, SdB, 6, DB, DA, CaipB, (inst & 7), GMB, SMB)
Oper (Op0da0, DoBclr8, DB, SdB, 6, DB, DA, CmaiB, (inst & 7), GMB, SMB)
Oper (Op0da8, DoBclr8, DB, SdB, 6, DB, DA, Cdai, (inst & 7), GMB, SMB)
Oper (Op0db0, DoBclr8, DB, SdB, 6, DB, DA, Caix, (inst & 7), GMB, SMB)
Oper (Op0db8, DoBclr8, DB, SdB, 6, DB, DA, Cear, (inst & 7), GMB, SMB)
Oper (Op0dc0, DoBset32, DL, SdL, 6, DL, DR, Cd, (inst & 7), GRL, SRL)
Oper (Op0dc8, DoMovep, DL, SdL, 6, DL, DA, Cdai, (inst & 7), GN, SPL)
Oper (Op0dd0, DoBset8, DB, SdB, 6, DB, DA, Cain, (inst & 7), GMB, SMB)
Oper (Op0dd8, DoBset8, DB, SdB, 6, DB, DA, CaipB, (inst & 7), GMB, SMB)
Oper (Op0de0, DoBset8, DB, SdB, 6, DB, DA, CmaiB, (inst & 7), GMB, SMB)
Oper (Op0de8, DoBset8, DB, SdB, 6, DB, DA, Cdai, (inst & 7), GMB, SMB)
Oper (Op0df0, DoBset8, DB, SdB, 6, DB, DA, Caix, (inst & 7), GMB, SMB)
Oper (Op0df8, DoBset8, DB, SdB, 6, DB, DA, Cear, (inst & 7), GMB, SMB)

OMoves (Op0e10, DB, Cain, GRB, SRB, GMB, SMB)
OMoves (Op0e18, DB, CaipB, GRB, SRB, GMB, SMB)
OMoves (Op0e20, DB, CmaiB, GRB, SRB, GMB, SMB)
OMoves (Op0e28, DB, Cdai, GRB, SRB, GMB, SMB)
OMoves (Op0e30, DB, Caix, GRB, SRB, GMB, SMB)
OMoves (Op0e38, DB, Ceaw, GRB, SRB, GMB, SMB)
OMoves (Op0e50, DW, Cain, GRW, SRW, GMW, SMW)
OMoves (Op0e58, DW, CaipB, GRW, SRW, GMW, SMW)
OMoves (Op0e60, DW, CmaiB, GRW, SRW, GMW, SMW)
OMoves (Op0e68, DW, Cdai, GRW, SRW, GMW, SMW)
OMoves (Op0e70, DW, Caix, GRW, SRW, GMW, SMW)
OMoves (Op0e78, DW, Ceaw, GRW, SRW, GMW, SMW)
OMoves (Op0e90, DL, Cain, GRL, SRL, GML, SML)
OMoves (Op0e98, DL, CaipB, GRL, SRL, GML, SML)
OMoves (Op0ea0, DL, CmaiB, GRL, SRL, GML, SML)
OMoves (Op0ea8, DL, Cdai, GRL, SRL, GML, SML)
OMoves (Op0eb0, DL, Caix, GRL, SRL, GML, SML)
OMoves (Op0eb8, DL, Ceaw, GRL, SRL, GML, SML)

Oper (Op0f00, DoBtst32, DL, SdL, 7, DL, DR, Cd, (inst & 7), GRL, SN)
Oper (Op0f08, DoMovep, DW, SdaiPW, (inst & 7), DW, DR, Cd, 7, GN, SRW)
Oper (Op0f10, DoBtst8, DB, SdB, 7, DB, DA, Cain, (inst & 7), GMB, SN)
Oper (Op0f18, DoBtst8, DB, SdB, 7, DB, DA, CaipB, (inst & 7), GMB, SN)
Oper (Op0f20, DoBtst8, DB, SdB, 7, DB, DA, CmaiB, (inst & 7), GMB, SN)
Oper (Op0f28, DoBtst8, DB, SdB, 7, DB, DA, Cdai, (inst & 7), GMB, SN)
Oper (Op0f30, DoBtst8, DB, SdB, 7, DB, DA, Caix, (inst & 7), GMB, SN)
Oper (Op0f38, DoBtst8, DB, SdB, 7, DB, DA, Cear, (inst & 7), GMB, SN)
Oper (Op0f40, DoBchg32, DL, SdL, 7, DL, DR, Cd, (inst & 7), GRL, SRL)
Oper (Op0f48, DoMovep, DL, SdaiPL, (inst & 7), DL, DR, Cd, 7, GN, SRL)
Oper (Op0f50, DoBchg8, DB, SdB, 7, DB, DA, Cain, (inst & 7), GMB, SMB)
Oper (Op0f58, DoBchg8, DB, SdB, 7, DB, DA, CaipB, (inst & 7), GMB, SMB)
Oper (Op0f60, DoBchg8, DB, SdB, 7, DB, DA, CmaiB, (inst & 7), GMB, SMB)
Oper (Op0f68, DoBchg8, DB, SdB, 7, DB, DA, Cdai, (inst & 7), GMB, SMB)
Oper (Op0f70, DoBchg8, DB, SdB, 7, DB, DA, Caix, (inst & 7), GMB, SMB)
Oper (Op0f78, DoBchg8, DB, SdB, 7, DB, DA, Cear, (inst & 7), GMB, SMB)
Oper (Op0f80, DoBclr32, DL, SdL, 7, DL, DR, Cd, (inst & 7), GRL, SRL)
Oper (Op0f88, DoMovep, DW, SdW, 7, DW, DA, Cdai, (inst & 7), GN, SPW)
Oper (Op0f90, DoBclr8, DB, SdB, 7, DB, DA, Cain, (inst & 7), GMB, SMB)
Oper (Op0f98, DoBclr8, DB, SdB, 7, DB, DA, CaipB, (inst & 7), GMB, SMB)
Oper (Op0fa0, DoBclr8, DB, SdB, 7, DB, DA, CmaiB, (inst & 7), GMB, SMB)
Oper (Op0fa8, DoBclr8, DB, SdB, 7, DB, DA, Cdai, (inst & 7), GMB, SMB)
Oper (Op0fb0, DoBclr8, DB, SdB, 7, DB, DA, Caix, (inst & 7), GMB, SMB)
Oper (Op0fb8, DoBclr8, DB, SdB, 7, DB, DA, Cear, (inst & 7), GMB, SMB)
Oper (Op0fc0, DoBset32, DL, SdL, 7, DL, DR, Cd, (inst & 7), GRL, SRL)
Oper (Op0fc8, DoMovep, DL, SdL, 7, DL, DA, Cdai, (inst & 7), GN, SPL)
Oper (Op0fd0, DoBset8, DB, SdB, 7, DB, DA, Cain, (inst & 7), GMB, SMB)
Oper (Op0fd8, DoBset8, DB, SdB, 7, DB, DA, CaipB, (inst & 7), GMB, SMB)
Oper (Op0fe0, DoBset8, DB, SdB, 7, DB, DA, CmaiB, (inst & 7), GMB, SMB)
Oper (Op0fe8, DoBset8, DB, SdB, 7, DB, DA, Cdai, (inst & 7), GMB, SMB)
Oper (Op0ff0, DoBset8, DB, SdB, 7, DB, DA, Caix, (inst & 7), GMB, SMB)
Oper (Op0ff8, DoBset8, DB, SdB, 7, DB, DA, Cear, (inst & 7), GMB, SMB)

Oper (Op0800, DoBtst32, DB, SimmB, 0, DL, DR, Cd, (inst & 7), GRL, SN)
Oper (Op0810, DoBtst8, DB, SimmB, 0, DB, DA, Cain, (inst & 7), GMB, SN)
Oper (Op0818, DoBtst8, DB, SimmB, 0, DB, DA, CaipB, (inst & 7), GMB, SN)
Oper (Op0820, DoBtst8, DB, SimmB, 0, DB, DA, CmaiB, (inst & 7), GMB, SN)
Oper (Op0828, DoBtst8, DB, SimmB, 0, DB, DA, Cdai, (inst & 7), GMB, SN)
Oper (Op0830, DoBtst8, DB, SimmB, 0, DB, DA, Caix, (inst & 7), GMB, SN)
Oper (Op0838, DoBtst8, DB, SimmB, 0, DB, DA, Cear, (inst & 7), GMB, SN)
Oper (Op0840, DoBchg32, DL, SimmB, 0, DL, DR, Cd, (inst & 7), GRL, SRL)
Oper (Op0850, DoBchg8, DB, SimmB, 0, DB, DA, Cain, (inst & 7), GMB, SMB)
Oper (Op0858, DoBchg8, DB, SimmB, 0, DB, DA, CaipB, (inst & 7), GMB, SMB)
Oper (Op0860, DoBchg8, DB, SimmB, 0, DB, DA, CmaiB, (inst & 7), GMB, SMB)
Oper (Op0868, DoBchg8, DB, SimmB, 0, DB, DA, Cdai, (inst & 7), GMB, SMB)
Oper (Op0870, DoBchg8, DB, SimmB, 0, DB, DA, Caix, (inst & 7), GMB, SMB)
Oper (Op0878, DoBchg8, DB, SimmB, 0, DB, DA, Cear, (inst & 7), GMB, SMB)
Oper (Op0880, DoBclr32, DL, SimmB, 0, DL, DR, Cd, (inst & 7), GRL, SRL)
Oper (Op0890, DoBclr8, DB, SimmB, 0, DB, DA, Cain, (inst & 7), GMB, SMB)
Oper (Op0898, DoBclr8, DB, SimmB, 0, DB, DA, CaipB, (inst & 7), GMB, SMB)
Oper (Op08a0, DoBclr8, DB, SimmB, 0, DB, DA, CmaiB, (inst & 7), GMB, SMB)
Oper (Op08a8, DoBclr8, DB, SimmB, 0, DB, DA, Cdai, (inst & 7), GMB, SMB)
Oper (Op08b0, DoBclr8, DB, SimmB, 0, DB, DA, Caix, (inst & 7), GMB, SMB)
Oper (Op08b8, DoBclr8, DB, SimmB, 0, DB, DA, Cear, (inst & 7), GMB, SMB)
Oper (Op08c0, DoBset32, DL, SimmB, 0, DL, DR, Cd, (inst & 7), GRL, SRL)
Oper (Op08d0, DoBset8, DB, SimmB, 0, DB, DA, Cain, (inst & 7), GMB, SMB)
Oper (Op08d8, DoBset8, DB, SimmB, 0, DB, DA, CaipB, (inst & 7), GMB, SMB)
Oper (Op08e0, DoBset8, DB, SimmB, 0, DB, DA, CmaiB, (inst & 7), GMB, SMB)
Oper (Op08e8, DoBset8, DB, SimmB, 0, DB, DA, Cdai, (inst & 7), GMB, SMB)
Oper (Op08f0, DoBset8, DB, SimmB, 0, DB, DA, Caix, (inst & 7), GMB, SMB)
Oper (Op08f8, DoBset8, DB, SimmB, 0, DB, DA, Cear, (inst & 7), GMB, SMB)
