#!/usr/bin/env python3
"""Turns an m68k-elf program linked by amiga/amiga.ld (one section, `.hunk`, at address 0, with --emit-relocs) into an AmigaDOS executable: one code hunk and the relocations
of its absolute addresses (HUNK_RELOC32). Every 32-bit absolute relocation becomes one entry; PC-relative ones need nothing; a 16- or 8-bit absolute one cannot be loaded
anywhere and is refused. With --no-relocs a program that needs any relocation is refused (one built with -mpcrel must not).
  elf2hunk.py [--no-relocs] program.elf program"""

import struct, sys

HUNK_HEADER, HUNK_CODE, HUNK_RELOC32, HUNK_END = 0x3F3, 0x3E9, 0x3EC, 0x3F2
R_68K_32, R_68K_16, R_68K_8 = 1, 2, 3
SHT_RELA, SHT_REL = 4, 9


def main(src, dst, no_relocs=False):
    e = open(src, "rb").read()
    if e[:4] != b"\x7fELF" or e[4] != 1 or e[5] != 2 or struct.unpack(">H", e[18:20])[0] != 4:
        sys.exit(f"{src}: not a 32-bit big-endian m68k ELF")
    shoff, = struct.unpack(">I", e[32:36])
    shentsize, shnum, shstrndx = struct.unpack(">HHH", e[46:52])
    secs = [struct.unpack(">IIIIIIIIII", e[shoff + i * shentsize: shoff + i * shentsize + 40]) for i in range(shnum)]
    strtab = secs[shstrndx]

    def name(s):
        o = strtab[4] + s[0]
        return e[o:e.index(b"\0", o)].decode()

    idx = [i for i, s in enumerate(secs) if name(s) == ".hunk"]
    if len(idx) != 1:
        sys.exit(f"{src}: no single .hunk section (amiga/amiga.ld)")
    h = secs[idx[0]]
    if h[3] != 0:
        sys.exit(f"{src}: .hunk is not linked at 0")
    data = bytearray(e[h[4]:h[4] + h[5]])
    data += b"\0" * (-len(data) % 4)
    relocs = []
    for s in secs:
        if s[1] not in (SHT_RELA, SHT_REL) or s[7] != idx[0]:
            continue
        ent = 12 if s[1] == SHT_RELA else 8
        for k in range(s[5] // ent):
            off, info = struct.unpack(">II", e[s[4] + k * ent: s[4] + k * ent + 8])
            t = info & 0xFF
            if t == R_68K_32:
                relocs.append(off)
            elif t in (R_68K_16, R_68K_8):
                sys.exit(f"{src}: a {16 if t == R_68K_16 else 8}-bit absolute address at {off:#x}: build with -fno-pic and no absolute short addressing of symbols")
    if no_relocs and relocs:
        sys.exit(f"{src}: {len(relocs)} absolute addresses, the first at {min(relocs):#x}: the program is to be PC-relative")
    n = len(data) // 4
    out = struct.pack(">IIIII", HUNK_HEADER, 0, 1, 0, 0) + struct.pack(">I", n)
    out += struct.pack(">II", HUNK_CODE, n) + bytes(data)
    if relocs:
        out += struct.pack(">III", HUNK_RELOC32, len(relocs), 0) + b"".join(struct.pack(">I", r) for r in sorted(relocs)) + struct.pack(">I", 0)
    out += struct.pack(">I", HUNK_END)
    open(dst, "wb").write(out)
    print(f"{dst}: {len(data)} bytes of code and data, {len(relocs)} relocations")


if __name__ == "__main__":
    args = [a for a in sys.argv[1:] if a != "--no-relocs"]
    if len(args) != 2:
        sys.exit(__doc__)
    main(args[0], args[1], "--no-relocs" in sys.argv)
