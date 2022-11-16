ifndef SUBMAKE
$(error SUBMAKE not set, run make from root folder)
endif

OUTPUT  := ../obj/libpjit.a
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
	@make -C ops clean 2>/dev/null || true
	@rm -rf obj/* 2>/dev/null || true
	@rm $(OUTPUT) 2>/dev/null || true