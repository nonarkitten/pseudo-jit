# Compiler optimizations
ifdef ($(DEBUG))
# Optimize for debugging, but include basic optimizers for not-terrible-code
CCFLAGS_OPT := -Og -g3 -ggdb -D_DEBUG
else
# Optimize for release and raise the bar for errors
CCFLAGS_OPT := -Os -Werror -s
endif

# Universally good compiler warnings	
CCFLAGS_WARN := \
	-Wall -Wshadow -Wdouble-promotion -Wformat-overflow -Wformat-truncation \
	-Wundef -Wno-unused-parameter

CCFLAGS := $(CCFLAGS_OPT) $(CCFLAGS_WARN) \
	-I. -I./inc -I./cpu -I./cpu/ops -mbig-endian -marm -mcpu=cortex-a8 \
	-mfpu=vfpv3 -mfloat-abi=hard  -ffast-math -ffixed-r5  \
	-ffunction-sections -fdata-sections -fno-exceptions -fomit-frame-pointer \
	-nostdlib -ffreestanding -g -fmax-errors=5

SUBMAKE := 1
AS := arm-none-eabi-gcc
AR := arm-none-eabi-ar
SZ := arm-none-eabi-size
CC := arm-none-eabi-gcc
OC := arm-none-eabi-objcopy

export COMMON_FLAGS DEFINES CCFLAGS AS AR SZ CC SUBMAKE

OUTPUT  := binaries/pjit.elf

BINARY  := $(patsubst %.elf,%.bin,$(OUTPUT))
MAPFILE := $(patsubst %.elf,%.map,$(OUTPUT))

LDFLAGS := -static -nostdlib -nostartfiles -T linker.lds \
	-Wl,--gc-sections \
	-Wl,--Map=$(MAPFILE) \
	-Wl,--build-id \
	-Wl,--be8 \
	-Wl,--format=elf32-bigarm

SRCDIRS := .

SOURCES += $(foreach dir,$(SRCDIRS),$(wildcard $(dir)/*.c) $(wildcard $(dir)/*.S))
OBJECTS += $(addprefix obj/,$(addsuffix .o,$(basename $(foreach file,$(SOURCES),$(notdir $(file))))))
VPATH += $(SRCDIRS)

ALLLIBS = -Lobj -lpjit -lsupport

.PHONY: premake all clean

all: premake $(OUTPUT)

premake:
	@mkdir -p obj
	@make -C cpu/ops
	@make -C cpu
	@make -C support

$(OUTPUT): $(OBJECTS)
	@echo
	@echo !!!!!!!! Building PJIT ...
	@echo
	$(CC) $(CCFLAGS) $(LDFLAGS) $(OBJECTS) -o $@ $(ALLLIBS)
	$(OC) --gap-fill=0xff -O binary $@ $(BINARY)
	$(SZ) $@

obj/%.o: %.c
	$(CC) $(CCFLAGS) -c $< -o $@

obj/%.o: %.S
	$(AS) $(CCFLAGS) -c $< -o $@

clean:
	-@make -C cpu/ops clean
	-@make -C cpu clean
	-@make -C support clean
	-@rm -rf $(OBJECTS)
	-@rmdir obj
	-@rm $(OUTPUT)
	-@rm $(BINARY)
	-@rm $(MAPFILE)
	