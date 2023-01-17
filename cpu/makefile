ifndef SUBMAKE
$(error SUBMAKE not set, run make from root folder)
endif

OUTPUT  := ../obj/libpjit.a
SOURCES  := $(wildcard *.c) pjit_support.s ops/m68k_da.c
OBJECTS  := \
	$(patsubst %.s,obj/%.o, \
	$(patsubst %.c,obj/%.o, \
	$(SOURCES) \
	))))

.PHONY: all
all: premake $(OUTPUT)

$(OUTPUT): $(OBJECTS)
	@echo
	@echo !!!!!!!! Building PJIT core...
	@echo
	$(AR) rcs $@ $(OBJECTS)
	$(SZ) $@

obj/%.o: %.c
	$(CC) $(CCFLAGS) -c $< -o $@

obj/%.o: ops/%.c
	$(CC) $(CCFLAGS) -c $< -o $@

obj/%.o: %.s
	$(AS) $(CCFLAGS) -c $< -o $@

obj/%.o: ops/%.s
	$(AS) $(CCFLAGS) -c $< -o $@

.PHONY: premake
premake:
	@mkdir -p obj
	
.PHONY: clean
clean:
	-@make -C ops clean
	-@rm -rf obj/*
	-@rmdir obj
	-@rm $(OUTPUT)