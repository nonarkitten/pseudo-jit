#include <stdint.h>     /* uimt32_t, etc */
#include <stdio.h>      /* printf, scanf */
#include <stdlib.h>     /* exit */
#include <setjmp.h>     /* jmp_buf, setjmp, longjmp */

//#include "uartStdio.h"

#ifndef __PJIT_H_
#define __PJIT_H_

/* Our CPU State */

/* Everything else we'll put into a RAM based struct with a
** single pointer to the base
*/
typedef struct {
    uint32_t d[8];          // Lt Data registers
    uint32_t a[8];          // Place to store address reg's
    uint32_t page;          // Base address of the current cache page
    union {
        struct {
            unsigned t : 1;      // Trace enable
            unsigned   : 1;
            unsigned s : 1;      // Supervisor mode
            unsigned   : 2;
            unsigned i : 3;      // Interrupt mask level
            unsigned   : 3;
            unsigned x : 1;      // eXtra carry flag
            unsigned n : 1;      // Negative flag
            unsigned z : 1;      // Zero flag
            unsigned v : 1;      // Overflow (sign flip) flag
            unsigned c : 1;      // Carry/Borrow flag
        };
        struct {
            unsigned       : 11;
            unsigned xnzvc : 5;
        };
        struct {
            unsigned       : 12;
            unsigned nzvc  : 4;
        };
        uint16_t srccr;     // All of the above
    };
    uint8_t map_xnzvc[16];
    uint8_t map_nzvc[16];
} cpu_t;

/* These registers are to be kept in ARM registers
** for maximum performance; if there's any changes
** made here, be sure to also fix up pjit.c as well
*/
register uint32_t __d0 asm("r4");
register uint32_t __d1 asm("r5");

register uint32_t __a0 asm("r6");
register uint32_t __a1 asm("r7");

register uint32_t __a4 asm("r8");
register uint32_t __a5 asm("r9");
register uint32_t __a6 asm("r10");
register uint32_t __a7 asm("r11");

register cpu_t *cpu asm("r12");

register uint32_t lr asm("lr");

#define ASM __asm__ __volatile__
#define ASM_GOTO __asm__ __volatile__ goto

/* Takes the current instruction pointer within the cache page
** and determines the original PC from that by merging back with
** the base pointer (which is the 68K start of this cache page)
*/
#define UPDATE_PC(TEMP) \
    TEMP = (((lr - 4) / 2) & (CACHE_PAGE_SIZE-1)) | cpu->page

/* Sets the ZN flags based on byte (8), word (16) or long
** word (32) value; use on any move, tst or similar
*/
#define TST_8(X) \
    ASM("rors\t%0, %1, %2" : "=r"(X) : "r"(X), "n"(8));
#define TST_16(X) \
    ASM("rors\t%0, %1, %2" : "=r"(X) : "r"(X), "n"(16));
#define TST_32(X) \
    ASM("tst\t%0, %1" :: "r"(X), "n"(0));

/* Run bogomips loop without having loaded the whole pjit table
** set loops to the number of empty loops to execute
*/
void test_pjit_bogomips(void);

/* Initialize pjit */
void pjit_init(void);

#pragma no_long_calls

#endif // __PJIT_H_


