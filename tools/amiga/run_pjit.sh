#!/bin/sh
# tools/amiga/run_pjit.sh <rom> <out.log> [extra -D flags]
# Builds tests/qemu/amiga_diff.c (PJIT's interpreter around the scripted Amiga model of tools/amiga/amiga.c) with the flags (SNAP, LIMIT, DENSE_FROM,
# TRACE_FROM / TRACE_TO) and runs it in QEMU with the ROM loaded at 0x50000000; the snapshots go to <out.log>, for tools/amiga/compare.py.
# e.g. tools/amiga/run_pjit.sh local/roms/Kickstart3.1.rom /tmp/pj.log -DSNAP=50000ull -DLIMIT=20000000ull
cd "$(dirname "$0")/../.." || exit 1
ROM=$1; OUT=$2; shift 2
make VARIANT=amiga EXTRA_CFLAGS="-Itools/amiga $*" qemu-amiga_diff 2>&1 | grep -E " error|Error"
qemu-system-arm -M virt -cpu cortex-a15 -m 512M -nographic -semihosting -kernel build/amiga/qemu/amiga_diff.elf \
    -device loader,file="$ROM",addr=0x50000000,force-raw=on > "$OUT" 2>&1
