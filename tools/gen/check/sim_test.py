#!/usr/bin/env python3
"""The generator's sequences, executed under Unicorn on random and edge-case states -- registers, a memory
area, extension words -- and compared with the 68000 oracle (tools/gen/check/oracle1.c: golden/mcl68k.c, one
instruction per case).
For each opcode of a family this file knows, it builds a state in which every addressing mode lands inside the
memory area
(registers, displacements, indexes, absolute and PC-relative addresses all chosen so), writes the extension
words the
oracle will read, and computes what the fill's prefix would leave in r1 (source) / r2 (destination) for the
sequence.

Needs build/oracle1 and build/helpers.bin (`make build/oracle1 build/helpers.bin`).   .venv/bin/python tools/gen/check/sim_test.py [states per opcode]
"""

import sys, os, random, subprocess, collections, re, struct

sys.path.insert(0, os.path.dirname(__file__))
sys.path[:0] = [os.path.join(os.path.dirname(os.path.abspath(__file__)), p) for p in ("..", "../..")]
from paths import build_path
from gen import Gen, read_offsets
from sim import Sim
import model68k

AREA, AREA_LEN = 0x4000, 0x1000
OP_ADDR = 0x1000 + 2 * (8 * 3 + 8 * 3 + 2)  # where the oracle puts the opcode (after its register preamble)
EDGE = [
    0,
    1,
    0x7F,
    0x80,
    0xFF,
    0x7FFF,
    0x8000,
    0xFFFF,
    0x7FFFFFFF,
    0x80000000,
    0xFFFFFFFF,
    0x12345678,
    0xFFFFFF80,
    0xFFFF8000,
]


def sext(v, bits):
    v &= (1 << bits) - 1
    return v - (1 << bits) if v >> (bits - 1) else v


class Case:
    def __init__(self, rng):
        self.rng = rng
        self.d = [self.val() for _ in range(8)]
        self.a = [self.val() for _ in range(7)] + [0x00080000]
        self.ccr = rng.getrandbits(5)
        self.area = bytes(rng.getrandbits(8) for _ in range(AREA_LEN))
        self.ext = []
        self.r = {"src": 0xDEADBEEF, "dst": 0xDEADBEEF}
        self.a_set = {}
        self.d_set = {}

    def val(self):
        return self.rng.choice(EDGE) if self.rng.random() < 0.6 else self.rng.getrandbits(32)

    def even(self, lo, hi):
        return self.rng.randrange(lo, hi, 2)

    def base(self, reg):  # an address register that points into the middle of the area
        if reg not in self.a_set:
            self.a_set[reg] = AREA + self.even(0x400, 0xC00)
            self.a[reg] = self.a_set[reg]
        return self.a[reg]

    def index_reg(self):
        free = [r for r in range(8) if r not in self.d_set]
        r = self.rng.choice(free)
        return r

    def add(self, role, mode, reg, width):
        """Build the EA: set registers, append extension words, record what the prefix leaves in r1 / r2."""
        ext_addr = OP_ADDR + 2 + 2 * len(self.ext)
        if mode in ("dn", "an"):
            return
        if mode == "stk":
            self.base(7)
            return  # the stack pointer into the area
        if mode == "disp":  # a bare displacement word (LINK)
            d = self.even(-0x40, 0x40)
            self.ext.append(d & 0xFFFF)
            self.r[role] = d & 0xFFFFFFFF
            return
        if mode == "mask":
            m = self.rng.getrandbits(16)
            self.ext.append(m)
            self.r[role] = m
            return  # a MOVEM register list
        if mode in ("(an)", "(an)+", "-(an)"):
            self.base(reg)
            return
        if mode == "d16":
            self.base(reg)
            d = self.even(-0x30, 0x30)
            self.ext.append(d & 0xFFFF)
            self.r[role] = d & 0xFFFFFFFF
            self.r[role] = sext(d, 16) & 0xFFFFFFFF
        elif mode == "idx":
            self.base(reg)
            x = self.index_reg()
            lng = self.rng.random() < 0.5
            d8 = self.even(-0x20, 0x20)
            v = self.even(-0x20, 0x20) & 0xFFFFFFFF
            if not lng:
                # garbage above the low word, which .W ignores
                v = (self.rng.getrandbits(16) << 16) | (v & 0xFFFF)
            self.d_set[x] = v
            self.d[x] = v
            self.ext.append(x << 12 | lng << 11 | d8 & 0xFF)
            self.r[role] = (sext(d8, 8) + (v if lng else sext(v, 16))) & 0xFFFFFFFF
        elif mode in ("absw", "absl"):
            t = AREA + self.even(0x400, 0xC00)
            self.ext += [t & 0xFFFF] if mode == "absw" else [t >> 16, t & 0xFFFF]
            self.r[role] = t
        elif mode == "pcd16":
            t = AREA + self.even(0x400, 0xC00)
            self.ext.append((t - ext_addr) & 0xFFFF)
            self.r[role] = t
        elif mode == "pcidx":
            t = AREA + self.even(0x400, 0xC00)
            x = self.index_reg()
            d8 = self.even(-0x20, 0x20)
            v = (t - ext_addr - d8) & 0xFFFFFFFF
            self.d_set[x] = v
            self.d[x] = v
            self.ext.append(x << 12 | 1 << 11 | d8 & 0xFF)
            self.r[role] = t
        elif mode == "imm":
            v = self.val()
            if width == 1:
                self.ext.append(v & 0xFF)
                self.r[role] = v & 0xFF
            elif width == 2:
                self.ext.append(v & 0xFFFF)
                self.r[role] = sext(v, 16) & 0xFFFFFFFF
            else:
                self.ext += [v >> 16, v & 0xFFFF]
                self.r[role] = v


def ea_name(mode, reg):
    if mode < 7:
        return ["dn", "an", "(an)", "(an)+", "-(an)", "d16", "idx"][mode]
    return {0: "absw", 1: "absl", 2: "pcd16", 3: "pcidx", 4: "imm"}.get(reg)


def roles(name, op):
    """-> [(role, mode name, register, width)] in extension-word order, or None for emitters this test does
    not know.
    """
    m = re.match(r"emit_(ADD|SUB|AND|OR|EOR|CMP)(BW|W|L|B)$", name or "")
    if m:
        kind = m.group(1).lower()
        size = (op >> 6) & 3
        w = 1 << size
        if kind in ("and", "or") and (op >> 3) & 7 == 1:
            return None  # AND/OR An,Dn is not a 68000 instruction (the opcode table has a row for it)
        if kind == "cmp" or (kind != "eor" and not op & 0x100):
            return [("src", ea_name((op >> 3) & 7, op & 7), op & 7, w)]
        return [("dst", ea_name((op >> 3) & 7, op & 7), op & 7, w)]
    m = re.match(r"emit_(ADDA|SUBA|CMPA)(W|L)$", name or "")
    if m:
        return [("src", ea_name((op >> 3) & 7, op & 7), op & 7, 2 if not op & 0x100 else 4)]
    m = re.match(r"emit_(ADDI|SUBI|ANDI|ORI|EORI|CMPI)(BW|L)$", name or "")
    if m:
        w = 1 << ((op >> 6) & 3)
        return [("src", "imm", 4, w), ("dst", ea_name((op >> 3) & 7, op & 7), op & 7, w)]
    m = re.match(r"emit_MOVE(B|W|L)$", name or "")
    if m:
        w = {"B": 1, "W": 2, "L": 4}[m.group(1)]
        return [
            ("src", ea_name((op >> 3) & 7, op & 7), op & 7, w),
            ("dst", ea_name((op >> 6) & 7, (op >> 9) & 7), (op >> 9) & 7, w),
        ]
    m = re.match(r"emit_MOVEA(W|L)$", name or "")
    if m:
        return [("src", ea_name((op >> 3) & 7, op & 7), op & 7, 2 if m.group(1) == "W" else 4)]
    if name == "emit_MOVEQ":
        return []
    if name == "emit_MOVE_SR_TO":
        return [("dst", ea_name((op >> 3) & 7, op & 7), op & 7, 2)]
    if name == "emit_MOVE_TO_SR":
        return [("src", ea_name((op >> 3) & 7, op & 7), op & 7, 2)]
    if name in ("emit_ANDI_TO_SR", "emit_ORI_TO_SR", "emit_EORI_TO_SR"):
        return [("src", "imm", 4, 2)]
    if name in ("emit_MOVE_TO_USP", "emit_MOVE_USP_TO"):
        return []
    if name == "emit_CHK":
        return [("src", ea_name((op >> 3) & 7, op & 7), op & 7, 2)]
    if name in ("emit_ABCD", "emit_SBCD"):
        return [("src", "-(an)", op & 7, 1), ("dst", "-(an)", (op >> 9) & 7, 1)] if op & 8 else []
    if name == "emit_NBCD":
        return [("dst", ea_name((op >> 3) & 7, op & 7), op & 7, 1)]
    if name in ("emit_DIVU", "emit_DIVS"):
        return [("src", ea_name((op >> 3) & 7, op & 7), op & 7, 2)]
    if name == "emit_TAS":
        return [("dst", ea_name((op >> 3) & 7, op & 7), op & 7, 1)]
    if name == "emit_PEA":
        return [("stk", "stk", 7, 4), ("src", ea_name((op >> 3) & 7, op & 7), op & 7, 4)]
    if name == "emit_LINK":
        return [("stk", "stk", 7, 4), ("src", "disp", 0, 2)]
    if name == "emit_UNLK":
        return [("src", "(an)", op & 7, 4)]
    if name == "emit_MOVEP":
        return [("src", "d16", op & 7, 2)]
    if name in ("emit_ANDI_TO_CCR", "emit_ORI_TO_CCR", "emit_EORI_TO_CCR"):
        return [("src", "imm", 4, 1)]
    if name == "emit_MOVE_TO_CCR":
        return [("src", ea_name((op >> 3) & 7, op & 7), op & 7, 2)]
    if name == "emit_MOVEM":
        mode = (op >> 3) & 7
        a = op & 7
        long_ = bool(op & 0x40)
        to_reg = bool(op & 0x400)
        return [("src", "mask", 0, 2), ("dst", ea_name(mode, a), a, 4 if long_ else 2)]
    m = re.match(r"emit_(ADDX|SUBX)(BW|L)$", name or "")
    if m:
        if not op & 8:
            return []
        w = 1 << ((op >> 6) & 3)
        return [("src", "-(an)", op & 7, w), ("dst", "-(an)", (op >> 9) & 7, w)]
    m = re.match(r"emit_NEGX(BW|L)$", name or "")
    if m:
        return [("dst", ea_name((op >> 3) & 7, op & 7), op & 7, 1 << ((op >> 6) & 3))]
    if name in ("emit_BCHG", "emit_BCLR", "emit_BSET", "emit_BTST"):
        mode = (op >> 3) & 7
        dst = ("dst", ea_name(mode, op & 7), op & 7, 4 if mode == 0 else 1)
        return [dst] if op & 0x100 else [("src", "imm", 4, 1), dst]
    if name == "emit_Scc":
        return [("dst", ea_name((op >> 3) & 7, op & 7), op & 7, 1)]
    m = re.match(r"emit_(ADDQ|SUBQ)(BW|L|AW)$", name or "")
    if m:
        w = 4 if m.group(2) == "AW" else 1 << ((op >> 6) & 3)
        return [("dst", ea_name((op >> 3) & 7, op & 7), op & 7, w)]
    m = re.match(r"emit_(CLR|NEG|NOT|TST)(BW|L)$", name or "")
    if m:
        return [("dst", ea_name((op >> 3) & 7, op & 7), op & 7, 1 << ((op >> 6) & 3))]
    if name in ("emit_EXTW", "emit_EXTL", "emit_SWAP", "emit_EXG"):
        return []
    if name == "emit_LEA":
        return [("src", ea_name((op >> 3) & 7, op & 7), op & 7, 4)]
    if name in ("emit_MULS", "emit_MULU"):
        return [("src", ea_name((op >> 3) & 7, op & 7), op & 7, 2)]
    m = re.match(r"emit_CMPM(B|W|L)$", name or "")
    if m:
        w = {"B": 1, "W": 2, "L": 4}[m.group(1)]
        return [("src", "(an)+", op & 7, w), ("dst", "(an)+", (op >> 9) & 7, w)]
    m = re.match(r"emit_(AS|LS|ROX|RO)(dBW|dL|d)$", name or "")
    if m:
        if m.group(2) != "d":
            return []  # register shifts: no EA
        return [("dst", ea_name((op >> 3) & 7, op & 7), op & 7, 2)]  # memory shifts: a word, by one
    return None


def name_bcd(op):
    return op & 0xF1F0 in (0xC100, 0x8100) or op & 0xFFC0 == 0x4800


def fix_bcd(c, op, rl):
    """ABCD/SBCD/NBCD operands with valid digits (the behaviour on A-F digits is a separate question)."""
    bcd = lambda: c.rng.randrange(10) << 4 | c.rng.randrange(10)
    m = model68k.Model(c.d, c.a, c.ccr, c.area, AREA, c.ext, OP_ADDR)
    area = bytearray(c.area)

    def put(e, v):
        if e[0] == "dn":
            c.d[e[1]] = (c.d[e[1]] & ~0xFF) | v
        elif e[0] == "mem":
            area[e[1] - AREA] = v

    if op & 0xF1F0 in (0xC100, 0x8100):
        if op & 8:
            se = m.ea("-(an)", op & 7, 1)
            de = m.ea("-(an)", (op >> 9) & 7, 1)
        else:
            se = ("dn", op & 7)
            de = ("dn", (op >> 9) & 7)
        put(se, bcd())
        put(de, bcd())
    else:
        put(m.ea(rl[0][1], op & 7, 1), bcd())
    c.area = bytes(area)


def make_case(rng, op, rl):
    for _ in range(40):
        c = Case(rng)
        for role, mode, reg, w in rl:
            c.add(role, mode, reg, w)
        c.op = op
        if name_bcd(op):
            fix_bcd(c, op, rl)
        if (
            op & 0xF1C0 == 0x4180
        ):  # CHK: only the no-trap cases go to the oracle (its trap path runs off into the vector table)
            m = model68k.Model(c.d, c.a, c.ccr, c.area, AREA, c.ext, OP_ADDR)
            bound = sext(m.read(m.ea(rl[0][1], op & 7, 2), 2), 16)
            if bound < 0:
                continue
            n = (op >> 9) & 7
            if n in c.d_set:
                continue  # Dn is also an index register: changing it would move the operand
            c.d[n] = (c.d[n] & ~0xFFFF) | rng.randrange(0, bound + 1)
        if (op & 0xF0C0) == 0x80C0:  # DIVU / DIVS: a zero divisor traps, which the harness cannot follow
            r = model68k.run_case(c, "emit_DIVU", op, rl, AREA, OP_ADDR)
            if r == "trap":
                continue
        return c
    raise SystemExit(f"could not build a usable case for {op:04X}")


def line(c):
    return (
        "%x %x " % (c.op, len(c.ext))
        + " ".join("%x" % e for e in c.ext)
        + " "
        + " ".join("%x" % x for x in c.d)
        + " "
        + " ".join("%x" % x for x in c.a)
        + " %x " % c.ccr
        + c.area.hex()
    )


NATURAL = False
_nat = {}


def natural_changed(g, op):
    """does the natural-polarity variant (Gen.natural) of this opcode differ from the ordinary sequence?"""
    if op not in _nat:
        g.natural = False
        a = [int(w) if not isinstance(w, tuple) else w for w in g.generate(op)]
        g.natural = True
        b = [int(w) if not isinstance(w, tuple) else w for w in g.generate(op)]
        _nat[op] = a != b
    return _nat[op]


# CCR bits the 68000 leaves undefined (N and V for BCD): not compared with the model
UNDEFINED = {
    "emit_CHK": 0xF,
    "emit_ABCD": 0xA,
    "emit_SBCD": 0xA,
    "emit_NBCD": 0xA,
}


def same(ref, t, name, usp):
    """Does the model's result `ref` match `t` = (d, a, full SR, area) and the user stack pointer `usp`?
    Undefined CCR bits are not compared.
    """
    if ref is None or ref == "trap":
        return False
    m = ~UNDEFINED.get(name, 0)
    return (
        list(ref[0]) == list(t[0])
        and list(ref[1]) == list(t[1])
        and (ref.sr_hi | ref[2]) & m == t[2] & m
        and ref[3] == t[3]
        and ref.usp == usp
    )


def name_is_bit(op):
    return (op & 0xF1C0) in (0x0140, 0x0180, 0x01C0, 0x0100) or (op & 0xFFC0) in (
        0x0840,
        0x0880,
        0x08C0,
        0x0800,
    )


def oracle_limit(op, who, c):
    """The few places where the oracle (golden/mcl68k.c) itself is the odd one out, not the generator: a mismatch
    that falls in one is
    reported but does not fail the run. Everything else is a failure."""
    if who.startswith("oracle is wrong"):
        if op & 0xF1FF == 0x013C:
            return "oracle limit: BTST Dn,#imm (the oracle takes the immediate as a 32-bit operand, modulo 32; the manual says a byte, modulo 8)"
        if op == 0x4E57:
            return "oracle limit: LINK A7 (the oracle pushes the undecremented SP; the 68000 pushes the decremented one)"
        if (op >> 12) in (2, 3) and (op >> 3) & 7 == 1:
            return "oracle limit: MOVE An,(An)+ / -(An) stored value (the generator and the model store the original An)"
    if name_is_bit(op) and who.startswith("code and oracle agree"):
        n = c.d[(op >> 9) & 7] if op & 0x100 else c.ext[0] & 0xFF
        if n >= (32 if (op >> 3) & 7 == 0 else 8):
            return (
                "oracle limit: BTST Dn,#imm bit number out of range (the oracle shares the code's behaviour)"
            )
    return who


def run_sharded(jobs):
    """JOBS=n: n copies of this script, each with SHARD=i/n (the cases whose opcode % n == i), run at once;
    their results are added up.
    """
    procs = [
        subprocess.Popen(
            [sys.executable] + sys.argv,
            env=dict(os.environ, SHARD="%d/%d" % (i, jobs), JOBS="1"),
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
        )
        for i in range(jobs)
    ]
    outs = [p.communicate()[0] for p in procs]
    ops = match = total = 0
    lines = collections.OrderedDict()
    for o in outs:
        for l in o.splitlines():
            m = re.match(r"(\d+) opcodes x (\d+) states: (\d+)/(\d+) runs match", l)
            if m:
                ops += int(m.group(1))
                match += int(m.group(3))
                total += int(m.group(4))
                per = m.group(2)
            else:
                lines[l] = lines.get(l, 0) + 1
    print(f"{ops} opcodes x {per} states: {match}/{total} runs match the oracle")
    for l in lines:
        print(l)
    sys.exit(1 if any(p.returncode for p in procs) else 0)


if __name__ == "__main__":
    if int(os.environ.get("JOBS", "1")) > 1 and "SHARD" not in os.environ:
        run_sharded(int(os.environ["JOBS"]))
    per_op = int(sys.argv[1]) if len(sys.argv) > 1 else 4
    offs = read_offsets()
    g = Gen(offs)
    sim = Sim(offs)
    # NATURAL=1: the block translator's natural-polarity compare variants (tools/gen/gen.py Gen.natural), C
    # inverted
    NATURAL = bool(os.environ.get("NATURAL"))
    g.natural = NATURAL
    rng = random.Random(68000)
    ops = []
    cases = []
    for op in range(65536):
        name = g.winner[op]
        rl = roles(name, op)
        if rl is None or g.generate(op) is None:
            continue
        if os.environ.get("ONLY") and not re.search(os.environ["ONLY"], name or ""):
            continue
        ops.append(op)
        for _ in range(per_op):
            cases.append(make_case(rng, op, rl))
    if os.environ.get(
        "SHARD"
    ):  # SHARD=i/n: only the opcodes with op % n == i (JOBS=n runs all n shards at once)
        si, sn = map(int, os.environ["SHARD"].split("/"))
        cases = [c for c in cases if c.op % sn == si]
        ops = [o for o in ops if o % sn == si]

    def oracle(batch):
        r = (
            subprocess.run(
                [build_path("host", "oracle1")],
                input="\n".join(line(c) for c in batch) + "\n",
                capture_output=True,
                text=True,
            )
            .stdout.strip()
            .splitlines()
        )
        if len(r) == len(batch):
            return r
        if len(batch) == 1:
            return [None]  # the oracle itself faulted on this one (an illegal combination)
        h = len(batch) // 2
        return oracle(batch[:h]) + oracle(batch[h:])

    out = []
    for i in range(0, len(cases), 4000):
        out += oracle(cases[i : i + 4000])
    skipped = collections.Counter(g.winner[c.op] for c, ln in zip(cases, out) if ln is None)
    if skipped:
        print("oracle could not run:", dict(skipped))
    bad = collections.Counter()
    first = {}
    for c, ln in zip(cases, out):
        if ln is None:
            continue
        name = g.winner[c.op]
        rl = roles(name, c.op)
        f = ln.split()
        want_d = [int(x, 16) for x in f[1:9]]
        want_a = [int(x, 16) for x in f[9:17]]
        want_ccr = int(f[17], 16)
        want_usp = int(f[18], 16)
        want_area = bytes.fromhex(f[19])
        try:
            seq = g.generate(c.op)
            gd, ga, gc, garea = sim.run(seq, c.d, c.a, c.ccr, area=c.area, r1=c.r["src"], r2=c.r["dst"])
            if NATURAL and natural_changed(g, c.op):
                gc ^= 1  # the natural-polarity compare leaves C inverted (no borrow): compare it as the 68K C
        except Exception as e:
            sys.exit(
                f"{name} op={c.op:04X} ext={c.ext} r={c.r} a={[hex(x) for x in c.a]} d={[hex(x) for x in c.d]} ccr={c.ccr:x}: {e}"
            )
        gusp = sim.last_usp
        mk = ~UNDEFINED.get(name, 0)
        ref = model68k.run_case(c, name, c.op, rl, AREA, OP_ADDR)
        code = (gd, ga, gc, garea)
        orc = (want_d, want_a, want_ccr, want_area)
        code_ok_oracle = (
            gd == want_d
            and ga == want_a
            and gc & mk == want_ccr & mk
            and garea == want_area
            and gusp == want_usp
        )
        if code_ok_oracle:
            # code and oracle agree: still ask the model, which can catch a bug they share
            if ref is not None and ref != "trap" and not same(ref, code, name, gusp):
                k = (
                    name,
                    oracle_limit(
                        c.op, "code and oracle agree, the MODEL differs (a shared bug or a model bug)", c
                    ),
                )
                bad[k] += 1
                first.setdefault(k, (c, gd, list(ref[0]), ga, list(ref[1]), gc, ref[2], garea == ref[3]))
            continue
        # the oracle and the code disagree: the independent model says who is right
        if ref is None:
            who = "unmodelled"
        elif same(ref, code, name, gusp):
            who = "oracle is wrong (the model agrees with the code)"
        elif same(ref, orc, name, want_usp):
            who = "the CODE is wrong (the model agrees with the oracle)"
        else:
            who = "all three differ"
        k = (name, oracle_limit(c.op, who, c))
        bad[k] += 1
        first.setdefault(k, (c, gd, want_d, ga, want_a, gc, want_ccr, garea == want_area))
    tot = len(cases) - sum(skipped.values())
    if skipped:
        print(
            f"  the oracle faulted on {sum(skipped.values())} cases (illegal combinations), skipped: {dict(skipped)}"
        )
    print(f"{len(ops)} opcodes x {per_op} states: {tot - sum(bad.values())}/{tot} runs match the oracle")
    for k, n in sorted(bad.items()):
        c, gd, wd, ga, wa, gc, wc, same_area = first[k]
        print(
            f"  {k[0]} [{k[1]}]: {n} mismatches, e.g. {c.op:04X} ext={[hex(e) for e in c.ext]} ccr_in={c.ccr:02x} d_in={[hex(x) for x in c.d]} got_ccr={gc:02x} want={wc:02x}"
            + ("" if gd == wd else f" D: {[hex(x) for x in gd]} vs {[hex(x) for x in wd]}")
            + ("" if ga == wa else f" A: {[hex(x) for x in ga]} vs {[hex(x) for x in wa]}")
            + ("" if same_area else " area differs")
        )
    sys.exit(1 if any(not k[1].startswith("oracle limit") for k in bad) else 0)
