# All critical output, source folders
OUTDIR  := binaries
BINARY  := pjit.elf
SRCDIRS := . clib/ hal/ pjit/ src/ greenpak/
CROSS   := arm-none-eabi-

GENERATED := src/pru_text.c src/pru_data.c
GREENPAK := greenpak/buffee.hex

# CPU type and common options
CCFLAGS_CPU := \
	-fno-exceptions -fno-unwind-tables -nostdlib -fno-common -ffreestanding \
	-marm -mcpu=cortex-a8 -mfpu=vfpv3 -mfloat-abi=hard \
	-ffunction-sections -fdata-sections -fomit-frame-pointer \
	-std=gnu11 -MMD -fmax-errors=5

# Universally good compiler warnings	
CCFLAGS_WARN := \
	-Wall -Wshadow -Wdouble-promotion -Wformat-overflow -Wformat-truncation \
	-Wundef -Wno-unused-parameter -Wno-discarded-qualifiers

ifeq ($(MAKECMDGOALS),clean)
BUILD := *
else ifdef RELEASE
BUILD := release
else
BUILD := debug
endif

# Compiler optimizations
ifeq ($(BUILD),release)
# Optimize for release and raise the bar for errors
CCFLAGS_OPT := -Os -ffast-math -Werror -s
else
# Optimize for debugging, but include basic optimizers for not-terrible-code
CCFLAGS_OPT := -Og -g3 -ggdb -ftree-vectorize -D_DEBUG
endif

# Set noisey output
ifndef VERBOSE
QUIET := @
endif

# Any build defintions
DEFINES := -DVERSION="${shell git describe --tags --no-dirty | sed 's/\(.*\)-\(.*\)-.*/\1-\2/'}"

# Combine all the flags so far
CFLAGS := $(CCFLAGS_WARN) $(CCFLAGS_CPU) $(CCFLAGS_OPT) $(DEFINES)

AS := $(QUIET)$(CROSS)gcc
AR := $(QUIET)$(CROSS)ar
SZ := $(QUIET)$(CROSS)size
CC := $(QUIET)$(CROSS)gcc
OC := $(QUIET)$(CROSS)objcopy
DI := $(QUIET)$(CROSS)objdump --disassemble 
MD := $(QUIET)mkdir -p
RM := rm -f
RD := rm -rf

OUTPUT := $(OUTDIR)/$(BUILD)/$(BINARY)
OBJDIR := $(OUTDIR)/$(BUILD)/obj

BINARY  := $(patsubst %.elf,%.bin,$(OUTPUT))
MAPFILE := $(patsubst %.elf,%.map,$(OUTPUT))
DISFILE := $(patsubst %.elf,%.asm,$(OUTPUT))

LDFLAGS := -static -ffreestanding -T linker.lds \
	-Wl,--gc-sections \
	-Wl,--Map=$(MAPFILE)

INCLUDES := -I./inc $(addprefix -I./,$(SRCDIRS))
SOURCES := $(foreach dir,$(SRCDIRS),$(wildcard $(dir)/*.c) $(wildcard $(dir)/*.s)) $(GENERATED) $(GREENPAK)
OBJECTS := $(sort $(addprefix $(OBJDIR)/,$(addsuffix .o,$(basename $(foreach file,$(SOURCES),$(notdir $(file)))))))

VPATH := $(SRCDIRS)

.PHONY: all premake clean debug release generate-pru

# $(info $(addprefix $(OBJDIR)/,$(notdir $(filter %.d,$(SOURCES:%.c=%.d)))))

all: premake $(OUTPUT) $(GREENPAK)

clean:
	@echo Cleaning $(OUTPUT)...
	-$(RM) $(OBJDIR)/*
	-$(RD) $(OBJDIR)
	-$(RM) $(OUTPUT)
	-$(RM) $(BINARY)
	-$(RM) $(MAPFILE)
	-$(RM) $(MAPFILE).bak
	-$(RM) $(DISFILE)
	-$(RD) $(OUTDIR)/$(BUILD)
	-$(RM) greenpak/*.c
	-$(RM) $(GENERATED)
	@make -C pru -f pru.mk clean
	-@echo 'Finished clean'

premake:
	$(MD) $(OBJDIR)

generate-pru: pru/main.c
	@make -C pru -f pru.mk

$(GENERATED): generate-pru

$(OUTPUT): $(OBJECTS) $(GENERATED)
	@echo Building PJIT Bootloader...
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o $@
	$(DI) $@ > $(DISFILE) &
	$(OC) -O binary $@ $(BINARY)
	$(SZ) $@
	$(QUIET)nm --print-size --size-sort --radix=x $@ | grep -v -E "\.debug|\.group" > $(@:%.elf=%.sz)

$(OBJDIR)/%.o: %.hex
	@echo Compiling $<...
	$(OC) --input-target=ihex --output-target=binary '$<' $(@:%.o=%.bin)
	$(QUIET)xxd -i -u -a -n nvmData $(@:%.o=%.bin) >$(<:%.hex=%.c)
	$(QUIET)sed -i '' 's/0X/0x/g' $(<:%.hex=%.c)
	$(CC) $(CFLAGS) $(INCLUDES) -S -c $(<:%.hex=%.c) -o $(@:%.o=%.s) &
	$(CC) $(CFLAGS) $(INCLUDES) -c $(<:%.hex=%.c) -o $@

$(OBJDIR)/%.o: %.c
	@echo Compiling $<...
	$(CC) $(CFLAGS) $(INCLUDES) -S -c $< -o $(@:%.o=%.s) &
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJDIR)/%.o: %.s
	@echo Assembling $<...
	$(AS) $(CFLAGS) -c $< -o $@

-include $(addprefix $(OBJDIR)/,$(notdir $(filter %.d,$(SOURCES:%.c=%.d))))
