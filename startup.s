@******************************************************************************
@
@ startup_ARMCA8.S - Init code routine for Cortex A8 cores and Linaro baremetal
@
@******************************************************************************
@
@ Copyright (C) 2013 Texas Instruments Incorporated - http:@@www.ti.com/
@  (Rafael de Souza - 2014) Added VFP and NEON initialization routines
@
@
@  Redistribution and use in source and binary forms, with or without
@  modification, are permitted provided that the following conditions
@  are met:
@
@    Redistributions of source code must retain the above copyright
@    notice, this list of conditions and the following disclaimer.
@
@    Redistributions in binary form must reproduce the above copyright
@    notice, this list of conditions and the following disclaimer in the
@    documentation and/or other materials provided with the
@    distribution.
@
@    Neither the name of Texas Instruments Incorporated nor the names of
@    its contributors may be used to endorse or promote products derived
@    from this software without specific prior written permission.
@
@  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
@  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
@  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
@  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
@  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
@  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
@  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
@  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
@  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
@  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
@  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
@
@******************************************************************************

@****************************** Global Symbols*******************************
        .global Entry
        .global _init
        .global main
        .global _stack
        .global _bss_start
        .global _bss_end
        .global start_boot

@************************ Internal Definitions ******************************
@
@ Define the stack sizes for different modes. The user/system mode will use
@ the rest of the total stack size
@
        .set  UND_STACK_SIZE, 0x8
        .set  ABT_STACK_SIZE, 0x8
        .set  FIQ_STACK_SIZE, 0x8
        .set  IRQ_STACK_SIZE, 0x1000
        .set  SVC_STACK_SIZE, 0x8

@
@ to set the mode bits in CPSR for different modes
@        
        .set  MODE_USR, 0x10            
        .set  MODE_FIQ, 0x11
        .set  MODE_IRQ, 0x12
        .set  MODE_SVC, 0x13
        .set  MODE_ABT, 0x17
        .set  MODE_UND, 0x1B
        .set  MODE_SYS, 0x1F            

        .equ  I_F_BIT, 0xC0               

@**************************** Code Section ***********************************
        .text

@
@ This code is assembled for ARM instructions
@
        .code 32

@******************************************************************************
@
@******************************************************************************
@
@ The reset handler sets up the stack pointers for all the modes. The FIQ and
@ IRQ shall be disabled during this. Then clears the BSS section, enters the  
@ main function. 

Entry:
@
@ Set up the Stack for Undefined mode
@
         LDR   r0, =_stack                     @ Read the stack address
         MSR   cpsr_c, #MODE_UND|I_F_BIT       @ switch to undef  mode
         MOV   sp,r0                           @ write the stack pointer
         SUB   r0, r0, #UND_STACK_SIZE         @ give stack space
@
@ Set up the Stack for abort mode
@
         MSR   cpsr_c, #MODE_ABT|I_F_BIT       @ Change to abort mode
         MOV   sp, r0                          @ write the stack pointer
         SUB   r0,r0, #ABT_STACK_SIZE          @ give stack space
@
@ Set up the Stack for FIQ mode
@
         MSR   cpsr_c, #MODE_FIQ|I_F_BIT       @ change to FIQ mode
         MOV   sp,r0                           @ write the stack pointer
         SUB   r0,r0, #FIQ_STACK_SIZE          @ give stack space
@
@ Set up the Stack for IRQ mode
@
         MSR   cpsr_c, #MODE_IRQ|I_F_BIT       @ change to IRQ mode
         MOV   sp,r0                           @ write the stack pointer
         SUB   r0,r0, #IRQ_STACK_SIZE          @ give stack space
@
@ Set up the Stack for SVC mode
@
         MSR   cpsr_c, #MODE_SVC|I_F_BIT       @ change to SVC mode
         MOV   sp,r0                           @ write the stack pointer
         SUB   r0,r0, #SVC_STACK_SIZE          @ give stack space
@
@ Set up the Stack for USer/System mode
@
         MSR   cpsr_c, #MODE_SYS|I_F_BIT       @ change to system mode
         MOV   sp,r0                           @ write the stack pointer

@
@ Invalidate and Enable Branch Prediction
@
		MOV		r0, #0
		MCR		p15, #0, r0, c7, c5, #6
		ISB
		MRC		p15, #0, r0, c1, c0, #0
		ORR		r0, r0, #0x00000800
		MCR		p15, #0, r0, c1, c0, #0

@
@ Set up the Vector Base Address Regsiter
@
		LDR		r0, = __isr_vector
		MCR		p15, 0, r0, c12, c0, 0                     @@ Write VBAR Register

@
@ Enable NEON extensions and the VFP. Must be done before entering user mode.
@
		MRC 	p15, #0, r3, c1, c0, #2                    @@ Read CPACR
		ORR 	r3, r3, #0x00F00000                        @@ Enable access to CP10 and CP11
		MCR 	p15, #0, r3, c1, c0, #2                    @@ Write CPACR
		MOV 	r3, #0
		MOV 	r0, #0x40000000
		FMXR	FPEXC, r0                                  @@ Set FPEXC bit 30 to enable VFP
		MCR 	p15, #0, r3, c7, c5, #4                    @@ flush prefetch buffer because of FMXR above

@
@ Disable the watchdog
@
		LDR		r0, =0x44E35000			                   @@ load SOC_WDT_1_REGS
		LDR		r1, =0xAAAA
		STR		r1, [r0, #0x48]			                   @@ store 0xaaaa to WDT_WSPR
1:		LDR		r1, [r0, #0x34]			                   @@ loop until WDT_WWPS is 0
		CMP		r1, #0x0
		BNE		1b
		LDR		r1, =0x5555
		STR		r1, [r0, #0x48]			                   @@ store 0x5555 to WDT_WSPR
2:		LDR		r1, [r0, #0x34]			                   @@ loop until WDT_WWPS is 0
		CMP		r1, #0x0
		BNE		2b

@
@ allow unaligned access, effective only when MMU is enabled
@
		MRC		p15, 0, r0, c1, c0, 0
		BIC		r0, #0x2
		MCR		p15, 0, r0, c1, c0, 0

@
@ Clear the BSS section here
@
         LDR   r0, =_bss_start                 			   @@ Start address of BSS
         LDR   r1, =(_bss_end - 0x04)                      @@ End address of BSS
         MOV   r2, #0
Clear_Bss_Section:
         STR   r2, [r0], #4                                @@ Clear one word in BSS
         CMP   r0, r1
         BLE   Clear_Bss_Section                           @@ Clear till BSS end

@
@ Enter the main function. 
@
@ Initialize the system
        LDR		r0, =_init
        BLX		r0

@ Call main
        LDR		r0, =main
        BLX		r0
@ Die
1:		NOP
		B		1b

@
@ Set the Interrupt vector table here
@

		.section .isr_vector
		.align 4
		.globl  __isr_vector
__isr_vector:
        LDR   pc, [pc, #24]                                 @@ 0x00 Reset
        LDR   pc, [pc, #24]                                 @@ 0x04 Undefined Instruction
        LDR   pc, [pc, #24]                                 @@ 0x08 Supervisor Call
        LDR   pc, [pc, #24]                                 @@ 0x0C Prefetch Abort
        LDR   pc, [pc, #24]                                 @@ 0x10 Data Abort
        LDR   pc, [pc, #-8]                                 @@ 0x14 Not used
        LDR   pc, [pc, #24]                                 @@ 0x18 IRQ interrupt
        LDR   pc, [pc, #24]                                 @@ 0x1C FIQ interrupt

        .long  Entry
        .long  Undefined_Instruction_Handler
        .long  SVC_Handler
        .long  Prefect_Abort_Handler
        .long  Data_Abort_Handler
        .long  0
        .long  IRQ_Handler
        .long  FIQ_Handler

@
@ External interrupts
@
        .long  Default_Handler
    
        .size  __isr_vector, . - __isr_vector

@
@ Macro to define default handlers. Default handler
@ will be weak symbol and just dead loops. They can be
@ overwritten by other handlers
@
        .macro  def_default_handler    handler_name
        .align 1
        .thumb_func
        .type  \handler_name, %function
\handler_name :
        B    .
        .size  \handler_name, . - \handler_name
        .endm
    
        def_default_handler  NMI_Handler
        def_default_handler  HardFault_Handler
        def_default_handler  SVC_Handler
        def_default_handler  PendSV_Handler
        def_default_handler  SysTick_Handler
        def_default_handler  Default_Handler
        def_default_handler  Undefined_Instruction_Handler
        def_default_handler  Prefect_Abort_Handler
        def_default_handler  Data_Abort_Handler
        def_default_handler  IRQ_Handler
        def_default_handler  FIQ_Handler

       .weak  DEF_IRQHandler
       .set  DEF_IRQHandler, Default_Handler
   
@
@ End of the file
@
       .end
    