# All critical output, source folders
OUTDIR  := binaries
BINARY  := pjit.elf
SRCDIRS := . clib/ hal/ pjit/ src/
CROSS   := arm-none-eabi-

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

# Compiler optimizations
ifeq ($(BUILD),release)
# Optimize for release and raise the bar for errors
CCFLAGS_OPT := -O3 -ffast-math -Werror -Wno-clobbered -s
else
# Optimize for debugging, but include basic optimizers for not-terrible-code
CCFLAGS_OPT := -Og -ftree-vectorize -D_DEBUG
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
	-Wl,--Map=$(MAPFILE)

INCLUDES := -I./inc $(addprefix -I./,$(SRCDIRS))
SOURCES := $(foreach dir,$(SRCDIRS),$(wildcard $(dir)/*.c) $(wildcard $(dir)/*.s))
OBJECTS := $(addprefix $(OBJDIR)/,$(addsuffix .o,$(basename $(foreach file,$(SOURCES),$(notdir $(file))))))

VPATH := $(SRCDIRS)

.PHONY: all premake clean debug release

all: premake $(OUTPUT)

clean:
	$(RM) $(OBJDIR)/* 2>/dev/null || true
	$(RD) $(OBJDIR) 2>/dev/null || true
	$(RM) $(OUTPUT) 2>/dev/null || true
	$(RM) $(BINARY) 2>/dev/null || true
	$(RM) $(MAPFILE) 2>/dev/null || true	
	$(RM) $(MAPFILE).bak 2>/dev/null || true	
	$(RM) $(DISFILE) 2>/dev/null || true
	$(RM) $(OUTDIR)/$(BUILD) 2>/dev/null || true

premake:
	$(MD) $(OBJDIR)

$(OUTPUT): $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o $@
	$(DI) $@ > $(DISFILE) &
	$(OC) -O binary $@ $(BINARY)
	$(SZ) $@
	$(QUIET)nm --print-size --size-sort --radix=x $@ | grep -v -E "\.debug|\.group" > $(@:%.elf=%.sz)

$(OBJDIR)/%.o: %.c
	@echo Compiling $<...
	$(CC) $(CFLAGS) $(INCLUDES) -S -c $< -o $(@:%.o=%.s) 2>/dev/null &
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJDIR)/%.o: %.s
	@echo Assembling $<...
	$(AS) $(CFLAGS) -c $< -o $@

-include $(addprefix $(OBJDIR)/,$(notdir $(filter %.d,$(SOURCES:%.c=%.d))))
