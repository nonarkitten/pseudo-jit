"""The generator: the ARM instruction sequences for every 68000 opcode, one method per emitter function, built from the same pieces the sequences are made of: effective-address
load / store, the ALU core with its flag fix-ups, the X flag. `Gen(offsets).generate(opcode)` returns the sequence for one opcode.

Conventions (the runtime's): r5 = the dispatch table with cpu_t just below it; A0-A7 = r6-r13; D0 = r4 (pin-d0, the default), D1-D7 big-endian words at r5 + CPU_D0 + 4n; r0-r3
scratch; r1 / r2 hold the source / destination extension value (an immediate, an address, a displacement + index) that the prefix of the translated code computed. ARM N Z V C are the
68K's N Z V C (C is the 68K's carry, so inverted after a subtract); X is in the GE flags, inverted. A packed EA is (size << 6) | (mode << 3) | register, size 0/1/2 = byte/word/long.
"""

import os
import re

import optab
from arm32 import *
from paths import asm_offsets_path
from emitters.base import Seq, X_LAZY, PIN_D0, FC_GPIO
from emitters.ea import EAMixin
from emitters.alu import AluMixin
from emitters.shift import ShiftMixin
from emitters.misc import MiscMixin
from emitters.xops import XopsMixin
from emitters.flow import FlowMixin
from emitters.sr import SrMixin
from emitters.complex import ComplexMixin


def read_offsets(path=None):
    """The cpu_t offsets (CPU_D0, CPU_SR, ...) that `make` read out of the compiler's output; the active
    build's unless a path is given.
    """
    path = path or asm_offsets_path()
    d = {}
    for l in open(path):
        m = re.match(r"\.equ (\w+), (-?\d+)", l)
        if m:
            d[m.group(1)] = int(m.group(2))
    return d


class Gen(EAMixin, AluMixin, ShiftMixin, MiscMixin, XopsMixin, FlowMixin, SrMixin, ComplexMixin):
    """Generates the ARM sequence for any 68000 opcode: `Gen(offsets).generate(opcode)`.  The emitters are the
    mixins; this class holds the
    state they share (the offsets of cpu_t, the winner table of optab_rows.py) and the dispatch."""

    interp = False

    # PJIT_SPLIT_LONG=1: a long access to guest memory through an effective address is two halfword accesses
    # (the Amiga window is Device memory, where a long at a 2-aligned address
    # faults and is emulated by the abort handler); data and address registers stay long operations
    split_long = os.environ.get("PJIT_SPLIT_LONG") == "1"

    # ALU ops through an address register (r2) and one routine per operation and size: measured +3.0% on Tiny
    # BASIC, 0 on Dhrystone, DRSR -42 KB: off
    split_alu = os.environ.get("PJIT_SPLIT_ALU") == "1"

    # True: the flat forms the block runtime's forwarder and scheduler were built for (D-slot accesses by
    # address in the word), not the split ones
    legacy = os.environ.get("PJIT_LEGACY") == "1"

    def __init__(self, offsets, lazy_x=None):
        self.lazy_x = X_LAZY if lazy_x is None else lazy_x  # see X_LAZY: the pure X producers do not save X
        self.pin = PIN_D0
        self.routines = {}
        self.natural = False
        self.o = offsets
        self.SCRATCH = offsets["CPU_SCRATCH"]
        self.D0 = offsets["CPU_D0"]
        self.XB = offsets["CPU_SR"] + 1
        self.winner = optab.load()
        self.table = {}
        for n in dir(self):
            if n.startswith("emit_"):
                self.table[n] = getattr(self, n)

    def generate(self, op, interp=False):
        """The sequence for an opcode, or None if this generator does not implement its emitter yet."""
        name = self.winner[op] or "emit_ILLEGAL"  # an opcode no row claims is illegal
        fn = self.table.get(name)
        self.interp = interp
        try:
            r = fn(op) if fn else None
            return self.coalesce_d0(r) if (self.pin and r is not None) else r
        finally:
            self.interp = False

    def coalesce_d0(self, seq):
        """The pinned D0 copied to a scratch register, operated on there and copied back (`mov rX, r4 ... mov r4,
        rX`) is operated on in place: both moves go and rX becomes r4
        in between. Exact when nothing between the moves names r4, rX is not named before the first move or
        after the last, and the sequence is straight-line (no branch, no
        svc, no symbol: every word is one regalloc.fields() knows) up to the closing `bx lr`."""
        if isinstance(seq, Seq) and (seq.split or getattr(seq, "shared", None)):
            return seq
        w = list(seq)
        if len(w) < 4 or isinstance(w[-1], tuple) or int(w[-1]) != 0xE12FFF1E:
            return seq
        import regalloc

        body = w[:-1]
        fl = [regalloc.fields(x) for x in body]
        if any(f is None for f in fl):
            return seq

        def regs(k):
            return {int(body[k]) >> sh & 15 for sh, _ in fl[k]}

        for i, x in enumerate(body):
            for rx in (0, 1, 2, 3):
                if int(x) != 0xE1A00004 | rx << 12:
                    continue  # mov rX, r4
                js = [j for j in range(len(body) - 1, i, -1) if int(body[j]) == 0xE1A04000 | rx]  # mov r4, rX
                if not js:
                    continue
                j = js[0]
                if any(rx in regs(k) for k in range(i)) or any(
                    rx in regs(k) for k in range(j + 1, len(body))
                ):
                    continue
                if any(4 in regs(k) for k in range(i + 1, j)):
                    continue
                out = body[:i]
                for k in range(i + 1, j):
                    x2 = int(body[k])
                    for sh, _ in fl[k]:
                        if x2 >> sh & 15 == rx:
                            x2 = x2 & ~(15 << sh) | 4 << sh
                    out.append(x2)
                out += body[j + 1 :] + [w[-1]]
                return self.coalesce_d0(Seq(out) if isinstance(seq, Seq) else out)
        return seq


# the ALU-family emitters, by name
for _n, _op in (("ADD", "add"), ("SUB", "sub"), ("AND", "and"), ("OR", "or"), ("EOR", "eor"), ("CMP", "cmp")):
    for _s in ("BW", "W", "L", "B"):
        setattr(Gen, f"emit_{_n}{_s}", (lambda o: lambda self, op: self._ea_alu(op, o))(_op))
for _n, _op in (("ADDA", "add"), ("SUBA", "sub"), ("CMPA", "cmp")):
    for _s in ("W", "L"):
        setattr(Gen, f"emit_{_n}{_s}", (lambda o: lambda self, op: self._areg_alu(op, o))(_op))
for _n, _op in (
    ("ADDI", "add"),
    ("SUBI", "sub"),
    ("ANDI", "and"),
    ("ORI", "or"),
    ("EORI", "eor"),
    ("CMPI", "cmp"),
):
    for _s in ("BW", "L"):
        setattr(Gen, f"emit_{_n}{_s}", (lambda o: lambda self, op: self._imd_alu(op, o))(_op))

for _n, _pair in (
    ("AS", ("asl", "asr")),
    ("LS", ("lsl", "lsr")),
    ("RO", ("rol", "ror")),
    ("ROX", ("roxl", "roxr")),
):
    for _s in ("dBW", "dL"):
        setattr(Gen, f"emit_{_n}{_s}", (lambda p: lambda self, op: self._reg_shift(op, p))(_pair))
    setattr(Gen, f"emit_{_n}d", (lambda p: lambda self, op: self._mem_shift(op, p))(_pair))

for _n, _op in (("CLR", "clr"), ("NEG", "neg"), ("NOT", "not")):
    for _s in ("BW", "L"):
        setattr(Gen, f"emit_{_n}{_s}", (lambda o: lambda self, op: self._unary(op, o))(_op))
