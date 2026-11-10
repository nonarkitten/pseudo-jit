"""Register allocation over a straight-line region of ARM words: the sequences are written with r0-r3 hard-wired
as scratch; here those registers are *virtual* -- every
definition starts a new value, every use names the value it reads -- and a linear scan hands physical
registers out of a pool (r0-r4 and lr by default) to the values
as they become live. The A registers (r6-r13), the base r5, the flags and memory are not touched.

That does three things at once: it lifts the "r0-r3 only" limit (two 68K instructions can overlap, a value can
stay in r4 across instructions), it shows how many
registers the region really needs (`pressure`), and it is the substrate the scheduler and register homes work
on.

A word's register fields are classified by encoding (data processing, multiply, loads and stores, extends, bit
fields, media, mrs / msr, movw / movt ...); a word the
table does not know makes the region unallocatable (returned unchanged). The table is checked against
capstone's read / write sets in test_fields().

  allocate(words, pool=POOL, order=None, prefer=True) -> (words, peak)   or (None, None) if the region cannot be allocated
  order: optionally the indices of `words` in the order to be emitted (a schedule); definitions and uses are
  renamed along the *original* program order, then emitted
         in the new order, and the scan runs over the new order."""

import armdce

POOL = (0, 1, 2, 3, 4, 14)
TRACKED = (0, 1, 2, 3)


def fields(w):
    """[(shift, 'u' | 'd' | 'ud')] for the register fields of an ARM word, or None if the encoding is not in
    the table.
    """
    if isinstance(w, tuple):
        return None
    w = int(w)
    cond = w >> 28
    if cond == 15:
        return None
    k = w >> 25 & 7
    if (w & 0x0FFF03F0) in (0x06AF0070, 0x06BF0070, 0x06EF0070, 0x06FF0070):
        return [(12, "d"), (0, "u")]  # sxtb sxth uxtb uxth
    if w & 0x0FF00000 == 0x03000000:
        return [(12, "d")]  # movw
    if w & 0x0FF00000 == 0x03400000:
        return [(12, "ud")]  # movt
    if w & 0x0FE0007F == 0x07C0001F and False:
        return None
    if w & 0x0FE00070 == 0x07C00010 and (w & 15) != 15:
        return [(12, "ud"), (0, "u")]  # bfi
    if w & 0x0FE0007F == 0x07C0001F:
        return [(12, "ud")]  # bfc
    if w & 0x0FA00070 == 0x07A00050:
        return [(12, "d"), (0, "u")]  # ubfx sbfx
    if w & 0x0FFF0FF0 == 0x016F0F10:
        return [(12, "d"), (0, "u")]  # clz
    if w & 0x0FFF0FF0 in (0x06BF0F30, 0x06BF0FB0, 0x06FF0FB0, 0x06FF0F30):
        return [(12, "d"), (0, "u")]  # rev rev16 revsh rbit
    if w & 0x0FF00FF0 == 0x06500F90 or w & 0x0FF00FF0 == 0x06800FB0:
        return [(16, "u"), (12, "d"), (0, "u")]  # uadd8 / sel
    if w & 0x0FFF0FFF == 0x010F0000:
        return [(12, "d")]  # mrs rd, apsr
    if w & 0x0FB0FFF0 == 0x0120F000 and not w & 0x00400000:
        return [(0, "u")]  # msr apsr_..., rm
    if w == (cond << 28 | 0x0320F000):
        return []  # nop
    if k == 0 and (w & 0x0FC000F0) == 0x00000090:  # mul / mla
        f = [(16, "d"), (0, "u"), (8, "u")]
        if w >> 21 & 1:
            f.append((12, "u"))
        return f
    if k == 0 and (w & 0x90) == 0x90 and w & 0x60:  # ldrh / ldrsb / ldrsh / strh
        load = w >> 20 & 1
        wb = w >> 21 & 1 or not w >> 24 & 1
        f = [(12, "d" if load else "u"), (16, "ud" if wb else "u")]
        if not w >> 22 & 1:
            f.append((0, "u"))
        return f
    if k in (2, 3) and not (k == 3 and w & 0x10):  # ldr / str / ldrb / strb
        load = w >> 20 & 1
        wb = w >> 21 & 1 or not w >> 24 & 1
        f = [(12, "d" if load else "u"), (16, "ud" if wb else "u")]
        if k == 3:
            f.append((0, "u"))
        return f
    if (
        k == 0 and w >> 21 & 15 == 6 and not w & 0xFF0 and (w >> 16 & 15) == (w >> 12 & 15) == (w & 15)
    ):  # sbc rX,rX,rX: -(not C), whatever rX held: it only defines rX
        return [(12, "d"), (16, "d"), (0, "d")]
    dp = (
        k in (0, 1)
        and not (k == 0 and (w & 0x90) == 0x90)
        and not (k == 0 and (w & 0x0F900000) == 0x01000000)
        and not (k == 1 and (w & 0x0F900000) == 0x03000000 and not w >> 20 & 1)
    )
    if dp:
        opc = w >> 21 & 15
        f = []
        if opc not in (13, 15):
            f.append((16, "u"))
        if opc not in (8, 9, 10, 11):
            f.append((12, "d"))
        if k == 0:
            f.append((0, "u"))
            if w & 0x10:
                f.append((8, "u"))
        return f
    return None


def regs_of(w, f):
    """(reads, writes) as sets of register numbers according to the field table."""
    rd, wr = set(), set()
    for sh, kind in f:
        r = w >> sh & 15
        if "u" in kind:
            rd.add(r)
        if "d" in kind:
            wr.add(r)
    return rd, wr


class Fail(Exception):
    pass


def rename(words):
    """-> (list of (word, [(shift, vreg)] in field order), the vregs' (first def index, last use index,
    preferred physical)). Raises Fail.
    """
    cur = {}
    nv = [0]
    info = {}
    out = []

    def fresh(p, idx):
        v = nv[0]
        nv[0] += 1
        info[v] = [idx, idx, p]
        return (
            v  # (only the preferred register, info[v][2], is used: lifetimes are recomputed from the fields)
        )

    for idx, w in enumerate(words):
        f = fields(w)
        if f is None:
            raise Fail("unknown encoding %08x" % (int(w) if not isinstance(w, tuple) else 0))
        w = int(w)
        conditional = w >> 28 != 14
        sub = []
        uses = [(sh, w >> sh & 15) for sh, kind in f if "u" in kind]
        for sh, r in uses:
            if r in TRACKED:
                if r not in cur:
                    raise Fail("r%d read before it is defined" % r)
        # all reads of this word see the old values: record them before any definition
        read_v = {(sh): cur.get(r) for sh, r in uses}
        defs = {}
        for sh, kind in f:
            r = w >> sh & 15
            if "d" in kind and r in TRACKED:
                if conditional or "u" in kind:  # keeps the old value when it does not write: the same value
                    if r not in cur:
                        raise Fail("conditional write to an undefined r%d" % r)
                    defs[sh] = cur[r]
                else:
                    defs[sh] = None  # a new value (allocated below, once per register)
        newv = {}
        for sh, v in list(defs.items()):
            if v is None:
                r = w >> sh & 15
                if r not in newv:
                    newv[r] = fresh(r, idx)
                defs[sh] = newv[r]
        for r, v in newv.items():
            cur[r] = v
        for sh, kind in f:
            r = w >> sh & 15
            if r not in TRACKED:
                continue
            if "u" in kind and "d" not in kind:
                sub.append((sh, read_v[sh]))
            elif "d" in kind:
                sub.append((sh, defs[sh]))
        # a plain-read field of a register this word also redefines must keep the OLD value's name: handled by
        # read_v above (it was captured first)
        out.append((w, sub))
    return out, info


def allocate(words, pool=POOL, order=None, prefer=True):
    try:
        ren, info = rename(words)
    except Fail:
        return None, None
    n = len(words)
    seq = list(range(n)) if order is None else list(order)
    # intervals in the emitted order
    pos = {orig: p for p, orig in enumerate(seq)}
    first = {}
    last = {}
    for orig in seq:
        w, sub = ren[orig]
        p = pos[orig]
        for sh, v in sub:
            first[v] = min(first.get(v, p), p)
            last[v] = max(last.get(v, p), p)
    # a value is live from its first appearance to its last (reads included); registers can be reused at the
    # point of the last read
    assign = {}
    free = list(pool)
    busy = {}  # busy: physical -> value
    peak = 0
    ends = sorted(last.items(), key=lambda kv: kv[1])
    by_start = sorted(first, key=lambda v: (first[v], v))
    active = []
    for v in by_start:
        s = first[v]
        for a in list(
            active
        ):  # expire values whose last use is before this definition (<=: reuse in the same word is fine)
            if last[a] <= s and not (last[a] == s and first[a] == s and False):
                active.remove(a)
                free.append(assign[a])
        pref = info[v][2] if prefer else None
        cand = [r for r in pool if r in free]
        if not cand:
            return None, None
        r = pref if pref in cand else cand[0]
        free.remove(r)
        assign[v] = r
        active.append(v)
        peak = max(peak, len(active))
    out = []
    for orig in seq:
        w, sub = ren[orig]
        for sh, v in sub:
            w = (w & ~(15 << sh)) | assign[v] << sh
        out.append(w)
    return out, peak


def test_fields(words_iter):
    """Checks the field table against capstone: the registers it says are read / written (r0-r15) equal
    armdce.analyse's. Returns the number of words checked.
    """
    n = 0
    for w in words_iter:
        if isinstance(w, tuple):
            continue
        f = fields(w)
        if f is None:
            continue
        if (
            int(w) >> 21 & 15 == 6
            and int(w) >> 25 & 7 == 0
            and not int(w) & 0xFF0
            and (int(w) >> 16 & 15) == (int(w) >> 12 & 15) == (int(w) & 15)
        ):
            n += 1
            continue  # the sbc idiom: capstone sees reads
        a = armdce.analyse(w)
        rd, wr = regs_of(int(w), f)
        ar = {r for r in range(16) if a.reads >> r & 1}
        aw = {r for r in range(16) if a.writes >> r & 1}
        if (rd != ar or wr != aw) and not a.barrier:
            raise AssertionError(
                "%08x: fields say read %s write %s, capstone read %s write %s"
                % (int(w), sorted(rd), sorted(wr), sorted(ar), sorted(aw))
            )
        n += 1
    return n
