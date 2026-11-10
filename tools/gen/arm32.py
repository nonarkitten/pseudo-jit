"""A small ARM32 (ARMv7-A, ARM state) encoder: just the instructions the generator emits. Every function returns
an `Insn`: the 32-bit
instruction word (it is an int, so it compares, hashes and shifts like one) that also carries `.text`, the
assembly the *call* meant
(`ldrsb r0, [r5, #-61]`). The static tables are written with that text, and `make check-asm` has GNU as
assemble it and compares
with the words: an independent check of this encoder. Conditions are given as names ('al', 'cs', 'cc', ...).
"""

COND = {
    "eq": 0,
    "ne": 1,
    "cs": 2,
    "hs": 2,
    "cc": 3,
    "lo": 3,
    "mi": 4,
    "pl": 5,
    "vs": 6,
    "vc": 7,
    "hi": 8,
    "ls": 9,
    "ge": 10,
    "lt": 11,
    "gt": 12,
    "le": 13,
    "al": 14,
    "nv": 15,
}
R = {f"r{i}": i for i in range(16)}
R.update({"sp": 13, "lr": 14, "pc": 15})


def reg(r):
    return R[r] if isinstance(r, str) else r


def rn_(r):
    n = reg(r)
    return {13: "sp", 14: "lr", 15: "pc"}.get(n, f"r{n}")


def cc_(cond):
    return "" if cond == "al" else cond


class Insn(int):
    """An instruction word with the assembly text that produced it."""

    text = None


def _i(word, text):
    i = Insn(word)
    i.text = text
    return i


def enc_imm(v):
    """The ARM modified immediate (rotate, imm8) for v, or None."""
    v &= 0xFFFFFFFF
    for rot in range(0, 32, 2):
        r = ((v << rot) | (v >> (32 - rot))) & 0xFFFFFFFF if rot else v
        if r <= 0xFF:
            return (rot // 2, r)
    return None


_DP = dict(
    and_=0,
    eor=1,
    sub=2,
    rsb=3,
    add=4,
    adc=5,
    sbc=6,
    rsc=7,
    tst=8,
    teq=9,
    cmp=10,
    cmn=11,
    orr=12,
    mov=13,
    bic=14,
    mvn=15,
)
SHIFT = {"lsl": 0, "lsr": 1, "asr": 2, "ror": 3}


def dp(op, rd, rn, op2, s=False, cond="al"):
    """Data processing. op2 is ('imm', value) or ('reg', rm) or ('reg', rm, shift_name, amount) or ('rsh', rm,
    shift_name, rs).
    """
    o = _DP[op]
    test = op in ("tst", "teq", "cmp", "cmn")
    w = COND[cond] << 28 | o << 21 | (1 << 20 if s or test else 0) | reg(rn) << 16 | reg(rd) << 12
    if op2[0] == "imm":
        e = enc_imm(op2[1])
        if e is None:
            raise ValueError(f"not an ARM immediate: {op2[1]:#x}")
        w |= 1 << 25 | e[0] << 8 | e[1]
        o2 = "#0x%x" % (op2[1] & 0xFFFFFFFF)
    elif op2[0] == "rsh":  # ('rsh', rm, shift_name, rs): shifted by a register
        w |= reg(op2[1]) | SHIFT[op2[2]] << 5 | 1 << 4 | reg(op2[3]) << 8
        o2 = f"{rn_(op2[1])}, {op2[2]} {rn_(op2[3])}"
    else:
        w |= reg(op2[1])
        o2 = rn_(op2[1])
        if len(op2) > 2:
            w |= op2[3] << 7 | SHIFT[op2[2]] << 5
            if op2[3] or op2[2] != "lsl":
                o2 += f", {op2[2]} #{op2[3]}"
    name = op.rstrip("_") + ("s" if s and not test else "") + cc_(cond)
    if op in ("mov", "mvn"):
        text = f"{name}\t{rn_(rd)}, {o2}"
    elif test:
        text = f"{name}\t{rn_(rn)}, {o2}"
    else:
        text = f"{name}\t{rn_(rd)}, {rn_(rn)}, {o2}"
    return _i(w, text)


def mov(rd, op2, **k):
    return dp("mov", rd, 0, op2, **k)


def mvn(rd, op2, **k):
    return dp("mvn", rd, 0, op2, **k)


def add(rd, rn, op2, **k):
    return dp("add", rd, rn, op2, **k)


def sub(rd, rn, op2, **k):
    return dp("sub", rd, rn, op2, **k)


def and_(rd, rn, op2, **k):
    return dp("and_", rd, rn, op2, **k)


def orr(rd, rn, op2, **k):
    return dp("orr", rd, rn, op2, **k)


def eor(rd, rn, op2, **k):
    return dp("eor", rd, rn, op2, **k)


def cmp_(rn, op2, **k):
    return dp("cmp", 0, rn, op2, **k)


def lsl(rd, rm, n, **k):
    return mov(rd, ("reg", rm, "lsl", n), **k)


def lsr(rd, rm, n, **k):
    return mov(rd, ("reg", rm, "lsr", n), **k)


def _addr(rn, off, kind):
    if kind == "post":
        return f"[{rn_(rn)}], #{off}"
    base = f"[{rn_(rn)}, #{off}]" if off else f"[{rn_(rn)}]"
    return base + ("!" if kind == "pre" else "")


def _ls(load, rt, rn, off, byte=False, kind="off", cond="al"):
    """Word/byte load or store, immediate offset. kind: 'off' = [rn,#off], 'pre' = [rn,#off]!, 'post' =
    [rn],#off.
    """
    u = 1 if off >= 0 else 0
    p = 0 if kind == "post" else 1
    wb = 1 if kind == "pre" else 0
    w = (
        COND[cond] << 28
        | 0x04000000
        | p << 24
        | u << 23
        | (1 << 22 if byte else 0)
        | wb << 21
        | (1 << 20 if load else 0)
        | reg(rn) << 16
        | reg(rt) << 12
        | abs(off)
    )
    return _i(
        w, f"{'ldr' if load else 'str'}{'b' if byte else ''}{cc_(cond)}\t{rn_(rt)}, {_addr(rn, off, kind)}"
    )


def ldr(rt, rn, off, **k):
    return _ls(True, rt, rn, off, **k)


def str_(rt, rn, off, **k):
    return _ls(False, rt, rn, off, **k)


def ldrb(rt, rn, off, **k):
    return _ls(True, rt, rn, off, byte=True, **k)


def strb(rt, rn, off, **k):
    return _ls(False, rt, rn, off, byte=True, **k)


def _ls_reg(load, rt, rn, rm, byte=False, cond="al"):
    w = (
        COND[cond] << 28
        | 0x06000000
        | 1 << 24
        | 1 << 23
        | (1 << 22 if byte else 0)
        | (1 << 20 if load else 0)
        | reg(rn) << 16
        | reg(rt) << 12
        | reg(rm)
    )
    return _i(
        w, f"{'ldr' if load else 'str'}{'b' if byte else ''}{cc_(cond)}\t{rn_(rt)}, [{rn_(rn)}, {rn_(rm)}]"
    )


def ldr_r(rt, rn, rm, **k):
    return _ls_reg(True, rt, rn, rm, **k)


def str_r(rt, rn, rm, **k):
    return _ls_reg(False, rt, rn, rm, **k)


def strb_r(rt, rn, rm, **k):
    return _ls_reg(False, rt, rn, rm, byte=True, **k)


_SH = {1: "h", 2: "sb", 3: "sh"}


def _lsx(load, sh, rt, rn, off, kind="off", cond="al"):
    """Halfword / signed byte / signed halfword: sh bits = 0b01 H, 0b10 SB, 0b11 SH."""
    u = 1 if off >= 0 else 0
    a = abs(off)
    p = 0 if kind == "post" else 1
    wb = 1 if kind == "pre" else 0
    w = (
        COND[cond] << 28
        | p << 24
        | u << 23
        | 1 << 22
        | wb << 21
        | (1 << 20 if load else 0)
        | reg(rn) << 16
        | reg(rt) << 12
        | (a >> 4) << 8
        | 0x90
        | sh << 5
        | (a & 15)
    )
    return _i(w, f"{'ldr' if load else 'str'}{_SH[sh]}{cc_(cond)}\t{rn_(rt)}, {_addr(rn, off, kind)}")


def _lsx_reg(load, sh, rt, rn, rm, cond="al"):
    w = (
        COND[cond] << 28
        | 1 << 24
        | 1 << 23
        | (1 << 20 if load else 0)
        | reg(rn) << 16
        | reg(rt) << 12
        | 0x90
        | sh << 5
        | reg(rm)
    )
    return _i(w, f"{'ldr' if load else 'str'}{_SH[sh]}{cc_(cond)}\t{rn_(rt)}, [{rn_(rn)}, {rn_(rm)}]")


def ldrh(rt, rn, off, **k):
    return _lsx(True, 1, rt, rn, off, **k)


def strh(rt, rn, off, **k):
    return _lsx(False, 1, rt, rn, off, **k)


def ldrsb(rt, rn, off, **k):
    return _lsx(True, 2, rt, rn, off, **k)


def ldrsh(rt, rn, off, **k):
    return _lsx(True, 3, rt, rn, off, **k)


def ldrsb_r(rt, rn, rm, **k):
    return _lsx_reg(True, 2, rt, rn, rm, **k)


def ldrsh_r(rt, rn, rm, **k):
    return _lsx_reg(True, 3, rt, rn, rm, **k)


def strh_r(rt, rn, rm, **k):
    return _lsx_reg(False, 1, rt, rn, rm, **k)


def _ext(base, name, rd, rm):
    return _i(base | reg(rd) << 12 | reg(rm), f"{name}\t{rn_(rd)}, {rn_(rm)}")


def sxtb(rd, rm):
    return _ext(0xE6AF0070, "sxtb", rd, rm)


def sxth(rd, rm):
    return _ext(0xE6BF0070, "sxth", rd, rm)


def uxtb(rd, rm):
    return _ext(0xE6EF0070, "uxtb", rd, rm)


def uxth(rd, rm):
    return _ext(0xE6FF0070, "uxth", rd, rm)


def mrs(rd, cond="al"):
    return _i(COND[cond] << 28 | 0x010F0000 | reg(rd) << 12, f"mrs{cc_(cond)}\t{rn_(rd)}, apsr")


def msr_f(rm, cond="al"):
    return _i(COND[cond] << 28 | 0x0128F000 | reg(rm), f"msr{cc_(cond)}\tcpsr_f, {rn_(rm)}")


def msr_g(rm, cond="al"):
    # GE[3:0] only (bits 19:16)
    return _i(COND[cond] << 28 | 0x0124F000 | reg(rm), f"msr{cc_(cond)}\tapsr_g, {rn_(rm)}")


def uadd8(rd, rn, rm, cond="al"):
    return _i(
        COND[cond] << 28 | 0x06500F90 | reg(rn) << 16 | reg(rd) << 12 | reg(rm),
        f"uadd8{cc_(cond)}\t{rn_(rd)}, {rn_(rn)}, {rn_(rm)}",
    )


def sel(rd, rn, rm, cond="al"):
    return _i(
        COND[cond] << 28 | 0x06800FB0 | reg(rn) << 16 | reg(rd) << 12 | reg(rm),
        f"sel{cc_(cond)}\t{rn_(rd)}, {rn_(rn)}, {rn_(rm)}",
    )


def ubfx(rd, rn, lsb, width, cond="al"):
    return _i(
        COND[cond] << 28 | 0x07E00050 | (width - 1) << 16 | reg(rd) << 12 | lsb << 7 | reg(rn),
        f"ubfx{cc_(cond)}\t{rn_(rd)}, {rn_(rn)}, #{lsb}, #{width}",
    )


def bx(rm, cond="al"):
    return _i(COND[cond] << 28 | 0x012FFF10 | reg(rm), f"bx{cc_(cond)}\t{rn_(rm)}")


def nop():
    return _i(0xE320F000, "nop")


def udf0():
    return _i(0xE7F000F0, "udf\t#0")


def bfi(rd, rn, lsb, width, cond="al"):
    return _i(
        COND[cond] << 28 | 0x07C00010 | reg(rd) << 12 | lsb << 7 | (lsb + width - 1) << 16 | reg(rn),
        f"bfi{cc_(cond)}\t{rn_(rd)}, {rn_(rn)}, #{lsb}, #{width}",
    )


def clz(rd, rm, cond="al"):
    return _i(
        COND[cond] << 28 | 0x016F0F10 | reg(rd) << 12 | reg(rm), f"clz{cc_(cond)}\t{rn_(rd)}, {rn_(rm)}"
    )


def b(delta, cond="al"):
    """Branch to the instruction `delta` words after this one (the ARM field is relative to pc = here + 8)."""
    return _i(
        COND[cond] << 28 | 0x0A000000 | ((delta - 2) & 0xFFFFFF),
        f"b{cc_(cond)}\t. {'+' if delta >= 0 else '-'} {4 * abs(delta)}",
    )


def mul(rd, rm, rs, cond="al"):
    return _i(
        COND[cond] << 28 | 0x00000090 | reg(rd) << 16 | reg(rs) << 8 | reg(rm),
        f"mul{cc_(cond)}\t{rn_(rd)}, {rn_(rm)}, {rn_(rs)}",
    )


def movw(rd, imm16, cond="al"):
    return _i(
        COND[cond] << 28 | 0x03000000 | (imm16 >> 12 & 0xF) << 16 | reg(rd) << 12 | imm16 & 0xFFF,
        f"movw{cc_(cond)}\t{rn_(rd)}, #0x{imm16:x}",
    )


def movt(rd, imm16, cond="al"):
    return _i(
        COND[cond] << 28 | 0x03400000 | (imm16 >> 12 & 0xF) << 16 | reg(rd) << 12 | imm16 & 0xFFF,
        f"movt{cc_(cond)}\t{rn_(rd)}, #0x{imm16:x}",
    )


def dsb():
    return _i(0xF57FF04F, "dsb\tsy")


def svc(n, cond="al"):
    return _i(COND[cond] << 28 | 0x0F000000 | n, f"svc{cc_(cond)}\t#{n}")


def block(load, rn, regs, mode="ia", wb=False, cond="al"):
    """LDM / STM. mode 'ia' (increment after) or 'db' (decrement before); regs a list of register numbers."""
    p, u = {"ia": (0, 1), "db": (1, 0)}[mode]
    mask = sum(1 << reg(r) for r in regs)
    rs = sorted(reg(r) for r in regs)
    parts, i = [], 0
    while i < len(rs):  # r0-r2 style ranges
        j = i
        while j + 1 < len(rs) and rs[j + 1] == rs[j] + 1:
            j += 1
        parts.append(rn_(rs[i]) if j == i else f"{rn_(rs[i])}-{rn_(rs[j])}")
        i = j + 1
    w = (
        COND[cond] << 28
        | 0x08000000
        | p << 24
        | u << 23
        | (1 << 21 if wb else 0)
        | (1 << 20 if load else 0)
        | reg(rn) << 16
        | mask
    )
    return _i(
        w,
        f"{'ldm' if load else 'stm'}{mode}{cc_(cond)}\t{rn_(rn)}{'!' if wb else ''}, {{{', '.join(parts)}}}",
    )


def ldm(rn, regs, **k):
    return block(True, rn, regs, **k)


def stm(rn, regs, **k):
    return block(False, rn, regs, **k)


def sym(name, cond="al", link=False):
    """A branch to a runtime symbol, left symbolic (the static tables write it as `b name`)."""
    return ("bl" if link else "b", cond, name)
