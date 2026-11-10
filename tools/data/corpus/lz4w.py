#!/usr/bin/env python3
"""Builds lz4w decompression programs for the corpus (tools/data/corpus.py extra images): the decompressor of
github.com/nonarkitten/GnGeo (lz4w/lz4w_unpack.s, GNU syntax, from the
SGDK / GnGeo lineage), converted to Motorola syntax for vasm, a driver that unpacks a compressed block (made
by tools/data/corpus/lz4w_pack.c from a chosen input) and the destination
buffer, assembled at $1000.

  tools/data/corpus/lz4w.py <lz4w_unpack.s> <input> <name> <bytes> <out.bin>
"""

import sys, os, re, subprocess, tempfile, shutil


def convert(src):
    out = []
    for l in src.splitlines():
        l = re.sub(r"\|.*$", lambda m: ";" + m.group(0)[1:], l)  # comments
        l = re.sub(r"%(a[0-7]|d[0-7]|sp|pc)\b", r"\1", l)  # register names
        if re.match(r"\s*\.(globl|global)", l):
            continue
        l = re.sub(r"^(\s*)\.align\s+\d+", r"\1even", l)
        l = re.sub(r"^(\s*)\.long\b", r"\1dc.l", l)
        l = re.sub(r"^(\s*)\.word\b", r"\1dc.w", l)
        l = re.sub(r"^(\s*)\.byte\b", r"\1dc.b", l)
        m = re.match(r"(\s*move\.l\s+)\((\.jump_table-\.jump_base)\)-2\(pc,d0\.w\),(a4)(.*)$", l)
        if m:
            # GNU as takes the d8 of d8(pc,xn) as written (jump_table - jump_base - 2: the label is the
            # instruction, the extension word two bytes on); vasm works the displacement out
            l = "%s(.jump_table,pc,d0.w),%s%s" % (
                m.group(1),
                m.group(3),
                m.group(4),
            )
        out.append(l)
    return "\n".join(out)


def main():
    unpack, inp, name, nbytes, outbin = sys.argv[1:6]
    nbytes = int(nbytes)
    vasm = os.environ.get("VASM") or shutil.which("vasmm68k_mot")
    here = os.path.dirname(os.path.abspath(__file__))
    wd = tempfile.mkdtemp()
    subprocess.check_call(["cc", "-O2", "-Wno-everything", "-o", wd + "/pack", here + "/lz4w_pack.c"])
    data = open(inp, "rb").read()[:nbytes]
    open(wd + "/in", "wb").write(data)
    subprocess.check_call([wd + "/pack", wd + "/in", wd + "/packed"], stderr=subprocess.DEVNULL)
    n = len(open(wd + "/packed", "rb").read())
    src = (
        """        org     $1000
start:  lea     packed,a0
        lea     dest,a1
        bsr     lz4w_unpack_a
        stop    #$2700
"""
        + convert(open(unpack).read())
        + """
        even
packed: incbin  "%s/packed"
        even
dest:   ds.b    %d
""" % (wd, nbytes + 16)
    )
    open(wd + "/p.s", "w").write(src)
    r = subprocess.run(
        [vasm, "-Fbin", "-m68000", "-quiet", "-o", outbin, wd + "/p.s"], capture_output=True, text=True
    )
    if r.returncode:
        print(r.stdout + r.stderr, file=sys.stderr)
        sys.exit(1)
    print(
        "%s: %d bytes -> %d packed; image %d bytes" % (name, nbytes, n, os.path.getsize(outbin)),
        file=sys.stderr,
    )
    shutil.rmtree(wd)


main()
