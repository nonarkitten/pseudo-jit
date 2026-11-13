# ======================================================================================================================================
#  The boards:  BeagleBone Black images (cycle-accurate benchmarks on a Cortex-A8), Buffee payloads and the bundles the bootloader loads,
#  the Buffee bootloader itself, and `make firmware`.   Also the Cortex-A8 core model, which needs the same objects.   Included by the Makefile.
#
#  The BeagleBone (AM335x) and Buffee (OSD335x) are the same core: one set of objects, two board layers (platform/beaglebone,
#  platform/buffee).  Buffee payloads are built with FC=1 (their own VARIANT, `buffee`, unless you name one).
# ======================================================================================================================================

BBB_INCLUDES = $(INCLUDES) -Iplatform/beaglebone -Iplatform/buffee -Igolden
BBB_CFLAGS   = -mcpu=cortex-a8 -marm -ffreestanding -nostdlib -O1 -g -Wall $(BBB_INCLUDES) $(CORE_DEFS) $(BLK_CEXTRA)
BBB_ASFLAGS  = -mcpu=cortex-a8 -marm -ffreestanding -nostdlib -g -Ipjit -Iplatform -Iplatform/beaglebone -Iplatform/buffee -I$(GEN) \
               -Wa,--defsym,PJIT_A8=1 -Wa,--defsym,PJIT_A8_HZ=1000000000 $(CORE_ASM) $(BUFFEE_ASM) $(BBB_ASEXTRA) $(ASX)
BBB_LDFLAGS  = -nostdlib -Wl,--build-id=none
BBB_LD      ?= platform/beaglebone/bbb.ld

BO = $(BBB)/o

$(BBB_STAMP): FORCE | $(BUILD)
	$(call stamp,$@,$(BBB_CFLAGS) | $(BBB_ASFLAGS))

# ---- objects ---------------------------------------------------------------------------------------------------------------------
$(BO)/tests/%.o: %.c $(BBB_STAMP)
	@mkdir -p $(dir $@)
	$(CC) $(BBB_CFLAGS) -MMD -MP -c $< -o $@
$(BO)/%.o: %.c $(BBB_STAMP)
	@mkdir -p $(dir $@)
	$(CC) $(BBB_CFLAGS) -MMD -MP -c $< -o $@
$(BO)/%.o: %.s $(GEN)/asm_offsets.inc $(BBB_STAMP)
	@mkdir -p $(dir $@)
	$(CC) $(BBB_ASFLAGS) -Wa,-MD,$(@:.o=.d) -c $< -o $@
$(BO)/%.o: %.S $(GEN)/asm_offsets.inc $(BBB_STAMP)
	@mkdir -p $(dir $@)
	$(CC) $(BBB_ASFLAGS) -MMD -MP -c $< -o $@
$(BO)/tables.o: $(GEN)/tables.s $(BBB_STAMP)
	@mkdir -p $(dir $@)
	$(CC) $(BBB_ASFLAGS) -c $< -o $@
$(BO)/btables.o: $(GEN)/btables.s $(BBB_STAMP)
	@mkdir -p $(dir $@)
	$(CC) $(BBB_ASFLAGS) -c $< -o $@

# the order below is the link order (it decides where code lands, so it is kept stable)
BBB_CORE_OBJS = $(patsubst %.c,$(BO)/%.o,$(PJIT_C_SRCS)) $(patsubst %.s,$(BO)/%.o,$(PJIT_S_SRCS)) \
                $(BO)/platform/pjit_libc.o $(BO)/platform/mmu_stub.o $(BO)/platform/arm_cache.o $(BO)/platform/cp15.o $(BO)/platform/mem_be.o
BBB_TABLE_OBJS = $(BO)/tables.o $(if $(HAS_BLOCKS),$(BO)/btables.o)
BBB_OBJS    = $(BBB_CORE_OBJS) $(BO)/platform/beaglebone/boot.o $(BO)/platform/beaglebone/bbb.o $(BBB_TABLE_OBJS)
BUFFEE_OBJS = $(BBB_CORE_OBJS) $(BBB_TABLE_OBJS) $(BO)/platform/buffee/buffee.o $(BO)/platform/buffee/boot.o

# ONBUFFEE=1: the BeagleBone benchmarks (tests/bbb) linked with the Buffee board layer instead, to run on a Buffee (make ONBUFFEE=1 VARIANT=onbuffee bundle-bbb_dhrystone)
ifeq ($(ONBUFFEE),1)
BBB_OBJS    = $(BUFFEE_OBJS) $(BO)/platform/buffee/onbuffee.o
BBB_LD      = platform/buffee/bench_dram.ld
BBB_LDFLAGS += -Wl,--wrap=pjit_cache_init
BBB_CFLAGS  += -DON_BUFFEE
endif
# the bundles of all the benchmarks: make ONBUFFEE=1 VARIANT=onbuffee onbuffee-all, then tools/buffee/run_bench.py build/onbuffee/buffee
onbuffee-all: $(patsubst %,$(BUNDLES)/%.lzt,$(BBB_TESTS))

-include $(BBB_OBJS:.o=.d) $(BUFFEE_OBJS:.o=.d)

# ---- BeagleBone images -----------------------------------------------------------------------------------------------------------
# `make bbb-bbb_dhrystone` -> $(BBB)/bbb_dhrystone.bin; U-Boot loads it at 0x82000000 and runs it (tools/bbb/run.py, platform/beaglebone/README.md).
# Everything is built for -mcpu=cortex-a8: QEMU's A15 would accept a udiv the board cannot execute.
BBB_TESTS = bbb_dhrystone bbb_bogomips bbb_micro bbb_branch bbb_opcost bbb_tinybasic bbb_cadence bbb_wcet bbb_claimcost bbb_window bbb_bcache \
	bbb_mispred bbb_tbsample bbb_tbcycles bbb_blocks bbb_blocks_cf bbb_blocks_sched bbb_dhcycles bbb_irq bbb_chain bbb_chain_prof bbb_lzt

$(BBB)/%.elf: $(BO)/tests/%.o $(BBB_OBJS) $(BBB_LD)
	$(CC) $(BBB_CFLAGS) $(BBB_LDFLAGS) -T $(BBB_LD) $(BBB_OBJS) $< -o $@ -lgcc
	$(OBJDUMP) -d $@ > $(BBB)/$*.lst
$(BBB)/%.bin: $(BBB)/%.elf
	$(OBJCOPY) -O binary $< $@

# bbb_lzt decodes a bundle of the real tables. The bundle is packed here from the tables of another image of the same build (660 KB of C: made when wanted,
# never kept in the tree); a build without the block runtime packs without the block translator's tables.
$(BBB)/lzt_bundle.h: $(BBB)/bbb_dhrystone.elf tools/pack/lzt_elf.py tools/pack/lzt.py
	python3 tools/pack/lzt_elf.py $< $(BBB)/lzt_bundle.lzt --rel $(if $(HAS_BLOCKS),,--interp) --header $@
$(BO)/tests/bbb_lzt.o: $(BBB)/lzt_bundle.h
$(BO)/tests/bbb_lzt.o: BBB_CFLAGS += -I$(BBB)

bbb-all: $(patsubst %,$(BBB)/%.bin,$(BBB_TESTS))
bbb-%: $(BBB)/%.bin
	@ls -l $<

# ---- Buffee payloads and bundles -------------------------------------------------------------------------------------------------
# `make bundle-boot` -> $(BUNDLES)/buffee_boot.lzt: the LZTB bundle the bootloader's menu 2 receives over xmodem (or its flash slot
# stores; docs/buffee/BUFFEE_MAP.md).   `make buffee-hello` stops at the ELF.   Tests: tests/buffee/.
BUFFEE_TESTS = buffee_chips buffee_floppy buffee_fc buffee_weight buffee_shotgun buffee_cia4 buffee_top buffee_endian buffee_hello \
	buffee_boot buffee_fried buffee_blit buffee_bus buffee_reroll buffee_speed buffee_peekrom buffee_smc buffee_abort buffee_regtest \
	buffee_blitdur buffee_chipfull buffee_crumbs buffee_kspeed buffee_gpreset buffee_boardkind

$(patsubst %,$(BBB)/%.elf,$(BUFFEE_TESTS)): $(BBB)/%.elf: $(BO)/tests/%.o $(BUFFEE_OBJS) platform/buffee/pjit_dram.ld
	$(CC) $(BBB_CFLAGS) $(BBB_LDFLAGS) -T platform/buffee/pjit_dram.ld $(BUFFEE_OBJS) $< -o $@ -lgcc
	$(OBJDUMP) -d $@ > $(BBB)/$*.lst

$(BUNDLES)/%.lzt: $(BBB)/%.elf | $(BUNDLES)
	python3 tools/pack/lzt_elf.py $< $@ --image

buffee-%: $(BBB)/buffee_%.elf
	@ls -l $<
bundle-%: $(BUNDLES)/buffee_%.lzt
	@ls -l $<

# ---- the Cortex-A8 core model ----------------------------------------------------------------------------------------------------
# QEMU's A15 accepts udiv / sdiv the Cortex-A8 cannot execute.  These smoke tests are built for the A8 (the BeagleBone objects) and run
# on QEMU's realview-pb-a8 model with the board glue of platform/qemu_a8.  Not every test can: the interrupt and timer tests need the GIC.
A8DIR = $(BUILD)/a8
A8_TESTS = smoke_run smoke_run_dr smoke_xmodes_interp smoke_xmodes_dr smoke_exception_interp smoke_exception_dr smoke_control_interp \
	smoke_control_dr smoke_alias_interp smoke_alias_dr smoke_index smoke_index_dr smoke_links smoke_rts smoke_bcd_exec \
	smoke_mul_exec smoke_ea_addr_exec smoke_ea_alu_interp smoke_ea_alu_dr smoke_div
A8_GLUE_FLAGS = -mcpu=cortex-a8 -marm -ffreestanding -nostdlib -g -Ipjit -Iplatform -I$(GEN) -Wa,--defsym,PJIT_A8=1 -Wa,--defsym,PJIT_A8_HZ=1000000000
A8_GLUE = $(A8DIR)/boot.o $(A8DIR)/semi.o $(A8DIR)/console.o $(A8DIR)/irq.o
$(A8DIR)/boot.o: platform/qemu_virt/boot.S | $(A8DIR)
	$(CC) $(A8_GLUE_FLAGS) -c $< -o $@
$(A8DIR)/semi.o: platform/qemu_virt/semihost_exit.c | $(A8DIR)
	$(CC) $(A8_GLUE_FLAGS) -O1 -c $< -o $@
$(A8DIR)/console.o: platform/qemu_a8/console_uart.c | $(A8DIR)
	$(CC) $(A8_GLUE_FLAGS) -O1 -c $< -o $@
$(A8DIR)/irq.o: platform/qemu_a8/irq_stub.c | $(A8DIR)
	$(CC) $(A8_GLUE_FLAGS) -O1 -c $< -o $@
$(A8DIR):
	@mkdir -p $@
$(A8DIR)/%.elf: $(BO)/tests/%.o $(BBB_CORE_OBJS) $(BBB_TABLE_OBJS) $(A8_GLUE) platform/qemu_a8/a8.ld
	$(CC) -mcpu=cortex-a8 -marm -nostdlib -Wl,--build-id=none -T platform/qemu_a8/a8.ld $(BBB_CORE_OBJS) $(BBB_TABLE_OBJS) $(A8_GLUE) $< -o $@ -lgcc
a8-test: toolchain-check $(patsubst %,$(A8DIR)/%.elf,$(A8_TESTS))
	@bash tests/run_a8.sh $(A8DIR) $(A8_TESTS)

# ---- the Buffee bootloader (bootloader/) ----------------------------------------------------------------------------------------
# Two images of the same source: asn (the one to flash) and asn_cal (also prints the clock-phase calibration). Output: build/bootloader/<BL>/bootloader.bin.
# Steps: the GreenPAK register image (tools/buffee/gen_asn_image.py -> bootloader/greenpak/asn_image.h), the PRU firmware (pru-gcc -> bootloader/src/pru_*.c),
# then bootloader/makefile.   docs/getting-started.md says where to get pru-gcc.
BL ?= asn
BL_DEFS_asn =
BL_DEFS_asn_cal = -DASN_CAL_DIAG
BL_DIR = build/bootloader/$(BL)
PRU_GCC_ROOT ?= $(HOME)/Dev/pru-toolchain/install

build/bootloader/asn_image.stamp: tools/buffee/gen_asn_image.py bootloader/greenpak/buffee4.hex
	@mkdir -p build/bootloader
	python3 tools/buffee/gen_asn_image.py > /dev/null
	@touch $@

build/bootloader/pru.stamp: bootloader/pru/main_gp4.c tools/buffee/pru_gcc.sh
	@mkdir -p build/bootloader
	@if [ -x $(PRU_GCC_ROOT)/bin/pru-gcc ] || command -v pru-gcc >/dev/null 2>&1; then \
	    PRU_GCC_ROOT=$(PRU_GCC_ROOT) tools/buffee/pru_gcc.sh bootloader main_gp4.c; \
	elif [ -f bootloader/src/pru_text.c ] && [ -f bootloader/src/pru_data.c ]; then \
	    echo "pru-gcc not found: using the PRU firmware already generated in bootloader/src/pru_*.c"; \
	else echo "error: pru-gcc not found and no generated PRU firmware (docs/getting-started.md, 'Toolchain')"; exit 1; fi
	@touch $@

$(BL_DIR)/bootloader.bin: FORCE build/bootloader/asn_image.stamp build/bootloader/pru.stamp
	$(MAKE) -C bootloader OUTDIR=$(abspath build/bootloader) BUILD=$(BL) VERSION=m68k-asn DEFS="$(BL_DEFS_$(BL))" -j$(JOBS)
	@ls -l $@

bootloader: $(BL_DIR)/bootloader.bin

# ---- make firmware ----------------------------------------------------------------------------------------------------------------
# What goes on a board: the bootloader (flash sector 0) and the standard Kickstart bundle (flash slot PJIT): the interpreter, slot cache and
# blocks, the Amiga window, the pace governor at 100% of a 68000, RAM code translated from $1000.   -> build/firmware/{bootloader.bin,buffee_boot.lzt}
# Install them with tools/buffee/restore_bootloader.py and restore_state.py (docs/getting-started.md).
FIRMWARE_CEXTRA = -DCLEAN -DDRT=0x1000u -DPACE=100
FIRMWARE_ASX    = -Wa,--defsym,PJIT_PACE=1
firmware: bootloader
	$(MAKE) VARIANT=fw BLK_CEXTRA='$(FIRMWARE_CEXTRA)' ASX='$(FIRMWARE_ASX)' bundle-boot
	@mkdir -p build/firmware
	cp build/bootloader/asn/bootloader.bin build/firmware/bootloader.bin
	cp build/fw/buffee/buffee_boot.lzt build/firmware/buffee_boot.lzt
	@ls -l build/firmware

.PHONY: bbb-all bootloader firmware a8-test
