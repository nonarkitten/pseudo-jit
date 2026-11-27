/*
 * bustest.c -- the self-test of the board's connection to the Amiga: every line of the 68000 socket the bootloader can exercise, from the ARM through the GreenPAK
 * and the buffers to the Amiga's own chips, checked to be connected, not stuck and not shorted to another, so that a bad joint or a badly seated board is named
 * ("A11 floats") instead of showing up as a Kickstart that crashes.
 *
 *   D0-D15    walking one and walking zero through a word of chip RAM: a line stuck high or low, two lines shorted, a line wrong on some reads only.
 *   UDS, LDS  a byte write to each half of a word leaves the other half alone.
 *   A1-A20    the address-bus test: a different word at $0 and at each power of two, read back; two addresses that read the same word share a cell, so a line
 *             is stuck or open (it shares with $0) or two lines are shorted (two powers of two share). Lines past the end of the chip RAM share with $0 or hold
 *             nothing on every pass: that is where the chip RAM ends (256 KB at least), not a fault; the report gives the size, which is the check for a line
 *             that makes 1 MB look like 512 KB (A19 stuck high). Where no RAM answers, a read returns what was last on the bus, often the word just written,
 *             so another word is written elsewhere before each read.
 *   floating  a line that is at the wrong level for stretches of time: $0 and each power of two written and read back at once (too short for the line to change
 *             level in between); one cell on some tries and two on the others names the line. A line that changes level in the middle of the longer address
 *             pass makes every read after it wrong, which names nothing.
 *   A18-A23   through the ROM, which reaches lines a small chip RAM does not: the two halves of a 512 KB Kickstart differ (A18); its first words are not seen at
 *             $E80000 (A20 stuck high); A21-A23 stuck high move the chip RAM away (the data test fails); A19-A23 stuck low move the ROM. ($F00000, where A19
 *             stuck high would show, answers nothing: a read there takes ~80 ms.)
 *   IPL0-2    Paula raises each level 1-6 from one interrupt bit (INTENA, INTREQ) and the board must read that level on its three inputs, and 0 with all off.
 *   VPA, E    the CIA-A takes a write to its port A and its direction register (the overlay bit).
 *
 * A line that floats is right most of the time, so the data, address, float and ROM checks run as rounds of one loop spread over about 140 ms, and the verdicts
 * come after it in the order each needs the one before: the data lines, then the address lines, then the rest. The words of chip RAM the test uses are put back
 * (a warm start keeps ExecBase at $4); interrupts are left off and the overlay on, as after a reset.
 *
 * Copyright (c) 2026 the Buffee Project - http://www.buffee.ca
 * Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (see LICENSE).
 */
#include "main.h"
#include "amiga.h"
#include "irq.h"

#define LINES        20                 /* A1..A20: offsets 2^1 .. 2^20 (2 MB of chip RAM) */
#define CHIP_MIN     17                 /* every Amiga has 256 KB of chip RAM: A1..A17 must work in it */
#define ROUNDS       64                 /* rounds of the loop, ROUND_GAP_US apart */
#define ROUND_GAP_US 2000
#define PROBE_EVERY  4                  /* the $E80000 probe on every 4th round: 16 tries */
#define DATA_WORD    0x000100u          /* the chip RAM word of the data and byte-lane tests */
#define SPARE_WORD   0x003FFEu          /* written before each read where no RAM may answer: no power of two, and inside 256 KB */
#define ROM          0xF80000u
#define A20_PROBE    0xE80000u          /* F80000 with A20 low: the autoconfig space, never the ROM */
#define INTENA       0xDFF09Au
#define INTREQ       0xDFF09Cu
#define CIAA_PRA     0xBFE001u
#define CIAA_DDRA    0xBFE201u

static int errors;

static void BadLine(void) {                                      /* the start of a fault's line; the first one heads the list */
    if (!errors) printf("Amiga bus test:\n");
    errors++;
    printf("  ");
}
#define Bad(...) do { BadLine(); printf(__VA_ARGS__); printf("\n"); } while (0)

static uint32_t Offset(int n) { return n ? 1u << n : 0u; }
static int Bits(uint16_t x) { int n = 0; for (; x; x &= (uint16_t)(x - 1)) n++; return n; }      /* no libgcc: no __builtin_popcount */

static uint16_t ReadBack(uint32_t a, uint16_t filler) {
    AmigaWrite16(SPARE_WORD, filler);                            /* where no RAM answers the read returns this, not the word written to a */
    return AmigaRead16(a);
}

/* What the rounds find, for the verdicts after the loop. */
static struct {
    uint16_t seen1, seen0, shorted[16];                          /* data: bits read as 1 / as 0; the lines that followed each walking bit */
    unsigned flips[16], flip_reads, whole, data_reads;           /* reads one or two bits off per line; reads off in more bits (an address, not a data line) */
    uint8_t bad[LINES + 1], partner[LINES + 1][LINES + 1];       /* address passes: line n wrong; n and i read the same word */
    uint8_t empty;                                               /* address passes where no cell below 256 KB kept its word */
    uint8_t shared[LINES + 1];                                   /* float rounds: $0 and 2^k one cell */
    unsigned other;                                              /* float rounds: another cell answered */
    uint16_t head[4];                                            /* the Kickstart's first words, 0 if there is none */
    uint8_t rom_odd, probe_hits, probe_tries;
    uint8_t uds_bad, lds_bad, lanes_lost;                        /* byte-write rounds where a strobe did the wrong thing; where no RAM answered (an address line) */
    uint16_t uds_seen, lds_seen;                                 /* the last wrong word read */
    uint8_t unnamed;                                             /* address passes wrong on some rounds, and no float round named the line */
} t;

/* A word read back one or two bits off is a data line: counted for the data verdict. Otherwise 1: the word as a whole is wrong. */
static int WholeWrong(uint16_t got, uint16_t want) {
    uint16_t x = got ^ want;
    int c;

    if (!x) return 0;
    if (Bits(x) > 2) return 1;
    for (c = 0; c < 16; c++) t.flips[c] += (x >> c) & 1;
    t.flip_reads++;
    return 0;
}

static void DataRound(void) {
    uint16_t want, got, x;
    int b, k, c;

    for (b = 0; b < 16; b++)
        for (k = 0; k < 2; k++) {                                /* walking one, then walking zero */
            want = k ? (uint16_t)~(1u << b) : (uint16_t)(1u << b);
            AmigaWrite16(DATA_WORD, want);
            got = ReadBack(DATA_WORD, (uint16_t)~want);
            t.data_reads++;
            x = got ^ want;
            if (Bits(x) > 2) { t.whole++; continue; }
            t.seen1 |= got; t.seen0 |= (uint16_t)~got;
            for (c = 0; c < 16; c++) t.flips[c] += (x >> c) & 1;
            t.flip_reads++;
            t.shorted[b] |= k ? (uint16_t)(~got & want) : (uint16_t)(got & ~want);    /* the other lines that followed b: wired AND, wired OR */
        }
    AmigaWrite16(DATA_WORD, 0x0000); AmigaWrite8(DATA_WORD, 0xA5);                    /* UDS alone, then LDS alone */
    if ((got = ReadBack(DATA_WORD, 0x5AFF)) == 0x5AFF) t.lanes_lost++;               /* the filler: no RAM answered, an address line moved the word */
    else if (WholeWrong(got, 0xA500)) { t.uds_bad++; t.uds_seen = got; }
    AmigaWrite8(DATA_WORD + 1, 0x5A);
    if ((got = ReadBack(DATA_WORD, 0xA5FF)) == 0xA5FF) t.lanes_lost++;
    else if (WholeWrong(got, 0xA55A)) { t.lds_bad++; t.lds_seen = got; }
}

static void AddressPass(int pass) {
    static uint16_t v[LINES + 1], r[LINES + 1];
    int n, i, held = 0;

    for (n = 0; n <= LINES; n++) v[n] = (uint16_t)((pass * 0x3C3Cu) ^ (0xA5A5u + n * 0x0B0Bu));     /* 21 different words, a new set each pass */
    for (n = 0; n <= LINES; n++) AmigaWrite16(Offset(n), v[n]);
    for (n = 0; n <= LINES; n++) { r[n] = ReadBack(Offset(n), (uint16_t)~v[n]); held += n <= CHIP_MIN && r[n] == v[n]; }
    t.empty += !held;
    for (n = 1; n <= LINES; n++) {                               /* n is right when its word is its own and no other address shows it */
        int wrong = r[n] != v[n], shared = 0;
        for (i = 0; i <= LINES; i++)
            if (i != n && (r[i] == v[n] || r[n] == v[i])) { shared = 1; if (t.partner[n][i] < 255) t.partner[n][i]++; }
        if (wrong && !shared && Bits(r[n] ^ v[n]) <= 2) {        /* its own word with a bit or two off: a data line */
            for (i = 0; i < 16; i++) t.flips[i] += ((r[n] ^ v[n]) >> i) & 1;
            t.flip_reads++;
            continue;
        }
        if (wrong || shared) t.bad[n]++;
    }
}

static void FloatRound(int round) {
    uint16_t x, y, r0, rk;
    int k;

    for (k = 1; k <= LINES; k++) {
        x = (uint16_t)(0x1234u + round * 0x0F0Fu + k * 0x0101u); y = (uint16_t)~x;
        AmigaWrite16(0, x); AmigaWrite16(Offset(k), y);
        r0 = ReadBack(0, (uint16_t)(x ^ 0x3C3Cu));                /* the filler is neither x nor y */
        rk = ReadBack(Offset(k), (uint16_t)(x ^ 0x3C3Cu));
        if (r0 == y && rk == y) t.shared[k]++;                   /* $0 got the word written to 2^k: one cell for these few accesses */
        else if (r0 != x || rk != y) t.other++;
    }
}

static void RomRound(int round) {
    int j, same;

    if (!t.head[0]) return;
    for (j = 0; j < 4; j++) if (WholeWrong(AmigaRead16(ROM + 2 * j), t.head[j])) { t.rom_odd++; break; }
    if (round % PROBE_EVERY) return;
    (void)AmigaRead16(DATA_WORD);                                /* another value on the bus first: an empty address does not repeat the ROM's words */
    for (same = 0, j = 0; j < 4 && same == j; j++) same += AmigaRead16(A20_PROBE + 2 * j) == t.head[j];
    t.probe_hits += same == 4;
    t.probe_tries++;
}

/* The verdicts. Data: 0, or 1 when the address and the other verdicts cannot be trusted. */
static int DataVerdict(void) {
    uint16_t paired = 0, good = t.seen1 & t.seen0;
    int b, c, before = errors;

    if (t.whole == t.data_reads) {
        Bad("chip RAM at $%06lX does not keep what is written (one of A19-A23 stuck high, or the ROM overlay still on): no line can be tested", (unsigned long)DATA_WORD);
        return 1;
    }
    for (b = 0; b < 16; b++) {
        if (!(t.seen1 >> b & 1)) { Bad("D%d stuck low or open (never read as 1)", b); continue; }
        if (!(t.seen0 >> b & 1)) { Bad("D%d stuck high (never read as 0)", b); continue; }
        for (c = b + 1; c < 16; c++)
            if ((t.shorted[b] >> c & 1) && (t.shorted[c] >> b & 1) && good >> c & 1) {
                Bad("D%d and D%d shorted", b, c);
                paired |= (uint16_t)(1u << b | 1u << c);
            }
    }
    for (b = 0; b < 16; b++) if (paired >> b & 1 || !(good >> b & 1)) t.flips[b] = 0;      /* named already */

    if (t.uds_bad) Bad("UDS: a byte write to the upper half (even address) went wrong on %d of %d tries (read back %04X, not A500)", t.uds_bad, ROUNDS, t.uds_seen);
    if (t.lds_bad) Bad("LDS: a byte write to the lower half (odd address) went wrong on %d of %d tries (read back %04X, not A55A)", t.lds_bad, ROUNDS, t.lds_seen);
    return errors != before;
}

/* Address: the number of lines the chip RAM covers. */
static int AddressVerdict(void) {
    static char list[128], cells[200];
    static uint8_t told[LINES + 1];
    char *lp, *cp;
    int n, i, top, members, zero, unsteady = 0, unsteady_above = 0, named = 0;

    if (t.empty == ROUNDS) { Bad("no chip RAM keeps a word at $0 or the powers of two below $40000 (one of A19-A23 stuck high?)"); return 0; }
    /* The chip RAM's end: the first line above CHIP_MIN that is wrong on every pass, sharing only with $0 or lines above it (or holding nothing). */
    for (top = CHIP_MIN + 1; top <= LINES; top++) {
        int low = 0;
        for (i = 1; i < top && i <= CHIP_MIN; i++) low |= t.partner[top][i] != 0;
        if (t.bad[top] == ROUNDS && !low) break;
    }
    memset(told, 0, sizeof told);
    for (n = 1; n < top; n++) {
        if (!t.bad[n] || told[n]) continue;
        if (t.bad[n] < ROUNDS) { unsteady++; continue; }        /* named by the float rounds, which can tell which line it is */
        zero = t.partner[n][0] >= ROUNDS / 2;                    /* wrong on every pass: the lines that share n's cell, and whether $0 does */
        lp = list + sprintf(list, "A%d", n); cp = cells + sprintf(cells, "$%06lX", (unsigned long)Offset(n));
        for (members = 1, i = n + 1; i < top; i++)
            if (t.partner[n][i] >= ROUNDS / 2 && t.bad[i] == ROUNDS) {
                told[i] = 1; members++;
                lp += sprintf(lp, ", A%d", i);
                cp += sprintf(cp, ", $%06lX", (unsigned long)Offset(i));
            }
        if (zero && members > 1) Bad("%s stuck low, or shorted to each other: %s and $000000 are one cell", list, cells);
        else if (zero) Bad("%s stuck or open: %s is the same cell as $000000", list, cells);
        else if (members > 1) Bad("%s shorted: %s are one cell", list, cells);
        else Bad("%s: $%06lX does not keep what is written", list, (unsigned long)Offset(n));
    }
    for (n = top; n <= LINES; n++)                               /* past the end of the chip RAM a line is wrong on every pass, not on some */
        if (t.bad[n] && t.bad[n] < ROUNDS) { unsteady++; unsteady_above = n; }
    for (n = 1; n < top; n++)
        if (t.shared[n] && t.shared[n] < ROUNDS) { Bad("A%d floats: $%06lX and $000000 were one cell on %d of %d tries", n, (unsigned long)Offset(n), t.shared[n], ROUNDS); named++; }
    if (unsteady && !named && unsteady_above) Bad("A%d floats: $%06lX was its own cell on some passes and not on others", unsteady_above, (unsigned long)Offset(unsteady_above));
    else if (unsteady && !named) t.unnamed = 1;                  /* the ROM's probes may name it; else the catch-all after them */
    return top - 1;
}

static void RomVerdict(void) {
    int k, same;

    if (!t.head[0]) {
        if (t.unnamed) Bad("an address line floats, but no single line could be named (chip RAM wrong on some passes)");
        if (!errors) info("Amiga bus: no Kickstart at $F80000: A18-A20 not checked through the ROM (one of A19-A23 stuck low looks like this)\n");
        return;
    }
    if (t.head[0] == 0x1114) {                                   /* 512 KB: the halves differ, at almost every word */
        for (k = 0, same = 0; k < 32; k++) same += AmigaRead16(ROM + 0x100 + k * 0x1F02u) == AmigaRead16(ROM + 0x40000 + 0x100 + k * 0x1F02u);
        if (same == 32) Bad("A18 stuck: the two halves of the 512 KB Kickstart read the same");
    }
    if (t.probe_hits == t.probe_tries) Bad("A20 stuck high: the Kickstart's first words are also at $%06lX", (unsigned long)A20_PROBE);
    else if (t.probe_hits) Bad("A20 floats: the Kickstart's first words at $%06lX on %d of %d tries", (unsigned long)A20_PROBE, t.probe_hits, t.probe_tries);
    else if (t.unnamed || (t.rom_odd && !errors))                 /* nothing named: say what was seen */
        Bad("an address line floats, but no single line could be named (chip RAM wrong on some passes%s; the Kickstart's first words wrong on %d of %d reads)",
            t.other ? ", another cell answering" : "", t.rom_odd, ROUNDS);
}

/* IPL0-2: Paula's level for one interrupt bit at a time. */
static void IPLTest(void) {
    static const uint8_t bit_of_level[7] = { 0, 2, 3, 4, 7, 11, 13 };    /* SOFT, PORTS, COPER, AUD0, RBF, EXTER: levels 1-6 */
    unsigned got, l, b, pass, ones = 0, zeros = 0, wrong = 0, seen[7];

    for (pass = 0; pass < 4; pass++)
        for (l = 0; l < 7; l++) {
            AmigaWrite16(INTENA, 0x7FFF);
            AmigaWrite16(INTREQ, 0x7FFF);
            if (l) { AmigaWrite16(INTENA, 0xC000 | 1u << bit_of_level[l]); AmigaWrite16(INTREQ, 0x8000 | 1u << bit_of_level[l]); }
            WaitUSDMTimer(20);
            got = GetIPL();
            if (!pass) seen[l] = got;
            ones |= got; zeros |= ~got & 7;
            wrong |= got ^ l;
        }
    AmigaWrite16(INTENA, 0x7FFF);
    AmigaWrite16(INTREQ, 0x7FFF);
    for (b = 0; b < 3; b++) {
        if (!(ones >> b & 1)) Bad("IPL%u stuck inactive (high) or open: the board never sees it", b);
        else if (!(zeros >> b & 1)) Bad("IPL%u stuck active (low)", b);
        else if (wrong >> b & 1) Bad("IPL%u wrong at some levels (shorted to another IPL line, or floating)", b);
    }
    if (wrong) printf("  (levels 0-6 read as %u %u %u %u %u %u %u)\n", seen[0], seen[1], seen[2], seen[3], seen[4], seen[5], seen[6]);
}

int AmigaBusTest(void) {
    static uint16_t saved[LINES + 1];
    uint16_t saved_data, saved_spare;
    int n, round, top = 0, before;

    errors = 0;
    DisableRomOverlay();
    if ((AmigaRead8(CIAA_DDRA) & 3) != 3 || (AmigaRead8(CIAA_PRA) & 1) != 0) {
        Bad("the CIA-A does not take writes (VPA, E, or an address line it decodes): DDRA %02X PRA %02X; the other tests need the ROM overlay off", AmigaRead8(CIAA_DDRA), AmigaRead8(CIAA_PRA));
        EnableRomOverlay();
        return -1;
    }
    memset(&t, 0, sizeof t);
    saved_data = AmigaRead16(DATA_WORD); saved_spare = AmigaRead16(SPARE_WORD);
    for (n = LINES; n >= 0; n--) saved[n] = AmigaRead16(Offset(n));
    for (n = 0; n < 4; n++) t.head[n] = AmigaRead16(ROM + 2 * n);
    if (t.head[0] != 0x1111 && t.head[0] != 0x1114) t.head[0] = 0;

    for (round = 0; round < ROUNDS; round++) {
        DataRound();
        AddressPass(round);
        FloatRound(round);
        RomRound(round);
        WaitUSDMTimer(ROUND_GAP_US);
    }
    for (n = LINES; n >= 0; n--) AmigaWrite16(Offset(n), saved[n]);      /* $0 last: where the higher offsets end up when they share it */
    AmigaWrite16(SPARE_WORD, saved_spare);

    if (!DataVerdict()) {
        before = errors;
        top = AddressVerdict();
        RomVerdict();
        if (errors == before) {                                  /* no address line to blame: bits that flipped are data lines that float */
            for (n = 0; n < 16; n++) if (t.flips[n]) Bad("D%d floats: wrong in %u of %u reads", n, t.flips[n], t.flip_reads);
        } else {
            for (n = 0; n < 16 && !t.flips[n]; n++) ;
            if (n < 16) printf("  (a few data reads were wrong too: the line above explains them)\n");
        }
        if (!errors && t.whole) Bad("the test word read wrong as a whole on %u of %u reads, yet no address line was found: a strobe, or a line that floats rarely", t.whole, t.data_reads);
        if (!errors) IPLTest();
        else printf("  (the interrupt lines are tested once the lines above are right)\n");
    }
    AmigaWrite16(DATA_WORD, saved_data);
    EnableRomOverlay();
    if (errors) return -1;
    info("Amiga bus: D0-D15, UDS, LDS, A1-A%d (%u KB of chip RAM), the ROM's lines, IPL0-2 and the CIA ok\n", top, 1u << (top + 1 - 10));
    return 0;
}
