################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Bsp/ST7735/lcd.c \
../Bsp/ST7735/logo_128_160.c \
../Bsp/ST7735/logo_160_80.c \
../Bsp/ST7735/st7735.c \
../Bsp/ST7735/st7735_reg.c 

OBJS += \
./Bsp/ST7735/lcd.o \
./Bsp/ST7735/logo_128_160.o \
./Bsp/ST7735/logo_160_80.o \
./Bsp/ST7735/st7735.o \
./Bsp/ST7735/st7735_reg.o 

C_DEPS += \
./Bsp/ST7735/lcd.d \
./Bsp/ST7735/logo_128_160.d \
./Bsp/ST7735/logo_160_80.d \
./Bsp/ST7735/st7735.d \
./Bsp/ST7735/st7735_reg.d 


# Each subdirectory must supply rules for building sources it contributes
Bsp/ST7735/%.o Bsp/ST7735/%.su Bsp/ST7735/%.cyclo: ../Bsp/ST7735/%.c Bsp/ST7735/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=c99 -g3 -DDEBUG -DTFT96=1 -DUSE_PWR_LDO_SUPPLY -DUSE_HAL_DRIVER -DSTM32H723xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I"/home/busoye_tm/Documents/tinyml/intrusion_detection_system/08-DCMI2LCD_tinyengine/Bsp/Camera" -I"/home/busoye_tm/Documents/tinyml/intrusion_detection_system/08-DCMI2LCD_tinyengine/Bsp/ST7735" -I"/home/busoye_tm/Documents/tinyml/intrusion_detection_system/08-DCMI2LCD_tinyengine/Bsp" -I"/home/busoye_tm/Documents/tinyml/intrusion_detection_system/08-DCMI2LCD_tinyengine/Core/Src/TinyEngine" -I"/home/busoye_tm/Documents/tinyml/intrusion_detection_system/08-DCMI2LCD_tinyengine/Core/Src/TinyEngine/include" -I"/home/busoye_tm/Documents/tinyml/intrusion_detection_system/08-DCMI2LCD_tinyengine/Drivers/CMSIS/arm_cmsis" -I"/home/busoye_tm/Documents/tinyml/intrusion_detection_system/08-DCMI2LCD_tinyengine/Core/Src/TinyEngine/codegen/Include" -O0 -ffunction-sections -fdata-sections -fpermissive -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Bsp-2f-ST7735

clean-Bsp-2f-ST7735:
	-$(RM) ./Bsp/ST7735/lcd.cyclo ./Bsp/ST7735/lcd.d ./Bsp/ST7735/lcd.o ./Bsp/ST7735/lcd.su ./Bsp/ST7735/logo_128_160.cyclo ./Bsp/ST7735/logo_128_160.d ./Bsp/ST7735/logo_128_160.o ./Bsp/ST7735/logo_128_160.su ./Bsp/ST7735/logo_160_80.cyclo ./Bsp/ST7735/logo_160_80.d ./Bsp/ST7735/logo_160_80.o ./Bsp/ST7735/logo_160_80.su ./Bsp/ST7735/st7735.cyclo ./Bsp/ST7735/st7735.d ./Bsp/ST7735/st7735.o ./Bsp/ST7735/st7735.su ./Bsp/ST7735/st7735_reg.cyclo ./Bsp/ST7735/st7735_reg.d ./Bsp/ST7735/st7735_reg.o ./Bsp/ST7735/st7735_reg.su

.PHONY: clean-Bsp-2f-ST7735

