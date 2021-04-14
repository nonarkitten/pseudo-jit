################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Add inputs and outputs from these tool invocations to the build variables 
LDS_SRCS += \
../AM335x.lds 

S_SRCS += \
../bogo_delay.s \
../startup.s 

C_SRCS += \
../bogomips_arm.c \
../bogomips_pjit.c \
../main.c 

S_DEPS += \
./bogo_delay.d \
./startup.d 

C_DEPS += \
./bogomips_arm.d \
./bogomips_pjit.d \
./main.d 

OBJS += \
./bogo_delay.o \
./bogomips_arm.o \
./bogomips_pjit.o \
./main.o \
./startup.o 

OBJS__QUOTED += \
"bogo_delay.o" \
"bogomips_arm.o" \
"bogomips_pjit.o" \
"main.o" \
"startup.o" 

C_DEPS__QUOTED += \
"bogomips_arm.d" \
"bogomips_pjit.d" \
"main.d" 

S_DEPS__QUOTED += \
"bogo_delay.d" \
"startup.d" 

S_SRCS__QUOTED += \
"../bogo_delay.s" \
"../startup.s" 

C_SRCS__QUOTED += \
"../bogomips_arm.c" \
"../bogomips_pjit.c" \
"../main.c" 


