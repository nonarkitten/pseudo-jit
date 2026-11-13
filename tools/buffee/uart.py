"""Serial-port helpers shared by the board tools in this directory.

The bootloader talks 115200 8N1 on the UART header (an FTDI USB-serial adapter); the AM335x boot ROM
and the bootloader's menu (2: run a bundle, 3: the flash images) take files over XMODEM-CRC.  The tools find the adapter
by themselves: `--port` (or the BUFFEE_PORT environment variable) wins, otherwise the only USB-serial
adapter present is used.
"""

import glob
import os
import sys

import serial
from xmodem import XMODEM

BAUD = 115200
PORT_HELP = "serial port (default: $BUFFEE_PORT, else the only USB-serial adapter found)"


def default_port():
    """$BUFFEE_PORT, else the only /dev/cu.usbserial-* (macOS) or /dev/ttyUSB* (Linux) present."""
    port = os.environ.get("BUFFEE_PORT")
    if port:
        return port
    found = sorted(glob.glob("/dev/cu.usbserial-*") + glob.glob("/dev/ttyUSB*"))
    if len(found) == 1:
        return found[0]
    if not found:
        sys.exit("no USB-serial adapter found: plug the FTDI cable in, or pass --port / set BUFFEE_PORT")
    sys.exit("several serial ports found (%s): pass --port or set BUFFEE_PORT" % ", ".join(found))


def add_port_argument(parser):
    """Adds --port to an argparse parser; resolve it with `args.port = args.port or default_port()`."""
    parser.add_argument("--port", default=None, help=PORT_HELP)


def open_port(port=None, timeout=0.05):
    """Opens the board's UART; says what to do if another program (a terminal emulator) holds it."""
    port = port or default_port()
    try:
        return serial.Serial(port, BAUD, timeout=timeout)
    except serial.SerialException as e:
        sys.exit(
            "cannot open %s: %s\n(another program has the port? close the terminal emulator and run this again)"
            % (port, e)
        )


class _Stream:
    """The bytes to send, with the read() method python-xmodem wants."""

    def __init__(self, data):
        self.data, self.pos = data, 0

    def read(self, n):
        chunk = self.data[self.pos : self.pos + n]
        self.pos += n
        return chunk


def xmodem_send(s, data, retry=16, timeout=30):
    """Sends `data` (bytes) over the open serial port `s` as XMODEM with 128-byte blocks (CRC when the
    receiver asks for it, which the boot ROM and the bootloader do).  True when everything was acknowledged.
    """

    def getc(n, timeout=1):
        s.timeout = timeout
        return s.read(n) or None

    def putc(d, timeout=1):
        s.write_timeout = timeout
        return s.write(d)

    return XMODEM(getc, putc, mode="xmodem").send(_Stream(data), retry=retry, timeout=timeout, quiet=True)
