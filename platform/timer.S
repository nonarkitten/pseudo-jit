@ timer.S — ARM Generic Timer access (CP15 CNTFRQ/CNTPCT), for the
@ bogomips benchmark's timing (tests/qemu/smoke_bogomips.c). This is a
@ mandatory CPU feature on cortex-a15 (the Generic Timer extension,
@ ARMv7-A Virtualization Extensions) -- not a memory-mapped peripheral
@ needing a base address or QEMU device setup, so there's nothing to
@ configure at boot: QEMU pre-programs CNTFRQ before our code ever
@ runs, and both registers are just... there.
@
@ Both are only restricted from *unprivileged* (PL0/User-mode) access
@ unless CNTKCTL explicitly grants it. PJIT's JIT'd 68K code runs in
@ ARM System mode (see platform/qemu_virt/boot.S / pjit_svc_entry's own
@ mode-switch), which is privileged (PL1) like every other mode this
@ tree ever runs in -- so no CNTKCTL configuration is needed either.

        .global read_cntfrq
        .global read_cntpct_lo

        .text
        .arm

@ Returns CNTFRQ (the counter's tick rate in Hz) in r0.
read_cntfrq:
        mrc     p15, 0, r0, c14, c0, 0
        bx      lr

@ Returns the low 32 bits of CNTPCT (the free-running physical count)
@ in r0. Only the low 32 bits: this is used for short (single-digit
@ seconds) relative-duration measurements via plain 32-bit subtraction,
@ never as an absolute timestamp, so the high word is never needed --
@ see tests/qemu/smoke_bogomips.c's own comment on why a wraparound
@ mid-measurement isn't a real concern here.
read_cntpct_lo:
        mrrc    p15, 0, r0, r1, c14
        bx      lr
