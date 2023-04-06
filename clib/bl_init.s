@******************************************************************************
@
@ init.S - Init code routines
@
@******************************************************************************
@
@ Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/
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
        .global _stack_top
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

@**************************** Code Seection ***********************************
        .text
        .section .startup

@
@ This code is assembled for ARM instructions
@
        .code 32

@ Protracted version of the interrupt table
Entry:
        B       Reset_Handler           /* Reset/Main Entry Point */
        LDR     pc, =Undef_Handler      /* 0x4  Undefined Instruction */
        LDR     pc, =SVC_Handler        /* 0x8  Software Interrupt */
        LDR     pc, =Prefetch_Handler   /* 0xC  Prefetch Abort */
        LDR     pc, =Abort_Handler      /* 0x10 Data Abort */
        B .                             /* 0x14 Reserved */
        LDR     pc, =IRQ_Handler        /* 0x18 IRQ (Regular IRQ)*/
        LDR     pc, =FIQ_Handler        /* 0x18 FIQ (Fast IRQ) */

@ Each of these will remain in SRAM and can be overridden by the
@ main application at a latter date and serve as stubs until then
Undef_Handler:
        BKPT
SVC_Handler:
        BKPT
Prefetch_Handler:
        BKPT
Abort_Handler:
        BKPT
IRQ_Handler:
        BKPT
FIQ_Handler:
        BKPT
Default_Handler:
        BKPT

@******************************************************************************
@
@******************************************************************************
@
@ The reset handler sets up the stack pointers for all the modes. The FIQ and
@ IRQ shall be disabled during this. Then clears the BSS section, enters the  
@ main function. 

Reset_Handler:
@
@ Set up the Vector Base Address Regsiter
@
        SUB   r0, r0, r0
        MCR   p15, 0, r0, c1, c0, 0
        LDR   r0, =Entry
	LDR   lr, =end                        @ init lr to avoid errmsg on gdb      
        MCR   p15, 0, r0, c12, c0, 0          @ Write VBAR Register
@
@ Set up the Stack for Undefined mode
@
        LDR   r0, =_stack_top                 @ Read the stack address
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
@ Set up the Stack for SVC (Supervisor) mode
@        
        @ MSR   cpsr_c, #MODE_SVC|I_F_BIT       @ change to SVC mode
        @ MOV   sp,r0                           @ write the stack pointer
        @ SUB   r0,r0, #SVC_STACK_SIZE          @ give stack space
@
@ Set up the Stack for System mode
@      
        MSR   cpsr_c, #MODE_SYS|I_F_BIT       @ change to system mode
        MOV   sp,r0                           @ write the stack pointer
@
@ Invalidate and Enable Branch Prediction  
@ Invalidate cache and Enable Branch Prediction 
@ Allow unaligned access, effective only when MMU is enabled
@ Enable both instruction and data caches and L2 cache
@
        MOV     r0, #0
        MCR     p15, #0, r0, c7, c5, #6
        ISB
        MRC     p15, #0, r0, c1, c0, #0
        ORR     r0, r0, #0x000004
        ORR     r0, r0, #0x001800
        ORR     r0, r0, #0x400000
        BIC     r0, r0, #0x000002
        MCR     p15, #0, r0, c1, c0, #0
        ISB
        MOV     r0, #2
        MRC     p15, #0, r1, c1, c0, #1
        ORR     r0,  r0, r1
        MCR     p15, #0, r0, c1, c0, #1
        DSB
        ISB

@
@ Enable Neon/VFP Co-Processor
@
        MRC     p15, #0, r1, c1, c0, #2           @ r1 = Access Control Register
        ORR     r1, r1, #(0xf << 20)              @ enable full access for p10,11
        MCR     p15, #0, r1, c1, c0, #2           @ Access Control Register = r1
        MOV     r1, #0
        MCR     p15, #0, r1, c7, c5, #4           @ flush prefetch buffer
        MOV     r0,#0x40000000
        FMXR    FPEXC, r0                        @ Set Neon/VFP Enable bit

@
@ Disable the watchdog
@
	LDR     r0, =0x44E35000	                  @@ load SOC_WDT_1_REGS
	LDR	r1, =0xAAAA
	STR	r1, [r0, #0x48]	                  @@ store 0xaaaa to WDT_WSPR
1:	LDR	r1, [r0, #0x34]	                  @@ loop until WDT_WWPS is 0
	CMP	r1, #0x0
	BNE	1b
	LDR	r1, =0x5555
	STR	r1, [r0, #0x48]	                  @@ store 0x5555 to WDT_WSPR
2:	LDR	r1, [r0, #0x34]	                  @@ loop until WDT_WWPS is 0
	CMP	r1, #0x0
	BNE	2b
@
@ Clear the BSS section here
@
        LDR   r4, =_bss_start                 @ Start address of BSS
        LDR   r9, =_bss_end                   @ End address of BSS
        MOV   r5, #0  
        MOV   r6, #0  
        MOV   r7, #0  
        MOV   r8, #0  
        B     2f
1:      STMIA r4!, {r4-r8}                    @ Clear four words in BSS
2:      CMP   r4, r9
        BLO   1b                              @ Clear till BSS end
@
@ Enter the main function. 
@
	// Initialize stdlib
	@ LDR	r0,=_init
	@ BLX	r0

	// Enter main program
	LDR	r0, =main
	BLX	r0
	
end:
1:	nop
	B	1b

@ End of the file
@
        .end
    
