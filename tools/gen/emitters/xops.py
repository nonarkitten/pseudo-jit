"""The extended-precision family that reads and writes X: ADDX, SUBX, NEGX.

Part of `gen.Gen`; emitters/__init__.py lists the other groups.
"""

from arm32 import *
from emitters.base import Seq


class XopsMixin:
    """The extended-precision family that reads and writes X: ADDX, SUBX, NEGX."""

    # ---- ADDX / SUBX / NEGX ----
    # The flags are never read back as a value: no mrs, no msr, no bitfield insert.
    # * sticky Z (Z is cleared by a non-zero result, otherwise it keeps its old value) takes two conditional
    # moves in front (r3 = 0 when Z is set, 1 when it is clear) and one
    # conditional test after the operation: `tsteq r3, r3` runs only when the result was zero (N is then clear
    # already, and a TST with an unshifted register leaves C and V
    #     alone) and sets Z := (r3 == 0) = the old Z.
    # * C := X is `sel` (GE = 1111 when X is clear, so the byte lanes pick 0 or -1) and `adds r0, r0, r0`,
    # which carries out of -1 only.
    # * the subtracts are done on the complement, ~(~d + s + X): C is then the borrow with no fix-up, V is the
    # subtract's, and the closing `mvns` puts the result in N and Z.
    # For a byte or word the operands sit at the top of the register; ~d has all ones below the field, so the
    # carry-in ripples into it with no help. ADDX has zeros there,
    #     so it sets them to ones when C (`orrcs`): the carry-in is then absorbed and the field gets the +1.

    def _x_front(self, tmp):
        """Z-keep into r3 (from the flags as the instruction found them), then C := X (clobbers N Z V and the
        two scratch registers tmp).
        """
        a, c = tmp
        return [
            mov(3, ("imm", 0), cond="eq"),
            mov(3, ("imm", 1), cond="ne"),
            mov(a, ("imm", 0)),
            mvn(c, ("imm", 0)),
            sel(0, a, c),
            dp("add", 0, 0, ("reg", 0), s=True),
        ]

    def _x_back(self, regD, shl):
        w = [dp("tst", 0, 3, ("reg", 3), cond="eq")]
        if shl:
            w.append(lsr(regD, regD, shl))
        return w

    def _xop_long(self, opcode, op):
        """ADDX.L / SUBX.L: three leader words that are
        unique to the operand registers -- r0 = the source value, r1 = the
        address of the destination -- and one routine that is the same for all 128 opcodes. Registers: r0
        source, r1 destination address, r2 scratch then destination data,
        r3 = old Z inverted (also the zero lane of the `sel`: it is 0 or 1, so its top bit is clear)."""
        if opcode & 8:  # -(Ay),-(Ax)
            ay, ax = (opcode & 7) + 6, ((opcode >> 9) & 7) + 6
            lead = [
                ldr(0, ay, -self.step(4, ay), kind="pre"),
                sub(ax, ax, ("imm", self.step(4, ax))),
                mov(1, ("reg", ax)),
            ]
        else:  # Dy,Dx
            lead = self.dload(0, opcode & 7) + [self._addr_of_d((opcode >> 9) & 7), nop()]
        core = (
            [dp("adc", 2, 2, ("reg", 0), s=True)]
            if op == "addx"
            else [mvn(2, ("reg", 2)), dp("adc", 2, 2, ("reg", 0), s=True), mvn(2, ("reg", 2), s=True)]
        )
        routine = (
            [
                mov(3, ("imm", 0), cond="eq"),
                mov(3, ("imm", 1), cond="ne"),
                mvn(2, ("imm", 0)),
                sel(2, 3, 2),
                dp("add", 2, 2, ("reg", 2), s=True),
                ldr(2, 1, 0),
            ]
            + core
            + [dp("tst", 0, 3, ("reg", 3), cond="eq"), str_(2, 1, 0)]
            + self.save_x(True)
            + [bx("lr")]
        )
        return Seq(lead + routine, split=len(lead))

    def _xop_narrow(self, opcode, op, width, shl):
        """ADDX / SUBX .B / .W in the target layout: the opcode's own words load the source (r0) and form the
        destination's address (r1); one routine per operation and size does the
        rest. r1 is parked in cpu->scratch so that it can be the mask temporary of the shifted add /
        complement.
        """
        ld, st, w8 = (ldrsb, strb, 1) if width == 8 else (ldrsh, strh, 2)
        if opcode & 8:  # -(Ay),-(Ax)
            ay, ax = (opcode & 7) + 6, ((opcode >> 9) & 7) + 6
            lead = [
                ld(0, ay, -self.step(w8, ay), kind="pre"),
                sub(ax, ax, ("imm", self.step(w8, ax))),
                mov(1, ("reg", ax)),
            ]
        else:  # Dy,Dx
            dx = self.D0 + ((opcode >> 9) & 7) * 4 + (4 - w8)
            lead = self.dload(0, opcode & 7, w8) + [
                add(1, 5, ("imm", dx)) if dx >= 0 else sub(1, 5, ("imm", -dx)),
                nop(),
            ]
        front = [
            mov(3, ("imm", 0), cond="eq"),
            mov(3, ("imm", 1), cond="ne"),
            mvn(2, ("imm", 0)),
            sel(2, 3, 2),
            dp("add", 2, 2, ("reg", 2), s=True),
        ]
        if op == "addx":
            core = [
                lsl(2, 2, shl),
                mvn(1, ("imm", 0xFF000000)) if shl == 24 else movw(1, 0xFFFF),
                orr(2, 2, ("reg", 1), cond="cs"),
                dp("adc", 2, 2, ("reg", 0, "lsl", shl), s=True),
            ]
        else:
            core = [
                mvn(2, ("reg", 2, "lsl", shl)),
                dp("adc", 2, 2, ("reg", 0, "lsl", shl), s=True),
            ] + self._x_complement(2, 1, shl)
        routine = (
            [str_(1, 5, self.SCRATCH)]
            + front
            + [ld(2, 1, 0)]
            + core
            + self._x_back(2, shl)
            + [ldr(1, 5, self.SCRATCH), st(2, 1, 0)]
            + self.save_x(True)
            + [bx("lr")]
        )
        return Seq(lead + routine, split=3)

    def _addr_of_d(self, n):
        off = self.D0 + n * 4
        return add(1, 5, ("imm", off)) if off >= 0 else sub(1, 5, ("imm", -off))

    def _x_complement(self, regD, spare, shl):
        """The closing complement of the subtracts. Long: `mvns` (N, Z of the result; C and V stay). Narrow: the
        carry rippling into the field left ones below it, which the
        complement would turn into a non-zero result, so N and Z come from `tst` against a mask of the field
        (an unshifted register operand: C and V stay) instead.
        """
        if not shl:
            return [mvn(regD, ("reg", regD), s=True)]
        mask = (
            [mov(spare, ("imm", 0xFF000000))]
            if shl == 24
            else [mvn(spare, ("imm", 0)), lsl(spare, spare, 16)]
        )
        return [mvn(regD, ("reg", regD))] + mask + [dp("tst", 0, regD, ("reg", spare))]

    def _xop(self, opcode, op):
        sEA = opcode & 0xC7
        dEA = ((opcode >> 9) & 7) | (opcode & 0xC0)
        if opcode & 8:
            sEA |= 0x20
            dEA |= 0x20
        width = 8 << ((opcode >> 6) & 3)
        shl = 32 - width if width < 32 else 0
        # the pinned D0 as the destination has no address for the shared routine
        d0dest = self.pin and not opcode & 8 and ((opcode >> 9) & 7) == 0
        if width == 32 and not d0dest:
            return self._xop_long(opcode, op)
        if not self.legacy and width != 32 and not d0dest:
            return self._xop_narrow(opcode, op, width, shl)
        s = self.ea_load(sEA, 1, 1, False)
        d = self.ea_load(dEA, 0, 2, True)
        if s is None or d is None:
            return None
        regS, regD = s[1], d[1]
        if op == "addx":
            core = []
            if shl:
                core += [
                    lsl(regD, regD, shl),
                    mvn(2, ("imm", 0xFF000000)) if shl == 24 else movw(2, 0xFFFF),
                    orr(regD, regD, ("reg", 2), cond="cs"),
                    dp("adc", regD, regD, ("reg", regS, "lsl", shl), s=True),
                ]
            else:
                core += [dp("adc", regD, regD, ("reg", regS), s=True)]
        else:  # d - s - X = ~(~d + s + X)
            core = [
                mvn(regD, ("reg", regD, "lsl", shl) if shl else ("reg", regD)),
                dp("adc", regD, regD, ("reg", regS, "lsl", shl) if shl else ("reg", regS), s=True),
            ]
            core += self._x_complement(regD, regS, shl)
        return (
            self._x_front((1, 2))
            + s[0]
            + d[0]
            + core
            + self._x_back(regD, shl)
            + self.ea_store(dEA, regD, 2, True)
            + self.save_x(True)
            + [bx("lr")]
        )

    def _negx(self, opcode):
        dEA = opcode & 0xFF
        width = 8 << (dEA >> 6)
        shl = 32 - width if width < 32 else 0
        d = self.ea_load(dEA, 0, 2, True)
        if d is None:
            return None
        if not self.legacy:
            a = self.ea_addr(dEA, 1, 2)
            if a is not None and len(a) <= 3:
                ld, st = {8: (ldrsb, strb), 16: (ldrsh, strh), 32: (ldr, str_)}[width]
                core = [
                    mvn(1, ("imm", 0)),
                    dp("adc", 2, 1, ("reg", 2, "lsl", shl) if shl else ("reg", 2), s=True),
                ] + self._x_complement(2, 1, shl)
                routine = (
                    [str_(1, 5, self.SCRATCH)]
                    + self._x_front((2, 0))
                    + [ld(2, 1, 0)]
                    + core
                    + self._x_back(2, shl)
                    + [ldr(1, 5, self.SCRATCH), st(2, 1, 0)]
                    + self.save_x(True)
                    + [bx("lr")]
                )
                return Seq(a + routine, split=len(a))
        regD = d[1]  # 0 - d - X = ~(-1 + d + X); -1 has ones below any field, so the carry-in ripples up
        core = [
            mvn(1, ("imm", 0)),
            dp("adc", regD, 1, ("reg", regD, "lsl", shl) if shl else ("reg", regD), s=True),
        ] + self._x_complement(regD, 1, shl)
        return (
            self._x_front((0, 1))
            + d[0]
            + core
            + self._x_back(regD, shl)
            + self.ea_store(dEA, regD, 2, True)
            + self.save_x(True)
            + [bx("lr")]
        )

    def emit_ADDXBW(self, op):
        return self._xop(op, "addx")

    def emit_ADDXL(self, op):
        return self._xop(op, "addx")

    def emit_SUBXBW(self, op):
        return self._xop(op, "subx")

    def emit_SUBXL(self, op):
        return self._xop(op, "subx")

    def emit_NEGXBW(self, op):
        return self._negx(op)

    def emit_NEGXL(self, op):
        return self._negx(op)
