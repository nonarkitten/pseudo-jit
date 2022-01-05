OUTPUT  := make_m68k
SOURCES := $(wildcard *.c)
OBJECTS := $(patsubst %.c,obj/%.o, $(SOURCES))

CFLAGS  := -g -Wall -Werror -Wno-misleading-indentation
CC      := gcc
CXX     := g++

.PHONY: all
all: premake $(OBJECTS)
	$(CXX) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(OBJECTS) -o $(OUTPUT)
	./$(OUTPUT)
	
.PHONY: premake
premake:
	mkdir -p obj

obj/%.o: %.c
	$(CC) $(CFLAGS) $(CCFLAGS) $(INCLUDE) -c $< -o $@

clean:
	rm -rf *.s
	rm -rf obj/*.o
	rm -f $(OUTPUT)
	rmdir obj
