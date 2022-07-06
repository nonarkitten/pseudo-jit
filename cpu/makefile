OUTPUT  := ../obj/pjit_core.o
SOURCES  := $(wildcard ops/*.s) $(wildcard *.c) pjit_support.s ops/m68k_da.c
OBJECTS  := \
	$(patsubst %.s,obj/%.o, \
	$(patsubst %.c,obj/%.o, \
	$(patsubst ops/%.s,obj/%.o, \
	$(patsubst ops/%.c,obj/%.o, \
	$(SOURCES) \
	))))

AS = arm-none-eabi-gcc -ggdb \
	-mfloat-abi=hard -mcpu=cortex-a8 -march=armv7-a -mfpu=vfpv3
SZ = arm-none-eabi-size
CC = arm-none-eabi-gcc -ggdb -I . -I ops -Os \
	-flto-compression-level=9 \
	-mfloat-abi=hard -march=armv7-a -marm -mfpu=vfpv3 -mtune=cortex-a8 \
	-ffunction-sections \
	-fdata-sections \
	-fno-exceptions \
	-fomit-frame-pointer \
	-ffast-math \
	-fno-common \
	-ffixed-r4 \
	-ffixed-r5 \
	-ffixed-r6 \
	-ffixed-r7 \
	-ffixed-r8 \
	-ffixed-r9 \
	-ffixed-r10 \
	-ffixed-r11 \
	-ffixed-r12

LIBS = 

.PHONY: all
all: premake $(OUTPUT)

$(OUTPUT): $(OBJECTS)
	$(CC) -Wl,--relocatable $(OBJECTS) $(LIBPATH) $(LIBS) -o $@
	$(SZ) $@

obj/%.o: %.c
	$(CC) -c $< -o $@

obj/%.o: ops/%.c
	$(CC) -c $< -o $@

obj/%.o: %.s
	$(AS) -c $< -o $@

obj/%.o: ops/%.s
	$(AS) -c $< -o $@

.PHONY: premake
premake:
	@make -C ops
	@mkdir -p obj
	
.PHONY: clean
clean:
	rm -rf obj/*
	rm $(OUTPUT)