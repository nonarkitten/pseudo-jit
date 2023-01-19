# All critical output, source folders
OUTDIR  := bin

BINARY  := buffee_bootloader.elf
SRCDIRS := . clib/ hal/ libbbb/src/ pjit/
CROSS   := arm-none-eabi-

# CPU type and common options
CCFLAGS_CPU := \
	-fno-exceptions -fno-unwind-tables -nostdlib -fno-common \
	-mbig-endian -ffreestanding -mbe8 \
	-marm -mcpu=cortex-a8 -mfpu=vfpv3 -mfloat-abi=hard \
	-ffunction-sections -fdata-sections -fomit-frame-pointer \
	-std=gnu11 -MMD -fmax-errors=5

# Universally good compiler warnings
CCFLAGS_WARN := \
	-Wall -Wshadow -Wdouble-promotion -Wformat-overflow -Wformat-truncation \
	-Wundef -Wno-unused-parameter

ifneq ($(filter debug,$(MAKECMDGOALS)),)
BUILD := debug
else ifneq ($(filter release,$(MAKECMDGOALS)),)
BUILD := release
else ifeq ($(MAKECMDGOALS),clean)
BUILD := *
else
$(error Must specify release or debug)
endif

# Compiler optimizations
ifeq ($(BUILD),release)
# Optimize for release and raise the bar for errors
CCFLAGS_OPT := -Os -ffast-math -Werror -s -ftree-vectorize
else
# Optimize for debugging, but include basic optimizers for not-terrible-code
CCFLAGS_OPT := -Og -g3 -ggdb -D_DEBUG -ftree-vectorize
endif

# Set noisey output
ifndef VERBOSE
QUIET := @
endif

# Any build defintions
DEFINES :=

# Combine all the flags so far
CFLAGS := $(CCFLAGS_WARN) $(CCFLAGS_CPU) $(CCFLAGS_OPT) $(DEFINES)

AS := $(QUIET)$(CROSS)gcc
AR := $(QUIET)$(CROSS)ar
SZ := $(QUIET)$(CROSS)size
CC := $(QUIET)$(CROSS)gcc
OC := $(QUIET)$(CROSS)objcopy
DI := $(QUIET)$(CROSS)objdump --disassemble
RM := $(QUIET)rm -rf
RD := $(QUIET)rmdir
MD := $(QUIET)mkdir -p

OUTPUT := $(OUTDIR)/$(BUILD)/$(BINARY)
OBJDIR := $(OUTDIR)/$(BUILD)/obj

BINARY  := $(patsubst %.elf,%.bin,$(OUTPUT))
MAPFILE := $(patsubst %.elf,%.map,$(OUTPUT))
DISFILE := $(patsubst %.elf,%.asm,$(OUTPUT))

LDFLAGS := -static -ffreestanding -T linker.lds \
	-Wl,--gc-sections \
	-Wl,--Map=$(MAPFILE) \
	-Wl,--be8 -Wl,--format=elf32-bigarm

INCLUDES := -I./inc -I./libbbb/inc/ $(addprefix -I./,$(SRCDIRS))
C_SOURCES := $(foreach dir,$(SRCDIRS),$(wildcard $(dir)/*.c))
C_OBJECTS := $(addprefix $(OBJDIR)/,$(addsuffix .o,$(basename $(foreach file,$(C_SOURCES),$(notdir $(file))))))
ASM_SOURCES := $(foreach dir,$(SRCDIRS),$(wildcard $(dir)/*.s))
ASM_OBJECTS := $(addprefix $(OBJDIR)/,$(addsuffix .o,$(basename $(foreach file,$(ASM_SOURCES),$(notdir $(file))))))
OBJECTS := $(C_OBJECTS) $(ASM_OBJECTS)

VPATH := $(SRCDIRS)

.PHONY: all premake clean debug release

clean:
	@echo Cleaning $(OUTPUT)...
	-@make -C pru -f pru.mk clean
	-$(RM) $(OBJDIR)/* 2>/dev/null
	-$(RD) $(OBJDIR) 2>/dev/null
	-$(RM) $(OUTPUT) 2>/dev/null
	-$(RM) $(BINARY) 2>/dev/null
	-$(RM) $(MAPFILE) 2>/dev/null
	-$(RM) $(MAPFILE).bak 2>/dev/null
	-$(RM) $(DISFILE) 2>/dev/null
	-$(RM) $(OUTDIR)/$(BUILD) 2>/dev/null

release debug: premake all
all: $(OUTPUT)

premake:
	$(MD) $(OBJDIR)
	@make -C pru -f pru.mk

$(OUTPUT): $(OBJECTS) pru/main.c
	@echo Building PJIT Bootloader...
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o $@
	$(DI) $@ > $(DISFILE) &
	$(OC) -O binary $@ $(BINARY)
	$(SZ) $@
	$(QUIET)nm --print-size --size-sort --radix=x $@ | grep -v -E "\.debug|\.group" > $(@:%.elf=%.sz)

$(OBJDIR)/%.o: %.c
	@echo Compiling $<...
	$(CC) $(CFLAGS) $(INCLUDES) -S -c $< -o $(@:%.o=%.s) &
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJDIR)/%.o: %.s
	@echo Assembling $<...
	$(AS) $(CFLAGS) -c $< -o $@

-include $(C_OBJECTS:%.o=%.d)
