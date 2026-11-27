#!/usr/bin/env python3
"""Writes the resident `buffee` command (build/amiga/resident.bin, the RomTag of amiga/buffee/resident.S linked at its guest address) as a C header for the PJIT payload,
which copies it into the MapROM megabyte at boot (platform/buffee/buffee.c). The header is checked in, so the firmware builds without m68k-elf-gcc; `make amiga-embed`
writes it again and `make amiga-test` fails while it differs from what the command's sources build.
  embed.py resident.bin address out.h          (address in hex: F01000)
  embed.py --check resident.bin address out.h"""

import sys

check = sys.argv[1] == "--check"
src, addr, out = sys.argv[2:] if check else sys.argv[1:]
b = open(src, "rb").read()
lines = ["/* buffee_cmd.h -- written by tools/amiga/embed.py from amiga/buffee/ (make amiga-embed): do not edit. The resident `buffee` command, a RomTag and the",
         " * program, linked at guest $%s; platform/buffee/buffee.c copies it there. */" % addr.upper(),
         "#define BUFFEE_CMD_AT 0x%sU" % addr.upper(),
         "static const unsigned char buffee_cmd[%d] = {" % len(b)]
for i in range(0, len(b), 24):
    lines.append("    " + ", ".join("0x%02X" % x for x in b[i:i + 24]) + ",")
lines.append("};")
text = "\n".join(lines) + "\n"
if check:
    try:
        same = open(out).read() == text
    except FileNotFoundError:
        same = False
    print(f"{out}: {'up to date' if same else 'differs from what amiga/buffee/ builds: make amiga-embed'}")
    sys.exit(0 if same else 1)
open(out, "w").write(text)
print(f"{out}: {len(b)} bytes at ${addr.upper()}")
