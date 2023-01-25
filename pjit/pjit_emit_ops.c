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

#include "pjit.h"
#include "pjit_emitters.h"

// Performs an Effective-Address (EA) Load
// Parameters
//  emit        stream to output opcodes
//  sEA         source size, effective addres and register
//  dReg        desired destination of the DATA, ignored on register 'loads' already in CPU
//  iReg        immediate register (r1/r2) and holding place for address
//  isRMW       if this is a read-modify-write operation
//
// Return Value
//  returns the actual register containing the expected data
//
extern uint8_t emit_EA_Load(uint32_t** emit, uint8_t sEA, uint8_t dReg, uint8_t iReg, int isRMW);

// Performs an Effective-Address (EA) Load
// Parameters
//  emit        stream to output opcodes
//  dEA         destination size, effective addres and register
//  sReg        source register with the DATA
//  iReg        immediate register (r1/r2) and holding place for address
//  isRMW       if this is a read-modify-write operation
//
// Return Value
//  if the Store was able to set condition code this returns the OPCODE
//  otherwise this will return 0
//
extern uint32_t emit_EA_Store(uint32_t** emit, uint8_t dEA, uint8_t sReg, uint8_t iReg, int isRMW);

#define IMPLICIT  (0b00111111)
#define IMMEDIATE (0b00111100)

#define NO_FLAGS  (0x20)

#pragma GCC diagnostic push
#pragma GCC diagnostic error "-Wswitch"

typedef enum {
    // Do nothing
    ALU_OP_NOP,

    // Adders
    ALU_OP_ADD,
    ALU_OP_ADDX,
    ALU_OP_SUB,
    ALU_OP_SUBX,
    ALU_OP_CMP,

    // Logical
    ALU_OP_AND,
    ALU_OP_EOR,
    ALU_OP_OR,

    // Multipliers
    // ALU_OP_DIVSW,
    // ALU_OP_DIVUW,
    ALU_OP_MULSW,
    ALU_OP_MULUW,

    // Shifts
    ALU_OP_ASL,
    ALU_OP_ASR,
    ALU_OP_LSL,
    ALU_OP_LSR,
    ALU_OP_ROL,
    ALU_OP_ROR,
    ALU_OP_ROXL,
    ALU_OP_ROXR,

    // Unary Ops
    ALU_OP_CLR,
    ALU_OP_NEG,
    ALU_OP_NEGX,
    ALU_OP_NOT,

    // Bitwise
    ALU_OP_BCHG,
    ALU_OP_BCLR,
    ALU_OP_BSET,
    ALU_OP_BTST,

    // Miscellaneous
    // ALU_OP_EXG,
    ALU_OP_EXTBW,
    ALU_OP_EXTWL,
    ALU_OP_SWAP,
    ALU_OP_TAS,
    ALU_OP_TST,

} ALU_OP_t;

/***
 *      _   _      _                   _____                 _   _
 *     | | | | ___| |_ __   ___ _ __  |  ___|   _ _ __   ___| |_(_) ___  _ __  ___
 *     | |_| |/ _ \ | '_ \ / _ \ '__| | |_ | | | | '_ \ / __| __| |/ _ \| '_ \/ __|
 *     |  _  |  __/ | |_) |  __/ |    |  _|| |_| | | | | (__| |_| | (_) | | | \__ \
 *     |_| |_|\___|_| .__/ \___|_|    |_|   \__,_|_| |_|\___|\__|_|\___/|_| |_|___/
 *                  |_|
 */
// @brief emit supervisor exception
static void emit_SVC(uint32_t** emit, int exception) {
    *(*emit)++ = nop();
    *(*emit)++ = svc(exception);
}
// @brief check if we're in supervisor mode, exception if not
static void emit_is_SVC(uint32_t** emit) {
    // r2 should hold SR
    *(*emit)++ = tst(r2, imm(0x2000));
    *(*emit)++ = svc_cc(ARM_CC_NE, PRIV);
}
// @brief load our X flag into the C flag
static void emit_load_X(uint32_t** emit) {
    // restore X -> C
    *(*emit)++ = ldrb(r0, r5, index_imm(1, 0, 1 + OFFSETOF(cpu_t, sr)));
    *(*emit)++ = rsbs(r0, r0, imm(0xf));
}
// @brief save out C flag back into the X
static void emit_save_X(uint32_t** emit) {
    // put C back into X, leave everything else
    *(*emit)++ = ldrb(r0, r5, index_imm(1, 0, (1 + OFFSETOF(cpu_t, sr))));
    *(*emit)++ = orr_cc(ARM_CC_CS, r0, r0, imm(0x10));
    *(*emit)++ = bic_cc(ARM_CC_CC, r0, r0, imm(0x10));
    *(*emit)++ = strb(r0, r5, index_imm(1, 0, (1 + OFFSETOF(cpu_t, sr))));
}
// @brief convert a value in r1 (e.g., 3) into a mask (e.g., 1<<3)
static void emit_src_bit_to_mask(uint32_t** emit) {
    // convert source 'r1' to 'bit position' and pre-test
    *(*emit)++ = mov(r0, imm(1));
    *(*emit)++ = lsl_reg(r1, r1, r0);
    *(*emit)++ = tst(r1, r2);
}
// @brief given a 68K SR value in reg, set ARM CPSR flags
static void emit_68k_to_arm_cc(uint32_t** emit, uint8_t reg) {
    // recompose flags -> MCR, reg has ...xnzvc
    *(*emit)++ = rbit(r0, reg);       // cvznx...0000
    *(*emit)++ = bfi(r0, reg, 2, 2);  // cvznx...00nz
    *(*emit)++ = ror_imm(r0, r0, 2);  // nzcvznx...00
    *(*emit)++ = msr(CPSR_f, r0);     // msr  CPSR_f, r0
}
// @brief get the ARM CPSR flags into the low 4-bit of reg
static void emit_arm_to_68k_cc(uint32_t** emit, uint8_t reg) {
    // get CURRENT condition flags (do NOT affect X)
    *(*emit)++ = bic(reg, r0, imm(0xF));
    *(*emit)++ = orr_cc(ARM_CC_CS, reg, reg, imm(0x01));
    *(*emit)++ = orr_cc(ARM_CC_VS, reg, reg, imm(0x02));
    *(*emit)++ = orr_cc(ARM_CC_EQ, reg, reg, imm(0x04));
    *(*emit)++ = orr_cc(ARM_CC_MI, reg, reg, imm(0x07));
}
// @brief load the 68K SR into reg
static void emit_load_SR(uint32_t** emit, uint8_t reg) {
    *(*emit)++ = ldrh(reg, r5, index_imm(1, 0, OFFSETOF(cpu_t, sr)));
    emit_arm_to_68k_cc(emit, reg);
}
// @brief save the 68K SR in reg to cpu_t state
static void emit_save_SR(uint32_t** emit, uint8_t reg) {
    // put reg -> sr
    *(*emit)++ = strh(reg, r5, index_imm(1, 0, OFFSETOF(cpu_t, sr)));
    emit_68k_to_arm_cc(emit, reg);
}
// @brief load the 68K CCR into reg
static void emit_load_CR(uint32_t** emit, uint8_t reg) {
    *(*emit)++ = ldrb(reg, r5, index_imm(1, 0, 1 + OFFSETOF(cpu_t, sr)));
    emit_arm_to_68k_cc(emit, reg);
}
// @brief save the 68K CCR in reg to cpu_t state
static void emit_save_CR(uint32_t** emit, uint8_t reg) {
    // put reg -> sr
    *(*emit)++ = strb(reg, r5, index_imm(1, 0, 1 + OFFSETOF(cpu_t, sr)));
    emit_68k_to_arm_cc(emit, reg);
}

// @brief Main arithmetic routine; does not add BX LR
// @param uint32_t** emit stram to output opcodes
// @param ALU_OP_t arithmetic operation to perform
// @param regS source (unmodifed) register
// @param regD destination (modified) register, or with NO_FLAGS to suppress setting flags
// @return Nothing
void emit_ALU(uint32_t** emit, ALU_OP_t op, uint8_t regS, uint8_t regD) {
    uint32_t opcode = 0;
    uint32_t cond   = 0;

    // call with regD | 0x10 to disable setting flags
    uint32_t is_addr_reg = (regD > 5);
    regD &= 0x1F;

    switch (op) {
        // Do nothing
        case ALU_OP_NOP:
            opcode = nop();
            break;

        // Adders
        case ALU_OP_ADD:
            cond   = 1;
            opcode = add(regD, regS, 0);
            break;
        case ALU_OP_ADDX:
            cond   = 1;
            opcode = adc(regD, regS, 0);
            break;
        case ALU_OP_SUB:
            cond   = 1;
            opcode = rsb(regD, regS, 0);
            break;
        case ALU_OP_SUBX:
            cond   = 1;
            opcode = rsc(regD, regS, 0);
            break;
        case ALU_OP_CMP:
            cond   = 1;
            opcode = cmp(regD, reg(regS));
            break;

        // Logical
        case ALU_OP_AND:
            cond   = 1;
            opcode = and(regD, regS, 0);
            break;
        case ALU_OP_EOR:
            cond   = 1;
            opcode = eor(regD, regS, 0);
            break;
        case ALU_OP_OR:
            cond   = 1;
            opcode = orr(regD, regS, 0);
            break;

        // Unary
        case ALU_OP_CLR:
            cond   = 1;
            opcode = mov(regD, imm(0));
            break;
        case ALU_OP_NEG:
            cond   = 1;
            opcode = rsb(regD, regD, 0);
            break;
        case ALU_OP_NEGX:
            cond   = 1;
            opcode = rsc(regD, regD, 0);
            break;
        case ALU_OP_NOT:
            cond   = 1;
            opcode = mvn(regD, regD);
            break;

        // Shifts
        case ALU_OP_ASL:
            cond   = 1;
            opcode = lsl_reg(regD, regD, regS);
            break;
        case ALU_OP_ASR:
            cond   = 1;
            opcode = asr_reg(regD, regD, regS);
            break;
        case ALU_OP_LSL:
            cond   = 1;
            opcode = lsl_reg(regD, regD, regS);
            break;
        case ALU_OP_LSR:
            cond   = 1;
            opcode = lsr_reg(regD, regD, regS);
            break;
        case ALU_OP_ROL:
            cond       = 1;
            *(*emit)++ = rsb(regS, regS, 31);
        case ALU_OP_ROR:
            cond   = 1;
            opcode = ror_reg(regD, regD, regS);
            break;
        case ALU_OP_ROXL:
            break;
        case ALU_OP_ROXR:
            break;

        // Multipliers
        // case ALU_OP_DIVSW:
        //     break;
        // case ALU_OP_DIVUW:
        //     break;
        case ALU_OP_MULSW:
            *(*emit)++ = sxth(regD, regD, 0);
            *(*emit)++ = sxth(regS, regS, 0);
            *(*emit)++ = mul(regD, regD, regS);
            break;
        case ALU_OP_MULUW:
            *(*emit)++ = uxth(regD, regD, 0);
            *(*emit)++ = uxth(regS, regS, 0);
            *(*emit)++ = mul(regD, regD, regS);
            break;

        // Bitwise
        case ALU_OP_BCHG:
            cond   = 0;
            opcode = eor(regD, regS, 0);
            break;
        case ALU_OP_BCLR:
            cond   = 0;
            opcode = bic(regD, regS, 0);
            break;
        case ALU_OP_BSET:
            cond   = 0;
            opcode = orr(regD, regS, 0);
            break;
        case ALU_OP_BTST:
            cond = 0;
            break;

        // Miscellaneous
        case ALU_OP_EXTBW:
            opcode = sxtb(regD, regS, 0);
            break;
        case ALU_OP_EXTWL:
            opcode = sxth(regD, regS, 0);
            break;
        case ALU_OP_SWAP:
            opcode = ror_imm(regD, regD, 16);
            break;
        case ALU_OP_TAS:
            break;
        case ALU_OP_TST:
            break;
    }

    // add the 's' flag
    if (cond && (regD < 5)) opcode |= (1 << 20);
    *(*emit)++ = opcode;

    // sign-extend do not set flags
    if ((op != ALU_OP_EXTBW) && (op != ALU_OP_EXTWL))
        *(*emit)++ = cmp(regD, 0);

    // fix up our reverse-subtraction
    if ((op == ALU_OP_SUB) || (op == ALU_OP_SUBX))
        *(*emit)++ = rsb(regD, regS, 0);

    return opcode;
}
#pragma GCC diagnostic pop

// @brief wrapper for ADDI, ANDI, CMPI, EORI, ORI, SUBI; adds BX LR
void emit_IMD_ALU(uint32_t** emit, uint16_t opcode, ALU_OP_t op) {
    uint8_t dEA  = (opcode & 0xFF);
    uint8_t regD = emit_EA_Load(emit, dEA, 0, 2, 1);
    emit_ALU(emit, op, 1, regD);
    if (op != ALU_OP_CMP) emit_EA_Store(emit, dEA, regD, 2, 1);
    *(*emit)++ = bx(lr);
}
// @brief wrapper for ADD, AND, CMP, EOR, OR, SUB; adds BX LR
void emit_EA_ALU(uint32_t** emit, uint16_t opcode, ALU_OP_t op) {
    uint8_t sEA = (opcode & 0xC0), dEA = (opcode & 0xC0);
    if ((op == ALU_OP_CMP) || !(op == ALU_OP_EOR) || !(opcode & 0x0100)) {
        // m->r
        dEA |= (opcode & 0x0E00) >> 9;
        sEA |= (opcode & 0x003F);
    } else {
        // r->m
        sEA |= (opcode & 0x0E00) >> 9;
        dEA |= (opcode & 0x003F);
    }
    uint8_t regS = emit_EA_Load(emit, sEA, 1, 1, 0);
    uint8_t regD = emit_EA_Load(emit, dEA, 0, 2, 1);
    emit_ALU(emit, op, regS, regD);
    if (op != ALU_OP_CMP) emit_EA_Store(emit, dEA, regD, 2, 1);
    *(*emit)++ = bx(lr);
}
// @brief wrapper for ABCD, ADDX, SBCD, SUBX; adds BX LR
void emit_BCD_XOP_ALU(uint32_t** emit, uint16_t opcode, ALU_OP_t op) {
    uint8_t sEA = opcode & 0xC7;
    uint8_t dEA = ((opcode >> 9) & 7) | (opcode & 0xC0);
    if (opcode & 0x0004) {
        sEA |= 0x20, dEA |= 0x20;
    }
    emit_load_X(emit);
    uint8_t regS = emit_EA_Load(emit, sEA, 1, 1, 0);
    uint8_t regD = emit_EA_Load(emit, dEA, 0, 2, 1);
    emit_ALU(emit, op, regS, regD);
    emit_EA_Store(emit, dEA, regD, 2, 1);
    emit_save_X(emit);
    *(*emit)++ = bx(lr);
}
// @brief wrapper for CMP, NEG, NOT; adds BX LR
void emit_UNARY_EA_ALU(uint32_t** emit, uint16_t opcode, ALU_OP_t op) {
    uint8_t dEA  = opcode & 0xFF;
    uint8_t regD = emit_EA_Load(emit, dEA, 0, 2, 1);
    emit_ALU(emit, op, 0, regD);
    emit_EA_Store(emit, dEA, regD, 2, 1);
    *(*emit)++ = bx(lr);
}
// @brief wrapper for NEGX, NBCD; adds BX LR
void emit_BCD_XOP_EA_ALU(uint32_t** emit, uint16_t opcode, ALU_OP_t op) {
    uint8_t dEA = opcode & 0xFF;
    emit_load_X(emit);
    uint8_t regD = emit_EA_Load(emit, dEA, 0, 2, 1);
    emit_ALU(emit, op, 0, regD);
    emit_EA_Store(emit, dEA, regD, 2, 1);
    emit_save_X(emit);
    *(*emit)++ = bx(lr);
}
// @brief perform an immediate or register (form 2) shift
static void emit_Mem_Shift(uint32_t** emit, uint16_t opcode, ALU_OP_t op) {
    uint8_t dEA   = opcode & 0x3F;
    uint8_t regD  = emit_EA_Load(emit, dEA, 0, 2, 1);
    uint8_t value = (opcode & 0x0E00) >> 9;
    *(*emit)++    = 0xE3A01000 | (value);  // mov r1, #1
    emit_ALU(emit, op, 1, regD);           // D = D <<>> S
    emit_EA_Store(emit, dEA, regD, 2, 1);
    *(*emit)++ = bx(lr);
}
// @brief perform a memory (form 1) shift by one
static void emit_ImmReg_Shift(uint32_t** emit, uint16_t opcode, ALU_OP_t op) {
    uint8_t regS, value = (opcode & 0x0E00) >> 9;
    if (opcode & 0x0020) {  // Register shit
        regS = emit_EA_Load(emit, value, 1, 1, 0);
    } else {  // Immediate Shift
        if (!value) value = 8;
        *(*emit)++ = 0xE3A01000 | (value);  // mov r1, value
        regS       = 1;
    }
    uint8_t regD = emit_EA_Load(emit, (opcode & 0xC7), 0, 2, 1);
    emit_ALU(emit, op, regS, regD);  // D = D <<>> S
    emit_EA_Store(emit, (opcode & 0xC7), regD, 2, 1);
    *(*emit)++ = bx(lr);
}
// @brief common routine for handling bitwise operators
static void emit_Bit_Op(uint32_t** emit, uint16_t opcode, ALU_OP_t op) {
    uint8_t regS = emit_EA_Load(emit, (opcode & 0x0100) ? ((opcode >> 9) & 7) : 0x3C, 1, 1, 0);
    *(*emit)++   = mov(r0, imm(1));
    *(*emit)++   = lsl_reg(regS, regS, r0);
    uint8_t regD = emit_EA_Load(emit, opcode & 0x3F, 0, 2, 1);
    emit_ALU(emit, op, regS, regD);
    emit_EA_Store(emit, opcode & 0x3F, regD, 2, 1);
    *(*emit)++ = bx(lr);
}

extern void handle_ABCD(uint32_t opccode);
extern void handle_DIVS(uint32_t opccode);
extern void handle_DIVU(uint32_t opccode);
extern void handle_NBCD(uint32_t opccode);
extern void handle_ROXd(uint32_t opccode);
extern void handle_SBCD(uint32_t opccode);

/***
 *       ___                      _        _____           _ _   _
 *      / _ \ _ __   ___ ___   __| | ___  | ____|_ __ ___ (_) |_| |_ ___ _ __ ___
 *     | | | | '_ \ / __/ _ \ / _` |/ _ \ |  _| | '_ ` _ \| | __| __/ _ \ '__/ __|
 *     | |_| | |_) | (_| (_) | (_| |  __/ | |___| | | | | | | |_| ||  __/ |  \__ \
 *      \___/| .__/ \___\___/ \__,_|\___| |_____|_| |_| |_|_|\__|\__\___|_|  |___/
 *           |_|
 */

void emit_ABCD(uint32_t** emit, uint16_t opcode) {
    *(*emit)++ = movw(r0, opcode);
    *(*emit)++ = bl_imm(calc_offset(*emit, handle_ABCD));
}
void emit_ADDBW(uint32_t** emit, uint16_t opcode) {
    emit_EA_ALU(emit, opcode, ALU_OP_ADD);
}
void emit_ADDL(uint32_t** emit, uint16_t opcode) {
    emit_EA_ALU(emit, opcode, ALU_OP_ADD);
}
void emit_ADDAL(uint32_t** emit, uint16_t opcode) {
    emit_EA_ALU(emit, opcode, ALU_OP_ADD);
}
void emit_ADDAW(uint32_t** emit, uint16_t opcode) {
    emit_EA_ALU(emit, opcode, ALU_OP_ADD);
}
void emit_ADDIBW(uint32_t** emit, uint16_t opcode) {
    emit_IMD_ALU(emit, opcode, ALU_OP_ADD);
}
void emit_ADDIL(uint32_t** emit, uint16_t opcode) {
    emit_IMD_ALU(emit, opcode, ALU_OP_ADD);
}
void emit_ADDQBW(uint32_t** emit, uint16_t opcode) {
    uint8_t value = (opcode & 0x0E00) >> 9;
    if (!value) value = 8;
    *(*emit)++ = 0xE3A01000 | (value);  // mov r1, value
    emit_IMD_ALU(emit, opcode, ALU_OP_ADD);
}
void emit_ADDQL(uint32_t** emit, uint16_t opcode) {
    uint8_t value = (opcode & 0x0E00) >> 9;
    if (!value) value = 8;
    *(*emit)++ = 0xE3A01000 | (value);  // mov r1, value
    emit_IMD_ALU(emit, opcode, ALU_OP_ADD);
}
void emit_ADDW(uint32_t** emit, uint16_t opcode) {
    emit_EA_ALU(emit, opcode, ALU_OP_ADD);
}
void emit_ADDXBW(uint32_t** emit, uint16_t opcode) {
    emit_BCD_XOP_ALU(emit, opcode, ALU_OP_ADDX);
}
void emit_ADDXL(uint32_t** emit, uint16_t opcode) {
    emit_BCD_XOP_ALU(emit, opcode, ALU_OP_ADDX);
}
void emit_ALINE(uint32_t** emit, uint16_t opcode) {
    return emit_SVC(emit, LINE_A);
}
void emit_ANDBW(uint32_t** emit, uint16_t opcode) {
    emit_EA_ALU(emit, opcode, ALU_OP_AND);
}
void emit_ANDL(uint32_t** emit, uint16_t opcode) {
    emit_EA_ALU(emit, opcode, ALU_OP_AND);
}
void emit_ANDIBW(uint32_t** emit, uint16_t opcode) {
    emit_IMD_ALU(emit, opcode, ALU_OP_AND);
}
void emit_ANDIL(uint32_t** emit, uint16_t opcode) {
    emit_IMD_ALU(emit, opcode, ALU_OP_AND);
}
void emit_ANDI_TO_CCR(uint32_t** emit, uint16_t opcode) {
    emit_load_CR(emit, 2);  // r2=sr, r1=imm
    emit_ALU(emit, ALU_OP_AND, 1, 2);
    emit_save_CR(emit, 2);  // put r2-> cpu_t and CPSR
    *(*emit)++ = bx(lr);
}
void emit_ANDI_TO_SR(uint32_t** emit, uint16_t opcode) {
    emit_load_SR(emit, 2);  // r2=sr, r1=imm
    emit_is_SVC(emit);
    emit_ALU(emit, ALU_OP_AND, 1, 2);
    emit_save_SR(emit, 2);  // put r2-> cpu_t and CPSR
    *(*emit)++ = bx(lr);
}
void emit_ANDW(uint32_t** emit, uint16_t opcode) {
    emit_EA_ALU(emit, opcode, ALU_OP_AND);
}
void emit_ASdBW(uint32_t** emit, uint16_t opcode) {
    emit_ImmReg_Shift(emit, opcode, (opcode & 0x0100) ? ALU_OP_ASL : ALU_OP_ASR);
}
void emit_ASdL(uint32_t** emit, uint16_t opcode) {
    emit_ImmReg_Shift(emit, opcode, (opcode & 0x0100) ? ALU_OP_ASL : ALU_OP_ASR);
}
void emit_ASd(uint32_t** emit, uint16_t opcode) {
    emit_Mem_Shift(emit, opcode, (opcode & 0x0100) ? ALU_OP_ASL : ALU_OP_ASR);
}
void emit_Bcc(uint32_t** emit, uint16_t opcode) {
#warning UNIMPLEMENTED
}
void emit_BCHG(uint32_t** emit, uint16_t opcode) {
    emit_Bit_Op(emit, opcode, ALU_OP_BCHG);
}
void emit_BCLR(uint32_t** emit, uint16_t opcode) {
    emit_Bit_Op(emit, opcode, ALU_OP_BCLR);
}
void emit_BRA(uint32_t** emit, uint16_t opcode) {
#warning UNIMPLEMENTED
}
void emit_BSET(uint32_t** emit, uint16_t opcode) {
    emit_Bit_Op(emit, opcode, ALU_OP_BSET);
}
void emit_BSR(uint32_t** emit, uint16_t opcode) {
#warning UNIMPLEMENTED
}
void emit_BTST(uint32_t** emit, uint16_t opcode) {
    emit_Bit_Op(emit, opcode, ALU_OP_BTST);
}
void emit_CHK(uint32_t** emit, uint16_t opcode) {
#warning UNIMPLEMENTED
}
void emit_CLRBW(uint32_t** emit, uint16_t opcode) {
    emit_UNARY_EA_ALU(emit, opcode, ALU_OP_CLR);
}
void emit_CLRL(uint32_t** emit, uint16_t opcode) {
    emit_UNARY_EA_ALU(emit, opcode, ALU_OP_CLR);
}
void emit_CMPBW(uint32_t** emit, uint16_t opcode) {
    emit_EA_ALU(emit, opcode, ALU_OP_CMP);
}
void emit_CMPL(uint32_t** emit, uint16_t opcode) {
    emit_EA_ALU(emit, opcode, ALU_OP_CMP);
}
void emit_CMPAL(uint32_t** emit, uint16_t opcode) {
    emit_EA_ALU(emit, opcode, ALU_OP_CMP);
}
void emit_CMPAW(uint32_t** emit, uint16_t opcode) {
    emit_EA_ALU(emit, opcode, ALU_OP_CMP);
}
void emit_CMPB(uint32_t** emit, uint16_t opcode) {
    emit_EA_ALU(emit, opcode, ALU_OP_CMP);
}
void emit_CMPIBW(uint32_t** emit, uint16_t opcode) {
    emit_IMD_ALU(emit, opcode, ALU_OP_CMP);
}
void emit_CMPIL(uint32_t** emit, uint16_t opcode) {
    emit_IMD_ALU(emit, opcode, ALU_OP_CMP);
}
void emit_CMPML(uint32_t** emit, uint16_t opcode) {
    uint8_t dEA = 0x18 | (opcode & 0xC0) | ((opcode >> 9) & 0x7);
    uint8_t sEA = 0x18 | (opcode & 0xC7);
    uint8_t regS = emit_EA_Load(emit, sEA, 1, 1, 0);
    uint8_t regD = emit_EA_Load(emit, dEA, 0, 2, 1);
    emit_ALU(emit, ALU_OP_CMP, regS, regD);
    *(*emit)++ = bx(lr);    
}
void emit_CMPMB(uint32_t** emit, uint16_t opcode) {
    emit_CMPML(emit, opcode);
}
void emit_CMPMW(uint32_t** emit, uint16_t opcode) {
    emit_CMPML(emit, opcode);
}
void emit_CMPW(uint32_t** emit, uint16_t opcode) {
    emit_EA_ALU(emit, opcode, ALU_OP_CMP);
}
void emit_DBcc(uint32_t** emit, uint16_t opcode) {
#warning UNIMPLEMENTED
}
void emit_DBRA(uint32_t** emit, uint16_t opcode) {
#warning UNIMPLEMENTED
}
void emit_DIVS(uint32_t** emit, uint16_t opcode) {
    *(*emit)++ = movw(r0, opcode);
    *(*emit)++ = bl_imm(calc_offset(*emit, handle_DIVS));
}
void emit_DIVU(uint32_t** emit, uint16_t opcode) {
    *(*emit)++ = movw(r0, opcode);
    *(*emit)++ = bl_imm(calc_offset(*emit, handle_DIVU));
}
void emit_EORBW(uint32_t** emit, uint16_t opcode) {
    emit_EA_ALU(emit, opcode, ALU_OP_EOR);
}
void emit_EORL(uint32_t** emit, uint16_t opcode) {
    emit_EA_ALU(emit, opcode, ALU_OP_EOR);
}
void emit_EORIBW(uint32_t** emit, uint16_t opcode) {
    emit_IMD_ALU(emit, opcode, ALU_OP_EOR);
}
void emit_EORIL(uint32_t** emit, uint16_t opcode) {
    emit_IMD_ALU(emit, opcode, ALU_OP_EOR);
}
void emit_EORI_TO_CCR(uint32_t** emit, uint16_t opcode) {
    emit_load_CR(emit, 2);  // r2=sr, r1=imm
    emit_ALU(emit, ALU_OP_EOR, 1, 2);
    emit_save_CR(emit, 2);  // put r2-> cpu_t and CPSR
    *(*emit)++ = bx(lr);
}
void emit_EORI_TO_SR(uint32_t** emit, uint16_t opcode) {
    emit_load_SR(emit, 2);  // r2=sr, r1=imm
    emit_is_SVC(emit);
    emit_ALU(emit, ALU_OP_EOR, 1, 2);
    emit_save_SR(emit, 2);  // put r2-> cpu_t and CPSR
    *(*emit)++ = bx(lr);
}
void emit_EXG(uint32_t** emit, uint16_t opcode) {
    // If the exchange is between data and address registers,
    // this field always specifies the data register.
    uint8_t Rx = (opcode >> 9) & 7;
    // If the exchange is between data and address registers,
    // this field always specifies the address register.
    uint8_t Ry = (opcode >> 0) & 7;

    switch ((opcode >> 3) & 0x1F) {
        default:
        case 0x08:
            break;  // data
        case 0x09:
            Rx |= 0x8;  // fallthru
        case 0x11:
            Ry |= 0x8;
            break;  // address
    }

    // if we're swapping between D2-D* and any of D0/D1, A0-A7, use SWP
    if ((Rx > 1 && Rx < 8) || (Ry > 1 && Ry < 8)) {
        if (Rx > 1 && Rx < 8) {
            *(*emit)++ = ldr(r0, r5, index_imm(1, 0, Rx * 4));
            if (Ry > 1 && Ry < 8) {
                *(*emit)++ = ldr(r1, r5, index_imm(1, 0, Ry * 4));
                *(*emit)++ = str(r1, r5, index_imm(1, 0, Rx * 4));
                *(*emit)++ = str(r0, r5, index_imm(1, 0, Ry * 4));
            } else {
                Ry         = (Ry < 2) ? (Ry + 3) : (Ry - 2);
                *(*emit)++ = str(Ry, r5, index_imm(1, 0, Rx * 4));
                *(*emit)++ = mov(Ry, r0);
            }
        } else {
            Rx         = (Rx < 2) ? (Rx + 3) : (Rx - 2);
            *(*emit)++ = mov(r0, Rx);
            *(*emit)++ = ldr(Rx, r5, index_imm(1, 0, Ry * 4));
            *(*emit)++ = ldr(r0, r5, index_imm(1, 0, Ry * 4));
        }
    }
    // if we're swapping among D0/D1, A0-A7 then we need three moves
    else {
        Rx         = (Rx < 2) ? (Rx + 3) : (Rx - 2);
        Ry         = (Ry < 2) ? (Ry + 3) : (Ry - 2);
        *(*emit)++ = mov(r0, Rx);
        *(*emit)++ = mov(Rx, Ry);
        *(*emit)++ = mov(Ry, r0);
    }
    *(*emit)++ = bx(lr);
}
void emit_EXTL(uint32_t** emit, uint16_t opcode) {
    emit_IMD_ALU(emit, opcode, ALU_OP_EXTWL);
}
void emit_EXTW(uint32_t** emit, uint16_t opcode) {
    emit_IMD_ALU(emit, opcode, ALU_OP_EXTBW);
}
void emit_FLINE(uint32_t** emit, uint16_t opcode) {
    emit_SVC(emit, LINE_F);
}
void emit_ILLEGAL(uint32_t** emit, uint16_t opcode) {
    emit_SVC(emit, ILLINSTR);
}
void emit_JMP(uint32_t** emit, uint16_t opcode) {
#warning UNIMPLEMENTED
}
void emit_JSR(uint32_t** emit, uint16_t opcode) {
#warning UNIMPLEMENTED
}
void emit_LEA(uint32_t** emit, uint16_t opcode) {
#warning UNIMPLEMENTED
}
void emit_LINK(uint32_t** emit, uint16_t opcode) {
#warning UNIMPLEMENTED
}
void emit_LSdBW(uint32_t** emit, uint16_t opcode) {
    emit_ImmReg_Shift(emit, opcode, (opcode & 0x0100) ? ALU_OP_LSL : ALU_OP_LSR);
}
void emit_LSdL(uint32_t** emit, uint16_t opcode) {
    emit_ImmReg_Shift(emit, opcode, (opcode & 0x0100) ? ALU_OP_LSL : ALU_OP_LSR);
}
void emit_LSd(uint32_t** emit, uint16_t opcode) {
    emit_Mem_Shift(emit, opcode, (opcode & 0x0100) ? ALU_OP_LSL : ALU_OP_LSR);
}
void emit_MOVE_SR_TO(uint32_t** emit, uint16_t opcode) {
    emit_load_SR(emit, 1);
    uint8_t dEA = 0x40 | (opcode & 0x3F);
    emit_EA_Store(emit, dEA, 1, 1, 0);
}
void emit_MOVE_TO_CCR(uint32_t** emit, uint16_t opcode) {
    uint8_t sEA  = opcode & 0x3F;
    uint8_t regS = emit_EA_Load(emit, sEA, 1, 1, 0);
    emit_save_CR(emit, regS);  // put r2-> cpu_t and CPSR
    *(*emit)++ = bx(lr);
}
void emit_MOVE_TO_SR(uint32_t** emit, uint16_t opcode) {
    emit_load_SR(emit, 2);  // r2=sr, r1=imm
    emit_is_SVC(emit);
    uint8_t sEA  = 0x40 | (opcode & 0x3F);
    uint8_t regS = emit_EA_Load(emit, sEA, 1, 1, 0);
    emit_save_SR(emit, regS);  // put r2-> cpu_t and CPSR
    *(*emit)++ = bx(lr);
}
void emit_MOVE_TO_USP(uint32_t** emit, uint16_t opcode) {
#warning UNIMPLEMENTED
}
void emit_MOVE_USP_TO(uint32_t** emit, uint16_t opcode) {
#warning UNIMPLEMENTED
}
void emit_MOVEAL(uint32_t** emit, uint16_t opcode) {
    uint8_t sEA  = 0x80 | (opcode & 0x3F);
    uint8_t regD = 6 + ((opcode & 0x0E00) >> 9) & 0x7;
    emit_EA_Load(emit, sEA, regD, 1, 0);
    *(*emit)++ = bx(lr);
}
void emit_MOVEAW(uint32_t** emit, uint16_t opcode) {
    uint8_t sEA  = 0x40 | (opcode & 0x3F);
    uint8_t regD = 6 + ((opcode & 0x0E00) >> 9) & 0x7;
    emit_EA_Load(emit, sEA, regD, 1, 0);
    *(*emit)++ = bx(lr);
}
void emit_MOVEB(uint32_t** emit, uint16_t opcode) {
    uint8_t sEA  = 0x00 | (opcode & 0x3F);
    uint8_t dEA  = 0x00 | ((opcode & 0x0E00) >> 9) | ((opcode & 0x01C0) >> 3);
    uint8_t regS = emit_EA_Load(emit, sEA, 0, 1, 0);
    *(*emit)++   = 0xE3500000 | (regS << 12);  // cmp reg, #0
    emit_EA_Store(emit, dEA, regS, 2, 0);
    *(*emit)++ = bx(lr);
}
void emit_MOVEM(uint32_t** emit, uint16_t opcode) {
#warning UNIMPLEMENTED
}
void emit_MOVEP(uint32_t** emit, uint16_t opcode) {
#warning UNIMPLEMENTED
}
void emit_MOVEQ(uint32_t** emit, uint16_t opcode) {
    int n = (int8_t)opcode;
    if (n > 0)
        *(*emit)++ = 0xE3A01000 | (n & 0xFFF);  // mov r1, value
    else
        *(*emit)++ = 0xE3E01000 | (~n & 0xFFF);  // mov r1, -value
    emit_EA_Store(emit, (opcode & 0x0E00) >> 9, 1, 2, 0);
    *(*emit)++ = bx(lr);
}
void emit_MOVEW(uint32_t** emit, uint16_t opcode) {
    uint8_t sEA  = 0x40 | (opcode & 0x3F);
    uint8_t dEA  = 0x40 | ((opcode & 0x0E00) >> 9) | ((opcode & 0x01C0) >> 3);
    uint8_t regS = emit_EA_Load(emit, sEA, 0, 1, 0);
    *(*emit)++   = 0xE3500000 | (regS << 12);  // cmp reg, #0
    emit_EA_Store(emit, dEA, regS, 2, 0);
    *(*emit)++ = bx(lr);
}
void emit_MOVEL(uint32_t** emit, uint16_t opcode) {
    uint8_t sEA  = 0x80 | (opcode & 0x3F);
    uint8_t dEA  = 0x80 | ((opcode & 0x0E00) >> 9) | ((opcode & 0x01C0) >> 3);
    uint8_t regD = (dEA < 2) ? (3 + dEA) : 0;
    uint8_t regS = emit_EA_Load(emit, sEA, regD, 1, 0);
    *(*emit)++   = 0xE3500000 | (regS << 12);  // cmp reg, #0
    emit_EA_Store(emit, dEA, regS, 2, 0);
    *(*emit)++ = bx(lr);
}
void emit_MULS(uint32_t** emit, uint16_t opcode) {
    emit_EA_ALU(emit, (opcode & 0xFEFF), ALU_OP_MULSW);
}
void emit_MULU(uint32_t** emit, uint16_t opcode) {
    emit_EA_ALU(emit, (opcode & 0xFEFF), ALU_OP_MULUW);
}
void emit_NBCD(uint32_t** emit, uint16_t opcode) {
    *(*emit)++ = movw(r0, opcode);
    *(*emit)++ = bl_imm(calc_offset(*emit, handle_NBCD));
}
void emit_NEGBW(uint32_t** emit, uint16_t opcode) {
    emit_UNARY_EA_ALU(emit, opcode, ALU_OP_NEG);
}
void emit_NEGL(uint32_t** emit, uint16_t opcode) {
    emit_UNARY_EA_ALU(emit, opcode, ALU_OP_NEG);
}
void emit_NEGXBW(uint32_t** emit, uint16_t opcode) {
    emit_BCD_XOP_EA_ALU(emit, opcode, ALU_OP_NEGX);
}
void emit_NEGXL(uint32_t** emit, uint16_t opcode) {
    emit_BCD_XOP_EA_ALU(emit, opcode, ALU_OP_NEGX);
}
void emit_NOP(uint32_t** emit, uint16_t opcode) {
    *(*emit)++ = nop();
    *(*emit)++ = bx(lr);
}
void emit_NOTBW(uint32_t** emit, uint16_t opcode) {
    emit_UNARY_EA_ALU(emit, opcode, ALU_OP_NOT);
}
void emit_NOTL(uint32_t** emit, uint16_t opcode) {
    emit_UNARY_EA_ALU(emit, opcode, ALU_OP_NOT);
}
void emit_ORBW(uint32_t** emit, uint16_t opcode) {
    emit_EA_ALU(emit, opcode, ALU_OP_OR);
}
void emit_ORL(uint32_t** emit, uint16_t opcode) {
    emit_EA_ALU(emit, opcode, ALU_OP_OR);
}
void emit_ORIBW(uint32_t** emit, uint16_t opcode) {
    emit_IMD_ALU(emit, opcode, ALU_OP_OR);
}
void emit_ORIL(uint32_t** emit, uint16_t opcode) {
    emit_IMD_ALU(emit, opcode, ALU_OP_OR);
}
void emit_ORI_TO_CCR(uint32_t** emit, uint16_t opcode) {
    emit_load_CR(emit, 2);  // r2=sr, r1=imm
    emit_ALU(emit, ALU_OP_OR, 1, 2);
    emit_save_CR(emit, 2);  // put r2-> cpu_t and CPSR
    *(*emit)++ = bx(lr);
}
void emit_ORI_TO_SR(uint32_t** emit, uint16_t opcode) {
    emit_load_SR(emit, 2);  // r2=sr, r1=imm
    emit_is_SVC(emit);
    emit_ALU(emit, ALU_OP_OR, 1, 2);
    emit_save_SR(emit, 2);  // put r2-> cpu_t and CPSR
    *(*emit)++ = bx(lr);
}
void emit_ORW(uint32_t** emit, uint16_t opcode) {
    emit_EA_ALU(emit, opcode, ALU_OP_OR);
}
void emit_PEA(uint32_t** emit, uint16_t opcode) {
#warning UNIMPLEMENTED
}
void emit_RESET(uint32_t** emit, uint16_t opcode) {
    emit_SVC(emit, RESET_SP);
}
void emit_ROdBW(uint32_t** emit, uint16_t opcode) {
    emit_ImmReg_Shift(emit, opcode, (opcode & 0x0100) ? ALU_OP_ROL : ALU_OP_ROR);
}
void emit_ROdL(uint32_t** emit, uint16_t opcode) {
    emit_ImmReg_Shift(emit, opcode, (opcode & 0x0100) ? ALU_OP_ROL : ALU_OP_ROR);
}
void emit_ROd(uint32_t** emit, uint16_t opcode) {
    emit_Mem_Shift(emit, opcode, (opcode & 0x0100) ? ALU_OP_ROL : ALU_OP_ROR);
}
void emit_ROXdBW(uint32_t** emit, uint16_t opcode) {
    *(*emit)++ = movw(r0, opcode);
    *(*emit)++ = bl_imm(calc_offset(*emit, handle_ROXd));
}
void emit_ROXdL(uint32_t** emit, uint16_t opcode) {
    *(*emit)++ = movw(r0, opcode);
    *(*emit)++ = bl_imm(calc_offset(*emit, handle_ROXd));
}
void emit_ROXd(uint32_t** emit, uint16_t opcode) {
    emit_Mem_Shift(emit, opcode, (opcode & 0x0100) ? ALU_OP_ROXL : ALU_OP_ROXR);
}
void emit_RTE(uint32_t** emit, uint16_t opcode) {
#warning UNIMPLEMENTED
}
void emit_RTR(uint32_t** emit, uint16_t opcode) {
#warning UNIMPLEMENTED
}
void emit_RTS(uint32_t** emit, uint16_t opcode) {
#warning UNIMPLEMENTED
}
void emit_SBCD(uint32_t** emit, uint16_t opcode) {
    *(*emit)++ = movw(r0, opcode);
    *(*emit)++ = bl_imm(calc_offset(*emit, handle_SBCD));
}
void emit_Scc(uint32_t** emit, uint16_t opcode) {
#warning UNIMPLEMENTED
}
void emit_STOP(uint32_t** emit, uint16_t opcode) {
    emit_load_SR(emit, 2);
    emit_is_SVC(emit);
    emit_save_SR(emit, 1);
    *(*emit)++ = hlt(0);
    *(*emit)++ = bx(lr);
}
void emit_SUBBW(uint32_t** emit, uint16_t opcode) {
    emit_EA_ALU(emit, opcode, ALU_OP_SUB);
}
void emit_SUBL(uint32_t** emit, uint16_t opcode) {
    emit_EA_ALU(emit, opcode, ALU_OP_SUB);
}
void emit_SUBAL(uint32_t** emit, uint16_t opcode) {
    emit_EA_ALU(emit, opcode, ALU_OP_SUB);
}
void emit_SUBAW(uint32_t** emit, uint16_t opcode) {
    emit_EA_ALU(emit, opcode, ALU_OP_SUB);
}
void emit_SUBIBW(uint32_t** emit, uint16_t opcode) {
    emit_IMD_ALU(emit, opcode, ALU_OP_SUB);
}
void emit_SUBIL(uint32_t** emit, uint16_t opcode) {
    emit_IMD_ALU(emit, opcode, ALU_OP_SUB);
}
void emit_SUBQBW(uint32_t** emit, uint16_t opcode) {
    uint8_t value = (opcode & 0x0E00) >> 9;
    if (!value) value = 8;
    *(*emit)++ = 0xE3A01000 | (value);  // mov r1, value
    emit_IMD_ALU(emit, opcode, ALU_OP_SUB);
}
void emit_SUBQL(uint32_t** emit, uint16_t opcode) {
    uint8_t value = (opcode & 0x0E00) >> 9;
    if (!value) value = 8;
    *(*emit)++ = 0xE3A01000 | (value);  // mov r1, value
    emit_IMD_ALU(emit, opcode, ALU_OP_SUB);
}
void emit_SUBW(uint32_t** emit, uint16_t opcode) {
    emit_EA_ALU(emit, opcode, ALU_OP_SUB);
}
void emit_SUBXBW(uint32_t** emit, uint16_t opcode) {
    emit_BCD_XOP_ALU(emit, opcode, ALU_OP_SUBX);
}
void emit_SUBXL(uint32_t** emit, uint16_t opcode) {
    emit_BCD_XOP_ALU(emit, opcode, ALU_OP_SUBX);
}
void emit_SWAP(uint32_t** emit, uint16_t opcode) {
    emit_IMD_ALU(emit, opcode, ALU_OP_SWAP);
}
void emit_TAS(uint32_t** emit, uint16_t opcode) {
#warning UNIMPLEMENTED
}
void emit_TRAP(uint32_t** emit, uint16_t opcode) {
    return emit_SVC(emit, TRAP0 + (opcode & 0xF));
}
void emit_TRAPV(uint32_t** emit, uint16_t opcode) {
    *(*emit)++ = 0xEF000000 | TRAPV;
    *(*emit)++ = bx(lr);
}
void emit_TSTBW(uint32_t** emit, uint16_t opcode) {
    emit_IMD_ALU(emit, opcode, ALU_OP_TST);
}
void emit_TSTL(uint32_t** emit, uint16_t opcode) {
    emit_IMD_ALU(emit, opcode, ALU_OP_TST);
}
void emit_UNLK(uint32_t** emit, uint16_t opcode) {
#warning UNIMPLEMENTED
}

/***
 *       ___                      _        _____     _     _
 *      / _ \ _ __   ___ ___   __| | ___  |_   _|_ _| |__ | | ___
 *     | | | | '_ \ / __/ _ \ / _` |/ _ \   | |/ _` | '_ \| |/ _ \
 *     | |_| | |_) | (_| (_) | (_| |  __/   | | (_| | |_) | |  __/
 *      \___/| .__/ \___\___/ \__,_|\___|   |_|\__,_|_.__/|_|\___|
 *           |_|
 */
typedef struct {
    uint16_t match, equal;
    void (*emit)(uint32_t**, uint16_t);
} op_details_t;

__attribute__((used)) const op_details_t optab_68000[] = {
    {0xFFB8, 0x0000, &emit_ORIBW      },
    {0xFFB0, 0x0010, &emit_ORIBW      },
    {0xFFB0, 0x0020, &emit_ORIBW      },
    {0xFFB8, 0x0030, &emit_ORIBW      },
    {0xFFBE, 0x0038, &emit_ORIBW      },
    {0xFFFF, 0x003C, &emit_ORI_TO_CCR },
    {0xFFFF, 0x007C, &emit_ORI_TO_SR  },
    {0xFFF8, 0x0080, &emit_ORIL       },
    {0xFFF0, 0x0090, &emit_ORIL       },
    {0xFFF0, 0x00A0, &emit_ORIL       },
    {0xFFF8, 0x00B0, &emit_ORIL       },
    {0xFFFE, 0x00B8, &emit_ORIL       },
    {0xFFB8, 0x0200, &emit_ANDIBW     },
    {0xFFB0, 0x0210, &emit_ANDIBW     },
    {0xFFB0, 0x0220, &emit_ANDIBW     },
    {0xFFB8, 0x0230, &emit_ANDIBW     },
    {0xFFBE, 0x0238, &emit_ANDIBW     },
    {0xFFFF, 0x023C, &emit_ANDI_TO_CCR},
    {0xFFFF, 0x027C, &emit_ANDI_TO_SR },
    {0xFFF8, 0x0280, &emit_ANDIL      },
    {0xFFF0, 0x0290, &emit_ANDIL      },
    {0xFFF0, 0x02A0, &emit_ANDIL      },
    {0xFFF8, 0x02B0, &emit_ANDIL      },
    {0xFFFE, 0x02B8, &emit_ANDIL      },
    {0xFFB8, 0x0400, &emit_SUBIBW     },
    {0xFFB0, 0x0410, &emit_SUBIBW     },
    {0xFFB0, 0x0420, &emit_SUBIBW     },
    {0xFFB8, 0x0430, &emit_SUBIBW     },
    {0xFFBE, 0x0438, &emit_SUBIBW     },
    {0xFFF8, 0x0480, &emit_SUBIL      },
    {0xFFF0, 0x0490, &emit_SUBIL      },
    {0xFFF0, 0x04A0, &emit_SUBIL      },
    {0xFFF8, 0x04B0, &emit_SUBIL      },
    {0xFFFE, 0x04B8, &emit_SUBIL      },
    {0xFFB8, 0x0600, &emit_ADDIBW     },
    {0xFFB0, 0x0610, &emit_ADDIBW     },
    {0xFFB0, 0x0620, &emit_ADDIBW     },
    {0xFFB8, 0x0630, &emit_ADDIBW     },
    {0xFFBE, 0x0638, &emit_ADDIBW     },
    {0xFFF8, 0x0680, &emit_ADDIL      },
    {0xFFF0, 0x0690, &emit_ADDIL      },
    {0xFFF0, 0x06A0, &emit_ADDIL      },
    {0xFFF8, 0x06B0, &emit_ADDIL      },
    {0xFFFE, 0x06B8, &emit_ADDIL      },
    {0xFFB8, 0x0A00, &emit_EORIBW     },
    {0xFFB0, 0x0A10, &emit_EORIBW     },
    {0xFFB0, 0x0A20, &emit_EORIBW     },
    {0xFFB8, 0x0A30, &emit_EORIBW     },
    {0xFFBE, 0x0A38, &emit_EORIBW     },
    {0xFFFF, 0x0A3C, &emit_EORI_TO_CCR},
    {0xFFFF, 0x0A7C, &emit_EORI_TO_SR },
    {0xFFF8, 0x0A80, &emit_EORIL      },
    {0xFFF0, 0x0A90, &emit_EORIL      },
    {0xFFF0, 0x0AA0, &emit_EORIL      },
    {0xFFF8, 0x0AB0, &emit_EORIL      },
    {0xFFFE, 0x0AB8, &emit_EORIL      },
    {0xFFB8, 0x0C00, &emit_CMPIBW     },
    {0xFFB0, 0x0C10, &emit_CMPIBW     },
    {0xFFB0, 0x0C20, &emit_CMPIBW     },
    {0xFFB8, 0x0C30, &emit_CMPIBW     },
    {0xFFBE, 0x0C38, &emit_CMPIBW     },
    {0xFFF8, 0x0C80, &emit_CMPIL      },
    {0xFFF0, 0x0C90, &emit_CMPIL      },
    {0xFFF0, 0x0CA0, &emit_CMPIL      },
    {0xFFF8, 0x0CB0, &emit_CMPIL      },
    {0xFFFE, 0x0CB8, &emit_CMPIL      },
    {0xFFF8, 0x0800, &emit_BTST       },
    {0xFFF0, 0x0810, &emit_BTST       },
    {0xFFF0, 0x0820, &emit_BTST       },
    {0xFFF8, 0x0830, &emit_BTST       },
    {0xFFFE, 0x0838, &emit_BTST       },
    {0xF1F8, 0x0100, &emit_BTST       },
    {0xF1F0, 0x0110, &emit_BTST       },
    {0xF1F0, 0x0120, &emit_BTST       },
    {0xF1F8, 0x0130, &emit_BTST       },
    {0xF1FE, 0x0138, &emit_BTST       },
    {0xFFF8, 0x0840, &emit_BCHG       },
    {0xFFF0, 0x0850, &emit_BCHG       },
    {0xFFF0, 0x0860, &emit_BCHG       },
    {0xFFF8, 0x0870, &emit_BCHG       },
    {0xFFFE, 0x0878, &emit_BCHG       },
    {0xF1F8, 0x0140, &emit_BCHG       },
    {0xF1F0, 0x0150, &emit_BCHG       },
    {0xF1F0, 0x0160, &emit_BCHG       },
    {0xF1F8, 0x0170, &emit_BCHG       },
    {0xF1FE, 0x0178, &emit_BCHG       },
    {0xFFF8, 0x0880, &emit_BCLR       },
    {0xFFF0, 0x0890, &emit_BCLR       },
    {0xFFF0, 0x08A0, &emit_BCLR       },
    {0xFFF8, 0x08B0, &emit_BCLR       },
    {0xFFFE, 0x08B8, &emit_BCLR       },
    {0xF1F8, 0x0180, &emit_BCLR       },
    {0xF1F0, 0x0190, &emit_BCLR       },
    {0xF1F0, 0x01A0, &emit_BCLR       },
    {0xF1F8, 0x01B0, &emit_BCLR       },
    {0xF1FE, 0x01B8, &emit_BCLR       },
    {0xFFF8, 0x08C0, &emit_BSET       },
    {0xFFF0, 0x08D0, &emit_BSET       },
    {0xFFF0, 0x08E0, &emit_BSET       },
    {0xFFF8, 0x08F0, &emit_BSET       },
    {0xFFFE, 0x08F8, &emit_BSET       },
    {0xF1F8, 0x01C0, &emit_BSET       },
    {0xF1F0, 0x01D0, &emit_BSET       },
    {0xF1F0, 0x01E0, &emit_BSET       },
    {0xF1F8, 0x01F0, &emit_BSET       },
    {0xF1FE, 0x01F8, &emit_BSET       },
    {0xF1B8, 0x0108, &emit_MOVEP      },
    {0xF1B8, 0x0188, &emit_MOVEP      },
    {0xF1F8, 0x1000, &emit_MOVEB      },
    {0xF1F0, 0x1010, &emit_MOVEB      },
    {0xF1F0, 0x1020, &emit_MOVEB      },
    {0xF1F8, 0x1030, &emit_MOVEB      },
    {0xF1FC, 0x1038, &emit_MOVEB      },
    {0xF1FF, 0x103C, &emit_MOVEB      },
    {0xF1B8, 0x1080, &emit_MOVEB      },
    {0xF1B0, 0x1090, &emit_MOVEB      },
    {0xF1B0, 0x10A0, &emit_MOVEB      },
    {0xF1B8, 0x10B0, &emit_MOVEB      },
    {0xF1BC, 0x10B8, &emit_MOVEB      },
    {0xF1BF, 0x10BC, &emit_MOVEB      },
    {0xF1B8, 0x1100, &emit_MOVEB      },
    {0xF1B0, 0x1110, &emit_MOVEB      },
    {0xF1B0, 0x1120, &emit_MOVEB      },
    {0xF1B8, 0x1130, &emit_MOVEB      },
    {0xF1BC, 0x1138, &emit_MOVEB      },
    {0xF1BF, 0x113C, &emit_MOVEB      },
    {0xF1F8, 0x1180, &emit_MOVEB      },
    {0xF1F0, 0x1190, &emit_MOVEB      },
    {0xF1F0, 0x11A0, &emit_MOVEB      },
    {0xF1F8, 0x11B0, &emit_MOVEB      },
    {0xF1FC, 0x11B8, &emit_MOVEB      },
    {0xF1FF, 0x11BC, &emit_MOVEB      },
    {0xFDF8, 0x11C0, &emit_MOVEB      },
    {0xFDF0, 0x11D0, &emit_MOVEB      },
    {0xFDF0, 0x11E0, &emit_MOVEB      },
    {0xFDF8, 0x11F0, &emit_MOVEB      },
    {0xFDFC, 0x11F8, &emit_MOVEB      },
    {0xFDFF, 0x11FC, &emit_MOVEB      },
    {0xF1E0, 0x2000, &emit_MOVEL      },
    {0xF1F0, 0x2020, &emit_MOVEL      },
    {0xF1F8, 0x2030, &emit_MOVEL      },
    {0xF1FC, 0x2038, &emit_MOVEL      },
    {0xF1FF, 0x203C, &emit_MOVEL      },
    {0xF1E0, 0x2040, &emit_MOVEAL     },
    {0xF1F0, 0x2060, &emit_MOVEAL     },
    {0xF1F8, 0x2070, &emit_MOVEAL     },
    {0xF1FC, 0x2078, &emit_MOVEAL     },
    {0xF1FF, 0x207C, &emit_MOVEAL     },
    {0xF1A0, 0x2080, &emit_MOVEL      },
    {0xF1B0, 0x20A0, &emit_MOVEL      },
    {0xF1B8, 0x20B0, &emit_MOVEL      },
    {0xF1BC, 0x20B8, &emit_MOVEL      },
    {0xF1BF, 0x20BC, &emit_MOVEL      },
    {0xF1A0, 0x2100, &emit_MOVEL      },
    {0xF1B0, 0x2120, &emit_MOVEL      },
    {0xF1B8, 0x2130, &emit_MOVEL      },
    {0xF1BC, 0x2138, &emit_MOVEL      },
    {0xF1BF, 0x213C, &emit_MOVEL      },
    {0xF1E0, 0x2180, &emit_MOVEL      },
    {0xF1F0, 0x21A0, &emit_MOVEL      },
    {0xF1F8, 0x21B0, &emit_MOVEL      },
    {0xF1FC, 0x21B8, &emit_MOVEL      },
    {0xF1FF, 0x21BC, &emit_MOVEL      },
    {0xFDE0, 0x21C0, &emit_MOVEL      },
    {0xFDF0, 0x21E0, &emit_MOVEL      },
    {0xFDF8, 0x21F0, &emit_MOVEL      },
    {0xFDFC, 0x21F8, &emit_MOVEL      },
    {0xFDFF, 0x21FC, &emit_MOVEL      },
    {0xF1E0, 0x3000, &emit_MOVEW      },
    {0xF1F0, 0x3020, &emit_MOVEW      },
    {0xF1F8, 0x3030, &emit_MOVEW      },
    {0xF1FC, 0x3038, &emit_MOVEW      },
    {0xF1FF, 0x303C, &emit_MOVEW      },
    {0xF1E0, 0x3040, &emit_MOVEAW     },
    {0xF1F0, 0x3060, &emit_MOVEAW     },
    {0xF1F8, 0x3070, &emit_MOVEAW     },
    {0xF1FC, 0x3078, &emit_MOVEAW     },
    {0xF1FF, 0x307C, &emit_MOVEAW     },
    {0xF1A0, 0x3080, &emit_MOVEW      },
    {0xF1B0, 0x30A0, &emit_MOVEW      },
    {0xF1B8, 0x30B0, &emit_MOVEW      },
    {0xF1BC, 0x30B8, &emit_MOVEW      },
    {0xF1BF, 0x30BC, &emit_MOVEW      },
    {0xF1A0, 0x3100, &emit_MOVEW      },
    {0xF1B0, 0x3120, &emit_MOVEW      },
    {0xF1B8, 0x3130, &emit_MOVEW      },
    {0xF1BC, 0x3138, &emit_MOVEW      },
    {0xF1BF, 0x313C, &emit_MOVEW      },
    {0xF1E0, 0x3180, &emit_MOVEW      },
    {0xF1F0, 0x31A0, &emit_MOVEW      },
    {0xF1F8, 0x31B0, &emit_MOVEW      },
    {0xF1FC, 0x31B8, &emit_MOVEW      },
    {0xF1FF, 0x31BC, &emit_MOVEW      },
    {0xFDE0, 0x31C0, &emit_MOVEW      },
    {0xFDF0, 0x31E0, &emit_MOVEW      },
    {0xFDF8, 0x31F0, &emit_MOVEW      },
    {0xFDFC, 0x31F8, &emit_MOVEW      },
    {0xFDFF, 0x31FC, &emit_MOVEW      },
    {0xFFF8, 0x40C0, &emit_MOVE_SR_TO },
    {0xFFF0, 0x40D0, &emit_MOVE_SR_TO },
    {0xFFF0, 0x40E0, &emit_MOVE_SR_TO },
    {0xFFF8, 0x40F0, &emit_MOVE_SR_TO },
    {0xFFFE, 0x40F8, &emit_MOVE_SR_TO },
    {0xFFF8, 0x44C0, &emit_MOVE_TO_CCR},
    {0xFFF0, 0x44D0, &emit_MOVE_TO_CCR},
    {0xFFF0, 0x44E0, &emit_MOVE_TO_CCR},
    {0xFFF8, 0x44F0, &emit_MOVE_TO_CCR},
    {0xFFFC, 0x44F8, &emit_MOVE_TO_CCR},
    {0xFFFF, 0x44FC, &emit_MOVE_TO_CCR},
    {0xFFF8, 0x46C0, &emit_MOVE_TO_SR },
    {0xFFF0, 0x46D0, &emit_MOVE_TO_SR },
    {0xFFF0, 0x46E0, &emit_MOVE_TO_SR },
    {0xFFF8, 0x46F0, &emit_MOVE_TO_SR },
    {0xFFFC, 0x46F8, &emit_MOVE_TO_SR },
    {0xFFFF, 0x46FC, &emit_MOVE_TO_SR },
    {0xFFB8, 0x4000, &emit_NEGXBW     },
    {0xFFB0, 0x4010, &emit_NEGXBW     },
    {0xFFB0, 0x4020, &emit_NEGXBW     },
    {0xFFB8, 0x4030, &emit_NEGXBW     },
    {0xFFBE, 0x4038, &emit_NEGXBW     },
    {0xFFF8, 0x4080, &emit_NEGXL      },
    {0xFFF0, 0x4090, &emit_NEGXL      },
    {0xFFF0, 0x40A0, &emit_NEGXL      },
    {0xFFF8, 0x40B0, &emit_NEGXL      },
    {0xFFFE, 0x40B8, &emit_NEGXL      },
    {0xFFB8, 0x4200, &emit_CLRBW      },
    {0xFFB0, 0x4210, &emit_CLRBW      },
    {0xFFB0, 0x4220, &emit_CLRBW      },
    {0xFFB8, 0x4230, &emit_CLRBW      },
    {0xFFBE, 0x4238, &emit_CLRBW      },
    {0xFFF8, 0x4280, &emit_CLRL       },
    {0xFFF0, 0x4290, &emit_CLRL       },
    {0xFFF0, 0x42A0, &emit_CLRL       },
    {0xFFF8, 0x42B0, &emit_CLRL       },
    {0xFFFE, 0x42B8, &emit_CLRL       },
    {0xFFB8, 0x4400, &emit_NEGBW      },
    {0xFFB0, 0x4410, &emit_NEGBW      },
    {0xFFB0, 0x4420, &emit_NEGBW      },
    {0xFFB8, 0x4430, &emit_NEGBW      },
    {0xFFBE, 0x4438, &emit_NEGBW      },
    {0xFFF8, 0x4480, &emit_NEGL       },
    {0xFFF0, 0x4490, &emit_NEGL       },
    {0xFFF0, 0x44A0, &emit_NEGL       },
    {0xFFF8, 0x44B0, &emit_NEGL       },
    {0xFFFE, 0x44B8, &emit_NEGL       },
    {0xFFB8, 0x4600, &emit_NOTBW      },
    {0xFFB0, 0x4610, &emit_NOTBW      },
    {0xFFB0, 0x4620, &emit_NOTBW      },
    {0xFFB8, 0x4630, &emit_NOTBW      },
    {0xFFBE, 0x4638, &emit_NOTBW      },
    {0xFFF8, 0x4680, &emit_NOTL       },
    {0xFFF0, 0x4690, &emit_NOTL       },
    {0xFFF0, 0x46A0, &emit_NOTL       },
    {0xFFF8, 0x46B0, &emit_NOTL       },
    {0xFFFE, 0x46B8, &emit_NOTL       },
    {0xFFF8, 0x4880, &emit_EXTW       },
    {0xFFF8, 0x48C0, &emit_EXTL       },
    {0xFFF8, 0x4800, &emit_NBCD       },
    {0xFFF0, 0x4810, &emit_NBCD       },
    {0xFFF0, 0x4820, &emit_NBCD       },
    {0xFFF8, 0x4830, &emit_NBCD       },
    {0xFFFE, 0x4838, &emit_NBCD       },
    {0xFFF8, 0x4840, &emit_SWAP       },
    {0xFFF8, 0x4850, &emit_PEA        },
    {0xFFF8, 0x4868, &emit_PEA        },
    {0xFFF8, 0x4870, &emit_PEA        },
    {0xFFFC, 0x4878, &emit_PEA        },
    {0xFFFF, 0x4AFC, &emit_ILLEGAL    },
    {0xFFF8, 0x4AC0, &emit_TAS        },
    {0xFFF0, 0x4AD0, &emit_TAS        },
    {0xFFF0, 0x4AE0, &emit_TAS        },
    {0xFFF8, 0x4AF0, &emit_TAS        },
    {0xFFFE, 0x4AF8, &emit_TAS        },
    {0xFFB8, 0x4A00, &emit_TSTBW      },
    {0xFFB0, 0x4A10, &emit_TSTBW      },
    {0xFFB0, 0x4A20, &emit_TSTBW      },
    {0xFFB8, 0x4A30, &emit_TSTBW      },
    {0xFFBE, 0x4A38, &emit_TSTBW      },
    {0xFFF8, 0x4A80, &emit_TSTL       },
    {0xFFF0, 0x4A90, &emit_TSTL       },
    {0xFFF0, 0x4AA0, &emit_TSTL       },
    {0xFFF8, 0x4AB0, &emit_TSTL       },
    {0xFFFE, 0x4AB8, &emit_TSTL       },
    {0xFFF8, 0x4E40, &emit_TRAPV      },
    {0xFFFF, 0x4E48, &emit_TRAP       },
    {0xFFFF, 0x4E49, &emit_TRAP       },
    {0xFFFF, 0x4E4A, &emit_TRAP       },
    {0xFFFF, 0x4E4B, &emit_TRAP       },
    {0xFFFF, 0x4E4C, &emit_TRAP       },
    {0xFFFF, 0x4E4D, &emit_TRAP       },
    {0xFFFF, 0x4E4E, &emit_TRAP       },
    {0xFFFF, 0x4E4F, &emit_TRAP       },
    {0xFFF8, 0x4E50, &emit_LINK       },
    {0xFFF8, 0x4E58, &emit_UNLK       },
    {0xFFF8, 0x4E60, &emit_MOVE_TO_USP},
    {0xFFF8, 0x4E68, &emit_MOVE_USP_TO},
    {0xFFFF, 0x4E70, &emit_RESET      },
    {0xFFFF, 0x4E71, &emit_NOP        },
    {0xFFFF, 0x4E72, &emit_STOP       },
    {0xFFFF, 0x4E73, &emit_RTE        },
    {0xFFFF, 0x4E75, &emit_RTS        },
    {0xFFFF, 0x4E76, &emit_TRAPV      },
    {0xFFFF, 0x4E77, &emit_RTR        },
    {0xFFF8, 0x4E90, &emit_JSR        },
    {0xFFF8, 0x4EA8, &emit_JSR        },
    {0xFFF8, 0x4EB0, &emit_JSR        },
    {0xFFFC, 0x4EB8, &emit_JSR        },
    {0xFFF8, 0x4ED0, &emit_JMP        },
    {0xFFF8, 0x4EE8, &emit_JMP        },
    {0xFFF8, 0x4EF0, &emit_JMP        },
    {0xFFFC, 0x4EF8, &emit_JMP        },
    {0xFFB8, 0x4890, &emit_MOVEM      },
    {0xFFB0, 0x48A0, &emit_MOVEM      },
    {0xFFB8, 0x48B0, &emit_MOVEM      },
    {0xFFBE, 0x48B8, &emit_MOVEM      },
    {0xFFB0, 0x4C90, &emit_MOVEM      },
    {0xFFB8, 0x4CA8, &emit_MOVEM      },
    {0xFFB8, 0x4CB0, &emit_MOVEM      },
    {0xFFBC, 0x4CB8, &emit_MOVEM      },
    {0xF1F8, 0x41D0, &emit_LEA        },
    {0xF1F8, 0x41E8, &emit_LEA        },
    {0xF1F8, 0x41F0, &emit_LEA        },
    {0xF1FC, 0x41F8, &emit_LEA        },
    {0xF1F8, 0x4180, &emit_CHK        },
    {0xF1F0, 0x4190, &emit_CHK        },
    {0xF1F0, 0x41A0, &emit_CHK        },
    {0xF1F8, 0x41B0, &emit_CHK        },
    {0xF1FE, 0x41B8, &emit_CHK        },
    {0xF1B8, 0x5000, &emit_ADDQBW     },
    {0xF1B0, 0x5010, &emit_ADDQBW     },
    {0xF1B0, 0x5020, &emit_ADDQBW     },
    {0xF1B8, 0x5030, &emit_ADDQBW     },
    {0xF1BE, 0x5038, &emit_ADDQBW     },
    {0xF1E0, 0x5080, &emit_ADDQL      },
    {0xF1F0, 0x50A0, &emit_ADDQL      },
    {0xF1F8, 0x50B0, &emit_ADDQL      },
    {0xF1FE, 0x50B8, &emit_ADDQL      },
    {0xF1B8, 0x5100, &emit_SUBQBW     },
    {0xF1B0, 0x5110, &emit_SUBQBW     },
    {0xF1B0, 0x5120, &emit_SUBQBW     },
    {0xF1B8, 0x5130, &emit_SUBQBW     },
    {0xF1BE, 0x5138, &emit_SUBQBW     },
    {0xF1E0, 0x5180, &emit_SUBQL      },
    {0xF1F0, 0x51A0, &emit_SUBQL      },
    {0xF1F8, 0x51B0, &emit_SUBQL      },
    {0xF1FE, 0x51B8, &emit_SUBQL      },
    {0xF0F8, 0x50C0, &emit_Scc        },
    {0xF0F0, 0x50D0, &emit_Scc        },
    {0xF0F0, 0x50E0, &emit_Scc        },
    {0xF0F8, 0x50F0, &emit_Scc        },
    {0xF0FE, 0x50F8, &emit_Scc        },
    {0xFFF8, 0x51C8, &emit_DBRA       },
    {0xF0F8, 0x50C8, &emit_DBcc       },
    {0xFF00, 0x6000, &emit_BRA        },
    {0xFF00, 0x6100, &emit_BSR        },
    {0xF800, 0x6800, &emit_Bcc        },
    {0xFC00, 0x6400, &emit_Bcc        },
    {0xFE00, 0x6200, &emit_Bcc        },
    {0xF100, 0x7000, &emit_MOVEQ      },
    {0xF1F8, 0x80C0, &emit_DIVU       },
    {0xF1F0, 0x80D0, &emit_DIVU       },
    {0xF1F0, 0x80E0, &emit_DIVU       },
    {0xF1F8, 0x80F0, &emit_DIVU       },
    {0xF1FC, 0x80F8, &emit_DIVU       },
    {0xF1FF, 0x80FC, &emit_DIVU       },
    {0xF1F8, 0x81C0, &emit_DIVS       },
    {0xF1F0, 0x81D0, &emit_DIVS       },
    {0xF1F0, 0x81E0, &emit_DIVS       },
    {0xF1F8, 0x81F0, &emit_DIVS       },
    {0xF1FC, 0x81F8, &emit_DIVS       },
    {0xF1FF, 0x81FC, &emit_DIVS       },
    {0xF1F8, 0x8100, &emit_SBCD       },
    {0xF1F8, 0x8108, &emit_SBCD       },
    {0xF1B8, 0x8000, &emit_ORBW       },
    {0xF1B0, 0x8010, &emit_ORBW       },
    {0xF1B0, 0x8020, &emit_ORBW       },
    {0xF1B8, 0x8030, &emit_ORBW       },
    {0xF1BC, 0x8038, &emit_ORBW       },
    {0xF1BF, 0x803C, &emit_ORBW       },
    {0xF1F8, 0x8048, &emit_ORW        },
    {0xF1B0, 0x8110, &emit_ORBW       },
    {0xF1B0, 0x8120, &emit_ORBW       },
    {0xF1B8, 0x8130, &emit_ORBW       },
    {0xF1BE, 0x8138, &emit_ORBW       },
    {0xF1F8, 0x8080, &emit_ORL        },
    {0xF1F8, 0x8088, &emit_ORL        },
    {0xF1F0, 0x8090, &emit_ORL        },
    {0xF1F0, 0x80A0, &emit_ORL        },
    {0xF1F8, 0x80B0, &emit_ORL        },
    {0xF1FC, 0x80B8, &emit_ORL        },
    {0xF1FF, 0x80BC, &emit_ORL        },
    {0xF1F0, 0x8190, &emit_ORL        },
    {0xF1F0, 0x81A0, &emit_ORL        },
    {0xF1F8, 0x81B0, &emit_ORL        },
    {0xF1FE, 0x81B8, &emit_ORL        },
    {0xF1B8, 0x9000, &emit_SUBBW      },
    {0xF1B0, 0x9010, &emit_SUBBW      },
    {0xF1B0, 0x9020, &emit_SUBBW      },
    {0xF1B8, 0x9030, &emit_SUBBW      },
    {0xF1BC, 0x9038, &emit_SUBBW      },
    {0xF1BF, 0x903C, &emit_SUBBW      },
    {0xF1F8, 0x9048, &emit_SUBW       },
    {0xF1B0, 0x9110, &emit_SUBBW      },
    {0xF1B0, 0x9120, &emit_SUBBW      },
    {0xF1B8, 0x9130, &emit_SUBBW      },
    {0xF1BE, 0x9138, &emit_SUBBW      },
    {0xF1F8, 0x9080, &emit_SUBL       },
    {0xF1F8, 0x9088, &emit_SUBL       },
    {0xF1F0, 0x9090, &emit_SUBL       },
    {0xF1F0, 0x90A0, &emit_SUBL       },
    {0xF1F8, 0x90B0, &emit_SUBL       },
    {0xF1FC, 0x90B8, &emit_SUBL       },
    {0xF1FF, 0x90BC, &emit_SUBL       },
    {0xF1F0, 0x9190, &emit_SUBL       },
    {0xF1F0, 0x91A0, &emit_SUBL       },
    {0xF1F8, 0x91B0, &emit_SUBL       },
    {0xF1FE, 0x91B8, &emit_SUBL       },
    {0xF1B8, 0x9100, &emit_SUBXBW     },
    {0xF1B8, 0x9108, &emit_SUBXBW     },
    {0xF1F8, 0x9180, &emit_SUBXL      },
    {0xF1F8, 0x9188, &emit_SUBXL      },
    {0xF1E0, 0x90C0, &emit_SUBAW      },
    {0xF1F0, 0x90E0, &emit_SUBAW      },
    {0xF1F8, 0x90F0, &emit_SUBAW      },
    {0xF1FC, 0x90F8, &emit_SUBAW      },
    {0xF1FF, 0x90FC, &emit_SUBAW      },
    {0xF1E0, 0x91C0, &emit_SUBAL      },
    {0xF1F0, 0x91E0, &emit_SUBAL      },
    {0xF1F8, 0x91F0, &emit_SUBAL      },
    {0xF1FC, 0x91F8, &emit_SUBAL      },
    {0xF1FF, 0x91FC, &emit_SUBAL      },
    {0xF000, 0xA000, &emit_ALINE      },
    {0xF1F8, 0xB000, &emit_CMPB       },
    {0xF1F0, 0xB010, &emit_CMPB       },
    {0xF1E0, 0xB040, &emit_CMPW       },
    {0xF1B0, 0xB020, &emit_CMPBW      },
    {0xF1B8, 0xB030, &emit_CMPBW      },
    {0xF1BC, 0xB038, &emit_CMPBW      },
    {0xF1BF, 0xB03C, &emit_CMPBW      },
    {0xF1E0, 0xB080, &emit_CMPL       },
    {0xF1F0, 0xB0A0, &emit_CMPL       },
    {0xF1F8, 0xB0B0, &emit_CMPL       },
    {0xF1FC, 0xB0B8, &emit_CMPL       },
    {0xF1FF, 0xB0BC, &emit_CMPL       },
    {0xF1E0, 0xB0C0, &emit_CMPAW      },
    {0xF1F0, 0xB0E0, &emit_CMPAW      },
    {0xF1F8, 0xB0F0, &emit_CMPAW      },
    {0xF1FC, 0xB0F8, &emit_CMPAW      },
    {0xF1FF, 0xB0FC, &emit_CMPAW      },
    {0xF1E0, 0xB1C0, &emit_CMPAL      },
    {0xF1F0, 0xB1E0, &emit_CMPAL      },
    {0xF1F8, 0xB1F0, &emit_CMPAL      },
    {0xF1FC, 0xB1F8, &emit_CMPAL      },
    {0xF1FF, 0xB1FC, &emit_CMPAL      },
    {0xF1F8, 0xB108, &emit_CMPMB      },
    {0xF1F8, 0xB148, &emit_CMPMW      },
    {0xF1F8, 0xB188, &emit_CMPML      },
    {0xF1B8, 0xB100, &emit_EORBW      },
    {0xF1B0, 0xB110, &emit_EORBW      },
    {0xF1B0, 0xB120, &emit_EORBW      },
    {0xF1B8, 0xB130, &emit_EORBW      },
    {0xF1BE, 0xB138, &emit_EORBW      },
    {0xF1F8, 0xB180, &emit_EORL       },
    {0xF1F0, 0xB190, &emit_EORL       },
    {0xF1F0, 0xB1A0, &emit_EORL       },
    {0xF1F8, 0xB1B0, &emit_EORL       },
    {0xF1FE, 0xB1B8, &emit_EORL       },
    {0xF1B8, 0xC000, &emit_ANDBW      },
    {0xF1B0, 0xC010, &emit_ANDBW      },
    {0xF1B0, 0xC020, &emit_ANDBW      },
    {0xF1B8, 0xC030, &emit_ANDBW      },
    {0xF1BC, 0xC038, &emit_ANDBW      },
    {0xF1BF, 0xC03C, &emit_ANDBW      },
    {0xF1F8, 0xC048, &emit_ANDW       },
    {0xF1B0, 0xC110, &emit_ANDBW      },
    {0xF1B0, 0xC120, &emit_ANDBW      },
    {0xF1B8, 0xC130, &emit_ANDBW      },
    {0xF1BE, 0xC138, &emit_ANDBW      },
    {0xF1F8, 0xC080, &emit_ANDL       },
    {0xF1F8, 0xC088, &emit_ANDL       },
    {0xF1F0, 0xC090, &emit_ANDL       },
    {0xF1F0, 0xC0A0, &emit_ANDL       },
    {0xF1F8, 0xC0B0, &emit_ANDL       },
    {0xF1FC, 0xC0B8, &emit_ANDL       },
    {0xF1FF, 0xC0BC, &emit_ANDL       },
    {0xF1F0, 0xC190, &emit_ANDL       },
    {0xF1F0, 0xC1A0, &emit_ANDL       },
    {0xF1F8, 0xC1B0, &emit_ANDL       },
    {0xF1FE, 0xC1B8, &emit_ANDL       },
    {0xF1F8, 0xC0C0, &emit_MULU       },
    {0xF1F0, 0xC0D0, &emit_MULU       },
    {0xF1F0, 0xC0E0, &emit_MULU       },
    {0xF1F8, 0xC0F0, &emit_MULU       },
    {0xF1FC, 0xC0F8, &emit_MULU       },
    {0xF1FF, 0xC0FC, &emit_MULU       },
    {0xF1F8, 0xC1C0, &emit_MULS       },
    {0xF1F0, 0xC1D0, &emit_MULS       },
    {0xF1F0, 0xC1E0, &emit_MULS       },
    {0xF1F8, 0xC1F0, &emit_MULS       },
    {0xF1FC, 0xC1F8, &emit_MULS       },
    {0xF1FF, 0xC1FC, &emit_MULS       },
    {0xF1F8, 0xC100, &emit_ABCD       },
    {0xF1F8, 0xC108, &emit_ABCD       },
    {0xF1F8, 0xC140, &emit_EXG        },
    {0xF1F8, 0xC148, &emit_EXG        },
    {0xF1F8, 0xC188, &emit_EXG        },
    {0xF1B8, 0xD000, &emit_ADDBW      },
    {0xF1B0, 0xD010, &emit_ADDBW      },
    {0xF1B0, 0xD020, &emit_ADDBW      },
    {0xF1B8, 0xD030, &emit_ADDBW      },
    {0xF1BC, 0xD038, &emit_ADDBW      },
    {0xF1BF, 0xD03C, &emit_ADDBW      },
    {0xF1F8, 0xD048, &emit_ADDW       },
    {0xF1B0, 0xD110, &emit_ADDBW      },
    {0xF1B0, 0xD120, &emit_ADDBW      },
    {0xF1B8, 0xD130, &emit_ADDBW      },
    {0xF1BE, 0xD138, &emit_ADDBW      },
    {0xF1F8, 0xD080, &emit_ADDL       },
    {0xF1F8, 0xD088, &emit_ADDL       },
    {0xF1F0, 0xD090, &emit_ADDL       },
    {0xF1F0, 0xD0A0, &emit_ADDL       },
    {0xF1F8, 0xD0B0, &emit_ADDL       },
    {0xF1FC, 0xD0B8, &emit_ADDL       },
    {0xF1FF, 0xD0BC, &emit_ADDL       },
    {0xF1F0, 0xD190, &emit_ADDL       },
    {0xF1F0, 0xD1A0, &emit_ADDL       },
    {0xF1F8, 0xD1B0, &emit_ADDL       },
    {0xF1FE, 0xD1B8, &emit_ADDL       },
    {0xF1B8, 0xD100, &emit_ADDXBW     },
    {0xF1B8, 0xD108, &emit_ADDXBW     },
    {0xF1F8, 0xD180, &emit_ADDXL      },
    {0xF1F8, 0xD188, &emit_ADDXL      },
    {0xF1E0, 0xD0C0, &emit_ADDAW      },
    {0xF1F0, 0xD0E0, &emit_ADDAW      },
    {0xF1F8, 0xD0F0, &emit_ADDAW      },
    {0xF1FC, 0xD0F8, &emit_ADDAW      },
    {0xF1FF, 0xD0FC, &emit_ADDAW      },
    {0xF1E0, 0xD1C0, &emit_ADDAL      },
    {0xF1F0, 0xD1E0, &emit_ADDAL      },
    {0xF1F8, 0xD1F0, &emit_ADDAL      },
    {0xF1FC, 0xD1F8, &emit_ADDAL      },
    {0xF1FF, 0xD1FC, &emit_ADDAL      },
    {0xFEF0, 0xE0D0, &emit_ASd        },
    {0xFEF0, 0xE0E0, &emit_ASd        },
    {0xFEF8, 0xE0F0, &emit_ASd        },
    {0xFEFE, 0xE0F8, &emit_ASd        },
    {0xFEF0, 0xE2D0, &emit_LSd        },
    {0xFEF0, 0xE2E0, &emit_LSd        },
    {0xFEF8, 0xE2F0, &emit_LSd        },
    {0xFEFE, 0xE2F8, &emit_LSd        },
    {0xFEF0, 0xE4D0, &emit_ROXd       },
    {0xFEF0, 0xE4E0, &emit_ROXd       },
    {0xFEF8, 0xE4F0, &emit_ROXd       },
    {0xFEFE, 0xE4F8, &emit_ROXd       },
    {0xFEF0, 0xE6D0, &emit_ROd        },
    {0xFEF0, 0xE6E0, &emit_ROd        },
    {0xFEF8, 0xE6F0, &emit_ROd        },
    {0xFEFE, 0xE6F8, &emit_ROd        },
    {0xF0B8, 0xE000, &emit_ASdBW      },
    {0xF0F8, 0xE080, &emit_ASdL       },
    {0xF0B8, 0xE008, &emit_LSdBW      },
    {0xF0F8, 0xE088, &emit_LSdL       },
    {0xF0B8, 0xE010, &emit_ROXdBW     },
    {0xF0F8, 0xE090, &emit_ROXdL      },
    {0xF0B8, 0xE018, &emit_ROdBW      },
    {0xF0F8, 0xE098, &emit_ROdL       },
    {0xF0B8, 0xE020, &emit_ASdBW      },
    {0xF0F8, 0xE0A0, &emit_ASdL       },
    {0xF0B8, 0xE028, &emit_LSdBW      },
    {0xF0F8, 0xE0A8, &emit_LSdL       },
    {0xF0B8, 0xE030, &emit_ROXdBW     },
    {0xF0F8, 0xE0B0, &emit_ROXdL      },
    {0xF0B8, 0xE038, &emit_ROdBW      },
    {0xF0F8, 0xE0B8, &emit_ROdL       },
    {0xF000, 0xF000, &emit_FLINE      },
 // Leave last
    {0x0000, 0x0000, emit_ILLEGAL    }
};

__attribute__((used)) const op_details_t optab_68020[] = {
};

__attribute__((used)) const op_details_t optab_68030[] = {
};

__attribute__((used)) const op_details_t optab_68040[] = {
};

__attribute__((used)) const op_details_t optab_68882[] = {
};

__attribute__((used)) const op_details_t optab_040fpu[] = {
};

__attribute__((used)) const op_details_t optab_040mmu[] = {
};


/***
 *      __  __       _          ___                      _
 *     |  \/  | __ _(_)_ __    / _ \ _ __   ___ ___   __| | ___
 *     | |\/| |/ _` | | '_ \  | | | | '_ \ / __/ _ \ / _` |/ _ \
 *     | |  | | (_| | | | | | | |_| | |_) | (_| (_) | (_| |  __/
 *     |_|  |_|\__,_|_|_| |_|  \___/| .__/ \___\___/ \__,_|\___|
 *                                  |_|
 *      _____           _ _   _              ____             _   _
 *     | ____|_ __ ___ (_) |_| |_ ___ _ __  |  _ \ ___  _   _| |_(_)_ __   ___
 *     |  _| | '_ ` _ \| | __| __/ _ \ '__| | |_) / _ \| | | | __| | '_ \ / _ \
 *     | |___| | | | | | | |_| ||  __/ |    |  _ < (_) | |_| | |_| | | | |  __/
 *     |_____|_| |_| |_|_|\__|\__\___|_|    |_| \_\___/ \__,_|\__|_|_| |_|\___|
 *
 */
static void emit_op(uint32_t** emit, op_details_t* op, uint16_t opcode) {
    while (1) {
        if ((opcode & op->match) == op->equal) {
            op->emit(emit, opcode);
            return;
        }
        op++;
    }
}

void emit_opcode_table() {
    // Generate opcode tables
    uint32_t* stubs      = cpu->opcode_stubs;
    uint32_t* prior_stub = 0;
    uint16_t  stub_count = 0;

    // check opcode mode
    uint8_t c = config.cpu_features;

    // We can only be one CPU at a time
         if(c & cpu_enable_68040) c &= ~(cpu_enable_68000 | cpu_enable_68020 | cpu_enable_68030);
    else if(c & cpu_enable_68030) c &= ~(cpu_enable_68000 | cpu_enable_68020 | cpu_enable_68040);
    else if(c & cpu_enable_68020) c &= ~(cpu_enable_68000 | cpu_enable_68030 | cpu_enable_68040);
    else                          c &= ~(cpu_enable_68020 | cpu_enable_68030 | cpu_enable_68040);

    // 24 bit mode is only allowed with 68EC020 and 68000
    if(!(c & (cpu_enable_68000 | cpu_enable_68020)))
        c |= cpu_enable_32bits;

    // Instruction cache is allowed on 020+
    if(~(c & (cpu_enable_68020 | cpu_enable_68030 | cpu_enable_68040)))
        c &= ~cpu_enable_icache;

    // Data cache is allowed on 030+
    if(~(c & (cpu_enable_68030 | cpu_enable_68040)))
        c &= ~cpu_enable_icache;

    // MMU is only permitted on the 68040
    if(~(c & cpu_enable_68040))
        c &= ~cpu_enable_mmu;

    if(c != config.cpu_features) {
        config.cpu_features = c;
        config.is_dirty = 1;
    }

    for (int opcode = 0; opcode < 65536; opcode++) {
        static uint32_t buffer[32];  // big enough?
        uint32_t*       b;
        uint32_t        len;

        b = buffer;

        // Start with the 68000 (common base)
        emit_op(&b, optab_68000, opcode);

        // Add applicable 680x0 opcodes
             if(c & cpu_enable_68040) emit_op(&b, optab_68040, opcode);
        else if(c & cpu_enable_68030) emit_op(&b, optab_68030, opcode);
        else if(c & cpu_enable_68020) emit_op(&b, optab_68020, opcode);

        // Add applicable FPU opcodes
        if(c & cpu_enable_fpu) {
            // 68040 has a very different FPU
            if(c & cpu_enable_68040)
                emit_op(&b, optab_040fpu, opcode);
            // ...than the 68020 and 68030 (the 68882)
            else if(c & (cpu_enable_68020 | cpu_enable_68030))
                emit_op(&b, optab_68882, opcode);
            // and the 68000 does not have an FPU
        }

        // Add applicable MMU opcodes
        if(c & cpu_enable_mmu)
            emit_op(&b, optab_040mmu, opcode);

        len = b - buffer;

        if (len <= 2) {
            // opcode fits entire in one or two instructions
            // We need to trust the emitters to align to the
            // no-branch-branch rule of instruction order
            cpu->opcode_table[opcode * 2 + 0] = buffer[0];
            cpu->opcode_table[opcode * 2 + 1] = buffer[1];
        } else {
            // opcode needs more than two instructions
            // copy the first instruction and stub the rest out
            // We need to trust that the emitters place the more
            // variable instructions forward to optimize this
            cpu->opcode_table[opcode * 2 + 0] = buffer[0];
            // Search previous stubs, each stub is prefixed with
            // a pointer to the previous stub
            uint32_t* ss = stubs;
            uint32_t* from = &cpu->opcode_table[opcode * 2 + 1];
            while ((ss = (uint32_t*)*ss)) {
                if (0 == memcmp(&ss[1], &buffer[1], (len - 1) * 4)) break;
            }
            if (ss) {
                // Found duplicate stub
                cpu->opcode_table[opcode * 2 + 1] = b_imm(calc_offset(from, &ss[1]));
            } else {
                // Create new stub
                *stubs                            = prior_stub;
                prior_stub                        = stubs++;
                cpu->opcode_table[opcode * 2 + 1] = b_imm(calc_offset(from, stubs));
                memcpy(stubs, &buffer[1], (len - 1) * 4);
            }
        }
    }
}
