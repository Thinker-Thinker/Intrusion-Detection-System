################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/TinyEngine/src/kernels/fp_requantize_op/add_fpreq.c \
../Core/Src/TinyEngine/src/kernels/fp_requantize_op/convolve_1x1_s8_ch16_fpreq.c \
../Core/Src/TinyEngine/src/kernels/fp_requantize_op/convolve_1x1_s8_ch24_fpreq.c \
../Core/Src/TinyEngine/src/kernels/fp_requantize_op/convolve_1x1_s8_ch48_fpreq.c \
../Core/Src/TinyEngine/src/kernels/fp_requantize_op/convolve_1x1_s8_ch8_fpreq.c \
../Core/Src/TinyEngine/src/kernels/fp_requantize_op/convolve_1x1_s8_fpreq.c \
../Core/Src/TinyEngine/src/kernels/fp_requantize_op/convolve_1x1_s8_fpreq_mask.c \
../Core/Src/TinyEngine/src/kernels/fp_requantize_op/convolve_1x1_s8_fpreq_mask_partialCH.c \
../Core/Src/TinyEngine/src/kernels/fp_requantize_op/convolve_s8_kernel3_inputch3_stride2_pad1_fpreq.c \
../Core/Src/TinyEngine/src/kernels/fp_requantize_op/mat_mul_kernels_fpreq.c 

OBJS += \
./Core/Src/TinyEngine/src/kernels/fp_requantize_op/add_fpreq.o \
./Core/Src/TinyEngine/src/kernels/fp_requantize_op/convolve_1x1_s8_ch16_fpreq.o \
./Core/Src/TinyEngine/src/kernels/fp_requantize_op/convolve_1x1_s8_ch24_fpreq.o \
./Core/Src/TinyEngine/src/kernels/fp_requantize_op/convolve_1x1_s8_ch48_fpreq.o \
./Core/Src/TinyEngine/src/kernels/fp_requantize_op/convolve_1x1_s8_ch8_fpreq.o \
./Core/Src/TinyEngine/src/kernels/fp_requantize_op/convolve_1x1_s8_fpreq.o \
./Core/Src/TinyEngine/src/kernels/fp_requantize_op/convolve_1x1_s8_fpreq_mask.o \
./Core/Src/TinyEngine/src/kernels/fp_requantize_op/convolve_1x1_s8_fpreq_mask_partialCH.o \
./Core/Src/TinyEngine/src/kernels/fp_requantize_op/convolve_s8_kernel3_inputch3_stride2_pad1_fpreq.o \
./Core/Src/TinyEngine/src/kernels/fp_requantize_op/mat_mul_kernels_fpreq.o 

C_DEPS += \
./Core/Src/TinyEngine/src/kernels/fp_requantize_op/add_fpreq.d \
./Core/Src/TinyEngine/src/kernels/fp_requantize_op/convolve_1x1_s8_ch16_fpreq.d \
./Core/Src/TinyEngine/src/kernels/fp_requantize_op/convolve_1x1_s8_ch24_fpreq.d \
./Core/Src/TinyEngine/src/kernels/fp_requantize_op/convolve_1x1_s8_ch48_fpreq.d \
./Core/Src/TinyEngine/src/kernels/fp_requantize_op/convolve_1x1_s8_ch8_fpreq.d \
./Core/Src/TinyEngine/src/kernels/fp_requantize_op/convolve_1x1_s8_fpreq.d \
./Core/Src/TinyEngine/src/kernels/fp_requantize_op/convolve_1x1_s8_fpreq_mask.d \
./Core/Src/TinyEngine/src/kernels/fp_requantize_op/convolve_1x1_s8_fpreq_mask_partialCH.d \
./Core/Src/TinyEngine/src/kernels/fp_requantize_op/convolve_s8_kernel3_inputch3_stride2_pad1_fpreq.d \
./Core/Src/TinyEngine/src/kernels/fp_requantize_op/mat_mul_kernels_fpreq.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/TinyEngine/src/kernels/fp_requantize_op/%.o Core/Src/TinyEngine/src/kernels/fp_requantize_op/%.su Core/Src/TinyEngine/src/kernels/fp_requantize_op/%.cyclo: ../Core/Src/TinyEngine/src/kernels/fp_requantize_op/%.c Core/Src/TinyEngine/src/kernels/fp_requantize_op/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=c99 -g3 -DDEBUG -DTFT96=1 -DUSE_PWR_LDO_SUPPLY -DUSE_HAL_DRIVER -DSTM32H723xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I"/home/busoye_tm/Documents/tinyml/intrusion_detection_system/08-DCMI2LCD_tinyengine/Bsp/Camera" -I"/home/busoye_tm/Documents/tinyml/intrusion_detection_system/08-DCMI2LCD_tinyengine/Bsp/ST7735" -I"/home/busoye_tm/Documents/tinyml/intrusion_detection_system/08-DCMI2LCD_tinyengine/Bsp" -I"/home/busoye_tm/Documents/tinyml/intrusion_detection_system/08-DCMI2LCD_tinyengine/Core/Src/TinyEngine" -I"/home/busoye_tm/Documents/tinyml/intrusion_detection_system/08-DCMI2LCD_tinyengine/Core/Src/TinyEngine/include" -I"/home/busoye_tm/Documents/tinyml/intrusion_detection_system/08-DCMI2LCD_tinyengine/Drivers/CMSIS/arm_cmsis" -I"/home/busoye_tm/Documents/tinyml/intrusion_detection_system/08-DCMI2LCD_tinyengine/Core/Src/TinyEngine/codegen/Include" -O0 -ffunction-sections -fdata-sections -fpermissive -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-TinyEngine-2f-src-2f-kernels-2f-fp_requantize_op

clean-Core-2f-Src-2f-TinyEngine-2f-src-2f-kernels-2f-fp_requantize_op:
	-$(RM) ./Core/Src/TinyEngine/src/kernels/fp_requantize_op/add_fpreq.cyclo ./Core/Src/TinyEngine/src/kernels/fp_requantize_op/add_fpreq.d ./Core/Src/TinyEngine/src/kernels/fp_requantize_op/add_fpreq.o ./Core/Src/TinyEngine/src/kernels/fp_requantize_op/add_fpreq.su ./Core/Src/TinyEngine/src/kernels/fp_requantize_op/convolve_1x1_s8_ch16_fpreq.cyclo ./Core/Src/TinyEngine/src/kernels/fp_requantize_op/convolve_1x1_s8_ch16_fpreq.d ./Core/Src/TinyEngine/src/kernels/fp_requantize_op/convolve_1x1_s8_ch16_fpreq.o ./Core/Src/TinyEngine/src/kernels/fp_requantize_op/convolve_1x1_s8_ch16_fpreq.su ./Core/Src/TinyEngine/src/kernels/fp_requantize_op/convolve_1x1_s8_ch24_fpreq.cyclo ./Core/Src/TinyEngine/src/kernels/fp_requantize_op/convolve_1x1_s8_ch24_fpreq.d ./Core/Src/TinyEngine/src/kernels/fp_requantize_op/convolve_1x1_s8_ch24_fpreq.o ./Core/Src/TinyEngine/src/kernels/fp_requantize_op/convolve_1x1_s8_ch24_fpreq.su ./Core/Src/TinyEngine/src/kernels/fp_requantize_op/convolve_1x1_s8_ch48_fpreq.cyclo ./Core/Src/TinyEngine/src/kernels/fp_requantize_op/convolve_1x1_s8_ch48_fpreq.d ./Core/Src/TinyEngine/src/kernels/fp_requantize_op/convolve_1x1_s8_ch48_fpreq.o ./Core/Src/TinyEngine/src/kernels/fp_requantize_op/convolve_1x1_s8_ch48_fpreq.su ./Core/Src/TinyEngine/src/kernels/fp_requantize_op/convolve_1x1_s8_ch8_fpreq.cyclo ./Core/Src/TinyEngine/src/kernels/fp_requantize_op/convolve_1x1_s8_ch8_fpreq.d ./Core/Src/TinyEngine/src/kernels/fp_requantize_op/convolve_1x1_s8_ch8_fpreq.o ./Core/Src/TinyEngine/src/kernels/fp_requantize_op/convolve_1x1_s8_ch8_fpreq.su ./Core/Src/TinyEngine/src/kernels/fp_requantize_op/convolve_1x1_s8_fpreq.cyclo ./Core/Src/TinyEngine/src/kernels/fp_requantize_op/convolve_1x1_s8_fpreq.d ./Core/Src/TinyEngine/src/kernels/fp_requantize_op/convolve_1x1_s8_fpreq.o ./Core/Src/TinyEngine/src/kernels/fp_requantize_op/convolve_1x1_s8_fpreq.su ./Core/Src/TinyEngine/src/kernels/fp_requantize_op/convolve_1x1_s8_fpreq_mask.cyclo ./Core/Src/TinyEngine/src/kernels/fp_requantize_op/convolve_1x1_s8_fpreq_mask.d ./Core/Src/TinyEngine/src/kernels/fp_requantize_op/convolve_1x1_s8_fpreq_mask.o ./Core/Src/TinyEngine/src/kernels/fp_requantize_op/convolve_1x1_s8_fpreq_mask.su ./Core/Src/TinyEngine/src/kernels/fp_requantize_op/convolve_1x1_s8_fpreq_mask_partialCH.cyclo ./Core/Src/TinyEngine/src/kernels/fp_requantize_op/convolve_1x1_s8_fpreq_mask_partialCH.d ./Core/Src/TinyEngine/src/kernels/fp_requantize_op/convolve_1x1_s8_fpreq_mask_partialCH.o ./Core/Src/TinyEngine/src/kernels/fp_requantize_op/convolve_1x1_s8_fpreq_mask_partialCH.su ./Core/Src/TinyEngine/src/kernels/fp_requantize_op/convolve_s8_kernel3_inputch3_stride2_pad1_fpreq.cyclo ./Core/Src/TinyEngine/src/kernels/fp_requantize_op/convolve_s8_kernel3_inputch3_stride2_pad1_fpreq.d ./Core/Src/TinyEngine/src/kernels/fp_requantize_op/convolve_s8_kernel3_inputch3_stride2_pad1_fpreq.o ./Core/Src/TinyEngine/src/kernels/fp_requantize_op/convolve_s8_kernel3_inputch3_stride2_pad1_fpreq.su ./Core/Src/TinyEngine/src/kernels/fp_requantize_op/mat_mul_kernels_fpreq.cyclo ./Core/Src/TinyEngine/src/kernels/fp_requantize_op/mat_mul_kernels_fpreq.d ./Core/Src/TinyEngine/src/kernels/fp_requantize_op/mat_mul_kernels_fpreq.o ./Core/Src/TinyEngine/src/kernels/fp_requantize_op/mat_mul_kernels_fpreq.su

.PHONY: clean-Core-2f-Src-2f-TinyEngine-2f-src-2f-kernels-2f-fp_requantize_op

