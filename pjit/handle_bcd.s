@
@ Copyright (c) 2020-2023 Renee Cousins, the Buffee Project - http://www.buffee.ca
@
@ Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (see LICENSE).
@
@ handle_bcd.s -- ABCD, SBCD and NBCD: BCD add, subtract and negate with extend.
@
@ Each computes the raw sum or difference plus the incoming X, finds both the binary carry and the decimal carry (a decimal-only overflow such as 0x09+0x01 trips no
@ binary carry, so it is found by probing sum + 0x66) and corrects the result by 0x00, 0x06, 0x60 or 0x66. tests/qemu/smoke_bcd_exec.c checks all 100 x 100 digit pairs
@ with both X states.
@
@ They are reached by a plain B, not BL, and end in `bx lr`, returning to whatever called the opcode's slot (branch without link all the way down). r0-r4 are scratch (r4
@ is saved in cpu->r0 and restored, because the test calls them as ordinary AAPCS functions); r5, r6-r13 and lr are left alone.
@
@ abcd() / sbcd(): r0 = the source byte (zero-extended by the caller), r1 = a pointer to the destination byte (a memory address for -(An), the low byte of a D register for
@ Dn,Dn). nbcd(): r0 = that destination pointer.
@
@ Flags: X is read from the GE flags (GE0 set means X clear) and written back there; N, Z, V and C are the live ARM flags, with Z only ever cleared. The result is also
@ stored in the CCR byte of cpu->sr.
@

        .include "asm_offsets.inc"
        .syntax unified
        .arm
        .text

        .global abcd
        .type abcd, %function
abcd:
        @ r4 needs saving here specifically because these functions are
        @ callable two different ways: tail-branched into from JIT'd
        @ code (where r4 has no meaning to preserve -- every opcode's
        @ emitted sequence is self-contained) and, for
        @ tests/qemu/smoke_bcd_exec.c's exhaustive check, called directly as
        @ an ordinary AAPCS function, where r4 *is* callee-saved and a
        @ caller is entitled to assume it survives. cpu->r0 (the T0
        @ field, not the ARM register r0) is free scratch at this point
        @ for exactly this purpose.
        str     r4, [r5, CPU_R0]            
        uxtb    r0, r0                      @ S, defensively
        ldrb    r2, [r1]                    @ D = *dst
        mrs     r3, cpsr                    @ X is kept in the GE flags: GE0 is set when X is CLEAR (tools/gen/gen.py save_x)
        ubfx    r3, r3, #16, #1
        eor     r3, r3, #1                  @ r3 = X
        add     r4, r0, r2                  
        add     r4, r4, r3                  @ r4 = ss = S+D+X
        @ bc (binary carry into bit3/bit7) = ((S&D)|(~ss&(S|D))) & 0x88
        and     r3, r0, r2                  @ r3 = S&D
        orr     r0, r0, r2                  @ r0 = S|D (S,D no longer needed individually)
        bic     r0, r0, r4                  @ r0 = (S|D)&~ss
        orr     r3, r3, r0                  
        and     r3, r3, #0x88               @ r3 = bc
        @ dc (decimal carry) = (((ss+0x66)^ss)&0x110)>>1 -- catches a
        @ decimal-only overflow (e.g. 0x09+0x01=0x0A) that bc alone
        @ misses, since no *binary* carry happens in that nibble.
        add     r0, r4, #0x66               
        eor     r0, r0, r4                  
        and     r0, r0, #0x110              
        lsr     r0, r0, #1                  @ r0 = dc
        orr     r2, r3, r0                  @ r2 = bc|dc (bc kept in r3 for the C flag below)
        sub     r0, r2, r2, lsr #2          @ r0 = corf = (bc|dc) - ((bc|dc)>>2)
        add     r2, r4, r0                  @ r2 = rr = ss + corf
        @ C-source = bc | (ss & ~rr); V-source = ~ss & rr = rr & ~ss
        bic     r0, r4, r2                  
        orr     r0, r0, r3                  @ r0 = C-source
        bic     r3, r2, r4                  @ r3 = V-source
        uxtb    r2, r2                      @ final byte result
        strb    r2, [r1]                    
        lsr     r0, r0, #7                  
        and     r0, r0, #1                  @ r0 = C
        lsr     r3, r3, #7                  
        and     r3, r3, #1                  @ r3 = V
        mrs     r4, cpsr                    @ live flags are authoritative (cpu->sr only
        lsr     r4, r4, #28                 @ holds X): N Z C V -> b3..b0, so Z is bit 2
        lsr     r1, r4, #2                  
        and     r1, r1, #1                  @ r1 = old Z
        cmp     r2, #0                      
        moveq   r4, #1                      
        movne   r4, #0                      @ r4 = (result == 0)
        and     r1, r1, r4                  @ r1 = new (sticky) Z
        lsr     r4, r2, #7                  @ r4 = N
        mov     r2, r0                      @ CCR: C at bit0
        orr     r2, r2, r0, lsl #4          @ X (= C) at bit4
        orr     r2, r2, r3, lsl #1          @ V at bit1
        orr     r2, r2, r1, lsl #2          @ Z at bit2
        orr     r2, r2, r4, lsl #3          @ N at bit3
        strb    r2, [r5, CPU_SR+1]          
        @ Result flags into the live ARM CPSR too (X stays in cpu->sr):
        @ the next 68K instruction, e.g. a Bcc, reads the live flags.
        and     r3, r2, #8                  
        lsl     r3, r3, #28                 @ N -> bit 31
        and     r0, r2, #4                  
        orr     r3, r3, r0, lsl #28         @ Z -> bit 30
        and     r0, r2, #1                  
        orr     r3, r3, r0, lsl #29         @ C -> bit 29
        and     r0, r2, #2                  
        orr     r3, r3, r0, lsl #27         @ V -> bit 28
        msr     cpsr_f, r3                  
        and     r0, r2, #0x10               @ X (= C) into the GE flags: GE[3:0] = 1111 when it is clear
        eor     r0, r0, #0x10
        lsl     r0, r0, #12
        rsb     r0, r0, r0, lsl #4
        msr     apsr_g, r0
        ldr     r4, [r5, CPU_R0]            
        bx      lr

        .global sbcd
        .type sbcd, %function
sbcd:
        str     r4, [r5, CPU_R0]            @ see abcd()'s comment
        uxtb    r0, r0                      @ S
        ldrb    r2, [r1]                    @ D = *dst
        mrs     r3, cpsr                    @ X is kept in the GE flags: GE0 is set when X is CLEAR (tools/gen/gen.py save_x)
        ubfx    r3, r3, #16, #1
        eor     r3, r3, #1                  @ r3 = X
        sub     r4, r2, r0                  
        sub     r4, r4, r3                  @ r4 = dd = D-S-X
        @ bc = ((S|dd)&~D) | (dd&S), &0x88 -- no separate decimal-carry
        @ probe needed here: unlike addition, the borrow chain already
        @ reflects a decimal-only underflow.
        orr     r3, r0, r4                  @ r3 = S|dd
        bic     r3, r3, r2                  @ r3 = (S|dd)&~D
        and     r0, r4, r0                  @ r0 = dd&S (S,D no longer needed individually)
        orr     r3, r3, r0                  
        and     r3, r3, #0x88               @ r3 = bc
        sub     r0, r3, r3, lsr #2          @ r0 = corf
        sub     r2, r4, r0                  @ r2 = rr = dd - corf
        @ C-source = bc | (rr & ~dd); V-source = dd & ~rr
        bic     r0, r2, r4                  
        orr     r0, r0, r3                  @ r0 = C-source
        bic     r3, r4, r2                  @ r3 = V-source
        uxtb    r2, r2                      
        strb    r2, [r1]                    
        lsr     r0, r0, #7                  
        and     r0, r0, #1                  @ C
        lsr     r3, r3, #7                  
        and     r3, r3, #1                  @ V
        mrs     r4, cpsr                    @ live flags are authoritative (cpu->sr only
        lsr     r4, r4, #28                 @ holds X): N Z C V -> b3..b0, so Z is bit 2
        lsr     r1, r4, #2                  
        and     r1, r1, #1                  @ old Z
        cmp     r2, #0                      
        moveq   r4, #1                      
        movne   r4, #0                      
        and     r1, r1, r4                  @ new Z
        lsr     r4, r2, #7                  @ N
        mov     r2, r0                      
        orr     r2, r2, r0, lsl #4          
        orr     r2, r2, r3, lsl #1          
        orr     r2, r2, r1, lsl #2          
        orr     r2, r2, r4, lsl #3          
        strb    r2, [r5, CPU_SR+1]          
        @ Result flags into the live ARM CPSR too (X stays in cpu->sr):
        @ the next 68K instruction, e.g. a Bcc, reads the live flags.
        and     r3, r2, #8                  
        lsl     r3, r3, #28                 @ N -> bit 31
        and     r0, r2, #4                  
        orr     r3, r3, r0, lsl #28         @ Z -> bit 30
        and     r0, r2, #1                  
        orr     r3, r3, r0, lsl #29         @ C -> bit 29
        and     r0, r2, #2                  
        orr     r3, r3, r0, lsl #27         @ V -> bit 28
        msr     cpsr_f, r3                  
        and     r0, r2, #0x10               @ X (= C) into the GE flags: GE[3:0] = 1111 when it is clear
        eor     r0, r0, #0x10
        lsl     r0, r0, #12
        rsb     r0, r0, r0, lsl #4
        msr     apsr_g, r0
        ldr     r4, [r5, CPU_R0]            
        bx      lr

        .global nbcd
        .type nbcd, %function
nbcd:
        str     r4, [r5, CPU_R0]            @ see abcd()'s comment
        ldrb    r2, [r0]                    @ D = *dst
        mrs     r3, cpsr                    @ X is kept in the GE flags: GE0 is set when X is CLEAR (tools/gen/gen.py save_x)
        ubfx    r3, r3, #16, #1
        eor     r3, r3, #1                  @ r3 = X
        rsb     r4, r2, #0                  @ r4 = -D
        sub     r4, r4, r3                  @ r4 = dd = -D-X
        @ bc = (D|dd) & 0x88 -- sbcd's formula with the implicit
        @ source (D-S-X's "S") substituted with 0: NBCD is 0-D-X.
        orr     r3, r2, r4                  @ r3 = D|dd (D no longer needed)
        and     r3, r3, #0x88               @ r3 = bc
        sub     r1, r3, r3, lsr #2          @ r1 = corf
        sub     r2, r4, r1                  @ r2 = rr = dd - corf
        bic     r1, r2, r4                  
        orr     r1, r1, r3                  @ r1 = C-source
        bic     r3, r4, r2                  @ r3 = V-source
        uxtb    r2, r2                      
        strb    r2, [r0]                    
        lsr     r1, r1, #7                  
        and     r1, r1, #1                  @ C
        lsr     r3, r3, #7                  
        and     r3, r3, #1                  @ V
        mrs     r4, cpsr                    @ live flags are authoritative (cpu->sr only
        lsr     r4, r4, #28                 @ holds X): N Z C V -> b3..b0, so Z is bit 2
        lsr     r0, r4, #2                  @ (r0/dst ptr no longer needed post-store)
        and     r0, r0, #1                  @ old Z
        cmp     r2, #0                      
        moveq   r4, #1                      
        movne   r4, #0                      
        and     r0, r0, r4                  @ new Z
        lsr     r4, r2, #7                  @ N
        mov     r2, r1                      @ CCR: C at bit0
        orr     r2, r2, r1, lsl #4          @ X
        orr     r2, r2, r3, lsl #1          @ V
        orr     r2, r2, r0, lsl #2          @ Z
        orr     r2, r2, r4, lsl #3          @ N
        strb    r2, [r5, CPU_SR+1]          
        @ Result flags into the live ARM CPSR too (X stays in cpu->sr):
        @ the next 68K instruction, e.g. a Bcc, reads the live flags.
        and     r3, r2, #8                  
        lsl     r3, r3, #28                 @ N -> bit 31
        and     r0, r2, #4                  
        orr     r3, r3, r0, lsl #28         @ Z -> bit 30
        and     r0, r2, #1                  
        orr     r3, r3, r0, lsl #29         @ C -> bit 29
        and     r0, r2, #2                  
        orr     r3, r3, r0, lsl #27         @ V -> bit 28
        msr     cpsr_f, r3                  
        and     r0, r2, #0x10               @ X (= C) into the GE flags: GE[3:0] = 1111 when it is clear
        eor     r0, r0, #0x10
        lsl     r0, r0, #12
        rsb     r0, r0, r0, lsl #4
        msr     apsr_g, r0
        ldr     r4, [r5, CPU_R0]            
        bx      lr

