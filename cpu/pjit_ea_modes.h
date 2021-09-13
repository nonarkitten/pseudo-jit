/*
 * file: pjit_ea_modes.h
 * author: Renee Cousins
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 *
 * This are all macros for the various effective addressing modes
 * These are combined with the alu operations to form most of the
 * various 68k instructions.
 *
 */

#ifndef OP68KH
#define OP68KH

/*
 * Other Register Access
 */
#define GetPC() pc
#define SetPC(addr) pc = addr

#define GetMPCB() GetMemB (pc + 1)
#define GetMPCW() GetMemW (pc)
#define GetMPCL() GetMemL (pc)

#define GetRegB(regno) (int8) cpu->d[regno]
#define GetRegW(regno) (int16) cpu->d[regno]
#define GetRegL(regno) (int32) cpu->d[regno]

#define GetARegW(regno) (int16) cpu->a[regno]
#define GetARegL(regno) (int32) cpu->a[regno]

#define SetRegB(regno, value)  *((int8 *) &cpu->d[regno])=value
#define SetRegW(regno, value)  *((int16 *)&cpu->d[regno])=value
#define SetARegW(regno, value) *((int16 *)&cpu->a[regno])=value
#define SetRegL(regno, value)  cpu->d[regno]=value
#define SetARegL(regno, value) cpu->a[regno]=value

/*
 * Addressing modes (target)
 */
/* d, a, ain, aip, mai, dai, aix, imm, (eaw,ear,eac) */
#define CN(address,spec)
#define Cs(address,spec) address = spec;
#define Cd(address,spec) address = spec;
#define Ca(address,spec) address = 8 + spec;
#define Cain(address,spec) address = cpu->a[spec];
#define CaipB(address,spec) if (spec == 7) cpu->a[spec] += 1; address = cpu->a[spec]; cpu->a[spec] += 1;
#define CaipB15(address,spec) cpu->a[spec] += 1; address = cpu->a[spec]; cpu->a[spec] += 1;
#define CaipW(address,spec) address = cpu->a[spec]; cpu->a[spec] += 2;
#define CaipL(address,spec) address = cpu->a[spec]; cpu->a[spec] += 4;
#define CmaiB(address,spec) cpu->a[spec] -= 1; address = cpu->a[spec]; if (spec == 7) cpu->a[spec] -= 1;
#define CmaiB15(address,spec) cpu->a[spec] -= 1; address = cpu->a[spec]; cpu->a[spec] -= 1;
#define CmaiW(address,spec) cpu->a[spec] -= 2; address = cpu->a[spec];
#define CmaiL(address,spec) cpu->a[spec] -= 4; address = cpu->a[spec];
#define Cdai(address,spec) address = cpu->a[spec] + GetMPCW (); pc += 2;
#define Caix(address,spec) { register uint16 mod, regno; regno = spec; mod = GetMPCW(); pc += 2; address = cpu->a[regno] + (int8) mod + ((mod & 0x800) ? GetRegL(mod >> 12) : GetRegW(mod >> 12)); }
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
#define Cear(address,spec) {\
	register uint16 mod;\
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
#define SainB(var,spec) var = GetMemB (cpu->a[spec]);
#define SainW(var,spec) var = GetMemW (cpu->a[spec]);
#define SainL(var,spec) var = GetMemL (cpu->a[spec]);
#define SainA(var,spec) var = (cpu->a[spec]);
#define SaipB(var,spec) if (spec == 7) cpu->a[spec] += 1; var = GetMemB (cpu->a[spec]);cpu->a[spec] += 1;
#define SaipW(var,spec) var = GetMemW (cpu->a[spec]); cpu->a[spec] += 2;
#define SaipL(var,spec) var = GetMemL (cpu->a[spec]); cpu->a[spec] += 4;
#define SmaiB(var,spec) cpu->a[spec] -= 1; var = GetMemB (cpu->a[spec]); if (spec == 7) cpu->a[spec] -= 1;
#define SmaiW(var,spec) cpu->a[spec] -= 2; var = GetMemW (cpu->a[spec]);
#define SmaiL(var,spec) cpu->a[spec] -= 4; var = GetMemL (cpu->a[spec]);
#define SdaiB(var,spec) var = GetMemB (cpu->a[spec] + GetMPCW ()); pc += 2;
#define SdaiW(var,spec) var = GetMemW (cpu->a[spec] + GetMPCW ()); pc += 2;
#define SdaiL(var,spec) var = GetMemL (cpu->a[spec] + GetMPCW ()); pc += 2;
#define SdaiA(var,spec) var = (cpu->a[spec] + GetMPCW ()); pc += 2;
#define SaixB(var,spec) {register uint16 mod, regno; regno = spec; mod = GetMPCW(); pc += 2; var = GetMemB(cpu->a[regno] + (int8) mod + ((mod & 0x800) ? GetRegL(mod >> 12) : GetRegW(mod >> 12)));};
#define SaixW(var,spec) {register uint16 mod, regno; regno = spec; mod = GetMPCW(); pc += 2; var = GetMemW(cpu->a[regno] + (int8) mod + ((mod & 0x800) ? GetRegL(mod >> 12) : GetRegW(mod >> 12)));};
#define SaixL(var,spec) {register uint16 mod, regno; regno = spec; mod = GetMPCW(); pc += 2; var = GetMemL(cpu->a[regno] + (int8) mod + ((mod & 0x800) ? GetRegL(mod >> 12) : GetRegW(mod >> 12)));};
#define SaixA(var,spec) {register uint16 mod, regno; regno = spec; mod = GetMPCW(); pc += 2; var = (cpu->a[regno] + (int8) mod + ((mod & 0x800) ? GetRegL(mod >> 12) : GetRegW(mod >> 12)));};
#define SearB(var,spec) {\
	register uint16 mod;\
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
#define SearW(var,spec) {\
	register uint16 mod;\
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
#define SearL(var,spec) {\
	register uint16 mod;\
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
#define SearA(var,spec) {\
	register uint16 mod;\
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

#endif
