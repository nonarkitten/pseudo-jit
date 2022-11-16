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
	-@rm -rf *.s
	-@rm -rf pjit_extword.h
	-@rm -f $(OUTPUT)
