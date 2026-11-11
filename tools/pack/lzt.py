#!/usr/bin/env python3
"""LZT1: the table-image packer for boot-time loading (decoder: pjit/lzt.s).

The image is cut into 16-byte records (the DT's slots; the other tables do not mind), the records are split
into 16 byte planes (plane k = byte k of every record) and the
planes are compressed as one LZ4 block (`lz4 -9` quality, liblz4's HC compressor through ctypes: needs liblz4,
e.g. Homebrew's).  On the 2.4 MB table arena: 291 KB with
plain `lz4 -9`, 113 KB with the planes.  Layout (little-endian words): 'LZT1', raw size, records (the input is
padded with zeros to a multiple of 256 bytes), compressed size, then the block.

  tools/pack/lzt.py pack   in out [level]      (level 1..12, default 12)
  tools/pack/lzt.py unpack in out              (reference decoder, checks the format)
  tools/pack/lzt.py header in name > file.h    (a C array of the packed image, for tests)"""

import sys, struct, ctypes, ctypes.util, os


def _lib():
    for p in (
        ctypes.util.find_library("lz4"),
        "/opt/homebrew/lib/liblz4.dylib",
        "/opt/local/lib/liblz4.dylib",
        "liblz4.so.1",
    ):
        try:
            if p:
                return ctypes.CDLL(p)
        except OSError:
            pass
    sys.exit("liblz4 not found")


lz = _lib()


def planes(raw, stride=16):
    n = (len(raw) + 255) // 256 * 256
    raw = raw + bytes(n - len(raw))
    r = n // stride
    return (raw if stride == 1 else bytes(b for k in range(stride) for b in raw[k::stride])), r


def compress(pl, level=12):
    cap = len(pl) + len(pl) // 255 + 64
    out = ctypes.create_string_buffer(cap)
    n = lz.LZ4_compress_HC(pl, out, len(pl), cap, level)
    if n <= 0:
        sys.exit("compression failed")
    return out.raw[:n]


def pack_part(raw, level=12, force=None):
    """the best of plain, 4-, 8- and 16-byte planes (or just the forced one): (stride_log2, padded raw size,
    packed block)
    """
    best = None
    for sl in ((0, 2, 3, 4) if force is None else (force,)):
        pl, _ = planes(raw, 1 << sl)
        blk = compress(pl, level)
        if best is None or len(blk) < len(best[2]):
            best = (sl, len(pl), blk)
    return best


def bundle(parts, level=12):
    """parts: [(offset from the load base, bytes)] -> an LZTB bundle: 'LZTB', n, n x (offset, padded size,
    packed size, stride_log2), then the blocks (4-aligned)
    """
    ents, blobs = [], []
    for p in parts:
        off, raw = p[0], p[1]
        sl, size, blk = pack_part(raw, level, p[2] if len(p) > 2 else None)
        ents.append(struct.pack("<IIII", off, size, len(blk), sl))
        blobs.append(blk + bytes(-len(blk) % 4))
    return b"LZTB" + struct.pack("<I", len(parts)) + b"".join(ents) + b"".join(blobs)


def unbundle(img):
    if img[:4] != b"LZTB":
        sys.exit("not an LZTB bundle")
    n = struct.unpack("<I", img[4:8])[0]
    pos = 8 + 16 * n
    out = []
    for i in range(n):
        off, size, psize, sl = struct.unpack("<IIII", img[8 + 16 * i : 24 + 16 * i])
        pl = ctypes.create_string_buffer(size)
        if lz.LZ4_decompress_safe(img[pos : pos + psize], pl, psize, size) != size:
            sys.exit("corrupt block %d" % i)
        pl = pl.raw
        st = 1 << sl
        if sl:
            r = size // st
            o = bytearray(size)
            [o.__setitem__(slice(k, None, st), pl[k * r : (k + 1) * r]) for k in range(st)]
            pl = bytes(o)
        out.append((off, pl))
        pos += (psize + 3) & ~3
    return out


def pack(raw, level=12):
    pl, r = planes(raw)
    cap = len(pl) + len(pl) // 255 + 64
    out = ctypes.create_string_buffer(cap)
    n = lz.LZ4_compress_HC(pl, out, len(pl), cap, level)
    if n <= 0:
        sys.exit("compression failed")
    return b"LZT1" + struct.pack("<III", len(raw), r, n) + out.raw[:n]


def unpack(img):
    if img[:4] != b"LZT1":
        sys.exit("not an LZT1 image")
    raw_n, r, n = struct.unpack("<III", img[4:16])
    pl = ctypes.create_string_buffer(16 * r)
    if lz.LZ4_decompress_safe(img[16 : 16 + n], pl, n, 16 * r) != 16 * r:
        sys.exit("corrupt block")
    pl = pl.raw
    out = bytearray(16 * r)
    for k in range(16):
        out[k::16] = pl[k * r : (k + 1) * r]
    return bytes(out[:raw_n])


if __name__ == "__main__":
    a = sys.argv[1:]
    if len(a) >= 3 and a[0] == "pack":
        raw = open(a[1], "rb").read()
        img = pack(raw, int(a[3]) if len(a) > 3 else 12)
        assert unpack(img) == raw
        open(a[2], "wb").write(img)
        print("%d -> %d bytes (%.1f%%)" % (len(raw), len(img), 100.0 * len(img) / max(1, len(raw))))
    elif len(a) == 3 and a[0] == "unpack":
        open(a[2], "wb").write(unpack(open(a[1], "rb").read()))
    elif len(a) == 3 and a[0] == "header":
        img = pack(open(a[1], "rb").read())
        print(
            "static const unsigned char %s[] __attribute__((aligned(4))) = {%s};"
            % (a[2], ",".join(str(b) for b in img))
        )
    else:
        sys.exit(__doc__)
