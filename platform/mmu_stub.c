/*
 * mmu_stub.c -- platform helpers: the fatal-fault reporter boot.S's vector stubs call.
 */
#include <stdint.h>
#include "pjit_libc.h"





/// @brief Reports an ARM-level fault and halts.
/// @details Called from boot.S's vector stubs, on their own stack. Turns
///          so that a fault shows as a message and not as a silent hang.
void pjit_fault_report(uint32_t kind, uint32_t lr, uint32_t spsr, uint32_t dfar)
{
    static const char *name[] = { "?", "UNDEFINED INSTRUCTION",
                                  "PREFETCH ABORT", "DATA ABORT", "IRQ", "FIQ" };
    printf("\n[FAULT] %s  lr=%08X spsr=%08X (mode=%02X %s %s) dfar=%08X\n",
           (kind < 6) ? name[kind] : "?", lr, spsr, spsr & 0x1F,
           (spsr & 0x20) ? "Thumb" : "ARM", (spsr & 0x200) ? "BE" : "LE", dfar);
}
