#!/usr/bin/env python3
"""Writes an 880 KB OFS floppy image (ADF) named "Reboot" holding one program, Reboot, which calls exec's ColdReboot():
  reboot_adf.py out.adf
On Buffee the 68000's RESET instruction inside ColdReboot ends the run and pulses the Amiga's RESET line: run DF0:Reboot from a Shell and the console
shows "RESET at $..." before Kickstart starts again. Workbench 3.1 has no reboot command of its own (Kickstart 2.0 or later: ColdReboot is exec V36).
The disk is not bootable."""

import struct, sys

BLOCKS, ROOT, BITMAP, HEADER, DATA = 1760, 880, 881, 882, 883
DAYS = 17670                                                  # 2026-05-18 (days since 1978-01-01), any valid date

CODE = bytes.fromhex("2C780004" "4EAEFD2A" "7000" "4E75")    # movea.l 4.w,a6 ; jsr _LVOColdReboot(a6) (-726) ; moveq #0,d0 ; rts
assert len(CODE) % 4 == 0
EXE = struct.pack(">6L", 0x3F3, 0, 1, 0, 0, len(CODE) // 4) + struct.pack(">2L", 0x3E9, len(CODE) // 4) + CODE + struct.pack(">L", 0x3F2)


def longs(b):
    return struct.unpack(">128L", b)


def put_checksum(b, at):
    """The long at byte `at` makes the block's longs add up to 0 (mod 2^32)."""
    s = sum(longs(b)) & 0xFFFFFFFF
    struct.pack_into(">L", b, at, (-s) & 0xFFFFFFFF)


def bcpl(b, at, name):
    b[at] = len(name)
    b[at + 1:at + 1 + len(name)] = name.encode("latin1")


def name_hash(name):
    h = len(name)
    for c in name.upper():
        h = (h * 13 + ord(c)) & 0x7FF
    return h % 72


img = bytearray(BLOCKS * 512)
img[0:4] = b"DOS\0"                                           # OFS; no boot code, so not bootable
struct.pack_into(">L", img, 8, ROOT)

root = bytearray(512)
struct.pack_into(">L", root, 0, 2)                            # T_HEADER
struct.pack_into(">L", root, 12, 72)                          # hash table size
struct.pack_into(">L", root, 24 + 4 * name_hash("Reboot"), HEADER)
struct.pack_into(">l", root, 312, -1)                         # the bitmap is valid
struct.pack_into(">L", root, 316, BITMAP)
for at in (420, 472, 484):                                    # changed, volume changed, created
    struct.pack_into(">3L", root, at, DAYS, 0, 0)
bcpl(root, 432, "Reboot")
struct.pack_into(">l", root, 508, 1)                          # ST_ROOT
put_checksum(root, 20)

bm = bytearray(512)
used = {ROOT, BITMAP, HEADER, DATA}
for blk in range(2, BLOCKS):                                  # bit n = block n + 2, 1 = free
    if blk not in used:
        n = blk - 2
        off = 4 + 4 * (n // 32)
        struct.pack_into(">L", bm, off, struct.unpack_from(">L", bm, off)[0] | 1 << (n % 32))
put_checksum(bm, 0)

hdr = bytearray(512)
struct.pack_into(">5L", hdr, 0, 2, HEADER, 1, 0, DATA)        # T_HEADER, itself, one data block, -, the first data block
struct.pack_into(">L", hdr, 24 + 4 * 71, DATA)                # data block table, filled from the end
struct.pack_into(">L", hdr, 324, len(EXE))
struct.pack_into(">3L", hdr, 420, DAYS, 0, 0)
bcpl(hdr, 432, "Reboot")
struct.pack_into(">L", hdr, 500, ROOT)                        # parent
struct.pack_into(">l", hdr, 508, -3)                          # ST_FILE
put_checksum(hdr, 20)

data = bytearray(512)
struct.pack_into(">5L", data, 0, 8, HEADER, 1, len(EXE), 0)  # T_DATA, its header, sequence 1, bytes, no next block
data[24:24 + len(EXE)] = EXE
put_checksum(data, 20)

for blk, b in ((ROOT, root), (BITMAP, bm), (HEADER, hdr), (DATA, data)):
    img[blk * 512:(blk + 1) * 512] = b
open(sys.argv[1], "wb").write(img)
print(f"{sys.argv[1]}: {len(img)} bytes, Reboot ({len(EXE)} bytes) calls ColdReboot()")
