/*
 * Copyright (c) 2020-2023 Renee Cousins, the Buffee Project - http://www.buffee.ca
 *
 * This is part of PJIT the Pseudo-JIT 68K emulator.
 *
 * PJIT is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * PJIT is licensed under a Creative Commons
 * Attribution-NonCommercial-ShareAlike 4.0 International License.
 *
 * Under the terms of this license you are free copy and redistribute
 * the material in any medium or format as well as remix, transform,
 * and build upon the material.
 *
 * You must give appropriate credit, provide a link to the license,
 * and indicate if changes were made. You may do so in any reasonable
 * manner, but not in any way that suggests the licensor endorses you
 * or your use.
 *
 * You may not use the material for commercial purposes.
 *
 * If you remix, transform, or build upon the material, you must
 * distribute your contributions under the same license as the original.
 *
 * You may not apply legal terms or technological measures that legally
 * restrict others from doing anything the license permits.
 *
 * Portions of PJIT have been derived from the following:
 *
 *     Castaway (formerly FAST), GPL version 2 License
 *     Copyright (c) 1994-2002 Martin Döring, Joachim Hönig
 *    
 *     Cyclone 68K, GPL version 2 License
 *     Copyright (c) 2004,2011 Dave "FinalDave" Haywood
 *     Copyright (c) 2005-2011 Graûvydas "notaz" Ignotas
 *    
 *     TI StarterWare, modified BSD 3-Clause License
 *     Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/
 *
 *     libbbb, Apache License, Version 2.0
 *     Copyright 2015 University of Applied Sciences Western Switzerland / Fribourg
 */

/*  Almost all 68K instructions fall into a simple
    SOURCE-EA and DESTINATION-EA pattern. In the case
    of MOVE, sEA is bits[5:0] and dEA is bits[11:6].
    With most ALU operations, one EA is bits[5:0] and
    the other EA is bits[11-9] with either a fixed
    direction, or a direction bit in bit[8] where
    0 is Dn op <ea> -> Dn and 1 is <ea> op Dn -> <ea>.


    Register Use
    r0  is our one working register

    r1  is the temporary source address value
        this is loaded before the operand is called
        through the extension words

        when a source is loaded (usually the first
        operation), then this is replaced with the
        source value

        when an opcode has an implicit operand, these
        should be decoded from the opcode and placed
        into r1 before hand with special handling of
        r0 after the ALU is called

        the ALU routines are all set to perform:

            r0 = r0 ALU-OP r1

    r2  is the temporary destination address value
        this is loaded before the operand is called
        through the extension words; for any indexed
        post-increment or absolute address, this will
        be the address we keep to the end; for pre-
        decrement or regular address modes, we'll use
        the CPU registers

        when the destination data is read, r2 will be
        updated with the actual address read from and
        if performing a read-modify-write operation,
        where to store the result.

    r3-r4 are the data register D0 and D1

    r5 is our CPU state data

    r6-r13 are the address registers A0 to A7

    r14 is the Link Register
    r15 is the Program Counter
*/

#include "pjit_emitters.h"

typedef struct {
    unsigned enable_32b_addr : 1;
} config_t;

// @brief Performs an Effective-Address (EA) Load
// @param uint32t** emit  stream to output opcodes
// @param uint8_t   sEA   source size, effective addres and register
// @param uint8_t   dReg  desired destination of the DATA, ignored on register 'loads' already in CPU
// @param uint8_t   iReg  immediate register (r1/r2) and holding place for address
// @param int       isRMW if this is a read-modify-write operation
// @return (uint8_t) the actual register containing the expected data
uint8_t emit_EA_Load(uint32_t** emit, uint8_t sEA, uint8_t dReg, uint8_t iReg, int isRMW) {
    int width = 1 << (sEA >> 6);  // 00 = 1, 01 = 2, 10 = 4, 11 = invalid

    switch (sEA & 0x38) {
        // Data Register Dx
        case 0x00: {
            if ((sEA & 7) < 2) {  // D0 or D1
                uint8_t reg = (sEA & 7) + 3;
                switch (width) {
                    case 1:
                        *(*emit)++ = 0xE6AF0070 | (dReg << 12) | reg;
                        return dReg;  // sxtb dReg, r3/r4
                    case 2:
                        *(*emit)++ = 0xE6BF0070 | (dReg << 12) | reg;
                        return dReg;  // sxth dReg, r3/r4
                    case 4:
                        return reg;  // return r3/r4 directly
                }
            } else {  // D2 to D7
                uint16_t offset = (width == 4) ? ((sEA & 0x7) << 2) : (((sEA & 0x3) << 2) + ((sEA & 0x4) << 6));
                switch (width) {
                    case 1:
                        *(*emit)++ = 0xE1D510D3 | (dReg << 12) | offset;
                        return dReg;  // ldrsb dReg, [r5, #offset + 3]
                    case 2:
                        *(*emit)++ = 0xE1D510F2 | (dReg << 12) | offset;
                        return dReg;  // ldrsh dReg, [r5, #offset + 2]
                    case 4:
                        *(*emit)++ = 0xE5950000 | (dReg << 12) | offset;
                        return dReg;  // ldr   dReg, [r5, #offset]
                }
            }
            break;
        }

        // Address Register Ax
        case 0x08: {
            uint8_t reg = (sEA & 7) + 6;
            switch (width) {
                case 2:
                    *(*emit)++ = 0xE6BF0070 | (dReg << 12) | reg;
                    return dReg;  // sxth dReg, r6-r13
                case 4:
                    return reg;  // return r6-r13 directly
            }
            break;
        }

        // Address Direct (Ax)
        case 0x10: {
        case_0x10:
            uint8_t reg = (sEA & 7) + 6;
            switch (width) {
                case 1:
                    *(*emit)++ = 0xE1D000D0 | (dReg << 12) | (reg << 16);
                    return dReg;  // ldrsb dReg, [Ax]
                case 2:
                    *(*emit)++ = 0xE1D000F0 | (dReg << 12) | (reg << 16);
                    return dReg;  // ldrsh dReg, [Ax]
                case 4:
                    *(*emit)++ = 0xE5900000 | (dReg << 12) | (reg << 16);
                    return dReg;  // ldr   dReg, [Ax]
            }
            break;
        }

        // Address Post-Incemrent (Ax)+, convert to (Ax) for RMW
        case 0x18: {
            if (isRMW) goto case_0x10;
            uint8_t reg = (sEA & 7) + 6;
            switch (width) {
                case 1:
                    *(*emit)++ = 0xE0D000D1 | (dReg << 12) | (reg << 16);
                    return dReg;  // ldrsb dReg, [Ax], #1
                case 2:
                    *(*emit)++ = 0xE0D000F2 | (dReg << 12) | (reg << 16);
                    return dReg;  // ldrsh dReg, [Ax], #2
                case 4:
                    *(*emit)++ = 0xE4900004 | (dReg << 12) | (reg << 16);
                    return dReg;  // ldr   dReg, [Ax], #4
            }
            break;
        }

        // Address Pre-Decrement -(Ax)
        case 0x20: {
            uint8_t reg = (sEA & 7) + 6;
            switch (width) {
                case 1:
                    *(*emit)++ = 0xE17000D1 | (dReg << 12) | (reg << 16);
                    return dReg;  // ldrsb dREg, [Ax, #-1]!
                case 2:
                    *(*emit)++ = 0xE17000F2 | (dReg << 12) | (reg << 16);
                    return dReg;  // ldrsh dREg, [Ax, #-2]!
                case 4:
                    *(*emit)++ = 0xE5300004 | (dReg << 12) | (reg << 16);
                    return dReg;  // ldr   dREg, [Ax, #-4]!
            }
            break;
        }

        // Address Indexed or Displacement (Ax, Ix)
        case 0x28:
        case 0x30: {
            uint8_t reg = (sEA & 7) + 6;
            switch (width) {
                case 1:
                    *(*emit)++ = 0xE19000D0 | (dReg << 12) | (reg << 16) | iReg;
                    return dReg;  // ldrsb dReg, [Ix, Ax]
                case 2:
                    *(*emit)++ = 0xE19000F0 | (dReg << 12) | (reg << 16) | iReg;
                    return dReg;  // ldrsh dReg, [Ix, Ax]
                case 4:
                    *(*emit)++ = 0xE7900000 | (dReg << 12) | (reg << 16) | iReg;
                    return dReg;  // ldr   dReg, [Ix, Ax]
            }
            break;
        }

        // Address Absolute or PC Indexed or Displacement (Ix), or Immediate
        case 0x38: {
            if ((sEA & 7) == 4)
                return iReg;  // Immediate
            else if ((sEA & 7) > 4)
                break;  // Invalid

            switch (width) {
                case 1:
                    *(*emit)++ = 0xE1D000D0 | (dReg << 12) | (iReg << 16);
                    return dReg;  // ldrsb dReg, [Ix]
                case 2:
                    *(*emit)++ = 0xE1D000F0 | (dReg << 12) | (iReg << 16);
                    return dReg;  // ldrsh dReg, [Ix]
                case 4:
                    *(*emit)++ = 0xE5900000 | (dReg << 12) | (iReg << 16);
                    return dReg;  // ldr   dReg, [Ix]
            }
            break;
        }

    }             // end switch
    return 0xFF;  // invalid otherwise
}

// @brief Performs an Effective-Address (EA) Load
// @param uint32_t** emit  stream to output opcodes
// @param uint8_t    dEA   destination size, effective addres and register
// @param uint8_t    sReg  source register with the DATA
// @param uint8_t    iReg  immediate register (r1/r2) and holding place for address
// @param int        isRMW if this is a read-modify-write operation
// @return Nothing
void emit_EA_Store(uint32_t** emit, uint8_t dEA, uint8_t sReg, uint8_t iReg, int isRMW) {
    int     width = 1 << (dEA >> 6);
    uint8_t dReg  = (dEA & 7);

    switch (dEA & 0x38) {
        // Data Register Dx
        case 0x00: {
            if (dReg < 2) {  // D0 or D1
                if (sReg == dReg) return;
                dReg += 3;
                switch (width) {
                    case 1:
                        *(*emit)++ = 0xE7C70010 | (dReg << 12) | sReg;
                        return;  // bfi dReg, sReg, 0, 8
                    case 2:
                        *(*emit)++ = 0xE7CF0010 | (dReg << 12) | sReg;
                        return;  // bfi dReg, sReg, 0, 16
                    case 4:
                        *(*emit)++ = 0xE1A00000 | (dReg << 12) | sReg;
                        return;  // mov dReg, sReg
                }
            } else {  // D2 to D7
                uint16_t offset = (width == 2) ? ((dEA & 0x3) << 2) + ((dEA & 0x4) << 6) : (dEA & 0x7) << 2;
                switch (width) {
                    case 1:
                        *(*emit)++ = 0xE5C50003 | (sReg << 12) | offset;
                        return;  // strb sReg, [r5, #dReg_offset]
                    case 2:
                        *(*emit)++ = 0xE1C500B2 | (sReg << 12) | offset;
                        return;  // strh sReg, [r5, #dReg_offset]
                    case 4:
                        *(*emit)++ = 0xE5850000 | (sReg << 12) | offset;
                        return;  // str  sReg, [r5, #dReg_offset]
                }
            }
            break;
        }

        // Address Register Ax
        case 0x08: {
            if ((width == 4) && (sReg == dReg)) return;
            dReg += 6;
            switch (width) {
                case 2:
                    *(*emit)++ = 0xE6BF0070 | (dReg << 12) | sReg;
                    return;  // sxth dReg, sReg
                case 4:
                    *(*emit)++ = 0xE1A00000 | (dReg << 12) | sReg;
                    return;  // mov dReg, sReg
            }
            break;
        }

        // Address Direct (Ax)
        case 0x10: {
        case_0x10:
            dReg += 6;
            switch (width) {
                case 1:
                    *(*emit)++ = 0xE5C00000 | (sReg << 12) | (dReg << 16);
                    return;  // strb sReg, [dReg]
                case 2:
                    *(*emit)++ = 0xE1C000B0 | (sReg << 12) | (dReg << 16);
                    return;  // strh sReg, [dReg]
                case 4:
                    *(*emit)++ = 0xE5800000 | (sReg << 12) | (dReg << 16);
                    return;  // str  sReg, [dReg]
            }
            break;
        }

        // Address Post-Incemrent (Ax)+
        case 0x18: {
            dReg += 6;
            switch (width) {
                case 1:
                    *(*emit)++ = 0xE4C00001 | (sReg << 12) | (dReg << 16);
                    return;  // strb sReg, [dReg], #1
                case 2:
                    *(*emit)++ = 0xE0C000B2 | (sReg << 12) | (dReg << 16);
                    return;  // strh sReg, [dReg], #2
                case 4:
                    *(*emit)++ = 0xE4800004 | (sReg << 12) | (dReg << 16);
                    return;  // str  sReg, [dReg], #4
            }
            break;
        }

        // Address Pre-Decrement -(Ax), adjust RMW to (Ax)
        case 0x20: {
            if (isRMW) goto case_0x10;
            dReg += 6;
            switch (width) {
                case 1:
                    *(*emit)++ = 0xE5600001 | (sReg << 12) | (dReg << 16);
                    return;  // strb sReg, [dReg, #-1]!
                case 2:
                    *(*emit)++ = 0xE16000B2 | (sReg << 12) | (dReg << 16);
                    return;  // strh sReg, [dReg, #-2]!
                case 4:
                    *(*emit)++ = 0xE5200004 | (sReg << 12) | (dReg << 16);
                    return;  // str  sReg, [dReg, #-4]!
            }
            break;
        }

        // Address Indexed or Displacement (Ax, Ix)
        case 0x28:
        case 0x30: {
            dReg += 6;
            switch (width) {
                case 1:
                    *(*emit)++ = 0xE7C00000 | (sReg << 12) | (dReg << 16) | iReg;
                    return;  // strb sReg, [dReg, iReg]
                case 2:
                    *(*emit)++ = 0xE18000B0 | (sReg << 12) | (dReg << 16) | iReg;
                    return;  // strh sReg, [dReg, iReg]
                case 4:
                    *(*emit)++ = 0xE7800000 | (sReg << 12) | (dReg << 16) | iReg;
                    return;  // str  sReg, [dReg, iReg]
            }
            break;
        }

        // Address Absolute or PC Indexed or Displacement (Ix)
        case 0x38: {
            switch (width) {
                case 1:
                    *(*emit)++ = 0xE5C00000 | (sReg << 12) | (iReg << 16);
                    return;  // strb sReg, [dReg]
                case 2:
                    *(*emit)++ = 0xE1C000B0 | (sReg << 12) | (iReg << 16);
                    return;  // strh sReg, [dReg]
                case 4:
                    *(*emit)++ = 0xE5800000 | (sReg << 12) | (iReg << 16);
                    return;  // str  sReg, [dReg]
            }
            break;
        }

    }  // end switch
}