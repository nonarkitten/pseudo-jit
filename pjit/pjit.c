/*
 * Copyright (c) 2020-2023 Renee Cousins, the Buffee Project - http://www.buffee.ca
 *
 * Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (see LICENSE).
 */

/// @file pjit.c
/// @brief Per-opcode helpers computed from the extension-word table: how many
///        guest words an opcode occupies, and whether an ARM word reads a
///        register. tests/qemu/smoke_ot.c checks the generated tables against
///        them; nothing here runs while guest code runs (the runtime is
///        pjit/runtime.s and pjit/runtime_dr.s).
/// @see docs/design/OVERVIEW.md

#include "pjit.h"

typedef enum {
    EXT_NONE,
    EXT_I16,
    EXT_I32,
    EXT_D8_XAREG,
    EXT_D16_PC,
    EXT_D8_XPC,
    EXT_BDISP,      // source only: Bcc/BRA.W's 16-bit displacement, biased by -2
    EXT_COUNT
} ext_t;

// The flat opcode -> extension-mode table of ext_table.c (maintained by hand; see the header of that file).
extern const uint8_t pjit_ext_table[65536];

/// @brief Extension-word modes for an opcode, packed (src << 4) | dst.
static uint8_t get_ext(uint16_t opcode) {
    return pjit_ext_table[opcode];
}

/// @brief How many 68K words (the opcode and its extension words) `opcode`
///        occupies, from its extension modes.
uint32_t pjit_ext_word_count(uint16_t opcode) {
    uint32_t ext = get_ext(opcode);
    uint32_t src = 0, dst = 0;

    switch ((ext_t)((ext >> 4) & 0x0F)) {
    case EXT_I16: src = 1; break;
    case EXT_I32: src = 2; break;
    case EXT_D8_XAREG: src = 1; break;
    case EXT_D16_PC: src = 1; break;
    case EXT_D8_XPC: src = 1; break;
    case EXT_BDISP: src = 1; break;
    default: break;
    }
    switch ((ext_t)(ext & 0x0F)) {
    case EXT_I16: dst = 1; break;
    case EXT_I32: dst = 2; break;
    case EXT_D8_XAREG: dst = 1; break;
    case EXT_D16_PC: dst = 1; break;
    case EXT_D8_XPC: dst = 1; break;
    default: break;
    }

    return 1u + src + dst;
}

/// Does ARM word `w` read register `r`? Conservative for the encodings the
/// generator uses as leaders; anything unrecognised reads nothing.
int pjit_arm_reads_reg(uint32_t w, unsigned r) {
    unsigned rn = (w >> 16) & 15, rd = (w >> 12) & 15, rm = w & 15, rs = (w >> 8) & 15;
    switch ((w >> 26) & 3) {
    case 0:
        if ((w & 0x0FF00000u) == 0x03000000u) return 0;                 // movw
        if ((w & 0x0FF00000u) == 0x03400000u) return rd == r;           // movt
        if ((w & 0x0E000090u) == 0x00000090u && (w & 0x60)) {           // ldrh/strh/ldrsb/...
            int store = !(w & (1u << 20));
            return rn == r || (!(w & (1u << 22)) && rm == r) || (store && rd == r);
        }
        {
            unsigned opc = (w >> 21) & 15;
            int uses_rn = !(opc == 13 || opc == 15);
            int imm = (w >> 25) & 1;
            return (uses_rn && rn == r) || (!imm && rm == r) || (!imm && (w & 0x10) && rs == r);
        }
    case 1: {
        int store = !(w & (1u << 20));
        return rn == r || (((w >> 25) & 1) && rm == r) || (store && rd == r);
    }
    default:
        return 0;
    }
}
