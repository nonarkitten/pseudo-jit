"""The A7 effect of a 68K instruction, for deciding whether a routine can be inlined into a block: how far does
it move the stack pointer, and does it
write the slot where the return address sits? Parses capstone's text of the instruction (prototype; the
runtime will want an opcode table).

  a7effect(m, sfx, ops, delta, frames) -> (new delta, new frames) or a string naming why the routine is
  refused
delta is the stack pointer relative to its value on entry to the routine (so the return address is at 0..3);
frames maps a LINK register to the delta it set.
"""

import re
import capstone

md = capstone.Cs(capstone.CS_ARCH_M68K, capstone.CS_MODE_M68K_000)
md.detail = True


def text(mem, pc):
    i = next(md.disasm(bytes(mem[pc : pc + 12]), pc), None)
    if i is None:
        return None
    mn = i.mnemonic.split(".")
    return mn[0], (mn[1] if len(mn) > 1 else "l"), [o.lower() for o in split_ops(i.op_str)], i.size


def split_ops(s):
    out = []
    d = 0
    cur = ""
    for ch in s:
        if ch == "(":
            d += 1
        elif ch == ")":
            d -= 1
        if ch == "," and d == 0:
            out.append(cur.strip())
            cur = ""
        else:
            cur += ch
    if cur.strip():
        out.append(cur.strip())
    return out


def imm(tok):
    m = re.fullmatch(r"#(-?)(\$?)([0-9a-f]+)", tok)
    if not m:
        return None
    v = int(m.group(3), 16 if m.group(2) else 10)
    return -v if m.group(1) else v


def reglist(tok):
    n = 0
    for part in tok.split("/"):
        if "-" in part:
            a, b = part.split("-")
            n += int(b[1:]) - int(a[1:]) + 1
        else:
            n += 1
    return n


def sext_for(v, sfx, default):
    """An immediate as a signed number of the operand's size (the text gives it as an unsigned hex)."""
    bits = {"b": 8, "w": 16, "l": 32}.get(sfx or default, 16)
    v &= (1 << bits) - 1
    return v - (1 << bits) if v >> (bits - 1) else v


NOWRITE_M = {"cmp", "cmpi", "cmpa", "cmpm", "tst", "btst", "pea", "jmp", "jsr", "lea"}


def a7effect(m, sfx, ops, d, frames, allow_positive=False):
    """(new delta, new frames) or a string naming why the callee is refused."""
    sz = 2 if sfx in "bw" else 4
    d0 = d
    frames = dict(frames)
    if m == "movem":
        if ops[-1] == "-(a7)":
            d -= reglist(ops[0]) * sz
        elif ops[0] == "(a7)+":
            d += reglist(ops[-1]) * sz
    elif m == "pea":
        d -= 4
    elif m == "link":
        v = imm(ops[1])
        if v is None:
            return "link"
        # LINK.W's displacement is a signed word (capstone prints it unsigned: #$ffe8 is -24)
        v = sext_for(v, sfx, "w")
        d -= 4
        frames[ops[0]] = d
        d += v
    elif m == "unlk":
        if ops[0] not in frames:
            return "unlk without link"
        d = frames.pop(ops[0]) + 4
    else:
        for t in ops:
            if t == "-(a7)":
                d -= sz
            elif t == "(a7)+":
                d += sz
        if ops and ops[-1] == "a7":
            v = imm(ops[0]) if len(ops) > 1 else None
            if v is not None and m in ("adda", "suba"):
                v = sext_for(v, sfx, "w")  # ADDA.W / SUBA.W sign-extend their word operand
            if m in ("addq", "addi", "add", "adda") and v is not None:
                d += v
            elif m in ("subq", "subi", "sub", "suba") and v is not None:
                d -= v
            elif m == "lea" and re.fullmatch(r"-?\$[0-9a-f]+\(a7\)", ops[0]):
                d += int(ops[0].split("(")[0].replace("$", ""), 16)
            else:
                return "A7 written"
        if ops and m not in NOWRITE_M:
            mm = re.fullmatch(r"(-?)(\$[0-9a-f]+)?\(a7\)", ops[-1])
            if mm:
                off = int(mm.group(2)[1:], 16) * (-1 if mm.group(1) else 1) if mm.group(2) else 0
                if d + off < 4 and d + off + {"b": 1, "w": 2}.get(sfx, 4) > 0:
                    return "writes the return address"
    if d > 0 and not allow_positive:
        return "pops past the return address"
    return d, frames


def a_writes(mem, pc):
    """The address registers an instruction may change (a set of 0-7), or None if that cannot be said.
    Conservative: a register named as a destination, used with
    (An)+ / -(An), or implied (LINK, UNLK, the stack pointer for calls, pushes and pops)."""
    t = text(mem, pc)
    if t is None:
        return None
    m, sfx, ops, size = t
    w = set()
    for o in ops:
        for k in range(8):
            if o in ("-(a%d)" % k, "(a%d)+" % k):
                w.add(k)
    if m in ("jsr", "bsr", "rts", "rtr", "rte", "pea", "link", "unlk"):
        w.add(7)
    if m in ("link", "unlk") and ops:
        w.add(int(ops[0][1])) if ops[0][:1] == "a" and ops[0][1:].isdigit() else None
    if m == "movem" and ops:
        pass
    if (
        m in ("cmp", "cmpa", "cmpi", "cmpm", "tst", "btst", "jmp", "jsr", "pea", "chk", "bra", "bcc", "bsr")
        or m.startswith("b")
        and m not in ("bchg", "bclr", "bset", "bfchg", "bfclr", "bfset")
    ):
        return w  # (compares, tests and branches write no register other than the implied ones)
    if m == "exg":
        for o in ops:
            if o[:1] == "a" and o[1:].isdigit():
                w.add(int(o[1:]))
        return w
    dst = ops[-1] if ops else ""
    if dst[:1] == "a" and dst[1:].isdigit() and len(dst) == 2:
        w.add(int(dst[1]))
    if m == "movem":  # MOVEM <ea>,regs loads registers: any of them might be an A register
        return set(range(8))
    return w


SIZES = {"b": 1, "w": 2, "l": 4}


def autos(ops, sfx):
    """[(register, +size / -size)] for the (An)+ / -(An) operands of an instruction (A7 steps a byte by 2)."""
    out = []
    sz = SIZES.get(sfx, 4)
    for o in ops:
        for k in range(8):
            if o == "(a%d)+" % k:
                out.append((k, 2 if (k == 7 and sz == 1) else sz))
            elif o == "-(a%d)" % k:
                out.append((k, -(2 if (k == 7 and sz == 1) else sz)))
    return out


def dest_regs(m, ops):
    """(explicit A registers, explicit D registers) written as a destination (not through (An)+ / -(An)), or
    None where it cannot be said.
    """
    if m in (
        "cmp",
        "cmpa",
        "cmpi",
        "cmpm",
        "tst",
        "btst",
        "jmp",
        "jsr",
        "pea",
        "chk",
        "rts",
        "rtr",
        "rte",
        "nop",
        "bra",
        "bsr",
    ) or (m.startswith("b") and m not in ("bchg", "bclr", "bset")):
        return set(), set()
    if (
        m in ("movem", "exg", "dbra", "dbf")
        or m.startswith("db")
        or m in ("divu", "divs", "mulu", "muls", "swap")
    ):
        pass
    ar, dr = set(), set()
    if m == "exg":
        for o in ops:
            if o[:1] == "a" and o[1:].isdigit():
                ar.add(int(o[1:]))
            if o[:1] == "d" and o[1:].isdigit():
                dr.add(int(o[1:]))
        return ar, dr
    if m == "movem":
        return set(range(8)), set(range(8))
    if m in ("link", "unlk"):
        if ops and ops[0][:1] == "a" and ops[0][1:].isdigit():
            ar.add(int(ops[0][1:]))
        ar.add(7)
        return ar, dr
    if m.startswith("db") and ops and ops[0][:1] == "d" and ops[0][1:].isdigit():
        dr.add(int(ops[0][1:]))
        return ar, dr
    dst = ops[-1] if ops else ""
    if len(dst) == 2 and dst[1].isdigit():
        if dst[0] == "a":
            ar.add(int(dst[1]))
        elif dst[0] == "d":
            dr.add(int(dst[1]))
    if m in ("divu", "divs", "mulu", "muls", "swap", "ext", "extb") and len(dst) == 2 and dst[0] == "d":
        dr.add(int(dst[1]))
    return ar, dr


def writes_memory(m, sfx, ops):
    """Does the instruction store to memory other than by pushing below the stack pointer? (it could then
    change a stacked value we have tracked)
    """
    if m in (
        "cmp",
        "cmpa",
        "cmpi",
        "cmpm",
        "tst",
        "btst",
        "jmp",
        "jsr",
        "bsr",
        "pea",
        "lea",
        "chk",
        "rts",
        "nop",
        "bra",
        "unlk",
        "link",
        "exg",
        "swap",
        "ext",
        "movea",
        "adda",
        "suba",
        "moveq",
        "divu",
        "divs",
        "mulu",
        "muls",
    ):
        return False
    if m.startswith("b") and m not in ("bchg", "bclr", "bset"):
        return False
    if m.startswith("db"):
        return False
    if m == "movem":
        return True if not (ops and ops[-1] == "-(a7)") else False
    if not ops:
        return False
    dst = ops[-1]
    if dst == "-(a7)":
        return False
    if (
        (len(dst) == 2 and dst[0] in "ad" and dst[1].isdigit())
        or dst[:1] == "#"
        or dst in ("sr", "ccr", "usp")
    ):
        return False
    if (
        m
        in (
            "addq",
            "subq",
            "add",
            "sub",
            "addi",
            "subi",
            "and",
            "or",
            "eor",
            "andi",
            "ori",
            "eori",
            "neg",
            "negx",
            "not",
            "clr",
            "move",
            "tas",
            "bchg",
            "bclr",
            "bset",
            "scc",
        )
        or m.startswith("s")
        or m.startswith(("as", "ls", "ro"))
    ):
        return True
    return True
