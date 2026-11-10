#!/bin/sh
# Builds the PRU firmware with pru-gcc (gnupru; PRU_GCC_ROOT names its install directory, default ~/Dev/pru-toolchain/install) and writes it as C arrays to
# <dir>/src/pru_text.c and pru_data.c, which are checked in so that the bootloader builds without pru-gcc.
#   tools/buffee/pru_gcc.sh bootloader [main_gp4.c]
D=$(cd "${1:-bootloader}" && pwd) || exit 1
SRC=${2:-main_gp4.c}
export PATH=${PRU_GCC_ROOT:-$HOME/Dev/pru-toolchain/install}/bin:$PATH
T=$(mktemp -d) || exit 1
trap 'rm -rf "$T"' EXIT
pru-gcc -mmcu=am335x.pru0 -O2 "$D/pru/$SRC" -o "$T/pru.elf" 2>&1 | grep -E "error" && exit 1
pru-objcopy -O binary -j .text "$T/pru.elf" "$T/text.bin" && pru-objcopy -O binary -j .data "$T/pru.elf" "$T/data.bin" || exit 1
(cd "$T" && xxd -i -a text.bin | sed 's/text_bin/pru_text_bin/g' > "$D/src/pru_text.c" && xxd -i -a data.bin | sed 's/data_bin/pru_data_bin/g' > "$D/src/pru_data.c")
