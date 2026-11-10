"""Load forwarding inside a block, at the ARM level. The D registers live in memory (r5 + CPU_D0 + 4n,
big-endian: the word at +0, the low halfword at +2, the low
byte at +3), so every 68K instruction loads its sources and stores its result. When the value a load wants is
still in a scratch register -- the instruction before
stored it from there, or an earlier load put it there -- the load becomes a register move (or an extend), or
disappears when it would be `mov r0,r0`.

A map says which of r0-r3 mirrors which D register and how:  'w' = the whole register, 'h' / 'sh' = its low
halfword zero- / sign-extended, 'b' / 'sb' likewise for
the low byte, 'l16' / 'l8' = a register whose low half / byte is the D register's (a halfword / byte store
stores that much). A write to a register drops its entries,
a store into a D register drops every other mirror of it, a branch inside a body, a call, a symbolic branch or
anything unknown clears the map. The caller clears it
at every instruction that is a branch target.

  forward(words, mirror, d0, ...) -> (new words, the map at the end)"""

import armdce

import os

# D0 lives in r4: its "slot accesses" are register moves / extends / bit-field inserts on r4
PIN_D0 = os.environ.get("PJIT_PIN_D0", "1") == "1"


def slot_pinned(w):
    """The pinned D0 as a slot: `mov rd, r4` / `uxt* sxt* rd, r4` are loads of D0, `mov r4, rs` / `bfi r4, rs,
    #0, #8|16` are stores. -> as slot(), or None.
    """
    cond = w >> 28 != 14
    if (w & 0x0FFF0FF0) == 0x01A00000:  # mov rd, rm
        rd, rm = w >> 12 & 15, w & 15
        if rm == 4 and rd != 4:
            return True, rd, 0, "w", cond
        if rd == 4 and rm != 4:
            return False, rm, 0, "w", cond
        return None
    for base, part in ((0x06AF0070, "sb"), (0x06EF0070, "b"), (0x06BF0070, "sh"), (0x06FF0070, "h")):
        if (w & 0x0FFF0FF0) == base and (w & 15) == 4 and (w >> 12 & 15) != 4:
            return True, w >> 12 & 15, 0, part, cond
    if (w & 0x0FFFFFF0) == 0x07C74010:
        return False, w & 15, 0, "l8", cond  # bfi r4, rs, #0, #8
    if (w & 0x0FFFFFF0) == 0x07CF4010:
        return False, w & 15, 0, "l16", cond  # bfi r4, rs, #0, #16
    return None


def slot(w, d0):
    """(is_load, rt, D register, part, conditional) for a load / store of a D register slot at r5 + imm, else
    None. part: 'w' 'h' 'sh' 'b' 'sb' (loads) / 'w' 'l16' 'l8' (stores).
    """
    if isinstance(w, tuple):
        return None
    w = int(w)
    if PIN_D0:
        r = slot_pinned(w)
        if r is not None:
            return r
    if (w >> 16 & 15) != 5:
        return None
    if w & 0x0E000000 == 0x04000000:  # ldr / str / ldrb / strb, immediate offset
        if not (w >> 24 & 1) or w >> 21 & 1:
            return None
        off = (w & 0xFFF) * (1 if w >> 23 & 1 else -1)
        byte = w >> 22 & 1
        load = w >> 20 & 1
        part = ("b" if load else "l8") if byte else "w"
    elif w & 0x0E400090 == 0x00400090 and w & 0x60:  # ldrh / ldrsb / ldrsh / strh, immediate offset
        if not (w >> 24 & 1) or w >> 21 & 1:
            return None
        off = ((w >> 4 & 0xF0) | (w & 0xF)) * (1 if w >> 23 & 1 else -1)
        load = w >> 20 & 1
        sh = w >> 5 & 3
        part = {1: "h" if load else "l16", 2: "sb", 3: "sh"}.get(sh)
        if part is None or (not load and sh != 1):
            return None
    else:
        return None
    rel = off - d0
    if not 0 <= rel < 32:
        return None
    n, byteoff = rel // 4, rel % 4
    if part in ("w",) and byteoff != 0:
        return None
    if part in ("h", "sh", "l16") and byteoff != 2:
        return None
    if part in ("b", "sb", "l8") and byteoff != 3:
        return None
    return bool(w >> 20 & 1), w >> 12 & 15, n, part, w >> 28 != 14


def mov_(rd, rm):
    return 0xE1A00000 | rd << 12 | rm


def ext_(kind, rd, rm):
    return (
        {"uxth": 0xE6FF0070, "sxth": 0xE6BF0070, "uxtb": 0xE6EF0070, "sxtb": 0xE6AF0070}[kind] | rd << 12 | rm
    )


# which mirror kinds can satisfy which load, and the instruction that does it
RULES = {
    "w": {"w": "mov"},
    "h": {"w": "uxth", "h": "mov", "sh": "uxth", "l16": "uxth"},
    "sh": {"w": "sxth", "h": "sxth", "sh": "mov", "l16": "sxth"},
    "b": {"w": "uxtb", "h": "uxtb", "sh": "uxtb", "l16": "uxtb", "b": "mov", "sb": "uxtb", "l8": "uxtb"},
    "sb": {"w": "sxtb", "h": "sxtb", "sh": "sxtb", "l16": "sxtb", "b": "sxtb", "sb": "mov", "l8": "sxtb"},
}
TRACKED = (0, 1, 2, 3)


def forward(words, mirror, d0, may_delete=True):
    mirror = dict(mirror)
    out = []
    stats = {"forwarded": 0, "deleted": 0}
    blocked = False
    for w in words:
        if blocked:
            out.append(w)
            # after a branch inside the body: what holds on the fall-through path need not hold at its target
            continue
        if isinstance(w, tuple):
            mirror.clear()
            blocked = True
            out.append(w)
            continue
        info = armdce.analyse(w)
        if info.branch is not None or info.barrier:
            mirror.clear()
            blocked = True
            out.append(w)
            continue
        s = slot(w, d0)
        if s is not None:
            load, rt, n, part, cond = s
            if cond:  # conditional: it may or may not have happened
                out.append(w)
                if load:
                    mirror = {r: e for r, e in mirror.items() if r != rt}
                else:
                    mirror = {r: e for r, e in mirror.items() if e[0] != n}
                continue
            if load:
                src = None
                for rx, (nn, kind) in mirror.items():
                    if nn == n and kind in RULES[part]:
                        src = rx
                        how = RULES[part][kind]
                        break
                if src is not None:
                    stats["forwarded"] += 1
                    if how == "mov":
                        if src == rt:
                            if may_delete:
                                stats["deleted"] += 1
                                continue  # the register already holds it
                            # (a nop keeps the word count: a branch inside the body has to land where it did)
                            out.append(0xE320F000)
                        else:
                            out.append(mov_(rt, src))
                    else:
                        out.append(ext_(how, rt, src))
                    # rt now mirrors the same register with the loaded width
                    mirror = {r: e for r, e in mirror.items() if r != rt}
                    if rt in TRACKED:
                        mirror[rt] = (n, part)
                    continue
                out.append(w)
                mirror = {r: e for r, e in mirror.items() if r != rt}
                if rt in TRACKED:
                    mirror[rt] = (n, part)
                continue
            # a store into the slot: every other mirror of that D register is stale
            out.append(w)
            mirror = {r: e for r, e in mirror.items() if e[0] != n}
            if rt in TRACKED:
                mirror[rt] = (n, part)
            continue
        # not a D slot access: a write to a scratch register drops what it mirrored; a store through r5 to
        # another slot touches nothing we track
        out.append(w)
        if info.writes & 0xF:
            mirror = {r: e for r, e in mirror.items() if not info.writes >> r & 1}
        if PIN_D0 and info.writes >> 4 & 1:
            # the pinned D0 was written by something that is not a store the map knows
            mirror = {r: e for r, e in mirror.items() if e[0] != 0}
        if (int(w) >> 28) != 14 and False:
            pass
    return out, ({} if blocked else mirror)
