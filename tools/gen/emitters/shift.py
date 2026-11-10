"""Shifts and rotates (ASL / ASR / LSL / LSR / ROL / ROR / ROXL / ROXR), register and memory forms.

Part of `gen.Gen`; emitters/__init__.py lists the other groups.
"""

from arm32 import *
from emitters.base import Seq


class ShiftMixin:
    """Shifts and rotates (ASL / ASR / LSL / LSR / ROL / ROR / ROXL / ROXR), register and memory forms."""

    # ---- shifts and rotates ----
    def _clear_v(self):
        return [mrs(3), dp("bic", 3, 3, ("imm", 0x10000000)), msr_f(3)]

    def _replicate(self, width):
        if width == 8:
            return [uxtb(0, 0), orr(0, 0, ("reg", 0, "lsl", 8)), orr(0, 0, ("reg", 0, "lsl", 16))]
        if width == 16:
            return [uxth(0, 0), orr(0, 0, ("reg", 0, "lsl", 16))]
        return []

    def shift_core(self, op, width, reg_count=False):
        """One shift or rotate: r0 = value (sign-extended), r1 = count (0-63); result in r0. Scratch r3 (and
        r1 once spent); ROXL / ROXR with a register count also use r2.
        """
        shl = 32 - width if width < 32 else 0
        w = []
        sh = lambda d, src, kind, r, **k: mov(d, ("rsh", src, kind, r), **k)
        # V and C are cleared first by `cmn r0,#0`, then the shift sets N, Z and C (not for a count of 0) and
        # leaves V alone: no `mrs` / `msr` round trip
        # r0 + 0: V and C clear (a count of 0 leaves C alone), N and Z from the value
        z = dp("cmn", 0, 0, ("imm", 0))
        if op == "asr":
            return [z, sh(0, 0, "asr", 1, s=True)]
        if op == "lsr":
            if width == 8:
                w.append(uxtb(0, 0))
            if width == 16:
                w.append(uxth(0, 0))
            return w + [z, sh(0, 0, "lsr", 1, s=True)]
        if op == "lsl":
            if shl:
                w.append(lsl(0, 0, shl))
            w += [z, sh(0, 0, "lsl", 1, s=True)]
            if shl:
                w.append(lsr(0, 0, shl))
            return w
        if op == "asl":
            # V is set first, from a signed overflow of `cmp r3,#1` (r3 = 0x80000000 when shifting back does
            # not restore the value, else 0), because the shift below leaves V
            # alone; the cmp also leaves C = 0 when there is no overflow, which is what a count of 0 needs. No
            # `mrs` / `msr`.
            if shl:
                w.append(lsl(0, 0, shl))
            w += [
                sh(3, 0, "lsl", 1),
                sh(3, 3, "asr", 1),
                cmp_(3, ("reg", 0)),
                mov(3, ("imm", 0x80000000), cond="ne"),
                mov(3, ("imm", 0), cond="eq"),
                cmp_(3, ("imm", 1)),
                sh(0, 0, "lsl", 1, s=True),
            ]
            if shl:
                w.append(lsr(0, 0, shl))
            return w
        if op == "ror":
            return self._replicate(width) + [z, sh(0, 0, "ror", 1, s=True)]
        if op == "rol":
            return self._replicate(width) + [
                dp("rsb", 1, 1, ("imm", 32)),
                sh(0, 0, "ror", 1, s=True),
                mrs(1),
                dp("bic", 1, 1, ("imm", 0x10000000)),
                bfi(1, 0, 29, 1),
                msr_f(1),
            ]
        # roxl / roxr: X and the operand form a (width + 1)-bit field that rotates, so the count is taken mod
        # width + 1 (9, 17, 33) and the rotation is a shift pair:
        # no loop, constant time (the loop this replaces ran up to 63 times, ~400 cycles). r3 = X on entry and
        # X' on exit.
        # r3 = X (GE0 is set when X is clear); rare enough for `mrs`
        w = [
            mrs(3),
            ubfx(3, 3, 16, 1),
            eor(3, 3, ("imm", 1)),
        ]
        # count 0-63 -> 0..width, with a multiply-shift reciprocal (r2 is free in the register form);
        # floor(n/9) = (57 n) >> 9 for n < 64
        if reg_count:
            if width == 8:
                w += [
                    dp("rsb", 2, 1, ("reg", 1, "lsl", 3)),
                    add(2, 1, ("reg", 2, "lsl", 3)),
                    lsr(2, 2, 9),
                    add(2, 2, ("reg", 2, "lsl", 3)),
                    dp("sub", 1, 1, ("reg", 2)),
                ]
            elif width == 16:
                # floor(n/17) = (61 n) >> 10 for n < 64
                w += [
                    add(2, 1, ("reg", 1, "lsl", 1)),
                    dp("rsb", 2, 2, ("reg", 1, "lsl", 6)),
                    lsr(2, 2, 10),
                    add(2, 2, ("reg", 2, "lsl", 4)),
                    dp("sub", 1, 1, ("reg", 2)),
                ]
            else:
                w += [cmp_(1, ("imm", 33)), dp("sub", 1, 1, ("imm", 33), cond="cs")]
        if width < 32:
            ext = uxtb if width == 8 else uxth
            w += [ext(0, 0), orr(0, 0, ("reg", 3, "lsl", width))]  # r0 = X : value, width + 1 bits
            if op == "roxl":
                w += [
                    sh(3, 0, "lsl", 1),
                    dp("rsb", 1, 1, ("imm", width + 1)),
                    orr(0, 3, ("rsh", 0, "lsr", 1)),
                ]
            else:
                w += [
                    sh(3, 0, "lsr", 1),
                    dp("rsb", 1, 1, ("imm", width + 1)),
                    orr(0, 3, ("rsh", 0, "lsl", 1)),
                ]
            # X' = the bit that rotated out of the top; the value, zero-extended
            w += [
                ubfx(3, 0, width, 1),
                ext(0, 0),
            ]
            w += [lsl(1, 0, shl), cmp_(1, ("imm", 0))]  # N, Z of the width-bit result, V clear
        else:  # a 33-bit rotation by n (1-32; 0 leaves everything as it is: the guard)
            w.append(cmp_(1, ("imm", 0)))
            guard = len(w)
            w.append(None)
            if op == "roxl":
                w += [
                    dp("rsb", 2, 1, ("imm", 32)),
                    sh(2, 0, "lsr", 2),
                    sh(0, 0, "lsl", 1),
                    orr(0, 0, ("reg", 2, "lsr", 1)),
                    dp("sub", 1, 1, ("imm", 1)),
                    sh(1, 3, "lsl", 1),
                    orr(0, 0, ("reg", 1)),
                    and_(3, 2, ("imm", 1)),
                ]
            else:
                w += [
                    dp("sub", 2, 1, ("imm", 1)),
                    sh(2, 0, "lsr", 2),
                    dp("rsb", 1, 1, ("imm", 32)),
                    sh(0, 0, "lsl", 1),
                    lsl(0, 0, 1),
                    orr(0, 0, ("reg", 2, "lsr", 1)),
                    sh(1, 3, "lsl", 1),
                    orr(0, 0, ("reg", 1)),
                    and_(3, 2, ("imm", 1)),
                ]
            w[guard] = b(len(w) - guard, "eq")
            w.append(cmp_(0, ("imm", 0)))
        w += [mrs(1), bfi(1, 3, 29, 1), msr_f(1)]  # C = X'
        return w

    def _shift_op(self, opcode, pair):
        return pair[0] if opcode & 0x100 else pair[1]

    def _mem_shift(self, opcode, pair):
        op = self._shift_op(opcode, pair)
        dEA = 0x40 | (opcode & 0x3F)
        r = self.ea_load(dEA, 0, 2, True)
        if r is None:
            return None
        if op in ("lsl", "lsr", "asr"):
            # a word shifted by 1: the same short form as a constant count on Dn (V cleared by `cmp r0,r0`, no
            # count register, no shift_core)
            w = list(r[0]) + [cmp_(0, ("reg", 0))]
            if op == "lsl":
                w += [lsl(0, 0, 16), lsl(0, 0, 1, s=True), lsr(0, 0, 16)]
            elif op == "lsr":
                w += [uxth(0, 0), lsr(0, 0, 1, s=True)]
            else:
                w.append(mov(0, ("reg", 0, "asr", 1), s=True))
            return w + self.ea_store(dEA, 0, 2, True) + self.save_x(False) + [bx("lr")]
        w = r[0] + [mov(1, ("imm", 1))] + self.shift_core(op, 16) + self.ea_store(dEA, 0, 2, True)
        if op not in ("rol", "ror"):
            w += self.save_x(op in ("roxl", "roxr"))  # ROXL/ROXR read X, so their X out feeds the next one
        return w + [bx("lr")]

    def _reg_shift(self, opcode, pair):
        op = self._shift_op(opcode, pair)
        value = (opcode & 0xE00) >> 9
        dEA = opcode & 0xC7
        width = 8 << ((opcode >> 6) & 3)
        reg_count = bool(opcode & 0x20)
        w = []
        fast = reg_count and (op in ("lsl", "asl", "asr", "ror") or (op == "lsr" and width == 32))
        # (the pinned D0 has no address for the routine to index: the flat form)
        split = reg_count and not self.legacy and not (self.pin and (dEA & 7) == 0)
        # the target layout: own words = the count register's load and the operand register's number; the
        # routine indexes by it
        w8 = width // 8
        own = []
        if split:
            own = self.dload(1, value) + [mov(2, ("imm", dEA & 7))]
            w += [add(3, 5, ("reg", 2, "lsl", 2)), str_(2, 5, self.SCRATCH), and_(1, 1, ("imm", 63))] + (
                [] if fast else [mov(2, ("reg", 1))]
            )
        elif reg_count:
            w += self.dload(1, value) + [and_(1, 1, ("imm", 63))] + ([] if fast else [mov(2, ("reg", 1))])
        else:
            w.append(mov(1, ("imm", value or 8)))
        r = (
            ([{1: ldrsb, 2: ldrsh, 4: ldr}[w8](0, 3, self.D0 + (4 - w8))], 0)
            if split
            else self.ea_load(dEA, 0, 2, True)
        )
        store = (
            [
                ldr(3, 5, self.SCRATCH),
                add(3, 5, ("reg", 3, "lsl", 2)),
                {1: strb, 2: strh, 4: str_}[w8](0, 3, self.D0 + (4 - w8)),
            ]
            if split
            else self.ea_store(dEA, 0, 2, True)
        )
        if not reg_count and op in ("lsl", "lsr", "asr"):
            # a constant count of 1-8 on a data register: V is cleared first (`cmp r0,r0`: no overflow), then
            # the shift sets N, Z and C and leaves V alone.
            # No `mrs`/`msr` (~10 cycles), no call: the whole instruction is these few words.
            n = value or 8
            shl = 32 - width if width < 32 else 0
            w = []
            w += r[0] + [cmp_(0, ("reg", 0))]
            if op == "lsl":
                if shl:
                    w.append(lsl(0, 0, shl))
                w.append(lsl(0, 0, n, s=True))
                if shl:
                    w.append(lsr(0, 0, shl))
            elif op == "lsr":
                if width == 8:
                    w.append(uxtb(0, 0))
                if width == 16:
                    w.append(uxth(0, 0))
                w.append(lsr(0, 0, n, s=True))
            else:
                w.append(mov(0, ("reg", 0, "asr", n), s=True))
            w += self.ea_store(dEA, 0, 2, True) + self.save_x(False)
            return w + [bx("lr")]
        w += r[0]
        sets_x = op not in ("rol", "ror")
        if fast and sets_x:
            # X := the last bit shifted out, unchanged for a count of 0: a scratch shift with C preloaded with
            # X (the count-0 case then yields X itself), then X := C.
            # The real shift below starts from `cmn r0,#0`, so a count of 0 leaves C = V = 0 and no flag needs
            # mending afterwards.
            shl = 32 - width if width < 32 else 0
            # C := X
            w += [
                mov(3, ("imm", 0)),
                mvn(2, ("imm", 0)),
                sel(3, 3, 2),
                dp("add", 3, 3, ("reg", 3), s=True),
            ]
            if op in ("lsl", "asl"):
                if shl:
                    w.append(lsl(3, 0, shl))
                w.append(mov(3, ("rsh", 3 if shl else 0, "lsl", 1), s=True))
            else:
                w.append(mov(3, ("rsh", 0, op, 1), s=True))
            w += self.save_x(True)
        w += self.shift_core(op, width, reg_count) + store
        if fast:
            pass
        elif reg_count and op not in ("roxl", "roxr"):
            # count 0: N is the operand's own top bit, which the zero-extended shift lost
            fixn = op == "lsr" and width < 32
            w += [clz(3, 2), lsr(3, 3, 5), mrs(2)]
            if fixn:
                w += [and_(1, 0, ("imm", 1 << (width - 1))), lsl(1, 1, 32 - width)]
            w.append(dp("tst", 0, 3, ("imm", 1)))
            if fixn:
                w.append(orr(2, 2, ("reg", 1), cond="ne"))
            if sets_x:
                # (eq = the count was not 0): GE := not C, only then. Nothing here sets the flags the
                # condition reads.
                w += [
                    ubfx(1, 2, 29, 1),
                    eor(1, 1, ("imm", 1)),
                    lsl(1, 1, 16),
                    dp("rsb", 1, 1, ("reg", 1, "lsl", 4)),
                    msr_g(1, cond="eq"),
                ]
            w += [dp("bic", 2, 2, ("imm", 0x20000000), cond="ne"), msr_f(2)]
        elif sets_x:
            w += self.save_x(op in ("roxl", "roxr"))
        return Seq(own + w + [bx("lr")], split=len(own)) if split else w + [bx("lr")]
