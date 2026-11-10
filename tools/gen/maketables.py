#!/usr/bin/env python3
"""Writes the static tables (build/tables.s) from the host-side generator: no QEMU run, no dump.

  .venv/bin/python tools/gen/maketables.py > build/tables.s

What it builds, in the layout pjit.h describes (offsets from build/asm_offsets.inc):
  pjit_arena : cpu_t (zero), DT (leader word + interpretive word per opcode), the extension-class table, the
  OT, the per-mask
               MOVEM.L A7 table, the inline table, the cache links (zero)
  pjit_drsr  : the bodies (everything after an emitter's first word), interned and aligned to UNIT_WORDS words
  (16 bytes), padded with UDF.
Only the bodies the opcodes really use are written (not those of rows that a later row overrides).
Branches to runtime routines stay symbolic (`b pjit_goto`); a DT branch into the DRSR is written `b
pjit_drsr+N`; branches within
a body are raw relative words."""

import sys, os, re, struct, collections

sys.path.insert(0, os.path.dirname(__file__))
from gen import Gen, read_offsets
import abstract
from arm32 import *
import capstone

PAD, BXLR, UNIT_WORDS = 0xE7F000F0, 0xE12FFF1E, int(os.environ.get("PJIT_UNIT_WORDS", "4"))
USED = [0]  # words of code in the DRSR (without the padding to 64-byte units)
INFLATE = int(os.environ.get("PJIT_INFLATE", "1"))
NOP = 0xE320F000
_m68k = capstone.Cs(capstone.CS_ARCH_M68K, capstone.CS_MODE_M68K_000)


def m68k_text(op):
    """The 68K instruction an opcode is, for the comments (extension words shown as zero)."""
    i = next(_m68k.disasm(op.to_bytes(2, "big") + bytes(10), 0), None)
    return f"{i.mnemonic} {i.op_str}".strip() if i else "(illegal)"


CC = ["eq", "ne", "cs", "cc", "mi", "pl", "vs", "vc", "hi", "ls", "ge", "lt", "gt", "le", "al", "nv"]

# ---- the extension-class table and the OT class bits (pjit/ext_table.c, pjit/pjit.c) ----
ext = bytes(
    int(x, 16)
    for x in re.findall(
        r"0x([0-9A-Fa-f]{2})", open("pjit/ext_table.c").read().split("pjit_ext_table[65536] = {")[1]
    )
)
assert len(ext) == 65536
EXT_NONE, EXT_I16, EXT_I32, EXT_D8_XAREG, EXT_D16_PC, EXT_D8_XPC, EXT_BDISP = range(7)
OT_LEN_2, OT_SPECIAL, OT_LEN_4, OT_LEN_6 = 0, 1, 2, 3


def ext_word_count(op):
    e = ext[op]
    src = {EXT_I16: 1, EXT_I32: 2, EXT_D8_XAREG: 1, EXT_D16_PC: 1, EXT_D8_XPC: 1, EXT_BDISP: 1}.get(
        e >> 4 & 15, 0
    )
    dst = {EXT_I16: 1, EXT_I32: 2, EXT_D8_XAREG: 1, EXT_D16_PC: 1, EXT_D8_XPC: 1}.get(e & 15, 0)
    return 1 + src + dst


def ot_class(op):
    s, d = ext[op] >> 4 & 15, ext[op] & 15
    if op & 0xF000 == 0x6000 or op & 0xF0F8 == 0x50C8 or op & 0xFF80 == 0x4E80 or op == 0x4E75:
        return OT_SPECIAL
    if s in (EXT_D8_XAREG, EXT_D16_PC, EXT_D8_XPC) or d in (EXT_D8_XAREG, EXT_D16_PC, EXT_D8_XPC):
        return OT_SPECIAL
    if 1 <= op >> 12 <= 3 and d != EXT_NONE:
        return OT_SPECIAL
    return {1: OT_LEN_2, 2: OT_LEN_4, 3: OT_LEN_6}.get(ext_word_count(op), OT_SPECIAL)


# ---- can an ARM word be copied into a cache slot and run there? ----
def inlinable(w):
    if isinstance(w, tuple) or w >> 28 == 0xF:
        return False
    if w == 0xE320F000:
        return True  # NOP: it only looked non-inlinable because its Rd field reads as r15
    rn, rd, rs, rm = w >> 16 & 15, w >> 12 & 15, w >> 8 & 15, w & 15
    bad = lambda r: r >= 14
    k = w >> 25 & 7
    if k == 0:
        if w & 0x0FFFFFF0 == 0x012FFF10:
            return False
        if w & 0x0FC000F0 == 0x00000090:
            return not (bad(rn) or bad(rd) or bad(rs) or bad(rm))
        if w & 0x0E000090 == 0x00000090:
            return not (bad(rn) or bad(rd) or (not w & 0x00400000 and bad(rm)))
        if w & 0x0F900000 == 0x01000000:
            return not (bad(rd) or bad(rm))
        return not (bad(rn) or bad(rd) or bad(rm) or (w & 0x10 and bad(rs)))
    if k == 1:
        if w & 0x0FB00000 == 0x03000000:
            return not bad(rd)
        return not (bad(rn) or bad(rd))
    if k == 2:
        return not (bad(rn) or bad(rd))
    if k == 3:
        if w & 0x10:
            return not (bad(rd) or bad(rm))
        return not (bad(rn) or bad(rd) or bad(rm))
    return False


# ---- the per-mask MOVEM.L A7 table ----
def movem_table():
    """Per mask over D2-D7 / A2-A6 (packed 0-2047): the routine that saves (half 0: MOVEM.L regs,-(A7)) or
    restores (half 1: MOVEM.L (A7)+,regs)
    exactly those registers, as LDM/STM runs. Entry: a guard that sends a misaligned stack to the generic
    routine.
    """
    out = []
    for half in range(2):
        for idx in range(2048):
            m = ((idx & 0x3F) << 2) | ((idx >> 6) << 10)
            # the guard: an aligned stack (A7 & 2 == 0) lands on the first word of the body in one jump; a
            # stack that is only halfword aligned lands
            # 16 words on, on the generic code with the mask the instruction had (LDM/STM fault on a
            # misaligned address)
            p = [and_(3, 13, ("imm", 2)), add(15, 15, ("reg", 3, "lsl", 5)), nop()]
            aregs = [6 + k for k in range(2, 7) if m & (1 << (8 + k))]  # A(k) = r(6+k)
            if half == 0:
                if aregs:
                    p.append(stm(13, aregs, mode="db", wb=True))
                hi = 7
                while hi >= 2:
                    if not m & (1 << hi):
                        hi -= 1
                        continue
                    lo = hi
                    while lo > 2 and m & (1 << (lo - 1)) and hi - lo + 1 < 3:
                        lo -= 1
                    n = hi - lo + 1
                    off = 64 - 4 * lo
                    if n == 1:
                        p += [ldr(0, 5, -off), str_(0, 13, -4, kind="pre")]
                    else:
                        p += [
                            sub(3, 5, ("imm", off)),
                            ldm(3, list(range(n))),
                            stm(13, list(range(n)), mode="db", wb=True),
                        ]
                    hi = lo - 1
            else:
                lo = 2
                while lo <= 7:
                    if not m & (1 << lo):
                        lo += 1
                        continue
                    hi = lo
                    while hi < 7 and m & (1 << (hi + 1)) and hi - lo + 1 < 3:
                        hi += 1
                    n = hi - lo + 1
                    off = 64 - 4 * lo
                    if n == 1:
                        p += [ldr(0, 13, 4, kind="post"), str_(0, 5, -off)]
                    else:
                        p += [
                            ldm(13, list(range(n)), wb=True),
                            sub(3, 5, ("imm", off)),
                            stm(3, list(range(n))),
                        ]
                    lo = hi + 1
                if aregs:
                    p.append(ldm(13, aregs, wb=True))
            p.append(bx("lr"))
            assert len(p) <= 19, (half, idx, len(p))
            raw = m if half else int("{:016b}".format(m)[::-1], 2)  # -(An) masks are the other way round
            p += [0] * (19 - len(p)) + [
                movw(1, raw),
                sym("movem_sp_restore_generic" if half else "movem_sp_save_generic"),
            ]
            out += p + [0] * (32 - len(p))
    return out


def fold_bits(seq):
    """Bit 0 / bit 1: the leader of this sequence is a load or store whose offset register is the fill prefix's
    r1 / r2 (a plain register offset, no
    shift, P=1, W=0) and that register is not read again afterwards (the leader loads into it, or no later
    word uses it, and no later word is a symbolic
    branch into a runtime routine that might). pjit_fill then turns a constant 16-bit displacement into the
    immediate-offset form of the leader and
    drops the `movw` / `sxth` that built it: `ldr r0,[sp,#16]` in place of three instructions."""
    L = int(seq[0])
    if (L & 0x0E000FF0) == 0x06000000:
        pass  # ldr / str / ldrb / strb [Rn, Rm]
    elif (L & 0x0E400F90) == 0x00000090 and (L & 0x60):
        pass  # ldrh / strh / ldrsb / ldrsh [Rn, Rm]
    else:
        return 0
    if (L & 0x01200000) != 0x01000000:
        return 0  # P = 1 (offset addressing), W = 0
    rm, rd = L & 15, L >> 12 & 15
    if rm not in (1, 2) or (L >> 16 & 15) == rm:
        return 0
    if rm != rd:
        for w in seq[1:]:
            if isinstance(w, tuple):
                return 0
            if rm in (int(w) >> 16 & 15, int(w) >> 12 & 15, int(w) >> 8 & 15, int(w) & 15):
                return 0
    return 1 << (rm - 1)


def build(g, gx=None, lib=None, collect=None):
    drsr, index, first = [], {}, {}
    pad = udf0()
    cur = [0xFFFF]
    crsr, unit_info = [], {}

    def normalise(body):
        """-> (the body's words without the final `bx lr`, a conditional `bx lr` turned into a branch to the
        end; info byte)
        """
        if not body or body[-1] != BXLR or any(isinstance(w, tuple) for w in body):
            return None, 0
        words = list(body[:-1])
        L = len(words)
        if L > 120:
            return None, 0x80
        for k, w in enumerate(words):
            if inlinable(w):
                continue
            if w >> 25 & 7 == 5 and not w >> 24 & 1 and w >> 28 != 0xF:  # b<cc> inside the body
                tgt = k + 2 + (((w & 0xFFFFFF) ^ 0x800000) - 0x800000)
                if k + 1 < tgt <= L:
                    continue
            elif (
                w & 0x0FFFFFFF == 0x012FFF1E and w >> 28 != 0xE
            ):  # bx<cc> lr: an early exit, to the end of the body
                words[k] = (w & 0xF0000000) | 0x0A000000 | ((L - k - 2) & 0xFFFFFF)
                continue
            return None, 0x80
        return words, 0x80 | (L + 1)

    def intern(body, raw=False):
        if collect is not None:
            collect.setdefault(tuple(body), set()).add(cur[0])
        if lib is not None and not raw and tuple(body) in lib.map:
            body = lib.map[tuple(body)]
        if lib is not None:
            body = [
                (
                    ("b", "al", "pjit_drsr+%d" % (intern(lib.routines[w[2]], True) * UNIT_WORDS * 4))
                    if isinstance(w, tuple) and w[0] == "REF"
                    else w
                )
                for w in body
            ]
        exp = []
        # ('CALL', 'al', name): a routine the generator named (gen.routines), entered with `add r4,pc,#0 ; b`
        # and left with `bx r4`
        for w in body:
            if isinstance(w, tuple) and w[0] == "CALL":
                exp += [
                    abstract.ADD_R4_PC,
                    ("b", "al", "pjit_drsr+%d" % (intern(g.routines[w[2]], True) * UNIT_WORDS * 4)),
                ]
            else:
                exp.append(w)
        body = exp
        key = tuple(body)
        if key not in index:
            index[key] = len(drsr) // UNIT_WORDS
            first[index[key]] = cur[0]
            USED[0] += len(body)
            drsr.extend(body)
            drsr.extend([pad] * (-len(body) % UNIT_WORDS))
            # PJIT_INFLATE=n: n times the footprint (an experiment: does the size of the tables matter on the
            # board?)
            drsr.extend([pad] * (UNIT_WORDS * (INFLATE - 1)))
            nw, info = normalise(body)
            unit_info[index[key]] = info
            crsr.extend(
                (nw if nw is not None else [])
                + [pad] * (len(drsr) - len(crsr) - (len(nw) if nw is not None else 0))
            )
        return index[key]

    illegal = intern([bx("lr")])
    dt, ot, ot_x, fold, form = [], [], [], [], []
    # Sharing a routine: a sequence of more than three words is split after its first m words (m = 2 or 3)
    # when what follows is the same for at least three opcodes (the
    # first m words name the registers, the tail is the operation). m = 1 is the leader + shared
    # body layout, which stays as it is.
    allseq = [g.generate(op) for op in range(65536)]

    def plain_words(ws):
        return all(isinstance(w, int) and inlinable(w) for w in ws)

    def tail_ok(t):  # a branch in the tail must stay inside the tail
        for k, w in enumerate(t):
            if (
                isinstance(w, int)
                and w >> 25 & 7 == 5
                and w >> 28 != 0xF
                and k + 2 + (((w & 0xFFFFFF) ^ 0x800000) - 0x800000) < 0
            ):
                return False
        return True

    tails = collections.Counter()
    for s in allseq:
        if (
            s is not None
            and gx is None
            and not getattr(s, "split", 0)
            and len(s) > 4
            and all(isinstance(w, int) for w in s)
        ):
            for m in (2, 3):
                tails[tuple(s[m:])] += 1

    def choose_split(seq):
        if (
            gx is not None
            or os.environ.get("PJIT_NOAUTOSPLIT")
            or os.environ.get("PJIT_NOSHARE")
            or hasattr(seq, "shared")
            or getattr(seq, "split", 0)
            or len(seq) <= 4
            or not all(isinstance(w, int) for w in seq)
            or seq[-1] != BXLR
        ):
            return 0
        for m in (2, 3):
            if tails[tuple(seq[m:])] >= 3 and plain_words(seq[:m]) and tail_ok(seq[m:]):
                return m
        return 0

    for op in range(65536):
        cur[0] = op
        seq, seqi = allseq[op], g.generate(op, interp=True)
        # the compact form (calls into generator routines) when the generator has one; the flat one is what
        # the checks run
        seq_d, seqi_d = getattr(seq, "shared", seq), getattr(seqi, "shared", seqi)
        if gx is not None:  # the exact twin (it saves X): the same leader, so the DT is shared
            seqx = gx.generate(op)
            assert seqx[0] == seq[0], f"{op:04X}: the exact and lazy sequences have different leaders"
            ot_x.append(intern(seqx[max(1, getattr(seqx, "split", 0)) :]) << 2 | ot_class(op))
            # the interpreter always tracks X exactly (only cached code is lazy)
            seqi = gx.generate(op, interp=True)
        assert seq is not None and len(seq) >= 2, f"{op:04X}"
        split = getattr(seq, "split", 0) or choose_split(seq)
        if (
            not split
            and gx is None
            and not os.environ.get("PJIT_NOAUTOSPLIT")
            and list(seqi) == list(seq)
            and 2 <= len(seq) <= 4
            and seq[-1] == BXLR
            and all(isinstance(w, int) and inlinable(w) for w in seq[:-1])
        ):
            # a sequence of at most three plain words: all of it is the opcode's own code, the routine is a
            # bare `bx lr`
            split = len(seq) - 1
        if split:
            # the target layout: `split` words of the opcode's own code, then a routine shared with the
            # family. The DT entry is [own words, nop padding, b routine] -- the same in
            # both worlds (a split sequence has no interpretive twin); the OT names the routine (the fill
            # copies the own words from the DT and then inlines / `bl`s the routine).
            assert 1 <= split <= 3, f"{op:04X}"
            if list(seqi) != list(seq):
                assert getattr(seqi, "split", 0) == split and list(seqi[:split]) == list(
                    seq[:split]
                ), f"{op:04X}: the interpretive sequence splits differently"
            assert all(
                isinstance(w, int) and inlinable(w) for w in seq[:split]
            ), f"{op:04X}: an own word that cannot be copied into a slot"
            u = intern(seq[split:])
            if gx is not None:
                assert getattr(seqx, "split", 0) == split and list(seqx[:split]) == list(
                    seq[:split]
                ), f"{op:04X}: the exact twin splits differently"
            ot.append(u << 2 | ot_class(op))
            fold.append(
                fold_bits(seq) & (fold_bits(seqx) if gx is not None else 3)
                if op not in (0x48E7, 0x4CDF)
                else 0
            )
            form.append(split)
            ui = u if list(seqi) == list(seq) else intern(seqi[split:])
            term = bx("lr") if seqi[split:] == [BXLR] else ("b", "al", "pjit_drsr+%d" % (ui * UNIT_WORDS * 4))
            # the terminal word follows the own words directly: the interpreter runs no padding
            dt += list(seq[:split]) + [term] + [NOP] * (3 - split)
            continue
        form.append(0)
        if seqi[0] != seq[0]:
            seqi = seq  # the leader runs in both worlds: they must agree
        assert not (
            isinstance(seq[0], int) and seq[0] >> 25 & 7 == 5
        ), f"{op:04X}: a relative branch as a leader"
        u = intern(seq_d[1:])
        ot.append(u << 2 | ot_class(op))
        fold.append(
            fold_bits(seq) & (fold_bits(seqx) if gx is not None else 3) if op not in (0x48E7, 0x4CDF) else 0
        )
        w1 = (
            seqi_d[1]
            if len(seqi_d) == 2
            else ("b", "al", "pjit_drsr+%d" % (intern(seqi_d[1:]) * UNIT_WORDS * 4))
        )
        dt += [seq[0], w1, NOP, NOP]  # a DT entry is one cache slot (4 words); words 2 and 3 are not used yet

    def compact_table(table):
        """Per opcode, for the loop compactor (pjit_fill, compact_try): bits 0-6 = the body's length in words + 1
        when it can be copied into a block (straight line, ends in an
        unconditional `bx lr`, every word an inlinable one or a forward branch inside the body; a conditional
        `bx lr` was rewritten to a branch to the end: see crsr), else 0;
        bit 7 = the body may be *called* from a block (it ends in `bx lr`, no word branches out to a runtime
        routine, nothing reads lr).
        """
        return [unit_info[table[op] >> 2] for op in range(65536)]

    def inline_table(table):
        inl = []
        for op in range(65536):
            e = drsr[(table[op] >> 2) * UNIT_WORDS :][:16]
            v = 0
            for k in range(16):
                if k < len(e) and e[k] == BXLR:
                    v = k + 1
                    break
                if k >= len(e) or not inlinable(e[k]):
                    break
            inl.append(v)
        return inl

    inl = inline_table(ot)
    if gx is None:
        ot_x = list(ot)
    return (
        dt,
        ot,
        drsr,
        inl,
        first,
        ot_x,
        inline_table(ot_x),
        fold,
        compact_table(ot),
        compact_table(ot_x),
        crsr,
        form,
    )


def xcons_table():
    """Per opcode, for the scan that decides whether a cache line is near an instruction that reads X
    (pjit_fill): bits 0-1 say whether the
    opcode reads X (1 = a pattern no 68000 brief extension word can have, so safe to test on every word; 2 =
    one that an extension word can
    look like: only trusted at an instruction start), bits 2-4 the number of extension words, so the scan can
    step over them, bits 5-6 the displacement-folding marks of fold_bits().
    Readers of X: ADDX SUBX ABCD SBCD ROXL (bit 8 set: never an extension word); NEGX, MOVE from SR, NBCD,
    ROXR (can collide with the
    extension word of an indexed mode: 0x4xxx is D4)."""
    t = []
    for op in range(65536):
        free = (
            ((op & 0xF130) in (0xD100, 0x9100) and (op & 0xC0) != 0xC0)
            or (op & 0xF1F0) in (0xC100, 0x8100)
            or ((op & 0xF118) == 0xE110 and (op & 0xC0) != 0xC0)
            or (op & 0xFFC0) == 0xE5C0
        )
        prone = (
            (op & 0xFF00) == 0x4000
            or (op & 0xFFC0) == 0x4800
            or ((op & 0xF018) == 0xE010 and (op & 0xC0) != 0xC0)
            or (op & 0xFFC0) == 0xE4C0
        )
        t.append((1 if free else 2 if prone else 0) | (ext_word_count(op) - 1) << 2)
    return t


def text(words, out, plain=False, notes=None):
    """Assembly lines for a word list: an instruction with text as itself, a symbolic branch as `b name`, a bare
    word as `.word`.
    plain=True writes every word as `.word` (the cross-check variant, `make check-asm`); notes maps a word
    index to a comment line.
    """
    buf = []

    def flush():
        nonlocal buf
        for i in range(0, len(buf), 8):
            out.append("\t.word\t" + ",".join("0x%08X" % x for x in buf[i : i + 8]))
        buf = []

    for i, w in enumerate(words):
        if notes and not plain and i in notes:
            flush()
            out.append("\t@ " + notes[i])
        if isinstance(w, tuple):
            flush()
            out.append("\t%s%s\t%s" % (w[0], "" if w[1] == "al" else w[1], w[2]))
        elif not plain and getattr(w, "text", None):
            flush()
            out.append("\t" + w.text)
        else:
            buf.append(int(w))
    flush()


def text_bytes(vals, unit, out):
    fmt, step = {1: (".byte", 16), 2: (".hword", 8)}[unit]
    for i in range(0, len(vals), step):
        out.append("\t%s\t%s" % (fmt, ",".join(str(v) for v in vals[i : i + step])))


def main():
    offs = read_offsets()
    lazy = os.environ.get("PJIT_X_LAZY", "0") == "1"
    g = Gen(offs, lazy_x=lazy)
    gx = Gen(offs, lazy_x=False) if lazy else None  # lazy tables + their exact twins, or just the exact ones
    plain = "--words" in sys.argv  # every word as .word: the variant `as` is checked against
    memo = {}
    gen0 = g.generate

    def memo_gen(op, interp=False):
        k = (op, interp)
        if k not in memo:
            memo[k] = gen0(op, interp=interp)
        return memo[k]

    g.generate = memo_gen
    if gx is not None:
        genx = gx.generate
        memox = {}

        def memo_genx(op, interp=False):
            k = (op, interp)
            if k not in memox:
                memox[k] = genx(op, interp=interp)
            return memox[k]

        gx.generate = memo_genx
    lib = None
    if not os.environ.get("PJIT_NOABSTRACT"):
        users = {}
        build(g, gx, collect=users)
        import weights

        profs = weights.profiles()
        # the budget: the share of the weighted executed instructions that may pay the extra branch
        # (abstract.py: 0.003 is the knee)
        eps = os.environ.get("PJIT_EPS", "0.003")
        chosen, cum, ops = abstract.budget_eligible(users, weights.combined(profs), float(eps))
        sys.stderr.write(
            "maketables: budget %s: %d bodies eligible; share of executed instructions affected: %s\n"
            % (
                eps,
                len(chosen),
                "  ".join("%s %.3f%%" % (n, 100 * sum(h.get(op, 0) for op in ops)) for n, h in profs.items()),
            )
        )
        lib = abstract.analyse(
            users,
            chosen,
            log=(lambda m: sys.stderr.write(m + "\n")) if os.environ.get("PJIT_STATS") else None,
        )
        # a rewritten body must expand to exactly the original
        for key, nb in lib.map.items():
            assert [int(w) if not isinstance(w, tuple) else w for w in abstract.expand(nb, lib)] == [
                int(w) if not isinstance(w, tuple) else w for w in key
            ]
        sys.stderr.write(
            "maketables: abstraction saves about %d words (%d bodies rewritten, %d routines)\n"
            % (lib.saved, len(lib.map), len(lib.routines))
        )
    USED[0] = 0
    dt, ot, drsr, inl, first, ot_x, inl_x, fold, cinl, cinl_x, crsr, form = build(g, gx, lib=lib)
    if os.environ.get("PJIT_STATS"):  # which families own the DRSR: words of code per emitter, bodies, users
        pad = udf0()
        users = collections.Counter(w >> 2 for w in ot)
        starts = sorted(first)
        fam = collections.defaultdict(lambda: [0, 0, 0, 0])
        for k, u in enumerate(starts):
            end = (starts[k + 1] if k + 1 < len(starts) else len(drsr) // UNIT_WORDS) * UNIT_WORDS
            body = drsr[u * UNIT_WORDS : end]
            while body and body[-1] == pad:
                body = body[:-1]
            f = fam[(g.winner[first[u]] or "illegal")[5:]]
            f[0] += len(body)
            f[1] += 1
            f[2] += users[u]
            if cinl[first[u]] == 0:
                f[3] += len(body)
        tot = [0, 0]
        for name, (w, n, us, nc) in sorted(fam.items(), key=lambda kv: -kv[1][0])[:45]:
            sys.stderr.write(
                "%-22s %6d words %5d bodies %6d users   %6d words not compactable\n" % (name, w, n, us, nc)
            )
        sys.stderr.write(
            "total %d words, %d not compactable\n"
            % (sum(v[0] for v in fam.values()), sum(v[3] for v in fam.values()))
        )
    cpu = offs["CPU_SIZE"]
    off = {
        "dt": cpu,
        "ext": cpu + offs["ARENA_EXT"],
        "ot": cpu + offs["ARENA_OT"],
        "movem": cpu + offs["ARENA_MOVEM"],
        "inl": cpu + offs["ARENA_INL"],
        "links": cpu + offs["ARENA_LINKS"],
        "ot_x": cpu + offs["ARENA_OTX"],
        "inl_x": cpu + offs["ARENA_INLX"],
        "xcons": cpu + offs["ARENA_XCONS"],
        "xexact": cpu + offs["ARENA_XEXACT"],
        "cinl": cpu + offs["ARENA_CINL"],
        "cinl_x": cpu + offs["ARENA_CINLX"],
        "form": cpu + offs["ARENA_FORM"],
    }
    assert off["ext"] - off["dt"] == 16 * 65536
    out = []

    def at(k, what):
        out.append(
            f'\t.if (. - pjit_arena) != {off[k]}\n\t.error "{what} is not at arena offset {off[k]}"\n\t.endif'
        )

    out.append("@ Generated by tools/gen/maketables.py (the host-side generator): do not edit.")
    out.append(
        '\t.syntax unified\n\t.arm\n\t.section .data.pjit_arena,"aw",%progbits\n\t.balign 64\n\t.global pjit_arena\n\t.type pjit_arena, %object\npjit_arena:'
    )
    out.append(f"\t.skip\t{off['dt']}\t\t@ cpu_t: zero (pjit_cache_init fills it in)")
    at("dt", "dt")
    text(
        dt,
        out,
        plain,
        {4 * op: f"{op:04X}  {m68k_text(op)}   [{(g.winner[op] or 'illegal')[5:]}]" for op in range(65536)},
    )
    at("ext", "ext")
    text_bytes(list(ext), 1, out)
    at("ot", "ot")
    text_bytes(ot, 2, out)
    at("movem", "movem")
    text(movem_table(), out, plain)
    at("inl", "inl")
    text_bytes(inl, 1, out)
    at("links", "links")
    out.append(f"\t.skip\t{off['ot_x'] - off['links']}\t\t@ links: zero")
    at("ot_x", "ot_x")
    text_bytes(ot_x, 2, out)
    at("inl_x", "inl_x")
    text_bytes(inl_x, 1, out)
    at("xcons", "xcons")
    text_bytes([a | f << 5 for a, f in zip(xcons_table(), fold)], 1, out)
    at("cinl", "cinl")
    text_bytes(cinl, 1, out)
    at("cinl_x", "cinl_x")
    text_bytes(cinl_x, 1, out)
    at("form", "form")
    text_bytes(form, 1, out)
    at("xexact", "xexact")
    out.append(f"\t.skip\t{offs['ARENA_SIZE'] - off['xexact']}\t\t@ xexact: zero")
    out.append("\t.size pjit_arena, . - pjit_arena")
    out.append(
        '\n\t.section .data.pjit_drsr,"aw",%progbits\n\t.balign 64\n\t.global pjit_drsr\n\t.type pjit_drsr, %object\npjit_drsr:'
    )
    text(
        drsr,
        out,
        plain,
        {
            u
            * UNIT_WORDS: f"{first[u]:04X}  {m68k_text(first[u])}   [{(g.winner[first[u]] or 'illegal')[5:]}]"
            for u in first
        },
    )
    out.append("\t.size pjit_drsr, . - pjit_drsr")
    out.append(
        '\n\t.section .data.pjit_crsr,"aw",%progbits\n\t.balign 64\n\t.global pjit_crsr\n\t.type pjit_crsr, %object\npjit_crsr:'
    )
    text(crsr, out, plain)
    out.append("\t.size pjit_crsr, . - pjit_crsr")
    print("\n".join(out))
    sys.stderr.write(
        "maketables: %d DRSR bytes (%d units), %d distinct bodies; %d bytes of code, %.0f%% padding\n"
        % (
            len(drsr) * 4,
            len(drsr) // UNIT_WORDS,
            len(set(map(lambda i: i, ot))),
            USED[0] * 4,
            100 - 100 * USED[0] * 4 / (len(drsr) * 4),
        )
    )


if __name__ == "__main__":
    main()
