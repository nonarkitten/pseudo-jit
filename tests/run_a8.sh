#!/usr/bin/env bash
# Runs the smoke tests that `make a8-test` has built for QEMU's Cortex-A8 core model (realview-pb-a8):    tests/run_a8.sh <elf directory> <test>...
#
# The target's real instruction set: the A15 that `make test` uses would silently accept a udiv / sdiv the Cortex-A8 cannot execute.
# Not every test can run here (the interrupt and timer tests need the GIC / generic timer), which is why the list is short.
ELFDIR="${1:?usage: tests/run_a8.sh <elf directory> <test>...}"; shift
QEMU="${QEMU:-qemu-system-arm}"
pass=0; fail=0
for t in "$@"; do
    out=$(python3 - "$QEMU" "$ELFDIR/$t.elf" <<'P'
import subprocess, sys
qemu, elf = sys.argv[1], sys.argv[2]
try:
    r = subprocess.run([qemu, "-M", "realview-pb-a8", "-m", "256M", "-nographic", "-semihosting", "-kernel", elf], capture_output=True, text=True, timeout=120)
    print(r.stdout)
except subprocess.TimeoutExpired:
    print("TIMEOUT")
P
)
    if echo "$out" | tr -d '\r' | grep -q '^PASS$' && ! echo "$out" | grep -qE 'FAIL|MISMATCH'; then
        echo "== $t: PASS (Cortex-A8)"; pass=$((pass + 1))
    else
        echo "== $t: FAIL"; echo "$out" | tail -5; fail=$((fail + 1))
    fi
done
echo "== summary (Cortex-A8 core model): $pass passed, $fail failed"
[ "$fail" -eq 0 ]
