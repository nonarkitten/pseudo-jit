
SOURCES  := $(wildcard generated/*.c)
OBJECTS  := $(patsubst generated/%.c,obj/%.o, $(SOURCES))

INCLUDE  := -I./inc

DEFINES :=
COMMON_FLAGS := $(EXTRA_FLAGS) -Iinc -mbig-endian -ffixed-r11 -marm -mcpu=cortex-a8 -Os \
	-mfpu=neon-vfpv4 -pedantic -pedantic-errors -fomit-frame-pointer \
	-Wall -Wextra -Werror -nostdlib -ffreestanding -g -fmax-errors=5

CFLAGS   := $(COMMON_FLAGS) $(DEFINES) -std=gnu11
CXXFLAGS := $(COMMON_FLAGS) $(DEFINES) -std=c++11
CC       := gcc $(FLAGS) $(INCLUDE) $(DEFINES) -Wall
LDFLAGS  := -static -nostdlib -nostartfiles -T linker.lds \
	-Wl,--Map=$(APPNAME).map \
	-Wl,--build-id \
	-Wl,--be8 \
	-Wl,--format=elf32-bigarm

.PHONY: all
all: premake  $(OBJECTS)
	$(CC) -s $(BBB_OBJECTS) -o $(APPNAME).elf $(LDFLAGS)

.PHONY: premake clean
premake:
	@mkdir -p bin obj
	@$(CC) src/makeops.c -o bin/makeops
	@bin/makeops

%.o:../generated/%.c
	@$(CC) -c $< -o $@

clean:
	rm -vrf generated/*
	rm -vrf obj/*
	rm -vrf bin/*

.PHONY: rebuild
rebuild: clean all






