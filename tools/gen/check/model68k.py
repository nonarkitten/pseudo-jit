"""An independent 68000 reference for the families the generator covers (MOVE, MOVEA, MOVEQ,
ADD/SUB/AND/OR/EOR/CMP, the A and I
forms), written from the architecture manual rather than from any emulator, to triangulate: when the generated
code and the
oracle (golden/mcl68k.c) disagree, this says which is right. Deliberately plain; the 68000 facts it encodes:
byte
post-increment / pre-decrement of A7 moves it by 2; ADDA/SUBA/CMPA.W sign-extend the source and use the whole
register; MOVE and the
logical ops clear V and C and leave X; ADD/SUB set X from C; the source operand of MOVE is read before the
destination's
side effects."""

M32 = 0xFFFFFFFF


def sext(v, bits):
    v &= (1 << bits) - 1
    return v - (1 << bits) if v >> (bits - 1) else v


class Model:
    def __init__(self, d, a, ccr, area, area_base, ext, op_addr):
        self.d = list(d)
        self.a = list(a)
        self.ccr = ccr
        self.mem = bytearray(area)
        self.base = area_base
        self.ext = list(ext)
        self.ei = 0
        self.op_addr = op_addr

    # -- memory (big-endian) --
    def rd(self, addr, w):
        o = addr - self.base
        return int.from_bytes(self.mem[o : o + w], "big")

    def wr(self, addr, w, v):
        o = addr - self.base
        self.mem[o : o + w] = (v & ((1 << (8 * w)) - 1)).to_bytes(w, "big")

    def next_ext(self):
        v = self.ext[self.ei]
        self.ei += 1
        return v

    def ext_addr(self):
        return self.op_addr + 2 + 2 * self.ei

    # -- an effective address: returns ('dn', n) | ('an', n) | ('mem', addr) | ('val', value) --
    def ea(self, mode, reg, w):
        if mode == "dn":
            return ("dn", reg)
        if mode == "an":
            return ("an", reg)
        step = 2 if (w == 1 and reg == 7) else w
        if mode == "(an)":
            return ("mem", self.a[reg] & M32)
        if mode == "(an)+":
            ad = self.a[reg]
            self.a[reg] = (ad + step) & M32
            return ("mem", ad)
        if mode == "-(an)":
            self.a[reg] = (self.a[reg] - step) & M32
            return ("mem", self.a[reg])
        if mode == "d16":
            return ("mem", (self.a[reg] + sext(self.next_ext(), 16)) & M32)
        if mode in ("idx", "pcidx"):
            ea_at = self.ext_addr()
            e = self.next_ext()
            x = e >> 12
            xv = self.a[x - 8] if x >= 8 else self.d[x]
            xv = xv if e & 0x800 else sext(xv, 16)
            base = self.a[reg] if mode == "idx" else ea_at
            return ("mem", (base + sext(e, 8) + xv) & M32)
        if mode == "absw":
            return ("mem", sext(self.next_ext(), 16) & M32)
        if mode == "absl":
            hi = self.next_ext()
            lo = self.next_ext()
            return ("mem", hi << 16 | lo)
        if mode == "pcd16":
            ea_at = self.ext_addr()
            return ("mem", (ea_at + sext(self.next_ext(), 16)) & M32)
        if mode == "imm":
            if w == 4:
                hi = self.next_ext()
                lo = self.next_ext()
                return ("val", hi << 16 | lo)
            return ("val", self.next_ext() & ((1 << 8 * w) - 1))

    def read(self, e, w):
        k, x = e
        if k == "dn":
            return self.d[x] & ((1 << 8 * w) - 1)
        if k == "an":
            return self.a[x] & ((1 << 8 * w) - 1)
        if k == "mem":
            return self.rd(x, w)
        return x & ((1 << 8 * w) - 1)

    def write(self, e, w, v):
        k, x = e
        mask = (1 << 8 * w) - 1
        if k == "dn":
            self.d[x] = (self.d[x] & ~mask & M32) | (v & mask)
        elif k == "an":
            self.a[x] = sext(v, 8 * w) & M32 if w < 4 else v & M32
        else:
            self.wr(x, w, v)

    def flags_nz(self, r, w):
        m = (1 << 8 * w) - 1
        r &= m
        return (8 if r >> (8 * w - 1) else 0) | (4 if r == 0 else 0)

    def alu(self, op, s, dv, w):
        """-> (result, new ccr). s, dv: unsigned at width w."""
        bits = 8 * w
        m = (1 << bits) - 1
        x = self.ccr & 0x10
        if op == "add":
            r = s + dv
            res = r & m
            c = r >> bits
            v = ((~(s ^ dv) & (s ^ res)) >> (bits - 1)) & 1
            return res, self.flags_nz(res, w) | v << 1 | c | (0x10 if c else 0)
        if op in ("sub", "cmp"):
            r = dv - s
            res = r & m
            c = 1 if s > dv else 0
            v = (((dv ^ s) & (dv ^ res)) >> (bits - 1)) & 1
            return res, self.flags_nz(res, w) | v << 1 | c | ((0x10 if c else 0) if op == "sub" else x)
        res = {"and": dv & s, "or": dv | s, "eor": dv ^ s}[op] & m
        return res, self.flags_nz(res, w) | x


def shift(kind, v, n, bits, ccr):
    """One bit at a time, straight from the manual's pictures. -> (result, ccr)."""
    m = (1 << bits) - 1
    msb = bits - 1
    x = 1 if ccr & 0x10 else 0
    c = 0
    vflag = 0
    if n == 0:
        c = x if kind.startswith("rox") else 0
    for _ in range(n):
        hi = v >> msb & 1
        lo = v & 1
        if kind == "asl":
            v = v << 1 & m
            vflag |= hi ^ (v >> msb & 1)
            c = x = hi
        elif kind == "lsl":
            v = v << 1 & m
            c = x = hi
        elif kind == "asr":
            v = v >> 1 | hi << msb
            c = x = lo
        elif kind == "lsr":
            v = v >> 1
            c = x = lo
        elif kind == "rol":
            v = (v << 1 | hi) & m
            c = hi
        elif kind == "ror":
            v = v >> 1 | lo << msb
            c = lo
        elif kind == "roxl":
            v = (v << 1 | x) & m
            c = x = hi
        elif kind == "roxr":
            v = v >> 1 | x << msb
            c = x = lo
    nz = (8 if v >> msb & 1 else 0) | (4 if v == 0 else 0)
    return v, nz | vflag << 1 | c | (0x10 if x else 0)


class Res(tuple):
    """(d, a, ccr, area) plus the SR's system byte and the user stack pointer (the supervisor-state
    instructions change them).
    """

    sr_hi = 0x2700
    usp = 0


LAST = {}


def run_case(case, name, op, rl, area_base, op_addr):
    LAST.clear()
    r = _run_case(case, name, op, rl, area_base, op_addr)
    if isinstance(r, tuple):
        r = Res(r)
        r.sr_hi = LAST.get("sr_hi", 0x2700)
        r.usp = LAST.get("usp", 0)
    return r


def _sr_write(m, new):
    """Install a new SR (the system byte and the CCR), banking A7 if the privilege level changed (we start
    supervisor).
    """
    old_s = LAST.get("sr_hi", 0x2700) >> 13 & 1
    new_s = new >> 13 & 1
    usp = LAST.get("usp", 0)
    if old_s and not new_s:
        LAST["ssp"] = m.a[7]
        m.a[7] = usp
    elif not old_s and new_s:
        LAST["usp"] = m.a[7]
        m.a[7] = LAST.get("ssp", 0)
    LAST["sr_hi"] = new & 0xA700
    m.ccr = new & 0x1F
    if not new_s:
        LAST["usp"] = m.a[7]


def _sr_value(m):
    return (LAST.get("sr_hi", 0x2700) | m.ccr) & 0xA71F


def _run_case(case, name, op, rl, area_base, op_addr):
    """Executes one covered instruction on a copy of `case` (sim_test.Case). Returns (d, a, ccr, area) or None
    if not modelled.
    """
    import re

    m = Model(case.d, case.a, case.ccr, case.area, area_base, case.ext, op_addr)
    mm = re.match(r"emit_(ADD|SUB|AND|OR|EOR|CMP)(BW|W|L|B)$", name)
    if mm:
        kind = mm.group(1).lower()
        w = 1 << ((op >> 6) & 3)
        mode = rl[0][1]
        reg = op & 7
        dn = (op >> 9) & 7
        if kind == "cmp" or (kind != "eor" and not op & 0x100):  # <ea>,Dn
            s = m.read(m.ea(mode, reg, w), w)
            dv = m.d[dn] & ((1 << 8 * w) - 1)
            res, m.ccr = m.alu(kind, s, dv, w)
            if kind != "cmp":
                m.write(("dn", dn), w, res)
        else:  # Dn,<ea>
            s = m.d[dn] & ((1 << 8 * w) - 1)
            e = m.ea(mode, reg, w)
            dv = m.read(e, w)
            res, m.ccr = m.alu(kind, s, dv, w)
            m.write(e, w, res)
        return m.d, m.a, m.ccr, bytes(m.mem)
    mm = re.match(r"emit_(ADDA|SUBA|CMPA)(W|L)$", name)
    if mm:
        kind = mm.group(1)[:-1].lower()
        w = 2 if mm.group(2) == "W" else 4
        s = m.read(m.ea(rl[0][1], op & 7, w), w)
        s = sext(s, 16) & M32 if w == 2 else s
        an = (op >> 9) & 7
        if kind == "cmp":
            _, m.ccr = m.alu("cmp", s, m.a[an] & M32, 4)
        else:
            m.a[an] = (m.a[an] + s if kind == "add" else m.a[an] - s) & M32
        return m.d, m.a, m.ccr, bytes(m.mem)
    mm = re.match(r"emit_(ADDI|SUBI|ANDI|ORI|EORI|CMPI)(BW|L)$", name)
    if mm:
        kind = mm.group(1)[:-1].lower()
        w = 1 << ((op >> 6) & 3)
        s = m.read(m.ea("imm", 4, w), w)
        e = m.ea(rl[1][1], op & 7, w)
        dv = m.read(e, w)
        res, m.ccr = m.alu(kind, s, dv, w)
        if kind != "cmp":
            m.write(e, w, res)
        return m.d, m.a, m.ccr, bytes(m.mem)
    mm = re.match(r"emit_MOVE(B|W|L)$", name)
    if mm:
        w = {"B": 1, "W": 2, "L": 4}[mm.group(1)]
        s = m.read(m.ea(rl[0][1], op & 7, w), w)
        e = m.ea(rl[1][1], (op >> 9) & 7, w)
        m.write(e, w, s)
        m.ccr = m.flags_nz(s, w) | (m.ccr & 0x10)
        return m.d, m.a, m.ccr, bytes(m.mem)
    mm = re.match(r"emit_MOVEA(W|L)$", name)
    if mm:
        w = 2 if mm.group(1) == "W" else 4
        s = m.read(m.ea(rl[0][1], op & 7, w), w)
        m.a[(op >> 9) & 7] = sext(s, 16) & M32 if w == 2 else s
        return m.d, m.a, m.ccr, bytes(m.mem)
    mm = re.match(r"emit_(AS|LS|ROX|RO)(dBW|dL|d)$", name)
    if mm:
        kind = mm.group(1).lower() + ("l" if op & 0x100 else "r")
        if mm.group(2) == "d":  # memory: a word, by one bit
            e = m.ea(rl[0][1], op & 7, 2)
            v = m.read(e, 2)
            res, m.ccr = shift(kind, v, 1, 16, m.ccr)
            m.write(e, 2, res)
        else:
            bits = 8 << ((op >> 6) & 3)
            dn = op & 7
            n = (op >> 9) & 7
            cnt = (m.d[n] & 63) if op & 0x20 else (n or 8)
            v = m.d[dn] & ((1 << bits) - 1)
            res, m.ccr = shift(kind, v, cnt, bits, m.ccr)
            m.write(("dn", dn), bits // 8, res)
        return m.d, m.a, m.ccr, bytes(m.mem)
    mm = re.match(r"emit_(ADDQ|SUBQ)(BW|L|AW)$", name)
    if mm:
        kind = mm.group(1)[:-1].lower()
        w = 4 if mm.group(2) == "AW" else 1 << ((op >> 6) & 3)
        q = ((op >> 9) & 7) or 8
        e = m.ea(rl[0][1], op & 7, w)
        if e[0] == "an":
            m.a[e[1]] = (m.a[e[1]] + q if kind == "add" else m.a[e[1]] - q) & M32
        else:
            res, m.ccr = m.alu(kind, q, m.read(e, w), w)
            m.write(e, w, res)
        return m.d, m.a, m.ccr, bytes(m.mem)
    mm = re.match(r"emit_(CLR|NEG|NOT|TST)(BW|L)$", name)
    if mm:
        kind = mm.group(1)
        w = 1 << ((op >> 6) & 3)
        msk = (1 << 8 * w) - 1
        e = m.ea(rl[0][1], op & 7, w)
        dv = m.read(e, w)
        x = m.ccr & 0x10
        if kind == "NEG":
            res, m.ccr = m.alu("sub", dv, 0, w)
            m.write(e, w, res)
        elif kind == "CLR":
            m.write(e, w, 0)
            m.ccr = 4 | x
        elif kind == "NOT":
            res = ~dv & msk
            m.write(e, w, res)
            m.ccr = m.flags_nz(res, w) | x
        else:
            m.ccr = m.flags_nz(dv, w) | x
        return m.d, m.a, m.ccr, bytes(m.mem)
    if name in ("emit_EXTW", "emit_EXTL"):
        n = op & 7
        x = m.ccr & 0x10
        if name == "emit_EXTW":
            m.d[n] = (m.d[n] & 0xFFFF0000) | (sext(m.d[n], 8) & 0xFFFF)
            m.ccr = m.flags_nz(m.d[n], 2) | x
        else:
            m.d[n] = sext(m.d[n], 16) & M32
            m.ccr = m.flags_nz(m.d[n], 4) | x
        return m.d, m.a, m.ccr, bytes(m.mem)
    if name == "emit_SWAP":
        n = op & 7
        m.d[n] = (m.d[n] << 16 | m.d[n] >> 16) & M32
        m.ccr = m.flags_nz(m.d[n], 4) | (m.ccr & 0x10)
        return m.d, m.a, m.ccr, bytes(m.mem)
    if name == "emit_EXG":
        rx = (op >> 9) & 7
        ry = op & 7
        k = (op >> 3) & 0x1F
        if k == 0x08:
            m.d[rx], m.d[ry] = m.d[ry], m.d[rx]
        elif k == 0x09:
            m.a[rx], m.a[ry] = m.a[ry], m.a[rx]
        elif k == 0x11:
            m.d[rx], m.a[ry] = m.a[ry], m.d[rx]
        return m.d, m.a, m.ccr, bytes(m.mem)
    if name == "emit_LEA":
        e = m.ea(rl[0][1], op & 7, 4)
        m.a[(op >> 9) & 7] = e[1] & M32
        return m.d, m.a, m.ccr, bytes(m.mem)
    if name in ("emit_MULS", "emit_MULU"):
        s = m.read(m.ea(rl[0][1], op & 7, 2), 2)
        n = (op >> 9) & 7
        dv = m.d[n] & 0xFFFF
        if name == "emit_MULS":
            s, dv = sext(s, 16), sext(dv, 16)
        m.d[n] = s * dv & M32
        m.ccr = m.flags_nz(m.d[n], 4) | (m.ccr & 0x10)
        return m.d, m.a, m.ccr, bytes(m.mem)
    mm = re.match(r"emit_CMPM(B|W|L)$", name)
    if mm:
        w = {"B": 1, "W": 2, "L": 4}[mm.group(1)]
        s = m.read(m.ea("(an)+", op & 7, w), w)
        dv = m.read(m.ea("(an)+", (op >> 9) & 7, w), w)
        _, m.ccr = m.alu("cmp", s, dv, w)
        return m.d, m.a, m.ccr, bytes(m.mem)
    if name in ("emit_BCHG", "emit_BCLR", "emit_BSET", "emit_BTST"):
        n = m.d[(op >> 9) & 7] if op & 0x100 else m.read(m.ea("imm", 4, 1), 1)
        e = m.ea(rl[-1][1], op & 7, 4 if rl[-1][1] == "dn" else 1)
        w = 4 if e[0] == "dn" else 1
        n %= 8 * w
        v = m.read(e, w)
        bit = 1 << n
        m.ccr = (m.ccr & ~4) | (0 if v & bit else 4)
        if name != "emit_BTST":
            m.write(e, w, {"emit_BCHG": v ^ bit, "emit_BCLR": v & ~bit, "emit_BSET": v | bit}[name])
        return m.d, m.a, m.ccr, bytes(m.mem)
    if name == "emit_Scc":
        c = m.ccr
        N, Z, V, C = bool(c & 8), bool(c & 4), bool(c & 2), bool(c & 1)
        t = [
            True,
            False,
            not C and not Z,
            C or Z,
            not C,
            C,
            not Z,
            Z,
            not V,
            V,
            not N,
            N,
            N == V,
            N != V,
            not Z and N == V,
            Z or N != V,
        ][(op >> 8) & 15]
        m.write(m.ea(rl[0][1], op & 7, 1), 1, 0xFF if t else 0)
        return m.d, m.a, m.ccr, bytes(m.mem)
    mm = re.match(r"emit_(ADDX|SUBX|NEGX)(BW|L)$", name)
    if mm:
        kind = mm.group(1)
        w = 1 << ((op >> 6) & 3)
        bits = 8 * w
        msk = (1 << bits) - 1
        x = 1 if m.ccr & 0x10 else 0
        if kind == "NEGX":
            e = m.ea(rl[0][1], op & 7, w)
            sv = m.read(e, w)
            dv = 0
            sv, dv = sv, 0
            dst = e
        else:
            if op & 8:
                se = m.ea("-(an)", op & 7, w)
                de = m.ea("-(an)", (op >> 9) & 7, w)
            else:
                se = ("dn", op & 7)
                de = ("dn", (op >> 9) & 7)
            sv = m.read(se, w)
            dv = m.read(de, w)
            dst = de
        if kind == "ADDX":
            r = sv + dv + x
            res = r & msk
            c = r >> bits
            v = ((~(sv ^ dv) & (sv ^ res)) >> (bits - 1)) & 1
        else:
            r = dv - sv - x
            res = r & msk
            c = 1 if sv + x > dv else 0
            v = (((dv ^ sv) & (dv ^ res)) >> (bits - 1)) & 1
        if kind == "NEGX":
            dv, sv = 0, m.read(dst, w)
        nz = (8 if res >> (bits - 1) else 0) | ((m.ccr & 4) if res == 0 else 0)
        m.write(dst, w, res)
        m.ccr = nz | v << 1 | c | (0x10 if c else 0)
        return m.d, m.a, m.ccr, bytes(m.mem)
    if name == "emit_TAS":
        e = m.ea(rl[0][1], op & 7, 1)
        v = m.read(e, 1)
        m.ccr = m.flags_nz(v, 1) | (m.ccr & 0x10)
        m.write(e, 1, v | 0x80)
        return m.d, m.a, m.ccr, bytes(m.mem)
    if name == "emit_PEA":
        e = m.ea(rl[1][1], op & 7, 4)
        m.a[7] = (m.a[7] - 4) & M32
        m.wr(m.a[7], 4, e[1])
        return m.d, m.a, m.ccr, bytes(m.mem)
    if name == "emit_LINK":
        n = op & 7
        d = sext(m.next_ext(), 16)
        sp = (m.a[7] - 4) & M32
        m.wr(sp, 4, sp if n == 7 else m.a[n])
        m.a[n] = sp
        m.a[7] = (sp + d) & M32
        return m.d, m.a, m.ccr, bytes(m.mem)
    if name == "emit_UNLK":
        n = op & 7
        sp = m.a[n]
        v = m.rd(sp, 4)
        m.a[7] = (sp + 4) & M32
        m.a[n] = v
        return m.d, m.a, m.ccr, bytes(m.mem)
    if name == "emit_MOVEP":
        n = op & 7
        dn = (op >> 9) & 7
        nb = 4 if op & 0x40 else 2
        ad = (m.a[n] + sext(m.next_ext(), 16)) & M32
        if op & 0x80:
            for k in range(nb):
                m.wr(ad + 2 * k, 1, m.d[dn] >> (8 * (nb - 1 - k)))
        else:
            v = 0
            for k in range(nb):
                v = v << 8 | m.rd(ad + 2 * k, 1)
            m.d[dn] = v if nb == 4 else (m.d[dn] & 0xFFFF0000) | v
        return m.d, m.a, m.ccr, bytes(m.mem)
    if name in ("emit_ANDI_TO_CCR", "emit_ORI_TO_CCR", "emit_EORI_TO_CCR"):
        i = m.next_ext() & 0x1F
        m.ccr = {"emit_ANDI_TO_CCR": m.ccr & i, "emit_ORI_TO_CCR": m.ccr | i, "emit_EORI_TO_CCR": m.ccr ^ i}[
            name
        ] & 0x1F
        return m.d, m.a, m.ccr, bytes(m.mem)
    if name == "emit_MOVE_TO_CCR":
        m.ccr = m.read(m.ea(rl[0][1], op & 7, 2), 2) & 0x1F
        return m.d, m.a, m.ccr, bytes(m.mem)
    if name == "emit_MOVEM":
        mask = m.next_ext()
        a = op & 7
        w = 4 if op & 0x40 else 2
        regs = lambda i: ("d", i) if i < 8 else ("a", i - 8)
        mode = (op >> 3) & 7
        if mode not in (3, 4):  # control modes: ascending addresses, An untouched
            ad = m.ea(rl[1][1], a, w)[1]
            for i in range(16):
                if mask >> i & 1:
                    k, n = regs(i)
                    if op & 0x400:
                        v = m.rd(ad, w)
                        v = sext(v, 16) & M32 if w == 2 else v
                        (m.d if k == "d" else m.a)[n] = v
                    else:
                        m.wr(ad, w, (m.d if k == "d" else m.a)[n])
                    ad = (ad + w) & M32
            return m.d, m.a, m.ccr, bytes(m.mem)
        if op & 0x400:  # (An)+ -> registers
            ad = m.a[a]
            for i in range(16):
                if mask >> i & 1:
                    v = m.rd(ad, w)
                    v = sext(v, 16) & M32 if w == 2 else v
                    ad = (ad + w) & M32
                    k, n = regs(i)
                    (m.d if k == "d" else m.a)[n] = v
            m.a[a] = ad  # PRM: the postincremented address, even if An is in the list
        else:  # registers -> -(An)
            orig = m.a[a]
            ad = orig
            for i in range(16):
                if mask >> i & 1:
                    k, n = regs(15 - i)
                    v = orig if (k == "a" and n == a) else (m.d if k == "d" else m.a)[n]
                    ad = (ad - w) & M32
                    m.wr(ad, w, v)
            m.a[a] = ad
        return m.d, m.a, m.ccr, bytes(m.mem)
    if name in ("emit_DIVU", "emit_DIVS"):
        dv = m.read(m.ea(rl[0][1], op & 7, 2), 2)
        n = (op >> 9) & 7
        dd = m.d[n]
        signed = op & 0x100 != 0
        if dv == 0:
            return "trap"
        if signed:
            dd, dv = sext(dd, 32), sext(dv, 16)
            q = abs(dd) // abs(dv)
            q = -q if (dd < 0) != (dv < 0) else q
            lo, hi = -32768, 32767
        else:
            q = dd // dv
            lo, hi = 0, 0xFFFF
        x = m.ccr & 0x10
        if not lo <= q <= hi:
            # overflow: V set, C clear, N and Z unchanged, the register unchanged
            m.ccr = (m.ccr & 0xC) | 2 | x
        else:
            r = dd - q * dv
            m.d[n] = (r & 0xFFFF) << 16 | (q & 0xFFFF)
            m.ccr = (8 if q & 0x8000 else 0) | (4 if q & 0xFFFF == 0 else 0) | x
        return m.d, m.a, m.ccr, bytes(m.mem)
    if name in ("emit_ABCD", "emit_SBCD", "emit_NBCD"):
        x = 1 if m.ccr & 0x10 else 0
        if name == "emit_NBCD":
            de = m.ea(rl[0][1], op & 7, 1)
            sv = m.read(de, 1)
            dv = 0
        else:
            if op & 8:
                se = m.ea("-(an)", op & 7, 1)
                de = m.ea("-(an)", (op >> 9) & 7, 1)
            else:
                se = ("dn", op & 7)
                de = ("dn", (op >> 9) & 7)
            sv = m.read(se, 1)
            dv = m.read(de, 1)
        if name == "emit_ABCD":
            lo = (sv & 15) + (dv & 15) + x
            if lo > 9:
                lo += 6
            r = (sv & 0xF0) + (dv & 0xF0) + lo
            c = 0
            if r > 0x99:
                r += 0x60
                c = 1
        else:  # SBCD and NBCD: dst - src - X
            lo = (dv & 15) - (sv & 15) - x
            r = (dv & 0xF0) - (sv & 0xF0) + lo
            c = 0
            if lo < 0:
                r -= 6
            if r < 0:
                r -= 0x60
                c = 1
        r &= 0xFF
        m.write(de, 1, r)
        # Z cleared if non-zero, else unchanged; N, V undefined (not compared)
        m.ccr = (m.ccr & 4 if r == 0 else 0) | c | (0x10 if c else 0)
        return m.d, m.a, m.ccr, bytes(m.mem)
    if name == "emit_CHK":
        bound = sext(m.read(m.ea(rl[0][1], op & 7, 2), 2), 16)
        dn = sext(m.d[(op >> 9) & 7], 16)
        if dn < 0 or dn > bound:
            return "trap"
        return m.d, m.a, m.ccr, bytes(m.mem)  # no trap: the CCR is undefined (not compared)
    if name == "emit_MOVE_SR_TO":
        m.write(m.ea(rl[0][1], op & 7, 2), 2, _sr_value(m))
        return m.d, m.a, m.ccr, bytes(m.mem)
    if name == "emit_MOVE_TO_SR":
        _sr_write(m, m.read(m.ea(rl[0][1], op & 7, 2), 2) & 0xA71F)
        return m.d, m.a, m.ccr, bytes(m.mem)
    if name in ("emit_ANDI_TO_SR", "emit_ORI_TO_SR", "emit_EORI_TO_SR"):
        i = m.next_ext()
        cur = _sr_value(m)
        _sr_write(
            m,
            {"emit_ANDI_TO_SR": cur & i, "emit_ORI_TO_SR": cur | i, "emit_EORI_TO_SR": cur ^ i}[name]
            & 0xA71F,
        )
        return m.d, m.a, m.ccr, bytes(m.mem)
    if name == "emit_MOVE_USP_TO":
        m.a[op & 7] = LAST.get("usp", 0)
        return m.d, m.a, m.ccr, bytes(m.mem)
    if name == "emit_MOVE_TO_USP":
        LAST["usp"] = m.a[op & 7]
        return m.d, m.a, m.ccr, bytes(m.mem)
    if name == "emit_MOVEQ":
        v = sext(op, 8) & M32
        m.d[(op >> 9) & 7] = v
        m.ccr = m.flags_nz(v, 4) | (m.ccr & 0x10)
        return m.d, m.a, m.ccr, bytes(m.mem)
    return None
