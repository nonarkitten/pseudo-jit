# What are we doing?
ifeq ($(MAKECMDGOALS),clean)
BUILD := *
else ifdef RELEASE
BUILD := release
else
BUILD := debug
endif

# Set noisey output
ifndef VERBOSE
QUIET := @
endif

export

.PHONY: all clean

all:
	@echo Building PJIT...
	@make -C pru -f pru.mk
	@make -j 8 -f pjit.mk

clean:
	@echo Cleaning PJIT...
	@make -C pru -f pru.mk clean
	@make -j 8 -f pjit.mk clean

-include $(addprefix $(OBJDIR)/,$(notdir $(filter %.d,$(SOURCES:%.c=%.d))))
