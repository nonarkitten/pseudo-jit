# ======================================================================================================================================
#  The translator core: what every image links and the switches that shape it.     Included by the top-level Makefile, first.
# ======================================================================================================================================

# ---- sources ---------------------------------------------------------------------------------------------------------------------
# The static tables (DT, OT, DRSR, ...) are $(GEN)/tables.s, written by tools/gen (mk/gen.mk) and assembled per platform.
PJIT_C_SRCS = pjit/ext_table.c pjit/cache.c pjit/pjit.c
PJIT_S_SRCS = pjit/handle_bcd.s pjit/handle_divw.s pjit/handle_movem.s pjit/handle_roxd.s pjit/runtime.s pjit/runtime_dr.s pjit/bcache.s pjit/lzt.s
# BLK=1: the block runtime (pjit/blk.s, docs/design/BLOCK_RUNTIME.md) replaces the slot cache (runtime_dr.s)
ifeq ($(BLK),1)
PJIT_S_SRCS := $(filter-out pjit/runtime_dr.s,$(PJIT_S_SRCS)) pjit/blk.s pjit/blksched.s
PJIT_C_SRCS += pjit/blkstate.c
endif
# TIER=1 (the default): both runtimes in one image -- the slot cache first, hot code promoted to blocks (docs/design/TIERING.md)
ifeq ($(TIER),1)
PJIT_S_SRCS += pjit/blk.s pjit/blksched.s pjit/tier.s
PJIT_C_SRCS += pjit/blkstate.c
endif
# the block translator's tables are linked whenever blocks are
HAS_BLOCKS = $(filter-out 00,$(BLK)$(TIER))

PLATFORM_SRCS     = platform/pjit_libc.c platform/mmu_stub.c platform/arm_cache.c
PLATFORM_ASM_SRCS = platform/cp15.S platform/mem_be.S platform/timer.S platform/bogo_delay.S

# ---- flags -----------------------------------------------------------------------------------------------------------------------
# -ffreestanding / -nostdlib: there is no libc (platform/pjit_libc.*) and the translator assumes no host OS.
INCLUDES = -Ipjit -Iplatform -Itests/include -Itests/data -Itests/qemu -Itests/bbb -Itests/buffee
CORE_DEFS = $(if $(filter 1,$(X_LAZY)),-DPJIT_X_LAZY) $(if $(filter 1,$(PIN_D0)),-DPJIT_PIN_D0) \
            $(if $(filter 1,$(BLK)),-DPJIT_BLK) $(if $(filter 1,$(TIER)),-DPJIT_TIER)

# the block runtime's switches reach the assembler as symbols (pjit/blk.s)
BLK_ASM_OPTS = -Wa,--defsym,PJIT_BLK_LIVE=$(BLK_LIVE) -Wa,--defsym,PJIT_BLK_FWD=$(BLK_FWD) -Wa,--defsym,PJIT_BLK_INLINE_RTS=$(BLK_INLINE_RTS) \
               -Wa,--defsym,PJIT_BLK_INLINE_GOTO=$(BLK_INLINE_GOTO) -Wa,--defsym,PJIT_BLK_FOLD=$(BLK_FOLD) -Wa,--defsym,PJIT_BLK_SCHED=$(BLK_SCHED) \
               -Wa,--defsym,PJIT_BLK_SCHED_MAX=$(BLK_SCHED_MAX) -Wa,--defsym,PJIT_BLK_SCHED_MIN=$(BLK_SCHED_MIN) \
               -Wa,--defsym,PJIT_BLK_SCHED_LOG=$(BLK_SCHED_LOG) -Wa,--defsym,PJIT_BLK_PROF=$(BLK_PROF) -Wa,--defsym,PJIT_BC_ALIGN=$(BLK_ALIGN)
BLK_ASM  = -Wa,--defsym,PJIT_BLK=1 -Wa,--defsym,PJIT_LATCH=1 $(BLK_ASM_OPTS)
TIER_ASM = -Wa,--defsym,PJIT_TIER=1 -Wa,--defsym,PJIT_LATCH=1 -Wa,--defsym,PJIT_TIER_T=$(TIER_T) $(TIER_EXTRA) $(BLK_ASM_OPTS)
PIN_ASM  = -Wa,--defsym,PJIT_PIN_D0=1
CORE_ASM = -Wa,--defsym,PJIT_COMPACT=$(COMPACT) $(if $(filter 1,$(PIN_D0)),$(PIN_ASM)) \
           $(if $(filter 1,$(BLK)),$(BLK_ASM)) $(if $(filter 1,$(TIER)),$(TIER_ASM))
# Buffee payloads (FC=1): the function-code GPIO, the IPL GPIO the interrupt path re-reads, the exception log
# (a comma inside $(if ...) would split its arguments: the switches live in a variable)
FC_ASM   = -Wa,--defsym,PJIT_IPL_GPIO=0x481AC138 -Wa,--defsym,PJIT_DLOG=1 -Wa,--defsym,PJIT_FC=1
BUFFEE_ASM = $(if $(filter 1,$(FC_KNOB)),$(FC_ASM))

LDFLAGS = -nostdlib -Wl,--build-id=none

# $(call stamp,<file>,<text>): write <file> only when its contents change, so whatever depends on it is rebuilt only then.
# (FORCE makes the recipe run every time; the unchanged mtime tells make nothing happened.)
define stamp
	@echo '$(2)' | cmp -s - $(1) || echo '$(2)' > $(1)
endef

# ---- flag stamps -----------------------------------------------------------------------------------------------------------------
# make does not see -D flags: each object family depends on a file that holds its flags and is rewritten only when they change.
QEMU_STAMP = $(BUILD)/.qemu.flags
BBB_STAMP  = $(BUILD)/.bbb.flags
$(BUILD):
	@mkdir -p $@
