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
        B       Abort_Handler           @ 0x10 Data Abort
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
@ Data abort: buffee_unaligned() (buffee.c) emulates the faulting access (on a board without the byte swapper every access to the Amiga's address space; a long at a 2-aligned
@ address, which a 68000 allows and Device memory does not; a store into translated code) and the handler resumes after the instruction, or the guest gets a bus or address
@ error; anything else is fatal (Fault_Common). The guest's common accesses to the Amiga's address space on a board without the swapper take the fast path first.
@ pjit_irq_entry (runtime.s) pushed {r0-r3, r12, lr} and calls pjit_platform_irq: the interrupted host PC is the saved lr, 20 bytes up
        .weak   pjit_platform_irq               @ weak: tests/bbb/bbb_irq.c has its own (the performance monitor as the interrupt source)
pjit_platform_irq:
        LDR     r0, [sp, #20]
        LDR     r1, =buffee_irq_pc
        STR     r0, [r1]
        LDR     r1, =buffee_irq_r4
        STR     r4, [r1]
        B       buffee_irq_c

@ ---- the fast path: the guest's common accesses to the Amiga's address space on a board without the byte swapper ----------------------------------------------------
@ What buffee_bus_emulate() does in C, for a domain fault in domain 1 (the Amiga's address space, buffee.c) by the guest's code (System mode, ARM, big-endian) doing LDR / STR /
@ LDRB / STRB / LDRH / STRH / LDRSB / LDRSH with an immediate offset and any indexing, or with a register offset and no write-back, under any condition. The address accessed is DFAR (a domain fault comes before the access, an odd
@ word or long is an alignment fault and goes the general way), so the base register is only written (the write-back), never read. r0-r3 are kept in abt_save while it
@ runs: a register operand 0-3 is read and written there, 13 and 14 in the user bank, 4-12 where they are. Anything else leaves with every register as it was.
        .macro  REG_WRITE done                  @ register r3 (0-14) := r0; sp = abt_save; clobbers r3
        ADD     pc, pc, r3, lsl #3
        NOP
        STR     r0, [sp, #0]
        B       \done
        STR     r0, [sp, #4]
        B       \done
        STR     r0, [sp, #8]
        B       \done
        STR     r0, [sp, #12]
        B       \done
        .irp    r, r4, r5, r6, r7, r8, r9, r10, r11, r12
        MOV     \r, r0
        B       \done
        .endr
        STR     r0, [sp, #16]                   @ 13: the user bank's sp (A7)
        B       .Lwsp\@
        STR     r0, [sp, #16]                   @ 14: its lr
        ADD     r3, sp, #16
        LDMIA   r3, {lr}^
        B       \done
.Lwsp\@:
        ADD     r3, sp, #16
        LDMIA   r3, {sp}^
        B       \done
        .endm

        .macro  REG_READ done                   @ r0 := register r3 (0-14); sp = abt_save; clobbers r3
        ADD     pc, pc, r3, lsl #3
        NOP
        LDR     r0, [sp, #0]
        B       \done
        LDR     r0, [sp, #4]
        B       \done
        LDR     r0, [sp, #8]
        B       \done
        LDR     r0, [sp, #12]
        B       \done
        .irp    r, r4, r5, r6, r7, r8, r9, r10, r11, r12
        MOV     r0, \r
        B       \done
        .endr
        B       .Lrsp\@                         @ 13
        NOP
        ADD     r3, sp, #16                     @ 14
        STMIA   r3, {lr}^
        NOP
        LDR     r0, [sp, #16]
        B       \done
.Lrsp\@:
        ADD     r3, sp, #16
        STMIA   r3, {sp}^
        NOP
        LDR     r0, [sp, #16]
        B       \done
        .endm

        .macro  WRITEBACK acc                   @ r1 = the offset, r2 = the instruction: the address (DFAR) to [sp, #20], the base register written back as the indexing says
        MRC     p15, 0, r0, c6, c0, 0           @ DFAR
        STR     r0, [sp, #20]
        TST     r2, #0x01000000                 @ P
        BEQ     .Lpost\@
        TST     r2, #0x00200000                 @ W
        BEQ     \acc
        B       .Lwb\@                          @ pre-indexed with write-back: the base becomes the address
.Lpost\@:
        TST     r2, #0x00200000                 @ post-indexed with W: the unprivileged forms (LDRT, ...)
        BNE     .Lslow
        TST     r2, #0x00800000                 @ U
        ADDNE   r0, r0, r1
        SUBEQ   r0, r0, r1
.Lwb\@:
        UBFX    r3, r2, #16, #4
        REG_WRITE \acc
        .endm

Abort_Handler:
        LDR     sp, =abt_save
        STMIA   sp, {r0-r3}
        MRC     p15, 0, r0, c5, c0, 0           @ DFSR: a domain fault (section 9, page B) in domain 1
        TST     r0, #0x400
        BNE     .Lslow
        AND     r0, r0, #0xFF
        CMP     r0, #0x19
        CMPNE   r0, #0x1B
        BNE     .Lslow
        MRS     r1, spsr                        @ the guest: System mode, ARM, big-endian
        MOVW    r2, #0x23F
        AND     r1, r1, r2
        MOVW    r2, #0x21F
        CMP     r1, r2
        BNE     .Lslow
        LDR     r2, [lr, #-8]                   @ the faulting instruction
        LSR     r1, r2, #28
        CMP     r1, #0xF                        @ any condition (a load or store whose condition fails cannot fault), not the unconditional space
        BEQ     .Lslow
        UBFX    r1, r2, #12, #4                 @ Rt and Rn: not the PC
        CMP     r1, #15
        UBFXNE  r1, r2, #16, #4
        CMPNE   r1, #15
        BEQ     .Lslow
        AND     r1, r2, #0x0E000000
        CMP     r1, #0x04000000
        BEQ     .Lword                          @ 010: LDR / STR / LDRB / STRB, immediate offset
        CMP     r1, #0x06000000
        BEQ     .Lword_reg                      @ 011: the same with a register offset
        CMP     r1, #0
        BNE     .Lslow
        AND     r1, r2, #0x90                   @ 000: the halfword forms (bits 7 and 4 set, SH not 0) ...
        CMP     r1, #0x90
        BNE     .Lslow
        ANDS    r1, r2, #0x60
        BEQ     .Lslow
        TST     r2, #0x00100000                 @ ... and of the stores only STRH (L = 0 with SH 10 / 11 is LDRD / STRD)
        BNE     1f
        CMP     r1, #0x20
        BNE     .Lslow
1:      TST     r2, #0x00400000                 @ I: an immediate offset
        BEQ     .Lhalf_reg
        AND     r1, r2, #0xF                    @ the offset, imm4H:imm4L
        AND     r3, r2, #0xF00
        ORR     r1, r1, r3, lsr #4
        WRITEBACK .Lhalf
.Lhalf:
        LDR     r0, [sp, #20]
        BIC     r3, r0, #0xFF000000
        ORR     r3, r3, #0x02000000             @ the window's own address (domain 0)
        TST     r2, #0x00100000
        BEQ     .Lst_half
        AND     r1, r2, #0x60                   @ SH: 01 LDRH, 10 LDRSB, 11 LDRSH
        CMP     r1, #0x40
        BEQ     .Lld_byte
        LDRH    r0, [r3]                        @ straight lanes, a little-endian load: the 68000's word
        CMP     r1, #0x60
        SXTHEQ  r0, r0
        B       .Lld_done
.Lst_half:
        UBFX    r3, r2, #12, #4
        REG_READ .Lst_half_go
.Lst_half_go:
        LDR     r1, [sp, #20]
        BIC     r1, r1, #0xFF000000
        ORR     r1, r1, #0x02000000
        STRH    r0, [r1]
        B       .Ldone
.Lhalf_reg:                                    @ a register offset: only without write-back (the address is DFAR, the offset register is never needed)
        AND     r3, r2, #0x01200000             @ P, W
        CMP     r3, #0x01000000
        BNE     .Lslow
        MRC     p15, 0, r0, c6, c0, 0
        STR     r0, [sp, #20]
        B       .Lhalf
.Lword_reg:
        TST     r2, #0x10                       @ (bit 4 set: the media instructions)
        BNE     .Lslow
        AND     r3, r2, #0x01200000
        CMP     r3, #0x01000000
        BNE     .Lslow
        MRC     p15, 0, r0, c6, c0, 0
        STR     r0, [sp, #20]
        B       .Lword_acc
.Lword:
        UBFX    r1, r2, #0, #12                 @ the offset
        WRITEBACK .Lword_acc
.Lword_acc:
        LDR     r0, [sp, #20]
        BIC     r3, r0, #0xFF000000
        ORR     r3, r3, #0x02000000
        TST     r2, #0x00100000
        BEQ     .Lst_word
        TST     r2, #0x00400000                 @ B
        BNE     .Lld_byte
        LDRH    r0, [r3]                        @ a long: always two 16-bit accesses
        LDRH    r1, [r3, #2]
        ORR     r0, r1, r0, lsl #16
        B       .Lld_done
.Lld_byte:                                      @ (LDRB, or LDRSB with SH = 10 in r1)
        BIC     r0, r3, #1
        LDRH    r0, [r0]
        TST     r3, #1
        LSREQ   r0, r0, #8                      @ an even address is the word's high byte
        ANDNE   r0, r0, #0xFF
        TST     r2, #0x0E000000                 @ the halfword class: LDRSB
        SXTBEQ  r0, r0
        B       .Lld_done
.Lst_word:
        UBFX    r3, r2, #12, #4
        REG_READ .Lst_word_go
.Lst_word_go:
        LDR     r1, [sp, #20]
        BIC     r1, r1, #0xFF000000
        ORR     r1, r1, #0x02000000
        TST     r2, #0x00400000
        BNE     .Lst_byte
        LSR     r3, r0, #16
        STRH    r3, [r1]
        STRH    r0, [r1, #2]
        B       .Ldone
.Lst_byte:
        EOR     r1, r1, #1                      @ the other lane, whose strobe the no-swapper GreenPAK image raises
        STRB    r0, [r1]
        B       .Ldone
.Lld_done:
        UBFX    r3, r2, #12, #4
        REG_WRITE .Ldone
.Ldone:
        MOVW    r1, #:lower16:buffee_bus_traps
        MOVT    r1, #:upper16:buffee_bus_traps
        LDR     r0, [r1]
        ADD     r0, r0, #1
        STR     r0, [r1]
        LDMIA   sp, {r0-r3}
        SUBS    pc, lr, #4                      @ on to the next instruction
.Lslow:
        LDMIA   sp, {r0-r3}

@ The frame holds the interrupted mode's own sp and lr: the user bank for User and System mode (the guest; its sp is A7), the banked pair otherwise (the platform's C in IRQ or SVC
@ mode, whose loads from the Amiga window are emulated too and may use either as a base). They are written back to that mode on the way out.
        LDR     sp, =fault_stack_top
        PUSH    {r0-r12, lr}                    @ [sp+0..48] r0-r12, [sp+52] lr (= the faulting instruction + 8)
        SUB     sp, sp, #8
        MRS     r1, spsr
        AND     r2, r1, #0x1F
        CMP     r2, #0x10                       @ User
        CMPNE   r2, #0x1F                       @ System
        STMIAEQ sp, {sp, lr}^                   @ [sp+0] the interrupted mode's sp, [sp+4] its lr
        BEQ     3f
        MRS     r3, cpsr
        ORR     r2, r2, #0xC0                   @ the interrupted mode, IRQ and FIQ masked
        MSR     cpsr_c, r2
        MOV     r4, sp
        MOV     r5, lr
        MSR     cpsr_c, r3
        STMIA   sp, {r4, r5}
3:      MOV     r0, sp
        MRC     p15, 0, r2, c5, c0, 0           @ DFSR
        MRC     p15, 0, r3, c6, c0, 0           @ DFAR
        BL      buffee_unaligned
        MOVS    r6, r0                          @ 0: fatal; 1: on to the next instruction; 2: the handler unprotected the page and posted a flush, run the store again
        BEQ     1f
        MRS     r1, spsr
        AND     r2, r1, #0x1F
        CMP     r2, #0x10
        CMPNE   r2, #0x1F
        LDMIAEQ sp, {sp, lr}^
        BEQ     4f
        LDMIA   sp, {r4, r5}
        MRS     r3, cpsr
        ORR     r2, r2, #0xC0
        MSR     cpsr_c, r2
        MOV     sp, r4
        MOV     lr, r5
        MSR     cpsr_c, r3
4:      NOP
        ADD     sp, sp, #8
        CMP     r6, #2
        LDMIA   sp!, {r0-r12, lr}
        SUBSNE  pc, lr, #4                      @ on to the next instruction
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
abt_save:       .space 32               @ the fast path's r0-r3, a user-bank register, the address
fault_stack:    .space 16384
fault_stack_top:
