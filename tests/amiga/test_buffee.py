#!/usr/bin/env python3
"""The `buffee` command (amiga/buffee/) against PJIT's control page (platform/buffee/control.c) on a simulated board (amiga_host.c, mock_board.c): each case runs one
command line on the golden 68000 core and checks what it printed, its return code, and what the simulated flash and EEPROM hold afterwards. The images it stores are
made up here: a "bundle" is LZTB and random bytes, a "Kickstart" has a Kickstart's start, version and checksum and random bytes otherwise.
The last cases run the resident form (amiga/buffee/resident.S): the RomTag's init adds the command to the resident list, and the command runs from there, several
times on one copy as a Shell runs it. The arguments are read by dos's ReadArgs (the harness's, as dos 36's) and, with dos 34 (Kickstart 1.3), by the command's own.
  test_buffee.py <amiga_host> <buffee> <resident.bin>"""

import os, random, re, struct, subprocess, sys, tempfile, zlib

host, prog, resident = sys.argv[1], sys.argv[2], sys.argv[3]
work = tempfile.mkdtemp(prefix="buffee_test_")
flash, eeprom = os.path.join(work, "flash.bin"), os.path.join(work, "eeprom.bin")
rnd = random.Random(1)
failures = 0


def blob(n):
    return bytes(rnd.getrandbits(8) for _ in range(n))


def kickstart(size, version=(40, 63)):
    """A made-up image that passes the checks a Kickstart has to pass: $1114 4EF9 at the start, the version at +12, the longs summing to $FFFFFFFF with the carry."""
    b = bytearray(blob(size))
    b[0:4] = bytes.fromhex("11144EF9" if size >= 0x80000 else "11114EF9")
    struct.pack_into(">HH", b, 12, *version)
    struct.pack_into(">I", b, size - 24, 0)
    s = 0
    for i in range(0, size, 4):
        s += struct.unpack_from(">I", b, i)[0]
        s = (s & 0xFFFFFFFF) + (s >> 32)
    struct.pack_into(">I", b, size - 24, (0xFFFFFFFF - s) & 0xFFFFFFFF)
    return bytes(b)


printed = []                                     # everything the command printed: checked for width and stray formats at the end


def run(*args, rom=None, res=False, tag=None, nodos=False, uc=None, ver=None, execver=None):
    cmd = [host, "-f", flash, "-e", eeprom, "-d", work] + (["-r", rom] if rom else []) + (["-V", str(ver)] if ver else []) + (["-X", str(execver)] if execver else []) + (["-T", str(tag)] if tag is not None else []) + (["-N", "1"] if nodos else []) + (["-U", str(uc)] if uc is not None else []) + (["-R", resident] if res else [prog]) + list(args)
    p = subprocess.run(cmd, capture_output=True, text=True, timeout=600)
    printed.append(p.stdout)
    return p.returncode, p.stdout, p.stderr


def check(name, ok, detail=""):
    global failures
    print(f"  {'ok  ' if ok else 'FAIL'} {name}" + (f"  ({detail})" if detail and not ok else ""))
    if not ok:
        failures += 1


def slot(off):
    f = open(flash, "rb").read()
    magic, ln, crc, seq = struct.unpack_from("<IIII", f, off)
    return (magic, ln, crc, seq), f[off + 0x100: off + 0x100 + ln]


bundle = b"LZTB" + blob(150000)
open(os.path.join(work, "pjit.lzt"), "wb").write(bundle)
bundle2 = b"LZTB" + blob(90000)
open(os.path.join(work, "pjit2.lzt"), "wb").write(bundle2)
open(os.path.join(work, "notabundle"), "wb").write(b"XXXX" + blob(1000))
ks = kickstart(0x80000)
open(os.path.join(work, "kick.rom"), "wb").write(ks)
bad = bytearray(ks); bad[1000] ^= 1
open(os.path.join(work, "bad.rom"), "wb").write(bad)
amiga_rom = os.path.join(work, "amiga.rom")
open(amiga_rom, "wb").write(kickstart(0x40000, (34, 5)))

print("buffee on the simulated board")
rc, out, err = run()
check("status on a blank board", rc == 0 and "  PJIT    empty" in out and "MAPROM=OFF" in out, out + err)

check("... and says the command is not resident (no tag found)", "Resident  no: Kickstart did not find the tag" in out, out)
rc, out, err = run(tag=1)
check("a tag that exec found but did not run is told apart", "Resident  no: Kickstart found the tag but did not run it" in out, out)

rc, out, err = run("HELP")
check("HELP explains the options", rc == 0 and "PJIT=<file>" in out, out)
rc, out, err = run("?")
check("? shows the template (then, given an empty line, the status)", rc == 0 and out.startswith("PJIT/K,ROM/K,") and "Next boot" in out, out)

rc, out, err = run("BOGUS=1")
check("an unknown argument is refused as ReadArgs refuses it", rc == 10 and "buffee: wrong number of arguments" in out, out)
rc, out, err = run("PACE=fast")
check("a /N value that is no number", rc == 10 and "buffee: bad number" in out, out)
rc, out, err = run("PACE=300")
check("a pace out of range", rc == 10 and "PACE is 0 to 255" in out, out)

before = open(eeprom, "rb").read() if os.path.exists(eeprom) else b""
rc, out, err = run("ROM=nosuch.rom", "FASTRAM=200000/8")
check("a file that cannot be opened leaves the settings as they were",
      rc == 10 and "Cannot open nosuch.rom" in out and "Settings saved" not in out and (open(eeprom, "rb").read() if os.path.exists(eeprom) else b"") == before, out)

rc, out, err = run("PJIT=df0:pjit.lzt")
h, data = slot(0x90000)
check("PJIT= stores into the update slot", h[0] == 0x544A4A50 and h[1] == len(bundle) and data == bundle and h[2] == zlib.crc32(bundle) and h[3] == 1, f"{h} {out}")
check("... and reboots", "[board reset]" in err and "Rebooting" in out, out + err)
h0, _ = slot(0x20000)
check("... and leaves the backup slot alone", h0[0] != 0x544A4A50)

rc, out, err = run("PJIT=pjit.lzt", "VERIFY")
check("VERIFY finds it", rc == 0 and "Same as the update slot, #1" in out, out)

rc, out, err = run("PJIT=pjit2.lzt", "VERIFY")
check("VERIFY of another file says no (return code 5)", rc == 5 and "Not in the flash" in out, out)

rc, out, err = run("PJIT=pjit2.lzt", "NOREBOOT")
h, data = slot(0x90000)
check("a second bundle gets #2", h[3] == 2 and data == bundle2, str(h))
check("NOREBOOT does not reboot", "[board reset]" not in err and "Used from the next boot" in out, out + err)

rc, out, err = run("PJIT=notabundle", "NOREBOOT")
h, data = slot(0x90000)
check("a file that is not a bundle is refused, the slot kept", rc == 10 and "Not a PJIT bundle" in out and data == bundle2, out)

rc, out, err = run("ROM=kick.rom", "NOREBOOT")
h, data = slot(0x100000)
check("ROM= stores a Kickstart", rc == 0 and "Kickstart 40.63, 512 KB" in out and h[0] == 0x4D4F5250 and data == ks, out)

rc, out, err = run("ROM=bad.rom", "NOREBOOT")
h, data = slot(0x100000)
check("a damaged Kickstart is refused, the image kept", rc == 10 and "checksum wrong" in out and data == ks, out)

rc, out, err = run("ROM=pjit.lzt", "NOREBOOT")
check("a file that is not a Kickstart is refused", rc == 10 and ("Not a Kickstart" in out or "256 KB, 512 KB or 1 MB" in out), out)

rc, out, err = run("ROM=kick.rom", "VERIFY")
check("ROM= VERIFY finds it", rc == 0 and "Same as the ROM image" in out, out)

rc, out, err = run("COPYROM", "NOREBOOT", rom=amiga_rom)
h, data = slot(0x100000)
check("COPYROM stores the Amiga's ROM (as MapROM sizes it: 512 KB)", rc == 0 and "Kickstart 34.5" in out and h[1] == 0x80000, out)

rc, out, err = run("FASTRAM=200000/8", "MAPROM=FLASH", "NOREBOOT")
rc2, out2, _ = run("STATUS")
check("FASTRAM= and MAPROM= are saved", rc == 0 and "FASTRAM=200000/8" in out2 and "MAPROM=FLASH" in out2, out + out2)

rc, out, err = run("FASTRAM=NONE", "NOREBOOT")
rc2, out2, _ = run()
check("FASTRAM=NONE clears it", "FASTRAM=NONE" in out2, out + out2)

rc, out, err = run("FASTRAM=123456/8", "NOREBOOT")
check("a region that is not 1 MB aligned is rejected", "rejected" in out, out)

rc, out, err = run("PACE=50")
check("PACE= changes the pace now, no reboot", rc == 0 and "Pace 50 %" in out and "[board reset]" not in err, out + err)

rc, out, err = run("PACE=70", "SAVE")
rc2, out2, _ = run()
check("PACE= SAVE keeps it", "PACE=70 " in out2, out2)

rc, out, err = run("CLOCK=600", "SAVE")
rc2, out2, _ = run()
check("CLOCK= SAVE keeps it", rc == 0 and "ARM clock 600 MHz" in out and "CLOCK=600 " in out2, out + out2)

rc, out, err = run("CLOCK=100", "SAVE")
check("a clock under 300 MHz is used but not saved", "Not saved" in out, out)

rc, out, err = run("L2=OFF")
check("L2=OFF switches the L2 off", rc == 0 and "L2 off" in out, out)
rc, out, err = run("L2=ON", "L1=OFF")
check("L2=ON L1=OFF", rc == 0 and "L1I off, L1D off, L2 on" in out, out)
rc, out, err = run("TRANSLATE=NONE")
check("TRANSLATE= is not offered", rc == 10 and "wrong number of arguments" in out, out)
rc, out, err = run("CLOCK=1100")
check("a clock above the boot clock is refused", rc == 10 and "At most" in out, out)

rc, out, err = run("SELFTEST=FULL,NOBUS", "REPORT=VERBOSE", "BUSCLOCK=7.09", "NOREBOOT")
rc2, out2, _ = run()
check("SELFTEST=, REPORT=, BUSCLOCK= are saved", "SELFTEST=FULL,NOBUS" in out2 and "REPORT=VERBOSE" in out2 and "BUSCLOCK=7.090" in out2, out + out2)

rc, out, err = run(res=True)
check("resident: the tag adds the command, which shows the status", rc == 0 and "SELFTEST=FULL,NOBUS" in out and "Resident  yes" in out, out + err)
check("... and its init told the board it added it", "[resident init: 4, use count 1]" in err, err)
rc, out, err = run(res=True, uc=-1)
check("an entry that dos marked as a system one (use count -1) is reported as such", "Resident  no: listed with use count -1\n" in out, out)
rc, out, err = run(res=True, uc=0)
check("an entry with use count 0, which Kickstart 3.1's Shell passes over, is reported as such",
      "Resident  no: added, but the Shell does not find it" in out and "[resident init: 4, use count -2147483648]" in err, out + err)
rc, out, err = run(res=True, nodos=True)
check("an init that finds no dos.library tells the board so", rc == 0 and "[resident init: 1," in err and "as expected without dos" in err, out + err)
rc, out, err = run(execver=37, ver=37)
check("Kickstart 2.04/2.05 (exec 37): the status says the command is resident from 3.0 on", rc == 0 and "Resident  no (Kickstart 3.0 and later)" in out, out)
rc, out, err = run("PJIT=pjit.lzt", "NOREBOOT", res=True)
h, data = slot(0x90000)
check("resident: PJIT= stores a bundle", rc == 0 and data == bundle and h[3] == 3, f"{h} {out}")

def rcs(err):
    return re.findall(r"\[rc (\d+)\]", err)


rc, out, err = run("BOGUS", "--", "STATUS", "--", "PJIT", "pjit.lzt", "VERIFY", "--", "PACE", "60", res=True)
check("resident, four runs on one copy: each reads its own arguments and nothing is freed twice",
      rcs(err) == ["10", "0", "0", "0"] and out.count("Next boot") == 1 and "Same as the update slot, #3" in out and "Pace 60 %" in out, out + err)
rc, out, err = run("PJIT=pjit.lzt", "VERIFY", "--", "BOGUS", "--", "STATUS")
check("the same for a program whose memory is not loaded again", rcs(err) == ["0", "10", "0"] and "Next boot" in out, out + err)

rc, out, err = run("PACE", "40", "PJIT=\"pjit.lzt\"", "VERIFY", ver=34)
check("Kickstart 1.3: the command's own reading of the template (KEY value, a quoted value)", rc == 0 and "Pace 40 %" in out and "Same as the update slot, #3" in out, out)
rc, out, err = run("?", ver=34)
check("Kickstart 1.3: ? shows the template", rc == 0 and out.startswith("PJIT/K,ROM/K,"), out)
rc, out, err = run("BOGUS", ver=34)
check("Kickstart 1.3: an unknown word is refused as ReadArgs refuses it", rc == 10 and "buffee: wrong number of arguments" in out, out)
rc, out, err = run("PACE=x", ver=34)
check("Kickstart 1.3: a /N value that is no number", rc == 10 and "buffee: bad number" in out, out)
rc, out, err = run("ROM", ver=34)
check("Kickstart 1.3: a keyword without its value", rc == 10 and "buffee: value after keyword missing" in out, out)
rc, out, err = run("NOREBOOT=1", ver=34)
check("Kickstart 1.3: a switch given a value", rc == 10 and "buffee: wrong number of arguments" in out, out)

lines = [l for o in printed for l in o.splitlines()]
wide = [l for l in lines if len(l) > 76 and not l.startswith("PJIT/K,")]
check("no line is wider than 76 columns (a Shell in topaz 8; the template excepted, which the Shell wraps)", not wide, "\n".join(wide[:5]))
stray = [l for l in lines if "%l" in l or "%z" in l]
check("no format is left unexpanded", not stray, "\n".join(stray[:5]))

print(f"{'all passed' if not failures else f'{failures} FAILED'}" + (f"  ({work})" if failures else ""))
if not failures:
    import shutil
    shutil.rmtree(work)
sys.exit(1 if failures else 0)
