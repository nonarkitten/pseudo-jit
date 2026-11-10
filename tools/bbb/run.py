#!/usr/bin/env python3
"""Run a bare-metal image on the BeagleBone that has Debian on its SD card, over the UART and ssh (see
platform/beaglebone/README.md).

  tools/bbb/run.py build/bbb/bbb_dhrystone.bin [more.bin ...]      # each image in turn; prints the console of every run

For each image: get the board into Linux if it is waiting in U-Boot, scp the image to the FAT partition
(/boot/firmware),
reboot, interrupt U-Boot's autoboot over the serial port, `fatload` and `go 0x82000000`, and stream the
console until the
image returns to U-Boot (or BBB_TIMEOUT seconds pass). The board is left at the U-Boot prompt.

Environment: BBB_SERIAL (default /dev/cu.usbserial-ABBGCTNI), BBB_HOST (ssh host, default `bbb`), BBB_TIMEOUT.
The board needs passwordless sudo for `reboot`."""

import os, sys, termios, time, select, subprocess

DEV = os.environ.get("BBB_SERIAL", "/dev/cu.usbserial-ABBGCTNI")
HOST = os.environ.get("BBB_HOST", "bbb")
TIMEOUT = float(os.environ.get("BBB_TIMEOUT", "300"))

fd = os.open(DEV, os.O_RDWR | os.O_NOCTTY | os.O_NONBLOCK)
a = termios.tcgetattr(fd)
a[0] = 0
a[1] = 0
a[2] = termios.CS8 | termios.CREAD | termios.CLOCAL
a[3] = 0
a[4] = a[5] = termios.B115200
a[6][termios.VMIN] = 0
a[6][termios.VTIME] = 0
termios.tcsetattr(fd, termios.TCSANOW, a)

buf = b""


def pump(t, echo=False):
    global buf
    end = time.time() + t
    while time.time() < end:
        if select.select([fd], [], [], 0.05)[0]:
            d = os.read(fd, 4096)
            if d:
                buf += d
                if echo:
                    sys.stdout.write(d.decode("latin1").replace("\r", ""))
                    sys.stdout.flush()


def wait_for(pat, timeout, spam=None, echo=False):
    start = len(buf)
    end = time.time() + timeout
    while time.time() < end:
        if spam:
            os.write(fd, spam)
        pump(0.1, echo)
        if pat in buf[start:]:
            return True
    return False


def ssh(cmd, **kw):
    return subprocess.run(
        ["ssh", "-o", "BatchMode=yes", "-o", "ConnectTimeout=5", HOST, cmd],
        capture_output=True,
        text=True,
        **kw,
    )


def ssh_up():
    return ssh("true").returncode == 0


def drain(quiet=2.0, limit=60):
    """Read until the port has been silent for `quiet` seconds (the OS keeps what the board printed while
    nobody was listening).
    """
    end = time.time() + limit
    last = time.time()
    while time.time() < end and time.time() - last < quiet:
        n = len(buf)
        pump(0.2)
        if len(buf) != n:
            last = time.time()


def to_linux():
    if ssh_up():
        return
    # not in Linux: if U-Boot is at its prompt (possibly after a run that is still printing), its answer to a
    # unique echo comes back
    # followed by the prompt; the old output queued on the port cannot contain that
    os.write(fd, b"\recho @@MARK\r")
    if wait_for(b"@@MARK\r\n=> ", 300):
        os.write(fd, b"boot\r")
        print("[bbb_run] U-Boot at its prompt: sent boot", flush=True)
    for _ in range(120):
        if ssh_up():
            return
        pump(2)
    sys.exit("board did not come up in Linux")


def run(image):
    name = os.path.basename(image)
    to_linux()
    r = subprocess.run(["scp", "-q", image, f"{HOST}:/boot/firmware/pjit.bin"])
    if r.returncode:
        sys.exit("scp failed")
    ssh("sync")
    start = len(buf)
    ssh("sudo -n reboot")  # needs `renee ALL=(ALL) NOPASSWD: ALL` in /etc/sudoers.d on the board
    if not wait_for(b"=> ", 150, b" \r"):
        sys.exit("no U-Boot prompt")
    print(f"===== {name} =====")
    os.write(fd, b"fatload mmc 0:1 0x82000000 pjit.bin\r")
    wait_for(b"=> ", 20)
    mark = len(buf)
    os.write(fd, b"go 0x82000000\r")
    wait_for(b"terminated, rc", TIMEOUT, echo=True)
    pump(1, echo=True)


if len(sys.argv) < 2:
    sys.exit(__doc__)
for img in sys.argv[1:]:
    run(img)
