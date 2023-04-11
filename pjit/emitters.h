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

#pragma once

#ifndef __PJIT_emitS_H__
#define __PJIT_emitS_H__

#include <stdint.h>

#include "arm.h"

extern void emit_ABCD(uint32_t**, uint16_t);
extern void emit_ADDBW(uint32_t**, uint16_t);
extern void emit_ADDL(uint32_t**, uint16_t);
extern void emit_ADDAL(uint32_t**, uint16_t);
extern void emit_ADDAW(uint32_t**, uint16_t);
extern void emit_ADDIBW(uint32_t**, uint16_t);
extern void emit_ADDIL(uint32_t**, uint16_t);
extern void emit_ADDQBW(uint32_t**, uint16_t);
extern void emit_ADDQL(uint32_t**, uint16_t);
extern void emit_ADDW(uint32_t**, uint16_t);
extern void emit_ADDXBW(uint32_t**, uint16_t);
extern void emit_ADDXL(uint32_t**, uint16_t);
extern void emit_ALINE(uint32_t**, uint16_t);
extern void emit_ANDBW(uint32_t**, uint16_t);
extern void emit_ANDL(uint32_t**, uint16_t);
extern void emit_ANDIBW(uint32_t**, uint16_t);
extern void emit_ANDIL(uint32_t**, uint16_t);
extern void emit_ANDI_TO_CCR(uint32_t**, uint16_t);
extern void emit_ANDI_TO_SR(uint32_t**, uint16_t);
extern void emit_ANDW(uint32_t**, uint16_t);
extern void emit_ASdBW(uint32_t**, uint16_t);
extern void emit_ASdL(uint32_t**, uint16_t);
extern void emit_ASd(uint32_t**, uint16_t);
extern void emit_Bcc(uint32_t**, uint16_t);
extern void emit_BCHG(uint32_t**, uint16_t);
extern void emit_BCLR(uint32_t**, uint16_t);
extern void emit_BRA(uint32_t**, uint16_t);
extern void emit_BSET(uint32_t**, uint16_t);
extern void emit_BSR(uint32_t**, uint16_t);
extern void emit_BTST(uint32_t**, uint16_t);
extern void emit_CHK(uint32_t**, uint16_t);
extern void emit_CLRBW(uint32_t**, uint16_t);
extern void emit_CLRL(uint32_t**, uint16_t);
extern void emit_CMPBW(uint32_t**, uint16_t);
extern void emit_CMPL(uint32_t**, uint16_t);
extern void emit_CMPAL(uint32_t**, uint16_t);
extern void emit_CMPAW(uint32_t**, uint16_t);
extern void emit_CMPB(uint32_t**, uint16_t);
extern void emit_CMPIBW(uint32_t**, uint16_t);
extern void emit_CMPIL(uint32_t**, uint16_t);
extern void emit_CMPML(uint32_t**, uint16_t);
extern void emit_CMPMB(uint32_t**, uint16_t);
extern void emit_CMPMW(uint32_t**, uint16_t);
extern void emit_CMPW(uint32_t**, uint16_t);
extern void emit_DBcc(uint32_t**, uint16_t);
extern void emit_DBRA(uint32_t**, uint16_t);
extern void emit_DIVS(uint32_t**, uint16_t);
extern void emit_DIVU(uint32_t**, uint16_t);
extern void emit_EORBW(uint32_t**, uint16_t);
extern void emit_EORL(uint32_t**, uint16_t);
extern void emit_EORIBW(uint32_t**, uint16_t);
extern void emit_EORIL(uint32_t**, uint16_t);
extern void emit_EORI_TO_CCR(uint32_t**, uint16_t);
extern void emit_EORI_TO_SR(uint32_t**, uint16_t);
extern void emit_EXG(uint32_t**, uint16_t);
extern void emit_EXTL(uint32_t**, uint16_t);
extern void emit_EXTW(uint32_t**, uint16_t);
extern void emit_FLINE(uint32_t**, uint16_t);
extern void emit_ILLEGAL(uint32_t**, uint16_t);
extern void emit_JMP(uint32_t**, uint16_t);
extern void emit_JSR(uint32_t**, uint16_t);
extern void emit_LEA(uint32_t**, uint16_t);
extern void emit_LINK(uint32_t**, uint16_t);
extern void emit_LSdBW(uint32_t**, uint16_t);
extern void emit_LSdL(uint32_t**, uint16_t);
extern void emit_LSd(uint32_t**, uint16_t);
extern void emit_MOVEL(uint32_t**, uint16_t);
extern void emit_MOVE_SR_TO(uint32_t**, uint16_t);
extern void emit_MOVE_TO_CCR(uint32_t**, uint16_t);
extern void emit_MOVE_TO_SR(uint32_t**, uint16_t);
extern void emit_MOVE_TO_USP(uint32_t**, uint16_t);
extern void emit_MOVE_USP_TO(uint32_t**, uint16_t);
extern void emit_MOVEAL(uint32_t**, uint16_t);
extern void emit_MOVEAW(uint32_t**, uint16_t);
extern void emit_MOVEB(uint32_t**, uint16_t);
extern void emit_MOVEM(uint32_t**, uint16_t);
extern void emit_MOVEP(uint32_t**, uint16_t);
extern void emit_MOVEQ(uint32_t**, uint16_t);
extern void emit_MOVEW(uint32_t**, uint16_t);
extern void emit_MULS(uint32_t**, uint16_t);
extern void emit_MULU(uint32_t**, uint16_t);
extern void emit_NBCD(uint32_t**, uint16_t);
extern void emit_NEGBW(uint32_t**, uint16_t);
extern void emit_NEGL(uint32_t**, uint16_t);
extern void emit_NEGXBW(uint32_t**, uint16_t);
extern void emit_NEGXL(uint32_t**, uint16_t);
extern void emit_NOP(uint32_t**, uint16_t);
extern void emit_NOTBW(uint32_t**, uint16_t);
extern void emit_NOTL(uint32_t**, uint16_t);
extern void emit_ORBW(uint32_t**, uint16_t);
extern void emit_ORL(uint32_t**, uint16_t);
extern void emit_ORIBW(uint32_t**, uint16_t);
extern void emit_ORIL(uint32_t**, uint16_t);
extern void emit_ORI_TO_CCR(uint32_t**, uint16_t);
extern void emit_ORI_TO_SR(uint32_t**, uint16_t);
extern void emit_ORW(uint32_t**, uint16_t);
extern void emit_PEA(uint32_t**, uint16_t);
extern void emit_RESET(uint32_t**, uint16_t);
extern void emit_ROdBW(uint32_t**, uint16_t);
extern void emit_ROdL(uint32_t**, uint16_t);
extern void emit_ROd(uint32_t**, uint16_t);
extern void emit_ROXdBW(uint32_t**, uint16_t);
extern void emit_ROXdL(uint32_t**, uint16_t);
extern void emit_ROXd(uint32_t**, uint16_t);
extern void emit_RTE(uint32_t**, uint16_t);
extern void emit_RTR(uint32_t**, uint16_t);
extern void emit_RTS(uint32_t**, uint16_t);
extern void emit_SBCD(uint32_t**, uint16_t);
extern void emit_Scc(uint32_t**, uint16_t);
extern void emit_STOP(uint32_t**, uint16_t);
extern void emit_SUBBW(uint32_t**, uint16_t);
extern void emit_SUBL(uint32_t**, uint16_t);
extern void emit_SUBAL(uint32_t**, uint16_t);
extern void emit_SUBAW(uint32_t**, uint16_t);
extern void emit_SUBIBW(uint32_t**, uint16_t);
extern void emit_SUBIL(uint32_t**, uint16_t);
extern void emit_SUBQBW(uint32_t**, uint16_t);
extern void emit_SUBQL(uint32_t**, uint16_t);
extern void emit_SUBW(uint32_t**, uint16_t);
extern void emit_SUBXBW(uint32_t**, uint16_t);
extern void emit_SUBXL(uint32_t**, uint16_t);
extern void emit_SWAP(uint32_t**, uint16_t);
extern void emit_TAS(uint32_t**, uint16_t);
extern void emit_TRAP(uint32_t**, uint16_t);
extern void emit_TRAPV(uint32_t**, uint16_t);
extern void emit_TSTBW(uint32_t**, uint16_t);
extern void emit_TSTL(uint32_t**, uint16_t);
extern void emit_UNLK(uint32_t**, uint16_t);

#endif