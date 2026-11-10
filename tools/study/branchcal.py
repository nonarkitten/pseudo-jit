#!/usr/bin/env python3
"""The structural calibration: the twelve control-flow loops of tests/bbb/bbb_branch.c, run through the block
runtime model (tools/study/blockrun.py) and compared with the
board's cycles / ARM instructions per iteration (`make BLK=1 ... bbb-bbb_branch`,
tools/study/branch_board.txt). A tiny control-flow emulator (just the opcodes these loops
use) produces each trace.
  .venv/bin/python tools/study/branchcal.py [--set=NAME=value ...]"""

import sys, os, struct

sys.path.insert(0, os.path.dirname(__file__))
sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))
sys.path[:0] = [os.path.join(os.path.dirname(os.path.abspath(__file__)), p) for p in ("../gen", "..")]
import cyclemodel, blockrun as br, blockasm as ba
from gen import Gen, read_offsets

B = 0x80000
PROGS = [  # name, words, a2_hw, jsr (at_hw, target_hw)
    ("SUBQ.L + BNE.S (loop crosses a line)", [0x2A3C, 0, 0, 0x5385, 0x66FC, 0x4E4F], 0, None),
    ("DBF D5 (one slot, loops on itself)", [0x3A3C, 0, 0x51CD, 0xFFFE, 0x4E4F], 0, None),
    ("BSR.S + RTS + SUBQ + BNE.S", [0x2A3C, 0, 0, 0x6106, 0x5385, 0x66FA, 0x4E4F, 0x4E75], 0, None),
    (
        "BSR.W + RTS (callee 3 lines away)",
        [0x2A3C, 0, 0, 0x6100, 0x0014, 0x5385, 0x66F8, 0x4E4F, 0, 0, 0, 0, 0, 0, 0x4E75],
        0,
        None,
    ),
    (
        "JSR abs.L + RTS (callee 4 lines away)",
        [0x2A3C, 0, 0, 0x4EB9, 0, 0, 0x5385, 0x66F6, 0x4E4F, 0, 0, 0, 0, 0, 0, 0, 0x4E75],
        0,
        (3, 16),
    ),
    (
        "JSR (A2) + RTS (callee 4 lines away)",
        [0x2A3C, 0, 0, 0x4E92, 0x5385, 0x66FA, 0x4E4F, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x4E75],
        16,
        None,
    ),
    (
        "JSR 8(A2) + RTS (callee 4 lines away)",
        [0x2A3C, 0, 0, 0x4EAA, 0x0008, 0x5385, 0x66F8, 0x4E4F, 0, 0, 0, 0, 0, 0, 0, 0, 0x4E75],
        12,
        None,
    ),
    (
        "JMP (A2) forward, SUBQ + BNE.S back to it",
        [0x2A3C, 0, 0, 0x4ED2, 0x4E71, 0x4E71, 0x5385, 0x66F6, 0x4E4F],
        6,
        None,
    ),
    (
        "BRA.S forward, SUBQ + BNE.S back to the BRA",
        [0x2A3C, 0, 0, 0x6004, 0x4E71, 0x4E71, 0x5385, 0x66F6, 0x4E4F],
        0,
        None,
    ),
    ("strcpy: MOVE.B (A1)+,(A0)+ ; DBF D5", [0x3A3C, 0, 0x10D9, 0x51CD, 0xFFFC, 0x4E4F], 0, None),
    (
        "BRA.S forward over a slot, then SUBQ + BNE.S",
        [0x2A3C, 0, 0, 0x6002, 0x4E71, 0x5385, 0x66F8, 0x4E4F],
        0,
        None,
    ),
    (
        "NOP x4 + SUBQ + BNE.S (loop is 6 halfwords)",
        [0x2A3C, 0, 0, 0x4E71, 0x4E71, 0x4E71, 0x4E71, 0x5385, 0x66F4, 0x4E4F],
        0,
        None,
    ),
]


def sx(v, b):
    return v - (1 << b) if v >> (b - 1) else v


def trace(words, a2_hw, jsr, n):
    w = list(words)
    if w[0] == 0x2A3C:
        w[1], w[2] = n >> 16, n & 0xFFFF
    else:
        w[1] = n - 1
    if jsr:
        w[jsr[0] + 1], w[jsr[0] + 2] = (B + 2 * jsr[1]) >> 16, (B + 2 * jsr[1]) & 0xFFFF
    img = b"".join(struct.pack(">H", x) for x in w)
    mem = ba.ImageMem(B, img)
    hw = lambda a: (a - B) // 2
    pc = B
    D5 = 0
    Z = False
    stack = []
    a2 = B + 2 * a2_hw
    out = []
    for _ in range(10 * n + 50):
        op = w[hw(pc)]
        out.append((pc, op))
        if op == 0x2A3C:
            D5 = w[hw(pc) + 1] << 16 | w[hw(pc) + 2]
            pc += 6
        elif op == 0x3A3C:
            D5 = w[hw(pc) + 1]
            pc += 4
        elif op == 0x5385:
            D5 -= 1
            Z = D5 == 0
            pc += 2
        elif op >> 8 == 0x66:
            if not Z:
                pc = pc + 2 + sx(op & 0xFF, 8)
            else:
                pc += 2
        elif op == 0x51CD:
            D5 = (D5 - 1) & 0xFFFF
            if D5 != 0xFFFF:
                pc = pc + 2 + sx(w[hw(pc) + 1], 16)
            else:
                pc += 4
        elif op >> 8 == 0x60:
            pc = pc + 2 + sx(op & 0xFF, 8)
        elif op == 0x6100:
            stack.append(pc + 4)
            pc = pc + 2 + sx(w[hw(pc) + 1], 16)
        elif op >> 8 == 0x61:
            stack.append(pc + 2)
            pc = pc + 2 + sx(op & 0xFF, 8)
        elif op == 0x4EB9:
            stack.append(pc + 6)
            pc = w[hw(pc) + 1] << 16 | w[hw(pc) + 2]
        elif op == 0x4E92:
            stack.append(pc + 2)
            pc = a2
        elif op == 0x4EAA:
            stack.append(pc + 4)
            pc = a2 + sx(w[hw(pc) + 1], 16)
        elif op == 0x4ED2:
            pc = a2
        elif op == 0x4E75:
            pc = stack.pop()
        elif op == 0x4E71 or op == 0x10D9:
            pc += 2
        elif op == 0x4E4F:
            break
        else:
            raise ValueError(hex(op))
    return mem, out


def board():
    res = {}
    for l in open(os.path.join(os.path.dirname(__file__), "branch_board.txt")):
        if not l.startswith("[BR] cached"):
            continue
        name = l[len("[BR] cached  ") :].strip()
        for pn, _, _, _ in PROGS:
            if name.startswith(pn):
                nums = name[len(pn) :].split()
                res[pn] = (float(nums[0]), float(nums[2]), float(nums[5]))
    return res


if __name__ == "__main__":
    for kv in sys.argv[1:]:
        if kv.startswith("--set="):
            k, v = kv[6:].split("=")
            setattr(cyclemodel, k, int(v))
            cyclemodel._cache.clear()
    g = Gen(read_offsets())
    bd = board()
    tot_e = 0
    n_e = 0
    print("%-48s %8s %8s %6s | %7s %7s" % ("", "board", "model", "err", "b.inst", "m.inst"))
    for name, words, a2_hw, jsr in PROGS:
        res = []
        for n in (300, 600):
            mem, tr = trace(words, a2_hw, jsr, n)
            rt = br.Runtime(mem, g, {})
            sim, ninstr, stats = br.replay(rt, tr, 0, {})
            res.append((sim.cycle, ninstr))
        cyc = (res[1][0] - res[0][0]) / 300.0
        ins = (res[1][1] - res[0][1]) / 300.0
        b = bd.get(name)
        if b:
            e = (cyc - b[0]) / b[0]
            tot_e += abs(e)
            n_e += 1
        print(
            "%-48s %8.2f %8.2f %+5.0f%% | %7.2f %7.2f"
            % (name, b[0] if b else 0, cyc, 100 * e if b else 0, b[1] if b else 0, ins)
        )
    print("mean |error| %.1f%%" % (100 * tot_e / max(1, n_e)))
