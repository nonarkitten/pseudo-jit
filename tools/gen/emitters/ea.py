"""Effective addresses and the data registers.

Part of `gen.Gen`; emitters/__init__.py lists the other groups.
"""

from arm32 import *


class EAMixin:
    """Effective addresses and the data registers."""

    @staticmethod
    def _tmp(*excl):
        for t in (3, 0, 1, 2):
            if t not in excl:
                return t

    def _ld_split(self, kind, dReg, base, iReg=None, off=0):
        """A long load from [base + off] (kind 'off'), or from [base + iReg] (kind 'reg') as two halfword
        loads; base and iReg survive, dReg may equal either.
        """
        if kind == "reg":
            t = self._tmp(dReg)
            return [
                add(t, base, ("reg", iReg)),
                ldrh(dReg, t, 0),
                ldrh(t, t, 2),
                orr(dReg, t, ("reg", dReg, "lsl", 16)),
            ]
        t = self._tmp(dReg, base)
        return [ldrh(t, base, off + 2), ldrh(dReg, base, off), orr(dReg, t, ("reg", dReg, "lsl", 16))]

    def _st_split(self, kind, sReg, base, iReg=None, off=0):
        if kind == "reg":
            a = self._tmp(sReg, base, iReg)
            h = self._tmp(sReg, base, iReg, a)
            return [add(a, base, ("reg", iReg)), lsr(h, sReg, 16), strh(h, a, 0), strh(sReg, a, 2)]
        h = self._tmp(sReg, base)
        return [lsr(h, sReg, 16), strh(h, base, off), strh(sReg, base, off + 2)]

    # ---- effective addresses ----
    def step(self, width, reg):
        """How far (An)+ / -(An) moves An: a byte through A7 (ARM r13) moves it by 2, so the stack stays
        word-aligned.
        """
        return 2 if (width == 1 and reg == 13) else width

    def dload(self, rd, n, width=4, signed=True):
        """Words that put data register n (its low `width` bytes, sign- or zero-extended) into rd: a load from
        cpu_t, or, for the pinned D0, a register move / extend.
        """
        if self.pin and n == 0:
            if width == 4:
                return [mov(rd, ("reg", 4))]
            return [
                {(2, True): sxth, (2, False): uxth, (1, True): sxtb, (1, False): uxtb}[(width, signed)](rd, 4)
            ]
        ld = {
            (4, True): ldr,
            (4, False): ldr,
            (2, True): ldrsh,
            (2, False): ldrh,
            (1, True): ldrsb,
            (1, False): ldrb,
        }[(width, signed)]
        return [ld(rd, 5, self.D0 + n * 4 + (4 - width))]

    def dstore(self, rs, n, width=4):
        """Words that write the low `width` bytes of rs into data register n (the rest of the register is
        kept).
        """
        if self.pin and n == 0:
            if width == 4:
                return [] if rs == 4 else [mov(4, ("reg", rs))]
            return [bfi(4, rs, 0, 8 * width)]
        return [{1: strb, 2: strh, 4: str_}[width](rs, 5, self.D0 + n * 4 + (4 - width))]

    def ea_load(self, sEA, dReg, iReg, rmw, inplace=False):
        """-> (words, register holding the value) or None for an invalid EA. `inplace`: the caller takes whatever
        register holds the value and neither relies on it being dReg nor
        changes it unless it writes the result back to the same place (the pinned D0 is then returned as r4
        itself).
        """
        width = 1 << (sEA >> 6)
        mode = sEA & 0x38
        w = []
        ls = {1: (ldrsb, ldrsb_r), 2: (ldrsh, ldrsh_r), 4: (ldr, ldr_r)}[width]
        if mode == 0x00:
            if self.pin and (sEA & 7) == 0:
                if inplace and width == 4:
                    return [], 4
                return self.dload(dReg, 0, width), dReg
            w.append(ls[0](dReg, 5, self.D0 + (sEA & 7) * 4 + (4 - width)))
            return w, dReg
        if mode == 0x08:
            reg = (sEA & 7) + 6
            if width == 1:
                return None
            if width == 2:
                w.append(sxth(dReg, reg))
                return w, dReg
            return w, reg
        sp = self.split_long and width == 4 and mode >= 0x10
        if mode == 0x18 and (rmw or dReg == (sEA & 7) + 6):
            mode = 0x10
        if mode == 0x10:
            if sp:
                return self._ld_split("off", dReg, (sEA & 7) + 6), dReg
            w.append(ls[0](dReg, (sEA & 7) + 6, 0))
            return w, dReg
        if mode == 0x18:
            if sp:
                reg = (sEA & 7) + 6
                return self._ld_split("off", dReg, reg) + [add(reg, reg, ("imm", 4))], dReg
            w.append(ls[0](dReg, (sEA & 7) + 6, self.step(width, (sEA & 7) + 6), kind="post"))
            return w, dReg
        if mode == 0x20:
            reg = (sEA & 7) + 6
            if sp:
                if dReg == reg:
                    return self._ld_split("off", dReg, reg, off=-4), dReg
                return [sub(reg, reg, ("imm", 4))] + self._ld_split("off", dReg, reg), dReg
            w.append(ls[0](dReg, reg, -self.step(width, reg), kind="off" if dReg == reg else "pre"))
            return w, dReg
        if mode in (0x28, 0x30):
            if sp:
                return self._ld_split("reg", dReg, (sEA & 7) + 6, iReg), dReg
            w.append(ls[1](dReg, (sEA & 7) + 6, iReg))
            return w, dReg
        if mode == 0x38:
            if (sEA & 7) == 4:  # immediate: already in iReg
                if width == 1:
                    w.append(sxtb(iReg, iReg))
                return w, iReg
            if (sEA & 7) > 4:
                return None
            if sp:
                return self._ld_split("off", dReg, iReg), dReg
            w.append(ls[0](dReg, iReg, 0))
            return w, dReg
        return None

    def ea_store(self, dEA, sReg, iReg, rmw):
        width = 1 << (dEA >> 6)
        mode = dEA & 0x38
        dReg = dEA & 7
        w = []
        st = {1: (strb, strb_r), 2: (strh, strh_r), 4: (str_, str_r)}[width]
        if mode == 0x00:
            if self.pin and dReg == 0:
                return self.dstore(sReg, 0, width)
            w.append(st[0](sReg, 5, self.D0 + dReg * 4 + (4 - width)))
            return w
        if mode == 0x08:
            if width == 4 and sReg in (dReg, dReg + 6):
                # already in place: no `mov r6, r6`
                return w
            dReg += 6
            if width == 2:
                w.append(sxth(dReg, sReg))
            elif width == 4:
                w.append(mov(dReg, ("reg", sReg)))
            return w
        sp = self.split_long and width == 4 and mode >= 0x10
        if mode == 0x20 and rmw:
            mode = 0x10
        if mode == 0x10:
            if sp:
                return self._st_split("off", sReg, dReg + 6)
            w.append(st[0](sReg, dReg + 6, 0))
            return w
        if mode == 0x18 and sp:
            dReg += 6
            return self._st_split("off", sReg, dReg) + [add(dReg, dReg, ("imm", 4))]
        if mode == 0x20 and sp:
            dReg += 6
            return self._st_split("off", sReg, dReg, off=-4) + [sub(dReg, dReg, ("imm", 4))]
        if mode == 0x18:
            dReg += 6
            if sReg == dReg:  # MOVE An,(An)+: store, then add
                w += [st[0](sReg, dReg, 0), add(dReg, dReg, ("imm", self.step(width, dReg)))]
            else:
                w.append(st[0](sReg, dReg, self.step(width, dReg), kind="post"))
            return w
        if mode == 0x20:
            dReg += 6
            if sReg == dReg:
                w += [
                    st[0](sReg, dReg, -self.step(width, dReg)),
                    sub(dReg, dReg, ("imm", self.step(width, dReg))),
                ]
            else:
                w.append(st[0](sReg, dReg, -self.step(width, dReg), kind="pre"))
            return w
        if mode in (0x28, 0x30):
            if sp:
                return self._st_split("reg", sReg, dReg + 6, iReg)
            w.append(st[1](sReg, dReg + 6, iReg))
            return w
        if mode == 0x38:
            if sp:
                return self._st_split("off", sReg, iReg)
            w.append(st[0](sReg, iReg, 0))
            return w
        return w

    # ---- the instruction groups ----
    def ea_addr(self, dEA, rd, iReg):
        """The address of an operand in rd (the split layout's read-modify-write: the routine loads and stores
        through it, so a data register is just an address in the cpu state).
        -> the words that form it (the update of An for (An)+ / -(An) included), or None. `iReg` holds the
        extension value (a displacement, or an absolute address).
        """
        width = 1 << (dEA >> 6)
        mode = dEA & 0x38
        n = dEA & 7
        if mode == 0x00:
            if self.pin and n == 0:
                return None  # the pinned D0 has no address: the callers fall back to their flat forms
            off = self.D0 + n * 4 + (4 - width)
            return [add(rd, 5, ("imm", off)) if off >= 0 else sub(rd, 5, ("imm", -off))]
        if mode == 0x10:
            return [mov(rd, ("reg", n + 6))]
        if mode == 0x18:
            return [mov(rd, ("reg", n + 6)), add(n + 6, n + 6, ("imm", self.step(width, n + 6)))]
        if mode == 0x20:
            return [sub(n + 6, n + 6, ("imm", self.step(width, n + 6))), mov(rd, ("reg", n + 6))]
        if mode in (0x28, 0x30):
            return [add(rd, n + 6, ("reg", iReg))]
        if mode == 0x38 and n in (0, 1):
            return [] if iReg == rd else [mov(rd, ("reg", iReg))]
        return None
