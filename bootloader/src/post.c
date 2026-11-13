/*
 * post.c -- the power-on self-test of the board's own memories. Every boot checks the DDR's data and address lines and the SPI flash's ID, which costs well under
 * a millisecond; a first boot (or one with post_full set) also writes and reads back every word of the DDR. What the boot does with the result is in main.c.
 *
 * Copyright (c) 2021-2026 the Buffee Project - http://www.buffee.ca
 * Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (see LICENSE).
 */
#include "main.h"
#include "post.h"

#define DDR_BASE 0x80000000u
#define DDR_SIZE 0x20000000u            /* 512 MB */
#define DDR_STEP 0x01000000u            /* the full test looks at ESC and prints progress every 16 MB */

int DDRLineTest(void) {
    volatile uint32_t* m = (volatile uint32_t*)DDR_BASE;
    uint32_t off;
    int bad = 0, b;

    for (b = 0; b < 32; b++) {                                      /* data lines: walking one; the complement in the next word keeps the bus from holding the value */
        m[0] = 1u << b; m[1] = ~(1u << b);
        if (m[0] != 1u << b || m[1] != ~(1u << b)) bad++;
    }
    m[0] = 0xAAAA5555u;                                                 /* address lines: a distinct value at every power-of-two offset; a stuck or shorted line aliases two */
    for (off = 4; off < DDR_SIZE; off <<= 1) m[off / 4] = off;
    if (m[0] != 0xAAAA5555u) bad++;
    for (off = 4; off < DDR_SIZE; off <<= 1) if (m[off / 4] != off) bad++;
    if (bad) printf("DDR: %d data or address line errors\n", bad);
    else info("DDR: data and address lines ok\n");
    return bad ? POST_FAILED : POST_OK;
}

/* Each word holds its own address (inverted in the second pass), so a fault in any address or data bit shows. */
__attribute__((noinline)) static void fill(uint32_t* p, uint32_t bytes, uint32_t x) {
    uint32_t* end = p + bytes / 4;
    uint32_t a;

    for (; p < end; p += 4) {
        a = (uint32_t)p ^ x;
        p[0] = a; p[1] = a ^ 4; p[2] = a ^ 8; p[3] = a ^ 12;
    }
}
__attribute__((noinline)) static uint32_t* check(uint32_t* p, uint32_t bytes, uint32_t x) {
    uint32_t* end = p + bytes / 4;

    for (; p < end; p++) if (*p != ((uint32_t)p ^ x)) return p;
    return 0;
}

static int stopped(void) { UART0Ready(); return uart_esc_seen || uart_break; }

int DDRFullTest(void) {
    double t0 = ReadDMTimerSeconds();
    uint32_t a, x, *bad;
    int pass;

    info("DDR: testing all of the 512 MB ");
    for (pass = 0; pass < 2; pass++) {
        x = pass ? 0xFFFFFFFFu : 0;
        for (a = DDR_BASE; a < DDR_BASE + DDR_SIZE; a += DDR_STEP) {
            fill((uint32_t*)a, DDR_STEP, x);
            if (stopped()) { printf("\nDDR test stopped\n"); return POST_STOPPED; }
        }
        for (a = DDR_BASE; a < DDR_BASE + DDR_SIZE; a += DDR_STEP) {
            bad = check((uint32_t*)a, DDR_STEP, x);
            if (bad) { printf("\nDDR: %08lX reads %08lX, expected %08lX\n", (unsigned long)bad, (unsigned long)*bad, (unsigned long)((uint32_t)bad ^ x)); return POST_FAILED; }
            if (((a + DDR_STEP) & 0x3FFFFFFu) == 0) info(".");
            if (stopped()) { printf("\nDDR test stopped\n"); return POST_STOPPED; }
        }
    }
    info(" ok in %0.1f s\n", ReadDMTimerSeconds() - t0);
    return POST_OK;
}

int FlashIDTest(void) {
    uint16_t id = ReadSPIID();

    if (id == 0x0000 || id == 0xFFFF) { printf("SPI flash: no answer (ID %04X)\n", id); return POST_FAILED; }
    info("SPI flash ID %04X\n", id);
    return POST_OK;
}
