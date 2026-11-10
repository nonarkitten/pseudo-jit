"""The block assembler (host-side prototype of the runtime translator): 68K code at a guest address -> one
straight ARM block.

   form        a list of items in layout order from the guest address: plain instructions, Bcc / BRA / DBcc, and calls. A BSR / JSR with a static target whose routine
               qualifies (a contiguous region, every instruction one this translates, branches only inside it,
               the stack pointer back where it was at every RTS, the
               return address never written, at most K bytes) is *inlined*: a push of the static return
               address, the routine's instructions as their own address scope,
               its RTS an `add sp,sp,#4`. If it does not qualify, or the block would grow past the cap, the
               items are cut back to where they were (the bump pointer
               reset) and the call is the end of the block: the push, then the router. JMP, dynamic JSR and an
               RTS that is not an inlined one end the block the same way.
   liveness    backward over the items, two flag groups (N Z V C, and X in the GE flags), a real control-flow graph for the forward branches; a back-edge, an exit and
               every other end of the block want everything
   emit        per instruction: the prefix that leaves the source / destination extension value in r1 / r2 (as pjit_fill does), then the generator's sequence cut down
               by tools/gen/armdce.py for what is wanted after it, its `bx lr` dropped and its conditional
               `bx<cc> lr` turned into branches to the end;
               exits are stubs `movw/movt r0,#target ; b pjit_goto` after the code. DBcc tests its counter
               without touching the flags (`clz`) and picks its way with
               `add pc,pc,r3,lsl #2`.

Not here yet (the block ends in front of the instruction and the router hands it to the interpreter): RTE /
TRAP / TRAPV / STOP, MOVEM, instructions whose sequences
call into the runtime other than DIV / BCD helpers, and exceptions inside a block (they would need the guest
PC of the trapping instruction).
"""

import os, sys, struct

sys.path.insert(0, os.path.dirname(__file__))
from arm32 import *
import armdce, a7, fwd, constfold, regalloc
import known as known_mod
import maketables as mt
import gen as _gen

# blocks use the flat forms their forwarder / scheduler / folder were built for (D-slot accesses by address in
# the word)
_gen.Gen.legacy = True

SABOTAGE = False  # the test's sensitivity check: pretend no flag is wanted where the block ends
GOTO = "pjit_goto"
BXLR = 0xE12FFF1E
CONDS = ["eq", "ne", "cs", "cc", "mi", "pl", "vs", "vc", "hi", "ls", "ge", "lt", "gt", "le", "al"]
# 68K condition field -> name
CC68 = [
    "t",
    "f",
    "hi",
    "ls",
    "cc",
    "cs",
    "ne",
    "eq",
    "vc",
    "vs",
    "pl",
    "mi",
    "ge",
    "lt",
    "gt",
    "le",
]
DIV_TAILS = ("handle_DIVU_tail", "handle_DIVS_tail")
HELPERS = {
    "handle_DIVU_tail",
    "handle_DIVS_tail",
    "abcd",
    "sbcd",
    "nbcd",
    "movem_sp_save",
    "movem_sp_restore",
    "m2r_word",
    "m2r_long",
    "r2m_word",
    "r2m_long",
}


def sext(v, bits):
    v &= (1 << bits) - 1
    return v - (1 << bits) if v >> (bits - 1) else v


# ---- constants -----------------------------------------------------------------------------------------------------------------------------
def movt(rd, imm16):
    return int(0xE3400000 | (imm16 >> 12 & 0xF) << 16 | rd << 12 | imm16 & 0xFFF)


def rot_imm(v):
    """The 12-bit ARM immediate field for v, or None."""
    v &= 0xFFFFFFFF
    for r in range(0, 32, 2):
        x = ((v << r) | (v >> (32 - r))) & 0xFFFFFFFF if r else v
        if x < 256:
            return (r // 2) << 8 | x
    return None


def load_const(rd, v):
    v &= 0xFFFFFFFF
    if rot_imm(v) is not None:
        return [mov(rd, ("imm", v))]
    if rot_imm(~v) is not None:
        return [mvn(rd, ("imm", ~v & 0xFFFFFFFF))]
    if v <= 0xFFFF:
        return [movw(rd, v)]
    return [movw(rd, v & 0xFFFF), movt(rd, v >> 16)]


def add_const(rd, rn, v):
    """rd = rn + v (v a signed constant), flags untouched; uses r3 as a scratch if v does not fit an
    immediate.
    """
    if v == 0:
        return [] if rd == rn else [mov(rd, ("reg", rn))]
    if rot_imm(v & 0xFFFFFFFF) is not None and v > 0:
        return [add(rd, rn, ("imm", v))]
    if rot_imm(-v & 0xFFFFFFFF) is not None and v < 0:
        return [sub(rd, rn, ("imm", -v))]
    return load_const(3, v) + [add(rd, rn, ("reg", 3))]


class ImageMem:
    """A program image served at its own addresses (zero elsewhere): mem[a] and mem[a:b] for the decoder."""

    def __init__(self, base, image):
        self.base = base
        self.image = image

    def __getitem__(self, a):
        if isinstance(a, slice):
            return bytes(self[x] for x in range(a.start, a.stop))
        o = a - self.base
        return self.image[o] if 0 <= o < len(self.image) else 0

    def __len__(self):
        return 1 << 32


# ---- decode --------------------------------------------------------------------------------------------------------------------------------
class Insn:
    pass


def read_w(mem, a):
    return mem[a] << 8 | mem[a + 1]


def decode(mem, pc):
    """kinds: plain, bcc (Bcc and BRA), dbcc, bsr, jsr, jmp, rts, stop."""
    i = Insn()
    i.pc = pc
    i.op = read_w(mem, pc)
    n = mt.ext_word_count(i.op)
    i.words = [read_w(mem, pc + 2 * k) for k in range(n)]
    i.size = 2 * n
    e = mt.ext[i.op]
    i.src, i.dst = e >> 4 & 15, e & 15
    i.kind = "plain"
    hi = i.op >> 12
    op = i.op
    if hi == 6:  # Bcc / BRA / BSR
        c = op >> 8 & 15
        d = op & 0xFF
        if d == 0xFF:
            i.kind = "stop"  # the 68020 long form is out of scope
        else:
            i.cond = c
            if d == 0:
                i.disp = sext(i.words[1], 16)
                i.size = 4
            else:
                i.disp = sext(d, 8)
                i.size = 2
            i.target = pc + 2 + i.disp
            i.kind = "bsr" if c == 1 else "bcc"
    elif op & 0xF0F8 == 0x50C8:  # DBcc
        i.kind = "dbcc"
        i.cond = op >> 8 & 15
        i.reg = op & 7
        i.disp = sext(i.words[1], 16)
        i.size = 4
        i.target = pc + 2 + i.disp
    elif op & 0xFFC0 in (0x4E80, 0x4EC0):  # JSR / JMP <ea>
        mode, reg = op >> 3 & 7, op & 7
        i.kind = "jsr" if op & 0xFFC0 == 0x4E80 else "jmp"
        i.static = None
        if mode == 7 and reg == 0:
            i.static = sext(i.words[1], 16) & 0xFFFFFFFF
        elif mode == 7 and reg == 1:
            i.static = i.words[1] << 16 | i.words[2]
        elif mode == 7 and reg == 2:
            i.static = pc + 2 + sext(i.words[1], 16)
        elif mode not in (2, 5, 6) and not (mode == 7 and reg == 3):
            i.kind = "stop"
    elif op == 0x4E75:
        i.kind = "rts"
    elif op in (0x4E73, 0x4E77, 0x4E76, 0x4E72, 0x4E70) or op & 0xFFF0 == 0x4E40 or hi in (0xA, 0xF):
        i.kind = "stop"  # RTE RTR TRAPV STOP RESET TRAP, line A / F
    return i


def copy_insn(i):
    j = Insn()
    j.__dict__.update(i.__dict__)
    return j


# ---- the extension-value prefix ----------------------------------------------------------------------------------------------------------------
def prefix(i, which, k, d0):
    """ARM words that leave the extension value of the source (r1) or destination (r2) in its register; k =
    index of its first extension word in i.words.
    """
    cls = i.src if which == "src" else i.dst
    rd = 1 if which == "src" else 2
    EXT_NONE, EXT_I16, EXT_I32, EXT_D8_XAREG, EXT_D16_PC, EXT_D8_XPC, EXT_BDISP = range(7)
    if cls == EXT_NONE:
        return [], 0
    if cls == EXT_I16:
        return load_const(rd, sext(i.words[k], 16)), 1
    if cls == EXT_I32:
        return load_const(rd, i.words[k] << 16 | i.words[k + 1]), 2
    addr = i.pc + 2 * k
    if cls == EXT_D16_PC:
        return load_const(rd, addr + sext(i.words[k], 16)), 1
    if cls in (EXT_D8_XAREG, EXT_D8_XPC):
        w = i.words[k]
        xr = w >> 12 & 7
        long_ = w >> 11 & 1
        d8 = sext(w, 8)
        isA = w >> 15 & 1
        out = []
        if isA:
            out.append(mov(rd, ("reg", 6 + xr)) if long_ else sxth(rd, 6 + xr))
        else:
            out.append(ldr(rd, 5, d0 + xr * 4) if long_ else ldrsh(rd, 5, d0 + xr * 4 + 2))
        out += add_const(rd, rd, d8 + (addr if cls == EXT_D8_XPC else 0))
        return out, 1
    raise ValueError("extension class %d" % cls)


def prefix_all(i, d0):
    kk = 1
    pre = []
    if i.src:
        p, used = prefix(i, "src", kk, d0)
        pre += p
        kk += used
    if i.dst:
        p, used = prefix(i, "dst", kk, d0)
        pre += p
        kk += used
    return pre


# ---- the generator's sequences as inline code -----------------------------------------------------------------------------------------------------
def inline_body(seq, tail_ok=False, div_ok=False):
    """A sequence with its final `bx lr` dropped and every conditional `bx<cc> lr` turned into a branch to the
    end; None if it is not fit for a block
    (a branch to a runtime routine that does not return here). Symbolic tail calls to helpers that return
    through lr become `bl`. With tail_ok a final
    `b pjit_goto` is kept (the sequence ends the block)."""
    words = list(seq)
    if words[-1] == BXLR:
        words.pop()
    elif isinstance(words[-1], tuple) and words[-1][1] == "al" and words[-1][2] in HELPERS:
        words[-1] = ("bl", "al", words[-1][2])
    elif tail_ok and isinstance(words[-1], tuple) and words[-1][1] == "al" and words[-1][2] == GOTO:
        return words
    else:
        return None
    n = len(words)
    for k, w in enumerate(words):
        if isinstance(w, tuple):
            if w[0] == "bl" and w[2] in HELPERS:
                continue
            if div_ok and w[0] == "b" and w[1] == "al" and w[2] in DIV_TAILS:
                continue  # DIV's helper returns through memory to the words after the `b`
            return None
        w = int(w)
        if w & 0x0FFFFFFF == 0x012FFF1E:  # bx<cc> lr
            words[k] = (w & 0xF0000000) | 0x0A000000 | ((n - k - 2) & 0xFFFFFF)
        elif w >> 25 & 7 == 5 and not w >> 24 & 1:
            off = ((w & 0xFFFFFF) ^ 0x800000) - 0x800000
            if k + 2 + off < 0 or k + 2 + off > n:
                return None  # (a branch inside the body is position independent, backward ones included)
    return words


# ---- which routines can be inlined ----------------------------------------------------------------------------------------------------------------
WHY = [None]


def no(reason):
    WHY[0] = reason
    return None


def qualify(gen, mem, entry, k_max, depth=3, stack=()):
    """The routine's instructions in address order if it can be inlined, else None. See the module doc."""
    if depth < 0 or entry in stack:
        return no("recursion or nesting too deep")
    reach = {}
    work = [(entry, 0, ())]
    total = 0
    while work:
        pc, d, fr = work.pop()
        if pc in reach:
            if reach[pc][1] != d:
                return None  # two paths with different stack depths
            continue
        i = decode(mem, pc)
        reach[pc] = (i, d)
        total += i.size
        if total > k_max:
            return no("larger than K bytes")
        if i.kind == "plain":
            seq = gen.generate(i.op)
            if seq is None or inline_body(seq) is None:
                return no("an instruction this assembler does not translate")
            t = a7.text(mem, pc)
            if t is None:
                return no("an instruction this assembler does not translate")
            eff = a7.a7effect(t[0], t[1], t[2], d, dict(fr))
            if isinstance(eff, str):
                return no("the stack pointer: " + eff)
            work.append((pc + i.size, eff[0], tuple(sorted(eff[1].items()))))
        elif i.kind == "bcc":
            work.append((i.target, d, fr))
            if CC68[i.cond] != "t":
                work.append((pc + i.size, d, fr))
        elif i.kind == "dbcc":
            work += [(i.target, d, fr), (pc + i.size, d, fr)]
        elif i.kind == "rts":
            if d != 0:
                return no("RTS with the stack pointer not back at the return address")
        elif i.kind in ("bsr", "jsr"):
            tgt = i.target if i.kind == "bsr" else i.static
            if tgt is None or qualify(gen, mem, tgt, k_max, depth - 1, stack + (entry,)) is None:
                return no("calls a routine that does not qualify, or through a register")
            work.append((pc + i.size, d, fr))
        else:
            return no("an instruction that ends a routine in a way it cannot inline (JMP, TRAP, RTE, ...)")
    addrs = sorted(reach)
    if addrs[0] != entry:
        return no("a branch below the entry")
    for a, b in zip(addrs, addrs[1:]):
        if b != a + reach[a][0].size:
            return None  # a gap: code that is not part of the routine, or data
    return [reach[a][0] for a in addrs]


# ---- the block -----------------------------------------------------------------------------------------------------------------------------
class Item:
    def __init__(self, kind, ins=None, **kw):
        self.kind = kind
        self.ins = ins
        self.tgt_addr = None
        self.scope = None
        self.tgt = None
        self.end_of = None
        self.__dict__.update(kw)


class Fail(Exception):
    pass


class Retry(Exception):
    pass


def form(gen, mem, start, cap, inline, k_max, nest, follow=False, tree=False, track=False, callthru=False):
    """See _form. With track, a block whose resolutions through known registers / stacked constants turn out
    unsound (a loop changes what a call or return inside it was
    resolved through) is formed again without."""
    if track:
        try:
            return _form(gen, mem, start, cap, inline, k_max, nest, follow, tree, True, callthru)
        except Retry:
            pass
    return _form(gen, mem, start, cap, inline, k_max, nest, follow, tree, False, callthru)


def pops_return(mem, entry, limit=10):
    """Does the routine at entry, within its first few instructions (skipping calls), take its return address
    off the stack as data (MOVEA.L (A7)+,An and the like)?
    """
    pc = entry
    for _ in range(limit):
        i = decode(mem, pc)
        if i.kind in ("bsr", "jsr"):
            pc += i.size
            continue
        t = a7.text(mem, pc)
        if i.kind != "plain" or t is None:
            return False
        m, sfx, ops, size = t
        if m in ("move", "movea") and sfx == "l" and ops and ops[0] in ("(a7)+", "(a7)"):
            return True
        pc += i.size
    return False


_eff_cache = {}


def effects(gen, mem, r, k_max, depth=3):
    """(address registers, data registers, writes memory) a routine (the instruction list qualify() returned)
    may change, or None if that cannot be said.
    """
    A, D, W = set(), set(), False
    for i in r:
        if i.kind == "plain":
            t = a7.text(mem, i.pc)
            if t is None:
                return None
            m, sfx, ops, size = t
            aw = a7.a_writes(mem, i.pc)
            if aw is None:
                return None
            ar, dr = a7.dest_regs(m, ops)
            A |= aw | ar
            D |= dr
            W = W or a7.writes_memory(m, sfx, ops)
        elif i.kind == "dbcc":
            D.add(i.reg)
        elif i.kind in ("bsr", "jsr"):
            tgt = i.target if i.kind == "bsr" else i.static
            if tgt is None or depth <= 0:
                return None
            rr = qualify(gen, mem, tgt, k_max, depth)
            if rr is None:
                return None
            sub = effects(gen, mem, rr, k_max, depth - 1)
            if sub is None:
                return None
            A |= sub[0]
            D |= sub[1]
            W = W or sub[2]
    return A, D, W


def _form(gen, mem, start, cap, inline, k_max, nest, follow=False, tree=False, track=False, callthru=False):
    items = []
    spent = [0]
    K = known_mod.Known()  # what is known at the point decoding has reached, on the current path
    join_states = {}  # (scope id, forward branch target address) -> the Known state at each branch to it
    scopes = []  # every address scope of the block: the top one, and one for each routine followed into
    thru = [0]  # how many followed-into routines deep the current path is
    # (item index, kind 'a' / 'stk', register, the item its value came from): a call / jump / return made
    # static
    resolved = []
    awr = {}
    dwr = {}
    # per item: the address / data registers it may change, and whether it may change the stack (memory)
    swr = {}

    def place(scope, i, item):
        idx = len(items)
        scope[i.pc] = idx
        items.append(item)
        awr[idx] = set()
        dwr[idx] = set()
        swr[idx] = item.kind in ("push", "retpop", "retfollow", "call")
        if swr[idx]:
            awr[idx] = {7}
        return item

    def writes_of(i):
        w = a7.a_writes(mem, i.pc)
        return set(range(8)) if w is None else w

    def track_plain(i, idx):
        """Update what is known for the plain instruction just placed at item idx."""
        t = a7.text(mem, i.pc)
        op = i.op
        awr[idx] = writes_of(i)
        if t is None:
            dwr[idx] = set(range(8))
            swr[idx] = True
        else:
            m, sfx, ops, size = t
            dwr[idx] = a7.dest_regs(m, ops)[1]
            eff = a7.a7effect(m, sfx, ops, 0, {}, allow_positive=True)
            swr[idx] = a7.writes_memory(m, sfx, ops) or isinstance(eff, str) or eff[0] != 0
        K.step(mem, i, idx)
        if op & 0xF1FF == 0x41F9 and len(i.words) == 3:
            K.a[op >> 9 & 7] = (i.words[1] << 16 | i.words[2], idx)  # LEA abs.l,An
        elif op & 0xF1FF == 0x41F8:
            K.a[op >> 9 & 7] = (sext(i.words[1], 16) & 0xFFFFFFFF, idx)  # LEA abs.w,An
        elif op & 0xF1FF == 0x41FA:
            K.a[op >> 9 & 7] = (i.pc + 2 + sext(i.words[1], 16), idx)  # LEA d16(PC),An

    def note_branch(i, idx, scope):
        if i.target > i.pc:
            # a forward branch: its state is merged at the target
            join_states.setdefault((id(scope), i.target), []).append(K.copy())
        elif i.target in scope:  # a back-edge: what a resolution used must not change in the loop
            t = scope[i.target]
            if idx < t:
                return
            chg_a = set().union(*[awr.get(k, set()) for k in range(t, idx + 1)])
            chg_s = any(swr.get(k, False) for k in range(t, idx + 1))
            for ri, kind, key, di in resolved:
                if ri >= t and di < t and ((kind == "a" and key in chg_a) or (kind == "stk" and chg_s)):
                    raise Retry()

    def resolve(i):
        """A JSR / JMP through (An) or d16(An) with An known: a static target."""
        mode, reg = i.op >> 3 & 7, i.op & 7
        if i.static is None and mode in (2, 5) and reg in K.a:
            v, di = K.a[reg]
            i.static = (v + (sext(i.words[1], 16) if mode == 5 else 0)) & 0xFFFFFFFF
            resolved.append((len(items), "a", reg, di))
            i.resolved = True

    def region(pc, scope, ins_list, depth):
        """Appends the items for a routine's decoded instructions (ins_list) or, at the top level (ins_list
        None), for the guest code from pc.
        """
        nonlocal K
        k = 0
        while True:
            if ins_list is not None:
                if k >= len(ins_list):
                    return None
                i = ins_list[k]
                k += 1
            else:
                if (
                    tree and items and pc in scope
                ):  # a path that runs into code the block already has: jump there, the path ends
                    j = Insn()
                    j.pc = pc
                    j.size = 0
                    j.cond = 0
                    j.kind = "bcc"
                    j.target = pc
                    j.op = 0x6000
                    items.append(Item("bcc", j, tgt_addr=pc, scope=scope))
                    return None
                i = decode(mem, pc)
                if track and (id(scope), pc) in join_states:
                    # a join: only what holds on every way in
                    K = known_mod.Known.intersect([K] + join_states.pop((id(scope), pc)))
            if ins_list is None and (i.kind == "stop" or (items and spent[0] + i.size > cap)):
                items.append(Item("goto_pc", i, tgt_addr=i.pc))
                return None
            if i.kind == "plain":
                seq = gen.generate(i.op)
                body = inline_body(seq) if seq is not None else None
                if body is None:
                    if ins_list is None:
                        items.append(Item("goto_pc", i, tgt_addr=i.pc))
                        return None
                    raise Fail
                i.seq = seq
                place(scope, i, Item("plain", i))
                spent[0] += i.size
                pc = i.pc + i.size
                if ins_list is None:
                    if track:
                        track_plain(i, len(items) - 1)
                else:
                    awr[len(items) - 1] = set(range(8))
                    dwr[len(items) - 1] = set(range(8))
                    swr[len(items) - 1] = True
            elif i.kind == "bcc":
                if (
                    follow
                    and CC68[i.cond] == "t"
                    and ins_list is None
                    and i.target not in scope
                    and spent[0] + i.size <= cap
                ):
                    place(scope, i, Item("follow", i))
                    spent[0] += i.size
                    pc = i.target  # BRA to code not yet in the block: carry on there, no branch at all
                    continue
                it = place(scope, i, Item("bcc", i, tgt_addr=i.target, scope=scope))
                spent[0] += i.size
                pc = i.pc + i.size
                if track and ins_list is None:
                    note_branch(i, len(items) - 1, scope)
                if CC68[i.cond] == "t" and ins_list is None:
                    return None  # BRA ends a top-level block
            elif i.kind == "dbcc":
                place(scope, i, Item("dbcc", i, tgt_addr=i.target, scope=scope))
                spent[0] += i.size
                pc = i.pc + i.size
                dwr[len(items) - 1] = {i.reg}
                if track and ins_list is None:
                    K.d.pop(i.reg, None)
                    note_branch(i, len(items) - 1, scope)
            elif i.kind in ("bsr", "jsr"):
                if track and ins_list is None and i.kind == "jsr":
                    resolve(i)
                tgt = i.target if i.kind == "bsr" else i.static
                ret = i.pc + i.size
                done = False
                why = (
                    "inlining off"
                    if not inline
                    else (
                        "the target is computed at run time (JSR through a register or a displacement)"
                        if tgt is None
                        else "nesting depth" if depth >= nest else None
                    )
                )
                r = None
                if inline and tgt is not None and depth < nest:
                    r = qualify(gen, mem, tgt, k_max, nest - depth)
                    if r is None:
                        why = WHY[0]
                    elif spent[0] + i.size + sum(x.size for x in r) > cap:
                        why = "the block would pass its cap"
                    if r is not None and spent[0] + i.size + sum(x.size for x in r) <= cap:
                        mark = (len(items), spent[0])
                        spent[0] += i.size
                        place(scope, i, Item("push", i, ret=ret))
                        try:
                            region(tgt, {}, [copy_insn(x) for x in r], depth + 1)
                            end = len(items)
                            for it in items[mark[0] :]:
                                if it.kind == "retpop" and it.end_of is None:
                                    it.end_of = end
                            done = True
                        except Fail:
                            del items[mark[0] :]
                            spent[0] = mark[1]  # rollback: the bump pointer goes back to the mark
                            scope.pop(i.pc, None)
                            why = "a call inside it could not be inlined"
                if (
                    not done
                    and ins_list is None
                    and callthru
                    and tgt is not None
                    and thru[0] < 6
                    and spent[0] + i.size <= cap
                    and (callthru == "all" or pops_return(mem, tgt))
                ):
                    # follow into the routine as straight-line code, in a scope of its own (the push is real;
                    # what the routine does with the address is then known)
                    place(scope, i, Item("push", i, ret=ret))
                    spent[0] += i.size
                    if track:
                        K.push_ret(ret, len(items) - 1)
                    inner = {}
                    scopes.append(inner)
                    thru[0] += 1
                    try:
                        return region(tgt, inner, None, depth)
                    finally:
                        thru[0] -= 1
                if not done:
                    if ins_list is not None:
                        raise Fail
                    place(scope, i, Item("call", i, ret=ret, why=why))
                    spent[0] += i.size
                    return None
                pc = ret
                if track:  # the callee changed what it changed; the stack is as it was
                    eff = effects(gen, mem, r, k_max)
                    if eff is None:
                        K.a.clear()
                        K.d.clear()
                        K.stk.clear()
                    else:
                        for q in eff[0]:
                            K.a.pop(q, None)
                        for q in eff[1]:
                            K.d.pop(q, None)
                        if eff[2]:
                            K.stk.clear()
            elif i.kind == "rts":
                if ins_list is None:
                    tp = K.top() if track else None
                    if tp is not None and follow and tp[0] not in scope and spent[0] + i.size <= cap:
                        # a return to an address we know (the call that pushed it is in this block): no
                        # lookup, carry on there
                        resolved.append((len(items), "stk", 0, tp[1]))
                        place(scope, i, Item("retfollow", i))
                        spent[0] += i.size
                        old = K.sp
                        K.sp += 4
                        K.stk.pop(old, None)
                        pc = tp[0]
                        continue
                    place(scope, i, Item("rts", i))
                    return None
                place(scope, i, Item("retpop", i))
                spent[0] += i.size
            elif i.kind == "jmp":
                if ins_list is not None:
                    raise Fail
                if track:
                    resolve(i)
                if follow and i.static is not None and i.static not in scope and spent[0] + i.size <= cap:
                    place(scope, i, Item("follow", i))
                    spent[0] += i.size
                    pc = i.static
                    continue
                place(scope, i, Item("jmp", i))
                return None
            else:
                raise Fail

    top = {}
    scopes.append(top)
    try:
        region(start, top, None, 0)
        tried = set()
        # more paths: the targets of forward branches the block has not reached, in whichever scope the branch
        # was
        while tree:
            pend = [
                (id(it.scope), it.tgt_addr, it.scope)
                for it in items
                if it.kind in ("bcc", "dbcc")
                and any(it.scope is sc for sc in scopes)
                and it.tgt_addr not in it.scope
                and it.tgt_addr > it.ins.pc
                and (id(it.scope), it.tgt_addr) not in tried
            ]
            pend = [q for q in pend if spent[0] < cap]
            if not pend:
                break
            sid, t, sc = min(pend, key=lambda q: (q[1], q[0]))
            tried.add((sid, t))
            st = join_states.pop((sid, t), [])
            K = known_mod.Known.intersect(st) if (track and st) else known_mod.Known()
            if not (track and st):
                K.sp = None
            thru[0] = 0
            region(t, sc, None, 0)
    except Fail:
        raise AssertionError("a failure escaped to the top level")
    return items, top


def build(
    gen,
    mem,
    start,
    cap=120,
    optimise=True,
    inline=True,
    k_max=120,
    nest=2,
    forward=False,
    cfold=False,
    follow=False,
    tree=False,
    alloc=False,
    track=False,
    callthru=False,
    sched=False,
):
    """-> (words, info): ARM words (symbolic branches as tuples) and a dict (instructions, guest bytes, words,
    stubs, the addresses translated, the terminating
    instructions, the items, the number of inlined calls)."""
    d0 = gen.D0
    items, top = form(gen, mem, start, cap, inline, k_max, nest, follow, tree, track, callthru)
    n = len(items)
    ALL = 0 if SABOTAGE else 3
    # ---- resolve branch targets: an item index inside the scope, or None (an exit) ----
    for it in items:
        if it.kind in ("bcc", "dbcc"):
            it.tgt = it.scope.get(it.tgt_addr)
    # ---- liveness (backward over the items) ----
    live_in = [ALL] * (n + 1)
    for k in range(n - 1, -1, -1):
        it = items[k]
        i = it.ins
        nxt = live_in[k + 1]
        if not optimise:
            live_in[k] = 3 if it.kind != "plain" else 3
        if it.kind == "plain":
            L = nxt if optimise else 3
            try:
                v, li = armdce.dce(i.seq, L)
            except ValueError:
                v, li = list(i.seq), 3
            it.live_out = L
            it.body = inline_body(v)
            if optimise:
                live_in[k] = li
        elif it.kind in ("bcc", "dbcc"):
            c = CC68[i.cond]
            taken = ALL if it.tgt is None or it.tgt <= k else live_in[it.tgt]
            out = taken if (it.kind == "bcc" and c == "t") else taken | nxt
            reads = 0 if c in ("t", "f") or (it.kind == "dbcc" and c == "f") else 1
            if optimise:
                live_in[k] = out | reads
        elif it.kind == "retpop":
            if optimise:
                live_in[k] = live_in[it.end_of] if it.end_of is not None and it.end_of > k + 1 else nxt
        elif it.kind in ("push", "follow", "retfollow"):
            if optimise:
                live_in[k] = nxt
        else:
            live_in[k] = ALL  # a terminator: the router wants everything
    # ---- items that something other than the previous item can branch to: forwarding does not cross into them ----
    targeted = set()
    for k, it in enumerate(items):
        if it.kind in ("bcc", "dbcc") and it.tgt is not None:
            targeted.add(it.tgt)
        if it.kind == "dbcc":
            targeted.add(k + 1)
        if it.kind == "retpop" and it.end_of is not None and it.end_of > k + 1:
            targeted.add(it.end_of)
    # ---- emit ----
    out = []
    stubs = []
    mirror = {}
    fstats = {"forwarded": 0, "deleted": 0}
    regions = []
    cur = []

    def stub(t):
        stubs.append(t)
        return len(stubs) - 1

    def cond_branch(c, kind, tgt):
        here = len(out)
        if c == "t":
            return [(kind, "al", tgt)]
        if c == "hi":
            return [("rel", "cs", here + 2), (kind, "ne", tgt)]
        if c == "ls":
            return [(kind, "cs", tgt), (kind, "eq", tgt)]
        return [(kind, c, tgt)]

    pos = [0] * (n + 1)
    terminators = set()
    addrs = set()
    peaks = []

    def flush():
        nonlocal cur
        if not cur:
            return
        regions.append(list(cur))
        words_ = cur
        if alloc:
            res, peak = regalloc.allocate(cur)
            if res is not None:
                words_ = res
                peaks.append(peak)
            else:
                peaks.append(None)
        if sched:  # the asm scheduler (pjit/blksched.s) under Unicorn
            import sched_asm

            res = sched_asm.get(d0)([w for w in cur])
            if res is not None:
                words_ = res
        out.extend(words_)
        cur = []

    for k, it in enumerate(items):
        if k in targeted or it.kind != "plain":
            mirror = {}
            flush()
        pos[k] = len(out) + len(cur)
        i = it.ins
        if i is not None and it.kind != "goto_pc":
            addrs.add(i.pc)
        if it.kind == "plain":
            words_ = prefix_all(i, d0) + it.body
            if cfold:
                words_ = constfold.fold(words_)
                try:
                    # the constants nothing reads any more, and whatever the folding made dead
                    words_, _ = armdce.dce(words_, it.live_out)
                except ValueError:
                    pass
            bad = any(
                isinstance(w, tuple) or (int(w) >> 25 & 7 == 5) or (int(w) >> 24 & 15) == 15 for w in words_
            )
            if bad and sched:
                flush()
                # the asm translator forgets what r0-r3 mirror in front of an instruction it does not schedule
                mirror = {}
            if forward:
                has_branch = any(
                    (not isinstance(w, tuple)) and int(w) >> 25 & 7 == 5 and not int(w) >> 24 & 1
                    for w in words_
                )
                words_, mirror = fwd.forward(words_, mirror, d0, may_delete=not has_branch)
            if any(
                isinstance(w, tuple) or (int(w) >> 25 & 7 == 5) or (int(w) >> 24 & 15) == 15 for w in words_
            ):  # a body with a branch, a call or an svc in it: not scheduled or allocated
                flush()
                pos[k] = len(out)
                out += words_
            else:
                cur += words_
        elif it.kind == "bcc":
            c = CC68[i.cond]
            out += (
                cond_branch(c, "item", it.tgt)
                if it.tgt is not None
                else cond_branch(c, "stub", stub(it.tgt_addr))
            )
        elif it.kind == "dbcc":
            c = CC68[i.cond]
            r = i.reg
            if c != "f":
                # the condition holds: no decrement, fall through to the next instruction
                out += cond_branch(c, "item", k + 1)
            # r3 = 1 when the counter ran out: skip the branch
            out += [
                ldrsh(0, 5, d0 + 4 * r + 2),
                sub(0, 0, ("imm", 1)),
                strh(0, 5, d0 + 4 * r + 2),
                add(3, 0, ("imm", 1)),
                clz(3, 3),
                lsr(3, 3, 5),
                add(15, 15, ("reg", 3, "lsl", 2)),
                nop(),
            ]
            out.append(("item", "al", it.tgt) if it.tgt is not None else ("stub", "al", stub(it.tgt_addr)))
        elif it.kind == "push":
            out += load_const(0, it.ret) + [str_(0, 13, -4, kind="pre")]
        elif it.kind == "retpop":
            out.append(add(13, 13, ("imm", 4)))
            if it.end_of is not None and it.end_of > k + 1:
                out.append(("item", "al", it.end_of))
        elif it.kind == "follow":
            pass
        elif it.kind == "retfollow":
            out.append(add(13, 13, ("imm", 4)))
        elif it.kind == "goto_pc":
            out += load_const(0, it.tgt_addr) + [("b", "al", GOTO)]
        elif it.kind == "call":
            terminators.add(i.pc)
            tgt = i.target if i.kind == "bsr" else i.static
            if tgt is not None:
                out += load_const(0, tgt)
            else:
                # JMP with the same <ea> leaves the target in r0
                out += prefix_all(i, d0) + inline_body(gen.generate(0x4EC0 | i.op & 0x3F), tail_ok=True)[:-1]
            out += load_const(3, it.ret) + [str_(3, 13, -4, kind="pre"), ("b", "al", GOTO)]
        elif it.kind == "rts":
            terminators.add(i.pc)
            out += list(gen.generate(0x4E75))
        elif it.kind == "jmp":
            terminators.add(i.pc)
            if i.static is not None:
                out += load_const(0, i.static) + [("b", "al", GOTO)]
            else:
                out += prefix_all(i, d0) + inline_body(gen.generate(i.op), tail_ok=True)
    flush()
    pos[n] = len(out)
    last = items[-1]
    if not (
        last.kind in ("goto_pc", "call", "rts", "jmp") or (last.kind == "bcc" and CC68[last.ins.cond] == "t")
    ):
        raise AssertionError("a block that does not end")
    stub_at = []
    for t in stubs:
        stub_at.append(len(out))
        out += load_const(0, t) + [("b", "al", GOTO)]

    def branch(cond, frm, to):
        return (CONDS.index(cond) << 28) | 0x0A000000 | ((to - frm - 2) & 0xFFFFFF)

    res = []
    for k, w in enumerate(out):
        if isinstance(w, tuple) and w[0] in ("rel", "item", "stub"):
            tgt = w[2] if w[0] == "rel" else pos[w[2]] if w[0] == "item" else stub_at[w[2]]
            res.append(branch(w[1], k, tgt))
        else:
            res.append(w)
    real = [it for it in items if it.ins is not None and it.kind != "goto_pc"]
    info = dict(
        instructions=len(real),
        guest_bytes=sum(it.ins.size for it in real),
        words=len(res),
        code_words=stub_at[0] if stub_at else len(res),
        stubs=len(stubs),
        addrs=addrs,
        terminators=terminators,
        items=items,
        inlined=sum(1 for it in items if it.kind == "push"),
        stop="goto_pc" if last.kind == "goto_pc" else last.kind,
        regions=regions,
        peaks=peaks,
        pos=pos,
    )
    return res, info
