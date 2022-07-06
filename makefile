COMMON_FLAGS := $(EXTRA_FLAGS) -Iinc -mbig-endian -ffixed-r11 -marm -mcpu=cortex-a8 -Os \
	-mfpu=vfpv3 -pedantic -pedantic-errors -fomit-frame-pointer -mfloat-abi=hard \
	-ffixed-r3 -ffixed-r4 -ffixed-r5 -ffixed-r6 -ffixed-r7 \
	-ffixed-r8 -ffixed-r9 -ffixed-r10 -ffixed-r11 -ffixed-r12 \
	-ffunction-sections -fdata-sections -ffast-math -fno-exceptions \
	-Wall -Wextra -Werror -nostdlib -ffreestanding -g -fmax-errors=5

DEFINES :=

CFLAGS := $(COMMON_FLAGS) $(DEFINES) -std=gnu11
CXXFLAGS:= $(COMMON_FLAGS) $(DEFINES) -std=c++11

SUBMAKE := 1

AS := arm-none-eabi-gcc
AR := arm-none-eabi-ar
SZ := arm-none-eabi-size
CC := arm-none-eabi-gcc
OC := arm-none-eabi-objcopy

export COMMON_FLAGS DEFINES CFLAGS CXXFLAGS AS AR SZ CC SUBMAKE

OUTPUT  := binaries/pjit.elf
BINARY  := $(patsubst %.elf,%.bin,$(OUTPUT))
MAPFILE := $(patsubst %.elf,%.map,$(OUTPUT))

LDFLAGS := -static -nostdlib -nostartfiles -T linker.lds \
	-Wl,--gc-sections \
	-Wl,--Map=$(MAPFILE) \
	-Wl,--build-id \
	-Wl,--be8 \
	-Wl,--format=elf32-bigarm

SOURCES := $(wildcard *.s) $(wildcard *.c) 
OBJECTS := \
	$(patsubst %.s,obj/%.o, \
	$(patsubst %.c,obj/%.o, \
	$(SOURCES) \
	))

ALLLIBS = -Lobj -lpjit -lsupport

.PHONY: all
all: premake $(OUTPUT)

.PHONY: premake
premake:
	@make -C cpu/ops
	@make -C cpu
	@make -C support
	@mkdir -p obj

$(OUTPUT): $(OBJECTS)
	@echo
	@echo !!!!!!!! Building PJIT ...
	@echo
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o $@ $(ALLLIBS)
	$(OC) --gap-fill=0xff -O binary $@ $(BINARY)
	$(SZ) $@

obj/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

obj/%.o: %.s
	$(AS) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	@make -C cpu/ops clean
	@make -C cpu clean
	@make -C support clean
	@rm -rf obj/* 2>/dev/null || true
	@rm $(OUTPUT) 2>/dev/null || true
	@rm $(BINARY) 2>/dev/null || true
	@rm $(MAPFILE) 2>/dev/null || true
	