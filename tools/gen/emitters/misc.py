"""The small integer instructions: CLR / NEG / NOT, ADDQ / SUBQ, EXT, SWAP, TST, MULU / MULS, CMPM, LEA, EXG, bit
operations, Scc.

Part of `gen.Gen`; emitters/__init__.py lists the other groups.
"""

from arm32 import *
from emitters.base import Seq


class MiscMixin:
    """The small integer instructions: CLR / NEG / NOT, ADDQ / SUBQ, EXT, SWAP, TST, MULU / MULS, CMPM, LEA,
    EXG, bit operations, Scc.
    """

    def _unary(self, opcode, op):
        dEA = opcode & 0xFF
        d = self.ea_load(dEA, 0, 2, True, inplace=True)
        if d is None:
            return None
        regD = d[1]
        wd = [] if op == "clr" and (dEA & 0x38) == 0 else d[0]  # CLR Dn: the old value is never read
        w = wd + self.alu(op, 0, regD, 8 << (dEA >> 6)) + self.ea_store(dEA, regD, 2, True)
        if regD < 6 and op == "neg":
            w += self.save_x()
        return w + [bx("lr")]

    def _quick(self, opcode, op):
        v = ((opcode & 0xE00) >> 9) or 8
        if (
            op in ("add", "sub") and (opcode & 0x38) == 0x08
        ):  # ADDQ/SUBQ #v,An: no flags, the immediate goes straight into the instruction
            d = self.ea_load(opcode & 0xFF, 0, 2, True)
            if d is None:
                return None
            wd, regD = d
            return (
                list(wd)
                + [dp(op, regD, regD, ("imm", v))]
                + self.ea_store(opcode & 0xFF, regD, 2, True)
                + [bx("lr")]
            )
        if (
            op == "sub" and (opcode & 0xF8) == 0x80
        ):  # SUBQ.L #v,Dn: `rsbs d,d,#v-1` is ~d + v, so C is the 68K borrow with no `mov`/`mvn` pair
            d = self.ea_load(opcode & 0xFF, 0, 2, True, inplace=True)
            if d is None:
                return None
            wd, regD = d
            w = (
                list(wd)
                + [dp("rsb", regD, regD, ("imm", v - 1), s=True), mvn(regD, ("reg", regD), s=True)]
                + self.ea_store(opcode & 0xFF, regD, 2, True)
            )
            if regD < 6:
                w += self.save_x()
            return w + [bx("lr")]
        # the quick value is an immediate of the operation itself (1-8 << the field's shift: always encodable)
        return self._imd_alu(opcode, op, imm=v)

    def emit_ADDQBW(self, op):
        return self._quick(op, "add")

    def emit_ADDQL(self, op):
        return self._quick(op, "add")

    def emit_ADDQAW(self, op):
        return self._quick((op & ~0xC0) | 0x80, "add")

    def emit_SUBQBW(self, op):
        return self._quick(op, "sub")

    def emit_SUBQL(self, op):
        return self._quick(op, "sub")

    def emit_SUBQAW(self, op):
        return self._quick((op & ~0xC0) | 0x80, "sub")

    def emit_EXTL(self, op):
        return self._imd_alu((2 << 6) | (op & 7), "extwl")

    def emit_EXTW(self, op):
        # the byte straight to a sign-extended halfword: no halfword load that the extend then discards
        n = op & 7
        return self.dload(0, n, 1) + self.nz_clear_vc(0) + self.dstore(0, n, 2) + [bx("lr")]

    def emit_SWAP(self, op):
        return self._imd_alu((op & 0x3F) | 0x80, "swap")

    def emit_TSTBW(self, op):
        return self._imd_alu(op, "tst")

    def emit_TSTL(self, op):
        return self._imd_alu(op, "tst")

    def _mul(self, opcode, op):
        opcode = (opcode & 0xFE3F) | 0x80
        sEA = 0x40 | (opcode & 0x3F)
        dEA = 0x80 | ((opcode & 0xE00) >> 9)
        if op == "mulsw":
            # both operands are loaded as sign-extended halfwords already (ldrsh of the D slot's low half): no
            # extend, just the multiply
            s = self.ea_load(sEA, 1, 1, False)
            d = self.ea_load(0x40 | (dEA & 7), 0, 2, False)
            if s is None or d is None:
                return None
            return (
                s[0]
                + d[0]
                + [mul(0, 0, s[1])]
                + self.nz_clear_vc(0)
                + self.ea_store(dEA, 0, 2, True)
                + [bx("lr")]
            )
        s = self.ea_load(sEA, 1, 1, False)
        d = self.ea_load(dEA, 0, 2, True)
        if s is None or d is None:
            return None
        return s[0] + d[0] + self.alu(op, s[1], d[1], 32) + self.ea_store(dEA, d[1], 2, True) + [bx("lr")]

    def emit_MULS(self, op):
        return self._mul(op, "mulsw")

    def emit_MULU(self, op):
        return self._mul(op, "muluw")

    def _cmpm(self, opcode):
        dEA = 0x18 | (opcode & 0xC0) | ((opcode >> 9) & 7)
        sEA = 0x18 | (opcode & 0xC7)
        s = self.ea_load(sEA, 1, 1, False)
        d = self.ea_load(dEA, 0, 2, False)
        return s[0] + d[0] + self.alu("cmp", s[1], d[1], 8 << (dEA >> 6)) + [bx("lr")]

    def emit_CMPMB(self, op):
        return self._cmpm(op)

    def emit_CMPMW(self, op):
        return self._cmpm(op)

    def emit_CMPML(self, op):
        return self._cmpm(op)

    def emit_LEA(self, op):
        d = 6 + ((op >> 9) & 7)
        s = 6 + (op & 7)
        mode = (op >> 3) & 7
        if mode == 2:
            return [mov(d, ("reg", s)), bx("lr")]
        if mode in (5, 6):
            return [add(d, s, ("reg", 1)), bx("lr")]
        return [mov(d, ("reg", 1)), bx("lr")]

    def emit_EXG(self, op):
        rx = (op >> 9) & 7
        ry = op & 7
        D0 = self.D0
        k = (op >> 3) & 0x1F
        if k == 0x08:
            w = self.dload(0, rx) + self.dload(1, ry) + self.dstore(1, rx) + self.dstore(0, ry)
        elif k == 0x09:
            w = [mov(0, ("reg", 6 + rx)), mov(6 + rx, ("reg", 6 + ry)), mov(6 + ry, ("reg", 0))]
        elif k == 0x11:
            w = self.dload(0, rx) + self.dstore(6 + ry, rx) + [mov(6 + ry, ("reg", 0))]
        else:
            w = []
        return w + [bx("lr")]

    def _bit(self, opcode, op):
        s = self.ea_load((opcode >> 9) & 7 if opcode & 0x100 else 0x3C, 1, 1, False)
        if s is None:
            return None
        w, regS = s
        w = list(w)
        if w and w[-1] == sxtb(regS, regS):
            w.pop()  # the bit number is masked just below: no sign extension needed
        # the bit number is taken modulo the operand's width
        w.append(and_(regS, regS, ("imm", 31 if not opcode & 0x38 else 7)))
        w += [mov(0, ("imm", 1)), mov(regS, ("rsh", 0, "lsl", regS))]
        dEA = (opcode & 0x3F) if opcode & 0x38 else (0x80 | (opcode & 0x3F))
        if not self.legacy:
            # the target layout: own words = the bit number (r1) and the address of the operand (r2); one
            # routine per operation and operand size (a data register's long, or a byte)
            a = self.ea_addr(dEA, 2, 2)
            own = list(s[0]) + ([mov(1, ("reg", regS))] if regS != 1 else [])
            if a is not None and regS == 1 and len(own) + len(a) <= 3:
                if own and own[-1] == sxtb(1, 1):
                    own.pop()
                long_ = not opcode & 0x38
                routine = [
                    and_(1, 1, ("imm", 31 if long_ else 7)),
                    mov(0, ("imm", 1)),
                    mov(1, ("rsh", 0, "lsl", 1)),
                    (ldr if long_ else ldrsb)(0, 2, 0),
                ] + self.alu(op, 1, 0, 32)
                if op != "btst":
                    routine.append((str_ if long_ else strb)(0, 2, 0))
                return Seq(own + a + routine + [bx("lr")], split=len(own) + len(a))
        d = self.ea_load(dEA, 0, 2, op != "btst", inplace=True)
        if d is None:
            return None
        regD = d[1]
        w += d[0] + self.alu(op, regS, regD, 32)
        if op != "btst":
            w += self.ea_store(dEA, regD, 2, True)
        return w + [bx("lr")]

    def emit_BCHG(self, op):
        return self._bit(op, "bchg")

    def emit_BCLR(self, op):
        return self._bit(op, "bclr")

    def emit_BSET(self, op):
        return self._bit(op, "bset")

    def emit_BTST(self, op):
        return self._bit(op, "btst")

    def emit_Scc(self, op):
        cc = self.ARMCC[(op >> 8) & 15]
        if cc == "al":
            w = [mov(0, ("imm", 0xFF))]
        elif cc == "nv":
            w = [mov(0, ("imm", 0))]
        elif cc == "hi":
            w = [mov(0, ("imm", 0xFF)), mov(0, ("imm", 0), cond="cs"), mov(0, ("imm", 0), cond="eq")]
        elif cc == "ls":
            w = [mov(0, ("imm", 0)), mov(0, ("imm", 0xFF), cond="cs"), mov(0, ("imm", 0xFF), cond="eq")]
        else:
            w = [mov(0, ("imm", 0xFF), cond=cc), mov(0, ("imm", 0), cond=self.NUMCC[COND[cc] ^ 1])]
        if (
            self.pin and (op & 0x3F) == 0 and cc not in ("hi", "ls")
        ):  # Scc D0: set / clear the low byte of r4 under the condition itself
            if cc == "al":
                return [orr(4, 4, ("imm", 0xFF)), bx("lr")]
            if cc == "nv":
                return [dp("bic", 4, 4, ("imm", 0xFF)), bx("lr")]
            return [
                orr(4, 4, ("imm", 0xFF), cond=cc),
                dp("bic", 4, 4, ("imm", 0xFF), cond=self.NUMCC[COND[cc] ^ 1]),
                bx("lr"),
            ]
        return w + self.ea_store(op & 0x3F, 0, 2, False) + [bx("lr")]
