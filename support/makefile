ifndef SUBMAKE
$(error SUBMAKE not set, run make from root folder)
endif

OUTPUT  := ../obj/libsupport.a
SOURCES  := $(wildcard *.s) $(wildcard *.c)
OBJECTS  := \
	$(patsubst %.s,obj/%.o, \
	$(patsubst %.c,obj/%.o, \
	$(SOURCES) \
	))

.PHONY: all
all: premake $(OUTPUT)

$(OUTPUT): $(OBJECTS)
	@echo
	@echo !!!!!!!! Building PJIT support...
	@echo
	$(AR) rcs $@ $(OBJECTS)
	$(SZ) $@

obj/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

obj/%.o: %.s
	$(AS) $(CFLAGS) -c $< -o $@

.PHONY: premake
premake:
	@mkdir -p obj
	
.PHONY: clean
clean:
	@rm -rf obj/* 2>/dev/null || true
	@rm $(OUTPUT) 2>/dev/null || true