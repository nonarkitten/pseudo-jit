@ irq.S -- the interrupt source for the QEMU virt board: a GICv2 and the ARM generic
@ timer's physical-timer PPI (INTID 30), which stands in for a 68K level-6 device.
@
@ pjit_platform_irq is the platform hook pjit_irq_entry (pjit/runtime.s) calls from IRQ
@ mode: it acknowledges the interrupt, silences the device and returns the 68K level it
@ posts in r0 (0 = nothing / spurious). It may clobber r0-r3 and nothing else.
@ qemu_virt_timer_arm(ticks) is the harness's way to raise one, `ticks` counter ticks
@ from now.

        .equ    GICD, 0x08000000
        .equ    GICC, 0x08010000

        .text
        .arm

        .global qemu_virt_timer_arm
qemu_virt_timer_arm:
        ldr     r1, =GICD
        mov     r2, #1
        str     r2, [r1]                @ GICD_CTLR: forward interrupts
        mov     r2, #(1 << 30)
        str     r2, [r1, #0x100]        @ GICD_ISENABLER0: PPI 30
        ldr     r1, =GICC
        mov     r2, #0xFF
        str     r2, [r1, #4]            @ GICC_PMR: every priority
        mov     r2, #1
        str     r2, [r1]                @ GICC_CTLR
        mcr     p15, 0, r0, c14, c2, 0  @ CNTP_TVAL
        mov     r2, #1
        mcr     p15, 0, r2, c14, c2, 1  @ CNTP_CTL: enabled, unmasked
        bx      lr

        .global pjit_platform_irq
pjit_platform_irq:
        ldr     r1, =GICC
        ldr     r2, [r1, #0xC]          @ GICC_IAR
        mov     r0, #0
        ubfx    r3, r2, #0, #10
        cmp     r3, #30
        bne     1f
        mov     r0, #2
        mcr     p15, 0, r0, c14, c2, 1  @ CNTP_CTL: masked (the device's line drops)
        mov     r0, #6
1:      str     r2, [r1, #0x10]         @ GICC_EOIR
        bx      lr
