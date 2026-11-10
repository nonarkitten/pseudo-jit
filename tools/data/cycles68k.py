#!/usr/bin/env python3
"""Estimated MC68000 cycles for an instruction trace (8-byte big-endian entries: PC, opcode, as written by
pjit_prefetch_trace; see tests/qemu/smoke_dhrystone_trace.c). An approximation of the Motorola timing tables:
the effective-address costs are exact, instruction base times are exact for the common forms, shifts and
MOVEM use typical counts (the trace has the opcode, not the extension words), MULU/MULS/DIVU/DIVS use
averages.

  tools/data/cycles68k.py a.bin [b.bin RUNS_DIFF]   -> cycles (and per-run cycles from the difference)
"""

import sys, collections

EA_BW = {0: 0, 1: 0, 2: 4, 3: 4, 4: 6, 5: 8, 6: 10}
EA_L = {0: 0, 1: 0, 2: 8, 3: 8, 4: 10, 5: 12, 6: 14}


def ea(mode, reg, long):
    if mode == 7:
        c = {0: (8, 12), 1: (12, 16), 2: (8, 12), 3: (10, 14), 4: (4, 8)}.get(reg, (8, 12))
        return c[1] if long else c[0]
    return (EA_L if long else EA_BW)[mode]


def dst_write(mode, reg, long):  # MOVE's destination, as a write
    if mode == 7:
        c = {0: (8, 12), 1: (12, 16)}.get(reg, (8, 12))
        return c[1] if long else c[0]
    c = {0: 0, 1: 0, 2: 4, 3: 4, 4: 4, 5: 8, 6: 10}[mode]
    return c + (4 if long and mode >= 2 else 0)


def cycles(pc, op, next_pc):
    t = op >> 12
    mode = (op >> 3) & 7
    reg = op & 7
    size = (op >> 6) & 3
    long = size == 2
    taken = next_pc is not None and next_pc != pc + (4 if (op & 0xFF) == 0 else 2)
    if t in (1, 2, 3):  # MOVE / MOVEA
        L = t == 2
        return 4 + ea(mode, reg, L) + dst_write((op >> 6) & 7, (op >> 9) & 7, L), "MOVE"
    if t == 7:
        return 4, "MOVEQ"
    if t == 6:  # Bcc / BRA / BSR
        if (op >> 8) == 0x61:
            return 18, "BSR"
        if taken or (op >> 8) == 0x60:
            return 10, "Bcc taken"
        return (12 if (op & 0xFF) == 0 else 8), "Bcc not taken"
    if t == 5:
        if size == 3:
            if mode == 1:
                return (10 if taken else 14), "DBcc"
            return (4 if mode == 0 else 8 + ea(mode, reg, False)), "Scc"
        if mode == 1:
            return 8, "ADDQ An"
        if mode == 0:
            return (8 if long else 4), "ADDQ Dn"
        return (12 if long else 8) + ea(mode, reg, long), "ADDQ mem"
    if t == 0:
        if op & 0x0100 or (op & 0xFF00) == 0x0800:  # BTST/BCHG/BCLR/BSET
            imm = (op & 0x0100) == 0
            base = 4 if not imm else 8
            if mode == 0:
                return (6 if (op & 0xC0) == 0 else 8) + (4 if imm else 0), "BIT Dn"
            return base + ea(mode, reg, False) + (4 if (op & 0xC0) else 0), "BIT mem"
        if mode == 0:
            return (16 if long else 8), "IMM Dn"
        return (20 if long else 12) + ea(mode, reg, long), "IMM mem"
    if t == 4:
        if (op & 0xFFF8) in (0x4880, 0x48C0, 0x4840):
            return 4, "EXT/SWAP"
        if op & 0xFF80 in (0x4880, 0x4C80):
            return 12 + 8 * 6 + ea(mode, reg, True), "MOVEM"  # ~6 registers, long
        if (op & 0xF1C0) == 0x41C0:
            return (
                {0: 4, 2: 4, 5: 8, 6: 12}.get(mode, 8 if reg < 2 else 12)
                if mode != 7
                else (8 if reg < 2 else 12)
            ), "LEA"
        if (op & 0xFFC0) == 0x4E80:
            return {2: 16, 5: 18, 6: 22}.get(mode, 18 if reg == 2 else 20), "JSR"
        if (op & 0xFFC0) == 0x4EC0:
            return {2: 8, 5: 10, 6: 14}.get(mode, 10 if reg == 2 else 12), "JMP"
        if op == 0x4E75:
            return 16, "RTS"
        if (op & 0xFFF0) == 0x4E40:
            return 34, "TRAP"
        if (op & 0xFFF8) == 0x4E50:
            return 16, "LINK"
        if (op & 0xFFF8) == 0x4E58:
            return 12, "UNLK"
        if (op & 0xFFC0) == 0x4840:
            return 12 + ea(mode, reg, True), "PEA"
        if (op & 0xFF00) in (0x4200, 0x4400, 0x4600, 0x4000):  # CLR/NEG/NOT/NEGX
            if size == 3:
                return 12, "SR move"
            if mode == 0:
                return (6 if long else 4), "unary Dn"
            return (12 if long else 8) + ea(mode, reg, long), "unary mem"
        if (op & 0xFF00) == 0x4A00:
            return 4 + ea(mode, reg, long), "TST"
        return 8, "misc"
    if t in (8, 9, 0xB, 0xC, 0xD):
        opm = (op >> 6) & 7
        if t == 8 and opm == 3:
            return 136, "DIVU"
        if t == 8 and opm == 7:
            return 150, "DIVS"
        if t == 0xC and opm == 3:
            return 54, "MULU"
        if t == 0xC and opm == 7:
            return 54, "MULS"
        if opm in (3, 7) and t in (9, 0xB, 0xD):  # ADDA/SUBA/CMPA
            L = opm == 7
            return (
                (6 if L else 8) + ea(mode, reg, L) if not (L and mode < 2 or mode == 7 and reg == 4) else 8
            ), "ALU A"
        if t == 0xB and opm in (4, 5, 6) and mode == 1:
            return 12, "CMPM"
        L = opm in (2, 6)
        if opm < 3:  # <ea>,Dn
            return (6 if L else 4) + ea(mode, reg, L) + (
                2 if L and (mode < 2 or (mode == 7 and reg == 4)) else 0
            ), "ALU ea,Dn"
        if t == 0xB and mode == 0 and not L and opm in (4, 5):
            return 4, "EOR Dn"
        if mode == 0:
            return (8 if L else 4), "ALUX"
        return (12 if L else 8) + ea(mode, reg, L), "ALU Dn,ea"
    if t == 0xE:
        if size == 3:
            return 8 + ea(mode, reg, False), "shift mem"
        n = ((op >> 9) & 7) or 8 if not (op & 0x20) else 4
        return (8 if long else 6) + 2 * n, "shift"
    return 34, "exception"


def total(path):
    d = open(path, "rb").read()
    ent = [
        (int.from_bytes(d[i : i + 4], "big"), int.from_bytes(d[i + 4 : i + 8], "big") & 0xFFFF)
        for i in range(0, len(d) - 7, 8)
    ]
    tot = 0
    cls = collections.Counter()
    cnt = collections.Counter()
    for i, (pc, op) in enumerate(ent):
        c, k = cycles(pc, op, ent[i + 1][0] if i + 1 < len(ent) else None)
        tot += c
        cls[k] += c
        cnt[k] += 1
    return tot, len(ent), cls, cnt


if __name__ == "__main__":
    ta, na, ca, na_ = total(sys.argv[1])
    print(
        "%s: %d instructions, %d estimated 68000 cycles (%.2f per instruction)"
        % (sys.argv[1], na, ta, ta / na)
    )
    if len(sys.argv) > 2:
        tb, nb, cb, nb_ = total(sys.argv[2])
        runs = int(sys.argv[3])
        pi, pc_ = (nb - na) / runs, (tb - ta) / runs
        print(
            "per run: %.0f instructions, %.0f 68000 cycles (%.2f per instruction; at 7.09 MHz = %.1f us)"
            % (pi, pc_, pc_ / pi, pc_ / 7.09)
        )
        print("where the cycles go (per run):")
        for k, v in sorted(((k, (cb[k] - ca[k]) / runs) for k in cb), key=lambda kv: -kv[1])[:14]:
            print("  %-14s %7.0f cycles  %5.1f instructions" % (k, v, (nb_[k] - na_[k]) / runs))
