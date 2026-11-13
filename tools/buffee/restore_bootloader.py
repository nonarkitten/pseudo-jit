#!/usr/bin/env python3
"""Puts the Buffee bootloader (`make firmware` writes it to build/firmware/bootloader.bin) on a board: a virgin one
(the AM335x ROM asks for an image: 'CCC...') or one that runs the pseudo-jit_1 firmware.
  restore_bootloader.py --yes-erase-boot-sector [--image PATH] [--port P]          (the UART must be free: close Serial.app first)

What it touches, in order (nothing else):
  1. looks at the menu: already the Buffee bootloader -> says so and stops; the pseudo-jit_1 menu ->
  continues; anything else -> stops.
  2. pseudo-jit_1 menu 3, 'Quick-test SPI flash' (that firmware's SPITest): erases sector 0 (the FIRST 4 KB of the
  SPI flash, i.e. the boot header of the pseudo-jit_1 image) and writes
     16 bytes of test pattern / zeros there. It is NOT menu S/1 (a chip erase: it would also wipe the stored
     Kickstart bundle at 0x20000 and the ROM image at 0x100000).
  3. 'X' (reboot): the AM335x ROM finds no valid image in the SPI flash and asks for one over the UART
  (xmodem-CRC, 'CCC...').
  4. xmodem of bootloader.bin into internal RAM; it starts and shows its menu.
  5. tools/buffee/update_boot.py: menu 3, 6 + xmodem of the same file; the bootloader programs it into the flash (4
  KB sectors up to the image length, then verifies).
Afterwards the EEPROM settings (config_t, CRC-checked) may need a look: menu 4 (the pseudo-jit_1 firmware used
another layout and may have reset it), and the GreenPAK RAM image is
loaded by the Buffee bootloader at every boot (the NVM still holds the pseudo-jit_1 'straight through' design;
it does not matter).
Recovery if a step fails: the board is then at the ROM's 'CCC' prompt (power-cycle it, or J-Link reset) -> run
this script again, steps 1-2 are skipped when the ROM asks for an image.
"""

import sys, time, argparse, subprocess, os
from uart import add_port_argument, default_port, open_port, xmodem_send
import serial

ap = argparse.ArgumentParser()
add_port_argument(ap)
ap.add_argument("--image", default="build/firmware/bootloader.bin")
ap.add_argument(
    "--yes-erase-boot-sector",
    action="store_true",
    help="required: confirms that sector 0 (4 KB) of the SPI flash may be erased",
)
a = ap.parse_args()
a.port = a.port or default_port()
data = open(a.image, "rb").read()
if len(data) > 0x1C000 or (data[3] != 0xEA and data[0] != 0xEA):
    print("warning: %s does not look like a bootloader image (first bytes %s)" % (a.image, data[:4].hex()))
try:
    s = open_port(a.port, timeout=0.2)
except serial.SerialException as e:
    sys.exit(
        "cannot open %s: %s\n(Serial.app or another terminal has the port: disconnect it and run this again)"
        % (a.port, e)
    )


def read_for(t):
    end = time.time() + t
    out = ""
    while time.time() < end:
        out += s.read(2000).decode("latin1")
    return out


def xmodem(buf):
    return xmodem_send(s, buf)


s.reset_input_buffer()
out = read_for(1.0)
if "C" * 3 not in out:  # not at the ROM prompt: look at the menu (a bare CR is harmless at a menu prompt)
    s.write(b"\r")
    out += read_for(1.0)
    if (
        "Exit to main menu" in out and "Set GPMC timing" in out
    ):  # the pseudo-jit_1 GPMC test submenu: X leaves it (only here: at the main menu X reboots)
        s.write(b"x\r")
        out += read_for(1.0)
    s.write(b"H\r")
    out += read_for(1.5)
    if ("Load PJIT bundle" in out and "Update the bootloader" in out) or ("Boot PJIT from the flash" in out and "Flash images" in out):
        sys.exit("This is already the Buffee bootloader. Nothing to do (update_boot.py replaces it with a newer one).")
    if "Quick-test SPI flash" not in out or "Manage SPI Flash" not in out:
        sys.exit("Unrecognised firmware (menu not seen). Last output: %r" % out[-300:])
    if not a.yes_erase_boot_sector:
        sys.exit(
            "pseudo-jit_1 firmware found. Re-run with --yes-erase-boot-sector to erase sector 0 of the SPI flash (see the docstring)."
        )
    print("pseudo-jit_1 menu found; erasing sector 0 with menu 3 (4 KB, not a chip erase)")
    s.write(b"3\r")
    out = read_for(1.0)
    if "Are you sure" not in out:
        sys.exit("no confirmation prompt: %r" % out[-200:])
    s.write(b"y\r")
    out = read_for(4.0)
    print(out.strip()[-400:])
    if "Erasing page 0" not in out:
        sys.exit("the SPI test did not run as expected")
    s.write(b"x\r")
    out = read_for(1.0)
    if "Are you sure" in out:
        s.write(b"y\r")
    print("rebooting; waiting for the ROM prompt (it may take up to ~10 s)")
t = time.time()
out = ""
while time.time() - t < 60 and "CCC" not in out:
    out += read_for(0.5)
if "CCC" not in out:
    sys.exit(
        "the ROM never asked for an image (output %r); power-cycle the board and run this again" % out[-200:]
    )
print("ROM asks for an image; sending %d bytes" % len(data))
if not xmodem(data):
    sys.exit("xmodem into RAM failed; power-cycle and run this again")
out = read_for(6.0)
print(out.strip()[-300:])
s.close()
print("bootloader runs from RAM; programming the flash with update_boot.py")
sys.exit(
    subprocess.call(
        [
            sys.executable,
            os.path.join(os.path.dirname(os.path.abspath(__file__)), "update_boot.py"),
            "--port",
            a.port,
            a.image,
            "--reboot",
        ]
    )
)
