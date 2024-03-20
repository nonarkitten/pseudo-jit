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

#include "emitters.h"
#include "pjit.h"

// @brief Performs an Effective-Address (EA) Load
// @param uint32t** emit  stream to output opcodes
// @param uint8_t   sEA   source size, effective addres and register
// @param uint8_t   dReg  desired destination of the DATA, ignored on register 'loads' already in CPU
// @param uint8_t   iReg  immediate register (r1/r2) and holding place for address
// @param int       isRMW if this is a read-modify-write operation
// @return (uint8_t) the actual register containing the expected data
__attribute__((target("thumb")))
uint8_t emit_EA_Load(uint32_t** emit, uint8_t sEA, uint8_t dReg, uint8_t iReg, int isRMW) {
    int width = 1 << (sEA >> 6);  // 00 = 1, 01 = 2, 10 = 4, 11 = invalid

    // In little endian mode, the physical bus is byteswapped making words read
    // "normally" from the bus, but bytes and long-words will not be correct
    int swap = ((sEA & 0x30) && cpu_state.config.cpu_little_endian);

    if ((sEA & 0x30) && cpu_state.config.cpu_enable_32bits) {
        switch (sEA & 0x38) {
            default:
                return 0xFF;
                
            // Data Register Dx
            case 0x00: {
                if ((sEA & 7) < 2) {  // D0 or D1
                    uint8_t reg = (sEA & 7) + 3;
                    switch (width) {
                        case 1: *(*emit)++ = sxtb(dReg, reg, 0); break;
                        case 2: *(*emit)++ = sxth(dReg, reg, 0); break;
                        case 4: dReg = reg; break;
                    }
                } else {  // D2 to D7
                    uint16_t offset = (sEA & 0x7) * 4 + (4 - width);
                    switch (width) {
                        case 1: *(*emit)++ = ldrsb(dReg, 5, index_imm(1, 0, offset)); break;
                        case 2: *(*emit)++ = ldrsh(dReg, 5, index_imm(1, 0, offset)); break;
                        case 4: *(*emit)++ = ldr(dReg, 5, index_imm(1, 0, offset)); break;
                    }
                }
                return dReg; // avoid swap
            }

            // Address Register Ax
            case 0x08: {
                uint8_t reg = (sEA & 7) + 6;
                switch (width) {
                    case 1: return 0xFF;
                    case 2: *(*emit)++ = sxth(dReg, reg, 0); break;
                    case 4: dReg = reg; break;
                }
                return dReg; // avoid swap
            }

            // Address Direct (Ax)
            case 0x10: 
            case_0x10: {
                uint8_t reg = (sEA & 7) + 6;
                switch (width) {
                    case 1: if(!swap) *(*emit)++ = ldrsb(dReg, reg, 0); break;
                    case 2: *(*emit)++ = ldrsh(dReg, reg, 0); break;
                    case 4: *(*emit)++ = ldr(dReg, reg, 0); break;
                }
                break;
            }

            // Address Post-Incemrent (Ax)+, convert to (Ax) for RMW
            case 0x18: {
                if (isRMW) goto case_0x10;
                uint8_t reg = (sEA & 7) + 6;
                switch (width) {
                    case 1: if(!swap) *(*emit)++ = ldrsb(dReg, reg, index_imm(0, 1, 1)); break;
                    case 2: *(*emit)++ = ldrsh(dReg, reg, index_imm(0, 1, 2)); break;
                    case 4: *(*emit)++ = ldr(dReg, reg, index_imm(0, 1, 4)); break;
                }
                break;
            }

            // Address Pre-Decrement -(Ax)
            case 0x20: {
                uint8_t reg = (sEA & 7) + 6;
                switch (width) {
                    case 1: if(!swap) *(*emit)++ = ldrsb(dReg, reg, index_imm(1, 1, -1)); break;
                    case 2: *(*emit)++ = ldrsh(dReg, reg, index_imm(0, 1, -2)); break;
                    case 4: *(*emit)++ = ldr(dReg, reg, index_imm(0, 1, -4)); break;
                }
                break;
            }

            // Address Indexed or Displacement (Ax, Ix)
            case 0x28:
            case 0x30: {
                uint8_t reg = (sEA & 7) + 6;
                switch (width) {
                    case 1: if(!swap) *(*emit)++ = ldrsb(dReg, reg, index_add_reg(1, 0, iReg)); break;
                    case 2: *(*emit)++ = ldrsh(dReg, reg, index_add_reg(1, 0, iReg)); break;
                    case 4: *(*emit)++ = ldr(dReg, reg, index_add_reg(1, 0, iReg)); break;
                }
                break;
            }

            // Address Absolute or PC Indexed or Displacement (Ix), or Immediate
            case 0x38: {
                if ((sEA & 7) == 4) dReg = iReg;  // Immediate
                else if ((sEA & 7) > 4) return 0xFF;  // Invalid
                else switch (width) {
                    case 1: if(!swap) *(*emit)++ = ldrsb(dReg, iReg, 0); break;
                    case 2: *(*emit)++ = ldrsh(dReg, iReg, 0); break;
                    case 4: *(*emit)++ = ldr(dReg, iReg, 0); break;
                }
                break;
            }
        } // end switch

        if(swap) switch (width) {
            case 1: *(*emit)++ = asr_imm(dReg, dReg, 8);
            case 2: break;
            case 4: *(*emit)++ = ror_imm(dReg, dReg, 16);
        }
        return dReg;
    } else {
        // In 24-bit mode, we need to mask out the high address; for loads that's a lot more
        // work because we'll need to perform the normal address adjustment first and then
        // change the addressing mode to the simple (Ax) form

        switch (sEA & 0x38) {

        }
        *(*emit)++ = bic(r0, dReg, imm(0xFF000000));
        dReg = r0;


    }
}

// @brief Performs an Effective-Address (EA) Store
// @param uint32_t** emit  stream to output opcodes
// @param uint8_t    dEA   destination size, effective addres and register
// @param uint8_t    sReg  source register with the DATA
// @param uint8_t    iReg  immediate register (r1/r2) and holding place for address
// @param int        isRMW if this is a read-modify-write operation
// @return Nothing
__attribute__((target("thumb")))
void emit_EA_Store(uint32_t** emit, uint8_t dEA, uint8_t sReg, uint8_t iReg, int isRMW) {
    int     width = 1 << (dEA >> 6);
    uint8_t dReg  = (dEA & 7);

    // In little endian mode, the physical bus is byteswapped making words read
    // "normally" from the bus, but bytes and long-words will not be correct
    if ((dEA & 0x30) && cpu_state.config.cpu_little_endian) {
        switch (width == 1) {
            case 1: {
                // xor the address LSB to perform "byte swap"
                *(*emit)++ = eor(r0, dReg, imm(1));
                dReg = r0;
                break;
            }
            case 4: {
                // swap the 16-bit parts
                *(*emit)++ = ror_imm(r0, sReg, 16);
                sReg = r0;
                break;
            }
        } // end switch
    }

    // In 24-bit mode, we need to mask out the high address
    if ((dEA & 0x30) && !cpu_state.config.cpu_enable_32bits) {
        *(*emit)++ = bic(r0, dReg, imm(0xFF000000));
        dReg = r0;
    }

    switch (dEA & 0x38) {
        default: return;

        // Data Register Dx
        case 0x00: {
            if (dReg < 2) {  // D0 or D1
                if (sReg == dReg) return;
                dReg += 3;
                switch (width) {
                    case 1: *(*emit)++ = bfi(dReg, sReg, 0, 8); return;
                    case 2: *(*emit)++ = bfi(dReg, sReg, 0, 16); return;
                    case 4: *(*emit)++ = mov(dReg, sReg); return;
                }
            } else {  // D2 to D7
                uint16_t offset = (dEA & 0x7) * 4 + (4 - width);
                switch (width) {
                    case 1: *(*emit)++ = strb(sReg, 5, index_imm(1, 0, offset)); return;
                    case 2: *(*emit)++ = strh(sReg, 5, index_imm(1, 0, offset)); return;
                    case 4: *(*emit)++ = str(sReg, 5, index_imm(1, 0, offset)); return;
                }
            }
            break;
        }

        // Address Register Ax
        case 0x08: {
            if ((width == 4) && (sReg == dReg)) return;
            dReg += 6;
            switch (width) {
                case 2: *(*emit)++ = sxth(dReg, sReg, 0); return;
                case 4: *(*emit)++ = mov(dReg, sReg); return;
            }
            break;
        }

        // Address Direct (Ax)
        case 0x10: {
        case_0x10:
            dReg += 6;
            switch (width) {
                case 1:
                    *(*emit)++ = strb(sReg, dReg, 0);
                    return;
                case 2:
                    *(*emit)++ = strh(sReg, dReg, 0);
                    return;
                case 4:
                    *(*emit)++ = str(sReg, dReg, 0);
                    return;
            }
            break;
        }

        // Address Post-Incemrent (Ax)+
        case 0x18: {
            dReg += 6;
            switch (width) {
                case 1:
                    *(*emit)++ = strb(sReg, dReg, index_imm(0, 1, 1));
                    return;
                case 2:
                    *(*emit)++ =  strh(sReg, dReg, index_imm(0, 1, 2));
                    return;
                case 4:
                    *(*emit)++ =  str(sReg, dReg, index_imm(0, 1, 4));
                    return;
            }
            break;
        }

        // Address Pre-Decrement -(Ax), adjust RMW to (Ax)
        case 0x20: {
            if (isRMW) goto case_0x10;
            dReg += 6;
            switch (width) {
                case 1:
                    *(*emit)++ = strb(sReg, dReg, index_imm(1, 1, -1));
                    return;
                case 2:
                    *(*emit)++ = strh(sReg, dReg, index_imm(1, 1, -2));
                    return;
                case 4:
                    *(*emit)++ = str(sReg, dReg, index_imm(1, 1, -4));
                    return;
            }
            break;
        }

        // Address Indexed or Displacement (Ax, Ix)
        case 0x28:
        case 0x30: {
            dReg += 6;
            switch (width) {
                case 1:
                    *(*emit)++ = strb(sReg, dReg, index_add_reg(1, 0, iReg));
                    return;
                case 2:
                    *(*emit)++ = strh(sReg, dReg, index_add_reg(1, 0, iReg));
                    return;
                case 4:
                    *(*emit)++ = str(sReg, dReg, index_add_reg(1, 0, iReg));
                    return;
            }
            break;
        }

        // Address Absolute or PC Indexed or Displacement (Ix)
        case 0x38: {
            switch (width) {
                case 1:
                    *(*emit)++ = strb(sReg, iReg, 0);
                    return;
                case 2:
                    *(*emit)++ = strh(sReg, iReg, 0);
                    return;
                case 4:
                    *(*emit)++ = str(sReg, iReg, 0);
                    return;
            }
            break;
        }

    }  // end switch
}