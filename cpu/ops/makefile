OUTPUT  := make_m68k
SOURCES := $(wildcard *.c)
OBJECTS := $(patsubst %.c,%.o, $(SOURCES))

CFLAGS  := -g -Wall -Werror

.PHONY: all
all: $(OBJECTS)
	$(CXX) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(OBJECTS) -o $(OUTPUT)
	
%: %.c
	$(CC) $(CFLAGS) $(CCFLAGS) $(INCLUDE) $< -o $@
	$(OUTPUT)

clean:
	rm -rf *.s
	rm -rf *.o
	rm -f $(OUTPUT)