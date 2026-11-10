# The Buffee bootloader: the boot image the AM335x ROM loads into its on-chip SRAM from the SPI flash. Built from the top-level Makefile (`make bootloader`), which first writes the
# GreenPAK register image (greenpak/asn_image.h) and, where pru-gcc is installed, the PRU firmware (src/pru_text.c, src/pru_data.c).
#   make -C bootloader OUTDIR=<dir> BUILD=<name> VERSION=<text> DEFS=<more -D switches>      ->  <dir>/<name>/bootloader.{elf,bin,map,asm,sz}
OUTDIR  ?= binaries
BUILD   ?= asn
VERSION ?= dev
DEFS    ?=
SRCDIRS := . clib hal src
CROSS   := arm-none-eabi-

CFLAGS := \
	-Wall -Wshadow -Wdouble-promotion -Wformat-overflow -Wformat-truncation -Wundef -Wno-unused-parameter -Wno-discarded-qualifiers \
	-fno-exceptions -fno-unwind-tables -nostdlib -fno-common -ffreestanding \
	-marm -mcpu=cortex-a8 -mfpu=vfpv3 -mfloat-abi=hard \
	-ffunction-sections -fdata-sections -fomit-frame-pointer \
	-std=gnu11 -fpermissive -MMD -MP -fmax-errors=5 \
	-Og -g3 -ggdb -ftree-vectorize -D_DEBUG \
	-DVERSION=$(VERSION) $(DEFS)

ifndef VERBOSE
Q := @
endif
CC := $(Q)$(CROSS)gcc

OUT     := $(OUTDIR)/$(BUILD)
OBJDIR  := $(OUT)/obj
ELF     := $(OUT)/bootloader.elf
SOURCES := $(foreach dir,$(SRCDIRS),$(wildcard $(dir)/*.c) $(wildcard $(dir)/*.s))
OBJECTS := $(sort $(addprefix $(OBJDIR)/,$(addsuffix .o,$(basename $(notdir $(SOURCES))))))
VPATH   := $(SRCDIRS)

LDFLAGS := -static -ffreestanding -T linker.lds -Wl,--gc-sections -Wl,--Map=$(ELF:.elf=.map)
INCLUDES := -isystem ./shim -I./inc $(addprefix -I./,$(SRCDIRS)) -I./greenpak

.PHONY: all clean
all: $(ELF)

$(ELF): $(OBJECTS) linker.lds
	@echo Linking $(notdir $@)...
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o $@
	$(Q)$(CROSS)objdump --disassemble $@ > $(@:.elf=.asm)
	$(Q)$(CROSS)objcopy -O binary $@ $(@:.elf=.bin)
	$(Q)$(CROSS)size $@
	$(Q)$(CROSS)nm --print-size --size-sort --radix=x $@ | grep -v -E "\.debug|\.group" > $(@:.elf=.sz)

$(OBJDIR)/%.o: %.c | $(OBJDIR)
	@echo Compiling $<...
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJDIR)/%.o: %.s | $(OBJDIR)
	@echo Assembling $<...
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	$(Q)mkdir -p $@

clean:
	rm -rf $(OUT)

-include $(OBJECTS:.o=.d)
