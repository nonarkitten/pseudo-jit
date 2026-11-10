"""A small static evaluator over what a block's translator can know about the 68K machine at a point along a
path: address registers that hold a constant, data
registers that do, the stack pointer relative to where tracking began, and the constants sitting on the stack
(return addresses pushed by calls). It is what turns
`bsr TSTC ; dc.b 'x',off` into straight-line code: the pushed return address R is known, `movea.l (a7)+,a1`
makes a1 = R, `move.b (a1)+,d1` reads a byte of the code stream
(immutable: self-modifying code is not supported anywhere in the cache), `adda.l d1,a1` is arithmetic on
constants, and `jmp (a1)` has a static target.

Every claim is checked against the 68000 oracle by tools/gen/check/known_test.py: after each instruction, each
value this says it knows must equal the real register / stack slot.

  state.a[n]   = (value, item)   address register n holds value; item = the earliest block item its value depends on (for the back-edge check)
  state.d[n]   = (value, item)   the same for data registers (whole 32-bit value known)
  state.sp     = bytes the stack pointer has moved from its value when tracking began, or None (unknown: nothing on the stack is known either)
  state.stk[x] = (value, item)   the long at (original A7 + x)
Reads only come from memory that is code stream: [R, R + 16) for every return address R pushed by a call
tracked here (the bytes after a call site: inline data). That is a property of the program, not of the stack,
so the windows (shared by all copies) outlive the pop.
Anything not understood invalidates what it could change (a_writes / dest_regs, memory writes clear the stack,
an unknown stack pointer clears it).
"""

import re
import a7

M32 = 0xFFFFFFFF
SZ = a7.SIZES


def sx(v, bits):
    v &= (1 << bits) - 1
    return (v - (1 << bits)) & M32 if v >> (bits - 1) else v


def parse_ea(tok):
    """('imm', v) | ('d', n) | ('a', n) | ('ind', k, disp, mode) with mode '' / '+' / '-' | None."""
    m = re.fullmatch(r"#(-?)(\$?)([0-9a-f]+)", tok)
    if m:
        return (
            "imm",
            (
                -int(m.group(3), 16 if m.group(2) else 10)
                if m.group(1)
                else int(m.group(3), 16 if m.group(2) else 10)
            )
            & M32,
        )
    m = re.fullmatch(r"([ad])([0-7])", tok)
    if m:
        return (m.group(1), int(m.group(2)))
    m = re.fullmatch(r"\(a([0-7])\)(\+?)", tok)
    if m:
        return ("ind", int(m.group(1)), 0, m.group(2))
    m = re.fullmatch(r"-\(a([0-7])\)", tok)
    if m:
        return ("ind", int(m.group(1)), 0, "-")
    m = re.fullmatch(r"(-?)\$([0-9a-f]+)\(a([0-7])\)", tok)
    if m:
        return ("ind", int(m.group(3)), -int(m.group(2), 16) if m.group(1) else int(m.group(2), 16), "")
    return None


class Known:
    def __init__(self):
        self.a = {}
        self.d = {}
        self.sp = 0
        self.stk = {}
        self.windows = []

    def copy(self):
        k = Known()
        k.a = dict(self.a)
        k.d = dict(self.d)
        k.sp = self.sp
        k.stk = dict(self.stk)
        k.windows = self.windows
        return k

    @staticmethod
    def intersect(states):
        """What holds on every one of these states (a path that can be entered several ways)."""
        out = Known()
        first = states[0]
        out.windows = first.windows
        for r, (v, di) in first.a.items():
            if all(s.a.get(r, (None,))[0] == v for s in states):
                out.a[r] = (v, min(s.a[r][1] for s in states))
        for r, (v, di) in first.d.items():
            if all(s.d.get(r, (None,))[0] == v for s in states):
                out.d[r] = (v, min(s.d[r][1] for s in states))
        if all(s.sp == first.sp for s in states) and first.sp is not None:
            out.sp = first.sp
            for x, (v, di) in first.stk.items():
                if all(s.stk.get(x, (None,))[0] == v for s in states):
                    out.stk[x] = (v, min(s.stk[x][1] for s in states))
        else:
            out.sp = None
        return out

    # ---- the stack ----
    def push_ret(self, ret, idx):
        # the bytes after a call site are code stream (maybe inline data): they never change, whatever happens
        # to the stack
        self.windows.append((ret & M32, (ret & M32) + 16))
        if self.sp is None:
            return
        self.sp -= 4
        self._drop_below(self.sp, self.sp + 4)
        self.stk[self.sp] = (ret & M32, idx)

    def top(self):
        """(value, item) of the long on top of the stack if it is known."""
        return self.stk.get(self.sp) if self.sp is not None else None

    def _drop_below(self, lo, hi):
        for x in [x for x in self.stk if lo - 3 <= x < hi]:
            del self.stk[x]

    def _window(self, addr, size):
        return any(lo <= addr and addr + size <= hi for lo, hi in self.windows)

    # ---- evaluating one plain instruction ----
    def clear(self):
        self.a.clear()
        self.d.clear()
        self.stk.clear()
        self.sp = None

    def _read(self, mem, tok, sz, idx):
        """The value an operand reads, or None."""
        e = parse_ea(tok)
        if e is None:
            return None
        if e[0] == "imm":
            return e[1] & ((1 << 8 * sz) - 1)
        if e[0] == "d":
            return (self.d[e[1]][0] & ((1 << 8 * sz) - 1)) if e[1] in self.d else None
        if e[0] == "a":
            return (self.a[e[1]][0] & ((1 << 8 * sz) - 1)) if e[1] in self.a else None
        _, k, disp, mode = e
        if k == 7:
            if self.sp is None or mode in ("+", "-"):
                if mode == "+" and sz == 4 and self.sp is not None and self.sp in self.stk:
                    return self.stk[self.sp][0]
                return None
            x = self.sp + disp
            return self.stk[x][0] if sz == 4 and x in self.stk else None
        if k not in self.a:
            return None
        base = self.a[k][0]
        addr = (base - sz if mode == "-" else base + disp) & M32
        if not self._window(addr, sz):
            return None
        v = 0
        for j in range(sz):
            v = v << 8 | mem[addr + j]
        return v

    def step(self, mem, i, idx):
        """Update for the plain instruction i (decoded Insn) that is item idx."""
        t = a7.text(mem, i.pc)
        if t is None:
            self.clear()
            return
        m, sfx, ops, size = t
        sz = SZ.get(sfx, 4)
        # the source value (read before anything changes), for the forms that use it
        src = self._read(mem, ops[0], sz, idx) if len(ops) >= 2 else None
        sp0 = self.sp
        # ---- the stack pointer: every instruction goes through here ----
        if self.sp is not None:
            eff = a7.a7effect(m, sfx, ops, self.sp, {}, allow_positive=True)
            if isinstance(eff, str):
                newsp = None
            else:
                newsp = eff[0]
        else:
            newsp = None
        autoupd = a7.autos(ops, sfx)
        ar, dr = a7.dest_regs(m, ops)
        wrote_mem = a7.writes_memory(m, sfx, ops)
        # register updates computed from the old state, applied after the invalidations
        newa = {}
        newd = {}
        dst = ops[-1] if ops else ""
        de = parse_ea(dst) if ops else None
        if m in ("move", "movea") and len(ops) == 2 and de is not None:
            if de[0] == "d" and m == "move":
                n = de[1]
                if src is not None:
                    if sz == 4:
                        newd[n] = (src, idx)
                    elif n in self.d:
                        newd[n] = ((self.d[n][0] & ~((1 << 8 * sz) - 1) & M32) | src, min(idx, self.d[n][1]))
            elif de[0] == "a":
                n = de[1]
                if src is not None:
                    newa[n] = ((sx(src, 8 * sz) if sz == 2 else src) & M32, idx)
        elif m == "moveq" and de is not None and de[0] == "d" and src is not None:
            newd[de[1]] = (sx(src, 8), idx)
        elif m == "clr" and de is not None and de[0] == "d":
            n = de[1]
            if sz == 4:
                newd[n] = (0, idx)
            elif n in self.d:
                newd[n] = (self.d[n][0] & ~((1 << 8 * sz) - 1) & M32, self.d[n][1])
        elif (
            m in ("adda", "suba") and de is not None and de[0] == "a" and de[1] in self.a and src is not None
        ):
            n = de[1]
            v = sx(src, 16) if sz == 2 else src
            newa[n] = (
                ((self.a[n][0] + v) if m == "adda" else (self.a[n][0] - v)) & M32,
                min(idx, self.a[n][1]),
            )
        elif (
            m in ("addq", "subq") and de is not None and de[0] == "a" and de[1] in self.a and src is not None
        ):
            n = de[1]
            newa[n] = (
                ((self.a[n][0] + src) if m == "addq" else (self.a[n][0] - src)) & M32,
                min(idx, self.a[n][1]),
            )
        elif (
            m in ("addq", "subq")
            and de is not None
            and de[0] == "ind"
            and de[1] == 7
            and de[3] == ""
            and sz == 4
            and self.sp is not None
            and (self.sp + de[2]) in self.stk
            and src is not None
        ):
            x = self.sp + de[2]
            v, di = self.stk[x]
            self.stk[x] = (((v + src) if m == "addq" else (v - src)) & M32, min(di, idx))
            wrote_mem = False  # a stacked constant adjusted in place
        elif m == "lea" and len(ops) == 2 and de is not None and de[0] == "a":
            e = parse_ea(ops[0])
            if e is not None and e[0] == "ind" and e[3] == "" and e[1] != 7 and e[1] in self.a:
                newa[de[1]] = ((self.a[e[1]][0] + e[2]) & M32, min(idx, self.a[e[1]][1]))
        # ---- invalidate what the instruction may have changed ----
        if self.sp is not None and newsp is None:
            self.stk.clear()
        for k in ar:
            if k != 7:
                self.a.pop(k, None)
        for k in dr:
            self.d.pop(k, None)
        for k, delta in autoupd:  # (An)+ / -(An): a known register moves by a known amount
            if k == 7 or k in ar:
                continue
            if k in self.a:
                self.a[k] = ((self.a[k][0] + delta) & M32, self.a[k][1])
        if wrote_mem:
            self.stk.clear()
        self.sp = newsp
        if newsp is not None and sp0 is not None:
            if newsp < sp0:
                self._drop_below(newsp, sp0)  # something was written at the new stack top
            elif newsp > sp0:
                for x in [x for x in self.stk if x < newsp]:
                    del self.stk[x]
        for n, v in newa.items():
            self.a[n] = v
        for n, v in newd.items():
            self.d[n] = v
