"""Constant folding at the ARM level, for the words of one 68K instruction (the prefix that leaves its extension
values in r1 / r2, then its sequence).

The prefix puts constants in r1 / r2, and the sequence then sign-extends them, shifts them, adds them to a
base, uses them as an index. Known constants are
tracked in r0-r3 and used to
  * evaluate an instruction whose inputs are all known (`mov r1,#$20 ; sxtb r1,r1` -> `mov r1,#$20`; the value
  stays known),
  * turn a known register operand into an immediate where the encoding allows it (`adds r3,r3,r1,lsl #24` ->
  `adds r3,r3,#$20000000`; a commutative operation with
    the constant in the first operand is swapped, `sub` becomes `rsb`),
  * turn a known register offset into an immediate offset (`ldrsb r0,[r6,r2]` with r2 = $2e -> `ldrsb
  r0,[r6,#$2e]`).
The constant loads that nothing reads any more are left for armdce.dce to remove. A flag-setting logical
operation (its carry comes from the shifter, which an
immediate does not reproduce) only takes an immediate that needs no rotation and had no shift; arithmetic ones
take any. Tracking stops after a branch inside the
words (what holds on the fall-through path need not hold at its target).

  fold(words) -> new words"""

import armdce

M32 = 0xFFFFFFFF


def rot_imm(v):
    """(the 12-bit immediate field, the rotation) for v, or None."""
    v &= M32
    for r in range(0, 32, 2):
        x = ((v << r) | (v >> (32 - r))) & M32 if r else v
        if x < 256:
            return (r // 2) << 8 | x, r
    return None


def sx(v, bits):
    v &= (1 << bits) - 1
    return (v - (1 << bits)) & M32 if v >> (bits - 1) else v


def shift(v, kind, amt):
    v &= M32
    if kind == 0:
        return (v << amt) & M32
    if kind == 1:
        return 0 if amt >= 32 else v >> amt
    if kind == 2:
        s = v - (1 << 32) if v >> 31 else v
        return (s >> min(amt, 31)) & M32
    return ((v >> amt) | (v << (32 - amt))) & M32  # ror (amt 1..31)


OPS = {
    0: lambda a, b: a & b,
    1: lambda a, b: a ^ b,
    2: lambda a, b: (a - b) & M32,
    3: lambda a, b: (b - a) & M32,
    4: lambda a, b: (a + b) & M32,
    12: lambda a, b: a | b,
    13: lambda a, b: b,
    14: lambda a, b: a & ~b & M32,
    15: lambda a, b: ~b & M32,
}
LOGICAL = {0, 1, 8, 9, 12, 13, 14, 15}
COMMUTE = {0: 0, 1: 1, 4: 4, 12: 12, 2: 3, 3: 2}  # and eor add orr: themselves; sub <-> rsb
EXT = {
    0x06AF0070: lambda v: sx(v, 8),
    0x06BF0070: lambda v: sx(v, 16),
    0x06EF0070: lambda v: v & 0xFF,
    0x06FF0070: lambda v: v & 0xFFFF,
}


def mov_imm(rd, v):
    a = rot_imm(v)
    if a is not None:
        return 0xE3A00000 | rd << 12 | a[0]
    b = rot_imm(~v & M32)
    if b is not None:
        return 0xE3E00000 | rd << 12 | b[0]
    return None


def decode_op2(w):
    """Operand 2 of a data-processing word: ('imm', value), ('reg', rm, shift kind, amount), or None (shifted
    by a register, or rrx).
    """
    if w >> 25 & 1:
        rot = 2 * (w >> 8 & 15)
        b = w & 0xFF
        return ("imm", ((b >> rot) | (b << (32 - rot))) & M32 if rot else b)
    if w & 0x10:
        return None
    kind = w >> 5 & 3
    amt = w >> 7 & 31
    if kind == 0 and amt == 0:
        return ("reg", w & 15, 0, 0)
    if kind == 3 and amt == 0:
        return None
    if kind in (1, 2) and amt == 0:
        amt = 32
    return ("reg", w & 15, kind, amt)


FEATURES = {"eval": True, "imm": True, "swap": True, "off": True, "ext": True}


def fold(words):
    consts = {}
    out = []
    blocked = False

    def kill(mask):
        for r in list(consts):
            if mask >> r & 1:
                del consts[r]

    for w in words:
        if blocked or isinstance(w, tuple):
            out.append(w)
            if isinstance(w, tuple):
                blocked = True
            continue
        w = int(w)
        info = armdce.analyse(w)
        if info.branch is not None or info.barrier:
            consts.clear()
            blocked = True
            out.append(w)
            continue
        al = w >> 28 == 14
        k = w >> 25 & 7
        # ---- movw / movt ----
        if w & 0x0FF00000 == 0x03000000:
            rd = w >> 12 & 15
            if al:
                consts[rd] = (w >> 4 & 0xF000) | (w & 0xFFF)
            else:
                kill(1 << rd)
            out.append(w)
            continue
        if w & 0x0FF00000 == 0x03400000:
            rd = w >> 12 & 15
            if al and rd in consts:
                consts[rd] = (consts[rd] & 0xFFFF) | (((w >> 4 & 0xF000) | (w & 0xFFF)) << 16)
            else:
                kill(1 << rd)
            out.append(w)
            continue
        # ---- sign / zero extends of a known value ----
        if (w & 0x0FFF0FF0) in EXT:
            rd, rm = w >> 12 & 15, w & 15
            if FEATURES["ext"] and al and rm in consts:
                v = EXT[w & 0x0FFF0FF0](consts[rm])
                n = mov_imm(rd, v)
                out.append(n if n is not None else w)
                consts[rd] = v
                continue
            kill(1 << rd)
            out.append(w)
            continue
        # ---- data processing (not the multiply / extra load-store / misc / movw-movt encodings) ----
        dp = (
            k in (0, 1)
            and not (k == 0 and (w & 0x90) == 0x90)
            and not (k == 0 and (w & 0x0F900000) == 0x01000000)
            and not (k == 1 and (w & 0x0F900000) == 0x03000000 and not w >> 20 & 1)
        )
        if dp:
            opc = w >> 21 & 15
            s = w >> 20 & 1
            rn = w >> 16 & 15
            rd = w >> 12 & 15
            uses_rn = opc not in (13, 15)
            op2 = decode_op2(w)
            new = w
            val2 = None
            if op2 is not None:
                if op2[0] == "imm":
                    val2 = op2[1]
                elif op2[1] in consts:
                    val2 = shift(consts[op2[1]], op2[2], op2[3]) if op2[3] else consts[op2[1]]
            rn_known = (not uses_rn) or rn in consts
            evaluable = FEATURES["eval"] and al and not s and val2 is not None and rn_known and opc in OPS
            if evaluable:
                v = OPS[opc](consts.get(rn, 0), val2)
                n = mov_imm(rd, v)
                out.append(n if n is not None else w)
                consts[rd] = v
                continue
            if (
                FEATURES["imm"] and op2 is not None and op2[0] == "reg" and val2 is not None
            ):  # a known operand 2 register becomes an immediate
                a = rot_imm(val2)
                if a is not None and (
                    not (s and opc in LOGICAL) or (a[1] == 0 and op2[2] == 0 and op2[3] == 0)
                ):
                    new = (w & 0xFFFFF000) | 1 << 25 | a[0]
            elif (
                FEATURES["swap"]
                and uses_rn
                and rn in consts
                and op2 is not None
                and op2[0] == "reg"
                and op2[1] not in consts
                and op2[2] == 0
                and op2[3] == 0
                and opc in COMMUTE
                and opc not in (5, 6, 7)
            ):  # a known first operand: swap a commutative operation
                a = rot_imm(consts[rn])
                if a is not None and (not (s and opc in LOGICAL) or a[1] == 0):
                    new = (
                        (w & 0xF0000000)
                        | 1 << 25
                        | COMMUTE[opc] << 21
                        | s << 20
                        | op2[1] << 16
                        | rd << 12
                        | a[0]
                    )
            out.append(new)
            if opc in (8, 9, 10, 11):
                pass  # compares and tests have no destination
            elif al and val2 is not None and rn_known and opc in OPS:
                consts[rd] = OPS[opc](consts.get(rn, 0), val2)
            else:
                kill(1 << rd)
            continue
        # ---- a load or store with a known register offset -> an immediate offset ----
        if (
            FEATURES["off"]
            and w & 0x0E000010 == 0x06000000
            and w >> 24 & 1
            and not w >> 21 & 1
            and not w & 0xFE0
            and (w & 15) in consts
        ):  # ldr / str / ldrb / strb [rn, +/-rm]
            v = consts[w & 15]
            sv = v - (1 << 32) if v >> 31 else v
            if not w >> 23 & 1:
                sv = -sv
            if abs(sv) < 4096:
                new = (
                    (w & 0xF0000000)
                    | 0x05000000
                    | (1 << 23 if sv >= 0 else 0)
                    | (w & 0x00500000)
                    | (w & 0x000FF000)
                    | abs(sv)
                )
                out.append(new)
                kill(info.writes)
                continue
        if (
            FEATURES["off"]
            and w & 0x0E400F90 == 0x00000090
            and w & 0x60
            and w >> 24 & 1
            and not w >> 21 & 1
            and (w & 15) in consts
        ):  # ldrh / ldrsb / ldrsh / strh [rn, +/-rm]
            v = consts[w & 15]
            sv = v - (1 << 32) if v >> 31 else v
            if not w >> 23 & 1:
                sv = -sv
            if abs(sv) < 256:
                new = (
                    (w & 0xF0000000)
                    | 0x01400000
                    | (1 << 23 if sv >= 0 else 0)
                    | (w & 0x00100000)
                    | (w & 0x000FF000)
                    | (w & 0xF0)
                    | (abs(sv) >> 4) << 8
                    | (abs(sv) & 15)
                )
                out.append(new)
                kill(info.writes)
                continue
        out.append(w)
        kill(info.writes)
    return out
