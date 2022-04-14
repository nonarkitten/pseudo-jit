APPNAME  := m68k_test
# SOURCES  := $(wildcard ops/*.s) $(wildcard *.c) pjit_support.s ops/m68k_da.c
# OBJECTS  := \
# 	$(patsubst ops/%.s,obj/%.o, \
# 	$(patsubst %.c,obj/%.o, \
# 	$(patsubst %.s,obj/%.o, \
# 	$(patsubst ops/%.c,obj/%.o, \
# 	$(SOURCES) \
# 	))))
SOURCES  := $(wildcard ops/*.s)
OBJECTS  := $(patsubst ops/%.s,obj/%.o, $(SOURCES))

AS = arm-none-eabi-gcc -ggdb \
	-mfloat-abi=hard -mcpu=cortex-a8 -march=armv7-a -mfpu=vfpv3
SZ = sarm-none-eabi-ize
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
all: premake $(OBJECTS)
	$(CC) -Wl,--gc-sections $(OBJECTS) -o $(APPNAME) $(LIBPATH) $(LIBS)
	$(SZ) $(APPNAME)

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
	mkdir -p obj
	
.PHONY: clean
clean:
	rm -rf obj/*
	rm $(APPNAME)