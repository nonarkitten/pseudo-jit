#include "pjit.h"
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

__attribute__((naked)) void C_SVC_Handler(uint32_t svc, uint32_t* stack) {
    // save r0~r3 immediately
    asm("strd   r0, r1, [r5, %0]" :: "i"(__offsetof(cpu_t, r0)));
    asm("strd   r2, r3, [r5, %0]" :: "i"(__offsetof(cpu_t, r2)));

    // change to system mode so we're using the same stack and LR
    uint32_t m68k_pc = cache_reverse();

    // switch User SP with Supervisor SP
    if(!(cpu->sr & 0x2000)) {
        asm("str    sp, [r5, %0]" :: "i"(__offsetof(cpu_t, usp)));
        asm("ldr    sp, [r5, %0]" :: "i"(__offsetof(cpu_t, ssp)));
    }
    // regardless of exception group, all will push the PC and SR
    asm("str    %0, [sp, #-4]!" :: "r"(m68k_pc));
    asm("strh   %0, [sp, #-2]!" :: "r"(cpu->sr));

    uint32_t new_m68k_pc;

    if(svc < BUSERR) {
        // Reset
        new_m68k_pc = *(uint32_t*)(cpu->vbr + 4);
        cpu->a7 = *(uint32_t*)cpu->vbr;

    } else {
        new_m68k_pc = *(uint32_t*)(cpu->vbr + svc * 4);
        if(svc <= ADDRESSERR) {
            // Exception group 0
            // push Instruction register
            uint16_t opcode = *(uint16_t*)m68k_pc;
            asm("strh   %0, [sp, #-2]!" :: "r"(opcode));
            // push Access address
            asm("ldr    r0, [r5, %0]!" :: "i"(__offsetof(cpu_t, r0)));
            asm("str    r0, [sp, #-4]!");
            // push Access type, b4=read, b3=instr, b2~0 = FC2~0
            uint16_t fc;
            asm("ldr    %0, [r5, %1]!" : "=r"(fc) : "i"(__offsetof(cpu_t, r1)));
            fc |= (fc & 0x0008) ? 0x0002 : 0x0001;
            if (cpu->sr & 0x2000) fc |= 0x0004;
            asm("strh   %0, [sp, #-2]!" :: "r"(fc));
        }
    }

    cpu->sr |= 0x2000;
    jump_normal(new_m68k_pc);
}