"""The X flag, the ALU core with the 68K flag fix-ups, and the MOVE / ADD / SUB / AND / OR / EOR / CMP families.

Part of `gen.Gen`; emitters/__init__.py lists the other groups.
"""

from arm32 import *
from emitters.base import Seq, X_LAZY


class AluMixin:
    """The X flag, the ALU core with the 68K flag fix-ups, and the MOVE / ADD / SUB / AND / OR / EOR / CMP
    families.
    """

    # ---- flags ----
    def save_x(self, exact=False):  # X := C, without reading the byte (its low nibble is never trusted)
        if self.lazy_x and not exact:
            return []  # lazy: pure X producers do not save it (see X_LAZY); ADDX & co keep theirs
        # X lives in the GE flags while the guest runs: GE[3:0] = 1111 when X is CLEAR ("GE = not X"). `sbc
        # r0,r0,r0` turns C into data (0 when C
        # is set, -1 when clear); `uadd8 r0,r0,r0` makes every byte lane carry exactly when that data is -1.
        # Neither touches N Z C V, and no ordinary
        # instruction touches GE, so X survives MOVE / CMP / logical ops with no memory round trip (measured
        # ~0.9 cycles against 2.0).
        # (r3, not r0: the result stays in r0 for the next instruction's load to be forwarded from)
        return [
            dp("sbc", 3, 3, ("reg", 3)),
            uadd8(3, 3, 3),
        ]

    @staticmethod
    def nz_clear_vc(r):
        return [dp("add", r, r, ("imm", 0), s=True)]

    @staticmethod
    def carry_fixup(regS, regD, invert=True):
        scratch = 0
        if scratch in (regS, regD):
            scratch = 1
        if scratch in (regS, regD):
            scratch = 3
        if invert:
            # N Z V as they are, C flipped, built from the condition codes: `mrs` serialises the Cortex-A8
            # pipeline (~9 cycles measured on
            # the BeagleBone, tests/bbb/bbb_micro.c); mov + four conditional orr + msr is ~7 cycles for the
            # whole fix-up against ~12.
            return [
                mov(scratch, ("imm", 0)),
                orr(scratch, scratch, ("imm", 0x80000000), cond="mi"),
                orr(scratch, scratch, ("imm", 0x40000000), cond="eq"),
                orr(scratch, scratch, ("imm", 0x20000000), cond="cc"),
                orr(scratch, scratch, ("imm", 0x10000000), cond="vs"),
                msr_f(scratch),
            ]
        return [mrs(scratch), dp("bic", scratch, scratch, ("imm", 0x20000000)), msr_f(scratch)]

    def alu(self, op, regS, regD, width, imm=None):
        """One ALU operation, regD = regD <op> regS (add sub cmp and or eor; the unary clr neg not tst extbw
        extwl swap; mulsw muluw).
        No trailing bx lr."""
        sets_flags = regD < 6
        w = []
        shl = 32 - width if width < 32 else 0
        if imm is not None:
            regS = -1  # an immediate source (ADDQ / SUBQ): op2 below is ('imm', imm << shl)

        def src(shift=0):
            return (
                ("imm", imm << shift)
                if imm is not None
                else (("reg", regS, "lsl", shift) if shift else ("reg", regS))
            )

        narrow = bool(shl) and sets_flags and op in ("add", "sub", "cmp", "neg")
        # the shifts of the narrow operands ride on the barrel shifter of the operation itself
        fold = narrow and op in ("add", "sub", "cmp") and regS != regD
        if narrow:
            if fold:
                if op == "add":
                    w.append(lsl(regD, regD, shl))
            else:
                if op != "neg":
                    w.append(lsl(3, regS, shl))
                    regS = 3
                w.append(lsl(regD, regD, shl))
        apply_flags = sets_flags
        logical = op in ("and", "or", "eor", "not", "clr")
        if op == "mulsw" or op == "muluw":
            ext = sxth if op == "mulsw" else uxth
            w += [ext(regD, regD), ext(regS, regS), mul(regD, regD, regS)]
            return w + (self.nz_clear_vc(regD) if sets_flags else [])
        if op in ("bchg", "bclr", "bset", "btst"):
            # only Z changes; N, V, C stay
            w += [
                mrs(3),
                dp("tst", 0, regD, ("reg", regS)),
                orr(3, 3, ("imm", 0x40000000), cond="eq"),
                dp("bic", 3, 3, ("imm", 0x40000000), cond="ne"),
                msr_f(3),
            ]
            if op != "btst":
                w.append(
                    {
                        "bchg": lambda: eor(regD, regD, ("reg", regS)),
                        "bclr": lambda: dp("bic", regD, regD, ("reg", regS)),
                        "bset": lambda: orr(regD, regD, ("reg", regS)),
                    }[op]()
                )
            return w
        if op == "swap":
            return [mov(regD, ("reg", regD, "ror", 16))] + ([] if not sets_flags else self.nz_clear_vc(regD))
        if op in ("extbw", "extwl"):
            return [(sxtb if op == "extbw" else sxth)(regD, regD)] + (
                self.nz_clear_vc(regD) if sets_flags else []
            )
        if self.natural and apply_flags and op == "cmp" and regS != regD:
            # the ARM flags in their natural polarity (C = no borrow: the 68K C is the opposite), for a
            # compare whose flags the next Bcc is the only reader of
            # (pjit/blk.s swaps CS / CC and uses the ARM HI / LS directly): no `mvn` / `mvns` pair around the
            # flag-setting add
            if fold:
                t = next(r for r in (0, 1, 3) if r not in (regS, regD))
                return w + [lsl(t, regD, shl), cmp_(t, ("reg", regS, "lsl", shl))]
            return w + [cmp_(regD, ("reg", regS))]
        if apply_flags and op in ("sub", "cmp", "neg") and (op == "neg" or regS != regD):
            # 68K subtract flags with no fix-up: a - b computed as ~(~a + b).  C of (~a + b) is (b > a), the 68K borrow; V is the same as a - b's
            # (t -> -t-1 maps the signed range onto itself); the closing `mvns` (no shift, so C and V stay)
            # puts the true result in N and Z.
            # Replaces `sub` + `mrs`/`eor`/`msr` (~12 cycles, mrs alone ~9 on the Cortex-A8,
            # tests/bbb/bbb_micro.c) with three 1-cycle ALU ops.
            if (
                op == "neg"
            ):  # 0 - d: ~0 + d = d - 1, and `subs d, d, #1` sets the same C and V as `adds d, d, #-1`
                w += [sub(regD, regD, ("imm", 1), s=True), mvn(regD, ("reg", regD), s=True)]
            else:
                t = regD if op == "sub" else next(r for r in (0, 1, 3) if r not in (regS, regD))
                if fold:
                    w += [
                        mvn(t, ("reg", regD, "lsl", shl)),
                        add(t, t, src(shl), s=True),
                        mvn(t, ("reg", t), s=True),
                    ]
                else:
                    w += [mvn(t, ("reg", regD)), add(t, t, src(), s=True), mvn(t, ("reg", t), s=True)]
            if narrow and op != "cmp":
                w.append(lsr(regD, regD, shl))
            return w
        word = {
            "add": lambda: add(regD, regD, src(shl if fold else 0), s=apply_flags),
            "sub": lambda: sub(regD, regD, src(), s=apply_flags),
            "cmp": lambda: cmp_(regD, ("reg", regS)),
            "and": lambda: and_(regD, regD, ("reg", regS)),
            "or": lambda: orr(regD, regD, ("reg", regS)),
            "eor": lambda: eor(regD, regD, ("reg", regS)),
            "clr": lambda: mov(regD, ("imm", 0)),
            "neg": lambda: dp("rsb", regD, regD, ("imm", 0), s=apply_flags),
            "not": lambda: mvn(regD, ("reg", regD)),
            "tst": lambda: add(regD, regD, ("imm", 0), s=apply_flags),
        }[op]()
        w.append(word)
        if narrow and op != "cmp":
            w.append(lsr(regD, regD, shl))
        if op == "cmp":
            return w + self.carry_fixup(regS, regD)
        if op in ("sub", "neg"):
            if apply_flags:
                w += self.carry_fixup(regS, regD)
            return w
        if apply_flags and logical:
            w += self.nz_clear_vc(regD)
        return w

    def _ea_alu_split(self, opcode, op, sEA, dEA):
        """ALU op between a data register and an <ea> in the target layout: the opcode's own words load the
        source into r1 and form the destination's address in r2; the routine
        (one per operation and size) loads the destination through r2, operates, stores through r2 and saves
        X.
        """
        s = self.ea_load(sEA, 1, 1, False)
        a = self.ea_addr(dEA, 2, 2)
        if s is None or a is None:
            return None
        own = list(s[0]) + ([mov(1, ("reg", s[1]))] if s[1] != 1 else []) + a
        if not 1 <= len(own) <= 3:
            return None
        width = 1 << (dEA >> 6)
        ld = {1: ldrsb, 2: ldrsh, 4: ldr}[width]
        st = {1: strb, 2: strh, 4: str_}[width]
        routine = (
            [ld(0, 2, 0)]
            + self.alu(op, 1, 0, 8 * width)
            + ([st(0, 2, 0)] if op != "cmp" else [])
            + (self.save_x() if op in ("add", "sub") else [])
            + [bx("lr")]
        )
        return Seq(own + routine, split=len(own))

    def _ea_alu(self, opcode, op):
        sEA = dEA = opcode & 0xC0
        if op == "cmp" or (op != "eor" and not (opcode & 0x100)):
            dEA |= (opcode & 0xE00) >> 9
            sEA |= opcode & 0x3F
        else:
            sEA |= (opcode & 0xE00) >> 9
            dEA |= opcode & 0x3F
        if self.split_alu and not self.legacy and (dEA & 0x38) != 0x08:
            r = self._ea_alu_split(opcode, op, sEA, dEA)
            if r is not None:
                return r
        s = self.ea_load(sEA, 1, 1, False, inplace=True)
        d = self.ea_load(dEA, 0, 2, True, inplace=True)
        if s is None or d is None:
            return None
        (ws, regS), (wd, regD) = s, d
        w = ws + wd + self.alu(op, regS, regD, 8 << ((opcode >> 6) & 3))
        if op != "cmp":
            w += self.ea_store(dEA, regD, 2, True)
        if regD < 6 and op in ("add", "sub"):
            w += self.save_x()
        return w + [bx("lr")]

    def _areg_alu(self, opcode, op):
        sEA = (0x40 if not (opcode & 0x100) else 0x80) | (opcode & 0x3F)
        dEA = 0x88 | ((opcode >> 9) & 7)
        s = self.ea_load(sEA, 1, 1, False, inplace=True)
        d = self.ea_load(dEA, 0, 2, True)
        if s is None or d is None:
            return None
        if op == "cmp" and self.natural:
            return s[0] + d[0] + [cmp_(d[1], ("reg", s[1])), bx("lr")]
        if (
            op == "cmp"
        ):  # CMPA sets the flags (ADDA and SUBA do not): the same three-word compare as CMP, ~(~a + b)
            t = next(r for r in (0, 1, 3) if r not in (s[1], d[1]))
            return (
                s[0]
                + d[0]
                + [
                    mvn(t, ("reg", d[1])),
                    add(t, t, ("reg", s[1]), s=True),
                    mvn(t, ("reg", t), s=True),
                    bx("lr"),
                ]
            )
        return s[0] + d[0] + self.alu(op, s[1], d[1], 32) + [bx("lr")]

    def _imd_alu(self, opcode, op, imm=None):
        dEA = opcode & 0xFF
        d = self.ea_load(dEA, 0, 2, op not in ("cmp", "tst"), inplace=True)
        if d is None:
            return None
        wd, regD = d
        w = list(wd)
        if (dEA >> 6) == 0 and imm is None and op in ("and", "or", "eor"):
            w.append(sxtb(1, 1))  # add / sub / cmp shift the operand up by 24 anyway: the upper bits are gone
        w += self.alu(op, 1, regD, 8 << (dEA >> 6), imm=imm)
        if op not in ("cmp", "tst"):
            w += self.ea_store(dEA, regD, 2, True)
        if regD < 6 and op in ("add", "sub", "neg"):
            w += self.save_x()
        return w + [bx("lr")]

    def _move(self, opcode, size):
        sEA = size << 6 | (opcode & 0x3F)
        dEA = size << 6 | ((opcode & 0x0E00) >> 9) | ((opcode & 0x01C0) >> 3)
        if self.pin and size == 2 and (dEA & 0x3F) == 0:
            # MOVE.L <ea>,D0: the pinned D0 is the destination of the load itself (no temporary, no store);
            # the flags come from the one `adds r4, src, #0`
            s = self.ea_load(sEA, 4, 1, False, inplace=True)
            if s is None:
                return None
            ws, regS = s
            return ws + [add(4, regS, ("imm", 0), s=True), bx("lr")]
        if self.pin and (sEA & 0x3F) == 0 and (dEA & 0x3F) != 0 and (dEA & 0x38) != 0x08:
            # MOVE <size> D0,<ea>: the store comes straight from r4 (a byte / halfword store takes the low
            # part), the flags from a scratch copy: nothing waits for them
            wf = (
                [add(0, 4, ("imm", 0), s=True)]
                if size == 2
                else [{0: sxtb, 1: sxth}[size](0, 4), add(0, 0, ("imm", 0), s=True)]
            )
            return wf + self.ea_store(dEA, 4, 2, False) + [bx("lr")]
        s = self.ea_load(sEA, 0, 1, False, inplace=True)
        if s is None:
            return None
        ws, regS = s
        return ws + self.nz_clear_vc(regS) + self.ea_store(dEA, regS, 2, False) + [bx("lr")]

    def _movea(self, opcode, size):
        sEA = size << 6 | (opcode & 0x3F)
        regD = 6 + ((opcode & 0x0E00) >> 9)
        s = self.ea_load(sEA, regD, 1, False, inplace=True)
        if s is None:
            return None
        ws, regS = s
        # (when the load already went into the address register there is nothing left to do; MOVEA A0,A0 has
        # no instruction at all and keeps a nop: a sequence needs a leader)
        return ws + ([mov(regD, ("reg", regS))] if regS != regD else [] if ws else [nop()]) + [bx("lr")]

    def emit_MOVEB(self, op):
        return self._move(op, 0)

    def emit_MOVEW(self, op):
        return self._move(op, 1)

    def emit_MOVEL(self, op):
        return self._move(op, 2)

    def emit_MOVEAW(self, op):
        return self._movea(op, 1)

    def emit_MOVEAL(self, op):
        return self._movea(op, 2)

    def emit_MOVEQ(self, op):
        n = op & 0xFF
        v = n - 256 if n & 0x80 else n
        if self.pin and (op >> 9) & 7 == 0:  # MOVEQ #n,D0: straight into r4
            return (
                [mov(4, ("imm", v)) if v >= 0 else mvn(4, ("imm", ~v & 0xFFFFFFFF))]
                + self.nz_clear_vc(4)
                + [bx("lr")]
            )
        first = mov(1, ("imm", v)) if v >= 0 else mvn(1, ("imm", ~v & 0xFFFFFFFF))
        return (
            [first]
            + self.nz_clear_vc(1)
            + self.ea_store(2 << 6 | ((op & 0x0E00) >> 9), 1, 2, False)
            + [bx("lr")]
        )
