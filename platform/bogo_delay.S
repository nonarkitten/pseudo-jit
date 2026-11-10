@ bogo_delay.S — the native-ARM counterpart to the 68K delay primitive
@ (SUBQ.L #1,Dn ; BNE.S) that tests/qemu/smoke_bogomips.c uses --
@ same shape, one native decrement-and-branch pair per iteration, no
@ 68K/JIT involved at all. Exists purely so
@ tests/qemu/smoke_native_bogomips.c can measure "how many of *these* fit in
@ a real second" as a baseline to compare the JIT'd 68K BogoMIPS figure
@ against -- see that file's own header comment for why.
@
@ r0 = loop count (in), destroyed.

        .global bogo_delay

        .text
        .arm

bogo_delay:
        subs    r0, r0, #1
        bne     bogo_delay
        bx      lr
