/// The realview board's interrupt controller is not wired up: no 68K interrupts on this platform (pjit_irq_enable stays off).
unsigned pjit_platform_irq(void) { return 0; }
