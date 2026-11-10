#!/usr/bin/env python3
"""Where a program's cycles go, by opcode: the per-opcode cached cost measured on the board
(tests/bbb/bbb_opcost.c, `[COST] OOOO cycles*100 ARM*100 mispredicts*100`)
weighted by a dynamic profile (tools/data/profiles/*.hist: `XXXX count per run`), with the generated sequence
length for each, ranked by their product.

   .venv/bin/python tools/study/profile_cost.py tools/data/profiles/dhrystone.hist run.log [measured cycles
   per run]

Control flow (Bcc, DBF, JSR, RTS, ...) has no per-opcode case: it is priced from tests/bbb/bbb_branch.c (see
CONTROL below) and marked with a *.
"""

import sys, re, collections

sys.path.insert(0, "tools/gen")
import capstone

md = capstone.Cs(capstone.CS_ARCH_M68K, capstone.CS_MODE_M68K_000)
# cycles per execution for the opcodes with no case: Bcc taken / not (an inline bcc: ~2.4 taken, ~1 not), DBF
# loop iteration, linked call + return pair (split), unlinked costs
CONTROL = {"bcc": 2.0, "bra": 2.4, "dbf": 5.5, "jsr": 14.0, "bsr": 14.0, "rts": 10.0, "jmp": 4.0}


def mnem(op):
    i = next(md.disasm(op.to_bytes(2, "big") + bytes(10), 0), None)
    return f"{i.mnemonic} {i.op_str}".strip() if i else "?"


def control_kind(op):
    if op == 0x4E75:
        return "rts"
    if op & 0xFFC0 == 0x4E80:
        return "jsr"
    if op & 0xFFC0 == 0x4EC0:
        return "jmp"
    if op & 0xFF00 == 0x6100:
        return "bsr"
    if op & 0xFF00 == 0x6000:
        return "bra"
    if op & 0xF000 == 0x6000:
        return "bcc"
    if op & 0xF0F8 == 0x50C8:
        return "dbf"
    return None


hist = {}
for l in open(sys.argv[1]):
    m = re.match(r"\s*([0-9A-Fa-f]{4})\s+(\d+)", l)
    if m:
        hist[int(m.group(1), 16)] = int(m.group(2))
cost = {}
for l in open(sys.argv[2], errors="replace"):
    m = re.match(r"\[COST\] ([0-9A-F]{4}) (-?\d+) (-?\d+) (-?\d+)", l)
    if m:
        cost[int(m.group(1), 16)] = (int(m.group(2)) / 100, int(m.group(3)) / 100, int(m.group(4)) / 100)
measured = float(sys.argv[3]) if len(sys.argv) > 3 else None
total_n = sum(hist.values())
rows = []
guess = 0
for op, n in hist.items():
    if op in cost:
        c, ins, mis = cost[op]
        star = " "
    else:
        k = control_kind(op)
        if k is None:
            continue
        c, ins, mis = CONTROL[k], 0, 0
        star = "*"
    rows.append((n * c, op, n, c, ins, mis, star))
covered = sum(r[2] for r in rows)
pred = sum(r[0] for r in rows)
print(
    f"{total_n} instructions per run in the profile; {covered} priced ({100 * covered / total_n:.0f}%); predicted {pred:.0f} cycles per run"
    + (f" (measured {measured:.0f}, {100 * pred / measured:.0f}%)" if measured else "")
)
print(f"{'opcode':6} {'68K':28} {'n/run':>5} {'cyc':>6} {'ARM':>5} {'n*cyc':>7} {'share':>6} {'cum':>6}")
cum = 0
for w, op, n, c, ins, mis, star in sorted(rows, reverse=True)[
    : int(sys.argv[4]) if len(sys.argv) > 4 else 30
]:
    cum += w
    print(
        f"{op:04X}{star}  {mnem(op):28} {n:5d} {c:6.2f} {ins:5.1f} {w:7.0f} {100 * w / pred:5.1f}% {100 * cum / pred:5.1f}%"
    )
