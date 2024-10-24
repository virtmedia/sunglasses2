################################################################################
# MRS Version: 1.9.2
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include lib/subdir.mk
-include User/subdir.mk
-include Startup/subdir.mk
-include Peripheral/src/subdir.mk
-include Debug/subdir.mk
-include Core/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
SECONDARY_FLASH += \
sunglasses2.hexAndbin \

SECONDARY_LIST += \
sunglasses2.lst \

SECONDARY_SIZE += \
sunglasses2.siz \


# All Target
all: sunglasses2.elf secondary-outputs

# Tool invocations
sunglasses2.elf: $(OBJS) $(USER_OBJS)
	@	@	riscv-none-embed-gcc -march=rv32ecxw -mabi=ilp32e -msmall-data-limit=0 -msave-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized  -g -T "J:\elektronika\Projekty\Wlasne\MounRiver\sunglasses2\Ld\Link.ld" -nostartfiles -Xlinker --gc-sections -Wl,-Map,"sunglasses2.map" --specs=nano.specs --specs=nosys.specs -o "sunglasses2.elf" $(OBJS) $(USER_OBJS) $(LIBS) -lprintf
	@	@
sunglasses2.hexAndbin: sunglasses2.elf
	@	riscv-none-embed-objcopy -O ihex "sunglasses2.elf"  "sunglasses2.hex" && riscv-none-embed-objcopy -O binary "sunglasses2.elf"  "sunglasses2.bin"
	@	@
sunglasses2.lst: sunglasses2.elf
	@	riscv-none-embed-objdump --all-headers --demangle --disassemble -M xw "sunglasses2.elf" > "sunglasses2.lst"
	@	@
sunglasses2.siz: sunglasses2.elf
	@	riscv-none-embed-size --format=berkeley "sunglasses2.elf"
	@	@
	$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) $(ASM_UPPER_DEPS)$(OBJS)$(SECONDARY_FLASH)$(SECONDARY_LIST)$(SECONDARY_SIZE)$(ASM_DEPS)$(S_DEPS)$(S_UPPER_DEPS)$(C_DEPS) sunglasses2.elf
	-@
post-build:
	-riscv-none-embed-objcopy -O ihex "sunglasses2.elf" "sunglasses2.hex" && riscv-none-embed-objcopy -O binary "sunglasses2.elf" "sunglasses2.bin"
	-@
secondary-outputs: $(SECONDARY_FLASH) $(SECONDARY_LIST) $(SECONDARY_SIZE)

.PHONY: all clean dependents post-build

-include ../makefile.targets
