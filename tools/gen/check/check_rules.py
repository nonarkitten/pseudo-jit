#!/usr/bin/env python3
"""The hard rules for emitted code (CLAUDE.md), checked on every generated sequence (leader + body, cached and
interpretive path):

- ends in a branch or a return (b / bx), never falls off the end
- no `bl` (no call that is not a tail branch). (push/pop cannot be checked: A7 *is* the real SP, so a guest
(A7)+ load is a `pop`.)
- no udiv / sdiv (Cortex-A8 has no hardware divide)
- no literal-pool load (`ldr rX, [pc, #n]`): after `setend be` it would read byte-swapped
- r4 is the guest PC in interpretive mode, so the cached path (the leader, which runs in both worlds, and the
DR body) never touches it
- nothing writes pc except `b` / `bx`   (the MOVEM per-mask guard's `add pc, pc, ...` lives in the table, not in a sequence)
"""

import sys, os, re

sys.path.insert(0, os.path.dirname(__file__))
sys.path[:0] = [os.path.join(os.path.dirname(os.path.abspath(__file__)), p) for p in ("..", "../..")]
import capstone
from gen import Gen, read_offsets
from arm32 import *

md = capstone.Cs(capstone.CS_ARCH_ARM, capstone.CS_MODE_ARM)
md.detail = True
from capstone.arm import ARM_REG_R4, ARM_REG_PC

g = Gen(read_offsets())
bad = {}


def flag(op, why, w=None):
    bad.setdefault(why, []).append(op)


def touches_r4(ins):
    return (
        any(ins.reg_name(r) == "r4" for r in ins.regs_read + ins.regs_write)
        or "r4" in ins.op_str.replace("r4,", "r4 ").split()
    )


n = 0
for op in range(65536):
    for interp in (False, True):
        seq = g.generate(op, interp=interp)
        n += 1
        last = seq[-1]
        if not (
            isinstance(last, tuple)
            or (isinstance(last, int) and (last & 0x0FFFFFF0 == 0x012FFF10 or last >> 25 & 7 == 5))
        ):
            flag(op, "does not end in a branch")
        for i, w in enumerate(seq):
            if isinstance(w, tuple):
                if w[0] == "bl":
                    flag(op, "bl")
                continue
            ins = next(md.disasm(w.to_bytes(4, "little"), 0), None)
            if ins is None:
                flag(op, "undecodable word")
                continue
            mn = ins.mnemonic
            if mn.startswith(("udiv", "sdiv")):
                flag(op, "hardware divide")
            if mn.startswith("ldr") and ins.op_str.replace(" ", "").find("[pc") >= 0:
                flag(op, "literal-pool load")
            if w >> 28 != 0xF and (w >> 25) & 7 == 5 and (w >> 24) & 1:
                flag(op, "bl")
            if not interp and not g.pin and touches_r4(ins):
                flag(op, "the cached path touches r4")  # (with the pinned D0, r4 is D0)
for why, ops in sorted(bad.items()):
    print(f"{why}: {len(ops)} sequences, e.g. {', '.join('%04X' % o for o in sorted(set(ops))[:6])}")
print(
    f"{n} sequences checked"
    + ("" if not bad else f"; {sum(len(set(v)) for v in bad.values())} opcodes break a rule")
)
sys.exit(1 if bad else 0)
