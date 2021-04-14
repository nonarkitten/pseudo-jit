################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

CG_TOOL_ROOT := C:/ti/ccs1020/ccs/tools/compiler/gcc-arm-none-eabi-9-2019-q4-major

GEN_OPTS__FLAG := 
GEN_CMDS__FLAG := 

ORDERED_OBJS += \
"./bogo_delay.o" \
"./bogomips_arm.o" \
"./bogomips_pjit.o" \
"./main.o" \
"./startup.o" \
"./libbbb/src/am335x_clock.o" \
"./libbbb/src/am335x_dmtimer1.o" \
"./libbbb/src/am335x_epwm.o" \
"./libbbb/src/am335x_gpio.o" \
"./libbbb/src/am335x_i2c.o" \
"./libbbb/src/am335x_mux.o" \
"./libbbb/src/am335x_spi.o" \
"./libbbb/src/am335x_uart.o" \
"./libbbb/src/bbb_fs.o" \
"./libbbb/src/bbb_syscall.o" \
"./libbbb/src/console.o" \
"./libbbb/src/dirent.o" \
"./libbbb/src/oled.o" \
"./libbbb/src/path.o" \
"./libbbb/src/string2.o" \
"./libbbb/src/vfs.o" \
"./support/cp15.o" \
"./support/cstd.o" \
"./support/init_hw.o" \
"./support/printf.o" \
"./support/sh_printstr.o" \
-Wl,-T"../AM335x.lds" \
$(GEN_CMDS__FLAG) \

-include ../makefile.init

RM := DEL /F
RMDIR := RMDIR /S/Q

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir_vars.mk
-include libbbb/src/subdir_vars.mk
-include support/subdir_vars.mk
-include subdir_rules.mk
-include libbbb/src/subdir_rules.mk
-include support/subdir_rules.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C55_DEPS)),)
-include $(C55_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(S67_DEPS)),)
-include $(S67_DEPS)
endif
ifneq ($(strip $(S62_DEPS)),)
-include $(S62_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(OPT_DEPS)),)
-include $(OPT_DEPS)
endif
ifneq ($(strip $(C??_DEPS)),)
-include $(C??_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(S??_DEPS)),)
-include $(S??_DEPS)
endif
ifneq ($(strip $(C64_DEPS)),)
-include $(C64_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(S64_DEPS)),)
-include $(S64_DEPS)
endif
ifneq ($(strip $(INO_DEPS)),)
-include $(INO_DEPS)
endif
ifneq ($(strip $(CLA_DEPS)),)
-include $(CLA_DEPS)
endif
ifneq ($(strip $(S55_DEPS)),)
-include $(S55_DEPS)
endif
ifneq ($(strip $(SV7A_DEPS)),)
-include $(SV7A_DEPS)
endif
ifneq ($(strip $(C62_DEPS)),)
-include $(C62_DEPS)
endif
ifneq ($(strip $(C67_DEPS)),)
-include $(C67_DEPS)
endif
ifneq ($(strip $(PDE_DEPS)),)
-include $(PDE_DEPS)
endif
ifneq ($(strip $(K_DEPS)),)
-include $(K_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C43_DEPS)),)
-include $(C43_DEPS)
endif
ifneq ($(strip $(S43_DEPS)),)
-include $(S43_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(SA_DEPS)),)
-include $(SA_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXE_OUTPUTS += \
pjit.out \

EXE_OUTPUTS__QUOTED += \
"pjit.out" \

BIN_OUTPUTS += \
pjit.bin \

BIN_OUTPUTS__QUOTED += \
"pjit.bin" \


# All Target
all: $(OBJS) $(LDS_SRCS) $(GEN_CMDS)
	@$(MAKE) --no-print-directory -Onone "pjit.out" secondary-outputs

# Tool invocations
pjit.out: $(OBJS) $(LDS_SRCS) $(GEN_CMDS)
	@echo 'Building target: "$@"'
	@echo 'Invoking: GNU Linker'
	"C:/ti/ccs1020/ccs/tools/compiler/gcc-arm-none-eabi-9-2019-q4-major/bin/arm-none-eabi-gcc-9.2.1.exe" -mtune=cortex-a8 -mfpu=neon-vfpv3 -Dam3358 -DDEBUG -Os -ffunction-sections -fdata-sections -g -gdwarf-3 -gstrict-dwarf -Wall -specs="rdimon.specs" -mcpu=cortex-a8 -Wl,-Map,"pjit.map" -nostartfiles -nodefaultlibs -nostdlib -Wl,--defsym,STACKSIZE=0x1C000 -Wl,--defsym,HEAPSIZE=0x400 -o"pjit.out" $(ORDERED_OBJS)
	@echo 'Finished building target: "$@"'
	@echo ' '

pjit.bin: $(EXE_OUTPUTS)
	@echo 'Building files: $(strip $(EXE_OUTPUTS__QUOTED))'
	@echo 'Invoking: GNU Objcopy Utility'
	"C:/ti/ccs1020/ccs/tools/compiler/gcc-arm-none-eabi-9-2019-q4-major/bin/arm-none-eabi-objcopy.exe" -O binary $(EXE_OUTPUTS__QUOTED) "pjit.bin"
	@echo 'Finished building: $(strip $(EXE_OUTPUTS__QUOTED))'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(BIN_OUTPUTS__QUOTED)$(EXE_OUTPUTS__QUOTED)
	-$(RM) "libbbb\src\string2.d" 
	-$(RM) "bogo_delay.o" "bogomips_arm.o" "bogomips_pjit.o" "main.o" "startup.o" "libbbb\src\am335x_clock.o" "libbbb\src\am335x_dmtimer1.o" "libbbb\src\am335x_epwm.o" "libbbb\src\am335x_gpio.o" "libbbb\src\am335x_i2c.o" "libbbb\src\am335x_mux.o" "libbbb\src\am335x_spi.o" "libbbb\src\am335x_uart.o" "libbbb\src\bbb_fs.o" "libbbb\src\bbb_syscall.o" "libbbb\src\console.o" "libbbb\src\dirent.o" "libbbb\src\oled.o" "libbbb\src\path.o" "libbbb\src\string2.o" "libbbb\src\vfs.o" "support\cp15.o" "support\cstd.o" "support\init_hw.o" "support\printf.o" "support\sh_printstr.o" 
	-$(RM) "bogomips_arm.d" "bogomips_pjit.d" "main.d" "libbbb\src\am335x_clock.d" "libbbb\src\am335x_dmtimer1.d" "libbbb\src\am335x_epwm.d" "libbbb\src\am335x_gpio.d" "libbbb\src\am335x_i2c.d" "libbbb\src\am335x_mux.d" "libbbb\src\am335x_spi.d" "libbbb\src\am335x_uart.d" "libbbb\src\bbb_fs.d" "libbbb\src\bbb_syscall.d" "libbbb\src\console.d" "libbbb\src\dirent.d" "libbbb\src\oled.d" "libbbb\src\path.d" "libbbb\src\vfs.d" "support\cstd.d" "support\init_hw.d" "support\printf.d" 
	-$(RM) "bogo_delay.d" "startup.d" "support\cp15.d" "support\sh_printstr.d" 
	-@echo 'Finished clean'
	-@echo ' '

secondary-outputs: $(BIN_OUTPUTS)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets

