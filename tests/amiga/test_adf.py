#!/usr/bin/env python3
"""tools/amiga/adf.py checked by reading back what it wrote, the way AmigaDOS reads a floppy: the boot block's and every block's checksum, the root's hash table and its
chains, directories, file headers and their extension blocks, the data blocks' links and sequence numbers, and the bitmap against the blocks in use. The files are made up
here: an empty one, one of a block, one that needs two extension blocks, names that share a hash slot, a directory two deep.
  test_adf.py"""

import os, random, struct, sys, tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "..", "..", "tools", "amiga"))
import adf  # noqa: E402

rnd = random.Random(2)
work = tempfile.mkdtemp(prefix="adf_test_")
files = {"empty": b"", "c/one": bytes(rnd.getrandbits(8) for _ in range(300)), "big.bin": bytes(rnd.getrandbits(8) for _ in range(488 * 150 + 17)),
         "s/startup-sequence": b"C:one\n", "a/b/deep": b"x" * 1000}
for n in range(12):                                                    # enough names for a few to share a hash slot
    files[f"f{n}"] = bytes([n]) * (n * 100)
args = []
for i, (path, data) in enumerate(files.items()):
    src = os.path.join(work, f"src{i}")
    open(src, "wb").write(data)
    args.append(f"{path}={src}")
out = os.path.join(work, "t.adf")
adf.main([out, "Test", "--boot"] + args)
img = open(out, "rb").read()
fail = []


def blk(n):
    return img[n * 512:(n + 1) * 512]


def L(b, o):
    return struct.unpack_from(">L", b, o)[0]


def l(b, o):
    return struct.unpack_from(">l", b, o)[0]


def summed(b):
    return sum(struct.unpack(">128L", b)) & 0xFFFFFFFF == 0


s = 0
for (w,) in struct.iter_unpack(">L", img[:1024]):
    s += w
    s = (s & 0xFFFFFFFF) + (s >> 32)
if img[:4] != b"DOS\0" or s != 0xFFFFFFFF:
    fail.append("boot block: not DOS\\0 or its checksum is wrong")
used = {0, 1, 880, 881}
found = {}


def walk(dirblk, prefix):
    d = blk(dirblk)
    for slot in range(72):
        e = L(d, 24 + 4 * slot)
        while e:
            b = blk(e)
            used.add(e)
            name = b[433:433 + b[432]].decode("latin1")
            if not summed(b) or L(b, 0) != 2 or L(b, 4) != e or L(b, 500) != dirblk:
                fail.append(f"{prefix}{name}: header block {e} is wrong")
            if adf.name_hash(name) != slot:
                fail.append(f"{prefix}{name}: in hash slot {slot}, not {adf.name_hash(name)}")
            if l(b, 508) == 2:
                walk(e, prefix + name + "/")
            else:
                size, data, seq, owner, tab = L(b, 324), b"", 1, e, b
                blocks = []
                while True:
                    n = L(tab, 8)
                    blocks += [L(tab, 24 + 4 * (71 - i)) for i in range(n)]
                    nxt = L(tab, 504)
                    if not nxt:
                        break
                    tab = blk(nxt); used.add(nxt)
                    if L(tab, 0) != 16 or not summed(tab) or L(tab, 500) != e:
                        fail.append(f"{prefix}{name}: extension block {nxt} is wrong")
                for k, db in enumerate(blocks):
                    d2 = blk(db); used.add(db)
                    if L(d2, 0) != 8 or L(d2, 4) != e or L(d2, 8) != k + 1 or not summed(d2) or L(d2, 16) != (blocks[k + 1] if k + 1 < len(blocks) else 0):
                        fail.append(f"{prefix}{name}: data block {db} (#{k + 1}) is wrong")
                    data += d2[24:24 + L(d2, 12)]
                if blocks and L(b, 16) != blocks[0]:
                    fail.append(f"{prefix}{name}: first_data is not the first data block")
                found[prefix + name] = data if len(data) == size else None
            e = L(b, 496)


root = blk(880)
if not summed(root) or l(root, 508) != 1 or L(root, 316) != 881:
    fail.append("root block is wrong")
walk(880, "")
for path, data in files.items():
    if found.get(path) != data:
        fail.append(f"{path}: not read back the same")
bm = blk(881)
if not summed(bm):
    fail.append("bitmap checksum")
for b in range(2, 1760):
    n = b - 2
    free = L(bm, 4 + 4 * (n // 32)) >> (n % 32) & 1
    if free == (b in used):
        fail.append(f"bitmap: block {b} is {'free' if free else 'in use'} but {'used' if b in used else 'unused'}")
        break
print(f"adf.py: {len(found)} files read back" + (": all the same" if not fail else ": " + "; ".join(fail[:5])))
if not fail:
    import shutil
    shutil.rmtree(work)
sys.exit(1 if fail else 0)
