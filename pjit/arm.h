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
 * 
 *     emu68 (https://github.com/michalsc), Mozilla Public License, v. 2.0
 *     Copyright © 2019 Michal Schulz <michal.schulz@gmx.de>
 */

#ifndef _ARM_H
#define _ARM_H

#include <stdint.h>

typedef enum {
    // Lower case are index
    r0,
    r1,
    r2,
    r3,
    r4,
    r5,
    r6,
    r7,
    r8,
    r9,
    r10,
    r11,
    r12,
    r13,
    r14,
    r15,
    ip  = r12,
    sp  = r13,
    lr  = r14,
    pc  = r15,
    // Upper case are masks
    R0  = 0x0001,
    R1  = 0x0002,
    R2  = 0x0004,
    R3  = 0x0008,
    R4  = 0x0010,
    R5  = 0x0020,
    R6  = 0x0040,
    R7  = 0x0080,
    R8  = 0x0100,
    R9  = 0x0200,
    R10 = 0x0400,
    R11 = 0x0800,
    R12 = 0x1000,
    R13 = 0x2000,
    R14 = 0x4000,
    R15 = 0x8000,
    IP  = R12,
    SP  = R13,
    LR  = R14,
    PC  = R15
} reg_mask_t;

typedef enum {
    ARM_CC_EQ, /* Z=1 */
    ARM_CC_NE, /* Z=0 */
    ARM_CC_CS, /* C=1 */
    ARM_CC_CC, /* C=0 */
    ARM_CC_MI, /* N=1 */
    ARM_CC_PL, /* N=0 */
    ARM_CC_VS, /* V=1 */
    ARM_CC_VC, /* V=0 */
    ARM_CC_HI, /* C=1 & Z=0 */
    ARM_CC_LS, /* C=0 & Z=1 */
    ARM_CC_GE, /* N=V */
    ARM_CC_LT, /* N != V */
    ARM_CC_GT, /* Z=0 && N=V */
    ARM_CC_LE, /* Z=1 || N != V */
    ARM_CC_AL, /* Always */
    ARM_CC_NV, /* Never */
} condition_t;

typedef enum {
    ARM_OP_AND,  // Rd := Rn & Op2
    ARM_OP_EOR,  // Rd := Rn ^ Op2
    ARM_OP_SUB,  // Rd := Rn - Op2
    ARM_OP_RSB,  // Rd := Op2 - Rn
    ARM_OP_ADD,  // Rd := Rn + Op2
    ARM_OP_ADC,  // Rd := Rn + Op2 + C
    ARM_OP_SBC,  // Rd := Rn + Op2 + C - 1
    ARM_OP_RSC,  // Rd := Op2 - Rn + C - 1
    ARM_OP_TST,  // set condition codes on Rn & Op2
    ARM_OP_TEQ,  // set condition codes on Rn | Op2
    ARM_OP_CMP,  // set condition codes on Rn - Op2
    ARM_OP_CMN,  // set condition codes on Rn + Op2
    ARM_OP_ORR,  // Rd := Rn | Op2
    ARM_OP_MOV,  // Rd := Op2
    ARM_OP_BIC,  // Rd := Rn & ~Op2
    ARM_OP_MVN,  // Rd := ~Op2
} alu_t;

typedef enum {
    ARM_SHIFT_LSL,  // X <- [ REGISTER ] <- 0 (X is lost)
    ARM_SHIFT_LSR,  // 0 -> [ REGISTER ] -> X (X is lost)
    ARM_SHIFT_ASR,  // S -> [ REGISTER ] -> X (X is lost, S is sign)
    ARM_SHIFT_ROR,  // L -> [ REGISTER ] -> L (L is given to high bit)
} shift_t;

typedef enum {
    POST_INDEX,
    PRE_INDEX
} index_t;

typedef enum {
    MUL_OP_MUL,
    MUL_OP_MLA,
    MUL_OP_RESERVED2,
    MUL_OP_MLS,
    MUL_OP_UMULL,
    MUL_OP_UMLAL,
    MUL_OP_SMULL,
    MUL_OP_SMLAL,
} mul_t;

typedef enum {
    CPSR_invalid,
    CPSR_s,
    CPSR_f,
    CPSR_fs
} msr_mask_t;

/* Converts generated ARM instruction to little-endian */
#define LE(X) __builtin_bswap32(X)

/* Calculate flexible second operand -- Immediate with Rotation */
static inline uint32_t imm(uint32_t value) {
    int shift;
    for (shift = 0; shift < 16; shift++) {
        if (!(value & 0xffffff00)) break;
        value = (value >> 2) | (value << 30);
    }
    shift &= 0xF;
    value &= 0xFF;
    return 0x02000000 | (shift << 8) | (value);
}
/* Calculate flexible second operand -- Register shifted by another register */
static inline uint32_t shiftreg(uint8_t shiftReg, shift_t shiftType, uint8_t regModified) {
    shiftReg &= 0xF;
    regModified &= 0xF;
    return 0x00000010 | (shiftReg << 8) | ((int)shiftType << 5) | (regModified);
}
/* Calculate flexible second operand -- Register shifted by an immediate */
static inline uint32_t shiftimm(uint8_t value, shift_t shiftType, uint8_t regModified) {
    value &= 0x1F;
    regModified &= 0xF;
    return 0x00000000 | (value << 7) | ((int)shiftType << 5) | (regModified);
}
/* Calculate flexible second operand -- Just our third register (no shift) */
static inline uint32_t reg(uint8_t rm) { return shiftimm(0, ARM_SHIFT_LSL, rm); }

// alu register shift immiate
static inline uint32_t alu_cc(alu_t op, condition_t cc, uint8_t sf, uint8_t d, uint8_t s, uint32_t op2) {
    d &= 0xF;
    s &= 0xF;
    sf &= 1;
    return LE((cc << 28) | (op << 21) | (sf << 20) | (s << 16) | (d << 12) | (op2));
}

// alu conditional operations
// Rd := Rn & Op2
static inline uint32_t and_cc(condition_t cc, uint8_t d, uint8_t s, uint32_t op2) { return alu_cc(ARM_OP_AND, cc, 0, d, s, op2); }
// Rd := Rn ^ Op2
static inline uint32_t eor_cc(condition_t cc, uint8_t d, uint8_t s, uint32_t op2) { return alu_cc(ARM_OP_EOR, cc, 0, d, s, op2); }
// Rd := Rn - Op2
static inline uint32_t sub_cc(condition_t cc, uint8_t d, uint8_t s, uint32_t op2) { return alu_cc(ARM_OP_SUB, cc, 0, d, s, op2); }
// Rd := Op2 - Rn
static inline uint32_t rsb_cc(condition_t cc, uint8_t d, uint8_t s, uint32_t op2) { return alu_cc(ARM_OP_RSB, cc, 0, d, s, op2); }
// Rd := Rn + Op2
static inline uint32_t add_cc(condition_t cc, uint8_t d, uint8_t s, uint32_t op2) { return alu_cc(ARM_OP_ADD, cc, 0, d, s, op2); }
// Rd := Rn + Op2 + C
static inline uint32_t adc_cc(condition_t cc, uint8_t d, uint8_t s, uint32_t op2) { return alu_cc(ARM_OP_ADC, cc, 0, d, s, op2); }
// Rd := Rn + Op2 + C - 1
static inline uint32_t sbc_cc(condition_t cc, uint8_t d, uint8_t s, uint32_t op2) { return alu_cc(ARM_OP_SBC, cc, 0, d, s, op2); }
// Rd := Op2 - Rn + C - 1
static inline uint32_t rsc_cc(condition_t cc, uint8_t d, uint8_t s, uint32_t op2) { return alu_cc(ARM_OP_RSC, cc, 0, d, s, op2); }
// Rd := Rn | Op2
static inline uint32_t orr_cc(condition_t cc, uint8_t d, uint8_t s, uint32_t op2) { return alu_cc(ARM_OP_ORR, cc, 0, d, s, op2); }
// Rd := Op2
static inline uint32_t mov_cc(condition_t cc, uint8_t d, uint32_t op2) { return alu_cc(ARM_OP_MOV, cc, 0, d, 0, op2); }
// Rd := Rn & ~Op2
static inline uint32_t bic_cc(condition_t cc, uint8_t d, uint8_t s, uint32_t op2) { return alu_cc(ARM_OP_BIC, cc, 0, d, s, op2); }
// Rd := ~Op2
static inline uint32_t mvn_cc(condition_t cc, uint8_t d, uint32_t op2) { return alu_cc(ARM_OP_MVN, cc, 0, d, 0, op2); }

// alu non-conditional operations
// Rd := Rn & Op2
static inline uint32_t and(uint8_t d, uint8_t s, uint32_t op2) { return alu_cc(ARM_OP_AND, ARM_CC_AL, 0, d, s, op2); }
// Rd := Rn ^ Op2
static inline uint32_t eor(uint8_t d, uint8_t s, uint32_t op2) { return alu_cc(ARM_OP_EOR, ARM_CC_AL, 0, d, s, op2); }
// Rd := Rn - Op2
static inline uint32_t sub(uint8_t d, uint8_t s, uint32_t op2) { return alu_cc(ARM_OP_SUB, ARM_CC_AL, 0, d, s, op2); }
// Rd := Op2 - Rn
static inline uint32_t rsb(uint8_t d, uint8_t s, uint32_t op2) { return alu_cc(ARM_OP_RSB, ARM_CC_AL, 0, d, s, op2); }
// Rd := Rn + Op2
static inline uint32_t add(uint8_t d, uint8_t s, uint32_t op2) { return alu_cc(ARM_OP_ADD, ARM_CC_AL, 0, d, s, op2); }
// Rd := Rn + Op2 + C
static inline uint32_t adc(uint8_t d, uint8_t s, uint32_t op2) { return alu_cc(ARM_OP_ADC, ARM_CC_AL, 0, d, s, op2); }
// Rd := Rn + Op2 + C - 1
static inline uint32_t sbc(uint8_t d, uint8_t s, uint32_t op2) { return alu_cc(ARM_OP_SBC, ARM_CC_AL, 0, d, s, op2); }
// Rd := Op2 - Rn + C - 1
static inline uint32_t rsc(uint8_t d, uint8_t s, uint32_t op2) { return alu_cc(ARM_OP_RSC, ARM_CC_AL, 0, d, s, op2); }
// Rd := Rn | Op2
static inline uint32_t orr(uint8_t d, uint8_t s, uint32_t op2) { return alu_cc(ARM_OP_ORR, ARM_CC_AL, 0, d, s, op2); }
// Rd := Op2
static inline uint32_t mov(uint8_t d, uint32_t op2) { return alu_cc(ARM_OP_MOV, ARM_CC_AL, 0, d, 0, op2); }
// Rd := Rn & ~Op2
static inline uint32_t bic(uint8_t d, uint8_t s, uint32_t op2) { return alu_cc(ARM_OP_BIC, ARM_CC_AL, 0, d, s, op2); }
// Rd := ~Op2
static inline uint32_t mvn(uint8_t d, uint32_t op2) { return alu_cc(ARM_OP_MVN, ARM_CC_AL, 0, d, 0, op2); }

// alu conditional operations, set-flags
// Rd := Rn & Op2, updates NZ
static inline uint32_t ands_cc(condition_t cc, uint8_t d, uint8_t s, uint32_t op2) { return alu_cc(ARM_OP_AND, cc, 1, d, s, op2); }
// Rd := Rn ^ Op2, updates NZ
static inline uint32_t eors_cc(condition_t cc, uint8_t d, uint8_t s, uint32_t op2) { return alu_cc(ARM_OP_EOR, cc, 1, d, s, op2); }
// Rd := Rn - Op2, updates NZCV
static inline uint32_t subs_cc(condition_t cc, uint8_t d, uint8_t s, uint32_t op2) { return alu_cc(ARM_OP_SUB, cc, 1, d, s, op2); }
// Rd := Op2 - Rn, updates NZCV
static inline uint32_t rsbs_cc(condition_t cc, uint8_t d, uint8_t s, uint32_t op2) { return alu_cc(ARM_OP_RSB, cc, 1, d, s, op2); }
// Rd := Rn + Op2, updates NZCV
static inline uint32_t adds_cc(condition_t cc, uint8_t d, uint8_t s, uint32_t op2) { return alu_cc(ARM_OP_ADD, cc, 1, d, s, op2); }
// Rd := Rn + Op2 + C, updates NZCV
static inline uint32_t adcs_cc(condition_t cc, uint8_t d, uint8_t s, uint32_t op2) { return alu_cc(ARM_OP_ADC, cc, 1, d, s, op2); }
// Rd := Rn + Op2 + C - 1, updates NZCV
static inline uint32_t sbcs_cc(condition_t cc, uint8_t d, uint8_t s, uint32_t op2) { return alu_cc(ARM_OP_SBC, cc, 1, d, s, op2); }
// Rd := Op2 - Rn + C - 1, updates NZCV
static inline uint32_t rscs_cc(condition_t cc, uint8_t d, uint8_t s, uint32_t op2) { return alu_cc(ARM_OP_RSC, cc, 1, d, s, op2); }
// set condition codes on Rn & Op2, updates NZ
static inline uint32_t tst_cc(condition_t cc, uint8_t s, uint32_t op2) { return alu_cc(ARM_OP_TST, cc, 1, 0, s, op2); }
// set condition codes on Rn | Op2, updates NZ
static inline uint32_t teq_cc(condition_t cc, uint8_t s, uint32_t op2) { return alu_cc(ARM_OP_TEQ, cc, 1, 0, s, op2); }
// set condition codes on Rn - Op2, updates NZCV
static inline uint32_t cmp_cc(condition_t cc, uint8_t s, uint32_t op2) { return alu_cc(ARM_OP_CMP, cc, 1, 0, s, op2); }
// set condition codes on Rn + Op2, updates NZCV
static inline uint32_t cmn_cc(condition_t cc, uint8_t s, uint32_t op2) { return alu_cc(ARM_OP_CMN, cc, 1, 0, s, op2); }
// Rd := Rn | Op2, updates NZ
static inline uint32_t orrs_cc(condition_t cc, uint8_t d, uint8_t s, uint32_t op2) { return alu_cc(ARM_OP_ORR, cc, 1, d, s, op2); }
// Rd := Op2, updates NZC
static inline uint32_t movs_cc(condition_t cc, uint8_t d, uint32_t op2) { return alu_cc(ARM_OP_MOV, cc, 1, d, 0, op2); }
// Rd := Rn & ~Op2, updates NZ
static inline uint32_t bics_cc(condition_t cc, uint8_t d, uint8_t s, uint32_t op2) { return alu_cc(ARM_OP_BIC, cc, 1, d, s, op2); }
// Rd := ~Op2, updates NZC
static inline uint32_t mvns_cc(condition_t cc, uint8_t d, uint32_t op2) { return alu_cc(ARM_OP_MVN, cc, 1, d, 0, op2); }

// alu non-conditional operations, set-flags
// Rd := Rn & Op2, updates NZ
static inline uint32_t ands(uint8_t d, uint8_t s, uint32_t op2) { return alu_cc(ARM_OP_AND, ARM_CC_AL, 1, d, s, op2); }
// Rd := Rn ^ Op2, updates NZ
static inline uint32_t eors(uint8_t d, uint8_t s, uint32_t op2) { return alu_cc(ARM_OP_EOR, ARM_CC_AL, 1, d, s, op2); }
// Rd := Rn - Op2, updates NZCV
static inline uint32_t subs(uint8_t d, uint8_t s, uint32_t op2) { return alu_cc(ARM_OP_SUB, ARM_CC_AL, 1, d, s, op2); }
// Rd := Op2 - Rn, updates NZCV
static inline uint32_t rsbs(uint8_t d, uint8_t s, uint32_t op2) { return alu_cc(ARM_OP_RSB, ARM_CC_AL, 1, d, s, op2); }
// Rd := Rn + Op2, updates NZCV
static inline uint32_t adds(uint8_t d, uint8_t s, uint32_t op2) { return alu_cc(ARM_OP_ADD, ARM_CC_AL, 1, d, s, op2); }
// Rd := Rn + Op2 + C, updates NZCV
static inline uint32_t adcs(uint8_t d, uint8_t s, uint32_t op2) { return alu_cc(ARM_OP_ADC, ARM_CC_AL, 1, d, s, op2); }
// Rd := Rn + Op2 + C - 1, updates NZCV
static inline uint32_t sbcs(uint8_t d, uint8_t s, uint32_t op2) { return alu_cc(ARM_OP_SBC, ARM_CC_AL, 1, d, s, op2); }
// Rd := Op2 - Rn + C - 1, updates NZCV
static inline uint32_t rscs(uint8_t d, uint8_t s, uint32_t op2) { return alu_cc(ARM_OP_RSC, ARM_CC_AL, 1, d, s, op2); }
// set condition codes on Rn & Op2, updates NZ
static inline uint32_t tst(uint8_t s, uint32_t op2) { return alu_cc(ARM_OP_TST, ARM_CC_AL, 1, 0, s, op2); }
// set condition codes on Rn | Op2, updates NZ
static inline uint32_t teq(uint8_t s, uint32_t op2) { return alu_cc(ARM_OP_TEQ, ARM_CC_AL, 1, 0, s, op2); }
// set condition codes on Rn - Op2, updates NZCV
static inline uint32_t cmp(uint8_t s, uint32_t op2) { return alu_cc(ARM_OP_CMP, ARM_CC_AL, 1, 0, s, op2); }
// set condition codes on Rn + Op2, updates NZCV
static inline uint32_t cmn(uint8_t s, uint32_t op2) { return alu_cc(ARM_OP_CMN, ARM_CC_AL, 1, 0, s, op2); }
// Rd := Rn | Op2, updates NZ
static inline uint32_t orrs(uint8_t d, uint8_t s, uint32_t op2) { return alu_cc(ARM_OP_ORR, ARM_CC_AL, 1, d, s, op2); }
// Rd := Op2, updates NZC
static inline uint32_t movs(uint8_t d, uint32_t op2) { return alu_cc(ARM_OP_MOV, ARM_CC_AL, 1, d, 0, op2); }
// Rd := Rn & ~Op2, updates NZ
static inline uint32_t bics(uint8_t d, uint8_t s, uint32_t op2) { return alu_cc(ARM_OP_BIC, ARM_CC_AL, 1, d, s, op2); }
// Rd := ~Op2, updates NZC
static inline uint32_t mvns(uint8_t d, uint32_t op2) { return alu_cc(ARM_OP_MVN, ARM_CC_AL, 1, d, 0, op2); }

// MOV signed value for convinience
static inline uint32_t mov_cc_signed(condition_t cc, int sf, uint8_t reg, int32_t val) {
    return (val >= 0) ? alu_cc(ARM_OP_MOV, cc, sf, reg, 0, imm(val)) : alu_cc(ARM_OP_MVN, cc, sf, reg, 0, imm(~val));
}
static inline uint32_t mov_signed(uint8_t reg, int32_t val) {
    return mov_cc_signed(ARM_CC_AL, 0, reg, val);
}
static inline uint32_t movs_signed(uint8_t reg, int8_t val) {
    return mov_cc_signed(ARM_CC_AL, 1, reg, val);
}

// All shifts are just MOV, added for convinience
static inline uint32_t lsl_cc_imm(condition_t cc, uint8_t d, uint8_t s, uint8_t v) { return alu_cc(ARM_OP_MOV, cc, 0, d, 0, shiftimm(v, ARM_SHIFT_LSL, s)); }
static inline uint32_t lsl_imm(uint8_t d, uint8_t s, uint8_t v) { return alu_cc(ARM_OP_MOV, ARM_CC_AL, 0, d, 0, shiftimm(v, ARM_SHIFT_LSL, s)); }
static inline uint32_t lsl_cc_reg(condition_t cc, uint8_t d, uint8_t s, uint8_t r) { return alu_cc(ARM_OP_MOV, cc, 0, d, 0, shiftreg(r, ARM_SHIFT_LSL, s)); }
static inline uint32_t lsl_reg(uint8_t d, uint8_t s, uint8_t r) { return alu_cc(ARM_OP_MOV, ARM_CC_AL, 0, d, 0, shiftreg(r, ARM_SHIFT_LSL, s)); }
static inline uint32_t lsls_cc_imm(condition_t cc, uint8_t d, uint8_t s, uint8_t v) { return alu_cc(ARM_OP_MOV, cc, 1, d, 0, shiftimm(v, ARM_SHIFT_LSL, s)); }
static inline uint32_t lsls_imm(uint8_t d, uint8_t s, uint8_t v) { return alu_cc(ARM_OP_MOV, ARM_CC_AL, 1, d, 0, shiftimm(v, ARM_SHIFT_LSL, s)); }
static inline uint32_t lsls_cc_reg(condition_t cc, uint8_t d, uint8_t s, uint8_t r) { return alu_cc(ARM_OP_MOV, cc, 1, d, 0, shiftreg(r, ARM_SHIFT_LSL, s)); }
static inline uint32_t lsls_reg(uint8_t d, uint8_t s, uint8_t r) { return alu_cc(ARM_OP_MOV, ARM_CC_AL, 1, d, 0, shiftreg(r, ARM_SHIFT_LSL, s)); }

static inline uint32_t lsr_cc_imm(condition_t cc, uint8_t d, uint8_t s, uint8_t v) { return alu_cc(ARM_OP_MOV, cc, 0, d, 0, shiftimm(v, ARM_SHIFT_LSR, s)); }
static inline uint32_t lsr_imm(uint8_t d, uint8_t s, uint8_t v) { return alu_cc(ARM_OP_MOV, ARM_CC_AL, 0, d, 0, shiftimm(v, ARM_SHIFT_LSR, s)); }
static inline uint32_t lsr_cc_reg(condition_t cc, uint8_t d, uint8_t s, uint8_t r) { return alu_cc(ARM_OP_MOV, cc, 0, d, 0, shiftreg(r, ARM_SHIFT_LSR, s)); }
static inline uint32_t lsr_reg(uint8_t d, uint8_t s, uint8_t r) { return alu_cc(ARM_OP_MOV, ARM_CC_AL, 0, d, 0, shiftreg(r, ARM_SHIFT_LSR, s)); }
static inline uint32_t lsrs_cc_imm(condition_t cc, uint8_t d, uint8_t s, uint8_t v) { return alu_cc(ARM_OP_MOV, cc, 1, d, 0, shiftimm(v, ARM_SHIFT_LSR, s)); }
static inline uint32_t lsrs_imm(uint8_t d, uint8_t s, uint8_t v) { return alu_cc(ARM_OP_MOV, ARM_CC_AL, 1, d, 0, shiftimm(v, ARM_SHIFT_LSR, s)); }
static inline uint32_t lsrs_cc_reg(condition_t cc, uint8_t d, uint8_t s, uint8_t r) { return alu_cc(ARM_OP_MOV, cc, 1, d, 0, shiftreg(r, ARM_SHIFT_LSR, s)); }
static inline uint32_t lsrs_reg(uint8_t d, uint8_t s, uint8_t r) { return alu_cc(ARM_OP_MOV, ARM_CC_AL, 1, d, 0, shiftreg(r, ARM_SHIFT_LSR, s)); }

static inline uint32_t asr_cc_imm(condition_t cc, uint8_t d, uint8_t s, uint8_t v) { return alu_cc(ARM_OP_MOV, cc, 0, d, 0, shiftimm(v, ARM_SHIFT_ASR, s)); }
static inline uint32_t asr_imm(uint8_t d, uint8_t s, uint8_t v) { return alu_cc(ARM_OP_MOV, ARM_CC_AL, 0, d, 0, shiftimm(v, ARM_SHIFT_ASR, s)); }
static inline uint32_t asr_cc_reg(condition_t cc, uint8_t d, uint8_t s, uint8_t r) { return alu_cc(ARM_OP_MOV, cc, 0, d, 0, shiftreg(r, ARM_SHIFT_ASR, s)); }
static inline uint32_t asr_reg(uint8_t d, uint8_t s, uint8_t r) { return alu_cc(ARM_OP_MOV, ARM_CC_AL, 0, d, 0, shiftreg(r, ARM_SHIFT_ASR, s)); }
static inline uint32_t asrs_cc_imm(condition_t cc, uint8_t d, uint8_t s, uint8_t v) { return alu_cc(ARM_OP_MOV, cc, 1, d, 0, shiftimm(v, ARM_SHIFT_ASR, s)); }
static inline uint32_t asrs_imm(uint8_t d, uint8_t s, uint8_t v) { return alu_cc(ARM_OP_MOV, ARM_CC_AL, 1, d, 0, shiftimm(v, ARM_SHIFT_ASR, s)); }
static inline uint32_t asrs_cc_reg(condition_t cc, uint8_t d, uint8_t s, uint8_t r) { return alu_cc(ARM_OP_MOV, cc, 1, d, 0, shiftreg(r, ARM_SHIFT_ASR, s)); }
static inline uint32_t asrs_reg(uint8_t d, uint8_t s, uint8_t r) { return alu_cc(ARM_OP_MOV, ARM_CC_AL, 1, d, 0, shiftreg(r, ARM_SHIFT_ASR, s)); }

static inline uint32_t ror_cc_imm(condition_t cc, uint8_t d, uint8_t s, uint8_t v) { return alu_cc(ARM_OP_MOV, cc, 0, d, 0, shiftimm(v, ARM_SHIFT_ROR, s)); }
static inline uint32_t ror_imm(uint8_t d, uint8_t s, uint8_t v) { return alu_cc(ARM_OP_MOV, ARM_CC_AL, 0, d, 0, shiftimm(v, ARM_SHIFT_ROR, s)); }
static inline uint32_t ror_cc_reg(condition_t cc, uint8_t d, uint8_t s, uint8_t r) { return alu_cc(ARM_OP_MOV, cc, 0, d, 0, shiftreg(r, ARM_SHIFT_ROR, s)); }
static inline uint32_t ror_reg(uint8_t d, uint8_t s, uint8_t r) { return alu_cc(ARM_OP_MOV, ARM_CC_AL, 0, d, 0, shiftreg(r, ARM_SHIFT_ROR, s)); }
static inline uint32_t rors_cc_imm(condition_t cc, uint8_t d, uint8_t s, uint8_t v) { return alu_cc(ARM_OP_MOV, cc, 1, d, 0, shiftimm(v, ARM_SHIFT_ROR, s)); }
static inline uint32_t rors_imm(uint8_t d, uint8_t s, uint8_t v) { return alu_cc(ARM_OP_MOV, ARM_CC_AL, 1, d, 0, shiftimm(v, ARM_SHIFT_ROR, s)); }
static inline uint32_t rors_cc_reg(condition_t cc, uint8_t d, uint8_t s, uint8_t r) { return alu_cc(ARM_OP_MOV, cc, 1, d, 0, shiftreg(r, ARM_SHIFT_ROR, s)); }
static inline uint32_t rors_reg(uint8_t d, uint8_t s, uint8_t r) { return alu_cc(ARM_OP_MOV, ARM_CC_AL, 1, d, 0, shiftreg(r, ARM_SHIFT_ROR, s)); }

// Calculate the offset for all branches, loads and store that use an immediate value
static inline int32_t calc_offset(uint32_t current, uint32_t target) { return target - current - 8; }

// Branch immediate
static inline uint32_t b_cc_imm(condition_t cc, int32_t offset) { return LE(0x0a000000 | (cc << 28) | ((offset >> 2) & 0x00ffffff)); }
static inline uint32_t bl_cc_imm(condition_t cc, int32_t offset) { return LE(0x0b000000 | (cc << 28) | ((offset >> 2) & 0x00ffffff)); }
static inline uint32_t blx_imm(condition_t cc, int32_t offset) { return LE(0xfa000000 | ((offset & 0x2) << 23) | ((offset >> 2) & 0x00ffffff)); }
static inline uint32_t b_imm(int32_t offset) { return b_cc_imm(ARM_CC_AL, offset); }
static inline uint32_t bl_imm(int32_t offset) { return bl_cc_imm(ARM_CC_AL, offset); }

// Branch register
static inline uint32_t blx_cc_reg(condition_t cc, uint8_t reg) { return LE(0x012fff30 | (cc << 28) | (reg)); }
static inline uint32_t blx_reg(uint8_t reg) { return blx_cc_reg(ARM_CC_AL, reg); }
static inline uint32_t bx_cc(condition_t cc, uint8_t reg) { return LE(0x012fff10 | (cc << 28) | (reg)); }
static inline uint32_t bx(uint8_t reg) { return bx_cc(ARM_CC_AL, reg); }
static inline uint32_t bx_lr() { return bx(lr); }

// Calculate the LDR/STR index for immediate offset
static inline uint32_t index_imm(index_t pre, int wb, int32_t offset) {
    uint32_t ls = (offset >= 0)
                      ? 0x00800000 | (0xFFF & (uint32_t)offset)
                      : 0x00000000 | (0xFFF & (uint32_t)(-offset));
    if (pre) ls |= 0x01000000;
    if (wb) ls |= 0x00200000;
    return LE(ls);
}
// Calculate the LDR/STR index for adding register and shift offset
static inline uint32_t index_add_reg_shift(index_t pre, int wb, uint8_t regModifed, shift_t shiftType, uint8_t shiftAmmount) {
    uint32_t offset = 0xFFF & ((shiftAmmount << 7) | (shiftType << 5) | (regModifed));
    uint32_t ls     = 0x02800000;
    if (pre) ls |= 0x01000000;
    if (wb) ls |= 0x00200000;
    return LE(ls | offset);
}
static inline uint32_t index_add_reg(index_t pre, int wb, uint8_t reg) {
    return index_add_reg_shift(pre, wb, reg, ARM_SHIFT_LSL, 0);
}
// Calculate the LDR/STR index for subtracting register and shift offset
static inline uint32_t index_sub_reg_shift(index_t pre, int wb, uint8_t regModifed, shift_t shiftType, uint8_t shiftAmmount) {
    uint32_t offset = 0xFFF & ((shiftAmmount << 7) | (shiftType << 5) | (regModifed));
    uint32_t ls     = 0x02000000;
    if (pre) ls |= 0x01000000;
    if (wb) ls |= 0x00200000;
    return LE(ls | offset);
}
static inline uint32_t index_sub_reg(index_t pre, int wb, uint8_t reg) {
    return index_sub_reg_shift(pre, wb, reg, ARM_SHIFT_LSL, 0);
}

static inline uint32_t ldsr_cc(condition_t cc, int byte, int load, uint8_t regData, uint8_t regAddr, uint32_t index) {
    uint32_t ls = load ? 0x04000000 : 0x04100000;
    if (byte) ls |= 0x00400000;
    regData &= 0xF;
    regAddr &= 0xF;
    return LE(ls | (regData << 16) | (regAddr << 12) | index);
}

static inline uint32_t ldr_cc(condition_t cc, uint8_t d, uint8_t a, uint32_t idx) { return ldsr_cc(cc, 0, 1, d, a, idx); }
static inline uint32_t ldr(uint8_t d, uint8_t a, uint32_t idx) { return ldsr_cc(ARM_CC_AL, 0, 1, d, a, idx); }
static inline uint32_t ldrb_cc(condition_t cc, uint8_t d, uint8_t a, uint32_t idx) { return ldsr_cc(cc, 1, 1, d, a, idx); }
static inline uint32_t ldrb(uint8_t d, uint8_t a, uint32_t idx) { return ldsr_cc(ARM_CC_AL, 1, 1, d, a, idx); }

static inline uint32_t str_cc(condition_t cc, uint8_t d, uint8_t a, uint32_t idx) { return ldsr_cc(cc, 0, 0, d, a, idx); }
static inline uint32_t str(uint8_t d, uint8_t a, uint32_t idx) { return ldsr_cc(ARM_CC_AL, 0, 0, d, a, idx); }
static inline uint32_t strb_cc(condition_t cc, uint8_t d, uint8_t a, uint32_t idx) { return ldsr_cc(cc, 1, 0, d, a, idx); }
static inline uint32_t strb(uint8_t d, uint8_t a, uint32_t idx) { return ldsr_cc(ARM_CC_AL, 1, 0, d, a, idx); }

static inline uint32_t ldsr_hw_cc(condition_t cc, int sex, int word, int load, uint8_t regData, uint8_t regAddr, uint32_t index) {
    if (index & 0x02000000) {
        // register does not support shift
        index = (index & 0x01BFF00F) | 0x00000090;
    } else {
        // index limited to 8-bit
        index = (index & 0x01BFF00F) | 0x00400090 | ((index & 0xF0) << 4);
    }
    if (word) index |= 0x200;
    if (sex) index |= 0x400;
    regData &= 0xF;
    regAddr &= 0xF;
    return LE(index | (regData << 16) | (regAddr << 12));
}
static inline uint32_t strh_cc(condition_t cc, uint8_t d, uint8_t a, uint32_t idx) { return ldsr_hw_cc(cc, 0, 1, 0, d, a, idx); }
static inline uint32_t ldrh_cc(condition_t cc, uint8_t d, uint8_t a, uint32_t idx) { return ldsr_hw_cc(cc, 0, 1, 1, d, a, idx); }
static inline uint32_t strh(uint8_t d, uint8_t a, uint32_t idx) { return ldsr_hw_cc(ARM_CC_AL, 0, 1, 0, d, a, idx); }
static inline uint32_t ldrh(uint8_t d, uint8_t a, uint32_t idx) { return ldsr_hw_cc(ARM_CC_AL, 0, 1, 1, d, a, idx); }

// Load Double
static inline uint32_t ldrd_cc(condition_t cc, uint8_t d, uint8_t a, uint32_t idx) { return ldsr_hw_cc(cc, 1, 0, 0, d, a, idx); }
// Load signed-byte
static inline uint32_t ldrsb_cc(condition_t cc, uint8_t d, uint8_t a, uint32_t idx) { return ldsr_hw_cc(cc, 1, 0, 1, d, a, idx); }
// Load Double
static inline uint32_t ldrd(uint8_t d, uint8_t a, uint32_t idx) { return ldsr_hw_cc(ARM_CC_AL, 1, 0, 0, d, a, idx); }
// Load signed-byte
static inline uint32_t ldrsb(uint8_t d, uint8_t a, uint32_t idx) { return ldsr_hw_cc(ARM_CC_AL, 1, 0, 1, d, a, idx); }

// Store double
static inline uint32_t strd_cc(condition_t cc, uint8_t d, uint8_t a, uint32_t idx) { return ldsr_hw_cc(cc, 1, 1, 0, d, a, idx); }
// Load signed-halfword
static inline uint32_t ldrsh_cc(condition_t cc, uint8_t d, uint8_t a, uint32_t idx) { return ldsr_hw_cc(cc, 1, 1, 1, d, a, idx); }
// Store double
static inline uint32_t strd(uint8_t d, uint8_t a, uint32_t idx) { return ldsr_hw_cc(ARM_CC_AL, 1, 1, 0, d, a, idx); }
// Load signed-halfword
static inline uint32_t ldrsh(uint8_t d, uint8_t a, uint32_t idx) { return ldsr_hw_cc(ARM_CC_AL, 1, 1, 1, d, a, idx); }

// Conditionally Multiply
static inline uint32_t mul_cc(condition_t cc, uint8_t dest, uint8_t n, uint8_t m) {
    dest &= 0xF;
    n &= 0xF;
    m &= 0xF;
    return LE(0x00000090 | (cc << 28) | (MUL_OP_MUL << 21) | (dest << 16) | (m << 8) | (n));
}
// Conditionally Multiply, Set-flags
static inline uint32_t muls_cc(condition_t cc, int8_t dest, uint8_t n, uint8_t m) {
    dest &= 0xF;
    n &= 0xF;
    m &= 0xF;
    return LE(0x00000090 | (cc << 28) | (MUL_OP_MUL << 21) | (1 << 20) | (dest << 16) | (m << 8) | (n));
}
// Multiply
static inline uint32_t mul(uint8_t dest, uint8_t n, uint8_t m) { return mul_cc(ARM_CC_AL, dest, n, m); }
// Multiply, Set-flags
static inline uint32_t muls(uint8_t dest, uint8_t n, uint8_t m) { return muls_cc(ARM_CC_AL, dest, n, m); }

// Conditionally Multiply and Accumulate
static inline uint32_t mla_cc(condition_t cc, uint8_t dest, uint8_t acc, uint8_t n, uint8_t m) {
    dest &= 0xF;
    acc &= 0xF;
    n &= 0xF;
    m &= 0xF;
    return LE(0x00000090 | (cc << 28) | (MUL_OP_MLA << 21) | (dest << 16) | (acc << 16) | (m << 8) | (n));
}
// Conditionally Multiply and Accumulate, Set-flags
static inline uint32_t mlas_cc(condition_t cc, uint8_t dest, uint8_t acc, uint8_t n, uint8_t m) {
    dest &= 0xF;
    acc &= 0xF;
    n &= 0xF;
    m &= 0xF;
    return LE(0x00000090 | (cc << 28) | (MUL_OP_MLA << 21) | (1 << 20) | (dest << 16) | (acc << 16) | (m << 8) | (n));
}
// Multiply and Accumulate
static inline uint32_t mla(uint8_t dest, uint8_t acc, uint8_t n, uint8_t m) { return mla_cc(ARM_CC_AL, dest, acc, n, m); }
// Multiply and Accumulate, Set-flags
static inline uint32_t mlas(uint8_t dest, uint8_t acc, uint8_t n, uint8_t m) { return mlas_cc(ARM_CC_AL, dest, acc, n, m); }

// Conditionally Multiply and Subtract
static inline uint32_t mls_cc(condition_t cc, uint8_t dest, uint8_t acc, uint8_t n, uint8_t m) {
    dest &= 0xF;
    acc &= 0xF;
    n &= 0xF;
    m &= 0xF;
    return LE(0x00000090 | (cc << 28) | (MUL_OP_MLS << 21) | (dest << 16) | (acc << 16) | (m << 8) | (n));
}
// Conditionally Multiply and Subtract, Set-flags
static inline uint32_t mlss_cc(condition_t cc, uint8_t dest, uint8_t acc, uint8_t n, uint8_t m) {
    dest &= 0xF;
    acc &= 0xF;
    n &= 0xF;
    m &= 0xF;
    return LE(0x00000090 | (cc << 28) | (MUL_OP_MLS << 21) | (1 << 20) | (dest << 16) | (acc << 16) | (m << 8) | (n));
}
// Multiply and Subtract
static inline uint32_t mls(uint8_t dest, uint8_t acc, uint8_t n, uint8_t m) { return mls_cc(ARM_CC_AL, dest, acc, n, m); }
// Multiply and Subtract, Set-flags
static inline uint32_t mlss(uint8_t dest, uint8_t acc, uint8_t n, uint8_t m) { return mlss_cc(ARM_CC_AL, dest, acc, n, m); }

static inline uint32_t mul_long_cc(condition_t cc, mul_t op, int sf, int8_t destLo, uint8_t destHi, uint8_t n, uint8_t m) {
    destLo &= 0xF;
    destHi &= 0xF;
    n &= 0xF;
    m &= 0xF;
    sf &= 1;
    return LE(0x00000090 | (cc << 28) | (MUL_OP_UMULL << 21) | (sf << 20) | (destHi << 16) | (destLo << 12) | (m << 8) | (n));
}

// Conditionally Multiply Unsigned, Long
static inline uint32_t umull_cc(condition_t cc, uint8_t destLo, uint8_t destHi, uint8_t n, uint8_t m) { return mul_long_cc(cc, MUL_OP_UMULL, 0, destLo, destHi, n, m); }
// Conditionally Multiply Unsigned, Long, Set-flags
static inline uint32_t umulls_cc(condition_t cc, int8_t destLo, uint8_t destHi, uint8_t n, uint8_t m) { return mul_long_cc(cc, MUL_OP_UMULL, 1, destLo, destHi, n, m); }
// Multiply Unsigned, Long
static inline uint32_t umull(uint8_t destLo, uint8_t destHi, uint8_t n, uint8_t m) { return umull_cc(ARM_CC_AL, destLo, destHi, n, m); }
// Multiply Unsigned, Long, Set-flags
static inline uint32_t umulls(uint8_t destLo, uint8_t destHi, uint8_t n, uint8_t m) { return umulls_cc(ARM_CC_AL, destLo, destHi, n, m); }

// Conditionally Multiply and Accumulate Unsigned, Long
static inline uint32_t umlal_cc(condition_t cc, uint8_t destLo, uint8_t destHi, uint8_t n, uint8_t m) { return mul_long_cc(cc, MUL_OP_UMLAL, 0, destLo, destHi, n, m); }
// Conditionally Multiply and Accumulate Unsigned, Long, Set-flags
static inline uint32_t umlals_cc(condition_t cc, int8_t destLo, uint8_t destHi, uint8_t n, uint8_t m) { return mul_long_cc(cc, MUL_OP_UMLAL, 1, destLo, destHi, n, m); }
// Multiply and Accumulate Unsigned, Long
static inline uint32_t umlal(uint8_t destLo, uint8_t destHi, uint8_t n, uint8_t m) { return umlal_cc(ARM_CC_AL, destLo, destHi, n, m); }
// Multiply and Accumulate Unsigned, Long, Set-flags
static inline uint32_t umlals(uint8_t destLo, uint8_t destHi, uint8_t n, uint8_t m) { return umlals_cc(ARM_CC_AL, destLo, destHi, n, m); }

// Conditionally Multiply Signed, Long
static inline uint32_t smull_cc(condition_t cc, uint8_t destLo, uint8_t destHi, uint8_t n, uint8_t m) { return mul_long_cc(cc, MUL_OP_SMULL, 0, destLo, destHi, n, m); }
// Conditionally Multiply Signed, Long, Set-flags
static inline uint32_t smulls_cc(condition_t cc, int8_t destLo, uint8_t destHi, uint8_t n, uint8_t m) { return mul_long_cc(cc, MUL_OP_SMULL, 1, destLo, destHi, n, m); }
// Multiply Signed, Long
static inline uint32_t smull(uint8_t destLo, uint8_t destHi, uint8_t n, uint8_t m) { return umull_cc(ARM_CC_AL, destLo, destHi, n, m); }
// Multiply Signed, Long, Set-flags
static inline uint32_t smulls(uint8_t destLo, uint8_t destHi, uint8_t n, uint8_t m) { return umulls_cc(ARM_CC_AL, destLo, destHi, n, m); }

// Conditionally Multiply and Accumulate Signed, Long
static inline uint32_t smlal_cc(condition_t cc, uint8_t destLo, uint8_t destHi, uint8_t n, uint8_t m) { return mul_long_cc(cc, MUL_OP_SMLAL, 0, destLo, destHi, n, m); }
// Conditionally Multiply and Accumulate Signed, Long, Set-flags
static inline uint32_t smlals_cc(condition_t cc, int8_t destLo, uint8_t destHi, uint8_t n, uint8_t m) { return mul_long_cc(cc, MUL_OP_SMLAL, 1, destLo, destHi, n, m); }
// Multiply and Accumulate Signed, Long
static inline uint32_t smlal(uint8_t destLo, uint8_t destHi, uint8_t n, uint8_t m) { return umlal_cc(ARM_CC_AL, destLo, destHi, n, m); }
// Multiply and Accumulate Signed, Long, Set-flags
static inline uint32_t smlals(uint8_t destLo, uint8_t destHi, uint8_t n, uint8_t m) { return umlals_cc(ARM_CC_AL, destLo, destHi, n, m); }

// Processor State Instructions
// Set endianess to big
static inline uint32_t setend_be() { return LE(0xf1010200); }
// Set endianess to little
static inline uint32_t setend_le() { return LE(0xf1010000); }
// Do nothing
static inline uint32_t nop(void) { return LE(0xe1a00000); }
// Move to Coprocessor from ARM register
static inline uint32_t mcr(uint8_t cp, uint8_t op1, uint8_t rd, uint8_t crn, uint8_t crm, uint8_t op2) {
    return LE(0xee000010 | (op1 << 21) | (crn << 16) | ((rd & 0xF) << 12) | (cp << 8) | (op2 << 5) | crm);
}
// Move to ARM register from Coprocessor
static inline uint32_t mrc(uint8_t cp, uint8_t op1, uint8_t rd, uint8_t crn, uint8_t crm, uint8_t op2) {
    return LE(0xee100010 | (op1 << 21) | (crn << 16) | ((rd & 0xF) << 12) | (cp << 8) | (op2 << 5) | crm);
}
// Move to Register from Special register
static inline uint32_t mrs(uint8_t reg) {
    return LE(0xe10f0000 | ((reg & 0xF) << 12));
}
// Move to Special register from ARM register
static inline uint32_t msr(uint8_t reg, msr_mask_t mask) {
    return LE(0xe120f000 | (reg & 0xF) | (mask << 16));
}
// Supervisor call
static inline uint32_t svc_cc(condition_t cc, uint32_t exception) {
    return LE(0x0F000000 | (cc << 28) | exception);
}
static inline uint32_t svc(uint32_t exception) { return svc_cc(ARM_CC_AL, exception); }
// Undefined instruction
static inline uint32_t udf(uint16_t imm) {
    return LE(0xe7f000f0 | (imm & 0x0f) | ((imm & 0xfff0) << 4));
}
// Halt
static inline uint32_t hlt_cc(condition_t cc, uint16_t immediate) {
    return LE(0x01000070 | (cc << 28) | ((immediate & 0xFFF0) << 4) | (immediate & 0xF));
}
static inline uint32_t hlt(uint16_t immediate) { return svc_cc(ARM_CC_AL, immediate); }

// Bit-field Instructions
// Signed Bit-field Extract
static inline uint32_t sbfx_cc(condition_t cc, uint8_t reg, uint8_t s, uint8_t lsb, uint8_t width) {
    return LE(0x07a00050 | (cc << 28) | (reg << 12) | (lsb << 7) | ((width - 1) << 16) | s);
}
static inline uint32_t sbfx(uint8_t reg, uint8_t s, uint8_t lsb, uint8_t width) { return sbfx_cc(ARM_CC_AL, reg, s, lsb, width); }
// Unsigned Bit-field Extract
static inline uint32_t ubfx_cc(condition_t cc, uint8_t reg, uint8_t s, uint8_t lsb, uint8_t width) {
    return LE(0x07e00050 | (cc << 28) | (reg << 12) | (lsb << 7) | ((width - 1) << 16) | s);
}
static inline uint32_t ubfx(uint8_t reg, uint8_t s, uint8_t lsb, uint8_t width) { return ubfx_cc(ARM_CC_AL, reg, s, lsb, width); }
// Bit-field Clear
static inline uint32_t bfc_cc(condition_t cc, uint8_t reg, uint8_t lsb, uint8_t width) {
    return LE(0x07c0001f | (cc << 28) | ((reg & 0xF) << 12) | ((lsb & 0x1F) << 7) | (((lsb + width - 1) & 0x1F) << 16));
}
static inline uint32_t bfc(uint8_t reg, uint8_t lsb, uint8_t width) { return bfc_cc(ARM_CC_AL, reg, lsb, width); }
// Conditionally Bit-field Insert
static inline uint32_t bfi_cc(condition_t cc, uint8_t reg, uint8_t s, uint8_t lsb, uint8_t width) {
    return LE(0x07c00010 | (cc << 28) | ((reg & 0xF) << 12) | ((lsb & 0x1F) << 7) | (((lsb + width - 1) & 0x1F) << 16) | s);
}
// Bit-field Insert
static inline uint32_t bfi(uint8_t reg, uint8_t s, uint8_t lsb, uint8_t width) { return bfi_cc(ARM_CC_AL, reg, s, lsb, width); }
// Conditionally Count Leading Zeroes
static inline uint32_t clz_cc(condition_t cc, uint8_t rd, uint8_t rm) {
    return LE(0x016f0f10 | (cc << 28) | ((rd & 0xF) << 12) | (rm & 0xF));
}
// Count Leading Zeroes
static inline uint32_t clz(uint8_t rd, uint8_t rm) { return clz_cc(ARM_CC_AL, rd, rm); }

// Block Move Instructions
static inline uint32_t ldst_m(condition_t cc, int pre, int up, int s, int wb, int load, uint8_t rn, uint16_t registers) {
    uint32_t op = 0x08000000 | ((rn & 0xF) << 16) | registers;
    if (pre) op |= 1 << 24;
    if (up) op |= 1 << 23;
    if (s) op |= 1 << 22;
    if (wb) op |= 1 << 21;
    if (load) op |= 1 << 20;
    return LE(op);
}
// Conditionally Store Multiple
static inline uint32_t stm_cc(condition_t cc, uint8_t rd, uint16_t registers) { return ldst_m(cc, 0, 0, 0, 0, 0, rd, registers); }
// Store Multiple
static inline uint32_t stm(uint8_t rd, uint16_t registers) { return ldst_m(ARM_CC_AL, 0, 0, 0, 0, 0, rd, registers); }
// Conditionally Load Multiple
static inline uint32_t ldm_cc(condition_t cc, uint8_t rd, uint16_t registers) { return ldst_m(cc, 0, 0, 0, 0, 1, rd, registers); }
// Load Multiple
static inline uint32_t ldm(uint8_t rd, uint16_t registers) { return ldst_m(ARM_CC_AL, 0, 0, 0, 0, 1, rd, registers); }
// Push Registers to stack
static inline uint32_t push(uint16_t registers) { return ldst_m(ARM_CC_AL, 1, 0, 0, 1, 0, 13, registers); }
// Pop Registers from stack
static inline uint32_t pop(uint16_t registers) { return ldst_m(ARM_CC_AL, 0, 1, 0, 1, 1, 13, registers); }

// Conditionally 16-bit Move to top half (leave lower half_
static inline uint32_t movt_cc(condition_t cc, uint8_t reg, uint16_t val) {
    reg &= 0xF;
    uint8_t  imm4  = val >> 12;
    uint16_t imm12 = val & 0xFFF;
    return LE(0x03400000 | (cc << 28) | (imm4 << 16) | (imm12) | (reg << 12));
}
// 16-bit Move to top half (leave lower half_
static inline uint32_t movt(uint8_t reg, uint16_t val) { return movt_cc(ARM_CC_AL, reg, val); }
// Conditionally 16-bit Move word, zero extend
static inline uint32_t movw_cc(condition_t cc, uint8_t reg, uint16_t val) {
    reg &= 0xF;
    uint8_t  imm4  = val >> 12;
    uint16_t imm12 = val & 0xFFF;
    return LE(0x03000000 | (cc << 28) | (imm4 << 16) | (imm12) | (reg << 12));
}
// 16-bit Move word, zero extend
static inline uint32_t movw(uint8_t reg, uint16_t val) { return movw_cc(ARM_CC_AL, reg, val); }
// Conditionally Reverse byte order (change endianess)
static inline uint32_t rev_cc(condition_t cc, uint8_t d, uint8_t s) {
    return LE(0x06bf0f30 | (cc << 28) | (d << 12) | s);
}
// Reverse byte order (change endianess)
static inline uint32_t rev(uint8_t d, uint8_t s) { return rev_cc(ARM_CC_AL, d, s); }

// Conditionally Reverse bit order
static inline uint32_t rbit_cc(condition_t cc, uint8_t d, uint8_t s) {
    return LE(0x06FF0F30 | (cc << 28) | (d << 12) | s);
}
// Reverse byte order (change endianess)
static inline uint32_t rbit(uint8_t d, uint8_t s) { return rev_cc(ARM_CC_AL, d, s); }



// Sign-Extend and transfer
// Conditionally Sign-Extend and transfer byte
static inline uint32_t sxtb_cc(condition_t cc, uint8_t d, uint8_t s, uint8_t rot) {
    return LE(0x06af0070 | (cc << 28) | (d << 12) | (s) | (rot << 10));
}
// Sign-Extend and transfer byte
static inline uint32_t sxtb(uint8_t d, uint8_t s, uint8_t rot) { return sxtb_cc(ARM_CC_AL, d, s, rot); }
// Conditionally Sign-Extend and transfer halfword
static inline uint32_t sxth_cc(condition_t cc, uint8_t d, uint8_t s, uint8_t rot) {
    return LE(0x06bf0070 | (cc << 28) | (d << 12) | (s) | (rot << 10));
}
// Sign-Extend and transfer halfword
static inline uint32_t sxth(uint8_t d, uint8_t s, uint8_t rot) { return sxth_cc(ARM_CC_AL, d, s, rot); }

// Unsigned-Extend and transfer
// Conditionally Unsigned-Extend and transfer byte
static inline uint32_t uxtb_cc(condition_t cc, uint8_t d, uint8_t s, uint8_t rot) {
    return LE(0x06ef0070 | (cc << 28) | (d << 12) | (s) | (rot << 10));
}
// Unsigned-Extend and transfer byte
static inline uint32_t uxtb(uint8_t d, uint8_t s, uint8_t rot) { return uxtb_cc(ARM_CC_AL, d, s, rot); }
// Conditionally Unsigned-Extend and transfer halfword
static inline uint32_t uxth_cc(condition_t cc, uint8_t d, uint8_t s, uint8_t rot) {
    return LE(0x06ff0070 | (cc << 28) | (d << 12) | (s) | (rot << 10));
}
// Unsigned-Extend and transfer halfword
static inline uint32_t uxth(uint8_t d, uint8_t s, uint8_t rot) { return uxth_cc(ARM_CC_AL, d, s, rot); }

// Division
// Signed divide
static inline uint32_t sdiv_cc(condition_t cc, uint8_t d, uint8_t dividend, uint8_t divisor) {
    return LE(0x0710f010 | (cc << 28) | (d << 16) | (divisor << 8) | dividend);
}
// Signed divide
static inline uint32_t sdiv(uint8_t d, uint8_t dividend, uint8_t divisor) { return sdiv_cc(ARM_CC_AL, d, dividend, divisor); }
// Unsigned divide
static inline uint32_t udiv_cc(condition_t cc, uint8_t d, uint8_t dividend, uint8_t divisor) {
    return LE(0x0730f010 | (cc << 28) | (d << 16) | (divisor << 8) | dividend);
}
// Unsigned divide
static inline uint32_t udiv(uint8_t d, uint8_t dividend, uint8_t divisor) { return udiv_cc(ARM_CC_AL, d, dividend, divisor); }

#endif /* _ARM_H */