unexport 

OUTPUT  := make_m68k
SOURCES := $(wildcard *.c)
OBJECTS := $(patsubst %.c,obj/%.o, $(SOURCES))

CFLAGS  := \
	-g -Wall -Werror -Wno-misleading-indentation \
	-Wno-format-security -Wuninitialized

CC      := gcc

.PHONY: all
all: premake $(OUTPUT)

$(OUTPUT): $(OBJECTS)
	@echo
	@echo !!!!!!!! Building opcodes...
	@echo
	$(CXX) $(CFLAGS) $(OBJECTS) -o $@
	./$(OUTPUT)
	
.PHONY: premake
premake:
	@mkdir -p obj

obj/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -rf *.s 2>/dev/null || true
	@rm -rf obj/*.o 2>/dev/null || true
	@rm -f $(OUTPUT) 2>/dev/null || true
	@rmdir obj 2>/dev/null || true
