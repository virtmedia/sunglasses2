################################################################################
# MRS Version: 1.9.2
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../lib/lis2dw12_reg.c 

OBJS += \
./lib/lis2dw12_reg.o 

C_DEPS += \
./lib/lis2dw12_reg.d 


# Each subdirectory must supply rules for building sources it contributes
lib/%.o: ../lib/%.c
	@	@	riscv-none-embed-gcc -march=rv32ecxw -mabi=ilp32e -msmall-data-limit=0 -msave-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized  -g -I"J:\elektronika\Projekty\Wlasne\MounRiver\sunglasses2\Debug" -I"J:\elektronika\Projekty\Wlasne\MounRiver\sunglasses2\lib" -I"J:\elektronika\Projekty\Wlasne\MounRiver\sunglasses2\Core" -I"J:\elektronika\Projekty\Wlasne\MounRiver\sunglasses2\User" -I"J:\elektronika\Projekty\Wlasne\MounRiver\sunglasses2\Peripheral\inc" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@	@
