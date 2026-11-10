"""The status register, the CCR and the privileged instructions: MOVE to / from SR / CCR / USP, ANDI / ORI / EORI
to SR / CCR, RTE, STOP, RESET.

Part of `gen.Gen`; emitters/__init__.py lists the other groups.
"""

from arm32 import *
from emitters.base import FC_GPIO, Seq


class SrMixin:
    """The status register, the CCR and the privileged instructions: MOVE to / from SR / CCR / USP, ANDI / ORI
    / EORI to SR / CCR, RTE, STOP, RESET.
    """

    # ---- SR / CCR ----
    def _arm_to_68k(self, r):
        return [
            dp("bic", r, r, ("imm", 0xF)),
            orr(r, r, ("imm", 1), cond="cs"),
            orr(r, r, ("imm", 2), cond="vs"),
            orr(r, r, ("imm", 4), cond="eq"),
            orr(r, r, ("imm", 8), cond="mi"),
        ]

    def _to_arm(self, r):
        w = [mov(3, ("imm", 0))]
        for bit, flag in ((8, 0x80000000), (4, 0x40000000), (1, 0x20000000), (2, 0x10000000)):
            w += [dp("tst", 0, r, ("imm", bit)), orr(3, 3, ("imm", flag), cond="ne")]
        return w + [msr_f(3)] + self._ge_from_image(r)

    @staticmethod
    def _ge_from_image(
        r,
    ):  # GE[3:0] := 1111 when bit 4 (X) of the image in r is clear; r3 is scratch; no flags
        return [
            and_(3, r, ("imm", 0x10)),
            eor(3, 3, ("imm", 0x10)),
            lsl(3, 3, 12),
            dp("rsb", 3, 3, ("reg", 3, "lsl", 4)),
            msr_g(3),
        ]

    @staticmethod
    def _x_into(r, tmp):  # bit 4 of the image in r := X (from GE); tmp is scratch; no flags
        return [
            mrs(tmp),
            ubfx(tmp, tmp, 16, 1),
            eor(tmp, tmp, ("imm", 1)),
            dp("bic", r, r, ("imm", 0x10)),
            orr(r, r, ("reg", tmp, "lsl", 4)),
        ]

    def _load_sr(self, r):
        o = self.o
        return (
            [ldrh(r, 5, o["CPU_SR"])]
            + self._x_into(r, 0)
            + self._arm_to_68k(r)
            + [
                dp("tst", 0, r, ("imm", 0x2000)),
                str_(13, 5, o["CPU_SSP"], cond="ne"),
                str_(13, 5, o["CPU_USP"], cond="eq"),
            ]
        )

    def _fc_sync(self, r):
        """FC2 follows the new S bit (the flags of the `tst` of the SR image in r are still live: NE =
        supervisor). GPIO2 CLEARDATAOUT / SETDATAOUT (0x481AC190 / 194) take a 1 bit
        to act on (the BE store of 0x40000 is the LE word 0x400 = GPIO2_10); r3 is the address, x the value (r
        and r0 are not touched: r0 may hold a PC); the dsb orders the pin
        change before the next access to the Amiga window."""
        if not FC_GPIO:
            return []
        x = 1 if r != 1 else 2
        return [
            movw(3, 0xC190),
            movt(3, 0x481A),
            mov(x, ("imm", 0x40000)),
            str_(x, 3, 4, cond="ne"),
            str_(x, 3, 0, cond="eq"),
            dsb(),
        ]

    def _save_sr(self, r, pc_in_r0):
        o = self.o
        return (
            [
                dp("bic", r, r, ("imm", 0x5800)),
                dp("bic", r, r, ("imm", 0xE0)),  # SR bits 14, 12, 11 and 7-5 do not exist: they always read 0
                ldrh(3, 5, o["CPU_SR"]),
                dp("tst", 0, 3, ("imm", 0x2000)),
                str_(13, 5, o["CPU_SSP"], cond="ne"),
                str_(13, 5, o["CPU_USP"], cond="eq"),
                strh(r, 5, o["CPU_SR"]),
                dp("tst", 0, r, ("imm", 0x2000)),
                ldr(13, 5, o["CPU_SSP"], cond="ne"),
                ldr(13, 5, o["CPU_USP"], cond="eq"),
            ]
            + self._fc_sync(r)
            + [
                ldrb(3, 5, o["CPU_IRQ_LEVEL"]),
                cmp_(3, ("imm", 0)),
                sym(
                    "pjit_irq_sr_r0" if pc_in_r0 else "pjit_irq_sr_i" if self.interp else "pjit_irq_sr_d",
                    "ne",
                ),
            ]
            + self._to_arm(r)
        )

    def _is_svc(self):
        return [dp("tst", 0, 2, ("imm", 0x2000)), svc(8, "eq")]

    def _load_cr(self, r):
        # X (bit 4) from GE
        return [mrs(r), ubfx(r, r, 16, 1), eor(r, r, ("imm", 1)), lsl(r, r, 4)] + self._arm_to_68k(r)

    def _save_cr(self, r):
        return [strb(r, 5, self.XB)] + self._to_arm(r)

    def _ccr_sr(self, op, name, sr):
        w = (self._load_sr(2) + self._is_svc()) if sr else self._load_cr(2)
        w += self.alu(name, 1, 2, 32)
        return w + (self._save_sr(2, False) if sr else self._save_cr(2)) + [bx("lr")]

    def emit_MOVE_SR_TO(self, op):
        dEA = 0x40 | (op & 0x3F)
        # the S-bit test inside _load_sr changes N, Z and C (a rotated immediate drives C), and MOVE from SR
        # leaves the CCR alone: keep it
        return [mrs(3)] + self._load_sr(1) + [msr_f(3)] + self.ea_store(dEA, 1, 2, False) + [bx("lr")]

    def emit_MOVE_TO_CCR(self, op):
        r = self.ea_load(0x40 | (op & 0x3F), 1, 1, False)  # the source is a word; only its low byte is used
        return None if r is None else r[0] + self._save_cr(r[1]) + [bx("lr")]

    def emit_MOVE_TO_SR(self, op):
        r = self.ea_load(0x40 | (op & 0x3F), 1, 1, False)
        if r is not None and not self.legacy and (op >> 3) & 7 not in (3, 4):
            # the target layout: the source is read first (modes with no side effect only: the privilege check
            # must come before an (An)+ / -(An) update), one routine does the rest
            return Seq(
                list(r[0])
                + ([mov(1, ("reg", r[1]))] if r[1] != 1 else [])
                + self._load_sr(2)
                + self._is_svc()
                + self._save_sr(1, False)
                + [bx("lr")],
                split=len(r[0]) + (r[1] != 1),
            )
        return (
            None
            if r is None
            else self._load_sr(2) + self._is_svc() + r[0] + self._save_sr(r[1], False) + [bx("lr")]
        )

    def _priv_keep_flags(self):
        # the privilege check, with N Z C left as they were
        return [mrs(3)] + self._load_sr(2) + self._is_svc() + [msr_f(3)]

    def emit_MOVE_TO_USP(self, op):
        return self._priv_keep_flags() + [str_(6 + (op & 7), 5, self.o["CPU_USP"]), bx("lr")]

    def emit_MOVE_USP_TO(self, op):
        return self._priv_keep_flags() + [ldr(6 + (op & 7), 5, self.o["CPU_USP"]), bx("lr")]

    def emit_ANDI_TO_CCR(self, op):
        return self._ccr_sr(op, "and", False)

    def emit_ORI_TO_CCR(self, op):
        return self._ccr_sr(op, "or", False)

    def emit_EORI_TO_CCR(self, op):
        return self._ccr_sr(op, "eor", False)

    def emit_ANDI_TO_SR(self, op):
        return self._ccr_sr(op, "and", True)

    def emit_ORI_TO_SR(self, op):
        return self._ccr_sr(op, "or", True)

    def emit_EORI_TO_SR(self, op):
        return self._ccr_sr(op, "eor", True)

    def emit_RTE(self, op):
        w = self._load_sr(2) + self._is_svc() + [ldrh(2, 13, 2, kind="post"), ldr(0, 13, 4, kind="post")]
        if self.interp and self.pin:
            return (
                w
                + [str_(0, 5, self.o["CPU_IPC"])]
                + self._save_sr(2, False)
                + [ldr(0, 5, self.o["CPU_IPC"]), sym("pjit_i_chk_pc")]
            )
        if self.interp:
            # (the SR write may poll for an interrupt: cpu->ipc must already name the target)
            return (
                w
                + [mov(4, ("reg", 0)), str_(4, 5, self.o["CPU_IPC"])]
                + self._save_sr(2, False)
                + [sym("pjit_i_chk_pc")]
            )
        return w + self._save_sr(2, True) + [sym("pjit_goto")]

    def emit_STOP(self, op):
        return self._load_sr(2) + self._is_svc() + self._save_sr(1, False) + [svc(0x100), bx("lr")]

    def emit_RESET(self, op):
        return self._priv_keep_flags() + [bx("lr")]
