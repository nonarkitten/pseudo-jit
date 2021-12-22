APPNAME  := m68k_test
SOURCES  := $(wildcard *.c) $(wildcard ops/*.s) pjit_support.s
OBJECTS  := $(patsubst %.c,obj/%.o, $(patsubst %.s,obj/%.o, $(patsubst ops/%.s,obj/%.o, $(SOURCES))))

AS = arm-none-eabi-as \
	-mfloat-abi=hard -mcpu=cortex-a8 -march=armv7-a -mfpu=vfpv3
SZ = arm-none-eabi-size
CC = arm-none-eabi-gcc -I . -I ops -Os \
	-flto-compression-level=9 \
	-mfloat-abi=hard -march=armv7-a -marm -mfpu=vfpv3 -mtune=cortex-a8 \
	-ffunction-sections \
	-fdata-sections \
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

.PHONY: all
all: premake $(OBJECTS)
	$(CC) -Wl,--gc-sections -s $(OBJECTS) -o $(APPNAME) $(LIBPATH) $(LIBS)
	$(SZ) $(APPNAME)

obj/%.o: %.c
	$(CC) -c $< -o $@

obj/%.o: ops/%.s
	$(AS) $< -o $@

obj/%.o: %.s
	$(AS) $< -o $@

.PHONY: premake
premake:
	mkdir -p obj