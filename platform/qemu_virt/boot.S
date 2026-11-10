/*
 * boot.S — ARM32 entry point for the QEMU "virt" machine: the exception vector table, one stack
 * per mode, and the cache and branch-predictor enable. The real boards start the same way
 * (platform/buffee/boot.S, platform/beaglebone/boot.S).
 *
 * What is deliberately different from the real boards:
 *  - No AM3358 watchdog-disable sequence (QEMU's "virt" board has no such hardware).
 *  - No NEON/VFP enable (nothing here needs it).
 *
 * The SVC vector goes to pjit_svc_entry and the IRQ vector to pjit_irq_entry (both in
 * pjit/runtime.s). The other vectors (undefined instruction, prefetch abort, data abort, FIQ)
 * report the fault and stop; a data abort first offers itself to a weak amiga_abort hook.
 */
.syntax unified
.arm
.section .text.boot

.global _start
.global Entry

.set MODE_USR, 0x10
.set MODE_FIQ, 0x11
.set MODE_IRQ, 0x12
.set MODE_SVC, 0x13
.set MODE_ABT, 0x17
.set MODE_UND, 0x1B
.set MODE_SYS, 0x1F
.set I_F_BIT,  0xC0

/* Real exception vector table. Installed via VBAR (below) rather than
 * relied on being at physical address 0, since QEMU's -kernel loader puts
 * us wherever the linker script says (0x40000000, see virt.ld). */
Entry:
        B       _start                  /* 0x00 Reset                  */
        LDR     pc, =Undef_Handler      /* 0x04 Undefined Instruction  */
        LDR     pc, =pjit_svc_entry     /* 0x08 Software Interrupt     */
        LDR     pc, =Prefetch_Handler   /* 0x0C Prefetch Abort         */
        LDR     pc, =Abort_Handler      /* 0x10 Data Abort             */
        B       .                       /* 0x14 Reserved                */
        LDR     pc, =pjit_irq_entry     /* 0x18 IRQ (pjit/runtime.s)   */
        LDR     pc, =FIQ_Handler        /* 0x1C FIQ                    */

/* A fault is reported and halts (a bare BKPT with no debugger attached just
 * loops, a silent hang). Each runs on its own stack: the
 * per-mode stacks set up below are only 8 bytes, and the faulting 68K
 * stack is exactly what cannot be trusted here. */
Undef_Handler:      MOV     r1, lr
                    MOV     r0, #1
                    B       Fault_Common
Prefetch_Handler:   MOV     r1, lr
                    MOV     r0, #2
                    B       Fault_Common
        .weak   amiga_abort
Abort_Handler:
        LDR     sp, =fault_stack_top
        PUSH    {r0-r12, lr}            /* [sp+0..48] r0-r12 (before anything touches them), [sp+52] lr (the faulting instruction + 8) */
        LDR     r4, =amiga_abort
        CMP     r4, #0
        BEQ     2f                      /* no hook linked: a fault is fatal, as before */
        SUB     sp, sp, #8
        STMIA   sp, {sp, lr}^           /* [sp+0] the interrupted mode's sp (the guest's A7), [sp+4] its lr */
        MOV     r0, sp
        MRS     r1, spsr
        MRC     p15, 0, r2, c5, c0, 0   /* DFSR */
        MRC     p15, 0, r3, c6, c0, 0   /* DFAR */
        BLX     r4                      /* int amiga_abort(frame, spsr, dfsr, dfar): 1 = handled, the instruction is skipped */
        CMP     r0, #0
        BEQ     1f
        LDMIA   sp, {sp, lr}^
        NOP
        ADD     sp, sp, #8
        LDMIA   sp!, {r0-r12, lr}
        SUBS    pc, lr, #4
1:      ADD     sp, sp, #8
2:      POP     {r0-r12, lr}
3:      MOV     r1, lr
        MOV     r0, #3
        B       Fault_Common
IRQ_Handler:        MOV     r1, lr
                    MOV     r0, #4
                    B       Fault_Common
FIQ_Handler:        MOV     r1, lr
                    MOV     r0, #5
                    B       Fault_Common

Fault_Common:
        SETEND  LE
        LDR     sp, =fault_stack_top
        MRS     r2, spsr                /* mode/T/E state at the fault */
        MRC     p15, 0, r3, c6, c0, 0   /* DFAR */
        BL      pjit_fault_report
        B       qemu_exit

.section .bss
.align 3
fault_stack:      .space 2048
fault_stack_top:
.section .text.boot

.global pjit_svc_entry     /* defined in pjit/runtime.s */
.global pjit_irq_entry

_start:
        /* Point VBAR at our real vector table (must happen before
         * anything below could fault) */
        LDR     r0, =Entry
        MCR     p15, 0, r0, c12, c0, 0

        /* Per-mode stacks. UND/ABT/FIQ get a token amount (these modes
         * aren't expected to run real code, just trap), IRQ gets a real
         * stack, SVC deliberately gets none: pjit_svc_entry uses the
         * banked sp and lr as scratch and switches to System mode before
         * it uses a stack. */
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

        /* Invalidate branch predictor + icache, enable branch prediction,
         * enable I/D caches (the MMU stays off, and QEMU faults on an
         * unaligned word or halfword access while it is off). */
        MOV     r0, #0
        MCR     p15, 0, r0, c7, c5, 6
        ISB
        MRC     p15, 0, r0, c1, c0, 0
        ORR     r0, r0, #0x1800         /* Z (branch pred) + I (icache) */
        ORR     r0, r0, #0x0004         /* C (dcache)                  */
        MCR     p15, 0, r0, c1, c0, 0
        ISB

        /* Zero .bss explicitly -- QEMU happens to start guest RAM zeroed,
           but real SDRAM doesn't, and pjit's cpu_state (a zero-initialized
           global) depends on this being genuinely zero, not "zero by
           accident". */
        LDR r0, =__bss_start
        LDR r1, =__bss_end
        MOV r2, #0
1:      CMP r0, r1
        BGE 2f
        STR r2, [r0], #4
        B   1b
2:
        BL  main
        B   qemu_exit          /* main() returned: fall through to a clean stop */
