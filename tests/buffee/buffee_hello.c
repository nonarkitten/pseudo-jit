/// @file buffee_hello.c
/// @brief First PJIT program on Buffee (loaded by the bootloader's `L` command, make bundle-<test>). Three guest programs in the DRAM at 0x08000000:
///        1. a loop (DBF / ADD) that must give a known sum: the translator and the slot cache work on this board;
///        2. reads of Agnus (VPOSR), the Kickstart ROM and chip RAM through the Amiga window, compared with the same reads made by C;
///        3. a vertical-blank interrupt (level 3): the guest installs an autovector handler in its own vector table, enables VERTB and counts frames for a second.
#include <stdint.h>
#include "pjit.h"
#include "bbb.h"
#include "buffee.h"
#include "mem_be.h"

extern void emit_opcode_table(void);
#define GUEST_CODE   0x08001000u
#define GUEST_VEC    0x08010000u                 /* the guest's vector table (VBR) */
#define GUEST_STACK  0x08100000u
#define GUEST_DATA   0x08020000u                 /* results the guest leaves (big-endian) */

static uint32_t hw;                              /* halfword index into the guest code */
static void w(uint16_t v) { write_BE_word(GUEST_CODE + hw * 2, v); hw++; }
static void wl(uint32_t v) { w(v >> 16); w(v & 0xFFFF); }
static void at(uint32_t off) { hw = off / 2; }

static uint32_t pjit_last_exit;
static uint32_t run(uint32_t entry)
{
    for (int i = 0; i < 8; i++) { pjit_set_dn(i, 0); pjit_set_an(i, 0); }
    pjit_set_an(7, GUEST_STACK);
    cpu->ssp = cpu->usp = __builtin_bswap32(GUEST_STACK);
    pjit_set_sr(0x2700);
    cpu->irq_level = 0; cpu->irq_flag = 0;
    pjit_cache_flush();
    uint32_t t0 = bbb_pmu_cycles();
    pjit_last_exit = pjit_run((uint16_t *)entry);
    return bbb_pmu_cycles() - t0;
}

static void cia_probe(const char *when)
{
    /* 17 ( which CIAA registers take a write: PRA ($BFE001, bits 1:0 outputs), DDRA ($BFE201), CRA ($BFEE01), CRB ($BFEF01), ICR mask ($BFED01), then PRA again */
    {
        static const struct { uint32_t a; uint8_t m; const char *n; } r[] = { { 0xBFE001, 3, "PRA" }, { 0xBFE201, 3, "DDRA" }, { 0xBFEE01, 6, "CRA" }, { 0xBFEF01, 6, "CRB" }, { 0xBFED01, 0x1C, "ICRmask" }, { 0xBFE001, 3, "PRA again" }, { 0xBFD001, 0, "(CIAB PRA read only)" } };
        printf("[17 %s]", when);
        for (unsigned k = 0; k < sizeof r / sizeof r[0]; k++) {
            volatile uint8_t *p = (volatile uint8_t *)r[k].a; uint8_t save = *p; uint32_t e = 0, ok = 0;
            if (r[k].m) for (int i = 0; i < 400; i++) { uint8_t v = (uint8_t)(i * 37u + 11u) & r[k].m; if (k == 4) { *p = 0x80 | 0; } *p = (save & ~r[k].m) | v; if ((*p & r[k].m) != v) e++; else ok++; }
            *p = save;
            printf(" %s %u/400 wrong;", r[k].n, (unsigned)e);
        }
        printf("\n");
    }

}

int main(void)
{
    int bad = 0;
    printf("=== PJIT on Buffee ===\n");
    if (*BUFFEE_MAILBOX == 1) { printf("restarted after the Amiga reset (events %u): PASS\n", (unsigned)buffee_reset_events); *BUFFEE_MAILBOX = 0; return 0; }
    pjit_cache_init(BUFFEE_CACHE_TOP);
    cpu->trap_exit_mask = 0x8000;
    emit_opcode_table();
    cpu->dr_threshold = DR_THRESHOLD_DEFAULT;
    pjit_irq_enable(1);
    printf("tables built; running from the guest DRAM at 0x%08X\n", GUEST_CODE);

    { printf("[0] floating bus: $E80000:"); for (uint32_t a = 0xE80000; a < 0xE80010; a += 2) printf(" %04X", read_BE_word(a));
      *(volatile uint8_t *)0xBFE201 |= 3; *(volatile uint8_t *)0xBFE001 &= 0xFC;
      volatile uint16_t *q = (volatile uint16_t *)0x90000; for (int i = 0; i < 4; i++) q[i] = 0x1000 + i; printf("; $90000 after writing 1000..1003:"); for (int i = 0; i < 4; i++) printf(" %04X", q[i]);
      *(volatile uint8_t *)0xBFE001 |= 1; printf("\n"); }
    cia_probe("early");
    /* 1: D1 = sum of 1..1000 (500500), D2 = 12345 * 3 */
    at(0);
    w(0x7000); w(0x7200); w(0x243C); wl(0);          /* MOVEQ #0,D0 ; MOVEQ #0,D1 ; MOVE.L #0,D2 */
    w(0x303C); w(999);                               /* MOVE.W #999,D0 */
    w(0x5281); w(0xD280);                            /* L: ADDQ.L #1,D1 ; ADD.L D0,D1 -- D1 += 1 + D0 (D0 counts 999..0): sum = 1000 + 999*1000/2 = 500500 */
    w(0x51C8); w(0xFFFA);                            /* DBF D0,L */
    w(0x243C); wl(12345); w(0x2602); w(0xD682); w(0xD682);   /* MOVE.L #12345,D2 ; MOVE.L D2,D3 ; ADD.L D2,D3 ; ADD.L D2,D3 */
    w(0x4E4F);                                       /* TRAP #15 */
    uint32_t c = run(GUEST_CODE);
    uint32_t d1 = pjit_get_dn(1), d3 = pjit_get_dn(3);
    printf("[1] loop: D1 = %u (expect 500500), D3 = %u (expect 37035), %u cycles\n", d1, d3, c);
    bad += (d1 != 500500) || (d3 != 37035);

    /* 2: the Amiga window from guest code */
    cpu->ssp = cpu->usp = __builtin_bswap32(GUEST_STACK);
    uint16_t vpos_c = read_BE_word(0xDFF004), rom_c = read_BE_word(0xF80000), rom2_c = read_BE_word(0xF80002), chip_c = read_BE_word(0x20000);
    at(0);
    w(0x3239); wl(0xDFF004);                         /* MOVE.W $DFF004,D1 */
    w(0x3439); wl(0xF80000);                         /* MOVE.W $F80000,D2 */
    w(0x3639); wl(0xF80002);                         /* MOVE.W $F80002,D3 */
    w(0x3839); wl(0x20000);                          /* MOVE.W $20000,D4 */
    w(0x2A39); wl(0xF80000);                         /* MOVE.L $F80000,D5 */
    w(0x4E4F);
    c = run(GUEST_CODE);
    printf("[2] window: VPOSR %04X (C %04X), ROM %04X %04X (C %04X %04X), chip $20000 %04X (C %04X), MOVE.L ROM %08X, %u cycles\n",
           pjit_get_dn(1) & 0xFFFF, vpos_c, pjit_get_dn(2) & 0xFFFF, pjit_get_dn(3) & 0xFFFF, rom_c, rom2_c, pjit_get_dn(4) & 0xFFFF, chip_c, pjit_get_dn(5), c);
    bad += ((pjit_get_dn(2) & 0xFFFF) != rom_c) || ((pjit_get_dn(3) & 0xFFFF) != rom2_c) || (pjit_get_dn(5) != (((uint32_t)rom_c << 16) | rom2_c));

    /* 3: VERTB through IPL2..0 */
    cpu->vbr = GUEST_VEC;
    for (int v = 0; v < 64; v++) write_BE_long(GUEST_VEC + 4 * v, 0);
    write_BE_long(GUEST_VEC + 4 * 27, GUEST_CODE + 0x400);                     /* level 3 autovector */
    write_BE_long(GUEST_DATA, 0);
    write_BE_long(GUEST_DATA + 12, 0);
    at(0x400);                                                                  /* handler: spurious? (VERTB not set in INTREQR) ; INTREQ = VERTB (clear) ; count++ ; RTE */
    w(0x0839); w(5); wl(0xDFF01F); w(0x6606); w(0x52B9); wl(GUEST_DATA + 12);   /* BTST #5,$DFF01E ; BNE.S +6 ; ADDQ.L #1,spurious */
    w(0x33FC); w(0x0020); wl(0xDFF09C);                                         /* MOVE.W #$0020,$DFF09C */
    w(0x52B9); wl(GUEST_DATA);                                                  /* ADDQ.L #1,GUEST_DATA */
    w(0x4E73);
    at(0);
    w(0x33FC); w(0x7FFF); wl(0xDFF09A);                                         /* MOVE.W #$7FFF,$DFF09A  (INTENA: all off) */
    w(0x33FC); w(0x7FFF); wl(0xDFF09C);                                         /* MOVE.W #$7FFF,$DFF09C  (INTREQ: all clear) */
    w(0x33FC); w(0xC020); wl(0xDFF09A);                                         /* MOVE.W #$C020,$DFF09A  (SET | INTEN | VERTB) */
    w(0x46FC); w(0x2000);                                                       /* MOVE #$2000,SR  (interrupts open) */
    w(0x243C); wl(60000000);                                                     /* MOVE.L #60000000,D2 */
    w(0x5382); w(0x66FC);                                                       /* L: SUBQ.L #1,D2 ; BNE.S L */
    w(0x33FC); w(0x0020); wl(0xDFF09A);                                         /* MOVE.W #$0020,$DFF09A  (VERTB off) */
    w(0x4E4F);
    c = run(GUEST_CODE);
    uint32_t frames = read_BE_long(GUEST_DATA);
    printf("[3] VERTB: %u interrupts (%u spurious: VERTB clear in INTREQR at entry), ",frames,read_BE_long(GUEST_DATA+12)); printf("%u interrupts during a %u-cycle loop (%u ms: PAL 50 per second), IPL events %u\n", frames, c, c / 1000000, (unsigned)buffee_ipl_events);
    bad += (frames == 0) || read_BE_long(GUEST_DATA + 12) != 0 || frames > 25;
    /* 4: a held level re-interrupts after RTE (the pins are a level, not an event): the handler does not acknowledge VERTB until its third entry */
    write_BE_long(GUEST_DATA + 4, 0);
    at(0x400);
    w(0x52B9); wl(GUEST_DATA + 4);                                              /* ADDQ.L #1,count */
    w(0x0CB9); wl(3); wl(GUEST_DATA + 4);                                       /* CMPI.L #3,count */
    w(0x6508);                                                                  /* BCS.S skip */
    w(0x33FC); w(0x0020); wl(0xDFF09C);                                         /* MOVE.W #$0020,$DFF09C */
    w(0x4E73);                                                                  /* skip: RTE */
    at(0);
    w(0x33FC); w(0x7FFF); wl(0xDFF09A); w(0x33FC); w(0x7FFF); wl(0xDFF09C);
    w(0x33FC); w(0xC020); wl(0xDFF09A);
    w(0x33FC); w(0x8020); wl(0xDFF09C);                                         /* INTREQ: SET | VERTB, now */
    w(0x46FC); w(0x2000);
    w(0x243C); wl(300000);
    w(0x5382); w(0x66FC);
    w(0x33FC); w(0x0020); wl(0xDFF09A);
    w(0x4E4F);
    c = run(GUEST_CODE);
    uint32_t held = read_BE_long(GUEST_DATA + 4);
    printf("[4] held level: handler entered %u times (expect 3, or 4 if a real vertical blank came in the %u us window)\n", held, c / 1000);
    bad += (held < 3 || held > 4);

    cia_probe("before 6");
    /* 6: MapROM: copy the Kickstart into the DRAM and map it at $F00000: the guest sums the first 64 KB (32768 words) from there and must get the sum read through the window */
    {
        uint32_t t0 = bbb_pmu_cycles();
        uint32_t sum = 0;
        for (uint32_t i = 0; i < 32768; i++) sum += read_BE_word(0xF80000 + 2 * i);          /* through the window, before the mapping */
        uint32_t t1 = bbb_pmu_cycles();
        uint32_t all = buffee_maprom(0x80000);
        uint32_t t2 = bbb_pmu_cycles();
        at(0);
        w(0x207C); wl(0xF80000);                         /* MOVEA.L #$F80000,A0 */
        w(0x7200); w(0x7400);                            /* MOVEQ #0,D1 ; MOVEQ #0,D2 */
        w(0x343C); w(32767);                             /* MOVE.W #32767,D2 */
        w(0x7000); w(0x3018);                            /* L: MOVEQ #0,D0 ; MOVE.W (A0)+,D0 */
        w(0xD280); w(0x51CA); w(0xFFF8);                 /* ADD.L D0,D1 ; DBF D2,L */
        w(0x4E4F);
        c = run(GUEST_CODE);
        printf("[6] MapROM: window sum of 64 KB %08X (%u us), copy of 512 KB %u ms (word sum %08X), guest sum from DRAM %08X (%u us)\n", sum, (t1 - t0) / 1000, (t2 - t1) / 1000000, all, pjit_get_dn(1), c / 1000);
        bad += (pjit_get_dn(1) != sum);
    }

    cia_probe("before 7");
    /* 7: the Kickstart's own ROM checksum loop (F800D8..F800F0) on the mapped ROM, against the same sum computed here */
    {
        const uint8_t *rom = (const uint8_t *)(BUFFEE_MAPROM_PHYS + 0x80000u);
        uint32_t d5 = 0; uint32_t off = 0; int d1 = -1, d2 = 1;
        for (;;) {
            uint32_t v = (uint32_t)rom[off] << 24 | rom[off + 1] << 16 | rom[off + 2] << 8 | rom[off + 3]; off += 4;
            uint64_t t = (uint64_t)d5 + v; d5 = (uint32_t)t; if (t >> 32) d5++;
            if ((uint16_t)--d1 != 0xFFFF) continue;
            d1 = (d1 & ~0xFFFF) | 0xFFFF;
            if ((uint16_t)--d2 == 0xFFFF) break;
        }
        at(0); w(0x41F9); wl(0xF80000); w(0x72FF); w(0x7401); w(0x7A00);      /* LEA $F80002.L,A0 ; MOVEQ #-1,D1 ; MOVEQ #1,D2 ; MOVEQ #0,D5 (as the Kickstart does) */
        w(0xDA98); w(0x6402); w(0x5285); w(0x51C9); w(0xFFF8); w(0x51CA); w(0xFFF4);   /* L: ADD.L (A0)+,D5 ; BCC.S +2 ; ADDQ.L #1,D5 ; DBRA D1,L ; DBRA D2,L */
        w(0x4E4F);
        c = run(GUEST_CODE);
        printf("[7] ROM checksum loop: guest D5 = %08X, host %08X, A0 = %08X (%u ms)\n", pjit_get_dn(5), d5, pjit_get_an(0), c / 1000000);
        bad += pjit_get_dn(5) != d5;
    }

    *(volatile uint8_t *)0xBFE201 |= 3; *(volatile uint8_t *)0xBFE001 &= 0xFC;        /* ROM overlay off: chip RAM at 0 */
    cia_probe("before 8");
    /* 8: long accesses at 2-aligned (not 4-aligned) chip RAM addresses: the abort handler redoes them as two halfwords (several addressing modes) */
    {
        at(0);
        w(0x41F9); wl(0x40002);                          /* LEA $90002.L,A0 */
        w(0x20BC); wl(0x11223344);                       /* MOVE.L #$11223344,(A0) */
        w(0x2210);                                       /* MOVE.L (A0),D1 */
        w(0x20FC); wl(0x55667788);                       /* MOVE.L #$55667788,(A0)+ */
        w(0x2420);                                       /* MOVE.L -(A0),D2 */
        w(0x43F9); wl(0x40000);                          /* LEA $90000.L,A1 */
        w(0x237C); wl(0xA1B2C3D4); w(0x0006);            /* MOVE.L #$A1B2C3D4,6(A1) */
        w(0x2629); w(0x0006);                            /* MOVE.L 6(A1),D3 */
        w(0x7806);                                       /* MOVEQ #6,D4 */
        w(0x2A31); w(0x4800);                            /* MOVE.L 0(A1,D4.L),D5 */
        w(0x2C18);                                       /* MOVE.L (A0)+,D6 */
        w(0x4E4F);
        c = run(GUEST_CODE);
        *(volatile uint8_t *)0xBFE001 |= 1;                       /* overlay back on */
        uint32_t m4 = (uint32_t)read_BE_word(0x40004) << 16 | read_BE_word(0x40006), m6 = (uint32_t)read_BE_word(0x40006) << 16 | read_BE_word(0x40008);
        printf("[8] unaligned longs: D1 %08X (11223344) D2 %08X (55667788) A0 %08X (00040006) D3 %08X (A1B2C3D4) D5 %08X (A1B2C3D4) D6 %08X (A1B2C3D4) mem $90004 %08X $90006 %08X\n",
               pjit_get_dn(1), pjit_get_dn(2), pjit_get_an(0), pjit_get_dn(3), pjit_get_dn(5), pjit_get_dn(6), m4, m6);
        bad += pjit_get_dn(1) != 0x11223344 || pjit_get_dn(2) != 0x55667788 || pjit_get_an(0) != 0x40006 || pjit_get_dn(3) != 0xA1B2C3D4 || pjit_get_dn(5) != 0xA1B2C3D4 || pjit_get_dn(6) != 0xA1B2C3D4;
    }

    cia_probe("before 9");
    /* 9: MOVEM to and from chip RAM (the Kickstart clears memory with it) */
    {
        *(volatile uint8_t *)0xBFE201 |= 3; *(volatile uint8_t *)0xBFE001 &= 0xFC;
        for (int i = 0; i < 40; i++) { write_BE_word(0x40100 + 2 * i, 0xDEAD); write_BE_word(0x40200 + 2 * i, 0xBEEF); }
        at(0);
        for (int r = 0; r < 8; r++) { w(0x203C | (r << 9)); wl(0x11110000u * (r + 1) + 0x100 + r); }     /* MOVE.L #imm,Dr */
        w(0x41F9); wl(0x40100);                          /* LEA $40100.L,A0 */
        w(0x48D0); w(0x00FF);                            /* MOVEM.L D0-D7,(A0) */
        w(0x43F9); wl(0x40220);                          /* LEA $40220.L,A1 */
        w(0x48E1); w(0xFF00);                            /* MOVEM.L D0-D7,-(A1) : predecrement mask is reversed: D0-D7 = bits 15..8 */
        w(0x4CD0); w(0xFF00);                            /* MOVEM.L (A0),A0-A7 ... (loads into D? no: bits 8-15 = A0-A7) -- skipped below */
        w(0x4E4F);
        /* the last MOVEM loads A0-A7 from the stored longs; replace it by a load of D0-D7 from $40100 after clearing D0-D7 */
        at(0);
        for (int r = 0; r < 8; r++) { w(0x203C | (r << 9)); wl(0x11110000u * (r + 1) + 0x100 + r); }
        w(0x41F9); wl(0x40100); w(0x48D0); w(0x00FF);
        w(0x43F9); wl(0x40220); w(0x48E1); w(0xFF00);
        for (int r = 0; r < 8; r++) w(0x7000 | (r << 9));  /* MOVEQ #0,Dr */
        w(0x4CD0); w(0x00FF);                            /* MOVEM.L (A0),D0-D7 */
        w(0x4E4F);
        run(GUEST_CODE);
        *(volatile uint8_t *)0xBFE001 |= 1;
        int mbad = 0;
        printf("[9] MOVEM: D0-D7 reloaded:"); for (int r = 0; r < 8; r++) { uint32_t e = 0x11110000u * (r + 1) + 0x100 + r; printf(" %08X", pjit_get_dn(r)); mbad += pjit_get_dn(r) != e; }
        *(volatile uint8_t *)0xBFE001 &= 0xFC;
        printf("\n    memory at $40100:"); for (int i = 0; i < 16; i++) printf(" %04X", read_BE_word(0x40100 + 2 * i));
        printf("\n    memory at $40200 (predecrement from $40220):"); for (int i = 0; i < 16; i++) printf(" %04X", read_BE_word(0x40200 + 2 * i)); printf("\n");
        *(volatile uint8_t *)0xBFE001 |= 1;
        bad += mbad != 0;
    }

    cia_probe("before 10");
    /* 10: byte writes to chip RAM (even and odd addresses), read back as words and bytes */
    {
        *(volatile uint8_t *)0xBFE201 |= 3; *(volatile uint8_t *)0xBFE001 &= 0xFC;
        for (int i = 0; i < 8; i++) write_BE_word(0x40300 + 2 * i, 0xAAAA);
        { volatile uint8_t *b = (volatile uint8_t *)0x40300; printf("[10c] C byte writes (word was AAAA):");
          b[0] = 0x01; printf(" even 01 -> %04X;", read_BE_word(0x40300)); b[1] = 0x02; printf(" odd 02 -> %04X;", read_BE_word(0x40300));
          write_BE_word(0x40302, 0xAAAA); b[3] = 0x77; printf(" odd 77 at +3 -> %04X;", read_BE_word(0x40302)); b[2] = 0x66; printf(" even 66 at +2 -> %04X\n", read_BE_word(0x40302));
          write_BE_word(0x40300, 0x1234); printf("     word 1234: byte reads even %02X odd %02X; ", b[0], b[1]);
          write_BE_word(0x40300, 0x00FF); printf("word 00FF: even %02X odd %02X; ", b[0], b[1]);
          write_BE_word(0x40300, 0xFF00); printf("word FF00: even %02X odd %02X\n", b[0], b[1]);
          write_BE_word(0x40300, 0x1234); b[0] = 0xFF; printf("     word 1234 then byte FF to even -> %04X; ", read_BE_word(0x40300));
          write_BE_word(0x40300, 0x1234); b[1] = 0xFF; printf("then byte FF to odd -> %04X; ", read_BE_word(0x40300));
          write_BE_word(0x40300, 0x1234); b[0] = 0x00; printf("byte 00 to even -> %04X; ", read_BE_word(0x40300));
          write_BE_word(0x40300, 0x1234); b[1] = 0x00; printf("byte 00 to odd -> %04X\n", read_BE_word(0x40300));
          for (int i = 0; i < 8; i++) write_BE_word(0x40300 + 2 * i, 0xAAAA); }
        at(0);
        w(0x41F9); wl(0x40300);                          /* LEA $40300.L,A0 */
        w(0x7001);                                       /* MOVEQ #1,D0 */
        w(0x10C0);                                       /* MOVE.B D0,(A0)+ */
        w(0x7002); w(0x10C0);                            /* MOVEQ #2,D0 ; MOVE.B D0,(A0)+ */
        w(0x7003); w(0x1080);                            /* MOVEQ #3,D0 ; MOVE.B D0,(A0) */
        w(0x7004); w(0x1140); w(0x0003);                 /* MOVEQ #4,D0 ; MOVE.B D0,3(A0) */
        w(0x1210); w(0x1428); w(0x0003);                 /* MOVE.B (A0),D1 ; MOVE.B 3(A0),D2 */
        w(0x4E4F);
        run(GUEST_CODE);
        printf("[10] byte writes: words at $40300:"); for (int i = 0; i < 4; i++) printf(" %04X", read_BE_word(0x40300 + 2 * i));
        printf(" (expect 0102 03AA AA04 AAAA... : bytes 01 02 03 .. at $40300,1,2 and 04 at $40305), D1 %02X D2 %02X\n", pjit_get_dn(1) & 0xFF, pjit_get_dn(2) & 0xFF);
        *(volatile uint8_t *)0xBFE001 |= 1;
    }

    cia_probe("before 11");
    /* 11: a blit (clear 8 words x 16 rows at $41000) started from guest code with long register writes, then WaitBlit */
    {
        *(volatile uint8_t *)0xBFE201 |= 3; *(volatile uint8_t *)0xBFE001 &= 0xFC;
        for (int i = 0; i < 16 * 8 + 8; i++) write_BE_word(0x41000 + 2 * i, 0xFFFF);
        at(0);
        w(0x4BF9); wl(0xDFF000);                         /* LEA $DFF000.L,A5 */
        w(0x3B7C); w(0x8240); w(0x0096);                 /* MOVE.W #$8240,$96(A5): DMACON SET | DMAEN | BLTEN */
        w(0x3B7C); w(0x0100); w(0x0040);                 /* MOVE.W #$0100,$40(A5): BLTCON0: D only, minterm 0 */
        w(0x3B7C); w(0x0000); w(0x0042);                 /* MOVE.W #0,$42(A5): BLTCON1 */
        w(0x2B7C); wl(0xFFFFFFFF); w(0x0044);            /* MOVE.L #-1,$44(A5): BLTAFWM / BLTALWM */
        w(0x3B7C); w(0x0000); w(0x0066);                 /* MOVE.W #0,$66(A5): BLTDMOD */
        w(0x2B7C); wl(0x41000); w(0x0054);               /* MOVE.L #$41000,$54(A5): BLTDPTH / BLTDPTL */
        w(0x3B7C); w(0x0408); w(0x0058);                 /* MOVE.W #$0408,$58(A5): BLTSIZE: 16 rows x 8 words, starts the blit */
        w(0x243C); wl(2000000);                          /* MOVE.L #2000000,D2 */
        w(0x3039); wl(0xDFF002);                         /* L: MOVE.W $DFF002,D0 */
        w(0x0800); w(14);                                /* BTST #14,D0 */
        w(0x57CA); w(0xFFF4);                            /* DBEQ D2,L: BTST sets Z when the bit is clear: loop while busy */
        w(0x4E4F);
        buffee_trace_custom();
        run(GUEST_CODE);
        { uint32_t n = buffee_wlog_pos; printf("[11] logged custom writes (%u):", (unsigned)n); for (uint32_t j = 0; j < n && j < 20; j++) printf(" %03X=%X/%u", (unsigned)(buffee_wlog[2 * j] & 0xFFF), (unsigned)buffee_wlog[2 * j + 1], (unsigned)(buffee_wlog[2 * j] >> 28)); printf("\n"); }
        printf("[11] blit: DMACONR %04X after the wait loop (D2 left %u of 2000000: BBUSY should have cleared quickly); memory $41000: %04X %04X ... %04X (expect 0)\n",
               pjit_get_dn(0) & 0xFFFF, pjit_get_dn(2), read_BE_word(0x41000), read_BE_word(0x41002), read_BE_word(0x41000 + 2 * 127));
        *(volatile uint8_t *)0xBFE001 |= 1;
    }

    cia_probe("before 12");
    /* 12: chip RAM read reliability with the display DMA on (5 bitplanes, lores) and the blitter busy: 20 s of read-verify of 64 KB at $60000, halfword and longword reads */
    {
        *(volatile uint8_t *)0xBFE201 |= 3; *(volatile uint8_t *)0xBFE001 &= 0xFC;
        for (uint32_t i = 0; i < 0x8000; i++) write_BE_word(0x60000 + 2 * i, (uint16_t)(i * 0x9E37u + 0x1234u));
        volatile uint16_t *c = (volatile uint16_t *)0xDFF000;
        #define CW(off, v) write_BE_word(0xDFF000 + (off), (v))
        CW(0x96, 0x7FFF); CW(0x9A, 0x7FFF); CW(0x100, 0x5200); CW(0x102, 0); CW(0x104, 0); CW(0x108, 0); CW(0x10A, 0);
        CW(0x8E, 0x2C81); CW(0x90, 0xF4C1); CW(0x92, 0x38); CW(0x94, 0xD0);
        for (int pl = 0; pl < 5; pl++) { uint32_t pt = 0x80000 + pl * 0x2800; CW(0xE0 + 4 * pl, pt >> 16); CW(0xE2 + 4 * pl, pt & 0xFFFF); }
        for (uint32_t i = 0; i < 0x6800; i++) write_BE_word(0x80000 + 2 * i, 0x5555);
        CW(0x96, 0x8000 | 0x0200 | 0x0100 | 0x0040);      /* DMAEN | BPLEN | BLTEN */
        (void)c;
        uint32_t errs = 0, loops = 0, lerrs = 0, berrs = 0; double t0 = 0; uint32_t c0 = bbb_pmu_cycles();
        while (bbb_pmu_cycles() - c0 < 8u * 1000000000u / 2) {
            /* keep the blitter busy: copy 0x2000 words $90000 -> $A0000 (A,D; 64 x 128 words) */
            if (!(read_BE_word(0xDFF002) & 0x4000)) {
                CW(0x40, 0x09F0); CW(0x42, 0); CW(0x44, 0xFFFF); CW(0x46, 0xFFFF); CW(0x64, 0); CW(0x66, 0);
                CW(0x50, 0x9); CW(0x52, 0x0000); CW(0x54, 0xA); CW(0x56, 0x0000); CW(0x58, (64 << 6) | 128);
            }
            for (uint32_t i = 0; i < 0x8000; i += 7) { uint16_t v = read_BE_word(0x60000 + 2 * i); if (v != (uint16_t)(i * 0x9E37u + 0x1234u)) errs++; }
            for (uint32_t i = 0; i < 0x8000 - 2; i += 11) { uint32_t v = read_BE_word(0x60000 + 2 * i) << 16 | read_BE_word(0x60000 + 2 * i + 2); (void)v; }
            loops++;
        }
        (void)t0; (void)lerrs; (void)berrs;
        CW(0x96, 0x7FFF);
        printf("[12] chip RAM reads with display + blitter DMA: %u passes, %u wrong halfwords\n", (unsigned)loops, (unsigned)errs);
        *(volatile uint8_t *)0xBFE001 |= 1;
        bad += errs != 0;
    }

    cia_probe("before 13");
    /* 13: write and 32-bit access integrity in chip RAM with display + blitter DMA still on: 4 s of write-verify (halfword writes, 32-bit writes, 32-bit reads) of 16 KB at $70000 */
    {
        *(volatile uint8_t *)0xBFE201 |= 3; *(volatile uint8_t *)0xBFE001 &= 0xFC;
        CW(0x96, 0x8000 | 0x0200 | 0x0100 | 0x0040);
        volatile uint16_t *h = (volatile uint16_t *)0x70000; volatile uint32_t *l = (volatile uint32_t *)0x70000;
        uint32_t e16 = 0, e32w = 0, e32r = 0, pass = 0, seed = 1; uint32_t c0 = bbb_pmu_cycles();
        while (bbb_pmu_cycles() - c0 < 2u * 1000000000u) {
            if (!(read_BE_word(0xDFF002) & 0x4000)) { CW(0x40, 0x09F0); CW(0x42, 0); CW(0x44, 0xFFFF); CW(0x46, 0xFFFF); CW(0x64, 0); CW(0x66, 0); CW(0x50, 0x9); CW(0x52, 0x0000); CW(0x54, 0xA); CW(0x56, 0x0000); CW(0x58, (64 << 6) | 128); }
            seed = seed * 1664525u + 1013904223u;
            for (uint32_t i = 0; i < 0x2000; i++) h[i] = (uint16_t)(seed + i * 0x3D5u);
            for (uint32_t i = 0; i < 0x2000; i++) if (h[i] != (uint16_t)(seed + i * 0x3D5u)) e16++;
            for (uint32_t i = 0; i < 0x1000; i++) l[i] = seed ^ (i * 0x9E3779B1u);
            for (uint32_t i = 0; i < 0x2000; i++) { uint32_t v = seed ^ ((i >> 1) * 0x9E3779B1u); uint16_t want = (i & 1) ? (uint16_t)(v >> 16) : (uint16_t)v; if (h[i] != want) e32w++; }
            for (uint32_t i = 0; i < 0x1000; i++) if (l[i] != (seed ^ (i * 0x9E3779B1u))) e32r++;
            pass++;
        }
        CW(0x96, 0x7FFF);
        printf("[13] write integrity with DMA: %u passes; halfword write-verify errors %u, 32-bit write errors %u, 32-bit read errors %u\n", (unsigned)pass, (unsigned)e16, (unsigned)e32w, (unsigned)e32r);
        *(volatile uint8_t *)0xBFE001 |= 1;
        bad += (e16 | e32w | e32r) != 0;
    }

    /* 14: CIA write reliability: 100000 write / read-back pairs each on CIAA DDRA ($BFE201, low 2 bits: restored afterwards) and CIAB DDRA ($BFD200), first with the DMA off, then with display + blitter DMA on */
    for (int dma = 0; dma < 2; dma++) {
        CW(0x96, dma ? (0x8000 | 0x0200 | 0x0100 | 0x0040) : 0x7FFF);
        volatile uint8_t *pa = (volatile uint8_t *)0xBFE201, *pb = (volatile uint8_t *)0xBFD200;
        uint8_t saved_a = *pa, saved_b = *pb; uint32_t ea = 0, eb = 0, lost_a = 0;
        for (uint32_t i = 0; i < 100000; i++) {
            if (dma && !(i & 0x3FF) && !(read_BE_word(0xDFF002) & 0x4000)) { CW(0x40, 0x09F0); CW(0x42, 0); CW(0x44, 0xFFFF); CW(0x46, 0xFFFF); CW(0x64, 0); CW(0x66, 0); CW(0x50, 0x9); CW(0x52, 0x0000); CW(0x54, 0xA); CW(0x56, 0x0000); CW(0x58, (64 << 6) | 128); }
            uint8_t v = (uint8_t)(i * 37u + 11u) & 3;
            *pa = v; { uint8_t r = *pa & 3; if (r != v) { ea++; if (r == 3) lost_a++; } }
            *pb = v; if ((*pb & 3) != v) eb++;
        }
        *pa = saved_a; *pb = saved_b;
        CW(0x96, 0x7FFF);
        printf("[14] CIA write/read-back, DMA %s: CIAA DDRA errors %u (stale 3: %u), CIAB DDRA errors %u of 100000\n", dma ? "on " : "off", (unsigned)ea, (unsigned)lost_a, (unsigned)eb);
        bad += (ea | eb) != 0;
    }

    /* 15: the same with a control register (CRA bits 1-2: PBON / OUTMODE, harmless): CIAA $BFEE01 and CIAB $BFDE00 */
    {
        volatile uint8_t *pa = (volatile uint8_t *)0xBFEE01, *pb = (volatile uint8_t *)0xBFDE00;
        uint8_t sa = *pa, sb = *pb; uint32_t ea = 0, eb = 0; uint32_t fa = 0;
        for (uint32_t i = 0; i < 100000; i++) {
            uint8_t v = (uint8_t)(i * 37u + 11u) & 0x06;
            *pa = v; { uint8_t r = *pa & 0x06; if (r != v) { if (ea < 4) printf("  CIAA CRA wrote %02X read %02X\n", v, r); ea++; } }
            *pb = v; if ((*pb & 0x06) != v) eb++;
        }
        *pa = sa & 0x7E; *pb = sb & 0x7E; (void)fa;
        printf("[15] CRA write/read-back: CIAA errors %u, CIAB errors %u of 100000\n", (unsigned)ea, (unsigned)eb);
    }

    /* 16: CIAA CRA alone, and with a gap, and with a read between write and read-back of a *different* register */
    {
        volatile uint8_t *pa = (volatile uint8_t *)0xBFEE01; uint8_t sa = *pa; uint32_t e1 = 0, e2 = 0, e3 = 0, e4 = 0;
        for (uint32_t i = 0; i < 20000; i++) { uint8_t v = (uint8_t)(i * 37u + 11u) & 0x06; *pa = v; if ((*pa & 6) != v) e1++; }
        for (uint32_t i = 0; i < 20000; i++) { uint8_t v = (uint8_t)(i * 37u + 11u) & 0x06; *pa = v; for (volatile int k = 0; k < 400; k++) ; if ((*pa & 6) != v) e2++; }
        for (uint32_t i = 0; i < 20000; i++) { uint8_t v = (uint8_t)(i * 37u + 11u) & 0x06; *pa = v; (void)*(volatile uint8_t *)0xBFEF01; if ((*pa & 6) != v) e3++; }
        for (uint32_t i = 0; i < 20000; i++) { uint8_t v = (uint8_t)(i * 37u + 11u) & 0x06; (void)*pa; *pa = v; (void)*pa; (void)*pa; if ((*pa & 6) != v) e4++; }
        *pa = sa & 0x7E;
        printf("[16] CIAA CRA: write+read %u, with a 400-loop gap %u, with another CIAA read between %u, read-write-read-read-read %u of 20000 errors\n", (unsigned)e1, (unsigned)e2, (unsigned)e3, (unsigned)e4);
    }

    cia_probe("late");

    /* 18: line-mode blits with the register values Kickstart's Draw() writes (a 4-pixel line, then a 628-pixel one) */
    {
        *(volatile uint8_t *)0xBFE201 |= 3; *(volatile uint8_t *)0xBFE001 &= 0xFC;
        for (int i = 0; i < 0x400; i++) write_BE_word(0x9000 + 2 * i, 0);
        CW(0x96, 0x8000 | 0x0200 | 0x0040);
        static const struct { uint16_t size, bmod, amod, cmod, con0, con1, err; } L[] = { { 0x102, 0, 0xFFFE, 2, 0x0BFA, 0x1051, 0xFFFF }, { 0x9D02, 0, 0xFFFE, 2, 0x0BFA, 0xF051, 0xFD8D } };
        for (int k = 0; k < 2; k++) {
            { uint32_t w = 0; while ((read_BE_word(0xDFF002) & 0x4000) && w < 1000000) w++; printf("[18] before blit %u: wait %u, DMACONR %04X\n", k, (unsigned)w, read_BE_word(0xDFF002)); }
            CW(0x62, L[k].bmod); CW(0x64, L[k].amod); CW(0x66, 2); CW(0x44, 0xFFFF); CW(0x46, 0xFFFF); CW(0x74, 0x8000); CW(0x60, L[k].cmod);
            CW(0x40, L[k].con0); CW(0x42, L[k].con1); CW(0x52, L[k].err); CW(0x48, 0); CW(0x4A, 0x9000); CW(0x54, 0); CW(0x56, 0x9000); CW(0x72, 0xFFFF);
            CW(0x58, L[k].size);
            uint32_t n = 0; while ((read_BE_word(0xDFF002) & 0x4000) && n < 200000) n++;
            printf("[18] line blit %u (BLTSIZE %04X): DMACONR %04X after %u polls %s\n", k, L[k].size, read_BE_word(0xDFF002), (unsigned)n, n >= 200000 ? "HUNG" : "done");
        }
        CW(0x96, 0x7FFF);
        *(volatile uint8_t *)0xBFE001 |= 1;
    }

    *(volatile uint8_t *)0xBFE201 |= 3; *(volatile uint8_t *)0xBFE001 &= 0xFC;       /* ROM overlay off: low memory is chip RAM */
    /* 21a: which channel reads what? D = ~C and D = ~A over source words of a known pattern, destination elsewhere */
    {
        CW(0x96, 0x7FFF); CW(0x96, 0x8000 | 0x0200 | 0x0040);
        for (int e = 0; e < 4; e++) {
            for (int i = 0; i < 10; i++) { write_BE_word(0x60000 + 2 * i, 0x1111 * (i + 1)); write_BE_word(0x62000 + 2 * i, 0x0000); }
            uint16_t con0 = e < 2 ? 0x0355 : 0x090F;               /* e 0,1: USEC|USED, D = ~C; e 2,3: USEA|USED, D = ~A */
            CW(0x40, con0); CW(0x42, 0); CW(0x44, 0xFFFF); CW(0x46, 0xFFFF); CW(0x60, 0); CW(0x62, 0); CW(0x64, 0); CW(0x66, 0);
            if (e < 2) { CW(0x48, 0x6); CW(0x4A, 0); } else { CW(0x50, 0x6); CW(0x52, 0); }
            CW(0x54, 0x6); CW(0x56, 0x2000);
            CW(0x58, 0x201);
            { uint32_t w = 0; while ((read_BE_word(0xDFF002) & 0x4000) && w < 200000) w++; }
            printf("[21a] e=%d con0 %04X: dest $B2000:", e, con0); for (int i = 0; i < 10; i++) printf(" %04X", read_BE_word(0x62000 + 2 * i)); printf("; src:"); for (int i = 0; i < 3; i++) printf(" %04X", read_BE_word(0x60000 + 2 * i)); printf("; DMACONR %04X\n", read_BE_word(0xDFF002));
        }
        CW(0x96, 0x7FFF);
    }

    /* 21: does one BLTSIZE write start exactly one blit? D = ~C over the same 8 words inverts them once if the blit runs once and twice (back to zero) if the write strobe reached Agnus twice;
     * a restart in the middle shows as a partial pattern. Without and with the display DMA on. */
    for (int pass = 0; pass < 2; pass++) {
        CW(0x96, 0x7FFF); CW(0x9A, 0x7FFF);
        if (pass) {
            CW(0x100, 0x5200); CW(0x102, 0); CW(0x104, 0); CW(0x108, 0); CW(0x10A, 0); CW(0x8E, 0x2C81); CW(0x90, 0xF4C1); CW(0x92, 0x38); CW(0x94, 0xD0);
            for (int pl = 0; pl < 5; pl++) { uint32_t pt = 0x80000 + pl * 0x2800; CW(0xE0 + 4 * pl, pt >> 16); CW(0xE2 + 4 * pl, pt & 0xFFFF); }
        }
        CW(0x96, 0x8000 | 0x0200 | 0x0040 | (pass ? 0x0100 : 0));
        uint32_t single = 0, none = 0, other = 0, hung = 0, shown = 0, N = 20000;
        for (uint32_t n = 0; n < N; n++) {
            for (int i = 0; i < 10; i++) write_BE_word(0x60000 + 2 * i, 0);
            { uint32_t w = 0; while ((read_BE_word(0xDFF002) & 0x4000) && w < 200000) w++; if (w >= 200000) { hung++; break; } }
            CW(0x40, 0x0355); CW(0x42, 0); CW(0x44, 0xFFFF); CW(0x46, 0xFFFF); CW(0x60, 0); CW(0x66, 0);
            CW(0x48, 0x6); CW(0x4A, 0); CW(0x54, 0x6); CW(0x56, 0);
            CW(0x58, 0x201);
            { uint32_t w = 0; while ((read_BE_word(0xDFF002) & 0x4000) && w < 200000) w++; if (w >= 200000) { hung++; printf("[21] HUNG after BLTSIZE, iter %u, DMACONR %04X INTREQR %04X\n", (unsigned)n, read_BE_word(0xDFF002), read_BE_word(0xDFF01E)); break; } }
            uint16_t v[10]; uint32_t ones = 0, zeros = 0;
            for (int i = 0; i < 10; i++) { v[i] = read_BE_word(0x60000 + 2 * i); if (v[i] == 0xFFFF) ones++; else if (v[i] == 0) zeros++; }
            if (ones == 8 && v[8] == 0 && v[9] == 0) single++;
            else if (zeros == 10) none++;
            else { other++; if (shown++ < 3) { printf("[21] iter %u odd result:", (unsigned)n); for (int i = 0; i < 10; i++) printf(" %04X", v[i]); printf(" ; again:"); for (int i = 0; i < 10; i++) printf(" %04X", read_BE_word(0x60000 + 2 * i)); printf(" ; DMACONR %04X\n", read_BE_word(0xDFF002)); } }
        }
        printf("[21] %s display DMA: %u single, %u all-zero (no start or two), %u other, %u hung (of %u)\n", pass ? "with" : "without", (unsigned)single, (unsigned)none, (unsigned)other, (unsigned)hung, (unsigned)N);
        CW(0x96, 0x7FFF);
    }

    /* 21b: (run twice: first with the first DMACONR poll delayed ~200 us after the BLTSIZE write, then immediately) is BLTSIZE latched as written? random H x W (1..8 each) D-only fill of 0xFFFF into zeroed memory; the number of words changed must be H * W */
    for (int delayed = 1; delayed >= 0; delayed--) {
        CW(0x96, 0x7FFF); CW(0x96, 0x8000 | 0x0200 | 0x0040);
        uint32_t seed = 12345, bad = 0, good = 0, hung21 = 0;
        for (uint32_t n = 0; n < 20000 && !hung21; n++) {
            seed = seed * 1664525u + 1013904223u; uint32_t H = 1 + (seed >> 16) % 8, W = 1 + (seed >> 24) % 8;
            for (int i = 0; i < 70; i++) write_BE_word(0x60000 + 2 * i, 0);
            CW(0x40, 0x01FF); CW(0x42, 0); CW(0x44, 0xFFFF); CW(0x46, 0xFFFF); CW(0x66, 0); CW(0x54, 0x6); CW(0x56, 0);
            CW(0x58, (uint16_t)(H << 6 | W));
            if (delayed) for (volatile int k = 0; k < 60000; k++) ;
            uint32_t w = 0; while ((read_BE_word(0xDFF002) & 0x4000) && w < 200000) w++;
            if (w >= 200000) { hung21 = 1; printf("[21b] HUNG at iter %u after BLTSIZE %04X (H %u W %u): DMACONR %04X\n", (unsigned)n, (unsigned)(H << 6 | W), (unsigned)H, (unsigned)W, read_BE_word(0xDFF002)); break; }
            uint32_t cnt = 0; for (int i = 0; i < 70; i++) if (read_BE_word(0x60000 + 2 * i) == 0xFFFF) cnt++;
            if (cnt == H * W) good++; else { if (bad++ < 12) printf("[21b] iter %u BLTSIZE %04X (H %u W %u): %u words written (expected %u)\n", (unsigned)n, (unsigned)(H << 6 | W), (unsigned)H, (unsigned)W, (unsigned)cnt, (unsigned)(H * W)); }
        }
        printf("[21b] %s poll: %u good, %u wrong, hung %u\n", delayed ? "delayed" : "immediate", (unsigned)good, (unsigned)bad, (unsigned)hung21);
        CW(0x96, 0x7FFF);
    }

    /* 19: byte reads of DMACONR ($DFF002 high byte, $DFF003 low byte) while idle and DMA on */
    {
        CW(0x96, 0x8000 | 0x0200 | 0x0100 | 0x0040);
        volatile uint8_t *d = (volatile uint8_t *)0xDFF002; uint32_t bad_hi = 0, n = 0;
        for (int i = 0; i < 100000; i++) { uint8_t hi = d[0]; uint16_t w = read_BE_word(0xDFF002); if (hi != (w >> 8) && !((hi ^ (w >> 8)) & ~0x40)) {} if (hi & 0x40) bad_hi++; n++; }
        printf("[19] DMACONR: word %04X, byte reads even %02X odd %02X; BBUSY seen in %u of %u idle byte reads\n", read_BE_word(0xDFF002), d[0], d[1], (unsigned)bad_hi, (unsigned)n);
        CW(0x96, 0x7FFF);
    }

    /* 20: chip identification registers */
    printf("[20] VPOSR %04X VHPOSR %04X DENISEID(07C) %04X, again %04X %04X; DMACONR %04X; $DFF07E %04X $DFF0FE %04X JOY0DAT %04X JOY1DAT %04X POTGOR %04X\n", read_BE_word(0xDFF004), read_BE_word(0xDFF006), read_BE_word(0xDFF07C), read_BE_word(0xDFF07C), read_BE_word(0xDFF07C), read_BE_word(0xDFF002), read_BE_word(0xDFF07E), read_BE_word(0xDFF0FE), read_BE_word(0xDFF00A), read_BE_word(0xDFF00C), read_BE_word(0xDFF016));

    /* 5: a RESET: the guest raises the RESET interrupt itself (GPIO3 IRQSTATUSRAW_0, bit 16: a big-endian store of 0x00000100) and spins; pjit_run() must return 0x100 */
    at(0);
    w(0x23FC); wl(0x00000100); wl(0x481AE024);                                  /* MOVE.L #$100,$481AE024 */
    w(0x60FE);                                                                  /* BRA.S * */
    uint32_t r = run(GUEST_CODE);
    (void)r;
    printf("[5] reset: pjit_run() returned 0x%X (expect 0x%X), RESET events %u\n", (unsigned)pjit_last_exit, BUFFEE_RESET_EXIT, (unsigned)buffee_reset_events);
    bad += (pjit_last_exit != BUFFEE_RESET_EXIT);
    printf(bad ? "FAIL\n" : "PASS (the bootloader restarts the payload after recalibrating)\n");
    if (!bad) { *BUFFEE_MAILBOX = 1; return BUFFEE_RESET_EXIT; }
    return 1;
}
