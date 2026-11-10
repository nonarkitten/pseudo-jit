# ======================================================================================================================================
#  The table generator (tools/gen), its checks, and the host-side 68000 oracle.     Included by the top-level Makefile.
#
#  The static tables the translator runs from -- DT, OT, DRSR, the ext-class table, the MOVEM and inline tables, and the block
#  translator's tables -- are written as assembly by tools/gen/maketables.py and tools/gen/btables.py (docs: tools/gen/README.md).
#  They are generated on the host in about 20 seconds; no emulator is involved.
# ======================================================================================================================================

# environment the generator reads
GENENV = PJIT_BUILD=$(BUILD) PJIT_FC=$(FC_KNOB) PJIT_X_LAZY=$(X_LAZY) PJIT_PIN_D0=$(PIN_D0)

# everything the tables are made from: the generator itself, the opcode tables it reads, and the profiles that weight the DRSR sharing
GEN_SRC = $(wildcard tools/gen/*.py tools/gen/emitters/*.py) pjit/ext_table.c $(wildcard tools/data/profiles/*.hist)

$(GEN) $(HOST) $(QEMUDIR) $(BBB) $(BUNDLES):
	@mkdir -p $@

# the switches that change the generated tables (a variant that changes one of them wants its own VARIANT; this catches the slips)
$(GEN)/tables.cfg: FORCE | $(GEN)
	$(call stamp,$@,pin=$(PIN_D0) lazy=$(X_LAZY) fc=$(FC_KNOB) live=$(BLK_LIVE))

# the constants the .s files need from cpu_t (CPU_D0, CPU_SR, ...), read out of the compiler's own output so they cannot drift from
# the struct (pjit/asm_offsets.c)
$(GEN)/asm_offsets.inc: pjit/asm_offsets.c pjit/pjit.h pjit/bcache.h pjit/blk.h $(QEMU_STAMP) | $(GEN)
	$(CC) $(CFLAGS) -S pjit/asm_offsets.c -o $(GEN)/asm_offsets.s
	sed -n 's/^->\([A-Za-z0-9_]*\) #\{0,1\}\(-\{0,1\}[0-9][0-9]*\).*/.equ \1, \2/p' $(GEN)/asm_offsets.s > $@

# interpreter + slot cache tables
$(GEN)/tables.s: $(GEN_SRC) $(GEN)/asm_offsets.inc $(GEN)/tables.cfg | $(GEN)
	$(GENENV) $(PY) tools/gen/maketables.py > $@

# the same words once more as bare `.word`s: GNU as must turn tables.s into exactly these (check-asm)
$(GEN)/tables_words.s: $(GEN_SRC) $(GEN)/asm_offsets.inc $(GEN)/tables.cfg | $(GEN)
	$(GENENV) $(PY) tools/gen/maketables.py --words > $@

# the block translator's tables
$(GEN)/btables.s: $(GEN_SRC) $(GEN)/asm_offsets.inc $(GEN)/tables.cfg | $(GEN)
	$(GENENV) $(PY) tools/gen/btables.py $(if $(filter 1,$(BLK_LIVE)),--live,) > $@

tables: $(GEN)/tables.s $(GEN)/btables.s

# a readable listing of every opcode's generated sequences (cached and interpretive path)
listing: $(GEN)/asm_offsets.inc $(GEN_SRC)
	@mkdir -p $(GEN)/listing
	@$(GENENV) $(PY) tools/gen/listing.py > $(GEN)/listing/opcodes.txt
	@echo "listing: $(GEN)/listing/opcodes.txt"

# ---- checks ----------------------------------------------------------------------------------------------------------------------
# The generator's sequences must reproduce what the encoder meant (check-asm), obey the hard rules for emitted code (check_rules.py),
# and agree with the 68000 oracle golden/mcl68k.c on random and edge-case states when run under Unicorn (sim_test.py, trap_test.py).
$(HOST)/oracle1: tools/gen/check/oracle1.c golden/mcl68k.c | $(HOST)
	cc -O1 -Igolden -Iplatform -include golden/shim.h tools/gen/check/oracle1.c golden/mcl68k.c -o $@

# the runtime helper routines the generated sequences branch to (DIV, BCD, MOVEM), linked with the generated tables at a fixed address
# so that tools/gen/check/sim.py can run them under Unicorn
$(HOST)/helpers.bin: $(QEMUDIR)/o/pjit/handle_bcd.o $(QEMUDIR)/o/pjit/handle_divw.o $(QEMUDIR)/o/pjit/handle_movem.o $(QEMUDIR)/o/tables.o | $(HOST)
	$(LD) -Ttext=0x400000 --unresolved-symbols=ignore-all -o $(HOST)/helpers.elf $^
	$(NM) --defined-only $(HOST)/helpers.elf > $(HOST)/helpers.sym
	$(OBJCOPY) -O binary $(HOST)/helpers.elf $@

check-asm: $(QEMUDIR)/o/tables.o $(QEMUDIR)/o/tables_words.o | $(HOST)
	@for sec in .data.pjit_arena .data.pjit_drsr; do \
	  $(OBJCOPY) -O binary -j $$sec $(QEMUDIR)/o/tables.o $(HOST)/_a.bin; \
	  $(OBJCOPY) -O binary -j $$sec $(QEMUDIR)/o/tables_words.o $(HOST)/_b.bin; \
	  cmp $(HOST)/_a.bin $(HOST)/_b.bin || exit 1; done
	@$(READELF) -r $(QEMUDIR)/o/tables.o | awk 'NF>=5 && $$3 ~ /^R_ARM/ {print $$1, $$3, $$5}' > $(HOST)/_ra.txt; \
	 $(READELF) -r $(QEMUDIR)/o/tables_words.o | awk 'NF>=5 && $$3 ~ /^R_ARM/ {print $$1, $$3, $$5}' > $(HOST)/_rb.txt; cmp $(HOST)/_ra.txt $(HOST)/_rb.txt
	@echo "check-asm: GNU as turns the generated mnemonics into exactly the encoder's words (arena, DRSR, relocations)"

check-gen: $(HOST)/helpers.bin $(HOST)/oracle1 $(GEN)/asm_offsets.inc check-asm
	$(GENENV) $(PY) tools/gen/check/check_rules.py
	$(GENENV) JOBS=$(JOBS) $(PY) tools/gen/check/sim_test.py 8
	$(GENENV) $(PY) tools/gen/check/trap_test.py 8

.PHONY: tables listing check-asm check-gen
