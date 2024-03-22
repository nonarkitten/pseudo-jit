OUTPUT := buffee-pru.out

CG_TOOL_ROOT := /Applications/ti/ti-cgt-pru_2.3.3

CMD_SRCS := AM335x_PRU.cmd

C_SRCS := main.c 

RM := rm -f
RMDIR := rm -rf

CLPRU_OPTS := \
    -v3 -O4 --opt_for_speed=2 --define=am3359 --define=pru0 -g \
    --diag_warning=225 --diag_wrap=off --display_error_number \
    --endian=little --hardware_mac=on --asm_listing

CC_CLPRU_OPTS := $(CLPRU_OPTS) \
    --include_path=$(CG_TOOL_ROOT)/pru/include \
    --include_path=$(CG_TOOL_ROOT)/include \
    --include_path=inc \
    --include_path=. \
    --preproc_with_compile \
    $(GEN_OPTS__FLAG)

LD_CLPRU_OPTS := $(C_SRCS:%.c=%.obj) \
    --library=$(CG_TOOL_ROOT)/lib/rtspruv1_le.lib \
    --library=$(CG_TOOL_ROOT)/lib/libc.a \
    --library=$(CMD_SRCS) --rom_model

.PHONY: all clean post-build

# All Target
all: $(OUTPUT)

# Tool invocations
$(OUTPUT): $(C_SRCS:%.c=%.obj)
	@echo 'Building target: "$@"'
	$(CG_TOOL_ROOT)/bin/lnkpru $(LD_CLPRU_OPTS) --output_file=$@ --map_file=$(@:%.out=%.map)
	$(CG_TOOL_ROOT)/bin/hexpru  -o "buffee-pru.hex" $(OUTPUT) bin.cmd
	-@gsed '$$ s/\x00*$$//' text.bin >pru_text.bin
	-@xxd -i -a pru_text.bin >../hal/pru_text.c
	-@gsed '$$ s/\x00*$$//' data.bin >pru_data.bin
	-@xxd -i -a pru_data.bin >../hal/pru_data.c
	-@echo ' '

%.obj: %.c
	@echo 'Building file: "$<"'
	$(CG_TOOL_ROOT)/bin/clpru $(CC_CLPRU_OPTS) $<
	
clean:
	-$(RM) $(C_SRCS:%.c=%.obj)
	-$(RM) $(C_SRCS:%.c=%.d)
	-$(RM) $(C_SRCS:%.c=%.lst)
	-$(RM) $(OUTPUT:%.out=%.*)
	-$(RM) text.bin pru_text.bin
	-$(RM) data.bin pru_data.bin
	-$(RM) ../pru_text.h
	-$(RM) ../pru_data.h
	-@echo 'Finished clean'
	-@echo ' '

