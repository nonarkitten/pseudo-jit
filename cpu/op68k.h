/*
 * Castaway
 *  (C) 1994 - 2002 Martin Doering, Joachim Hoenig
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */
#ifndef OP68KH
#define OP68KH

#include "config.h"



/*
 * Status Register Access
 */

#if SR_IMPLEMENTATION == 2

#define SetC(flag) if (flag) status |= 0x1U ; else status &= ~0x1U
#define SetV(flag) if (flag) status |= 0x2U ; else status &= ~0x2U
#define SetZ(flag) if (flag) status |= 0x4U ; else status &= ~0x4U
#define SetN(flag) if (flag) status |= 0x8U ; else status &= ~0x8U
#define SetX(flag) if (flag) status |= 0x10U; else status &= ~0x10U
#define SetXC(flag) if (flag) status |= 0x11U; else status &= ~0x11U


/* Optimized SetNZ  */
// before: #define SetNZ(operand) SetN(operand < 0); SetZ(operand == 0)
#define SetNZ(operand) \
    if (operand) \
        {status &= ~0xcU; if (operand < 0) status |= 0x8U ;} \
    else \
        { status |= 0x4U ; status &= ~0x8U ;}


/* Optimized ClrCVSetNZ - ClrCV is integrated into SetNZ */
// before #define ClrCVSetNZ(operand) ClrCV(); SetNZ(operand)
#define ClrCVSetNZ(operand) \
    if (operand) \
        {status &= ~0xfU; if (operand < 0) status |= 0x8U ;} \
    else \
        { status |= 0x4U ; status &= ~0xaU ;}


#define SetS(flag) \
    if ((status&0x2000U)) \
        ssp = areg[7]; \
    else \
        usp = areg[7]; \
    if ((flag)) \
        {status |= 0x2000U;   areg[7] = ssp; } \
    else \
        {status &= ~0x2000U ; areg[7] = usp; }

#define SetT(flag) if (flag) status |= 0x8000U; else status &= ~0x8000U
#define SetI(int) status = ((status&0xf8ffU)|(((int)<<8)&0x0700U))

#define ClrCV() status &= ~0x3U

#define GetC() (!!(status&0x1))
#define GetV() (!!(status&0x2))
#define GetZ() (!!(status&0x4))
#define GetN() (!!(status&0x8))
#define GetX() (!!(status&0x10))
#define GetS() (!!(status&0x2000))
#define GetT() (!!(status&0x8000))
#define GetI() ((status&0x0700)>>8)

#else

#define SetC(flag) status[0] = (!!(flag))
#define SetV(flag) status[1] = (!!(flag))
#if SR_IMPLEMENTATION == 1
#define SetZ(flag) \
    if (status_NZ < 0.0) { \
        status_NZ = (flag) ? -0.5 : -1.0; \
    } else { \
        status_NZ = (flag) ? 0.0 : +1.0; \
    }
#define SetN(flag) \
   if (status_NZ == 0.0 || status_NZ == -0.5) { \
        status_NZ = (flag) ? -0.5 : 0.0; \
    } else { \
        status_NZ = (flag) ? -1.0 : +1.0; \
    }
#define SetNZ(operand) status_NZ = operand;
#else
#define SetZ(flag) status[5] = (!!(flag))
#define SetN(flag) status[6] = (!!(flag))
#define SetNZ(operand) status[6] = ((operand) < 0); status[5] = ((operand) == 0)
#endif
#define SetX(flag) status[2] = (!!(flag))
#define SetI(int) intmask = (int)
#define SetS(flag) \
    if (status[3]) ssp = areg[7]; else usp = areg[7]; \
    status[3] = (!!(flag)); \
    if (status[3]) areg[7] = ssp; else areg[7] = usp
#define SetT(flag) status[4] = (!!(flag))

#define SetXC(flag) status[0] = status[2] = (!!(flag))
#define ClrCV() *((int16 *) status) = 0;
#define ClrCVSetNZ(operand) ClrCV(); SetNZ(operand)

#define GetC() status[0]
#define GetV() status[1]
#if SR_IMPLEMENTATION == 1
#define GetZ() (status_NZ == 0.0 || status_NZ == -0.5)
#define GetN() (status_NZ < 0.0)
#else
#define GetZ() status[5]
#define GetN() status[6]
#endif
#define GetX() status[2]
#define GetI() intmask
#define GetS() status[3]
#define GetT() status[4]

#endif

extern uint16   GetSRB(void);
extern uint16   GetSRW(void);
extern void     SetSRB(uint16);
extern void     SetSRW(uint16);


/*
 * Other Register Access
 */
#define GetPC() pc
#define SetPC(addr) pc = addr

#define GetMPCB() GetMemB (pc + 1)
#define GetMPCW() GetMemW (pc)
#define GetMPCL() GetMemL (pc)

#define GetRegB(regno) (int8) reg[regno]
#define GetRegW(regno) (int16) reg[regno]
#define GetRegL(regno) (int32) reg[regno]

#define GetARegW(regno) (int16) areg[regno]
#define GetARegL(regno) (int32) areg[regno]

#ifndef LITTLE_ENDIAN       /* BIG_ENDIAN */
#define SetRegB(regno, value) *(((int8 *) &reg[regno]) + 3) = value
#define SetRegW(regno, value) *(((int16 *) &reg[regno]) + 1) = value
#define SetARegW(regno, value) *(((int16 *) &areg[regno]) + 1) = value
#else                       /* LITTLE_ENDIAN */
#define SetRegB(regno, value) *((int8 *) &reg[regno]) = value
#define SetRegW(regno, value) *((int16 *) &reg[regno]) = value
#define SetARegW(regno, value) *((int16 *) &areg[regno]) = value
#endif
#define SetRegL(regno, value) reg[regno] = value
#define SetARegL(regno, value) areg[regno] = value



/*
 * Addressing modes (target)
 */
/* d, a, ain, aip, mai, dai, aix, imm, (eaw,ear,eac) */
#define CN(address,spec)
#define Cs(address,spec) address = spec;
#define Cd(address,spec) address = spec;
#define Ca(address,spec) address = 8 + spec;
#define Cain(address,spec) address = areg[spec];
#define CaipB(address,spec) if (spec == 7) areg[spec] += 1; address = areg[spec]; areg[spec] += 1;
#define CaipB15(address,spec) areg[spec] += 1; address = areg[spec]; areg[spec] += 1;
#define CaipW(address,spec) address = areg[spec]; areg[spec] += 2;
#define CaipL(address,spec) address = areg[spec]; areg[spec] += 4;
#define CmaiB(address,spec) areg[spec] -= 1; address = areg[spec]; if (spec == 7) areg[spec] -= 1;
#define CmaiB15(address,spec) areg[spec] -= 1; address = areg[spec]; areg[spec] -= 1;
#define CmaiW(address,spec) areg[spec] -= 2; address = areg[spec];
#define CmaiL(address,spec) areg[spec] -= 4; address = areg[spec];
#define Cdai(address,spec) address = areg[spec] + GetMPCW (); pc += 2;
#define Caix(address,spec) { register uint16 mod, regno; regno = spec; mod = GetMPCW(); pc += 2; address = areg[regno] + (int8) mod + ((mod & 0x800) ? GetRegL(mod >> 12) : GetRegW(mod >> 12)); }
#define Ceaw(address, spec) \
switch (spec) {\
case 0x0:\
    address = GetMPCW ();\
    pc += 2;\
    break;\
case 0x1:\
    address = GetMPCL ();\
    pc += 4;\
    break;\
default:\
    ExceptionGroup1(ILLINSTR);\
    address = 0;\
    break;\
}
#define Cear(address,spec) \
{ register uint16 mod;\
switch (spec) {\
case 0x0:\
    address = GetMPCW ();\
    pc += 2;\
    break;\
case 0x1:\
    address = GetMPCL ();\
    pc += 4;\
    break;\
case 0x2:\
    address = GetPC() + GetMPCW ();\
    pc += 2;\
    break;\
case 0x3:\
    mod = GetMPCW ();\
    address = GetPC() + (int8) mod + ((mod & 0x800) ? GetRegL(mod >> 12) : GetRegW(mod >> 12));\
    pc += 2;\
    break;\
default:\
    ExceptionGroup1(ILLINSTR);\
    address = 0;\
    break;\
}}

/*
 * Operand types: B byte, W word, L long, R regno, A address
 */
#define DN(name)
#define DB(name) register int8 name;
#define DW(name) register int16 name;
#define DL(name) register int32 name;
#define DR(name) register uint16 name;
#define DA(name) register int32 name;

/*
 * target macros
 */
#define GRB(regno, name) name = GetRegB (regno);
#define GRW(regno, name) name = GetRegW (regno);
#define GRL(regno, name) name = GetRegL (regno);
#define SRB(regno, name) SetRegB (regno, name);
#define SRW(regno, name) SetRegW (regno, name);
#define SRL(regno, name) SetRegL (regno, name);
#define GMB(address, name) name = GetMemB (address);
#define GMW(address, name) name = GetMemW (address);
#define GML(address, name) name = GetMemL (address);
#define GN(address, name)
#define SMB(address, name) SetMemB (address, name);
#define SMW(address, name) SetMemW (address, name);
#define SML(address, name) SetMemL (address, name);
#define SN(address, name)
#define GCB(dummy, name) name = GetSRB ();
#define GCW(dummy, name) name = GetSRW ();
#define SCB(dummy, name) SetSRB (name);
#define SCW(dummy, name) SetSRW (name);
#define GPW(address, name) \
    name = GetMemB(address);\
    name <<= 8;\
    name |= (uint8) GetMemB(address + 2);
#define GPL(address, name) \
    name = GetMemB(address);\
    name <<= 8;\
    name |= (uint8) GetMemB(address + 2);\
    name <<= 8;\
    name |= (uint8) GetMemB(address + 4);\
    name <<= 8;\
    name |= (uint8) GetMemB(address + 6);
#define SPW(address, name) \
    SetMemB(address, (int8) (name>>8));\
    SetMemB(address+ 2, (int8) name);
#define SPL(address, name) \
    SetMemB(address , (int8)(name>>24));\
    SetMemB(address + 2, (int8)(name>>16));\
    SetMemB(address + 4, (int8)(name>>8));\
    SetMemB(address + 6, (int8) name);

/*
 * source macros
 */
#define SdaiPW(var,spec) {uint32 a; Cdai(a,spec); var = GPW(a,var);}
#define SdaiPL(var,spec) {uint32 a; Cdai(a,spec); var = GPL(a,var);}
#define SNN(var,spec)
#define SNQ(var, dummy) var = (int8) inst;
#define ScB(var,spec) var = GetSRB();
#define ScW(var,spec) var = GetSRW();
#define Ss(var,spec) var = spec;
#define SimmB(var,spec) var = GetMPCB (); pc += 2;
#define SimmW(var,spec) var = GetMPCW (); pc += 2;
#define SimmL(var,spec) var = GetMPCL (); pc += 4;
#define SdB(var,spec) var = GetRegB (spec);
#define SdW(var,spec) var = GetRegW (spec);
#define SdL(var,spec) var = GetRegL (spec);
#define SaW(var,spec) var = GetARegW (spec);
#define SaL(var,spec) var = GetARegL (spec);
#define SainB(var,spec) var = GetMemB (areg[spec]);
#define SainW(var,spec) var = GetMemW (areg[spec]);
#define SainL(var,spec) var = GetMemL (areg[spec]);
#define SainA(var,spec) var = (areg[spec]);
#define SaipB(var,spec) if (spec == 7) areg[spec] += 1; var = GetMemB (areg[spec]);areg[spec] += 1;
#define SaipW(var,spec) var = GetMemW (areg[spec]); areg[spec] += 2;
#define SaipL(var,spec) var = GetMemL (areg[spec]); areg[spec] += 4;
#define SmaiB(var,spec) areg[spec] -= 1; var = GetMemB (areg[spec]); if (spec == 7) areg[spec] -= 1;
#define SmaiW(var,spec) areg[spec] -= 2; var = GetMemW (areg[spec]);
#define SmaiL(var,spec) areg[spec] -= 4; var = GetMemL (areg[spec]);
#define SdaiB(var,spec) var = GetMemB (areg[spec] + GetMPCW ()); pc += 2;
#define SdaiW(var,spec) var = GetMemW (areg[spec] + GetMPCW ()); pc += 2;
#define SdaiL(var,spec) var = GetMemL (areg[spec] + GetMPCW ()); pc += 2;
#define SdaiA(var,spec) var = (areg[spec] + GetMPCW ()); pc += 2;
#define SaixB(var,spec) {register uint16 mod, regno; regno = spec; mod = GetMPCW(); pc += 2; var = GetMemB(areg[regno] + (int8) mod + ((mod & 0x800) ? GetRegL(mod >> 12) : GetRegW(mod >> 12)));};
#define SaixW(var,spec) {register uint16 mod, regno; regno = spec; mod = GetMPCW(); pc += 2; var = GetMemW(areg[regno] + (int8) mod + ((mod & 0x800) ? GetRegL(mod >> 12) : GetRegW(mod >> 12)));};
#define SaixL(var,spec) {register uint16 mod, regno; regno = spec; mod = GetMPCW(); pc += 2; var = GetMemL(areg[regno] + (int8) mod + ((mod & 0x800) ? GetRegL(mod >> 12) : GetRegW(mod >> 12)));};
#define SaixA(var,spec) {register uint16 mod, regno; regno = spec; mod = GetMPCW(); pc += 2; var = (areg[regno] + (int8) mod + ((mod & 0x800) ? GetRegL(mod >> 12) : GetRegW(mod >> 12)));};
#define SearB(var,spec) \
{ register uint16 mod;\
switch (spec) {\
case 0x0:\
    var = GetMemB (GetMPCW ());\
    pc += 2;\
    break;\
case 0x1:\
    var = GetMemB (GetMPCL ());\
    pc += 4;\
    break;\
case 0x2:\
    var = GetMemB (GetPC() + GetMPCW ());\
    pc += 2;\
    break;\
case 0x3:\
    mod = GetMPCW ();\
    var = GetMemB (GetPC() + (int8) mod + ((mod & 0x800) ? GetRegL(mod >> 12) : GetRegW(mod >> 12)));\
    pc += 2;\
    break;\
case 0x4:\
    var = GetMPCB ();\
    pc += 2;\
    break;\
default:\
    ExceptionGroup1(ILLINSTR);\
    var = 0;\
    break;\
}}
#define SearW(var,spec) \
{ register uint16 mod;\
switch (spec) {\
case 0x0:\
    var = GetMemW (GetMPCW ());\
    pc += 2;\
    break;\
case 0x1:\
    var = GetMemW (GetMPCL ());\
    pc += 4;\
    break;\
case 0x2:\
    var = GetMemW (GetPC() + GetMPCW ());\
    pc += 2;\
    break;\
case 0x3:\
    mod = GetMPCW ();\
    var = GetMemW (GetPC() + (int8) mod + ((mod & 0x800) ? GetRegL(mod >> 12) : GetRegW(mod >> 12)));\
    pc += 2;\
    break;\
case 0x4:\
    var = GetMPCW ();\
    pc += 2;\
    break;\
default:\
    ExceptionGroup1(ILLINSTR);\
    var = 0;\
    break;\
}}
#define SearL(var,spec) \
{ register uint16 mod;\
switch (spec) {\
case 0x0:\
    var = GetMemL (GetMPCW ());\
    pc += 2;\
    break;\
case 0x1:\
    var = GetMemL (GetMPCL ());\
    pc += 4;\
    break;\
case 0x2:\
    var = GetMemL (GetPC() + GetMPCW ());\
    pc += 2;\
    break;\
case 0x3:\
    mod = GetMPCW ();\
    var = GetMemL (GetPC() + (int8) mod + ((mod & 0x800) ? GetRegL(mod >> 12) : GetRegW(mod >> 12)));\
    pc += 2;\
    break;\
case 0x4:\
    var = GetMPCL ();\
    pc += 4;\
    break;\
default:\
    ExceptionGroup1(ILLINSTR);\
    var = 0;\
    break;\
}}
#define SearA(var,spec) \
{ register uint16 mod;\
switch (spec) {\
case 0x0:\
    var = GetMPCW ();\
    pc += 2;\
    break;\
case 0x1:\
    var = GetMPCL ();\
    pc += 4;\
    break;\
case 0x2:\
    var = GetPC() + GetMPCW ();\
    pc += 2;\
    break;\
case 0x3:\
    mod = GetMPCW ();\
    var = GetPC() + (int8) mod + ((mod & 0x800) ? GetRegL(mod >> 12) : GetRegW(mod >> 12));\
    pc += 2;\
    break;\
default:\
    ExceptionGroup1(ILLINSTR);\
    var = 0;\
    break;\
}}

/*
 * Condition-Code queries
 */
#define CCt  0xff
#define CCf  0x00
#define CChi (!GetC() && !GetZ())
#define CCls (GetC() || GetZ())
#define CCcc !GetC()
#define CCcs GetC()
#define CCne !GetZ()
#define CCeq GetZ()
#define CCvc !GetV()
#define CCvs GetV()
#define CCpl !GetN()
#define CCmi GetN()
#define CCge !(GetN() ^ GetV())
#define CClt (GetN() ^ GetV())
#define CCgt (!(GetN() ^ GetV()) && !GetZ())
#define CCle ((GetN() ^ GetV()) || GetZ())

/*
 * Oper arguments:
 * Code                     Opcode
 * Op(target,source)        Macro used to execute operation
 * DeclS(name)              Macro to declare source (such as DB, DW or DL, or
                            DN=no source, DA=address, DR=register number)
 * GetS(var,spec)           Macro to fetch source, with addressing mode evaluation.
                            SNN = no fetch
                            SNQ = used in MOVEQ
                            ScB = condition code register
                            ScW = status register word (s-flag not checked)
                            Ss = source equals source specifier (spec1)
                            SaW, SaL = get #<spec1> data register value
                            SdB, SdW, SdL = get data register value
                            SainB, SainW, SainL = (A<spec>)
                            SainA = (LEA only)
                            SaixB, SaixW, SaixL = (d8,A<spec>,X<>)
                            SaixA = (LEA only)
                            SdaiB, SdaiW, SdaiL = d16(A<spec>)
                            SdaiA = (LEA only)
                            SaipB, SaipW, SaipL = (A<spec>)+
                            SmaiB, SmaiW, SmaiL = -(A<spec>)
                            SimmB, SimmW, SimmL = immediate
                            SdaiPW, SdaiPL = used in MOVEP
                            SearB, SearW, SearL = other addressing modes
                            SearA = (LEA only)
 * spec1                    source specifier, usually the data or address register number.
 * DeclT(name)              Macro to declare target (such as DB, DW or DL, or
                            DN=no target, DA=address, DR=register number)
 * DeclEA2(name)            Macro to declare target address (typically DA, DR, or DN)
 * CalcEA2(address,spec)    Macro to calculate address, with addressing mode evaluation.
                            CN = no address
                            Cs = equals target specifier (spec2)
                            Cd = equals target specifier (spec2) (data register number)
                            Ca = equals target specifier (spec2) + 8 (address register number)
                            Cain = (A<spec>)
                            Caix = (d8,A<spec>,X<>)
                            Cdai = d16(A<spec>)
                            CaipB, CaipW, CaipL = (A<spec>)+,
                            CaipB15 = stack pointer only
                            CmaiB, CmaiW, CmaiL = -(A<spec>)
                            CmaiB15 = stack pointer only
                            Ceaw = other addressing modes (if target is modified)
                            Cear = other addressing modes (if target is not modified)
 * spec2                    target specifier, usually the data or address register number.
 * GetEA2(address,name)     Macro to get target value
                            GCB = get condition code
                            GCW = get status (s-flag not checked)
                            GN = no read
                            GMB, GMW, GML = read from memory
                            GPW, GPL = (MOVEP)
                            GRB, GRW, GRL = read from register
 * SetEA2(address,name)     Macro to set target value
                            SCB = set condition code
                            SCW = set status (s-flag not checked)
                            SN = no write
                            SMB, SMW, SML = write to memory
                            SPW, SPL = (MOVEP)
                            SRB, SRW, SRL = write to register
 */

#if (!defined(DEBUG) || !defined(INTERNALTRACE))
#define Oper(Code, Op, DeclS, GetS, spec1, DeclT, DeclEA2, CalcEA2, spec2, GetEA2, SetEA2)\
void            Code(void)\
{\
    DeclS (source)\
    DeclT (target)\
    DeclEA2 (address2)\
    GetS (source, spec1)\
    CalcEA2 (address2, spec2)\
    GetEA2 (address2, target)\
    Op (target, source)\
    SetEA2 (address2, target)\
}
#else
/* quick'n dirty debug hack */
#undef DN
#define DN(name) int name;
#define Oper(Code, Op, DeclS, GetS, spec1, DeclT, DeclEA2, CalcEA2, spec2, GetEA2, SetEA2)\
void            Code(void)\
{\
    DeclS (source)\
    DeclT (target)\
    DeclEA2 (address2)\
    source = 0; \
    target = 0; \
    address2 = 0; \
    GetS (source, spec1)\
    CalcEA2 (address2, spec2)\
    GetEA2 (address2, target)\
    traceback[tbi].address = address2; \
    traceback[tbi].source = source; \
    traceback[tbi].target = target; \
    Op (target, source)\
    traceback[tbi].result = target; \
    SetEA2 (address2, target)\
}
#endif
#endif
