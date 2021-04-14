################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
libbbb/src/%.o: ../libbbb/src/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: GNU Compiler'
	"C:/ti/ccs1020/ccs/tools/compiler/gcc-arm-none-eabi-9-2019-q4-major/bin/arm-none-eabi-gcc-9.2.1.exe" -c -mcpu=cortex-a8 -march=armv7-a -mtune=cortex-a8 -marm -mfloat-abi=softfp -mfpu=neon-vfpv3 -Dam3358 -DDEBUG -I"D:/Workspaces/CCS10/pjit" -I"C:/ti/ccs1020/ccs/tools/compiler/gcc-arm-none-eabi-9-2019-q4-major/arm-none-eabi/include" -I"D:/Workspaces/CCS10/pjit/libbbb/inc" -I"D:/Workspaces/CCS10/pjit/support" -Os -ffunction-sections -fdata-sections -g -gdwarf-3 -gstrict-dwarf -Wall -specs="rdimon.specs" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)"  -mno-long-calls $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

libbbb/src/%.o: ../libbbb/src/%.S $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: GNU Compiler'
	"C:/ti/ccs1020/ccs/tools/compiler/gcc-arm-none-eabi-9-2019-q4-major/bin/arm-none-eabi-gcc-9.2.1.exe" -c -mcpu=cortex-a8 -march=armv7-a -mtune=cortex-a8 -marm -mfloat-abi=softfp -mfpu=neon-vfpv3 -Dam3358 -DDEBUG -I"D:/Workspaces/CCS10/pjit" -I"C:/ti/ccs1020/ccs/tools/compiler/gcc-arm-none-eabi-9-2019-q4-major/arm-none-eabi/include" -I"D:/Workspaces/CCS10/pjit/libbbb/inc" -I"D:/Workspaces/CCS10/pjit/support" -Os -ffunction-sections -fdata-sections -g -gdwarf-3 -gstrict-dwarf -Wall -specs="rdimon.specs" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


