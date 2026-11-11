#!/bin/sh
# Fetches the upstream sources the licence audit compares with into local/corpora/src/ (git-ignored) and indexes them (local/corpora/*.npy).
#   tools/audit/fetch_corpora.sh        (about 200 MB, mostly TI StarterWare; needs git, curl and the .venv with numpy: `make venv`)
# Then:  git ls-files -co --exclude-standard | .venv/bin/python tools/audit/shingle.py scan -        and        .venv/bin/python tools/audit/shingle.py blobs
set -e
cd "$(dirname "$0")/../.." || exit 1
PY="${PYTHON:-.venv/bin/python}"
SRC=local/corpora/src
mkdir -p "$SRC"
get() { [ -d "$SRC/$1" ] || git clone -q --depth 1 "$2" "$SRC/$1"; }

get starterware https://github.com/kryochronic/AM335X_StarterWare_02_00_01_01         # TI's download, "no modifications"
get cyclone https://github.com/notaz/cyclone68000                                      # Cyclone 68K (ARM assembler 68000 core)
get castaway https://github.com/Apaczer/DCaSTaway                                      # a fork of Castaway; src/m68k/castaway is its 68000 core
get libbbb https://gitlab.forge.hefr.ch/embsys/libbbb.git                              # Beaglebone Black system library, Apache 2.0
get mpaland https://github.com/mpaland/printf                                           # a tiny printf, MIT
if [ ! -d "$SRC/emu68_a32" ]; then                                                      # Emu68: its last AArch32 revision, the tree of June 2020, and today's
  git clone -q --bare https://github.com/michalsc/Emu68.git "$SRC/emu68.git"
  for pair in emu68_a32:4a09f1646f4eaf96e3b55c0f3587d1ad13fbfadf emu68_2020:036f292184c3130b067f732c52fac1faf4fede79 emu68_head:HEAD; do
    mkdir -p "$SRC/${pair%%:*}" && git --git-dir="$SRC/emu68.git" archive "${pair##*:}" | tar -x -C "$SRC/${pair%%:*}"
  done
fi
if [ ! -d "$SRC/uboot" ]; then                                                          # U-Boot: the directories AM33xx bootloader code usually descends from
  git clone -q --depth 1 --filter=blob:none --sparse https://github.com/u-boot/u-boot.git "$SRC/uboot"
  (cd "$SRC/uboot" && git sparse-checkout init --no-cone && git sparse-checkout set --no-cone '/arch/arm/mach-omap2/' '/arch/arm/include/asm/arch-am33xx/' \
     '/arch/arm/include/asm/arch-omap3/' '/board/ti/am335x/' '/drivers/i2c/omap24xx_i2c.c' '/drivers/spi/omap3_spi.c' '/drivers/serial/serial_omap.c' \
     '/drivers/gpio/omap_gpio.c' '/drivers/mtd/spi/' '/lib/string.c' '/lib/vsprintf.c' '/lib/tiny-printf.c' '/arch/arm/lib/' '/arch/arm/cpu/armv7/' \
     '/drivers/memory/' '/drivers/timer/' && git checkout -q)
fi
if [ ! -d "$SRC/menie" ]; then                                                          # Georges Menie's tiny printf (LGPL 2+) and xmodem (BSD 3-clause)
  mkdir -p "$SRC/menie"
  for f in printf-stdarg.c printf.c xmodem.c; do curl -sS -o "$SRC/menie/$f" "https://www.menie.org/georges/embedded/$f"; done
fi

"$PY" tools/audit/shingle.py index starterware "$SRC/starterware"
"$PY" tools/audit/shingle.py index cyclone "$SRC/cyclone"
"$PY" tools/audit/shingle.py index castaway "$SRC/castaway/src/m68k/castaway" "$SRC/castaway/src/st" "$SRC/castaway/src/config.h"
"$PY" tools/audit/shingle.py index libbbb "$SRC/libbbb"
"$PY" tools/audit/shingle.py index mpaland "$SRC/mpaland"
"$PY" tools/audit/shingle.py index emu68 "$SRC/emu68_a32" "$SRC/emu68_2020" "$SRC/emu68_head"
"$PY" tools/audit/shingle.py index uboot "$SRC/uboot"
"$PY" tools/audit/shingle.py index menie "$SRC/menie"
