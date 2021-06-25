/*
 * Castaway
 *  (C) 1994 - 2002 Martin Doering, Joachim Hoenig
 *
 * $File$ - 68k arithmetic instructions
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 *
 * revision history
 *  23.05.2002  JH  FAST1.0.1 code import: KR -> ANSI, restructuring
 *  12.06.2002  JH  Correct bus error/address error exception stack frame
 *  13.06.2002  JH  Merged in Martin's BCD implementation (untested),
 *                  completed jump table. STOP is now the only
 *                  unimplemented instruction.
 *  08.10.2002  JH  Fixed Z-Flag for ADD.B 0x80+0x80 and Add.W 0x8000+0x8000.
 *  30.10.2002  JH  Replaced "% 8" with "& 7". Helps lesser compilers to generate faster code.
 */
#ifndef PROTOH
static char     sccsid[] = "$Id: op68karith.c,v 1.6 2002/10/30 16:23:06 jhoenig Exp $";
#include "68000.h"
#include "op68k.h"

/*
 * Opfuncs.
 */

#define DoCmpB(target,source) \
    {register int32 tgt;\
    tgt = target - source;\
    SetC ((uint32) source > (uint32) target);\
    SetV ((tgt < -128) || (tgt > 127));\
    SetNZ ((int8) tgt);};
#define DoCmpW(target,source) \
    {register int32 tgt;\
    tgt = target - source;\
    SetC ((uint32) source > (uint32) target);\
    SetV ((tgt < -32768) || (tgt > 32767));\
    SetNZ ((int16) tgt);};
#define DoCmpL(target,source) \
    /* Overflow precondition: source has different sign */\
    SetV ((target < 0) != (source < 0));\
    SetC ((uint32) source > (uint32) target);\
    target -= source;\
    /* target and source now have same sign: overflow! */\
    SetV (GetV() && (target<0) == (source<0));\
    SetNZ(target);

#define DoCmpa(target,source) DoCmpL(target,source)
#define DoCmpm(target,source) \
    /* Overflow precondition: source has different sign */\
    SetV ((target < 0) != (source < 0));\
    SetC ((uint32) source > (uint32) target);\
    target -= source;\
    /* target and source now have same sign: overflow! */\
    SetV (GetV() && (target<0) == (source<0));\
    SetNZ(target);

#define DoDivs(target,source) \
    if (source == 0) {\
    ExceptionGroup2(DIVZ);\
    } else {\
        int16  quot, rest;\
    if ((target / source) > 32767 || (target / source) < -32768) {\
        SetV (1);\
        SetC (0);\
    } else {\
        rest = target % source;\
        quot = target / source;\
        target = (rest << 16) | (uint16) quot;\
        ClrCVSetNZ ((int16) target);\
    }\
    }

#define DoDivu(target,source) \
    if (source == 0) {\
    ExceptionGroup2(DIVZ);\
    } else {\
        uint16  quot, rest;\
    if (((uint32) target / (uint16) source) & 0xffff0000) {\
        SetV (1);\
        SetC (0);\
    } else {\
        rest = (uint32) target % (uint16) source;\
        quot = (uint32) target / (uint16) source;\
        target = (rest << 16) | quot;\
        ClrCVSetNZ ((int16) target);\
    }\
    }

#define DoMuls(target,source) \
    target = ((int16) target * source);\
    ClrCVSetNZ (target);

#define DoMulu(target,source) \
    target = ((uint16) target * (uint16) source);\
    ClrCVSetNZ (target);

#define DoSuba(target,source) target -= source;

#define DoSubx(target,source) \
    /* Overflow precondition: source has different sign */\
    SetV ((target < 0) != (source < 0));\
    if (GetX ()) {\
    SetXC ((uint32) source >= (uint32) target);\
    target -= source;\
    target -= 1;\
    } else {\
    SetXC ((uint32) source > (uint32) target);\
    target -= source;\
    }\
    /* target and source now have same sign: overflow! */\
    SetV (GetV() && (target<0) == (source<0));\
    SetNZ (target);

/* BCD all totally untested (MAD) */

#define DoNBCD(target, source) \
    { register int32 tgt; \
    tgt = - (source & 0xF) - GetX(); \
    if (tgt > 9) \
        tgt -= 6;  \
    tgt -= (source & 0xF0); \
    SetC((tgt & 0x1F0) > 0x90); \
    if (GetC()) \
        tgt -= 0x60; \
    SetNZ(tgt); \
    target = tgt; \
    };

#define DoABCD(target, source) \
    { register int32 tgt; \
    tgt = (source & 0xF) + (target & 0xF) + GetX(); \
    if(tgt >= 0xA) \
        tgt += 0x6; \
    tgt += (source & 0xF0) + (target & 0xF0); \
    if(tgt >= 0xA0) \
        tgt += 0x60; \
    SetXC (tgt & 0xFF00); \
    tgt &= 0xFF; \
    SetZ(tgt == 0); \
    target = tgt; \
    };

#define DoSBCD(target, source) \
    { register int32 tgt, hi; \
    tgt = (target & 0xF) - (source & 0xF) - GetX(); \
    hi = (target & 0xF0) - (source & 0xF0); \
    if(tgt < 0) { \
        tgt += 10; \
        hi -= 0x10; \
    } \
    if(hi < 0) { \
        hi -= 0x60; \
    } \
    tgt += hi; \
    SetXC (tgt & 0xFF00); \
    tgt &= 0xFF; \
    SetZ(tgt == 0); \
    target = tgt; \
    };

#endif

Oper (Opc0c0, DoMulu, DW, SdW, (inst & 7), DL, DR, Cd, 0, GRL, SRL)
Oper (Opc0d0, DoMulu, DW, SainW, (inst & 7), DL, DR, Cd, 0, GRL, SRL)
Oper (Opc0d8, DoMulu, DW, SaipW, (inst & 7), DL, DR, Cd, 0, GRL, SRL)
Oper (Opc0e0, DoMulu, DW, SmaiW, (inst & 7), DL, DR, Cd, 0, GRL, SRL)
Oper (Opc0e8, DoMulu, DW, SdaiW, (inst & 7), DL, DR, Cd, 0, GRL, SRL)
Oper (Opc0f0, DoMulu, DW, SaixW, (inst & 7), DL, DR, Cd, 0, GRL, SRL)
Oper (Opc0f8, DoMulu, DW, SearW, (inst & 7), DL, DR, Cd, 0, GRL, SRL)

Oper (Opc1c0, DoMuls, DW, SdW, (inst & 7), DL, DR, Cd, 0, GRL, SRL)
Oper (Opc1d0, DoMuls, DW, SainW, (inst & 7), DL, DR, Cd, 0, GRL, SRL)
Oper (Opc1d8, DoMuls, DW, SaipW, (inst & 7), DL, DR, Cd, 0, GRL, SRL)
Oper (Opc1e0, DoMuls, DW, SmaiW, (inst & 7), DL, DR, Cd, 0, GRL, SRL)
Oper (Opc1e8, DoMuls, DW, SdaiW, (inst & 7), DL, DR, Cd, 0, GRL, SRL)
Oper (Opc1f0, DoMuls, DW, SaixW, (inst & 7), DL, DR, Cd, 0, GRL, SRL)
Oper (Opc1f8, DoMuls, DW, SearW, (inst & 7), DL, DR, Cd, 0, GRL, SRL)

Oper (Opc2c0, DoMulu, DW, SdW, (inst & 7), DL, DR, Cd, 1, GRL, SRL)
Oper (Opc2d0, DoMulu, DW, SainW, (inst & 7), DL, DR, Cd, 1, GRL, SRL)
Oper (Opc2d8, DoMulu, DW, SaipW, (inst & 7), DL, DR, Cd, 1, GRL, SRL)
Oper (Opc2e0, DoMulu, DW, SmaiW, (inst & 7), DL, DR, Cd, 1, GRL, SRL)
Oper (Opc2e8, DoMulu, DW, SdaiW, (inst & 7), DL, DR, Cd, 1, GRL, SRL)
Oper (Opc2f0, DoMulu, DW, SaixW, (inst & 7), DL, DR, Cd, 1, GRL, SRL)
Oper (Opc2f8, DoMulu, DW, SearW, (inst & 7), DL, DR, Cd, 1, GRL, SRL)

Oper (Opc3c0, DoMuls, DW, SdW, (inst & 7), DL, DR, Cd, 1, GRL, SRL)
Oper (Opc3d0, DoMuls, DW, SainW, (inst & 7), DL, DR, Cd, 1, GRL, SRL)
Oper (Opc3d8, DoMuls, DW, SaipW, (inst & 7), DL, DR, Cd, 1, GRL, SRL)
Oper (Opc3e0, DoMuls, DW, SmaiW, (inst & 7), DL, DR, Cd, 1, GRL, SRL)
Oper (Opc3e8, DoMuls, DW, SdaiW, (inst & 7), DL, DR, Cd, 1, GRL, SRL)
Oper (Opc3f0, DoMuls, DW, SaixW, (inst & 7), DL, DR, Cd, 1, GRL, SRL)
Oper (Opc3f8, DoMuls, DW, SearW, (inst & 7), DL, DR, Cd, 1, GRL, SRL)

Oper (Opc4c0, DoMulu, DW, SdW, (inst & 7), DL, DR, Cd, 2, GRL, SRL)
Oper (Opc4d0, DoMulu, DW, SainW, (inst & 7), DL, DR, Cd, 2, GRL, SRL)
Oper (Opc4d8, DoMulu, DW, SaipW, (inst & 7), DL, DR, Cd, 2, GRL, SRL)
Oper (Opc4e0, DoMulu, DW, SmaiW, (inst & 7), DL, DR, Cd, 2, GRL, SRL)
Oper (Opc4e8, DoMulu, DW, SdaiW, (inst & 7), DL, DR, Cd, 2, GRL, SRL)
Oper (Opc4f0, DoMulu, DW, SaixW, (inst & 7), DL, DR, Cd, 2, GRL, SRL)
Oper (Opc4f8, DoMulu, DW, SearW, (inst & 7), DL, DR, Cd, 2, GRL, SRL)

Oper (Opc5c0, DoMuls, DW, SdW, (inst & 7), DL, DR, Cd, 2, GRL, SRL)
Oper (Opc5d0, DoMuls, DW, SainW, (inst & 7), DL, DR, Cd, 2, GRL, SRL)
Oper (Opc5d8, DoMuls, DW, SaipW, (inst & 7), DL, DR, Cd, 2, GRL, SRL)
Oper (Opc5e0, DoMuls, DW, SmaiW, (inst & 7), DL, DR, Cd, 2, GRL, SRL)
Oper (Opc5e8, DoMuls, DW, SdaiW, (inst & 7), DL, DR, Cd, 2, GRL, SRL)
Oper (Opc5f0, DoMuls, DW, SaixW, (inst & 7), DL, DR, Cd, 2, GRL, SRL)
Oper (Opc5f8, DoMuls, DW, SearW, (inst & 7), DL, DR, Cd, 2, GRL, SRL)

Oper (Opc6c0, DoMulu, DW, SdW, (inst & 7), DL, DR, Cd, 3, GRL, SRL)
Oper (Opc6d0, DoMulu, DW, SainW, (inst & 7), DL, DR, Cd, 3, GRL, SRL)
Oper (Opc6d8, DoMulu, DW, SaipW, (inst & 7), DL, DR, Cd, 3, GRL, SRL)
Oper (Opc6e0, DoMulu, DW, SmaiW, (inst & 7), DL, DR, Cd, 3, GRL, SRL)
Oper (Opc6e8, DoMulu, DW, SdaiW, (inst & 7), DL, DR, Cd, 3, GRL, SRL)
Oper (Opc6f0, DoMulu, DW, SaixW, (inst & 7), DL, DR, Cd, 3, GRL, SRL)
Oper (Opc6f8, DoMulu, DW, SearW, (inst & 7), DL, DR, Cd, 3, GRL, SRL)

Oper (Opc7c0, DoMuls, DW, SdW, (inst & 7), DL, DR, Cd, 3, GRL, SRL)
Oper (Opc7d0, DoMuls, DW, SainW, (inst & 7), DL, DR, Cd, 3, GRL, SRL)
Oper (Opc7d8, DoMuls, DW, SaipW, (inst & 7), DL, DR, Cd, 3, GRL, SRL)
Oper (Opc7e0, DoMuls, DW, SmaiW, (inst & 7), DL, DR, Cd, 3, GRL, SRL)
Oper (Opc7e8, DoMuls, DW, SdaiW, (inst & 7), DL, DR, Cd, 3, GRL, SRL)
Oper (Opc7f0, DoMuls, DW, SaixW, (inst & 7), DL, DR, Cd, 3, GRL, SRL)
Oper (Opc7f8, DoMuls, DW, SearW, (inst & 7), DL, DR, Cd, 3, GRL, SRL)

Oper (Opc8c0, DoMulu, DW, SdW, (inst & 7), DL, DR, Cd, 4, GRL, SRL)
Oper (Opc8d0, DoMulu, DW, SainW, (inst & 7), DL, DR, Cd, 4, GRL, SRL)
Oper (Opc8d8, DoMulu, DW, SaipW, (inst & 7), DL, DR, Cd, 4, GRL, SRL)
Oper (Opc8e0, DoMulu, DW, SmaiW, (inst & 7), DL, DR, Cd, 4, GRL, SRL)
Oper (Opc8e8, DoMulu, DW, SdaiW, (inst & 7), DL, DR, Cd, 4, GRL, SRL)
Oper (Opc8f0, DoMulu, DW, SaixW, (inst & 7), DL, DR, Cd, 4, GRL, SRL)
Oper (Opc8f8, DoMulu, DW, SearW, (inst & 7), DL, DR, Cd, 4, GRL, SRL)

Oper (Opc9c0, DoMuls, DW, SdW, (inst & 7), DL, DR, Cd, 4, GRL, SRL)
Oper (Opc9d0, DoMuls, DW, SainW, (inst & 7), DL, DR, Cd, 4, GRL, SRL)
Oper (Opc9d8, DoMuls, DW, SaipW, (inst & 7), DL, DR, Cd, 4, GRL, SRL)
Oper (Opc9e0, DoMuls, DW, SmaiW, (inst & 7), DL, DR, Cd, 4, GRL, SRL)
Oper (Opc9e8, DoMuls, DW, SdaiW, (inst & 7), DL, DR, Cd, 4, GRL, SRL)
Oper (Opc9f0, DoMuls, DW, SaixW, (inst & 7), DL, DR, Cd, 4, GRL, SRL)
Oper (Opc9f8, DoMuls, DW, SearW, (inst & 7), DL, DR, Cd, 4, GRL, SRL)

Oper (Opcac0, DoMulu, DW, SdW, (inst & 7), DL, DR, Cd, 5, GRL, SRL)
Oper (Opcad0, DoMulu, DW, SainW, (inst & 7), DL, DR, Cd, 5, GRL, SRL)
Oper (Opcad8, DoMulu, DW, SaipW, (inst & 7), DL, DR, Cd, 5, GRL, SRL)
Oper (Opcae0, DoMulu, DW, SmaiW, (inst & 7), DL, DR, Cd, 5, GRL, SRL)
Oper (Opcae8, DoMulu, DW, SdaiW, (inst & 7), DL, DR, Cd, 5, GRL, SRL)
Oper (Opcaf0, DoMulu, DW, SaixW, (inst & 7), DL, DR, Cd, 5, GRL, SRL)
Oper (Opcaf8, DoMulu, DW, SearW, (inst & 7), DL, DR, Cd, 5, GRL, SRL)

Oper (Opcbc0, DoMuls, DW, SdW, (inst & 7), DL, DR, Cd, 5, GRL, SRL)
Oper (Opcbd0, DoMuls, DW, SainW, (inst & 7), DL, DR, Cd, 5, GRL, SRL)
Oper (Opcbd8, DoMuls, DW, SaipW, (inst & 7), DL, DR, Cd, 5, GRL, SRL)
Oper (Opcbe0, DoMuls, DW, SmaiW, (inst & 7), DL, DR, Cd, 5, GRL, SRL)
Oper (Opcbe8, DoMuls, DW, SdaiW, (inst & 7), DL, DR, Cd, 5, GRL, SRL)
Oper (Opcbf0, DoMuls, DW, SaixW, (inst & 7), DL, DR, Cd, 5, GRL, SRL)
Oper (Opcbf8, DoMuls, DW, SearW, (inst & 7), DL, DR, Cd, 5, GRL, SRL)

Oper (Opccc0, DoMulu, DW, SdW, (inst & 7), DL, DR, Cd, 6, GRL, SRL)
Oper (Opccd0, DoMulu, DW, SainW, (inst & 7), DL, DR, Cd, 6, GRL, SRL)
Oper (Opccd8, DoMulu, DW, SaipW, (inst & 7), DL, DR, Cd, 6, GRL, SRL)
Oper (Opcce0, DoMulu, DW, SmaiW, (inst & 7), DL, DR, Cd, 6, GRL, SRL)
Oper (Opcce8, DoMulu, DW, SdaiW, (inst & 7), DL, DR, Cd, 6, GRL, SRL)
Oper (Opccf0, DoMulu, DW, SaixW, (inst & 7), DL, DR, Cd, 6, GRL, SRL)
Oper (Opccf8, DoMulu, DW, SearW, (inst & 7), DL, DR, Cd, 6, GRL, SRL)

Oper (Opcdc0, DoMuls, DW, SdW, (inst & 7), DL, DR, Cd, 6, GRL, SRL)
Oper (Opcdd0, DoMuls, DW, SainW, (inst & 7), DL, DR, Cd, 6, GRL, SRL)
Oper (Opcdd8, DoMuls, DW, SaipW, (inst & 7), DL, DR, Cd, 6, GRL, SRL)
Oper (Opcde0, DoMuls, DW, SmaiW, (inst & 7), DL, DR, Cd, 6, GRL, SRL)
Oper (Opcde8, DoMuls, DW, SdaiW, (inst & 7), DL, DR, Cd, 6, GRL, SRL)
Oper (Opcdf0, DoMuls, DW, SaixW, (inst & 7), DL, DR, Cd, 6, GRL, SRL)
Oper (Opcdf8, DoMuls, DW, SearW, (inst & 7), DL, DR, Cd, 6, GRL, SRL)

Oper (Opcec0, DoMulu, DW, SdW, (inst & 7), DL, DR, Cd, 7, GRL, SRL)
Oper (Opced0, DoMulu, DW, SainW, (inst & 7), DL, DR, Cd, 7, GRL, SRL)
Oper (Opced8, DoMulu, DW, SaipW, (inst & 7), DL, DR, Cd, 7, GRL, SRL)
Oper (Opcee0, DoMulu, DW, SmaiW, (inst & 7), DL, DR, Cd, 7, GRL, SRL)
Oper (Opcee8, DoMulu, DW, SdaiW, (inst & 7), DL, DR, Cd, 7, GRL, SRL)
Oper (Opcef0, DoMulu, DW, SaixW, (inst & 7), DL, DR, Cd, 7, GRL, SRL)
Oper (Opcef8, DoMulu, DW, SearW, (inst & 7), DL, DR, Cd, 7, GRL, SRL)

Oper (Opcfc0, DoMuls, DW, SdW, (inst & 7), DL, DR, Cd, 7, GRL, SRL)
Oper (Opcfd0, DoMuls, DW, SainW, (inst & 7), DL, DR, Cd, 7, GRL, SRL)
Oper (Opcfd8, DoMuls, DW, SaipW, (inst & 7), DL, DR, Cd, 7, GRL, SRL)
Oper (Opcfe0, DoMuls, DW, SmaiW, (inst & 7), DL, DR, Cd, 7, GRL, SRL)
Oper (Opcfe8, DoMuls, DW, SdaiW, (inst & 7), DL, DR, Cd, 7, GRL, SRL)
Oper (Opcff0, DoMuls, DW, SaixW, (inst & 7), DL, DR, Cd, 7, GRL, SRL)
Oper (Opcff8, DoMuls, DW, SearW, (inst & 7), DL, DR, Cd, 7, GRL, SRL)

Oper (Opb000, DoCmpB, DB, SdB, (inst & 7), DB, DR, Cd, 0, GRB, SN)
Oper (Opb010, DoCmpB, DB, SainB, (inst & 7), DB, DR, Cd, 0, GRB, SN)
Oper (Opb018, DoCmpB, DB, SaipB, (inst & 7), DB, DR, Cd, 0, GRB, SN)
Oper (Opb020, DoCmpB, DB, SmaiB, (inst & 7), DB, DR, Cd, 0, GRB, SN)
Oper (Opb028, DoCmpB, DB, SdaiB, (inst & 7), DB, DR, Cd, 0, GRB, SN)
Oper (Opb030, DoCmpB, DB, SaixB, (inst & 7), DB, DR, Cd, 0, GRB, SN)
Oper (Opb038, DoCmpB, DB, SearB, (inst & 7), DB, DR, Cd, 0, GRB, SN)
Oper (Opb040, DoCmpW, DW, SdW, (inst & 7), DW, DR, Cd, 0, GRW, SN)
Oper (Opb048, DoCmpW, DW, SaW, (inst & 7), DW, DR, Cd, 0, GRW, SN)
Oper (Opb050, DoCmpW, DW, SainW, (inst & 7), DW, DR, Cd, 0, GRW, SN)
Oper (Opb058, DoCmpW, DW, SaipW, (inst & 7), DW, DR, Cd, 0, GRW, SN)
Oper (Opb060, DoCmpW, DW, SmaiW, (inst & 7), DW, DR, Cd, 0, GRW, SN)
Oper (Opb068, DoCmpW, DW, SdaiW, (inst & 7), DW, DR, Cd, 0, GRW, SN)
Oper (Opb070, DoCmpW, DW, SaixW, (inst & 7), DW, DR, Cd, 0, GRW, SN)
Oper (Opb078, DoCmpW, DW, SearW, (inst & 7), DW, DR, Cd, 0, GRW, SN)
Oper (Opb080, DoCmpL, DL, SdL, (inst & 7), DL, DR, Cd, 0, GRL, SN)
Oper (Opb088, DoCmpL, DL, SaL, (inst & 7), DL, DR, Cd, 0, GRL, SN)
Oper (Opb090, DoCmpL, DL, SainL, (inst & 7), DL, DR, Cd, 0, GRL, SN)
Oper (Opb098, DoCmpL, DL, SaipL, (inst & 7), DL, DR, Cd, 0, GRL, SN)
Oper (Opb0a0, DoCmpL, DL, SmaiL, (inst & 7), DL, DR, Cd, 0, GRL, SN)
Oper (Opb0a8, DoCmpL, DL, SdaiL, (inst & 7), DL, DR, Cd, 0, GRL, SN)
Oper (Opb0b0, DoCmpL, DL, SaixL, (inst & 7), DL, DR, Cd, 0, GRL, SN)
Oper (Opb0b8, DoCmpL, DL, SearL, (inst & 7), DL, DR, Cd, 0, GRL, SN)
Oper (Opb0c0, DoCmpa, DL, SdW, (inst & 7), DL, DR, Ca, 0, GRL, SN)
Oper (Opb0c8, DoCmpa, DL, SaW, (inst & 7), DL, DR, Ca, 0, GRL, SN)
Oper (Opb0d0, DoCmpa, DL, SainW, (inst & 7), DL, DR, Ca, 0, GRL, SN)
Oper (Opb0d8, DoCmpa, DL, SaipW, (inst & 7), DL, DR, Ca, 0, GRL, SN)
Oper (Opb0e0, DoCmpa, DL, SmaiW, (inst & 7), DL, DR, Ca, 0, GRL, SN)
Oper (Opb0e8, DoCmpa, DL, SdaiW, (inst & 7), DL, DR, Ca, 0, GRL, SN)
Oper (Opb0f0, DoCmpa, DL, SaixW, (inst & 7), DL, DR, Ca, 0, GRL, SN)
Oper (Opb0f8, DoCmpa, DL, SearW, (inst & 7), DL, DR, Ca, 0, GRL, SN)

Oper (Opb108, DoCmpm, DB, SaipB, (inst & 7), DB, DA, CaipB, 0, GMB, SN)
Oper (Opb148, DoCmpm, DW, SaipW, (inst & 7), DW, DA, CaipW, 0, GMW, SN)
Oper (Opb188, DoCmpm, DL, SaipL, (inst & 7), DL, DA, CaipL, 0, GML, SN)
Oper (Opb1c0, DoCmpa, DL, SdL, (inst & 7), DL, DR, Ca, 0, GRL, SN)
Oper (Opb1c8, DoCmpa, DL, SaL, (inst & 7), DL, DR, Ca, 0, GRL, SN)
Oper (Opb1d0, DoCmpa, DL, SainL, (inst & 7), DL, DR, Ca, 0, GRL, SN)
Oper (Opb1d8, DoCmpa, DL, SaipL, (inst & 7), DL, DR, Ca, 0, GRL, SN)
Oper (Opb1e0, DoCmpa, DL, SmaiL, (inst & 7), DL, DR, Ca, 0, GRL, SN)
Oper (Opb1e8, DoCmpa, DL, SdaiL, (inst & 7), DL, DR, Ca, 0, GRL, SN)
Oper (Opb1f0, DoCmpa, DL, SaixL, (inst & 7), DL, DR, Ca, 0, GRL, SN)
Oper (Opb1f8, DoCmpa, DL, SearL, (inst & 7), DL, DR, Ca, 0, GRL, SN)

Oper (Opb200, DoCmpB, DB, SdB, (inst & 7), DB, DR, Cd, 1, GRB, SN)
Oper (Opb210, DoCmpB, DB, SainB, (inst & 7), DB, DR, Cd, 1, GRB, SN)
Oper (Opb218, DoCmpB, DB, SaipB, (inst & 7), DB, DR, Cd, 1, GRB, SN)
Oper (Opb220, DoCmpB, DB, SmaiB, (inst & 7), DB, DR, Cd, 1, GRB, SN)
Oper (Opb228, DoCmpB, DB, SdaiB, (inst & 7), DB, DR, Cd, 1, GRB, SN)
Oper (Opb230, DoCmpB, DB, SaixB, (inst & 7), DB, DR, Cd, 1, GRB, SN)
Oper (Opb238, DoCmpB, DB, SearB, (inst & 7), DB, DR, Cd, 1, GRB, SN)
Oper (Opb240, DoCmpW, DW, SdW, (inst & 7), DW, DR, Cd, 1, GRW, SN)
Oper (Opb248, DoCmpW, DW, SaW, (inst & 7), DW, DR, Cd, 1, GRW, SN)
Oper (Opb250, DoCmpW, DW, SainW, (inst & 7), DW, DR, Cd, 1, GRW, SN)
Oper (Opb258, DoCmpW, DW, SaipW, (inst & 7), DW, DR, Cd, 1, GRW, SN)
Oper (Opb260, DoCmpW, DW, SmaiW, (inst & 7), DW, DR, Cd, 1, GRW, SN)
Oper (Opb268, DoCmpW, DW, SdaiW, (inst & 7), DW, DR, Cd, 1, GRW, SN)
Oper (Opb270, DoCmpW, DW, SaixW, (inst & 7), DW, DR, Cd, 1, GRW, SN)
Oper (Opb278, DoCmpW, DW, SearW, (inst & 7), DW, DR, Cd, 1, GRW, SN)
Oper (Opb280, DoCmpL, DL, SdL, (inst & 7), DL, DR, Cd, 1, GRL, SN)
Oper (Opb288, DoCmpL, DL, SaL, (inst & 7), DL, DR, Cd, 1, GRL, SN)
Oper (Opb290, DoCmpL, DL, SainL, (inst & 7), DL, DR, Cd, 1, GRL, SN)
Oper (Opb298, DoCmpL, DL, SaipL, (inst & 7), DL, DR, Cd, 1, GRL, SN)
Oper (Opb2a0, DoCmpL, DL, SmaiL, (inst & 7), DL, DR, Cd, 1, GRL, SN)
Oper (Opb2a8, DoCmpL, DL, SdaiL, (inst & 7), DL, DR, Cd, 1, GRL, SN)
Oper (Opb2b0, DoCmpL, DL, SaixL, (inst & 7), DL, DR, Cd, 1, GRL, SN)
Oper (Opb2b8, DoCmpL, DL, SearL, (inst & 7), DL, DR, Cd, 1, GRL, SN)
Oper (Opb2c0, DoCmpa, DL, SdW, (inst & 7), DL, DR, Ca, 1, GRL, SN)
Oper (Opb2c8, DoCmpa, DL, SaW, (inst & 7), DL, DR, Ca, 1, GRL, SN)
Oper (Opb2d0, DoCmpa, DL, SainW, (inst & 7), DL, DR, Ca, 1, GRL, SN)
Oper (Opb2d8, DoCmpa, DL, SaipW, (inst & 7), DL, DR, Ca, 1, GRL, SN)
Oper (Opb2e0, DoCmpa, DL, SmaiW, (inst & 7), DL, DR, Ca, 1, GRL, SN)
Oper (Opb2e8, DoCmpa, DL, SdaiW, (inst & 7), DL, DR, Ca, 1, GRL, SN)
Oper (Opb2f0, DoCmpa, DL, SaixW, (inst & 7), DL, DR, Ca, 1, GRL, SN)
Oper (Opb2f8, DoCmpa, DL, SearW, (inst & 7), DL, DR, Ca, 1, GRL, SN)

Oper (Opb308, DoCmpm, DB, SaipB, (inst & 7), DB, DA, CaipB, 1, GMB, SN)
Oper (Opb348, DoCmpm, DW, SaipW, (inst & 7), DW, DA, CaipW, 1, GMW, SN)
Oper (Opb388, DoCmpm, DL, SaipL, (inst & 7), DL, DA, CaipL, 1, GML, SN)
Oper (Opb3c0, DoCmpa, DL, SdL, (inst & 7), DL, DR, Ca, 1, GRL, SN)
Oper (Opb3c8, DoCmpa, DL, SaL, (inst & 7), DL, DR, Ca, 1, GRL, SN)
Oper (Opb3d0, DoCmpa, DL, SainL, (inst & 7), DL, DR, Ca, 1, GRL, SN)
Oper (Opb3d8, DoCmpa, DL, SaipL, (inst & 7), DL, DR, Ca, 1, GRL, SN)
Oper (Opb3e0, DoCmpa, DL, SmaiL, (inst & 7), DL, DR, Ca, 1, GRL, SN)
Oper (Opb3e8, DoCmpa, DL, SdaiL, (inst & 7), DL, DR, Ca, 1, GRL, SN)
Oper (Opb3f0, DoCmpa, DL, SaixL, (inst & 7), DL, DR, Ca, 1, GRL, SN)
Oper (Opb3f8, DoCmpa, DL, SearL, (inst & 7), DL, DR, Ca, 1, GRL, SN)

Oper (Opb400, DoCmpB, DB, SdB, (inst & 7), DB, DR, Cd, 2, GRB, SN)
Oper (Opb410, DoCmpB, DB, SainB, (inst & 7), DB, DR, Cd, 2, GRB, SN)
Oper (Opb418, DoCmpB, DB, SaipB, (inst & 7), DB, DR, Cd, 2, GRB, SN)
Oper (Opb420, DoCmpB, DB, SmaiB, (inst & 7), DB, DR, Cd, 2, GRB, SN)
Oper (Opb428, DoCmpB, DB, SdaiB, (inst & 7), DB, DR, Cd, 2, GRB, SN)
Oper (Opb430, DoCmpB, DB, SaixB, (inst & 7), DB, DR, Cd, 2, GRB, SN)
Oper (Opb438, DoCmpB, DB, SearB, (inst & 7), DB, DR, Cd, 2, GRB, SN)
Oper (Opb440, DoCmpW, DW, SdW, (inst & 7), DW, DR, Cd, 2, GRW, SN)
Oper (Opb448, DoCmpW, DW, SaW, (inst & 7), DW, DR, Cd, 2, GRW, SN)
Oper (Opb450, DoCmpW, DW, SainW, (inst & 7), DW, DR, Cd, 2, GRW, SN)
Oper (Opb458, DoCmpW, DW, SaipW, (inst & 7), DW, DR, Cd, 2, GRW, SN)
Oper (Opb460, DoCmpW, DW, SmaiW, (inst & 7), DW, DR, Cd, 2, GRW, SN)
Oper (Opb468, DoCmpW, DW, SdaiW, (inst & 7), DW, DR, Cd, 2, GRW, SN)
Oper (Opb470, DoCmpW, DW, SaixW, (inst & 7), DW, DR, Cd, 2, GRW, SN)
Oper (Opb478, DoCmpW, DW, SearW, (inst & 7), DW, DR, Cd, 2, GRW, SN)
Oper (Opb480, DoCmpL, DL, SdL, (inst & 7), DL, DR, Cd, 2, GRL, SN)
Oper (Opb488, DoCmpL, DL, SaL, (inst & 7), DL, DR, Cd, 2, GRL, SN)
Oper (Opb490, DoCmpL, DL, SainL, (inst & 7), DL, DR, Cd, 2, GRL, SN)
Oper (Opb498, DoCmpL, DL, SaipL, (inst & 7), DL, DR, Cd, 2, GRL, SN)
Oper (Opb4a0, DoCmpL, DL, SmaiL, (inst & 7), DL, DR, Cd, 2, GRL, SN)
Oper (Opb4a8, DoCmpL, DL, SdaiL, (inst & 7), DL, DR, Cd, 2, GRL, SN)
Oper (Opb4b0, DoCmpL, DL, SaixL, (inst & 7), DL, DR, Cd, 2, GRL, SN)
Oper (Opb4b8, DoCmpL, DL, SearL, (inst & 7), DL, DR, Cd, 2, GRL, SN)
Oper (Opb4c0, DoCmpa, DL, SdW, (inst & 7), DL, DR, Ca, 2, GRL, SN)
Oper (Opb4c8, DoCmpa, DL, SaW, (inst & 7), DL, DR, Ca, 2, GRL, SN)
Oper (Opb4d0, DoCmpa, DL, SainW, (inst & 7), DL, DR, Ca, 2, GRL, SN)
Oper (Opb4d8, DoCmpa, DL, SaipW, (inst & 7), DL, DR, Ca, 2, GRL, SN)
Oper (Opb4e0, DoCmpa, DL, SmaiW, (inst & 7), DL, DR, Ca, 2, GRL, SN)
Oper (Opb4e8, DoCmpa, DL, SdaiW, (inst & 7), DL, DR, Ca, 2, GRL, SN)
Oper (Opb4f0, DoCmpa, DL, SaixW, (inst & 7), DL, DR, Ca, 2, GRL, SN)
Oper (Opb4f8, DoCmpa, DL, SearW, (inst & 7), DL, DR, Ca, 2, GRL, SN)

Oper (Opb508, DoCmpm, DB, SaipB, (inst & 7), DB, DA, CaipB, 2, GMB, SN)
Oper (Opb548, DoCmpm, DW, SaipW, (inst & 7), DW, DA, CaipW, 2, GMW, SN)
Oper (Opb588, DoCmpm, DL, SaipL, (inst & 7), DL, DA, CaipL, 2, GML, SN)
Oper (Opb5c0, DoCmpa, DL, SdL, (inst & 7), DL, DR, Ca, 2, GRL, SN)
Oper (Opb5c8, DoCmpa, DL, SaL, (inst & 7), DL, DR, Ca, 2, GRL, SN)
Oper (Opb5d0, DoCmpa, DL, SainL, (inst & 7), DL, DR, Ca, 2, GRL, SN)
Oper (Opb5d8, DoCmpa, DL, SaipL, (inst & 7), DL, DR, Ca, 2, GRL, SN)
Oper (Opb5e0, DoCmpa, DL, SmaiL, (inst & 7), DL, DR, Ca, 2, GRL, SN)
Oper (Opb5e8, DoCmpa, DL, SdaiL, (inst & 7), DL, DR, Ca, 2, GRL, SN)
Oper (Opb5f0, DoCmpa, DL, SaixL, (inst & 7), DL, DR, Ca, 2, GRL, SN)
Oper (Opb5f8, DoCmpa, DL, SearL, (inst & 7), DL, DR, Ca, 2, GRL, SN)

Oper (Opb600, DoCmpB, DB, SdB, (inst & 7), DB, DR, Cd, 3, GRB, SN)
Oper (Opb610, DoCmpB, DB, SainB, (inst & 7), DB, DR, Cd, 3, GRB, SN)
Oper (Opb618, DoCmpB, DB, SaipB, (inst & 7), DB, DR, Cd, 3, GRB, SN)
Oper (Opb620, DoCmpB, DB, SmaiB, (inst & 7), DB, DR, Cd, 3, GRB, SN)
Oper (Opb628, DoCmpB, DB, SdaiB, (inst & 7), DB, DR, Cd, 3, GRB, SN)
Oper (Opb630, DoCmpB, DB, SaixB, (inst & 7), DB, DR, Cd, 3, GRB, SN)
Oper (Opb638, DoCmpB, DB, SearB, (inst & 7), DB, DR, Cd, 3, GRB, SN)
Oper (Opb640, DoCmpW, DW, SdW, (inst & 7), DW, DR, Cd, 3, GRW, SN)
Oper (Opb648, DoCmpW, DW, SaW, (inst & 7), DW, DR, Cd, 3, GRW, SN)
Oper (Opb650, DoCmpW, DW, SainW, (inst & 7), DW, DR, Cd, 3, GRW, SN)
Oper (Opb658, DoCmpW, DW, SaipW, (inst & 7), DW, DR, Cd, 3, GRW, SN)
Oper (Opb660, DoCmpW, DW, SmaiW, (inst & 7), DW, DR, Cd, 3, GRW, SN)
Oper (Opb668, DoCmpW, DW, SdaiW, (inst & 7), DW, DR, Cd, 3, GRW, SN)
Oper (Opb670, DoCmpW, DW, SaixW, (inst & 7), DW, DR, Cd, 3, GRW, SN)
Oper (Opb678, DoCmpW, DW, SearW, (inst & 7), DW, DR, Cd, 3, GRW, SN)
Oper (Opb680, DoCmpL, DL, SdL, (inst & 7), DL, DR, Cd, 3, GRL, SN)
Oper (Opb688, DoCmpL, DL, SaL, (inst & 7), DL, DR, Cd, 3, GRL, SN)
Oper (Opb690, DoCmpL, DL, SainL, (inst & 7), DL, DR, Cd, 3, GRL, SN)
Oper (Opb698, DoCmpL, DL, SaipL, (inst & 7), DL, DR, Cd, 3, GRL, SN)
Oper (Opb6a0, DoCmpL, DL, SmaiL, (inst & 7), DL, DR, Cd, 3, GRL, SN)
Oper (Opb6a8, DoCmpL, DL, SdaiL, (inst & 7), DL, DR, Cd, 3, GRL, SN)
Oper (Opb6b0, DoCmpL, DL, SaixL, (inst & 7), DL, DR, Cd, 3, GRL, SN)
Oper (Opb6b8, DoCmpL, DL, SearL, (inst & 7), DL, DR, Cd, 3, GRL, SN)
Oper (Opb6c0, DoCmpa, DL, SdW, (inst & 7), DL, DR, Ca, 3, GRL, SN)
Oper (Opb6c8, DoCmpa, DL, SaW, (inst & 7), DL, DR, Ca, 3, GRL, SN)
Oper (Opb6d0, DoCmpa, DL, SainW, (inst & 7), DL, DR, Ca, 3, GRL, SN)
Oper (Opb6d8, DoCmpa, DL, SaipW, (inst & 7), DL, DR, Ca, 3, GRL, SN)
Oper (Opb6e0, DoCmpa, DL, SmaiW, (inst & 7), DL, DR, Ca, 3, GRL, SN)
Oper (Opb6e8, DoCmpa, DL, SdaiW, (inst & 7), DL, DR, Ca, 3, GRL, SN)
Oper (Opb6f0, DoCmpa, DL, SaixW, (inst & 7), DL, DR, Ca, 3, GRL, SN)
Oper (Opb6f8, DoCmpa, DL, SearW, (inst & 7), DL, DR, Ca, 3, GRL, SN)

Oper (Opb708, DoCmpm, DB, SaipB, (inst & 7), DB, DA, CaipB, 3, GMB, SN)
Oper (Opb748, DoCmpm, DW, SaipW, (inst & 7), DW, DA, CaipW, 3, GMW, SN)
Oper (Opb788, DoCmpm, DL, SaipL, (inst & 7), DL, DA, CaipL, 3, GML, SN)
Oper (Opb7c0, DoCmpa, DL, SdL, (inst & 7), DL, DR, Ca, 3, GRL, SN)
Oper (Opb7c8, DoCmpa, DL, SaL, (inst & 7), DL, DR, Ca, 3, GRL, SN)
Oper (Opb7d0, DoCmpa, DL, SainL, (inst & 7), DL, DR, Ca, 3, GRL, SN)
Oper (Opb7d8, DoCmpa, DL, SaipL, (inst & 7), DL, DR, Ca, 3, GRL, SN)
Oper (Opb7e0, DoCmpa, DL, SmaiL, (inst & 7), DL, DR, Ca, 3, GRL, SN)
Oper (Opb7e8, DoCmpa, DL, SdaiL, (inst & 7), DL, DR, Ca, 3, GRL, SN)
Oper (Opb7f0, DoCmpa, DL, SaixL, (inst & 7), DL, DR, Ca, 3, GRL, SN)
Oper (Opb7f8, DoCmpa, DL, SearL, (inst & 7), DL, DR, Ca, 3, GRL, SN)

Oper (Opb800, DoCmpB, DB, SdB, (inst & 7), DB, DR, Cd, 4, GRB, SN)
Oper (Opb810, DoCmpB, DB, SainB, (inst & 7), DB, DR, Cd, 4, GRB, SN)
Oper (Opb818, DoCmpB, DB, SaipB, (inst & 7), DB, DR, Cd, 4, GRB, SN)
Oper (Opb820, DoCmpB, DB, SmaiB, (inst & 7), DB, DR, Cd, 4, GRB, SN)
Oper (Opb828, DoCmpB, DB, SdaiB, (inst & 7), DB, DR, Cd, 4, GRB, SN)
Oper (Opb830, DoCmpB, DB, SaixB, (inst & 7), DB, DR, Cd, 4, GRB, SN)
Oper (Opb838, DoCmpB, DB, SearB, (inst & 7), DB, DR, Cd, 4, GRB, SN)
Oper (Opb840, DoCmpW, DW, SdW, (inst & 7), DW, DR, Cd, 4, GRW, SN)
Oper (Opb848, DoCmpW, DW, SaW, (inst & 7), DW, DR, Cd, 4, GRW, SN)
Oper (Opb850, DoCmpW, DW, SainW, (inst & 7), DW, DR, Cd, 4, GRW, SN)
Oper (Opb858, DoCmpW, DW, SaipW, (inst & 7), DW, DR, Cd, 4, GRW, SN)
Oper (Opb860, DoCmpW, DW, SmaiW, (inst & 7), DW, DR, Cd, 4, GRW, SN)
Oper (Opb868, DoCmpW, DW, SdaiW, (inst & 7), DW, DR, Cd, 4, GRW, SN)
Oper (Opb870, DoCmpW, DW, SaixW, (inst & 7), DW, DR, Cd, 4, GRW, SN)
Oper (Opb878, DoCmpW, DW, SearW, (inst & 7), DW, DR, Cd, 4, GRW, SN)
Oper (Opb880, DoCmpL, DL, SdL, (inst & 7), DL, DR, Cd, 4, GRL, SN)
Oper (Opb888, DoCmpL, DL, SaL, (inst & 7), DL, DR, Cd, 4, GRL, SN)
Oper (Opb890, DoCmpL, DL, SainL, (inst & 7), DL, DR, Cd, 4, GRL, SN)
Oper (Opb898, DoCmpL, DL, SaipL, (inst & 7), DL, DR, Cd, 4, GRL, SN)
Oper (Opb8a0, DoCmpL, DL, SmaiL, (inst & 7), DL, DR, Cd, 4, GRL, SN)
Oper (Opb8a8, DoCmpL, DL, SdaiL, (inst & 7), DL, DR, Cd, 4, GRL, SN)
Oper (Opb8b0, DoCmpL, DL, SaixL, (inst & 7), DL, DR, Cd, 4, GRL, SN)
Oper (Opb8b8, DoCmpL, DL, SearL, (inst & 7), DL, DR, Cd, 4, GRL, SN)
Oper (Opb8c0, DoCmpa, DL, SdW, (inst & 7), DL, DR, Ca, 4, GRL, SN)
Oper (Opb8c8, DoCmpa, DL, SaW, (inst & 7), DL, DR, Ca, 4, GRL, SN)
Oper (Opb8d0, DoCmpa, DL, SainW, (inst & 7), DL, DR, Ca, 4, GRL, SN)
Oper (Opb8d8, DoCmpa, DL, SaipW, (inst & 7), DL, DR, Ca, 4, GRL, SN)
Oper (Opb8e0, DoCmpa, DL, SmaiW, (inst & 7), DL, DR, Ca, 4, GRL, SN)
Oper (Opb8e8, DoCmpa, DL, SdaiW, (inst & 7), DL, DR, Ca, 4, GRL, SN)
Oper (Opb8f0, DoCmpa, DL, SaixW, (inst & 7), DL, DR, Ca, 4, GRL, SN)
Oper (Opb8f8, DoCmpa, DL, SearW, (inst & 7), DL, DR, Ca, 4, GRL, SN)

Oper (Opb908, DoCmpm, DB, SaipB, (inst & 7), DB, DA, CaipB, 4, GMB, SN)
Oper (Opb948, DoCmpm, DW, SaipW, (inst & 7), DW, DA, CaipW, 4, GMW, SN)
Oper (Opb988, DoCmpm, DL, SaipL, (inst & 7), DL, DA, CaipL, 4, GML, SN)
Oper (Opb9c0, DoCmpa, DL, SdL, (inst & 7), DL, DR, Ca, 4, GRL, SN)
Oper (Opb9c8, DoCmpa, DL, SaL, (inst & 7), DL, DR, Ca, 4, GRL, SN)
Oper (Opb9d0, DoCmpa, DL, SainL, (inst & 7), DL, DR, Ca, 4, GRL, SN)
Oper (Opb9d8, DoCmpa, DL, SaipL, (inst & 7), DL, DR, Ca, 4, GRL, SN)
Oper (Opb9e0, DoCmpa, DL, SmaiL, (inst & 7), DL, DR, Ca, 4, GRL, SN)
Oper (Opb9e8, DoCmpa, DL, SdaiL, (inst & 7), DL, DR, Ca, 4, GRL, SN)
Oper (Opb9f0, DoCmpa, DL, SaixL, (inst & 7), DL, DR, Ca, 4, GRL, SN)
Oper (Opb9f8, DoCmpa, DL, SearL, (inst & 7), DL, DR, Ca, 4, GRL, SN)

Oper (Opba00, DoCmpB, DB, SdB, (inst & 7), DB, DR, Cd, 5, GRB, SN)
Oper (Opba10, DoCmpB, DB, SainB, (inst & 7), DB, DR, Cd, 5, GRB, SN)
Oper (Opba18, DoCmpB, DB, SaipB, (inst & 7), DB, DR, Cd, 5, GRB, SN)
Oper (Opba20, DoCmpB, DB, SmaiB, (inst & 7), DB, DR, Cd, 5, GRB, SN)
Oper (Opba28, DoCmpB, DB, SdaiB, (inst & 7), DB, DR, Cd, 5, GRB, SN)
Oper (Opba30, DoCmpB, DB, SaixB, (inst & 7), DB, DR, Cd, 5, GRB, SN)
Oper (Opba38, DoCmpB, DB, SearB, (inst & 7), DB, DR, Cd, 5, GRB, SN)
Oper (Opba40, DoCmpW, DW, SdW, (inst & 7), DW, DR, Cd, 5, GRW, SN)
Oper (Opba48, DoCmpW, DW, SaW, (inst & 7), DW, DR, Cd, 5, GRW, SN)
Oper (Opba50, DoCmpW, DW, SainW, (inst & 7), DW, DR, Cd, 5, GRW, SN)
Oper (Opba58, DoCmpW, DW, SaipW, (inst & 7), DW, DR, Cd, 5, GRW, SN)
Oper (Opba60, DoCmpW, DW, SmaiW, (inst & 7), DW, DR, Cd, 5, GRW, SN)
Oper (Opba68, DoCmpW, DW, SdaiW, (inst & 7), DW, DR, Cd, 5, GRW, SN)
Oper (Opba70, DoCmpW, DW, SaixW, (inst & 7), DW, DR, Cd, 5, GRW, SN)
Oper (Opba78, DoCmpW, DW, SearW, (inst & 7), DW, DR, Cd, 5, GRW, SN)
Oper (Opba80, DoCmpL, DL, SdL, (inst & 7), DL, DR, Cd, 5, GRL, SN)
Oper (Opba88, DoCmpL, DL, SaL, (inst & 7), DL, DR, Cd, 5, GRL, SN)
Oper (Opba90, DoCmpL, DL, SainL, (inst & 7), DL, DR, Cd, 5, GRL, SN)
Oper (Opba98, DoCmpL, DL, SaipL, (inst & 7), DL, DR, Cd, 5, GRL, SN)
Oper (Opbaa0, DoCmpL, DL, SmaiL, (inst & 7), DL, DR, Cd, 5, GRL, SN)
Oper (Opbaa8, DoCmpL, DL, SdaiL, (inst & 7), DL, DR, Cd, 5, GRL, SN)
Oper (Opbab0, DoCmpL, DL, SaixL, (inst & 7), DL, DR, Cd, 5, GRL, SN)
Oper (Opbab8, DoCmpL, DL, SearL, (inst & 7), DL, DR, Cd, 5, GRL, SN)
Oper (Opbac0, DoCmpa, DL, SdW, (inst & 7), DL, DR, Ca, 5, GRL, SN)
Oper (Opbac8, DoCmpa, DL, SaW, (inst & 7), DL, DR, Ca, 5, GRL, SN)
Oper (Opbad0, DoCmpa, DL, SainW, (inst & 7), DL, DR, Ca, 5, GRL, SN)
Oper (Opbad8, DoCmpa, DL, SaipW, (inst & 7), DL, DR, Ca, 5, GRL, SN)
Oper (Opbae0, DoCmpa, DL, SmaiW, (inst & 7), DL, DR, Ca, 5, GRL, SN)
Oper (Opbae8, DoCmpa, DL, SdaiW, (inst & 7), DL, DR, Ca, 5, GRL, SN)
Oper (Opbaf0, DoCmpa, DL, SaixW, (inst & 7), DL, DR, Ca, 5, GRL, SN)
Oper (Opbaf8, DoCmpa, DL, SearW, (inst & 7), DL, DR, Ca, 5, GRL, SN)

Oper (Opbb08, DoCmpm, DB, SaipB, (inst & 7), DB, DA, CaipB, 5, GMB, SN)
Oper (Opbb48, DoCmpm, DW, SaipW, (inst & 7), DW, DA, CaipW, 5, GMW, SN)
Oper (Opbb88, DoCmpm, DL, SaipL, (inst & 7), DL, DA, CaipL, 5, GML, SN)
Oper (Opbbc0, DoCmpa, DL, SdL, (inst & 7), DL, DR, Ca, 5, GRL, SN)
Oper (Opbbc8, DoCmpa, DL, SaL, (inst & 7), DL, DR, Ca, 5, GRL, SN)
Oper (Opbbd0, DoCmpa, DL, SainL, (inst & 7), DL, DR, Ca, 5, GRL, SN)
Oper (Opbbd8, DoCmpa, DL, SaipL, (inst & 7), DL, DR, Ca, 5, GRL, SN)
Oper (Opbbe0, DoCmpa, DL, SmaiL, (inst & 7), DL, DR, Ca, 5, GRL, SN)
Oper (Opbbe8, DoCmpa, DL, SdaiL, (inst & 7), DL, DR, Ca, 5, GRL, SN)
Oper (Opbbf0, DoCmpa, DL, SaixL, (inst & 7), DL, DR, Ca, 5, GRL, SN)
Oper (Opbbf8, DoCmpa, DL, SearL, (inst & 7), DL, DR, Ca, 5, GRL, SN)

Oper (Opbc00, DoCmpB, DB, SdB, (inst & 7), DB, DR, Cd, 6, GRB, SN)
Oper (Opbc10, DoCmpB, DB, SainB, (inst & 7), DB, DR, Cd, 6, GRB, SN)
Oper (Opbc18, DoCmpB, DB, SaipB, (inst & 7), DB, DR, Cd, 6, GRB, SN)
Oper (Opbc20, DoCmpB, DB, SmaiB, (inst & 7), DB, DR, Cd, 6, GRB, SN)
Oper (Opbc28, DoCmpB, DB, SdaiB, (inst & 7), DB, DR, Cd, 6, GRB, SN)
Oper (Opbc30, DoCmpB, DB, SaixB, (inst & 7), DB, DR, Cd, 6, GRB, SN)
Oper (Opbc38, DoCmpB, DB, SearB, (inst & 7), DB, DR, Cd, 6, GRB, SN)
Oper (Opbc40, DoCmpW, DW, SdW, (inst & 7), DW, DR, Cd, 6, GRW, SN)
Oper (Opbc48, DoCmpW, DW, SaW, (inst & 7), DW, DR, Cd, 6, GRW, SN)
Oper (Opbc50, DoCmpW, DW, SainW, (inst & 7), DW, DR, Cd, 6, GRW, SN)
Oper (Opbc58, DoCmpW, DW, SaipW, (inst & 7), DW, DR, Cd, 6, GRW, SN)
Oper (Opbc60, DoCmpW, DW, SmaiW, (inst & 7), DW, DR, Cd, 6, GRW, SN)
Oper (Opbc68, DoCmpW, DW, SdaiW, (inst & 7), DW, DR, Cd, 6, GRW, SN)
Oper (Opbc70, DoCmpW, DW, SaixW, (inst & 7), DW, DR, Cd, 6, GRW, SN)
Oper (Opbc78, DoCmpW, DW, SearW, (inst & 7), DW, DR, Cd, 6, GRW, SN)
Oper (Opbc80, DoCmpL, DL, SdL, (inst & 7), DL, DR, Cd, 6, GRL, SN)
Oper (Opbc88, DoCmpL, DL, SaL, (inst & 7), DL, DR, Cd, 6, GRL, SN)
Oper (Opbc90, DoCmpL, DL, SainL, (inst & 7), DL, DR, Cd, 6, GRL, SN)
Oper (Opbc98, DoCmpL, DL, SaipL, (inst & 7), DL, DR, Cd, 6, GRL, SN)
Oper (Opbca0, DoCmpL, DL, SmaiL, (inst & 7), DL, DR, Cd, 6, GRL, SN)
Oper (Opbca8, DoCmpL, DL, SdaiL, (inst & 7), DL, DR, Cd, 6, GRL, SN)
Oper (Opbcb0, DoCmpL, DL, SaixL, (inst & 7), DL, DR, Cd, 6, GRL, SN)
Oper (Opbcb8, DoCmpL, DL, SearL, (inst & 7), DL, DR, Cd, 6, GRL, SN)
Oper (Opbcc0, DoCmpa, DL, SdW, (inst & 7), DL, DR, Ca, 6, GRL, SN)
Oper (Opbcc8, DoCmpa, DL, SaW, (inst & 7), DL, DR, Ca, 6, GRL, SN)
Oper (Opbcd0, DoCmpa, DL, SainW, (inst & 7), DL, DR, Ca, 6, GRL, SN)
Oper (Opbcd8, DoCmpa, DL, SaipW, (inst & 7), DL, DR, Ca, 6, GRL, SN)
Oper (Opbce0, DoCmpa, DL, SmaiW, (inst & 7), DL, DR, Ca, 6, GRL, SN)
Oper (Opbce8, DoCmpa, DL, SdaiW, (inst & 7), DL, DR, Ca, 6, GRL, SN)
Oper (Opbcf0, DoCmpa, DL, SaixW, (inst & 7), DL, DR, Ca, 6, GRL, SN)
Oper (Opbcf8, DoCmpa, DL, SearW, (inst & 7), DL, DR, Ca, 6, GRL, SN)

Oper (Opbd08, DoCmpm, DB, SaipB, (inst & 7), DB, DA, CaipB, 6, GMB, SN)
Oper (Opbd48, DoCmpm, DW, SaipW, (inst & 7), DW, DA, CaipW, 6, GMW, SN)
Oper (Opbd88, DoCmpm, DL, SaipL, (inst & 7), DL, DA, CaipL, 6, GML, SN)
Oper (Opbdc0, DoCmpa, DL, SdL, (inst & 7), DL, DR, Ca, 6, GRL, SN)
Oper (Opbdc8, DoCmpa, DL, SaL, (inst & 7), DL, DR, Ca, 6, GRL, SN)
Oper (Opbdd0, DoCmpa, DL, SainL, (inst & 7), DL, DR, Ca, 6, GRL, SN)
Oper (Opbdd8, DoCmpa, DL, SaipL, (inst & 7), DL, DR, Ca, 6, GRL, SN)
Oper (Opbde0, DoCmpa, DL, SmaiL, (inst & 7), DL, DR, Ca, 6, GRL, SN)
Oper (Opbde8, DoCmpa, DL, SdaiL, (inst & 7), DL, DR, Ca, 6, GRL, SN)
Oper (Opbdf0, DoCmpa, DL, SaixL, (inst & 7), DL, DR, Ca, 6, GRL, SN)
Oper (Opbdf8, DoCmpa, DL, SearL, (inst & 7), DL, DR, Ca, 6, GRL, SN)

Oper (Opbe00, DoCmpB, DB, SdB, (inst & 7), DB, DR, Cd, 7, GRB, SN)
Oper (Opbe10, DoCmpB, DB, SainB, (inst & 7), DB, DR, Cd, 7, GRB, SN)
Oper (Opbe18, DoCmpB, DB, SaipB, (inst & 7), DB, DR, Cd, 7, GRB, SN)
Oper (Opbe20, DoCmpB, DB, SmaiB, (inst & 7), DB, DR, Cd, 7, GRB, SN)
Oper (Opbe28, DoCmpB, DB, SdaiB, (inst & 7), DB, DR, Cd, 7, GRB, SN)
Oper (Opbe30, DoCmpB, DB, SaixB, (inst & 7), DB, DR, Cd, 7, GRB, SN)
Oper (Opbe38, DoCmpB, DB, SearB, (inst & 7), DB, DR, Cd, 7, GRB, SN)
Oper (Opbe40, DoCmpW, DW, SdW, (inst & 7), DW, DR, Cd, 7, GRW, SN)
Oper (Opbe48, DoCmpW, DW, SaW, (inst & 7), DW, DR, Cd, 7, GRW, SN)
Oper (Opbe50, DoCmpW, DW, SainW, (inst & 7), DW, DR, Cd, 7, GRW, SN)
Oper (Opbe58, DoCmpW, DW, SaipW, (inst & 7), DW, DR, Cd, 7, GRW, SN)
Oper (Opbe60, DoCmpW, DW, SmaiW, (inst & 7), DW, DR, Cd, 7, GRW, SN)
Oper (Opbe68, DoCmpW, DW, SdaiW, (inst & 7), DW, DR, Cd, 7, GRW, SN)
Oper (Opbe70, DoCmpW, DW, SaixW, (inst & 7), DW, DR, Cd, 7, GRW, SN)
Oper (Opbe78, DoCmpW, DW, SearW, (inst & 7), DW, DR, Cd, 7, GRW, SN)
Oper (Opbe80, DoCmpL, DL, SdL, (inst & 7), DL, DR, Cd, 7, GRL, SN)
Oper (Opbe88, DoCmpL, DL, SaL, (inst & 7), DL, DR, Cd, 7, GRL, SN)
Oper (Opbe90, DoCmpL, DL, SainL, (inst & 7), DL, DR, Cd, 7, GRL, SN)
Oper (Opbe98, DoCmpL, DL, SaipL, (inst & 7), DL, DR, Cd, 7, GRL, SN)
Oper (Opbea0, DoCmpL, DL, SmaiL, (inst & 7), DL, DR, Cd, 7, GRL, SN)
Oper (Opbea8, DoCmpL, DL, SdaiL, (inst & 7), DL, DR, Cd, 7, GRL, SN)
Oper (Opbeb0, DoCmpL, DL, SaixL, (inst & 7), DL, DR, Cd, 7, GRL, SN)
Oper (Opbeb8, DoCmpL, DL, SearL, (inst & 7), DL, DR, Cd, 7, GRL, SN)
Oper (Opbec0, DoCmpa, DL, SdW, (inst & 7), DL, DR, Ca, 7, GRL, SN)
Oper (Opbec8, DoCmpa, DL, SaW, (inst & 7), DL, DR, Ca, 7, GRL, SN)
Oper (Opbed0, DoCmpa, DL, SainW, (inst & 7), DL, DR, Ca, 7, GRL, SN)
Oper (Opbed8, DoCmpa, DL, SaipW, (inst & 7), DL, DR, Ca, 7, GRL, SN)
Oper (Opbee0, DoCmpa, DL, SmaiW, (inst & 7), DL, DR, Ca, 7, GRL, SN)
Oper (Opbee8, DoCmpa, DL, SdaiW, (inst & 7), DL, DR, Ca, 7, GRL, SN)
Oper (Opbef0, DoCmpa, DL, SaixW, (inst & 7), DL, DR, Ca, 7, GRL, SN)
Oper (Opbef8, DoCmpa, DL, SearW, (inst & 7), DL, DR, Ca, 7, GRL, SN)

Oper (Opbf08, DoCmpm, DB, SaipB, (inst & 7), DB, DA, CaipB15, 7, GMB, SN)
Oper (Opbf48, DoCmpm, DW, SaipW, (inst & 7), DW, DA, CaipW, 7, GMW, SN)
Oper (Opbf88, DoCmpm, DL, SaipL, (inst & 7), DL, DA, CaipL, 7, GML, SN)
Oper (Opbfc0, DoCmpa, DL, SdL, (inst & 7), DL, DR, Ca, 7, GRL, SN)
Oper (Opbfc8, DoCmpa, DL, SaL, (inst & 7), DL, DR, Ca, 7, GRL, SN)
Oper (Opbfd0, DoCmpa, DL, SainL, (inst & 7), DL, DR, Ca, 7, GRL, SN)
Oper (Opbfd8, DoCmpa, DL, SaipL, (inst & 7), DL, DR, Ca, 7, GRL, SN)
Oper (Opbfe0, DoCmpa, DL, SmaiL, (inst & 7), DL, DR, Ca, 7, GRL, SN)
Oper (Opbfe8, DoCmpa, DL, SdaiL, (inst & 7), DL, DR, Ca, 7, GRL, SN)
Oper (Opbff0, DoCmpa, DL, SaixL, (inst & 7), DL, DR, Ca, 7, GRL, SN)
Oper (Opbff8, DoCmpa, DL, SearL, (inst & 7), DL, DR, Ca, 7, GRL, SN)

Oper (Op80c0, DoDivu, DW, SdW, (inst & 7), DL, DR, Cd, 0, GRL, SRL)
Oper (Op80d0, DoDivu, DW, SainW, (inst & 7), DL, DR, Cd, 0, GRL, SRL)
Oper (Op80d8, DoDivu, DW, SaipW, (inst & 7), DL, DR, Cd, 0, GRL, SRL)
Oper (Op80e0, DoDivu, DW, SmaiW, (inst & 7), DL, DR, Cd, 0, GRL, SRL)
Oper (Op80e8, DoDivu, DW, SdaiW, (inst & 7), DL, DR, Cd, 0, GRL, SRL)
Oper (Op80f0, DoDivu, DW, SaixW, (inst & 7), DL, DR, Cd, 0, GRL, SRL)
Oper (Op80f8, DoDivu, DW, SearW, (inst & 7), DL, DR, Cd, 0, GRL, SRL)

Oper (Op81c0, DoDivs, DW, SdW, (inst & 7), DL, DR, Cd, 0, GRL, SRL)
Oper (Op81d0, DoDivs, DW, SainW, (inst & 7), DL, DR, Cd, 0, GRL, SRL)
Oper (Op81d8, DoDivs, DW, SaipW, (inst & 7), DL, DR, Cd, 0, GRL, SRL)
Oper (Op81e0, DoDivs, DW, SmaiW, (inst & 7), DL, DR, Cd, 0, GRL, SRL)
Oper (Op81e8, DoDivs, DW, SdaiW, (inst & 7), DL, DR, Cd, 0, GRL, SRL)
Oper (Op81f0, DoDivs, DW, SaixW, (inst & 7), DL, DR, Cd, 0, GRL, SRL)
Oper (Op81f8, DoDivs, DW, SearW, (inst & 7), DL, DR, Cd, 0, GRL, SRL)

Oper (Op82c0, DoDivu, DW, SdW, (inst & 7), DL, DR, Cd, 1, GRL, SRL)
Oper (Op82d0, DoDivu, DW, SainW, (inst & 7), DL, DR, Cd, 1, GRL, SRL)
Oper (Op82d8, DoDivu, DW, SaipW, (inst & 7), DL, DR, Cd, 1, GRL, SRL)
Oper (Op82e0, DoDivu, DW, SmaiW, (inst & 7), DL, DR, Cd, 1, GRL, SRL)
Oper (Op82e8, DoDivu, DW, SdaiW, (inst & 7), DL, DR, Cd, 1, GRL, SRL)
Oper (Op82f0, DoDivu, DW, SaixW, (inst & 7), DL, DR, Cd, 1, GRL, SRL)
Oper (Op82f8, DoDivu, DW, SearW, (inst & 7), DL, DR, Cd, 1, GRL, SRL)

Oper (Op83c0, DoDivs, DW, SdW, (inst & 7), DL, DR, Cd, 1, GRL, SRL)
Oper (Op83d0, DoDivs, DW, SainW, (inst & 7), DL, DR, Cd, 1, GRL, SRL)
Oper (Op83d8, DoDivs, DW, SaipW, (inst & 7), DL, DR, Cd, 1, GRL, SRL)
Oper (Op83e0, DoDivs, DW, SmaiW, (inst & 7), DL, DR, Cd, 1, GRL, SRL)
Oper (Op83e8, DoDivs, DW, SdaiW, (inst & 7), DL, DR, Cd, 1, GRL, SRL)
Oper (Op83f0, DoDivs, DW, SaixW, (inst & 7), DL, DR, Cd, 1, GRL, SRL)
Oper (Op83f8, DoDivs, DW, SearW, (inst & 7), DL, DR, Cd, 1, GRL, SRL)

Oper (Op84c0, DoDivu, DW, SdW, (inst & 7), DL, DR, Cd, 2, GRL, SRL)
Oper (Op84d0, DoDivu, DW, SainW, (inst & 7), DL, DR, Cd, 2, GRL, SRL)
Oper (Op84d8, DoDivu, DW, SaipW, (inst & 7), DL, DR, Cd, 2, GRL, SRL)
Oper (Op84e0, DoDivu, DW, SmaiW, (inst & 7), DL, DR, Cd, 2, GRL, SRL)
Oper (Op84e8, DoDivu, DW, SdaiW, (inst & 7), DL, DR, Cd, 2, GRL, SRL)
Oper (Op84f0, DoDivu, DW, SaixW, (inst & 7), DL, DR, Cd, 2, GRL, SRL)
Oper (Op84f8, DoDivu, DW, SearW, (inst & 7), DL, DR, Cd, 2, GRL, SRL)

Oper (Op85c0, DoDivs, DW, SdW, (inst & 7), DL, DR, Cd, 2, GRL, SRL)
Oper (Op85d0, DoDivs, DW, SainW, (inst & 7), DL, DR, Cd, 2, GRL, SRL)
Oper (Op85d8, DoDivs, DW, SaipW, (inst & 7), DL, DR, Cd, 2, GRL, SRL)
Oper (Op85e0, DoDivs, DW, SmaiW, (inst & 7), DL, DR, Cd, 2, GRL, SRL)
Oper (Op85e8, DoDivs, DW, SdaiW, (inst & 7), DL, DR, Cd, 2, GRL, SRL)
Oper (Op85f0, DoDivs, DW, SaixW, (inst & 7), DL, DR, Cd, 2, GRL, SRL)
Oper (Op85f8, DoDivs, DW, SearW, (inst & 7), DL, DR, Cd, 2, GRL, SRL)

Oper (Op86c0, DoDivu, DW, SdW, (inst & 7), DL, DR, Cd, 3, GRL, SRL)
Oper (Op86d0, DoDivu, DW, SainW, (inst & 7), DL, DR, Cd, 3, GRL, SRL)
Oper (Op86d8, DoDivu, DW, SaipW, (inst & 7), DL, DR, Cd, 3, GRL, SRL)
Oper (Op86e0, DoDivu, DW, SmaiW, (inst & 7), DL, DR, Cd, 3, GRL, SRL)
Oper (Op86e8, DoDivu, DW, SdaiW, (inst & 7), DL, DR, Cd, 3, GRL, SRL)
Oper (Op86f0, DoDivu, DW, SaixW, (inst & 7), DL, DR, Cd, 3, GRL, SRL)
Oper (Op86f8, DoDivu, DW, SearW, (inst & 7), DL, DR, Cd, 3, GRL, SRL)

Oper (Op87c0, DoDivs, DW, SdW, (inst & 7), DL, DR, Cd, 3, GRL, SRL)
Oper (Op87d0, DoDivs, DW, SainW, (inst & 7), DL, DR, Cd, 3, GRL, SRL)
Oper (Op87d8, DoDivs, DW, SaipW, (inst & 7), DL, DR, Cd, 3, GRL, SRL)
Oper (Op87e0, DoDivs, DW, SmaiW, (inst & 7), DL, DR, Cd, 3, GRL, SRL)
Oper (Op87e8, DoDivs, DW, SdaiW, (inst & 7), DL, DR, Cd, 3, GRL, SRL)
Oper (Op87f0, DoDivs, DW, SaixW, (inst & 7), DL, DR, Cd, 3, GRL, SRL)
Oper (Op87f8, DoDivs, DW, SearW, (inst & 7), DL, DR, Cd, 3, GRL, SRL)

Oper (Op88c0, DoDivu, DW, SdW, (inst & 7), DL, DR, Cd, 4, GRL, SRL)
Oper (Op88d0, DoDivu, DW, SainW, (inst & 7), DL, DR, Cd, 4, GRL, SRL)
Oper (Op88d8, DoDivu, DW, SaipW, (inst & 7), DL, DR, Cd, 4, GRL, SRL)
Oper (Op88e0, DoDivu, DW, SmaiW, (inst & 7), DL, DR, Cd, 4, GRL, SRL)
Oper (Op88e8, DoDivu, DW, SdaiW, (inst & 7), DL, DR, Cd, 4, GRL, SRL)
Oper (Op88f0, DoDivu, DW, SaixW, (inst & 7), DL, DR, Cd, 4, GRL, SRL)
Oper (Op88f8, DoDivu, DW, SearW, (inst & 7), DL, DR, Cd, 4, GRL, SRL)

Oper (Op89c0, DoDivs, DW, SdW, (inst & 7), DL, DR, Cd, 4, GRL, SRL)
Oper (Op89d0, DoDivs, DW, SainW, (inst & 7), DL, DR, Cd, 4, GRL, SRL)
Oper (Op89d8, DoDivs, DW, SaipW, (inst & 7), DL, DR, Cd, 4, GRL, SRL)
Oper (Op89e0, DoDivs, DW, SmaiW, (inst & 7), DL, DR, Cd, 4, GRL, SRL)
Oper (Op89e8, DoDivs, DW, SdaiW, (inst & 7), DL, DR, Cd, 4, GRL, SRL)
Oper (Op89f0, DoDivs, DW, SaixW, (inst & 7), DL, DR, Cd, 4, GRL, SRL)
Oper (Op89f8, DoDivs, DW, SearW, (inst & 7), DL, DR, Cd, 4, GRL, SRL)

Oper (Op8ac0, DoDivu, DW, SdW, (inst & 7), DL, DR, Cd, 5, GRL, SRL)
Oper (Op8ad0, DoDivu, DW, SainW, (inst & 7), DL, DR, Cd, 5, GRL, SRL)
Oper (Op8ad8, DoDivu, DW, SaipW, (inst & 7), DL, DR, Cd, 5, GRL, SRL)
Oper (Op8ae0, DoDivu, DW, SmaiW, (inst & 7), DL, DR, Cd, 5, GRL, SRL)
Oper (Op8ae8, DoDivu, DW, SdaiW, (inst & 7), DL, DR, Cd, 5, GRL, SRL)
Oper (Op8af0, DoDivu, DW, SaixW, (inst & 7), DL, DR, Cd, 5, GRL, SRL)
Oper (Op8af8, DoDivu, DW, SearW, (inst & 7), DL, DR, Cd, 5, GRL, SRL)

Oper (Op8bc0, DoDivs, DW, SdW, (inst & 7), DL, DR, Cd, 5, GRL, SRL)
Oper (Op8bd0, DoDivs, DW, SainW, (inst & 7), DL, DR, Cd, 5, GRL, SRL)
Oper (Op8bd8, DoDivs, DW, SaipW, (inst & 7), DL, DR, Cd, 5, GRL, SRL)
Oper (Op8be0, DoDivs, DW, SmaiW, (inst & 7), DL, DR, Cd, 5, GRL, SRL)
Oper (Op8be8, DoDivs, DW, SdaiW, (inst & 7), DL, DR, Cd, 5, GRL, SRL)
Oper (Op8bf0, DoDivs, DW, SaixW, (inst & 7), DL, DR, Cd, 5, GRL, SRL)
Oper (Op8bf8, DoDivs, DW, SearW, (inst & 7), DL, DR, Cd, 5, GRL, SRL)

Oper (Op8cc0, DoDivu, DW, SdW, (inst & 7), DL, DR, Cd, 6, GRL, SRL)
Oper (Op8cd0, DoDivu, DW, SainW, (inst & 7), DL, DR, Cd, 6, GRL, SRL)
Oper (Op8cd8, DoDivu, DW, SaipW, (inst & 7), DL, DR, Cd, 6, GRL, SRL)
Oper (Op8ce0, DoDivu, DW, SmaiW, (inst & 7), DL, DR, Cd, 6, GRL, SRL)
Oper (Op8ce8, DoDivu, DW, SdaiW, (inst & 7), DL, DR, Cd, 6, GRL, SRL)
Oper (Op8cf0, DoDivu, DW, SaixW, (inst & 7), DL, DR, Cd, 6, GRL, SRL)
Oper (Op8cf8, DoDivu, DW, SearW, (inst & 7), DL, DR, Cd, 6, GRL, SRL)

Oper (Op8dc0, DoDivs, DW, SdW, (inst & 7), DL, DR, Cd, 6, GRL, SRL)
Oper (Op8dd0, DoDivs, DW, SainW, (inst & 7), DL, DR, Cd, 6, GRL, SRL)
Oper (Op8dd8, DoDivs, DW, SaipW, (inst & 7), DL, DR, Cd, 6, GRL, SRL)
Oper (Op8de0, DoDivs, DW, SmaiW, (inst & 7), DL, DR, Cd, 6, GRL, SRL)
Oper (Op8de8, DoDivs, DW, SdaiW, (inst & 7), DL, DR, Cd, 6, GRL, SRL)
Oper (Op8df0, DoDivs, DW, SaixW, (inst & 7), DL, DR, Cd, 6, GRL, SRL)
Oper (Op8df8, DoDivs, DW, SearW, (inst & 7), DL, DR, Cd, 6, GRL, SRL)

Oper (Op8ec0, DoDivu, DW, SdW, (inst & 7), DL, DR, Cd, 7, GRL, SRL)
Oper (Op8ed0, DoDivu, DW, SainW, (inst & 7), DL, DR, Cd, 7, GRL, SRL)
Oper (Op8ed8, DoDivu, DW, SaipW, (inst & 7), DL, DR, Cd, 7, GRL, SRL)
Oper (Op8ee0, DoDivu, DW, SmaiW, (inst & 7), DL, DR, Cd, 7, GRL, SRL)
Oper (Op8ee8, DoDivu, DW, SdaiW, (inst & 7), DL, DR, Cd, 7, GRL, SRL)
Oper (Op8ef0, DoDivu, DW, SaixW, (inst & 7), DL, DR, Cd, 7, GRL, SRL)
Oper (Op8ef8, DoDivu, DW, SearW, (inst & 7), DL, DR, Cd, 7, GRL, SRL)

Oper (Op8fc0, DoDivs, DW, SdW, (inst & 7), DL, DR, Cd, 7, GRL, SRL)
Oper (Op8fd0, DoDivs, DW, SainW, (inst & 7), DL, DR, Cd, 7, GRL, SRL)
Oper (Op8fd8, DoDivs, DW, SaipW, (inst & 7), DL, DR, Cd, 7, GRL, SRL)
Oper (Op8fe0, DoDivs, DW, SmaiW, (inst & 7), DL, DR, Cd, 7, GRL, SRL)
Oper (Op8fe8, DoDivs, DW, SdaiW, (inst & 7), DL, DR, Cd, 7, GRL, SRL)
Oper (Op8ff0, DoDivs, DW, SaixW, (inst & 7), DL, DR, Cd, 7, GRL, SRL)
Oper (Op8ff8, DoDivs, DW, SearW, (inst & 7), DL, DR, Cd, 7, GRL, SRL)

Oper (Op0c00, DoCmpB, DB, SimmB, 0, DB, DR, Cd, (inst & 7), GRB, SN)
Oper (Op0c10, DoCmpB, DB, SimmB, 0, DB, DA, Cain, (inst & 7), GMB, SN)
Oper (Op0c18, DoCmpB, DB, SimmB, 0, DB, DA, CaipB, (inst & 7), GMB, SN)
Oper (Op0c20, DoCmpB, DB, SimmB, 0, DB, DA, CmaiB, (inst & 7), GMB, SN)
Oper (Op0c28, DoCmpB, DB, SimmB, 0, DB, DA, Cdai, (inst & 7), GMB, SN)
Oper (Op0c30, DoCmpB, DB, SimmB, 0, DB, DA, Caix, (inst & 7), GMB, SN)
Oper (Op0c38, DoCmpB, DB, SimmB, 0, DB, DA, Ceaw, (inst & 7), GMB, SN)
Oper (Op0c40, DoCmpW, DW, SimmW, 0, DW, DR, Cd, (inst & 7), GRW, SN)
Oper (Op0c50, DoCmpW, DW, SimmW, 0, DW, DA, Cain, (inst & 7), GMW, SN)
Oper (Op0c58, DoCmpW, DW, SimmW, 0, DW, DA, CaipW, (inst & 7), GMW, SN)
Oper (Op0c60, DoCmpW, DW, SimmW, 0, DW, DA, CmaiW, (inst & 7), GMW, SN)
Oper (Op0c68, DoCmpW, DW, SimmW, 0, DW, DA, Cdai, (inst & 7), GMW, SN)
Oper (Op0c70, DoCmpW, DW, SimmW, 0, DW, DA, Caix, (inst & 7), GMW, SN)
Oper (Op0c78, DoCmpW, DW, SimmW, 0, DW, DA, Ceaw, (inst & 7), GMW, SN)
Oper (Op0c80, DoCmpL, DL, SimmL, 0, DL, DR, Cd, (inst & 7), GRL, SN)
Oper (Op0c90, DoCmpL, DL, SimmL, 0, DL, DA, Cain, (inst & 7), GML, SN)
Oper (Op0c98, DoCmpL, DL, SimmL, 0, DL, DA, CaipL, (inst & 7), GML, SN)
Oper (Op0ca0, DoCmpL, DL, SimmL, 0, DL, DA, CmaiL, (inst & 7), GML, SN)
Oper (Op0ca8, DoCmpL, DL, SimmL, 0, DL, DA, Cdai, (inst & 7), GML, SN)
Oper (Op0cb0, DoCmpL, DL, SimmL, 0, DL, DA, Caix, (inst & 7), GML, SN)
Oper (Op0cb8, DoCmpL, DL, SimmL, 0, DL, DA, Ceaw, (inst & 7), GML, SN)

Oper (Opc100, DoABCD, DB, SdB,   (inst & 7), DB, DR, Cd, 0, GRB, SRB)
Oper (Opc108, DoABCD, DB, SmaiB, (inst & 7), DB, DA, CmaiB, 0, GMB, SMB)
Oper (Opc300, DoABCD, DB, SdB,   (inst & 7), DB, DR, Cd, 1, GRB, SRB)
Oper (Opc308, DoABCD, DB, SmaiB, (inst & 7), DB, DA, CmaiB, 1, GMB, SMB)
Oper (Opc500, DoABCD, DB, SdB,   (inst & 7), DB, DR, Cd, 2, GRB, SRB)
Oper (Opc508, DoABCD, DB, SmaiB, (inst & 7), DB, DA, CmaiB, 2, GMB, SMB)
Oper (Opc700, DoABCD, DB, SdB,   (inst & 7), DB, DR, Cd, 3, GRB, SRB)
Oper (Opc708, DoABCD, DB, SmaiB, (inst & 7), DB, DA, CmaiB, 3, GMB, SMB)
Oper (Opc900, DoABCD, DB, SdB,   (inst & 7), DB, DR, Cd, 4, GRB, SRB)
Oper (Opc908, DoABCD, DB, SmaiB, (inst & 7), DB, DA, CmaiB, 4, GMB, SMB)
Oper (Opcb00, DoABCD, DB, SdB,   (inst & 7), DB, DR, Cd, 5, GRB, SRB)
Oper (Opcb08, DoABCD, DB, SmaiB, (inst & 7), DB, DA, CmaiB, 5, GMB, SMB)
Oper (Opcd00, DoABCD, DB, SdB,   (inst & 7), DB, DR, Cd, 6, GRB, SRB)
Oper (Opcd08, DoABCD, DB, SmaiB, (inst & 7), DB, DA, CmaiB, 6, GMB, SMB)
Oper (Opcf00, DoABCD, DB, SdB,   (inst & 7), DB, DR, Cd, 7, GRB, SRB)
Oper (Opcf08, DoABCD, DB, SmaiB, (inst & 7), DB, DA, CmaiB, 7, GMB, SMB)

Oper (Op8100, DoSBCD, DB, SdB,   (inst & 7), DB, DR, Cd, 0, GRB, SRB)
Oper (Op8108, DoSBCD, DB, SmaiB, (inst & 7), DB, DA, CmaiB, 0, GMB, SMB)
Oper (Op8300, DoSBCD, DB, SdB,   (inst & 7), DB, DR, Cd, 1, GRB, SRB)
Oper (Op8308, DoSBCD, DB, SmaiB, (inst & 7), DB, DA, CmaiB, 1, GMB, SMB)
Oper (Op8500, DoSBCD, DB, SdB,   (inst & 7), DB, DR, Cd, 2, GRB, SRB)
Oper (Op8508, DoSBCD, DB, SmaiB, (inst & 7), DB, DA, CmaiB, 2, GMB, SMB)
Oper (Op8700, DoSBCD, DB, SdB,   (inst & 7), DB, DR, Cd, 3, GRB, SRB)
Oper (Op8708, DoSBCD, DB, SmaiB, (inst & 7), DB, DA, CmaiB, 3, GMB, SMB)
Oper (Op8900, DoSBCD, DB, SdB,   (inst & 7), DB, DR, Cd, 4, GRB, SRB)
Oper (Op8908, DoSBCD, DB, SmaiB, (inst & 7), DB, DA, CmaiB, 4, GMB, SMB)
Oper (Op8b00, DoSBCD, DB, SdB,   (inst & 7), DB, DR, Cd, 5, GRB, SRB)
Oper (Op8b08, DoSBCD, DB, SmaiB, (inst & 7), DB, DA, CmaiB, 5, GMB, SMB)
Oper (Op8d00, DoSBCD, DB, SdB,   (inst & 7), DB, DR, Cd, 6, GRB, SRB)
Oper (Op8d08, DoSBCD, DB, SmaiB, (inst & 7), DB, DA, CmaiB, 6, GMB, SMB)
Oper (Op8f00, DoSBCD, DB, SdB,   (inst & 7), DB, DR, Cd, 7, GRB, SRB)
Oper (Op8f08, DoSBCD, DB, SmaiB, (inst & 7), DB, DA, CmaiB, 7, GMB, SMB)

Oper (Op4800, DoNBCD, DB, SdB,   (inst & 7), DB, DR, Cd, 0, GRB, SRB)
Oper (Op4810, DoNBCD, DB, SainB, (inst & 7), DB, DR, Cd, 0, GRB, SRB)
Oper (Op4818, DoNBCD, DB, SaipB, (inst & 7), DB, DR, Cd, 0, GRB, SRB)
Oper (Op4820, DoNBCD, DB, SmaiB, (inst & 7), DB, DR, Cd, 0, GRB, SRB)
Oper (Op4828, DoNBCD, DB, SdaiB, (inst & 7), DB, DR, Cd, 0, GRB, SRB)
Oper (Op4830, DoNBCD, DB, SaixB, (inst & 7), DB, DR, Cd, 0, GRB, SRB)
Oper (Op4838, DoNBCD, DB, SearB, (inst & 7), DB, DR, Cd, 0, GRB, SRB)
