/*
* Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/ 
*/
/* 
*  Redistribution and use in source and binary forms, with or without 
*  modification, are permitted provided that the following conditions 
*  are met:
*
*    Redistributions of source code must retain the above copyright 
*    notice, this list of conditions and the following disclaimer.
*
*    Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the 
*    documentation and/or other materials provided with the   
*    distribution.
*
*    Neither the name of Texas Instruments Incorporated nor the names of
*    its contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
*  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
*  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
*  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
*  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
*  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/

/* ld script for StarterWare AM335x. */

/*
** The stack is kept at end of the image, and its size is 128 MB.
** The heap section is placed above the stack to support I/O
** operations using semihosting. The size of the section is 2KB.
*/

ENTRY(Entry)

MEMORY
{
    OCRAM : ORIGIN = 0x402F0400, LENGTH = 0x1FC00
}

SECTIONS
{
    .text : {
         _image_start = .;
         *(.startup)
         /* KEEP(*bl_init.o(.text)) */
         *(.rodata*)
         *(.text*)
    } > OCRAM
 	/* non-zero global and static variables */
 	. = ALIGN(32);
 	.data : {
         *(.data*)
         _image_end = .;
    } > OCRAM

 	/* zeroed global and static variables */
 	. = ALIGN(32);
 	_bss_start = .;
 	.bss :  { *(.bss) } > OCRAM

 	/* Put the stack inside the BSS to simplify clearing */
 	. = ALIGN(32);
 	.stack :
 	{
 		_stack_end = .;  /* the danger zone! */
 		/* . = . + 0x2000;  4kB of stack memory */
        _stack_top = ORIGIN(OCRAM) + LENGTH(OCRAM) - 8;
 		/* _stack_top = .;  stack grows upwards */
 	} > OCRAM

 	. = ALIGN(32);
 	_bss_end = .;
 }