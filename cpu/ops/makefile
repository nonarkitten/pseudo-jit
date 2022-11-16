unexport

OUTPUT  := make_m68k

CFLAGS  := -Og -Wall -Werror -Wno-misleading-indentation \
	-Wno-format-security -Wuninitialized

CC      := gcc

.PHONY: all clean	
all: $(OUTPUT)

$(OUTPUT):
	@echo
	@echo !!!!!!!! Building opcodes...
	@echo
	$(CC) $(CFLAGS) $(wildcard *.c) -o $(OUTPUT)
	./$(OUTPUT)

clean:
	@rm -rf *.s 2>/dev/null || true
	@rm -rf pjit_extword.h 2>/dev/null || true
	@rm -f $(OUTPUT) 2>/dev/null || true
