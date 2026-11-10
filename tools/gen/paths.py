"""Where the build puts the files the generator and its checks read and write.

`make` runs every Python tool with PJIT_BUILD set to the build directory of the active variant
(build/<variant>/); run by hand, the
default is build/default.  Layout (see the top of the Makefile):

    <build>/gen/    asm_offsets.inc, tables.s, btables.s, listing/    written by the generator
    <build>/host/   oracle1, helpers.bin, helpers.sym, flagmeta.json  host-side support for the checks
"""

import os

BUILD = os.environ.get("PJIT_BUILD", "build/default")
GEN = os.path.join(BUILD, "gen")
HOST = os.path.join(BUILD, "host")


def build_path(*parts):
    """A file below the active build directory, e.g. build_path('host', 'oracle1')."""
    return os.path.join(BUILD, *parts)


def asm_offsets_path():
    """The cpu_t offsets the assembler constants come from (written by `make`, see mk/gen.mk)."""
    return os.path.join(GEN, "asm_offsets.inc")
