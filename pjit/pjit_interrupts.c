#include "pjit.h"

// void ExceptionGroup0(
//     int           number,
//     unsigned long address,
//     int           ReadWrite) {
//     short sr = GetSRW(), context = 0;
//     #ifdef DEBUG
//     ON_TRAP(number);
//     assert(cpu_state != -3);
//     #endif
//     if (cpu_state == 0) {
//         cpu_state = -3;
//         longjmp(trap_buf, 1);
//     }
//     #if CPU_TYPE == 68000
//     else if (cpu_state > 0) {
//         context |= 0x8;
//     }
//     if (ReadWrite) context |= 0x10;
//     #else
//     if (ReadWrite) context |= 0x100;
//     #endif
//     if (GetS()) context |= 0x4;
//     if (ReadWrite && address == pc)
//         context |= 0x2;
//     else
//         context |= 0x1;
//     cpu_state = 0; /* begin group 0 exception processing */
//     SetS(1);
//     SetT(0);
//     #if CPU_TYPE == 68010
//     areg[7] -= 44; /* Rerun info */
//     areg[7] -= 4;
//     SetMemL(areg[7], address); /* fault address */
//     areg[7] -= 2;
//     SetMemW(areg[7], context);
//     areg[7] -= 2;
//     SetMemW(areg[7], 0x8000 | (number * 4));
//     areg[7] -= 4;
//     SetMemL(areg[7], GetPC());
//     areg[7] -= 2;
//     SetMemW(areg[7], sr);
//     #else
//     areg[7] = areg[7] - 14;
//     SetMemW(areg[7], context);
//     SetMemL(areg[7] + 2, address);
//     SetMemW(areg[7] + 6, inst);
//     SetMemW(areg[7] + 8, sr);
//     SetMemL(areg[7] + 10, GetPC());
//     #endif
//     SetPC(GetMemL((long)number * 4) + vbr);
//     /* end exception processing */
//     cpu_state = -1;
// }

// void ExceptionGroup1(int number) {
//     short sr = GetSRW();
//     #ifdef DEBUG
//     ON_TRAP(number);
//     #endif
//     cpu_state = 1; /* begin group 1 exception processing */
//     switch (number) {
//         case LINE_A:
//         case LINE_F:
//         case PRIV:
//         case ILLINSTR:
//             SetPC(GetPC() - 2);
//             break;
//     }
//     SetS(1);
//     SetT(0);
//     #if CPU_TYPE == 68010
//     areg[7] -= 2;
//     SetMemW(areg[7], number * 4);
//     #endif
//     areg[7] -= 4;
//     SetMemL(areg[7], GetPC());
//     areg[7] -= 2;
//     SetMemW(areg[7], sr);
//     SetPC(GetMemL((long)number * 4) + vbr);
//     /* end exception processing */
//     cpu_state = -1;
// }

// void Interrupt(void) {
//     short sr     = GetSRW();
//     int   number = QueryIRQ(intpri);
//     #ifdef DEBUG
//     ON_TRAP(number);
//     #endif
//     cpu_state = 1; /* begin group 1 exception processing */
//     SetI(intpri);
//     SetS(1);
//     SetT(0);
//     #if CPU_TYPE == 68010
//     areg[7] -= 2;
//     SetMemW(areg[7], number * 4);
//     #endif
//     areg[7] -= 4;
//     SetMemL(areg[7], GetPC());
//     areg[7] -= 2;
//     SetMemW(areg[7], sr);
//     SetPC(GetMemL((long)number * 4) + vbr);
//     /* end exception processing */
//     cpu_state = -1;
// }

// void ExceptionGroup2(int number) {
//     short sr = GetSRW();
//     #ifdef DEBUG
//     ON_TRAP(number);
//     #endif
//     cpu_state = 2; /* begin group 2 exception processing */
//     SetS(1);
//     SetT(0);
//     #if CPU_TYPE == 68010
//     areg[7] -= 2;
//     SetMemW(areg[7], number * 4);
//     #endif
//     areg[7] -= 4;
//     SetMemL(areg[7], GetPC());
//     areg[7] -= 2;
//     SetMemW(areg[7], sr);
//     SetPC(GetMemL((long)number * 4) + vbr);
//     /* end exception processing */
//     cpu_state = -1;
// }


__attribute__((naked)) void C_SVC_Handler(uint32_t svc, uint32_t* stack) {
    // save r0~r3 immediately
    asm("strd   r0, r1, [r5, %0]" :: "i"(offsetof(cpu_t, r0)));
    asm("strd   r2, r3, [r5, %0]" :: "i"(offsetof(cpu_t, r2)));

    // change to system mode so we're using the same stack and LR
    register uint32_t *out asm("lr");
    uint32_t           m68k_pc = cache_reverse((uint32_t)(out - 1));

    // switch User SP with Supervisor SP
    if(!(cpu->sr & 0x2000)) {
        asm("str    sp, [r5, %0]" :: "i"(offsetof(cpu_t, usp)));
        asm("ldr    sp, [r5, %0]" :: "i"(offsetof(cpu_t, ssp)));
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
            asm("ldr    r0, [r5, %0]!" :: "i"(offsetof(cpu_t, r0)));
            asm("str    r0, [sp, #-4]!");
            // push Access type, b4=read, b3=instr, b2~0 = FC2~0
            uint16_t fc;
            asm("ldr    %0, [r5, %1]!" : "=r"(fc) : "i"(offsetof(cpu_t, r1)));
            fc |= (fc & 0x0008) ? 0x0002 : 0x0001;
            if (cpu->sr & 0x2000) fc |= 0x0004;
            asm("strh   %0, [sp, #-2]!" :: "r"(fc));
        }
    }

    cpu->sr |= 0x2000;
    jump_normal(new_m68k_pc);
}