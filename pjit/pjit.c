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

#include "pjit.h"
#include "pjit_extword.h"

extern const uint32_t optab[65536];
extern const uint8_t  oplen[65536];

static void cpu_lookup_inline(void);
static void cpu_lookup_noinline(void);
static void cpu_lookup_nojit(void);

// The following Copyrights are included as the original author wrote them
// modified BSD 3-Clause License requires inclusion of Copyright with binaries
__attribute__((used)) const char *ti_c = "Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/";
// Apache License requires inclusion of Copyright with binaries
__attribute__((used)) const char *libbbb_c = "Copyright 2015 University of Applied Sciences Western Switzerland / Fribourg";
// Mozilla Public License, v. 2.0 requires inclusion of Copyright with binaries
__attribute__((used)) const char *emu68_c = "Copyright © 2019 Michal Schulz <michal.schulz@gmx.de>";
// GPL does NOT require inclusion of Copyright with binaries but are included anyway
__attribute__((used)) const char *c68k_c1 = "Copyright (c) 2004,2011 Dave \"FinalDave\" Haywood (emudave (at) gmail.com)";
__attribute__((used)) const char *c68k_c2 = "Copyright (c) 2005-2011 Graûvydas \"notaz\" Ignotas (notasas (at) gmail.com)";
__attribute__((used)) const char *cast_c  = "Copyright (c) 1994-2002 Martin Döring, Joachim Hönig";
// Our copyright
__attribute__((used)) const char *buffe_c = "Copyright (c) 2020-2023 Renee Cousins, the Buffee Project - http://www.buffee.ca";

/*   ____          _        _____           _ _   _
    / ___|___   __| | ___  | ____|_ __ ___ (_) |_| |_ ___ _ __ ___
   | |   / _ \ / _` |/ _ \ |  _| | '_ ` _ \| | __| __/ _ \ '__/ __|
   | |__| (_) | (_| |  __/ | |___| | | | | | | |_| ||  __/ |  \__ \
    \____\___/ \__,_|\___| |_____|_| |_| |_|_|\__|\__\___|_|  |___/ */

// save and restore the CPU registers
void save_cpu(void) {
    asm __volatile("str\tr3, [%0,%1]" ::"r"(cpu), "i"(OFFSETOF(cpu_t, d0)));
    asm __volatile("str\tr4, [%0,%1]" ::"r"(cpu), "i"(OFFSETOF(cpu_t, d1)));
    uint32_t *base = &cpu->a0;
    asm __volatile("stm\t%0!, {r6-r13}" ::"r"(base));
}
void restore_cpu(void) {
    asm __volatile("ldr\tr3, [%0,%1]" ::"r"(cpu),
                   "i"(OFFSETOF(cpu_t, d0)));  // + 1 cycle
    asm __volatile("ldr\tr4, [%0,%1]" ::"r"(cpu),
                   "i"(OFFSETOF(cpu_t, d1)));          // + 1 cycle
    uint32_t *base = &cpu->a0;                         // + 1 cycle
    asm __volatile("ldm\t%0!, {r6-r13}" ::"r"(base));  // + 4 cycles
}

// return ARM32 opcodes for various instructions
uint32_t emit_cc_branch(uint8_t cc, int32_t offset) {
    return (cc << 24) | (0x00FFFFFF & (offset >> 2));
}
uint32_t emit_branch(uint32_t current, uint32_t target) {
    return emit_cc_branch(0xEB, target - current - 8);
}
uint32_t emit_movw(uint32_t reg, uint16_t value) {
    return 0xE3000000 | ((value & 0xF000) << 4) | (reg << 12) | (value & 0xFFF);
}
uint32_t emit_movt(uint32_t reg, uint16_t value) {
    return 0xE3400000 | ((value & 0xF000) << 4) | (reg << 12) | (value & 0xFFF);
}
uint32_t emit_src_ext(uint32_t current, uint16_t opcode) {
    static const uint32_t ext_handler[32] = {
        (uint32_t)extword_src_0000,
        (uint32_t)extword_src_1000,
        (uint32_t)extword_src_2000,
        (uint32_t)extword_src_3000,
        (uint32_t)extword_src_4000,
        (uint32_t)extword_src_5000,
        (uint32_t)extword_src_6000,
        (uint32_t)extword_src_7000,
        (uint32_t)extword_src_8000,
        (uint32_t)extword_src_9000,
        (uint32_t)extword_src_A000,
        (uint32_t)extword_src_B000,
        (uint32_t)extword_src_C000,
        (uint32_t)extword_src_D000,
        (uint32_t)extword_src_E000,
        (uint32_t)extword_src_F000,
        (uint32_t)extword_src_0800,
        (uint32_t)extword_src_1800,
        (uint32_t)extword_src_2800,
        (uint32_t)extword_src_3800,
        (uint32_t)extword_src_4800,
        (uint32_t)extword_src_5800,
        (uint32_t)extword_src_6800,
        (uint32_t)extword_src_7800,
        (uint32_t)extword_src_8800,
        (uint32_t)extword_src_9800,
        (uint32_t)extword_src_A800,
        (uint32_t)extword_src_B800,
        (uint32_t)extword_src_C800,
        (uint32_t)extword_src_D800,
        (uint32_t)extword_src_E800,
        (uint32_t)extword_src_F800,
    };
    return emit_branch(current,
                       ext_handler[opcode >> 11] + ((opcode & 0xFF) * 8));
}
static inline uint32_t emit_dst_ext(uint32_t current, uint16_t opcode) {
    static const uint32_t ext_handler[32] = {
        (uint32_t)extword_dst_0000,
        (uint32_t)extword_dst_1000,
        (uint32_t)extword_dst_2000,
        (uint32_t)extword_dst_3000,
        (uint32_t)extword_dst_4000,
        (uint32_t)extword_dst_5000,
        (uint32_t)extword_dst_6000,
        (uint32_t)extword_dst_7000,
        (uint32_t)extword_dst_8000,
        (uint32_t)extword_dst_9000,
        (uint32_t)extword_dst_A000,
        (uint32_t)extword_dst_B000,
        (uint32_t)extword_dst_C000,
        (uint32_t)extword_dst_D000,
        (uint32_t)extword_dst_E000,
        (uint32_t)extword_dst_F000,
        (uint32_t)extword_dst_0800,
        (uint32_t)extword_dst_1800,
        (uint32_t)extword_dst_2800,
        (uint32_t)extword_dst_3800,
        (uint32_t)extword_dst_4800,
        (uint32_t)extword_dst_5800,
        (uint32_t)extword_dst_6800,
        (uint32_t)extword_dst_7800,
        (uint32_t)extword_dst_8800,
        (uint32_t)extword_dst_9800,
        (uint32_t)extword_dst_A800,
        (uint32_t)extword_dst_B800,
        (uint32_t)extword_dst_C800,
        (uint32_t)extword_dst_D800,
        (uint32_t)extword_dst_E800,
        (uint32_t)extword_dst_F800,
    };
    return emit_branch(current,
                       ext_handler[opcode >> 11] + ((opcode & 0xFF) * 8));
}
static inline uint32_t emit_return(void) { return 0xE12FFF1E; }

/*  Look up code and if it's small enough, replace the caller with the
    function body, otherwise, replace with the branch to it and then execute it
 */
__attribute__((naked)) void lookup_opcode(void) {
    static const uint8_t arm_bcc[16] = {
        //       M68K OP Description    ARMcc
        0xEB,  // 0000 T  Always         1110
        0xFF,  // 0001 SR Subroutine     emulate
        0xFF,  // 0010 HI Higher         emulate
        0xFF,  // 0011 LS Lower/Same     emulate
        0x3B,  // 0100 CC Carry Clear    0011
        0x2B,  // 0101 CS Carry Set      0010
        0x1B,  // 0110 NE Not Equal      0001
        0x0B,  // 0111 EQ Equal          0000
        0x7B,  // 1000 VC Overflow Clear 0111
        0x6B,  // 1001 VS Overflow Set   0110
        0x5B,  // 1010 PL Plus           0101
        0x4B,  // 1011 MI Minus          0100
        0xAB,  // 1100 GE Greater/Equal  1010
        0xBB,  // 1101 LT Lesser         1011
        0xCB,  // 1110 GT Greater        1100
        0xDB,  // 1111 LE Lesser/Equal   1101
    };
    register uint32_t *out asm("lr");

    save_cpu();

    /* Back step to the original branch and calculate our 68K Program Counter */
    uint16_t *pc = (uint16_t *)cache_reverse((uint32_t)--out);

    uint32_t i      = 0;
    uint32_t opcode = pc[i++];
    uint32_t opaddr = optab[opcode];
    uint32_t opea   = oplen[opcode];

    /* Handle Extension Words */
    // if (opea & EXT_WORD_SRC_EXT) {
    //     out[i] = emit_src_ext((uint32_t)out, pc[i]);
    //     i += 1;
    // } else {
    //     if (opea & EXT_WORD_SRC_16B) {
    //         out[i] = emit_movw(1, pc[i]);
    //         i += 1;
    //     }
    //     if (opea & EXT_WORD_SRC_32B) {
    //         out[i] = emit_movt(1, pc[i]);
    //         i += 1;
    //     }
    // }

    // if (opea & EXT_WORD_DST_EXT) {
    //     out[i] = emit_dst_ext((uint32_t)out, pc[i]);
    //     i += 1;
    // } else {
    //     if (opea & EXT_WORD_DST_16B) {
    //         out[i] = emit_movw(2, pc[i]);
    //         i += 1;
    //     }
    //     if (opea & EXT_WORD_DST_32B) {
    //         out[i] = emit_movt(2, pc[i]);
    //         i += 1;
    //     }
    // }

    uint32_t opcode_out = 0;

    if ((opea & 0xFF) == 1) {
        /*  Can we inline? If so, then just copy it here.
            Note that the subroutine Bcc should NEVER report 1 line. */
        opcode_out = *(uint32_t *)opaddr;

    } else if ((opcode & 0xF001) == 0x6000) {
        /*  Check if we can in-line Bcc
            1. we cannot BSR, BHI or BLS, those need to be handled
            2. we cannot inline branch if the offset if out-of-page */
        uint32_t cc = arm_bcc[(opcode & 0x0F00) >> 8];
        if (cc != 0xFF)
            opcode_out =
                emit_cc_branch(cc, (uint32_t)(int32_t)(int8_t)(opcode & 0xFE));
    }
    if (opcode_out)
        out[i] = opcode_out;
    else
        out[i] = emit_branch((uint32_t)out, opaddr);

    restore_cpu();
    asm("bx\t%0" ::"r"(out));
}

// /*  Look up code and if it's small enough and replace our
//     branch to jump to the opcode; this omits branch inlining. We use
//     this version to pad the 'edges' of each page to avoid having to
//     check if our 8-bit branch is outside of the page.  */
// __attribute__((naked)) void cpu_lookup_noinline(void) {
//     register uint32_t *out asm("lr");

//     save_cpu();

//     /* Back step to the original branch and calculate our 68K Program Counter */
//     uint16_t *pc = (uint16_t *)cache_reverse((uint32_t)--out);

//     uint32_t i      = 0;
//     uint32_t opcode = pc[i++];
//     uint32_t opaddr = optab[opcode];
//     uint32_t opea   = oplen[opcode];

//     /* Handle Extension Words */
//     if (opea & EXT_WORD_SRC_EXT) {
//         out[i] = emit_src_ext((uint32_t)out, pc[i]);
//         i += 1;
//     } else {
//         if (opea & EXT_WORD_SRC_16B) {
//             out[i] = emit_movw(1, pc[i]);
//             i += 1;
//         }
//         if (opea & EXT_WORD_SRC_32B) {
//             out[i] = emit_movt(1, pc[i]);
//             i += 1;
//         }
//     }

//     if (opea & EXT_WORD_DST_EXT) {
//         out[i] = emit_dst_ext((uint32_t)out, pc[i]);
//         i += 1;
//     } else {
//         if (opea & EXT_WORD_DST_16B) {
//             out[i] = emit_movw(2, pc[i]);
//             i += 1;
//         }
//         if (opea & EXT_WORD_DST_32B) {
//             out[i] = emit_movt(2, pc[i]);
//             i += 1;
//         }
//     }

//     /*  Can we inline? If so, then just copy it here */
//     if ((opea & 0xFF) == 1)
//         out[i] = *(uint32_t *)opaddr;
//     else
//         out[i] = emit_branch((uint32_t)out, opaddr);

//     restore_cpu();
//     asm("bx\t%0" ::"r"(out));
// }

// /*  Look up code and execute it. This version will never replace the calling
//    branch. This is not the most efficient way to do this, since it needs to
//    waste a whole cache page. FIXME */
// __attribute__((naked)) void cpu_lookup_nojit(void) {
//     register uint32_t *entry asm("lr");
//     static uint32_t    out[10];

//     save_cpu();

//     /* Back step to the original branch and calculate our 68K Program Counter */
//     uint16_t *pc = (uint16_t *)cache_reverse((uint32_t)(entry - 1));

//     uint32_t i      = 0;
//     uint32_t opcode = pc[i++];
//     uint32_t opaddr = optab[opcode];
//     uint32_t opea   = oplen[opcode];

//     /* Handle Extension Words */
//     if (opea & EXT_WORD_SRC_EXT) {
//         out[i] = emit_src_ext((uint32_t)out, pc[i]);
//         i += 1;
//     } else {
//         if (opea & EXT_WORD_SRC_16B) {
//             out[i] = emit_movw(1, pc[i]);
//             i += 1;
//         }
//         if (opea & EXT_WORD_SRC_32B) {
//             out[i] = emit_movt(1, pc[i]);
//             i += 1;
//         }
//     }

//     if (opea & EXT_WORD_DST_EXT) {
//         out[i] = emit_dst_ext((uint32_t)out, pc[i]);
//         i += 1;
//     } else {
//         if (opea & EXT_WORD_DST_16B) {
//             out[i] = emit_movw(2, pc[i]);
//             i += 1;
//         }
//         if (opea & EXT_WORD_DST_32B) {
//             out[i] = emit_movt(2, pc[i]);
//             i += 1;
//         }
//     }

//     /*  Can we inline? If so, then just copy it here */
//     if ((opea & 0xFF) == 1)
//         out[i++] = *(uint32_t *)opaddr;
//     else
//         out[i++] = emit_branch((uint32_t)out, opaddr);
//     out[i++] = emit_return();

//     restore_cpu();
//     asm("bx\t%0" ::"r"(out));
// }

__attribute__((naked)) void cpu_jump(uint32_t m68k_pc) {
    register uint32_t *out asm("lr");
    // save_cpu();
    out = cache_find_entry(m68k_pc);
    // restore_cpu();
    asm("bx\t%0" ::"r"(out));
}

__attribute__((naked)) void cpu_subroutine(uint32_t m68k_pc) {
    register uint32_t *out asm("lr");
    uint16_t          *pc = (uint16_t *)cache_reverse((uint32_t)(out - 1));
    asm("str\t%0, [sp, #-4]!" ::"r"(pc));
    // save_cpu();
    out = cache_find_entry(m68k_pc);
    // restore_cpu();
    asm("bx\t%0" ::"r"(out));
}

__attribute__((naked)) void branch_normal(uint32_t nothing, int32_t offset) {
    register uint32_t *out asm("lr");
    uint32_t           m68k_pc = offset + cache_reverse((uint32_t)(out - 1));
    // save_cpu();
    out = cache_find_entry(m68k_pc);
    // restore_cpu();
    asm("bx\t%0" ::"r"(out));
}

__attribute__((naked)) void branch_subroutine(uint32_t nothing,
                                              int32_t  offset) {
    register uint32_t *out asm("lr");
    uint32_t           m68k_pc = cache_reverse((uint32_t)(out - 1));
    asm("str\t%0, [sp, #-4]!" ::"r"(m68k_pc));
    m68k_pc += offset;
    // save_cpu();
    out = cache_find_entry(m68k_pc);
    // restore_cpu();
    asm("bx\t%0" ::"r"(out));
}
