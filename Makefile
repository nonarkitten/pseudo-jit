
SOURCES := $(wildcard generated/*.c)
OBJECTS := $(patsubst generated/%.c,obj/%.o, $(SOURCES))

INCLUDE := -I./inc

CC       := gcc $(FLAGS) $(INCLUDE) $(DEFINES) -Wall

.PHONY: all
all: premake  $(OBJECTS)
	$(CC) src/pjit.c -c -o obj/pjit.o

.PHONY: premake
premake:
	@mkdir -p bin obj
	@$(CC) src/makeops.c -o bin/makeops
	@bin/makeops

%.o:../generated/%.c
	@$(CC) -c $< -o $@

.PHONY: clean
clean:
	rm -vrf generated/*
	rm -vrf obj/*
	rm -vrf bin/*

.PHONY: rebuild
rebuild: clean all






