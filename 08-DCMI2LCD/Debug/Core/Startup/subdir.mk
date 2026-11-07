################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_SRCS += \
../Core/Startup/startup_stm32h723vgtx.s 

OBJS += \
./Core/Startup/startup_stm32h723vgtx.o 

S_DEPS += \
./Core/Startup/startup_stm32h723vgtx.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Startup/%.o: ../Core/Startup/%.s Core/Startup/subdir.mk
	arm-none-eabi-gcc -mcpu=cortex-m7 -g3 -DDEBUG -c -I"/home/busoye_tm/Documents/tinyml/intrusion_detection_system/08-DCMI2LCD/Bsp/Camera" -I"/home/busoye_tm/Documents/tinyml/intrusion_detection_system/08-DCMI2LCD/Bsp/ST7735" -I"/home/busoye_tm/Documents/tinyml/intrusion_detection_system/08-DCMI2LCD/Bsp" -x assembler-with-cpp -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@" "$<"

clean: clean-Core-2f-Startup

clean-Core-2f-Startup:
	-$(RM) ./Core/Startup/startup_stm32h723vgtx.d ./Core/Startup/startup_stm32h723vgtx.o

.PHONY: clean-Core-2f-Startup

