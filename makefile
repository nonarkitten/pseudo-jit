# All critical output, source folders
OUTDIR  := bin

BINARY  := buffee.elf
SRCDIRS := . clib/ hal/ libbbb/src/ pjit/
CROSS   := arm-none-eabi-

# CPU type and common options
CCFLAGS_CPU := -march=armv7-a -marm -mfpu=neon -mtune=cortex-a8		# CPU core type
CCFLAGS_CPU += -mfpu=neon-vfpv3 -mfloat-abi=hard -ffast-math		# CPU floating point
CCFLAGS_CPU += -ffunction-sections -fdata-sections					# Break functions & data into own sections
CCFLAGS_CPU += -std=gnu11	     									# We're using GNU extensions
CCFLAGS_CPU += -MMD     											# Build dependency files
CCFLAGS_CPU += -fomit-frame-pointer -nostdlib -ffreestanding		# Omit some OS overhead
CCFLAGS_CPU += -mbe8 -mbig-endian									# Compile for big-endian mode

# Universally good compiler warnings
CCFLAGS_WARN := \
	-Wall -Wshadow -Wdouble-promotion -Wformat-overflow -Wformat-truncation \
	-Wundef -Wno-unused-parameter -fmax-errors=5

ifneq ($(filter release,$(MAKECMDGOALS)),)
BUILD := release
else
BUILD := debug
endif

# Compiler optimizations
ifeq ($(BUILD),release)
# Optimize for release and raise the bar for errors
# Do not use -Os no matter how tempting! Os uniquely creates literal pools which break big-endian!
CCFLAGS_OPT := -O3 -Werror -s
$(info Compiling release build);
else
# Optimize for debugging, but include basic optimizers for not-terrible-code
CCFLAGS_OPT := -Og -g -D_DEBUG
$(info Compiling debug build; to compile release, specify 'release' after make);
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

LDFLAGS := -static -ffreestanding -nostdlib -nostartfiles -T linker.lds
LDFLAGS += -Wl,--Map=$(MAPFILE)
LDFLAGS += -Wl,--be8 -Wl,--format=elf32-bigarm
LDFLAGS += -Wl,--no-warn-rwx-segment
ifeq ($(BUILD),debug)
LDFLAGS += -Wl,--gc-sections
endif

INCLUDES := -I./inc -I./libbbb/inc/ $(addprefix -I./,$(SRCDIRS))
C_SOURCES := $(foreach dir,$(SRCDIRS),$(wildcard $(dir)/*.c))
C_OBJECTS := $(addprefix $(OBJDIR)/,$(addsuffix .o,$(basename $(foreach file,$(C_SOURCES),$(notdir $(file))))))
ASM_SOURCES := $(foreach dir,$(SRCDIRS),$(wildcard $(dir)/*.s))
ASM_OBJECTS := $(addprefix $(OBJDIR)/,$(addsuffix .o,$(basename $(foreach file,$(ASM_SOURCES),$(notdir $(file))))))
OBJECTS := $(C_OBJECTS) $(ASM_OBJECTS)

VPATH := $(SRCDIRS)

.PHONY: all premake clean debug release

all: premake $(OUTPUT)

release debug: all

premake:
	$(MD) $(OBJDIR)
	-@make -C pru -f pru.mk

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

$(OUTPUT): $(OBJECTS)
	@echo Building PJIT...
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
