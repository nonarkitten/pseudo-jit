# ======================================================================================================================================
#  PJIT / Buffee -- the top-level build.     `make help` lists the targets.
#
#  PJIT is a 68000 -> ARM32 translator (pjit/).  Buffee is the OSD335x board that runs it inside an Amiga.  This Makefile builds
#  the translator for QEMU (the tests), for the BeagleBone Black (benchmarks) and for Buffee (payload bundles), the Buffee bootloader,
#  and the static tables the translator runs from.   The pieces live in mk/:
#
#      mk/pjit.mk    the translator core: its sources, flags and switches
#      mk/gen.mk     the table generator (tools/gen), its checks and the host-side oracle
#      mk/qemu.mk    the tests that run under qemu-system-arm, the Cortex-A8 core model, golden/ (Ted Fried's suite)
#      mk/board.mk   BeagleBone images, Buffee payloads and bundles, the bootloader, `make firmware`
#
#  Everything is written to build/<VARIANT>/ (git-ignored).  A VARIANT is one build configuration; give every configuration that
#  changes the generated tables its own name (`make VARIANT=lazy X_LAZY=1 test`).  Changed compiler flags are noticed inside a
#  variant (flag stamps), so `make BLK_CEXTRA=-DFOO bbb-bbb_dhrystone` rebuilds what the flag touches.
#
#  Needs GNU make 3.81 or newer, arm-none-eabi-gcc and qemu-system-arm (`make toolchain-check`), and a Python virtual environment for
#  the table generator (`make venv`).
# ======================================================================================================================================

# ---- configuration ---------------------------------------------------------------------------------------------------------------
# Goals that build for the Buffee board use the function-code GPIO tables (FC=1), which differ from the QEMU ones: their own variant.
BOARD_GOALS := $(filter buffee-% bundle-%,$(MAKECMDGOALS))
ifneq ($(BOARD_GOALS),)
VARIANT ?= buffee
endif

VARIANT ?= default
BUILD   ?= build/$(VARIANT)

# the translator's build switches (docs/design/TIERING.md, docs/design/BLOCK_RUNTIME.md)
PIN_D0 ?= 1
X_LAZY ?= 0
BLK    ?= 0
TIER   ?= $(if $(filter 1,$(BLK)),0,1)
TIER_T ?= 64
TIER_EXTRA ?=
COMPACT ?= $(if $(filter 1,$(PIN_D0)),0,1)
BLK_LIVE ?= 0
BLK_FWD ?= 1
BLK_INLINE_RTS ?= 1
BLK_INLINE_GOTO ?= 0
BLK_FOLD ?= 1
BLK_ALIGN ?= 16
BLK_SCHED ?= 1
BLK_SCHED_MAX ?= 1000000
BLK_SCHED_MIN ?= 0
BLK_SCHED_LOG ?= 0
BLK_PROF ?= 0
#   PIN_D0    1: D0 lives in r4 in every mode and tier (0: r4 is a second link register)
#   X_LAZY    1: pure X producers do not save X (a speed / exactness trade; off)
#   FC        1: every SR write updates the FC2 (supervisor) GPIO -- Buffee.  Set by the Buffee goals; FC_KNOB is the value in use
#   BLK       1: the block runtime alone (pjit/blk.s).   TIER 1 (default): slot cache first, hot code promoted to blocks.  Both 0: slots only
#   TIER_T    taken-transfer events at one target before it is promoted to a block (1..128)
#   BLK_*     block-runtime switches, each measured on the board; the defaults are the winners (docs/design/BLOCK_RUNTIME.md)

EXTRA_CFLAGS ?=
BLK_CEXTRA ?=
BBB_ASEXTRA ?=
ASX ?=
#   EXTRA_CFLAGS  more C flags for the QEMU build, e.g. EXTRA_CFLAGS=-DONLY=3 (tests/qemu/smoke_corpus.c)
#   BLK_CEXTRA    more C flags for the board builds, e.g. -DBLK_RING_KB=32, or the Kickstart payload's -DCLEAN -DDRT=0x1000u -DPACE=100
#   BBB_ASEXTRA   more assembler switches for the board builds, e.g. -Wa,--defsym,PJIT_PAD=160 (tools/bbb/scan.sh); ASX is the same
#                 variable under the name the Kickstart payload recipes use

# make's built-in FC is the Fortran compiler: the knob only counts when you set it (command line or environment); Buffee goals set it by default
FC_KNOB := $(if $(filter command line environment,$(origin FC)),$(FC),$(if $(BOARD_GOALS),1,0))

JOBS ?= $(shell sysctl -n hw.ncpu 2>/dev/null || nproc 2>/dev/null || echo 4)

ifeq ($(BLK)$(TIER),11)
$(error BLK=1 and TIER=1 are different builds)
endif

# ---- toolchain -------------------------------------------------------------------------------------------------------------------
CROSS    ?= arm-none-eabi-
CC        = $(CROSS)gcc
LD        = $(CROSS)ld
OBJCOPY   = $(CROSS)objcopy
OBJDUMP   = $(CROSS)objdump
NM        = $(CROSS)nm
READELF   = $(CROSS)readelf
QEMU     ?= qemu-system-arm
PY       ?= $(if $(wildcard .venv/bin/python),.venv/bin/python,python3)

# ---- where things go -------------------------------------------------------------------------------------------------------------
GEN     := $(BUILD)/gen
HOST    := $(BUILD)/host
QEMUDIR := $(BUILD)/qemu
BBB     := $(BUILD)/bbb
BUNDLES := $(BUILD)/buffee
#   gen/     generated sources: asm_offsets.inc, tables.s, btables.s, listings
#   host/    host-side helpers of the generator checks: the 68000 oracle, helpers.bin
#   qemu/    Cortex-A15 objects and test ELFs for qemu-system-arm -M virt
#   bbb/     Cortex-A8 objects and images (BeagleBone and Buffee payloads)
#   buffee/  Buffee payload bundles (.lzt)

.DEFAULT_GOAL := all
.SECONDARY:
.PHONY: all test check help clean distclean venv toolchain-check FORCE

# a recipe that fails (the table generator without its Python packages, say) must not leave an empty or partial target that the next make takes for finished work
.DELETE_ON_ERROR:
FORCE:

include mk/pjit.mk
include mk/gen.mk
include mk/qemu.mk
include mk/board.mk

# ---- the usual ---------------------------------------------------------------------------------------------------------------------
all: toolchain-check $(QEMU_ELFS)

check: test check-gen golden

toolchain-check:
	@command -v $(CC) >/dev/null 2>&1 || { echo "error: $(CC) not found.  macOS: brew install arm-none-eabi-gcc"; exit 1; }
	@command -v $(QEMU) >/dev/null 2>&1 || { echo "error: $(QEMU) not found.  macOS: brew install qemu"; exit 1; }

venv:
	python3 -m venv .venv
	.venv/bin/pip install -r requirements.txt

clean:
	rm -rf $(BUILD)

distclean:
	rm -rf build

help:
	@echo "PJIT / Buffee build.   make <target> [VARIANT=name] [KNOB=value ...]       output: build/<VARIANT>/ (now: $(BUILD))"
	@echo
	@echo "  QEMU tests         all   test   check (= test + check-gen + golden)   qemu-<test> (build one)   run-<test> (build and run one)"
	@echo "  generator          tables   listing   check-asm   check-gen"
	@echo "  golden/            golden   golden-compare"
	@echo "  Cortex-A8 model    a8-test"
	@echo "  BeagleBone         bbb-<test>   e.g. bbb-bbb_dhrystone -> $(BBB)/bbb_dhrystone.bin      bbb-all"
	@echo "  Buffee payloads    buffee-<test> (ELF)   bundle-<test> (.lzt for the bootloader's menu 2 / flash slot)   e.g. bundle-boot"
	@echo "  Buffee firmware    bootloader [BL=asn|asn_cal]               firmware = the bootloader + the standard Kickstart bundle"
	@echo "  housekeeping       venv   toolchain-check   clean   distclean"
	@echo
	@echo "  Knobs: VARIANT PIN_D0 X_LAZY FC BLK TIER TIER_T BLK_* EXTRA_CFLAGS BLK_CEXTRA BBB_ASEXTRA      (documented at the top of this file)"
	@echo "  Docs:  docs/README.md    docs/building.md    docs/getting-started.md"
