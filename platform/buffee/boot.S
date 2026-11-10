@ Buffee payload boot (a copy of platform/beaglebone/boot.S; main's return value is the exit code); BeagleBone boot: entered from U-Boot (`go 0x82000000`) in SVC mode, little-endian, IRQs off, MMU and caches in whatever state U-Boot left.
@ Saves everything U-Boot needs back (callee-saved registers, sp, lr, CPSR and the CP15 state we are about to change), sets up our
@ own vectors and stacks, and calls bbb_init() (MMU, caches, log, watchdog, cycle counter), then main(). bbb_return gives the CPU back.
        .syntax unified
        .arm
        .section .text.boot

        .global _start
        .global Entry
        .global bbb_return
        .global pjit_svc_entry          @ pjit/runtime.s
        .global pjit_irq_entry

        .set MODE_SVC, 0x13
        .set MODE_ABT, 0x17
        .set MODE_UND, 0x1B
        .set MODE_IRQ, 0x12
        .set MODE_FIQ, 0x11
        .set MODE_SYS, 0x1F
        .set I_F_BIT,  0xC0

Entry:
        B       _start                  @ 0x00 Reset (also where `go` lands: the image starts here)
        LDR     pc, =Undef_Handler      @ 0x04 Undefined Instruction
        LDR     pc, =pjit_svc_entry     @ 0x08 Software Interrupt
        LDR     pc, =Prefetch_Handler   @ 0x0C Prefetch Abort
        LDR     pc, =Abort_Handler      @ 0x10 Data Abort
        B       .                       @ 0x14 Reserved
        LDR     pc, =pjit_irq_entry     @ 0x18 IRQ
        LDR     pc, =FIQ_Handler        @ 0x1C FIQ
        .ltorg

Undef_Handler:      MOV     r1, lr
                    MOV     r0, #1
                    B       Fault_Common
Prefetch_Handler:   MOV     r1, lr
                    MOV     r0, #2
                    B       Fault_Common
@ Data abort: a guest LDR / STR of a long at an address that is 2-aligned but not 4-aligned (a 68000 allows it) faults on the Amiga window, which is Device memory.
@ buffee_unaligned() (buffee.c) redoes the access as two halfwords and the handler resumes after the instruction; anything else is fatal (Fault_Common).
@ pjit_irq_entry (runtime.s) pushed {r0-r3, r12, lr} and calls pjit_platform_irq: the interrupted host PC is the saved lr, 20 bytes up
        .weak   pjit_platform_irq               @ weak: tests/bbb/bbb_irq.c has its own (the performance monitor as the interrupt source)
pjit_platform_irq:
        LDR     r0, [sp, #20]
        LDR     r1, =buffee_irq_pc
        STR     r0, [r1]
        LDR     r1, =buffee_irq_r4
        STR     r4, [r1]
        B       buffee_irq_c

Abort_Handler:
        LDR     sp, =fault_stack_top
        PUSH    {r0-r12, lr}                    @ [sp+0..48] r0-r12, [sp+52] lr (= the faulting instruction + 8)
        SUB     sp, sp, #8
        STMIA   sp, {sp, lr}^                   @ [sp+0] the interrupted mode's sp (the guest's A7), [sp+4] its lr
        MOV     r0, sp
        MRS     r1, spsr
        MRC     p15, 0, r2, c5, c0, 0           @ DFSR
        MRC     p15, 0, r3, c6, c0, 0           @ DFAR
        BL      buffee_unaligned
        CMP     r0, #0
        BEQ     1f
        CMP     r0, #2                          @ 2: the handler unprotected the page and posted a flush: run the store again
        BEQ     2f
        LDMIA   sp, {sp, lr}^
        NOP
        ADD     sp, sp, #8
        LDMIA   sp!, {r0-r12, lr}
        SUBS    pc, lr, #4                      @ on to the next instruction
2:      LDMIA   sp, {sp, lr}^
        NOP
        ADD     sp, sp, #8
        LDMIA   sp!, {r0-r12, lr}
        SUBS    pc, lr, #8                      @ back to the faulting instruction
1:      ADD     sp, sp, #8
        POP     {r0-r12, lr}
        MOV     r1, lr
        MOV     r0, #3
        B       Fault_Common
FIQ_Handler:        MOV     r1, lr
                    MOV     r0, #5
                    B       Fault_Common

Fault_Common:
        SETEND  LE
        LDR     sp, =fault_stack_top
        STMDB   sp!, {r0-r12}           @ r0 = fault type, r1 = lr, the rest as the faulting code had them
        MRS     r2, spsr                @ mode/T/E state at the fault
        MRC     p15, 0, r3, c6, c0, 0   @ DFAR
        BL      pjit_fault_report       @ prints into the log
        MOV     r0, sp
        BL      bbb_fault_dump          @ registers and the instruction words around lr
        BL      bbb_fault_exit          @ marks the log, hands the CPU back to U-Boot; does not return
        B       .

_start:
        @ ---- save U-Boot's state ------------------------------------------------------------------------------------------
        LDR     r12, =uboot_ctx
        STMIA   r12!, {r4-r11, sp, lr}          @ 10 words
        MRS     r0, cpsr
        MRC     p15, 0, r1, c1, c0, 0           @ SCTLR
        MRC     p15, 0, r2, c1, c0, 1           @ ACTLR
        MRC     p15, 0, r3, c2, c0, 0           @ TTBR0
        MRC     p15, 0, r4, c2, c0, 2           @ TTBCR
        MRC     p15, 0, r5, c3, c0, 0           @ DACR
        MRC     p15, 0, r6, c12, c0, 0          @ VBAR
        STMIA   r12, {r0-r6}

        @ ---- our vectors, stacks ---------------------------------------------------------------------------------------------
        LDR     r0, =Entry
        MCR     p15, 0, r0, c12, c0, 0          @ VBAR
        LDR     r0, =_stack_top
        MSR     cpsr_c, #MODE_UND | I_F_BIT
        MOV     sp, r0
        SUB     r0, r0, #8
        MSR     cpsr_c, #MODE_ABT | I_F_BIT
        MOV     sp, r0
        SUB     r0, r0, #8
        MSR     cpsr_c, #MODE_FIQ | I_F_BIT
        MOV     sp, r0
        SUB     r0, r0, #8
        MSR     cpsr_c, #MODE_IRQ | I_F_BIT
        MOV     sp, r0
        SUB     r0, r0, #0x1000
        MSR     cpsr_c, #MODE_SYS | I_F_BIT
        MOV     sp, r0

        @ ---- .bss (the image is not zeroed by U-Boot) ---------------------------------------------------------------------
        LDR     r0, =__bss_start
        LDR     r1, =__bss_end
        MOV     r2, #0
1:      CMP     r0, r1
        BGE     2f
        STR     r2, [r0], #4
        B       1b
2:
        BL      bbb_init
        BL      main
        BL      bbb_exit                        @ main's result goes back to the caller (the bootloader: 0x100 = restart after an Amiga reset)

        @ ---- bbb_return(r0 = exit code): put U-Boot's CPU state back and return to it ----------------------------------------
        @ Called last, with the data cache already cleaned (bbb_exit does that). Any mode, any endianness.
bbb_return:
        MSR     cpsr_c, #MODE_SVC | I_F_BIT
        SETEND  LE
        LDR     r12, =uboot_ctx
        ADD     r12, r12, #40
        LDMIA   r12, {r1-r7}                    @ r1 cpsr, r2 sctlr, r3 actlr, r4 ttbr0, r5 ttbcr, r6 dacr, r7 vbar
        @ MMU and caches off while the tables change
        MRC     p15, 0, r8, c1, c0, 0
        BIC     r8, r8, #0x1800                 @ Z (branch prediction), I
        BIC     r8, r8, #0x0005                 @ C, M
        MCR     p15, 0, r8, c1, c0, 0
        ISB
        MOV     r8, #0
        MCR     p15, 0, r8, c8, c7, 0           @ TLBIALL
        MCR     p15, 0, r8, c7, c5, 0           @ ICIALLU
        MCR     p15, 0, r8, c7, c5, 6           @ BPIALL
        DSB
        ISB
        MCR     p15, 0, r4, c2, c0, 0           @ TTBR0
        MCR     p15, 0, r5, c2, c0, 2           @ TTBCR
        MCR     p15, 0, r6, c3, c0, 0           @ DACR
        MCR     p15, 0, r7, c12, c0, 0          @ VBAR
        MCR     p15, 0, r3, c1, c0, 1           @ ACTLR
        ISB
        MCR     p15, 0, r2, c1, c0, 0           @ SCTLR: U-Boot's MMU and caches back
        ISB
        MSR     cpsr_fsxc, r1                   @ U-Boot's mode, flags, interrupt masks, E = 0
        LDR     r12, =uboot_ctx
        LDMIA   r12, {r4-r11, sp, lr}
        BX      lr                              @ r0 = the exit code: `go` returns

        .ltorg

        .section .data
        .align 3
uboot_ctx:      .space 80

        .section .bss
        .align 3
fault_stack:    .space 16384
fault_stack_top:
