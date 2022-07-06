ifndef SUBMAKE
$(error SUBMAKE not set, run make from root folder)
endif

OUTPUT  := ../obj/pjit_core.o
SOURCES  := $(wildcard ops/*.s) $(wildcard *.c) pjit_support.s ops/m68k_da.c
OBJECTS  := \
	$(patsubst %.s,obj/%.o, \
	$(patsubst %.c,obj/%.o, \
	$(patsubst ops/%.s,obj/%.o, \
	$(patsubst ops/%.c,obj/%.o, \
	$(SOURCES) \
	))))

.PHONY: all
all: premake $(OUTPUT)

$(OUTPUT): $(OBJECTS)
	$(CC) -Wl,--relocatable $(OBJECTS) -o $@
	$(SZ) $@

obj/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

obj/%.o: ops/%.c
	$(CC) $(CFLAGS) -c $< -o $@

obj/%.o: %.s
	$(AS) $(CFLAGS) -c $< -o $@

obj/%.o: ops/%.s
	$(AS) $(CFLAGS) -c $< -o $@

.PHONY: premake
premake:
	@echo
	@echo !!!!!!!! Building PJIT core...
	@echo
	@mkdir -p obj
	
.PHONY: clean
clean:
	@make -C ops clean 2>/dev/null || true
	@rm -rf obj/* 2>/dev/null || true
	@rm $(OUTPUT) 2>/dev/null || true