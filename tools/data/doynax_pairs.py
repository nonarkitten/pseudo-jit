#!/usr/bin/env python3
"""Builds tests/data/doynax_data.h for tests/qemu/smoke_doynax.c: the Doynax (Doynamite68k) depacker as 68000 machine code (assembled with m68k-elf-as from
the source you give with --depack) and a set of (packed, original) pairs. Pairs: every X.lz with an X.dat next to it under the
data dir, plus the other .dat files packed with the demo's own packer
(its lz.c, compiled to --lz), plus synthetic inputs. Usage: doynax_pairs.py --data
<Planet-Rocklobster demo/data> --lz <compiled lz> --depack <doynaxdepack.s> [--max-total KB]
The depacker is Johan Forslof's and the data is the demo's: neither has a licence we can pass on, so neither is in the repository and the generated header
is not committed. --depack takes GNU as (68000) source that defines `doynaxdepack` (A0 = packed data, A1 = destination).
"""

import argparse, os, subprocess, tempfile


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--data", required=True)
    ap.add_argument("--lz", required=True)
    ap.add_argument(
        "--depack",
        required=True,
        help="68000 source (GNU as) of the Doynamite68k depacker: defines doynaxdepack",
    )
    ap.add_argument("--max-total", type=int, default=700)
    ap.add_argument("--max-one", type=int, default=64)
    ap.add_argument(
        "--out", default=os.path.join(os.path.dirname(__file__), "..", "..", "tests", "data", "doynax_data.h")
    )
    a = ap.parse_args()
    tmp = tempfile.mkdtemp()
    open(tmp + "/d.s", "w").write(open(a.depack).read())
    subprocess.check_call(
        ["m68k-elf-as", "-m68000", "--register-prefix-optional", "-o", tmp + "/d.o", tmp + "/d.s"]
    )
    subprocess.check_call(["m68k-elf-objcopy", "-O", "binary", "-j", ".text", tmp + "/d.o", tmp + "/d.bin"])
    code = open(tmp + "/d.bin", "rb").read()
    pairs = []  # (name, packed, orig)
    seen = set()
    for root, _, files in os.walk(a.data):
        for f in sorted(files):
            p = os.path.join(root, f)
            base, ext = os.path.splitext(p)
            if ext == ".lz" and os.path.exists(base + ".dat"):
                pairs.append(
                    (
                        os.path.relpath(base, a.data) + "(repo)",
                        open(p, "rb").read(),
                        open(base + ".dat", "rb").read(),
                    )
                )
                seen.add(base + ".dat")
    for root, _, files in os.walk(a.data):
        for f in sorted(files):
            p = os.path.join(root, f)
            if f.endswith((".dat", ".spr", ".p61")) and p not in seen and 1024 <= os.path.getsize(p):
                o = tmp + "/x.lz"
                if subprocess.run([a.lz, "-o", o, p], capture_output=True).returncode == 0 and os.path.exists(
                    o
                ):
                    pairs.append((os.path.relpath(p, a.data), open(o, "rb").read(), open(p, "rb").read()))
    for n, _, d in list(
        pairs
    ):  # the repo's .lz files are not always a pack of the .dat next to them: also pack each .dat afresh
        pth = tmp + "/f.bin"
        open(pth, "wb").write(d)
        o = tmp + "/f.lz"
        if subprocess.run([a.lz, "-o", o, pth], capture_output=True).returncode == 0:
            pairs.append((n.replace("(repo)", "(fresh)"), open(o, "rb").read(), d))
    plot = (b"\x01\xe8\x00\x01") * 704 + b"\x4e\x75"  # the Vectorplots plot routine's image
    synth = [
        ("synth:plotcode", plot),
        ("synth:zeros", bytes(5000)),
        ("synth:ramp", bytes(i & 255 for i in range(20000))),
        ("synth:words", b"".join((i * 7 & 0xFFFF).to_bytes(2, "big") for i in range(10000))),
    ]
    import random

    rnd = random.Random(5)
    synth.append(("synth:random", bytes(rnd.randrange(256) for _ in range(8000))))
    synth.append(
        (
            "synth:mixed",
            b"".join(
                (
                    bytes(rnd.randrange(256) for _ in range(rnd.randrange(1, 40)))
                    if rnd.random() < 0.5
                    else bytes([rnd.randrange(4)]) * rnd.randrange(2, 300)
                )
                for _ in range(300)
            ),
        )
    )
    for k in range(
        120
    ):  # match-heavy random streams: every offset class, literal runs past 255 bytes, long matches
        out = bytearray()
        size = rnd.randrange(300, 30000)
        while len(out) < size:
            r = rnd.random()
            if r < 0.35 or len(out) < 4:
                out += bytes(
                    rnd.randrange(256) for _ in range(rnd.choice((1, 2, 3, 7, 20, 100, 260, 300, 700)))
                )
            elif r < 0.45:
                out += bytes([rnd.randrange(256)]) * rnd.randrange(2, 400)
            else:
                off = rnd.choice((1, 2, 3, 4, 8, 9, 17, 70, 130, 300, 1000, 1500, 5000, 8000))
                off = min(off, len(out))
                for _ in range(rnd.choice((2, 3, 4, 10, 40, 255, 256, 257, 400))):
                    out.append(out[-off])
        synth.append(("synth:rnd%03d" % k, bytes(out[:size])))
    for n, d in synth:
        p = tmp + "/s.bin"
        open(p, "wb").write(d)
        o = tmp + "/s.lz"
        if subprocess.run([a.lz, "-o", o, p], capture_output=True).returncode == 0:
            pairs.append((n, open(o, "rb").read(), d))
    pairs.sort(key=lambda x: (not x[0].startswith("synth"), x[0]))
    pairs = [x for x in pairs if len(x[2]) <= a.max_one * 1024 and len(x[1]) <= a.max_one * 1024]
    tot = 0
    sel = []
    for x in pairs:
        if tot + len(x[2]) > a.max_total * 1024:
            continue
        sel.append(x)
        tot += len(x[2])

    def arr(b):
        return ",".join(str(c) for c in b)

    with open(a.out, "w") as h:
        h.write("/* generated by tools/data/doynax_pairs.py; not committed */\n#include <stdint.h>\n")
        h.write("static const uint8_t doy_code[] = {%s};\n" % arr(code))
        for i, (n, p, o) in enumerate(sel):
            h.write(
                "static const uint8_t doy_p%d[] = {%s};\nstatic const uint8_t doy_o%d[] = {%s};\n"
                % (i, arr(p), i, arr(o))
            )
        h.write(
            "static const struct { const char *name; const uint8_t *packed, *orig; uint32_t plen, olen; } doy_pairs[] = {\n"
        )
        for i, (n, p, o) in enumerate(sel):
            h.write('  {"%s", doy_p%d, doy_o%d, %d, %d},\n' % (n, i, i, len(p), len(o)))
        h.write("};\n#define DOY_N %d\n" % len(sel))
    print("depacker %d bytes, %d pairs, %d KB originals" % (len(code), len(sel), tot // 1024))
    for n, p, o in sel:
        print("  %-40s packed %6d orig %6d" % (n, len(p), len(o)))


main()
