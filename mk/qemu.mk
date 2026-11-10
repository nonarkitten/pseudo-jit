# ======================================================================================================================================
#  Everything that runs under qemu-system-arm:  the smoke tests (`make test`), the Amiga differential harness, and golden/ -- Ted Fried's
#  MC68000 suite and the random-case oracle comparison.  (The Cortex-A8 core model, `make a8-test`, is in board.mk with the A8 objects.)  Included by the Makefile.
#
#  The tests live in tests/qemu/ (see tests/README.md).  Their objects are built with the same flags as the translator.
# ======================================================================================================================================

QEMU_MACHINE = -M virt -cpu cortex-a15 -m 128M -nographic -semihosting     # cortex-a15 so the A15-only instructions the tests use run

CFLAGS   = -mcpu=cortex-a15 -marm -ffreestanding -nostdlib -O1 -g -Wall $(INCLUDES) $(CORE_DEFS) $(EXTRA_CFLAGS)
ASFLAGS  = -mcpu=cortex-a15 -marm -ffreestanding -nostdlib -g -Ipjit -Iplatform -I$(GEN) $(CORE_ASM)

QEMU_VIRT_SRCS = platform/qemu_virt/boot.S platform/qemu_virt/irq.S platform/qemu_virt/console_uart.c platform/qemu_virt/semihost_exit.c
QEMU_VIRT_LD   = platform/qemu_virt/virt.ld

# test sources are found by name in their family directory
vpath %.c tests/qemu tests/bbb tests/buffee

QO = $(QEMUDIR)/o

$(QEMU_STAMP): FORCE | $(BUILD)
	$(call stamp,$@,$(CFLAGS) | $(ASFLAGS))

# ---- objects ---------------------------------------------------------------------------------------------------------------------
# $(QO)/pjit/cache.o <- pjit/cache.c, $(QO)/tests/smoke_run.o <- tests/qemu/smoke_run.c (vpath), ...   -MMD / -MD: header dependencies
$(QO)/tests/%.o: %.c $(QEMU_STAMP)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@
$(QO)/%.o: %.c $(QEMU_STAMP)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@
$(QO)/%.o: %.s $(GEN)/asm_offsets.inc $(QEMU_STAMP)
	@mkdir -p $(dir $@)
	$(CC) $(ASFLAGS) -Wa,-MD,$(@:.o=.d) -c $< -o $@
$(QO)/%.o: %.S $(GEN)/asm_offsets.inc $(QEMU_STAMP)
	@mkdir -p $(dir $@)
	$(CC) $(ASFLAGS) -MMD -MP -c $< -o $@
$(QO)/tables.o: $(GEN)/tables.s $(QEMU_STAMP)
	@mkdir -p $(dir $@)
	$(CC) $(ASFLAGS) -c $< -o $@
$(QO)/tables_words.o: $(GEN)/tables_words.s $(QEMU_STAMP)
	@mkdir -p $(dir $@)
	$(CC) $(ASFLAGS) -c $< -o $@
$(QO)/btables.o: $(GEN)/btables.s $(QEMU_STAMP)
	@mkdir -p $(dir $@)
	$(CC) $(ASFLAGS) -c $< -o $@

# what every qemu_virt-hosted test links: the translator, the freestanding libc shim, the virt board's boot / console glue, the tables
QEMU_CORE_OBJS = $(patsubst %.c,$(QO)/%.o,$(PJIT_C_SRCS)) $(patsubst %.s,$(QO)/%.o,$(PJIT_S_SRCS)) \
                 $(patsubst %.c,$(QO)/%.o,$(PLATFORM_SRCS)) $(patsubst %.S,$(QO)/%.o,$(PLATFORM_ASM_SRCS)) \
                 $(patsubst %.c,$(QO)/%.o,$(filter %.c,$(QEMU_VIRT_SRCS))) $(patsubst %.S,$(QO)/%.o,$(filter %.S,$(QEMU_VIRT_SRCS))) \
                 $(QO)/tables.o $(if $(HAS_BLOCKS),$(QO)/btables.o)

-include $(QEMU_CORE_OBJS:.o=.d)

# ---- the smoke tests -------------------------------------------------------------------------------------------------------------
# `make test` runs exactly these.  A new tests/qemu/*.c file is added HERE and nowhere else.
TEST_NAMES = smoke_tables smoke_div smoke_call_exec smoke_branch_exec smoke_loop_exec smoke_sub_exec smoke_ext_exec \
	smoke_extw_exec smoke_btst_exec smoke_btst_bit31 smoke_timer smoke_mul_exec smoke_ea_addr_exec smoke_supervisor_exec smoke_user_trap \
	smoke_bcd_exec smoke_movep_exec smoke_addq_an_word smoke_jmp_flags_preserved smoke_bra_flags smoke_ot smoke_run smoke_ea_alu_interp \
	smoke_xmodes_interp smoke_exception_interp smoke_control_interp smoke_run_dr smoke_ea_alu_dr smoke_xmodes_dr smoke_exception_dr \
	smoke_control_dr smoke_irq_interp smoke_irq_dr smoke_alias_interp smoke_alias_dr smoke_kernels smoke_movem_sp smoke_movem_sp_dr \
	smoke_links smoke_rts smoke_index smoke_index_dr smoke_xlazy smoke_fuse smoke_bsrb smoke_tinybasic smoke_movem_ea smoke_movem_ea_dr \
	smoke_defer smoke_loops smoke_bcache smoke_ext_audit smoke_ext_audit_interp smoke_lzt smoke_fill_spill smoke_evict_stress \
	smoke_fill_pad smoke_blkfast_empty
# these depend on the QEMU timer's real-time behaviour: run alone, after the others (an oversubscribed host makes them flaky)
TIMING_TESTS = smoke_timer smoke_irq_interp smoke_irq_dr
QEMU_ELFS = $(patsubst %,$(QEMUDIR)/%.elf,$(TEST_NAMES))
-include $(patsubst %,$(QO)/tests/%.d,$(TEST_NAMES))

$(QEMUDIR)/%.elf: $(QO)/tests/%.o $(QEMU_CORE_OBJS) $(QEMU_VIRT_LD)
	$(CC) $(CFLAGS) $(LDFLAGS) -T $(QEMU_VIRT_LD) $(QEMU_CORE_OBJS) $< -o $@ -lgcc
	$(OBJDUMP) -d $@ > $(QEMUDIR)/$*.lst

test: toolchain-check $(QEMU_ELFS)
	@JOBS=$(JOBS) TIMING_TESTS="$(TIMING_TESTS)" bash tests/run.sh $(QEMUDIR) $(TEST_NAMES)

qemu-%: $(QEMUDIR)/%.elf
	@ls -l $<
run-%: $(QEMUDIR)/%.elf
	$(QEMU) $(QEMU_MACHINE) -kernel $<

# ---- the Amiga differential harness (tools/amiga): PJIT's interpreter vs the golden core on a scripted Amiga model ----------------
AMIGA_OBJS = $(QO)/tests/amiga_hook.o $(QO)/tools/amiga/amiga.o
$(QO)/tests/amiga_hook.o: tests/qemu/amiga_hook.S $(QEMU_STAMP)
	@mkdir -p $(dir $@)
	$(CC) $(ASFLAGS) -MMD -MP -c $< -o $@
$(QEMUDIR)/amiga_diff.elf: $(QO)/tests/amiga_diff.o $(AMIGA_OBJS) $(QEMU_CORE_OBJS) $(QEMU_VIRT_LD)
	$(CC) $(CFLAGS) $(LDFLAGS) -T $(QEMU_VIRT_LD) $(QEMU_CORE_OBJS) $(QO)/tests/amiga_diff.o $(AMIGA_OBJS) -o $@ -lgcc
	$(OBJDUMP) -d $@ > $(QEMUDIR)/amiga_diff.lst

# ---- golden/ -----------------------------------------------------------------------------------------------------------------------
# Ted Fried's self-checking MC68000 opcode suite run through the real core (needs no oracle: see golden/mcl68k_testsuite.c), once through
# the interpreter alone (GOLDEN_INTERP) and once with everything cacheable (GOLDEN_DR); and random cases run through the host-native
# oracle (golden/mcl68k.c) and through PJIT, compared line by line.
GOLDEN_CFLAGS = $(CFLAGS) -Igolden
$(QO)/golden/suite_interp.o: golden/mcl68k_testsuite.c golden/mcl68k_test_image.h $(QEMU_STAMP)
	@mkdir -p $(dir $@)
	$(CC) $(GOLDEN_CFLAGS) -DGOLDEN_INTERP -c $< -o $@
$(QO)/golden/suite_dr.o: golden/mcl68k_testsuite.c golden/mcl68k_test_image.h $(QEMU_STAMP)
	@mkdir -p $(dir $@)
	$(CC) $(GOLDEN_CFLAGS) -DGOLDEN_DR -c $< -o $@
$(QO)/golden/pjit_side.o: golden/pjit_side.c golden/cases.h $(QEMU_STAMP)
	@mkdir -p $(dir $@)
	$(CC) $(GOLDEN_CFLAGS) -c $< -o $@
$(QEMUDIR)/golden_%.elf: $(QO)/golden/%.o $(QEMU_CORE_OBJS) $(QEMU_VIRT_LD)
	$(CC) $(GOLDEN_CFLAGS) $(LDFLAGS) -T $(QEMU_VIRT_LD) $(QEMU_CORE_OBJS) $< -o $@ -lgcc
	$(OBJDUMP) -d $@ > $(QEMUDIR)/golden_$*.lst
$(HOST)/golden_oracle: golden/oracle.c golden/mcl68k.c golden/cases.h | $(HOST)
	cc -O1 -g -Wall -Igolden -Iplatform -include golden/shim.h golden/oracle.c golden/mcl68k.c -o $@

# the BASIC image of test_progs/tinybasic.c on the oracle, the console output of two programs: an independent check of what smoke_tinybasic prints
$(HOST)/tb_oracle: golden/tb_oracle.c golden/mcl68k.c tests/data/tinybasic_image.h | $(HOST)
	cc -O1 -g -Wall -Igolden -Iplatform -Itests/data -include golden/shim.h golden/tb_oracle.c golden/mcl68k.c -o $@

tb-oracle: $(HOST)/tb_oracle
	@$(HOST)/tb_oracle 2>/dev/null | tr -d '\r' | grep -qx '55' && echo "tb-oracle: FOR/NEXT sum = 55: OK" || { echo "tb-oracle: FAIL (sum)"; exit 1; }
	@$(HOST)/tb_oracle -f test_progs/sieve.tbas 2>/dev/null | tr -d '\r' | grep -qx '303' && echo "tb-oracle: sieve of 1000 flags = 303 primes: OK" || { echo "tb-oracle: FAIL (sieve)"; exit 1; }

golden: toolchain-check $(QEMUDIR)/golden_suite_interp.elf $(QEMUDIR)/golden_suite_dr.elf
	@echo "== Ted Fried's MC68000 suite, interpreter only =="
	@$(QEMU) $(QEMU_MACHINE) -kernel $(QEMUDIR)/golden_suite_interp.elf 2>/dev/null | tr -d '\r' | tee $(QEMUDIR)/golden_suite_interp.log | tail -3
	@echo "== Ted Fried's MC68000 suite, slot cache + blocks =="
	@$(QEMU) $(QEMU_MACHINE) -kernel $(QEMUDIR)/golden_suite_dr.elf 2>/dev/null | tr -d '\r' | tee $(QEMUDIR)/golden_suite_dr.log | tail -3
	@grep -q '^PASS' $(QEMUDIR)/golden_suite_interp.log && grep -q '^PASS' $(QEMUDIR)/golden_suite_dr.log && echo "golden: PASS" || { echo "golden: FAIL"; exit 1; }

golden-compare: toolchain-check $(HOST)/golden_oracle $(QEMUDIR)/golden_pjit_side.elf
	@$(HOST)/golden_oracle > $(HOST)/golden_oracle.out
	@$(QEMU) $(QEMU_MACHINE) -kernel $(QEMUDIR)/golden_pjit_side.elf 2>/dev/null | tr -d '\r' | grep '^case ' > $(HOST)/golden_pjit.out
	@diff $(HOST)/golden_oracle.out $(HOST)/golden_pjit.out && echo "MATCH: $$(wc -l < $(HOST)/golden_oracle.out) cases, all identical"

.PHONY: test golden golden-compare tb-oracle
