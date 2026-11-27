# ======================================================================================================================================
#  The Amiga side: programs that run on the Amiga under AmigaOS (amiga/), built with m68k-elf-gcc into AmigaDOS executables.   Included by the Makefile.
#
#  make amiga        -> build/amiga/buffee          the `buffee` Shell command (docs/buffee/BUFFEE_CLI.md), PC-relative: no relocations
#  make amiga-test   runs it on the golden 68000 core against PJIT's control page (platform/buffee/control.c) and a simulated flash and EEPROM, and reads back
#                    a floppy written by tools/amiga/adf.py (tests/amiga/)
#  make adf          -> build/amiga/buffee.adf      a bootable floppy: the command, the firmware's PJIT bundle and a start-up sequence that shows the board's state
#  make amiga-embed  -> platform/buffee/buffee_cmd.h the command as a RomTag that the payload puts at $F01000 (resident on Kickstart 3.0 and later); checked in
#  They do not depend on VARIANT: one build serves every payload.
# ======================================================================================================================================

AMIGA        = build/amiga
M68K_CC     ?= m68k-elf-gcc
AMIGA_CFLAGS = -m68000 -mpcrel -Os -fomit-frame-pointer -ffreestanding -nostdlib -fno-builtin -Wall -Iamiga/include -Iplatform/buffee
AMIGA_LIB    = $(AMIGA)/o/start.o $(AMIGA)/o/os.o

$(AMIGA_LIB): $(AMIGA)/o/%.o: amiga/lib/%.S
	@mkdir -p $(dir $@)
	$(M68K_CC) -m68000 -c $< -o $@
$(AMIGA)/o/buffee.o: amiga/buffee/buffee.c platform/buffee/control.h amiga/include/amiga_os.h
	@mkdir -p $(dir $@)
	$(M68K_CC) $(AMIGA_CFLAGS) -c $< -o $@
$(AMIGA)/buffee: $(AMIGA)/o/buffee.o $(AMIGA_LIB) amiga/amiga.ld tools/amiga/elf2hunk.py
	$(M68K_CC) -m68000 -nostdlib -Wl,--emit-relocs -Wl,--no-warn-rwx-segments -T amiga/amiga.ld $(AMIGA_LIB) $(AMIGA)/o/buffee.o -lgcc -o $@.elf
	python3 tools/amiga/elf2hunk.py --no-relocs $@.elf $@

# the host test: the golden core, a simulated board, and the payload's control.c with the bootloader's own flash, EEPROM and settings code
AMIGA_HOST_BL  = bootloader/src/store.c bootloader/hal/config.c bootloader/src/settings_set.c
AMIGA_HOST_SRC = tests/amiga/amiga_host.c tests/amiga/mock_board.c platform/buffee/control.c golden/mcl68k.c $(AMIGA_HOST_BL)
$(AMIGA)/host/amiga_host: $(AMIGA_HOST_SRC) platform/buffee/control.h platform/buffee/bootloader_shim.h bootloader/src/store.h bootloader/inc/config.h
	@mkdir -p $(AMIGA)/host
	cc -O1 -g -w -Iplatform/buffee -Ibootloader/inc -Ibootloader/src -include platform/buffee/bootloader_shim.h -c bootloader/src/store.c -o $(AMIGA)/host/store.o
	cc -O1 -g -w -Iplatform/buffee -Ibootloader/inc -Ibootloader/src -include platform/buffee/bootloader_shim.h -c bootloader/hal/config.c -o $(AMIGA)/host/config.o
	cc -O1 -g -w -Iplatform/buffee -Ibootloader/inc -Ibootloader/src -include platform/buffee/bootloader_shim.h -c bootloader/src/settings_set.c -o $(AMIGA)/host/settings_set.o
	cc -O1 -g -Wall -Iplatform/buffee -Ibootloader/inc -Ibootloader/src -c platform/buffee/control.c -o $(AMIGA)/host/control.o
	cc -O1 -g -Wall -Iplatform/buffee -Ibootloader/inc -Ibootloader/src -c tests/amiga/mock_board.c -o $(AMIGA)/host/mock_board.o
	cc -O1 -g -Wall -Igolden -Iplatform -Iplatform/buffee -include golden/shim.h -c tests/amiga/amiga_host.c -o $(AMIGA)/host/amiga_host.o
	cc -O1 -g -w -Igolden -Iplatform -include golden/shim.h -c golden/mcl68k.c -o $(AMIGA)/host/mcl68k.o
	cc $(addprefix $(AMIGA)/host/,store.o config.o settings_set.o control.o mock_board.o amiga_host.o mcl68k.o) -o $@

# the resident command (Kickstart 3.0 and later): a RomTag with the program, linked at $F01000, which the payload copies into the MapROM megabyte (platform/buffee/buffee_cmd.h)
$(AMIGA)/buffee.bin: $(AMIGA)/buffee
	m68k-elf-objcopy -O binary -j .hunk $(AMIGA)/buffee.elf $@
$(AMIGA)/resident.bin: amiga/buffee/resident.S $(AMIGA)/buffee.bin
	$(M68K_CC) -m68000 -Wa,-I$(AMIGA) -c $< -o $(AMIGA)/o/resident.o
	$(M68K_CC) -m68000 -nostdlib -Wl,-Ttext=0xF01000 -Wl,--no-warn-rwx-segments $(AMIGA)/o/resident.o -o $(AMIGA)/resident.elf
	m68k-elf-objcopy -O binary $(AMIGA)/resident.elf $@

amiga: $(AMIGA)/buffee $(AMIGA)/resident.bin
amiga-embed: $(AMIGA)/resident.bin
	python3 tools/amiga/embed.py $< F01000 platform/buffee/buffee_cmd.h
amiga-test: $(AMIGA)/buffee $(AMIGA)/resident.bin $(AMIGA)/host/amiga_host
	python3 tests/amiga/test_buffee.py $(AMIGA)/host/amiga_host $(AMIGA)/buffee $(AMIGA)/resident.bin
	python3 tests/amiga/test_adf.py
	python3 tools/amiga/embed.py --check $(AMIGA)/resident.bin F01000 platform/buffee/buffee_cmd.h

adf: $(AMIGA)/buffee firmware tools/amiga/adf.py
	python3 tools/amiga/adf.py $(AMIGA)/buffee.adf Buffee --boot \
	    c/buffee=$(AMIGA)/buffee pjit.lzt=build/firmware/buffee_boot.lzt s/startup-sequence=amiga/buffee/startup-sequence

.PHONY: amiga amiga-embed amiga-test adf
