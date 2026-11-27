#!/usr/bin/env python3
"""Writes an 880 KB AmigaDOS floppy image (ADF, the original file system, OFS) holding the files given, in directories as their names say:
  adf.py out.adf VOLUME [--boot] path/on/disk=host/file ...
e.g.  adf.py buffee.adf Buffee --boot c/buffee=build/amiga/buffee s/startup-sequence=amiga/buffee/startup-sequence
--boot gives it a boot block that starts AmigaDOS (Kickstart 1.2 and later), which then runs s/startup-sequence. The boot block's code is our own: it finds dos.library's
resident tag and returns its init, as any bootable disk does. Nothing else is put on the disk."""

import struct, sys

BLOCKS, ROOT, BITMAP = 1760, 880, 881
DAYS = 17670                                     # 2026-05-19 (days since 1978-01-01): any valid date
T_HEADER, T_DATA, T_LIST = 2, 8, 16
ST_ROOT, ST_USERDIR, ST_FILE = 1, 2, -3
DATA_BYTES, TABLE = 488, 72

# lea dosname(pc),a1 ; jsr FindResident(a6) ; tst.l d0 ; beq.s fail ; move.l d0,a0 ; move.l rt_Init(a0),a0 ; moveq #0,d0 ; rts ; fail: moveq #-1,d0 ; rts ; dosname
BOOT_CODE = bytes.fromhex("43FA0018" "4EAEFFA0" "4A80" "670A" "2040" "20680016" "7000" "4E75" "70FF" "4E75") + b"dos.library\0"


def name_hash(name):
    h = len(name)
    for c in name.upper():
        h = (h * 13 + ord(c)) & 0x7FF
    return h % TABLE


def put_checksum(b, at):
    """The long at byte `at` makes the block's longs add up to 0 (mod 2^32)."""
    struct.pack_into(">L", b, at, 0)
    s = sum(struct.unpack(">128L", bytes(b))) & 0xFFFFFFFF
    struct.pack_into(">L", b, at, (-s) & 0xFFFFFFFF)


def bcpl(b, at, name):
    b[at] = len(name)
    b[at + 1:at + 1 + len(name)] = name.encode("latin1")


class Disk:
    def __init__(self, volume, boot):
        self.img = bytearray(BLOCKS * 512)
        self.used = {0, 1, ROOT, BITMAP}
        self.next = ROOT + 2
        self.blocks = {}                         # block -> bytearray (headers, written last with their checksums)
        self.img[0:4] = b"DOS\0"
        struct.pack_into(">L", self.img, 8, ROOT)
        if boot:
            self.img[12:12 + len(BOOT_CODE)] = BOOT_CODE
            s = 0
            for (w,) in struct.iter_unpack(">L", bytes(self.img[0:1024])):
                s += w
                if s > 0xFFFFFFFF:
                    s = (s & 0xFFFFFFFF) + 1
            struct.pack_into(">L", self.img, 4, ~s & 0xFFFFFFFF)
        root = bytearray(512)
        struct.pack_into(">L", root, 0, T_HEADER)
        struct.pack_into(">L", root, 12, TABLE)
        struct.pack_into(">l", root, 312, -1)    # the bitmap is valid
        struct.pack_into(">L", root, 316, BITMAP)
        for at in (420, 472, 484):               # changed, volume changed, created
            struct.pack_into(">3L", root, at, DAYS, 0, 0)
        bcpl(root, 432, volume)
        struct.pack_into(">l", root, 508, ST_ROOT)
        self.blocks[ROOT] = root
        self.dirs = {"": ROOT}
        self.entries = 0

    def alloc(self):
        while self.next in self.used:
            self.next = self.next + 1 if self.next + 1 < BLOCKS else 2
            if len(self.used) >= BLOCKS:
                sys.exit("the files do not fit on an 880 KB floppy")
        self.used.add(self.next)
        return self.next

    def link(self, parent, blk, name):
        """Puts the entry into its directory's hash table (at the head of the chain)."""
        p, h = self.blocks[parent], 24 + 4 * name_hash(name)
        struct.pack_into(">L", self.blocks[blk], 496, struct.unpack_from(">L", p, h)[0])
        struct.pack_into(">L", p, h, blk)

    def entry(self, parent, name, sec):
        blk = self.alloc()
        b = bytearray(512)
        struct.pack_into(">LL", b, 0, T_HEADER, blk)
        struct.pack_into(">3L", b, 420, DAYS, 0, 0)
        bcpl(b, 432, name)
        struct.pack_into(">L", b, 500, parent)
        struct.pack_into(">l", b, 508, sec)
        self.blocks[blk] = b
        self.link(parent, blk, name)
        self.entries += 1
        return blk

    def directory(self, path):
        if path not in self.dirs:
            up, _, name = path.rpartition("/")
            self.dirs[path] = self.entry(self.directory(up), name, ST_USERDIR)
        return self.dirs[path]

    def file(self, path, data):
        up, _, name = path.rpartition("/")
        hdr = self.entry(self.directory(up), name, ST_FILE)
        struct.pack_into(">L", self.blocks[hdr], 324, len(data))
        pieces = [data[i:i + DATA_BYTES] for i in range(0, len(data), DATA_BYTES)] or []
        datablocks = [self.alloc() for _ in pieces]
        for k, (blk, piece) in enumerate(zip(datablocks, pieces)):
            d = bytearray(512)
            nxt = datablocks[k + 1] if k + 1 < len(datablocks) else 0
            struct.pack_into(">5L", d, 0, T_DATA, hdr, k + 1, len(piece), nxt)
            d[24:24 + len(piece)] = piece
            put_checksum(d, 20)
            self.img[blk * 512:(blk + 1) * 512] = d
        table_owner, prev_ext = hdr, None
        for start in range(0, max(len(datablocks), 1), TABLE):         # the header's table, then one extension block per further 72
            chunk = datablocks[start:start + TABLE]
            if start:
                ext = self.alloc()
                b = bytearray(512)
                struct.pack_into(">LL", b, 0, T_LIST, ext)
                struct.pack_into(">L", b, 500, hdr)
                struct.pack_into(">l", b, 508, ST_FILE)
                self.blocks[ext] = b
                struct.pack_into(">L", self.blocks[table_owner], 504, ext)
                table_owner = ext
            t = self.blocks[table_owner]
            struct.pack_into(">L", t, 8, len(chunk))                   # high_seq: entries in this table
            for i, blk in enumerate(chunk):
                struct.pack_into(">L", t, 24 + 4 * (TABLE - 1 - i), blk)
        if datablocks:
            struct.pack_into(">L", self.blocks[hdr], 16, datablocks[0])

    def write(self, out):
        bm = bytearray(512)
        for blk in range(2, BLOCKS):                                    # bit n = block n + 2, 1 = free
            if blk not in self.used:
                n = blk - 2
                off = 4 + 4 * (n // 32)
                struct.pack_into(">L", bm, off, struct.unpack_from(">L", bm, off)[0] | 1 << (n % 32))
        put_checksum(bm, 0)
        self.img[BITMAP * 512:(BITMAP + 1) * 512] = bm
        for blk, b in self.blocks.items():
            put_checksum(b, 20)
            self.img[blk * 512:(blk + 1) * 512] = b
        open(out, "wb").write(self.img)
        free = BLOCKS - len(self.used)
        print(f"{out}: {self.entries} files and directories, {free} blocks ({free * DATA_BYTES // 1024} KB) free")


def main(argv):
    if len(argv) < 3:
        sys.exit(__doc__)
    out, volume, rest = argv[0], argv[1], argv[2:]
    boot = "--boot" in rest
    d = Disk(volume, boot)
    for spec in (a for a in rest if a != "--boot"):
        path, _, src = spec.partition("=")
        d.file(path, open(src, "rb").read())
    d.write(out)


if __name__ == "__main__":
    main(sys.argv[1:])
