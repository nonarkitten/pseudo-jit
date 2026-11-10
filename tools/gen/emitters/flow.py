"""Control flow: Bcc, BRA, BSR, DBcc, JMP, JSR, RTS, RTR, TRAP, TRAPV and the illegal / line-A / line-F / NOP
cases.

Part of `gen.Gen`; emitters/__init__.py lists the other groups.
"""

from arm32 import *
from emitters.base import Seq


class FlowMixin:
    """Control flow: Bcc, BRA, BSR, DBcc, JMP, JSR, RTS, RTR, TRAP, TRAPV and the illegal / line-A / line-F /
    NOP cases.
    """

    ARMCC = ["al", "nv", "hi", "ls", "cc", "cs", "ne", "eq", "vc", "vs", "pl", "mi", "ge", "lt", "gt", "le"]

    NUMCC = {v: k for k, v in COND.items() if k not in ("hs", "lo")}

    # ---- control flow (the DR path; branches to runtime routines stay symbolic) ----
    @staticmethod
    def mov_signed(r, v):
        return mov(r, ("imm", v)) if v >= 0 else mvn(r, ("imm", ~v & 0xFFFFFFFF))

    def _arm_cc(self, op):
        return self.ARMCC[(op >> 8) & 15]

    def emit_Bcc(self, op):
        off = (op & 0xFF) - 256 if op & 0x80 else op & 0xFF
        w = [self.mov_signed(1, off) if off else nop()]
        cc = self._arm_cc(op)
        if cc == "nv":
            cc = "al"
        # the displacement is added to the guest PC (r4; with the pinned D0: loaded from cpu->ipc into r0,
        # which is the new PC for pjit_i_chk_pc)
        if self.interp:
            pc = 0 if self.pin else 4
            if self.pin:
                w.append(ldr(0, 5, self.o["CPU_IPC"]))
            if cc == "hi":
                w += [b(2, "cs"), add(pc, pc, ("reg", 1), cond="ne")]
            elif cc == "ls":
                w += [add(pc, pc, ("reg", 1), cond="cs"), b(2, "cs"), add(pc, pc, ("reg", 1), cond="eq")]
            else:
                w.append(add(pc, pc, ("reg", 1), cond=cc))
            return w + [sym("pjit_i_chk_pc")]
        if cc == "hi":
            w += [b(2, "cs"), sym("pjit_d_bra", "ne")]
        elif cc == "ls":
            w += [sym("pjit_d_bra", "cs"), sym("pjit_d_bra", "eq")]
        else:
            w.append(sym("pjit_d_bra", cc))
        return w + [bx("lr")]

    emit_BRA = emit_Bcc

    def emit_BSR(self, op):
        off = (op & 0xFF) - 256 if op & 0x80 else op & 0xFF
        if self.interp:
            pc = 0 if self.pin else 4
            w = (
                [nop() if off == 0 else self.mov_signed(1, off)]
                + ([ldr(0, 5, self.o["CPU_IPC"])] if self.pin else [])
                + [str_(pc, 13, -4, kind="pre"), add(pc, pc, ("reg", 1))]
            )
            if off == 0:
                w.append(sub(pc, pc, ("imm", 2)))
            return w + [sym("pjit_i_chk_pc")]
        if off == 0:
            return [nop(), sym("pjit_d_bsr2p")]
        return [self.mov_signed(1, off), sym("pjit_d_bsr")]

    def emit_DBcc(self, op):
        cc = self._arm_cc(op)
        w = [nop()]
        if cc == "hi":
            w += [b(2, "cs"), bx("lr", "ne")]
        elif cc == "ls":
            w += [bx("lr", "cs"), bx("lr", "eq")]
        elif cc == "nv":
            w.append(nop())
        elif cc == "al":
            w.append(bx("lr"))
        else:
            w.append(bx("lr", cc))
        sEA = 0x40 | (op & 7)
        if not self.legacy and not (self.pin and (op & 7) == 0):
            # the target layout: one own word names the counter register, one routine per condition (and
            # world) does the rest
            w = w[1:] + [
                add(2, 5, ("reg", 2, "lsl", 2)),
                ldrsh(0, 2, self.D0 + 2),
                sub(0, 0, ("imm", 1)),
                strh(0, 2, self.D0 + 2),
            ]
            reg_ = 0
            w += [add(3, reg_, ("imm", 1)), clz(3, 3), lsr(3, 3, 5), sub(1, 1, ("imm", 2))]
            if self.interp:
                pc = 0 if self.pin else 4
                return Seq(
                    [mov(2, ("imm", op & 7))]
                    + w
                    + ([ldr(0, 5, self.o["CPU_IPC"])] if self.pin else [])
                    + [
                        add(15, 15, ("reg", 3, "lsl", 3)),
                        nop(),
                        add(pc, pc, ("reg", 1)),
                        sym("pjit_i_chk_pc"),
                        bx("lr"),
                    ],
                    split=1,
                )
            return Seq(
                [mov(2, ("imm", op & 7))]
                + w
                + [add(15, 15, ("reg", 3, "lsl", 2)), nop(), sym("pjit_d_bra"), bx("lr")],
                split=1,
            )
        r = self.ea_load(sEA, 0, 0, True)
        reg_ = r[1]
        w += r[0] + [sub(reg_, reg_, ("imm", 1))] + self.ea_store(sEA, reg_, 0, True)
        # the counter wrapped to -1: fall through to the next instruction, else take the branch. DBcc leaves
        # the CCR alone, so no `cmn`: r3 = 1 when counter + 1 is 0,
        # and `add pc` skips the branch (the nop is the word pc + 4 steps over)
        w += [add(3, reg_, ("imm", 1)), clz(3, 3), lsr(3, 3, 5), sub(1, 1, ("imm", 2))]
        if self.interp:
            pc = 0 if self.pin else 4
            return (
                w
                + ([ldr(0, 5, self.o["CPU_IPC"])] if self.pin else [])
                + [
                    add(15, 15, ("reg", 3, "lsl", 3)),
                    nop(),
                    add(pc, pc, ("reg", 1)),
                    sym("pjit_i_chk_pc"),
                    bx("lr"),
                ]
            )
        # pc reads 8 ahead: 0 lands on the branch, 1 on the return
        return w + [
            add(15, 15, ("reg", 3, "lsl", 2)),
            nop(),
            sym("pjit_d_bra"),
            bx("lr"),
        ]

    emit_DBRA = emit_DBcc

    def emit_JMP(self, op):
        mode = (op >> 3) & 7
        r = 6 + (op & 7)
        first = (
            mov(0, ("reg", r))
            if mode == 2
            else add(0, r, ("reg", 1)) if mode in (5, 6) else mov(0, ("reg", 1))
        )
        if self.interp:
            return (
                [first, nop(), sym("pjit_i_chk_pc")]
                if self.pin
                else [first, mov(4, ("reg", 0)), sym("pjit_i_chk_pc")]
            )
        return [first, sym("pjit_goto")]

    def emit_JSR(self, op):
        mode = (op >> 3) & 7
        r = 6 + (op & 7)
        first = mov(1, ("reg", r)) if mode == 2 else add(1, r, ("reg", 1)) if mode in (5, 6) else nop()
        if self.interp and self.pin:
            return [
                first,
                ldr(0, 5, self.o["CPU_IPC"]),
                str_(0, 13, -4, kind="pre"),
                mov(0, ("reg", 1)),
                sym("pjit_i_chk_pc"),
            ]
        if self.interp:
            return [first, str_(4, 13, -4, kind="pre"), mov(4, ("reg", 1)), sym("pjit_i_chk_pc")]
        return [first, sym("pjit_d_jsr")]

    def _goto(self):
        # (the target is in r0; with the pinned D0 pjit_i_chk_pc takes it from there)
        return (
            ([nop(), sym("pjit_i_chk_pc")] if self.pin else [mov(4, ("reg", 0)), sym("pjit_i_chk_pc")])
            if self.interp
            else [sym("pjit_goto")]
        )

    def emit_RTS(self, op):
        return [ldr(0, 13, 4, kind="post")] + self._goto()

    def emit_RTR(self, op):
        w = [ldrh(2, 13, 2, kind="post"), strb(2, 5, self.XB), mov(3, ("imm", 0))]
        for bit, flag in ((8, 0x80000000), (4, 0x40000000), (1, 0x20000000), (2, 0x10000000)):
            w += [dp("tst", 0, 2, ("imm", bit)), orr(3, 3, ("imm", flag), cond="ne")]
        return w + [msr_f(3)] + self._ge_from_image(2) + [ldr(0, 13, 4, kind="post")] + self._goto()

    def emit_TRAP(self, op):
        return [svc(32 + (op & 0xF)), bx("lr")]

    def emit_TRAPV(self, op):
        return [svc(7, "vs"), bx("lr")]

    def emit_ILLEGAL(self, op):
        return [svc(4), bx("lr")]

    def emit_ALINE(self, op):
        return [svc(10), bx("lr")]

    def emit_FLINE(self, op):
        return [svc(11), bx("lr")]

    def emit_NOP(self, op):
        return [nop(), bx("lr")]
