/// @file buffee_reroll.c
/// @brief Which Buffee-side piece holds the per-boot blitter-wedge state?  Rounds of 3000 random D-fill blits (as buffee_blit.c); after each round the bootloader re-initialises one piece
///        (tools: the bootloader loop in bootloader/src/main.c, exit codes BUFFEE_REROLL_EXIT(k): bit 0 PRU restart, bit 1 GreenPAK image reload, bit 2 InitGPMC) and calls us again.  Blocks of ROUNDS rounds per action:
///        0 = nothing but the recalibration (control), 1 = PRU, 2 = GreenPAK, 4 = GPMC, 7 = all three.  A piece that holds the state flips good <-> bad between rounds of its block; a piece that does not stays put.
///        One line per round: "REROLL round R action A: good G wrong W hung H".
#include <stdint.h>
#include "pjit.h"
#include "bbb.h"
#include "buffee.h"
#include "mem_be.h"

#define CW(off, v) write_BE_word(0xDFF000 + (off), (v))
#ifndef ROUNDS
#define ROUNDS 8
#endif
#ifndef BLITS
#define BLITS 3000
#endif
static const uint8_t actions[] = { 0, 1, 2, 4, 7 };
#define NACT ((uint32_t)(sizeof actions / sizeof actions[0]))
#define COUNT ((volatile uint32_t *)0x9F5FFFD0u)
int main(void)
{
    uint32_t round = (*BUFFEE_MAILBOX == 1) ? *COUNT + 1 : 0;
    *COUNT = round;
    uint32_t act = actions[(round / ROUNDS) % NACT];                       /* the action applied BEFORE this round (it follows the previous round's request) */
    uint32_t next = actions[((round + 1) / ROUNDS) % NACT];
    if (round == 0) printf("=== re-roll test: %u rounds of %u blits per action ===\n", (unsigned)ROUNDS, (unsigned)BLITS);
    *(volatile uint8_t *)0xBFE201 |= 3; *(volatile uint8_t *)0xBFE001 &= 0xFC;                  /* ROM overlay off: low memory is chip RAM */
    CW(0x96, 0x7FFF); CW(0x9A, 0x7FFF); CW(0x96, 0x8000 | 0x0200 | 0x0040);                     /* DMAEN | BLTEN */
    uint32_t seed = 12345 + round, good = 0, bad = 0, hung = 0;
    for (uint32_t n = 0; n < BLITS; n++) {
        seed = seed * 1664525u + 1013904223u; uint32_t H = 1 + (seed >> 16) % 8, W = 1 + (seed >> 24) % 8;
        for (int i = 0; i < 70; i++) write_BE_word(0x60000 + 2 * i, 0);
        CW(0x40, 0x01FF); CW(0x42, 0); CW(0x44, 0xFFFF); CW(0x46, 0xFFFF); CW(0x66, 0); CW(0x54, 0x6); CW(0x56, 0);
        CW(0x58, (uint16_t)(H << 6 | W));
        uint32_t w = 0; while ((read_BE_word(0xDFF002) & 0x4000) && w < 20000) w++;
        if (w >= 20000) { hung++; continue; }
        uint32_t cnt = 0; for (int i = 0; i < 70; i++) if (read_BE_word(0x60000 + 2 * i) == 0xFFFF) cnt++;
        if (cnt == H * W) good++; else bad++;
    }
    printf("REROLL round %u action %u: good %u wrong %u hung %u  (%s)\n", (unsigned)round, (unsigned)act, (unsigned)good, (unsigned)bad, (unsigned)hung, hung || bad ? "BAD" : "ok");
    CW(0x96, 0x7FFF);
    *(volatile uint8_t *)0xBFE001 |= 1;
    if (round + 1 < ROUNDS * NACT) return BUFFEE_REROLL_EXIT(next);
    *BUFFEE_MAILBOX = 0;
    printf("REROLL done\n");
    return 0;
}
