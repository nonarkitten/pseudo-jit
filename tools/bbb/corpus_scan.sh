#!/bin/zsh
# Per-program cold and warm cycles of the 68000 corpus (tests/qemu/smoke_corpus.c on the board: tests/bbb/bbb_corpus.c) on the BeagleBone over several code layouts.
# One image per pad is built at once (build/$VARIANT/p<pad>); only the runs on the board are one after the other.
#   MAKEX="PIN_D0=1" VARIANT=cpin tools/bbb/corpus_scan.sh <label> <pad words...>
# prints "<label> pad N: <name> <cold> <warm>" lines.   Needs tests/data/corpus_data.h (tools/data/corpus.py).
setopt null_glob
cd "$(dirname "$0")/../.."
L=$1; shift
V=${VARIANT:-corpus}
J=${JOBS:-$(sysctl -n hw.ncpu 2>/dev/null || echo 4)}
export V MAKEX
build_one() {
  n=$1; mkdir -p "build/$V/p$n"; rm -f "build/$V/p$n/fail"
  make ${=MAKEX} VARIANT="$V/p$n" BBB_ASEXTRA="-Wa,--defsym,PJIT_PAD=$n" bbb-bbb_corpus >/dev/null 2>&1 || touch "build/$V/p$n/fail"
}
printf '%s\n' "$@" | xargs -P "$J" -L1 zsh -c "$(functions build_one); build_one \$0"
for n in "$@"; do
  B2="build/$V/p$n"
  [ -f "$B2/fail" ] && { echo "$L pad $n: build failed"; continue; }
  for try in 1 2 3; do
    out=$(python3 -u tools/bbb/run.py "$B2"/bbb/bbb_corpus.bin 2>&1)
    echo "$out" | grep -q "total warm" && break
  done
  echo "$out" | grep -E '^\[CORPUS\] [a-z0-9-]+ +size' | awk -v l="$L" -v n="$n" '{print l, "pad", n ":", $2, $6, $8, $10, $12, $14}'
  echo "$out" | grep -E 'total warm|\[TEST\]|FAIL' | sed "s/^/$L pad $n: /"
done
