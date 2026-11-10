#!/usr/bin/env python3
"""Packs the tables of a linked image into an LZTB bundle (tools/pack/lzt.py, decoder pjit/lzt.s + pjit/lzt.h):
each table symbol's bytes, at its link address, with the best of plain / 4 / 8 / 16 byte planes.
  tools/pack/lzt_elf.py <elf> <out.lzt> [--interp] [--rel] [--header out.h]
      --interp: leave out the block translator's tables; --rel: offsets from the lowest table instead of
      absolute addresses;
      --image: the whole linked image (every LOAD segment cut at the table symbols; writes <out>.entry too);
      --no-code: leave out .text; --header: also a C header with the bundle and a checksum per part
      (tests/bbb/bbb_lzt.c)
A symbol's size is the distance to the next symbol (rounded up to 256; the zero tail is harmless)."""

import sys, os, struct, subprocess

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import lzt

INTERP = ["pjit_ext_table", "pjit_arena", "pjit_drsr", "pjit_crsr"]
CHUNK = 1 << 20  # --image: the largest part (and so the scratch buffer the loader needs: CHUNK + 16)
BLOCKS = ["pjit_bt", "pjit_bn", "pjit_bfr", "pjit_bbody", "pjit_binfo", "pjit_bv"]


def main():
    args = sys.argv[1:]
    hdr = args[args.index("--header") + 1] if "--header" in args else None
    a = [x for i, x in enumerate(args) if not x.startswith("--") and not (i and args[i - 1] == "--header")]
    if len(a) != 2:
        sys.exit(__doc__)
    elf = open(a[0], "rb").read()
    phoff, phentsize, phnum = (
        struct.unpack_from("<I", elf, 28)[0],
        struct.unpack_from("<H", elf, 42)[0],
        struct.unpack_from("<H", elf, 44)[0],
    )
    segs = [struct.unpack_from("<IIIIIIII", elf, phoff + i * phentsize) for i in range(phnum)]
    segs = [(va, off, fsz) for t, off, va, pa, fsz, msz, fl, al in segs if t == 1 and fsz]

    def read(addr, n):
        for va, off, fsz in segs:
            if va <= addr and addr + n <= va + fsz:
                return elf[off + addr - va : off + addr - va + n]
        sys.exit("address %08X not in the file" % addr)

    nm = subprocess.run(["arm-none-eabi-nm", "-S", a[0]], capture_output=True, text=True).stdout.splitlines()
    addr, sized, allv = {}, {}, []
    for l in nm:
        f = l.split()
        if len(f) >= 3:
            v = int(f[0], 16)
            addr[f[-1]] = v
            allv.append(v)
            if len(f) == 4:
                sized[f[-1]] = int(f[1], 16)
    if (
        "--image" in sys.argv
    ):  # the whole linked image: every LOAD segment, cut at the table symbols so each gets its own stride
        cuts = sorted({addr[n] for n in INTERP + BLOCKS if n in addr})
        parts = []
        for va, off, fsz in segs:
            pts = [va] + [c for c in cuts if va < c < va + fsz] + [va + fsz]
            for lo, hi in zip(pts, pts[1:]):
                for c in range(lo, hi, CHUNK):
                    # chunks of at most CHUNK bytes (a multiple of 256): the unpack scratch stays small
                    parts.append((c, read(c, min(hi, c + CHUNK) - c)))
        entry = struct.unpack_from("<I", elf, 24)[0]
        parts.sort(key=lambda p: p[0])
        img = lzt.bundle([(v - (parts[0][0] if "--rel" in sys.argv else 0), b) for v, b in parts])
        open(a[1], "wb").write(img)
        open(a[1] + ".entry", "w").write("0x%08X\n" % entry)
        print(
            "image: %d bytes in %d parts -> %d bytes, entry 0x%08X"
            % (sum(len(p[1]) for p in parts), len(parts), len(img), entry)
        )
        return
    parts = []
    extra = []  # the code itself (.text): with everything in DRAM it is loaded from the same image
    if "--no-code" not in sys.argv:
        for l in subprocess.run(
            ["arm-none-eabi-readelf", "-S", "-W", a[0]], capture_output=True, text=True
        ).stdout.splitlines():
            f = l.replace("[", " ").replace("]", " ").split()
            if len(f) > 5 and f[1] == ".text":
                extra.append((".text", int(f[3], 16), int(f[5], 16)))
    for name, v, size in extra:
        parts.append((v, read(v, size)))
        print("%-16s %08X %9d" % (name, v, size))
    for name in sorted(
        INTERP + ([] if "--interp" in sys.argv else BLOCKS), key=lambda n: addr[n]
    ):  # ascending: a part's zero padding only spills into parts loaded after it
        v = addr[name]
        end = max(va + fsz for va, off, fsz in segs if va <= v < va + fsz)
        size = sized.get(name) or min(x for x in allv if x > v) - v
        size = min(size, end - v)
        parts.append((v, read(v, size)))
        print("%-16s %08X %9d" % (name, v, size))
    parts.sort(key=lambda p: p[0])
    base = parts[0][0] if "--rel" in sys.argv else 0
    img = lzt.bundle([(v - base, b) for v, b in parts])
    if hdr:

        def wsum(b):
            b = b + bytes(-len(b) % 4)
            return (
                sum(int.from_bytes(b[i : i + 4], "little") * (i // 4 + 1) for i in range(0, len(b), 4))
                & 0xFFFFFFFF
            )

        open(hdr, "w").write(
            "// generated by tools/pack/lzt_elf.py: do not edit\nstatic const unsigned char real_bundle[] __attribute__((aligned(16))) = {%s};\n"
            "#define REAL_REGION %d\nstatic const struct { unsigned off, size, sum; } real_parts[] = {%s};\n#define REAL_NPARTS %d\n"
            % (
                ",".join(map(str, img)),
                parts[-1][0] - parts[0][0] + (len(parts[-1][1]) + 255) // 256 * 256,
                ",".join("{%d,%d,0x%08X}" % (v - parts[0][0], len(b), wsum(b)) for v, b in parts),
                len(parts),
            )
        )

    open(a[1], "wb").write(img)
    print("%d bytes raw -> %d bytes" % (sum(len(p[1]) for p in parts), len(img)))


main()
