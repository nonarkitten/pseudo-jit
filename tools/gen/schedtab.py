"""Bit-exact Python mirror of the word decoder in pjit/blksched.s (what the asm scheduler knows about an ARM
word), checked against armdce.analyse / regalloc.fields
over every word the generator can emit (tools/gen/schedtab.py --check).

decode(w) -> info (int) or None (unknown encoding: the region is not scheduled).
info bits: 3:0 f0, 7:4 f1, 11:8 f2, 15:12 f3: a register field of the word: bits 1:0 kind (0 none, 1 read, 2
write, 3 read + write), bits 3:2 the field's shift (0: 0, 1: 8, 2: 12, 3: 16);
           18:16 flag class (0 none, 1 data processing, 2 multiply, 3 SIMD (writes GE), 4 sel (reads GE), 5
           mrs, 6 msr), 20:19 unit (0 ALU, 1 load / store, 2 multiply),
           21 load, 22 store, 25:23 latency (cycles until a consumer may issue)
flags(w, info) -> (reads, writes)  N=1 Z=2 C=4 V=8 G=16; conditional words read what they write; a maybe-write
is a read too.
mem(w, info, d0) -> (reads, writes) resource bits: 0-7 D register slot k, 8 other cpu state, 9 guest memory /
stack.
"""

import sys, os

sys.path.insert(0, os.path.dirname(__file__))
SH = {0: 0, 8: 1, 12: 2, 16: 3}
SHV = [0, 8, 12, 16]
K_U, K_D, K_UD = 1, 2, 3


def pack(fields, fc=0, unit=0, load=0, store=0, lat=1):
    v = 0
    for j, (sh, k) in enumerate(fields):
        v |= (k | SH[sh] << 2) << (4 * j)
    return v | fc << 16 | unit << 19 | load << 21 | store << 22 | lat << 23


def fields_of(info):
    return [
        (SHV[(info >> (4 * j + 2)) & 3], (info >> (4 * j)) & 3) for j in range(4) if (info >> (4 * j)) & 3
    ]


def decode(w):
    w &= 0xFFFFFFFF
    cond = w >> 28
    if cond == 15:
        return None
    for mask, value, info in PATTERNS:
        if w & mask == value:
            return info
    k = w >> 25 & 7
    if k == 0 and (w & 0x0FC000F0) == 0x00000090:
        f = [(16, K_D), (0, K_U), (8, K_U)]
        if w >> 21 & 1:
            f.append((12, K_U))
        return pack(f, fc=2, unit=2, lat=4)
    if k == 0 and (w & 0x90) == 0x90 and w & 0x60:
        load = w >> 20 & 1
        wb = w >> 21 & 1 or not w >> 24 & 1
        f = [(12, K_D if load else K_U), (16, K_UD if wb else K_U)]
        if not w >> 22 & 1:
            f.append((0, K_U))
        return pack(f, unit=1, load=load, store=1 - load, lat=2 if load else 1)
    if k in (2, 3) and not (k == 3 and w & 0x10):
        load = w >> 20 & 1
        wb = w >> 21 & 1 or not w >> 24 & 1
        f = [(12, K_D if load else K_U), (16, K_UD if wb else K_U)]
        if k == 3:
            f.append((0, K_U))
        return pack(f, unit=1, load=load, store=1 - load, lat=2 if load else 1)
    if k == 0 and w >> 21 & 15 == 6 and not w & 0xFF0 and (w >> 16 & 15) == (w >> 12 & 15) == (w & 15):
        return pack([(12, K_D), (16, K_D), (0, K_D)], fc=1)
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
            f.append((16, K_U))
        if opc not in (8, 9, 10, 11):
            f.append((12, K_D))
        if k == 0:
            f.append((0, K_U))
            if w & 0x10:
                f.append((8, K_U))
        return pack(f, fc=1)
    return None


COND_READS = [2, 2, 4, 4, 1, 1, 8, 8, 6, 6, 9, 9, 11, 11, 0, 0]


def flags(w, info):
    w &= 0xFFFFFFFF
    cond = w >> 28
    fc = info >> 16 & 7
    r = COND_READS[cond]
    wr = 0
    may = 0
    if fc == 1:
        opc = w >> 21 & 15
        s = w >> 20 & 1
        k = w >> 25 & 7
        if opc in (5, 6, 7):
            r |= 4
        if k == 0 and (w & 0xE0) == 0x60 and not w & 0x10 and (w >> 7 & 31) == 0:
            r |= 4
        if s:
            wr = 15 if opc in (2, 3, 4, 5, 6, 7, 10, 11) else 3
            may = 0 if wr == 15 else 4
    elif fc == 2:
        if w >> 20 & 1:
            wr = 3
            may = 4
    elif fc == 3:
        wr = 16
    elif fc == 4:
        r |= 16
    elif fc == 5:
        r = 31
    elif fc == 6:
        m = w >> 16 & 15
        if w >> 22 & 1 or m & 3:
            return None
        if m & 8:
            wr |= 15
        if m & 4:
            wr |= 16
    wr |= may
    r |= may
    if cond < 14:
        r |= wr
    return r, wr


D0 = 0


def mem(w, info, d0):
    """resource bits read / written by a load / store"""
    if (info >> 19 & 3) != 1:
        return 0, 0
    w &= 0xFFFFFFFF
    load = info >> 21 & 1
    base = w >> 16 & 15
    bits = 1 << 9
    if base == 5:
        k = w >> 25 & 7
        if k == 0:
            off = ((w >> 4 & 0xF0) | (w & 15)) if w >> 22 & 1 else None
            imm = w >> 22 & 1
        elif k == 2:
            off = w & 0xFFF
            imm = 1
        else:
            off = None
            imm = 0
        pre = w >> 24 & 1 and not w >> 21 & 1
        if off is None or not pre:
            bits = 0x1FF  # register offset / post-index from r5: could be any of the cpu state
        else:
            o = off if w >> 23 & 1 else -off
            bits = 1 << ((o - d0) >> 2) if 0 <= o - d0 < 32 else 1 << 8
    if load:
        # guest loads stay in order with every other guest access (device registers)
        return (bits, bits) if bits == 1 << 9 else (bits, 0)
    return (0, bits)


# (mask, value, info): the fixed encodings, first match wins (the rest is code in blksched.s: mul, loads /
# stores, the sbc idiom, data processing)
# sxtb sxth uxtb uxth
# movw
# movt
# bfc
# bfi (Rm != 15; the encoding with Rm = 15 is bfc)
# ubfx sbfx
# clz
# rev rev16 revsh rbit
# uadd8
# sel
# mrs rd, apsr
# msr apsr_<fields>, rm
# nop
PATTERNS = [
    (0x0FAF03F0, 0x06AF0070, pack([(12, K_D), (0, K_U)])),
    (0x0FF00000, 0x03000000, pack([(12, K_D)])),
    (0x0FF00000, 0x03400000, pack([(12, K_UD)])),
    (0x0FE0007F, 0x07C0001F, pack([(12, K_UD)])),
    (0x0FE00070, 0x07C00010, pack([(12, K_UD), (0, K_U)])),
    (0x0FA00070, 0x07A00050, pack([(12, K_D), (0, K_U)])),
    (0x0FFF0FF0, 0x016F0F10, pack([(12, K_D), (0, K_U)])),
    (0x0FBF0F70, 0x06BF0F30, pack([(12, K_D), (0, K_U)])),
    (0x0FF00FF0, 0x06500F90, pack([(16, K_U), (12, K_D), (0, K_U)], fc=3)),
    (0x0FF00FF0, 0x06800FB0, pack([(16, K_U), (12, K_D), (0, K_U)], fc=4)),
    (0x0FFF0FFF, 0x010F0000, pack([(12, K_D)], fc=5, lat=2)),
    (0x0FF0FFF0, 0x0120F000, pack([(0, K_U)], fc=6)),
    (0x0FFFFFFF, 0x0320F000, pack([])),
]


def write_inc(path):
    o = (
        [
            "@ generated by tools/gen/schedtab.py --inc: the word-decoder tables of pjit/blksched.s (do not edit)",
            "\t.equ\tI_MUL3, 0x%08X" % pack([(16, K_D), (0, K_U), (8, K_U)], fc=2, unit=2, lat=4),
            "\t.equ\tI_MUL4, 0x%08X" % pack([(16, K_D), (0, K_U), (8, K_U), (12, K_U)], fc=2, unit=2, lat=4),
            "\t.equ\tI_IDIOM, 0x%08X" % pack([(12, K_D), (16, K_D), (0, K_D)], fc=1),
            "\t.equ\tI_DPBASE, 0x%08X" % pack([], fc=1),
            "\t.equ\tI_LDBASE, 0x%08X" % pack([], unit=1, load=1, lat=2),
            "\t.equ\tI_STBASE, 0x%08X" % pack([], unit=1, store=1, lat=1),
            "\t.balign 4",
            "s_pat:",
        ]
        + ["\t.word\t0x%08X, 0x%08X, 0x%08X" % p for p in PATTERNS]
        + [
            "\t.word\t0, 0, 0",
            "s_condtab:",
            "\t.byte\t" + ", ".join(str(c) for c in COND_READS),
            "\t.balign 4",
        ]
    )
    open(path, "w").write("\n".join(o) + "\n")


if __name__ == "__main__" and "--inc" in sys.argv:
    write_inc(sys.argv[sys.argv.index("--inc") + 1])
    sys.exit(0)
if __name__ == "__main__":
    import armdce, regalloc
    from gen import Gen, read_offsets
    import btables as bt

    offs = read_offsets()
    g = Gen(offs)
    arena, infos, index = [], [], {}
    words = set()
    for op in range(0, 65536):
        d = bt.ba.decode(bt.Mem(op), 0)
        if bt.KIND[d.kind] != bt.BT_PLAIN:
            continue
        seq = g.generate(op)
        if seq is None:
            continue
        vs = bt.variants(seq)
        if vs is None:
            continue
        for ws in vs[0]:
            for w in ws:
                if not isinstance(w, tuple):
                    words.add(int(w))
    # prefix words: movw / movt / ldr / ldrsh / add / mov (rd, #imm / pc)
    print(len(words), "distinct words")
    unknown = badfields = badflags = 0
    first = {}
    for w in sorted(words):
        info = decode(w)
        a = armdce.analyse(w)
        f = regalloc.fields(w)
        if info is None:
            if f is not None:
                print("decode None but regalloc knows", hex(w))
                badfields += 1
            else:
                unknown += 1
                first.setdefault(w >> 20, w)
            continue
        if f is None:
            print("decode knows, regalloc not", hex(w))
            badfields += 1
            continue
        if fields_of(info) != [(s, {"u": 1, "d": 2, "ud": 3}[kd]) for s, kd in f]:
            print("fields differ", hex(w), fields_of(info), f)
            badfields += 1
            continue
        if a.barrier or a.branch is not None:
            print("barrier but decoded", hex(w))
            badflags += 1
            continue
        fr = flags(w, info)
        if fr is None:
            print("flags unknown", hex(w))
            badflags += 1
            continue
        r, wr = fr
        er = a.freads | a.fmay
        ew = a.fkill | a.fmay
        if w >> 28 < 14:
            er |= ew
        if (r, wr) != (er, ew):
            print("flags differ %08x mine r%d w%d analyse r%d w%d" % (w, r, wr, er, ew))
            badflags += 1
    print(
        "%d words: %d unknown encodings (not schedulable), %d field mismatches, %d flag mismatches"
        % (len(words), unknown, badfields, badflags)
    )
    for k, w in list(first.items())[:20]:
        print("  unknown e.g. %08x" % w)
    sys.exit(1 if badfields or badflags else 0)
