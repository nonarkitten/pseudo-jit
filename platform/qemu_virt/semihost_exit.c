/*
 * semihost_exit.c — stop QEMU cleanly via the ARM semihosting SYS_EXIT call.
 *
 * Pass/fail for our tests is decided by run.sh grepping the UART transcript
 * for PASS/FAIL markers (see tests/run.sh), not by this process's exit
 * status -- semihosting exit-code propagation varies by QEMU version and
 * isn't worth depending on. This just needs to stop the emulator instead of
 * spinning forever so run.sh doesn't need a timeout/kill fallback.
 */
void qemu_exit(void)
{
    register unsigned r0 asm("r0") = 0x18;              /* SYS_EXIT */
    register unsigned r1 asm("r1") = 0x20026;            /* ADP_Stopped_ApplicationExit */
    __asm__ volatile ("svc 0x00123456" :: "r"(r0), "r"(r1));
    for (;;) { }   /* unreachable when run under qemu -semihosting */
}

/*
 * semihost_write_file -- write a buffer to a file on the *host* via ARM
 * semihosting (SYS_OPEN / SYS_WRITE / SYS_CLOSE). The tests that record instruction
 * traces use it to get them out of QEMU. Returns 0 on success.
 */
static unsigned semihost_call(unsigned op, const void *args)
{
    register unsigned r0 asm("r0") = op;
    register const void *r1 asm("r1") = args;
    __asm__ volatile ("svc 0x00123456" : "+r"(r0) : "r"(r1) : "memory");
    return r0;
}

int semihost_write_file(const char *path, const void *buf, unsigned len)
{
    unsigned n = 0;
    while (path[n]) n++;

    unsigned open_args[3] = { (unsigned)path, 5 /* "wb" */, n };
    int fd = (int)semihost_call(0x01 /* SYS_OPEN */, open_args);
    if (fd == -1) return -1;

    unsigned write_args[3] = { (unsigned)fd, (unsigned)buf, len };
    unsigned not_written = semihost_call(0x05 /* SYS_WRITE */, write_args);

    unsigned close_args[1] = { (unsigned)fd };
    semihost_call(0x02 /* SYS_CLOSE */, close_args);
    return not_written ? -1 : 0;
}

/// Default hook for the smoke tests that end with TRAP #15 and then look at D0
/// (tests/include/pjit_compat.h calls it after pjit_run() returns). A test that cares
/// defines its own; this one does nothing.
__attribute__((weak)) void pjit_test_exit(unsigned d0) { (void)d0; }
