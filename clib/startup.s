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

@**************************** Code Seection *********sho**************************
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
        B       .
SVC_Handler:        
        B       .
Prefetch_Handler:
        B       .
Abort_Handler:
        B       .
IRQ_Handler:
        B       .
FIQ_Handler:
        B       .
Default_Handler:
        B       .

@******************************************************************************
@
@******************************************************************************
@
@ The reset handler sets up the stack pointers for all the modes. The FIQ and
@ IRQ shall be disabled during this. Then clears the BSS section, enters the  
@ main function. 

Reset_Handler:
	// Enter main program
        B       main	
end:
1:	nop
	B	1b

@ End of the file
@
        .end
    
