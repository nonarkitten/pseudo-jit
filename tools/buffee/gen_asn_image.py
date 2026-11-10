#!/usr/bin/env python3
"""Generates bootloader/greenpak/asn_image.h: the GreenPAK register image of the design in which the GreenPAK drives ASn: the programmed design (greenpak/buffee4.hex) plus the
netlist changes below. The bootloader writes it into the GreenPAK RAM at every boot (hal/gpak.c), so the design that runs is always the one in this build.
Also writes the same image as Intel hex (default bootloader/greenpak/buffee4-flat.hex: import it into GreenPAK Designer; it holds the phase-A parity table 0xEA, the bootloader
picks 0xEA / 0xBA per boot).
  gen_asn_image.py [src.hex [out.h [out.hex]]]"""

import sys

src = sys.argv[1] if len(sys.argv) > 1 else "bootloader/greenpak/buffee4.hex"
out = sys.argv[2] if len(sys.argv) > 2 else "bootloader/greenpak/asn_image.h"
out_hex = sys.argv[3] if len(sys.argv) > 3 else "bootloader/greenpak/buffee4-flat.hex"
d = []
for l in open(src).read().split():
    n = int(l[1:3], 16)
    d += [int(l[9 + 2 * i : 11 + 2 * i], 16) for i in range(n)]
d += [0] * (256 - len(d))
new = list(d)


def setf(n, v):  # matrix output n <- matrix input v (6 bits at bit 6n)
    for k in range(6):
        bit = 6 * n + k
        r, sh = divmod(bit, 8)
        new[r] = (new[r] & ~(1 << sh)) | (((v >> k) & 1) << sh)


# X = OEn & WEn (LUT9, multi-function block 3, LUT mode)
setf(40, 12)
setf(41, 11)
new[0xB1] = 0x88
# UDS / LDS strobes = BEn | ASn_out: the 2-LUTs take the ASn_out flop directly ('flat'): every LUT stage between the flop and the pin costs ~17 ns, and the blitter wedge
# grows with that delay. The strobes end with ASn, as a 68000's do.
setf(0, 18)
setf(6, 18)
# UDS / LDS strobes: the GPMC data lanes are wired swapped to the Amiga's (a byte at an even address leaves on
# GPMC D7:D0 = Amiga D15:D8), so BEn0 must raise UDS and BEn1 LDS:
# with BEn0 on the LDS cell a byte write strobed the lane that carried no data (0xFF came back from the
# pull-ups on the other lane)
setf(1, 10)
setf(7, 9)
# ASn_out = DFF3 on CLK7En (the clock echo inverted), D = LUT10 (the parity gate), init high, Q output, nRST
# tied high
setf(8, 1)
setf(9, 43)
setf(10, 63)
new[0x94] = 0x10
new[0x9A] |= 0x10
# parity gate: P = DFF6 toggle flop (QB fed back, nRST tied high); LUT10 (block 4, LUT mode) = X | (P &
# ASn_out); 0xBA = the other phase
new[0x97] = 0x40
setf(18, 21)
setf(19, 63)
setf(43, 42)
setf(44, 21)
setf(45, 18)
new[0xB5] = 0xEA
# CSp from ASn_out, WAIT0 = DONE | X, ARM = DFF4 with D = VDD (one stage), pipe delay tap 0, pin 13 = DTACKn
# echo for the PRU
setf(37, 18)
setf(2, 42)
setf(12, 63)
new[0x9C] = 0x00
setf(78, 6)
# Release chain (chosen with tools/buffee/shotgun.py): MF1 (MULTFUNC_8BIT_1, 0xA7 / 0xA8) as a 3-bit LUT computing
# IN0 xor IN1 = CLK7En xor CLK7E (matrix 15 and 1), a short pulse on BOTH CLK7 edges at matrix 40;
# DFF5 (the first release flop, clock OUT14) is clocked by it,
# and the pipe delay is bypassed (LUT5's IN0, OUT23, takes ARM, matrix 19, instead of the pipe's out0, matrix
# 24): ASn low 5 -> ~4 clocks, bus cycle 8 -> 6 CLK7.
new[0xA7] = 0x00
new[0xA8] = 0x66
setf(34, 15)
setf(35, 1)
setf(14, 1)
setf(23, 19)
# DONE-driven end of write cycles: ASn_out and the strobes end on DONE (DFF5 = D5q) for
# WRITES, and WAIT0 lets the GPMC go only AFTER ASn_out has risen, so R/W (= the
# GPMC's WEn) rises after the strobes like a 68000's and a write is ~3 clocks long; reads still end on the
# GPMC's OE negation (it has to capture the data first).
#   E = X | (!WEn & (D5q | WRrel))   LUT4_0 (0xA1 / 0xA2 = 0xBBBA, IN0..IN3 = OUT30..33 <- X, WEn, D5q, WRrel) -> matrix 39 -> ASn_out's gate (OUT43)
# WRrel = DFF7 clocked by ASn_out rising, D = D5q, reset while WEn is high (nRST = !WEn from MF6 = LUT3_12,
# 0xBE = 0x55, matrix 45): set once ASn_out has risen with DONE true, drops with WEn
#   S = (D5q & WEn) | WRrel          LUT3_6 (pipe block in LUT mode: 0x9D bit 1 = 0, truth 0x9C = 0xF8, IN0..IN2 = OUT26..28 <- D5q, WEn, WRrel) -> matrix 24; WAIT0 = X | S (OUT3)
# (flops with plain resets: a Q5 held by a feedback reset never released when a write was followed by a read, X
# stayed low across them.)
# 0x9C is not the pipe delay's tap: ASNCalibrate's slow fallback must not write it (hal/gpmc.c).
new[0xA1] = 0xBA
new[0xA2] = 0xBB
setf(30, 42)
setf(31, 11)
setf(32, 20)
setf(33, 22)
# CIA reads (6800-style cycles; the 8520's D4 drives a 1 weakly and only while E is high): releasing WAIT
# through ARM -> LUT5 -> DFF5 sampled the data up to a clock after the PRU's TERMn,
# when D4 was already fading (CIAA DDRB bit 4 read 0 in ~half of the reads). For READS WAIT also releases
# straight on TERMn: V = D5q | !TERMn (MF7 = LUT3_13: 0xC3 = 0xBB,
# IN0 = D5q, IN1 = TERMn, matrix 46) replaces D5q as S's first input. Writes still go through D5q / WRrel (a
# CIA write needs its data and CS until E falls).
new[0xC3] = 0xBB
setf(52, 20)
setf(53, 8)
new[0x9D] = 0x18
new[0x9C] = 0xF8
setf(26, 46)
setf(27, 11)
setf(28, 22)
new[0xBE] = 0x55
setf(49, 11)
setf(20, 18)
setf(21, 20)
setf(22, 45)
setf(3, 24)
setf(43, 39)
# 68000 BUS TIMING, measured against the 68K CLK pin: ASn is asserted just after the rising edge of the clock (a 68000 asserts AS within 55 ns of the clock going HIGH, S2),
# and the release does not follow a DTACK that fast-RAM and ROM cycles assert at once (Gary), because a 68000 ignores DTACK until S4.
# (1) ASn_out (DFF3) is clocked by the raw echo (matrix 1): ASn and the strobes rise and fall just after the 68K clock's rising edge;
# (2) the DTACK mask: the programmable delay cell (both-edge delay, 125 ns setting = 168 ns at 3.3 V; input OUT56 <- CSp (41), output matrix 28, register 0x9D bits 4:3 = 11) delays
#     the cycle-active signal CSp, and ARM (DFF4, OUT11) is clocked by the delayed edge: the release chain is not armed before ~S4;
# (3) the first release flop DFF5 (OUT14) is clocked by the raw echo (matrix 1, rising edges only), so the acknowledge lands on an edge of a fixed phase (S5/S6) and the data is
#     taken at the end of S6 for every slave.
# The parity table (0xB5) is chosen at boot by ASNCalibrate by TIMING the reads (both tables are correct, one costs 8 CLK7 per cycle instead of 6).
setf(11, 28)
setf(56, 41)
# phase class flop (block 5 as a DFF, B9 = 0x08): D = DTACKn sampled when ARM rises, read back at 0x79 bit 4
setf(46, 19)
setf(47, 63)
setf(48, 6)
new[0xB9] = 0x08
new[0xBA] = 0x00
# pin 20 (IO14) drives ASn_out, push-pull (0x73 is already 'Schmitt input, push-pull 1x, no pull'); pull-ups
# (100 kOhm) on OEn / WEn (IO13 / IO12)
setf(90, 18)
setf(91, 63)
new[0x71] |= 0x60
new[0x72] |= 0x60
# 68K function codes: FC0 (pin 5, IO3) = constant 1 (matrix input 63), FC1 (pin 10, IO6, a GPO) released (output enable off: the ARM drives it from GPIO2_9);
# FC2 is the ARM's GPIO2_10 and has no GreenPAK pin (pin 13 is the DTACK echo for the PRU: its net must be cut from the 68K header)
setf(71, 63)
new[0x6A] &= 0x7F
regs_out = out.replace("asn_image.h", "asn_regs.h")
with open(regs_out, "w") as f:
    f.write("/* generated by tools/buffee/gen_asn_image.py: do not edit */\n")
    f.write(
        "/* matrix input 44 (0x79 bit 4) is the phase class flop; 0xB5 holds the parity truth table (0xEA phase A, 0xBA phase B, 0xAA no gate) */\n"
    )
    f.write(
        "#define ASN_REG_PHASE 0xB5\n#define ASN_REG_TAP 0x9C\n#define ASN_REG_OE 0x44\n#define ASN_PHASE_A 0xEA\n#define ASN_PHASE_B 0xBA\n#define ASN_PHASE_OFF 0xAA\n"
    )
with open(out, "w") as f:
    f.write(
        '/* generated by tools/buffee/gen_asn_image.py from greenpak/buffee4.hex: do not edit */\n#include "asn_regs.h"\n'
    )
    f.write("static const uint8_t asn_image[0xC8] = {\n")
    for i in range(0, 0xC8, 16):
        f.write("    " + ", ".join("0x%02X" % new[j] for j in range(i, min(i + 16, 0xC8))) + ",\n")
    f.write("};\n")
with open(
    out_hex, "w"
) as f:  # Intel hex, 16 bytes per record, the 256 register bytes (same layout as greenpak/buffee4.hex)
    for a in range(0, 256, 16):
        rec = [16, a >> 8, a & 0xFF, 0] + new[a : a + 16]
        f.write(":" + "".join("%02X" % x for x in rec) + "%02X\n" % ((-sum(rec)) & 0xFF))
    f.write(":00000001FF\n")
print("wrote", out, " changed registers:", " ".join("%02X" % i for i in range(0xC8) if new[i] != d[i]))
