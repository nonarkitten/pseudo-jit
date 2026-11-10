"""The instructions that call shared runtime routines: TAS, CHK, DIVU / DIVS, LINK / UNLK, PEA, MOVEP, MOVEM,
ABCD / SBCD / NBCD.

Part of `gen.Gen`; emitters/__init__.py lists the other groups.
"""

from arm32 import *
from emitters.base import Seq


class ComplexMixin:
    """The instructions that call shared runtime routines: TAS, CHK, DIVU / DIVS, LINK / UNLK, PEA, MOVEP,
    MOVEM, ABCD / SBCD / NBCD.
    """

    # ---- the rest ----
    def emit_TAS(self, op):
        dEA = op & 0x3F
        d = self.ea_load(dEA, 0, 2, True)
        if d is None:
            return None
        r = d[1]
        return (
            d[0]
            + self.nz_clear_vc(r)
            + [orr(r, r, ("imm", 0x80))]
            + self.ea_store(dEA, r, 2, True)
            + [bx("lr")]
        )

    def emit_CHK(self, op):
        d = self.ea_load(0x40 | ((op >> 9) & 7), 0, 2, False)
        s = self.ea_load(0x40 | (op & 0x3F), 1, 1, False)
        if d is None or s is None:
            return None
        return (
            d[0]
            + [cmp_(d[1], ("imm", 0)), svc(6, "mi")]
            + s[0]
            + [cmp_(d[1], ("reg", s[1])), svc(6, "gt"), bx("lr")]
        )

    def _div(self, op, signed):
        o = self.o
        sEA = 0x40 | (op & 0x3F)
        dEA = 0x80 | ((op & 0xE00) >> 9)
        if not self.legacy and not (
            self.pin and (op & 0xE00) == 0
        ):  # (the pinned D0 as the dividend has no address for the shared routine: the flat form)
            # the target layout: the opcode's own words load the divisor into r1 and name the dividend
            # register (r2 = n); ONE routine per signedness does the rest, so the DIV
            # family is two routines instead of a body per opcode
            s = self.ea_load(sEA, 1, 1, False)
            if s is None:
                return None
            own = (
                list(s[0]) + ([mov(1, ("reg", s[1]))] if s[1] != 1 else []) + [mov(2, ("imm", (op >> 9) & 7))]
            )
            routine = [
                str_(12, 5, o["CPU_R12"]),
                mrs(3),
                str_(3, 5, o["CPU_DIV_CPSR"]),
                add(3, 5, ("reg", 2, "lsl", 2)),
                str_(3, 5, o["CPU_DIV_DST"]),
                ldr(0, 3, self.D0),
                (sxth if signed else uxth)(1, 1),
                cmp_(1, ("imm", 0)),
                svc(5, "eq"),
                add(2, 15, ("imm", 4)),
                str_(2, 5, self.SCRATCH),
                sym("handle_DIVS_tail" if signed else "handle_DIVU_tail"),
                ldr(12, 5, o["CPU_R12"]),
                ldr(3, 5, o["CPU_DIV_DST"]),
                str_(0, 3, self.D0),
                msr_f(2),
                bx("lr"),
            ]
            return Seq(own + routine, split=len(own))
        s = self.ea_load(sEA, 1, 1, False)
        d = self.ea_load(dEA, 0, 2, True)
        if s is None or d is None:
            return None
        w = [str_(12, 5, o["CPU_R12"]), mrs(2), str_(2, 5, o["CPU_DIV_CPSR"])] + s[0] + d[0]
        if d[1] != 0:
            w.append(mov(0, ("reg", d[1])))
        if s[1] != 1:
            w.append(mov(1, ("reg", s[1])))
        # again, after the EA load: (A6)+ / -(A6) have changed A6, and the helper clobbers r12
        w.append(str_(12, 5, o["CPU_R12"]))
        w += [
            (sxth if signed else uxth)(1, 1),
            cmp_(1, ("imm", 0)),
            svc(5, "eq"),
            add(2, 15, ("imm", 4)),
            str_(2, 5, self.SCRATCH),
            sym("handle_DIVS_tail" if signed else "handle_DIVU_tail"),
            ldr(12, 5, o["CPU_R12"]),
        ]
        return w + self.ea_store(dEA, 0, 2, True) + [msr_f(2), bx("lr")]

    def emit_DIVS(self, op):
        return self._div(op, True)

    def emit_DIVU(self, op):
        return self._div(op, False)

    def emit_LINK(self, op):
        a = 6 + (op & 7)
        w = [sub(13, 13, ("imm", 4)), str_(13, 13, 0)] if a == 13 else [str_(a, 13, -4, kind="pre")]
        return w + [mov(a, ("reg", 13)), add(13, 13, ("reg", 1)), bx("lr")]

    def emit_UNLK(self, op):
        a = 6 + (op & 7)
        return [mov(13, ("reg", a)), ldr(a, 13, 0) if a == 13 else ldr(a, 13, 4, kind="post"), bx("lr")]

    def emit_PEA(self, op):
        a = 6 + (op & 7)
        mode = (op >> 3) & 7
        if mode == 2:
            w = [str_(a, 13, -4), sub(13, 13, ("imm", 4))] if a == 13 else [str_(a, 13, -4, kind="pre")]
        else:
            w = ([add(1, a, ("reg", 1))] if mode in (5, 6) else []) + [str_(1, 13, -4, kind="pre")]
        return w + [bx("lr")]

    def emit_MOVEP(self, op):
        a = 6 + (op & 7)
        dreg = (op >> 9) & 7
        nb = 4 if op & 0x40 else 2
        to_mem = bool(op & 0x80)
        d = self.D0 + dreg * 4 + (4 - nb)
        w = [add(2, a, ("reg", 1))]
        # the pinned D0 goes through its slot in cpu_t (stored first: MOVEP.W keeps the upper half)
        spill = self.pin and dreg == 0
        if spill:
            w.append(str_(4, 5, self.D0))
        for k in range(nb):
            w += [ldrb(0, 5, d + k), strb(0, 2, k * 2)] if to_mem else [ldrb(0, 2, k * 2), strb(0, 5, d + k)]
        if spill and not to_mem:
            w.append(ldr(4, 5, self.D0))
        return w + [bx("lr")]

    def emit_MOVEM(self, op):
        mode = (op >> 3) & 7
        a = 6 + (op & 7)
        word = not (op & 0x40)
        to_reg = bool(op & 0x400)
        D0 = self.D0
        ror1 = lambda: mov(1, ("reg", 1, "ror", 1), s=True)
        if a == 13 and not word and ((not to_reg and mode == 4) or (to_reg and mode == 3)):
            return [nop(), sym("movem_sp_restore" if to_reg else "movem_sp_save")]
        if (
            not self.legacy and not self.pin and ((to_reg and mode == 3) or (not to_reg and mode == 4))
        ):  # (the shared routine is entered with `add r4,pc,#0`: r4 is D0 when it is pinned)
            # the target layout: the register list runs in a routine shared by every address register (base
            # address in r2, called with `add r4,pc,#0 ; b`, returns with `bx r4`);
            # the opcode's own words copy the base in and the final address back out. The flat sequence (what
            # the checks run) is the same thing written out in line.
            name = "movem_%s_%s" % ("post" if to_reg else "pre", "w" if word else "l")
            if name not in self.routines:
                self.routines[name] = self._movem_routine(to_reg, word)
            flat = Seq([mov(2, ("reg", a))] + self.routines[name][:-1] + [mov(a, ("reg", 2)), bx("lr")])
            flat.shared = [mov(2, ("reg", a)), ("CALL", "al", name), mov(a, ("reg", 2)), bx("lr")]
            return flat
        if to_reg and mode == 3:
            w = [mrs(3)]
            for i in range(8):
                if self.pin and i == 0:
                    # the pinned D0: straight into r4
                    w += [
                        ror1(),
                        (
                            ldrsh(4, a, 2, kind="post", cond="cs")
                            if word
                            else ldr(4, a, 4, kind="post", cond="cs")
                        ),
                    ]
                else:
                    w += [
                        ror1(),
                        (
                            ldrsh(0, a, 2, kind="post", cond="cs")
                            if word
                            else ldr(0, a, 4, kind="post", cond="cs")
                        ),
                        str_(0, 5, D0 + i * 4, cond="cs"),
                    ]
            an = a - 6
            for i in range(8):
                dst = 2 if i == an else 6 + i
                w += [
                    ror1(),
                    (
                        ldrsh(dst, a, 2, kind="post", cond="cs")
                        if word
                        else ldr(dst, a, 4, kind="post", cond="cs")
                    ),
                ]
            # An ends up with the postincremented address even if it is in the list
            return w + [
                msr_f(3),
                bx("lr"),
            ]
        if not to_reg and mode == 4:
            w = [mrs(3), mov(2, ("reg", a))]
            for i in range(7, -1, -1):
                src = 2 if i == a - 6 else 6 + i
                w += [
                    ror1(),
                    (
                        strh(src, a, -2, kind="pre", cond="cs")
                        if word
                        else str_(src, a, -4, kind="pre", cond="cs")
                    ),
                ]
            for i in range(7, -1, -1):
                if self.pin and i == 0:
                    w += [
                        ror1(),
                        (
                            strh(4, a, -2, kind="pre", cond="cs")
                            if word
                            else str_(4, a, -4, kind="pre", cond="cs")
                        ),
                    ]
                else:
                    w += [
                        ror1(),
                        ldrh(0, 5, D0 + i * 4 + 2, cond="cs") if word else ldr(0, 5, D0 + i * 4, cond="cs"),
                        (
                            strh(0, a, -2, kind="pre", cond="cs")
                            if word
                            else str_(0, a, -4, kind="pre", cond="cs")
                        ),
                    ]
            return w + [msr_f(3), bx("lr")]
        first = mov(2, ("reg", a)) if mode == 2 else add(2, a, ("reg", 2)) if mode in (5, 6) else nop()
        return [
            first,
            sym(
                {0x0000: "r2m_word", 0x0040: "r2m_long", 0x0400: "m2r_word", 0x0440: "m2r_long"}[op & 0x0440]
            ),
        ]

    def _movem_routine(self, to_reg, word):
        """MOVEM through the base address in r2 (r1 = the register mask): (An)+ to registers or registers to
        -(An); the address registers are read / written as themselves, the
        caller copies the base in and the final address back (An ends with it even if it is in the list). Ends
        in `bx r4`.
        """
        D0 = self.D0
        ror1 = lambda: mov(1, ("reg", 1, "ror", 1), s=True)
        w = [mrs(3)]
        if to_reg:
            for i in range(8):
                w += [
                    ror1(),
                    ldrsh(0, 2, 2, kind="post", cond="cs") if word else ldr(0, 2, 4, kind="post", cond="cs"),
                    str_(0, 5, D0 + i * 4, cond="cs"),
                ]
            for i in range(8):
                w += [
                    ror1(),
                    (
                        ldrsh(6 + i, 2, 2, kind="post", cond="cs")
                        if word
                        else ldr(6 + i, 2, 4, kind="post", cond="cs")
                    ),
                ]
        else:
            for i in range(7, -1, -1):
                w += [
                    ror1(),
                    (
                        strh(6 + i, 2, -2, kind="pre", cond="cs")
                        if word
                        else str_(6 + i, 2, -4, kind="pre", cond="cs")
                    ),
                ]
            for i in range(7, -1, -1):
                w += [
                    ror1(),
                    ldrh(0, 5, D0 + i * 4 + 2, cond="cs") if word else ldr(0, 5, D0 + i * 4, cond="cs"),
                    strh(0, 2, -2, kind="pre", cond="cs") if word else str_(0, 2, -4, kind="pre", cond="cs"),
                ]
        return w + [msr_f(3), bx(4)]

    def _cpu_addr(self, r, off):
        return add(r, 5, ("imm", off)) if off >= 0 else sub(r, 5, ("imm", -off))

    def _bcd(self, op, name):
        s = op & 7
        d = (op >> 9) & 7
        if op & 8:
            w = [
                ldrb(0, 6 + s, -self.step(1, 6 + s), kind="pre"),
                sub(1, 6 + d, ("imm", self.step(1, 6 + d))),
                mov(6 + d, ("reg", 1)),
            ]
        else:
            # (the pinned D0: the helpers save r4 in cpu->r0 first and restore it last, so a destination
            # pointer to that slot's low byte updates D0)
            w = (self.dload(0, s, 1, False) if s == 0 and self.pin else [ldrb(0, 5, s * 4 + 3 + self.D0)]) + [
                self._cpu_addr(1, self.o["CPU_R0"] + 3 if self.pin and d == 0 else self.D0 + d * 4 + 3)
            ]
        return w + [sym(name)]

    def emit_ABCD(self, op):
        return self._bcd(op, "abcd")

    def emit_SBCD(self, op):
        return self._bcd(op, "sbcd")

    def emit_NBCD(self, op):
        dEA = op & 0x3F
        m = dEA & 0x38
        a = 6 + (dEA & 7)
        w = []
        if m == 0x00:
            w = [
                self._cpu_addr(
                    0, self.o["CPU_R0"] + 3 if self.pin and (dEA & 7) == 0 else self.D0 + (dEA & 7) * 4 + 3
                )
            ]
        elif m == 0x10:
            w = [mov(0, ("reg", a))]
        elif m == 0x18:
            w = [mov(0, ("reg", a)), add(a, a, ("imm", self.step(1, a)))]
        elif m == 0x20:
            w = [sub(a, a, ("imm", self.step(1, a))), mov(0, ("reg", a))]
        elif m in (0x28, 0x30):
            w = [add(0, a, ("reg", 2))]
        elif m == 0x38:
            w = [mov(0, ("reg", 2))]
        return w + [sym("nbcd")]
