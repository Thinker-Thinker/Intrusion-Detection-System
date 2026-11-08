################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/TinyEngine/src/kernels/int_forward_op/add.c \
../Core/Src/TinyEngine/src/kernels/int_forward_op/arm_convolve_s8_4col.c \
../Core/Src/TinyEngine/src/kernels/int_forward_op/arm_nn_mat_mult_kernel3_input3_s8_s16.c \
../Core/Src/TinyEngine/src/kernels/int_forward_op/arm_nn_mat_mult_kernel_s8_s16_reordered_8mul.c \
../Core/Src/TinyEngine/src/kernels/int_forward_op/arm_nn_mat_mult_kernel_s8_s16_reordered_oddch.c \
../Core/Src/TinyEngine/src/kernels/int_forward_op/avgpooling.c \
../Core/Src/TinyEngine/src/kernels/int_forward_op/concat_ch.c \
../Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8.c \
../Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8_SRAM.c \
../Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8_ch16.c \
../Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8_ch24.c \
../Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8_ch48.c \
../Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8_ch8.c \
../Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8_kbuf.c \
../Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8_oddch.c \
../Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8_skip_pad.c \
../Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_s8_kernel2x3_inputch3_stride2_pad1.c \
../Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_s8_kernel3_inputch3_stride2_pad1.c \
../Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_s8_kernel3_stride1_pad1.c \
../Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_s8_kernel3x2_inputch3_stride2_pad1.c \
../Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_u8_kernel3_inputch3_stride1_pad1.c \
../Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_u8_kernel3_inputch3_stride2_pad1.c \
../Core/Src/TinyEngine/src/kernels/int_forward_op/element_mult.c \
../Core/Src/TinyEngine/src/kernels/int_forward_op/fully_connected.c \
../Core/Src/TinyEngine/src/kernels/int_forward_op/mat_mul_fp.c \
../Core/Src/TinyEngine/src/kernels/int_forward_op/mat_mult_kernels.c \
../Core/Src/TinyEngine/src/kernels/int_forward_op/maxpooling.c \
../Core/Src/TinyEngine/src/kernels/int_forward_op/patchpadding_convolve_s8_kernel3_inputch3_stride2.c \
../Core/Src/TinyEngine/src/kernels/int_forward_op/patchpadding_depthwise_kernel3x3_stride1_inplace_CHW.c \
../Core/Src/TinyEngine/src/kernels/int_forward_op/patchpadding_depthwise_kernel3x3_stride2_inplace_CHW.c \
../Core/Src/TinyEngine/src/kernels/int_forward_op/patchpadding_kbuf_convolve_s8_kernel3_inputch3_stride2.c \
../Core/Src/TinyEngine/src/kernels/int_forward_op/stable_softmax.c \
../Core/Src/TinyEngine/src/kernels/int_forward_op/upsample_byte.c 

OBJS += \
./Core/Src/TinyEngine/src/kernels/int_forward_op/add.o \
./Core/Src/TinyEngine/src/kernels/int_forward_op/arm_convolve_s8_4col.o \
./Core/Src/TinyEngine/src/kernels/int_forward_op/arm_nn_mat_mult_kernel3_input3_s8_s16.o \
./Core/Src/TinyEngine/src/kernels/int_forward_op/arm_nn_mat_mult_kernel_s8_s16_reordered_8mul.o \
./Core/Src/TinyEngine/src/kernels/int_forward_op/arm_nn_mat_mult_kernel_s8_s16_reordered_oddch.o \
./Core/Src/TinyEngine/src/kernels/int_forward_op/avgpooling.o \
./Core/Src/TinyEngine/src/kernels/int_forward_op/concat_ch.o \
./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8.o \
./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8_SRAM.o \
./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8_ch16.o \
./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8_ch24.o \
./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8_ch48.o \
./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8_ch8.o \
./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8_kbuf.o \
./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8_oddch.o \
./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8_skip_pad.o \
./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_s8_kernel2x3_inputch3_stride2_pad1.o \
./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_s8_kernel3_inputch3_stride2_pad1.o \
./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_s8_kernel3_stride1_pad1.o \
./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_s8_kernel3x2_inputch3_stride2_pad1.o \
./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_u8_kernel3_inputch3_stride1_pad1.o \
./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_u8_kernel3_inputch3_stride2_pad1.o \
./Core/Src/TinyEngine/src/kernels/int_forward_op/element_mult.o \
./Core/Src/TinyEngine/src/kernels/int_forward_op/fully_connected.o \
./Core/Src/TinyEngine/src/kernels/int_forward_op/mat_mul_fp.o \
./Core/Src/TinyEngine/src/kernels/int_forward_op/mat_mult_kernels.o \
./Core/Src/TinyEngine/src/kernels/int_forward_op/maxpooling.o \
./Core/Src/TinyEngine/src/kernels/int_forward_op/patchpadding_convolve_s8_kernel3_inputch3_stride2.o \
./Core/Src/TinyEngine/src/kernels/int_forward_op/patchpadding_depthwise_kernel3x3_stride1_inplace_CHW.o \
./Core/Src/TinyEngine/src/kernels/int_forward_op/patchpadding_depthwise_kernel3x3_stride2_inplace_CHW.o \
./Core/Src/TinyEngine/src/kernels/int_forward_op/patchpadding_kbuf_convolve_s8_kernel3_inputch3_stride2.o \
./Core/Src/TinyEngine/src/kernels/int_forward_op/stable_softmax.o \
./Core/Src/TinyEngine/src/kernels/int_forward_op/upsample_byte.o 

C_DEPS += \
./Core/Src/TinyEngine/src/kernels/int_forward_op/add.d \
./Core/Src/TinyEngine/src/kernels/int_forward_op/arm_convolve_s8_4col.d \
./Core/Src/TinyEngine/src/kernels/int_forward_op/arm_nn_mat_mult_kernel3_input3_s8_s16.d \
./Core/Src/TinyEngine/src/kernels/int_forward_op/arm_nn_mat_mult_kernel_s8_s16_reordered_8mul.d \
./Core/Src/TinyEngine/src/kernels/int_forward_op/arm_nn_mat_mult_kernel_s8_s16_reordered_oddch.d \
./Core/Src/TinyEngine/src/kernels/int_forward_op/avgpooling.d \
./Core/Src/TinyEngine/src/kernels/int_forward_op/concat_ch.d \
./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8.d \
./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8_SRAM.d \
./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8_ch16.d \
./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8_ch24.d \
./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8_ch48.d \
./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8_ch8.d \
./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8_kbuf.d \
./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8_oddch.d \
./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8_skip_pad.d \
./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_s8_kernel2x3_inputch3_stride2_pad1.d \
./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_s8_kernel3_inputch3_stride2_pad1.d \
./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_s8_kernel3_stride1_pad1.d \
./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_s8_kernel3x2_inputch3_stride2_pad1.d \
./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_u8_kernel3_inputch3_stride1_pad1.d \
./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_u8_kernel3_inputch3_stride2_pad1.d \
./Core/Src/TinyEngine/src/kernels/int_forward_op/element_mult.d \
./Core/Src/TinyEngine/src/kernels/int_forward_op/fully_connected.d \
./Core/Src/TinyEngine/src/kernels/int_forward_op/mat_mul_fp.d \
./Core/Src/TinyEngine/src/kernels/int_forward_op/mat_mult_kernels.d \
./Core/Src/TinyEngine/src/kernels/int_forward_op/maxpooling.d \
./Core/Src/TinyEngine/src/kernels/int_forward_op/patchpadding_convolve_s8_kernel3_inputch3_stride2.d \
./Core/Src/TinyEngine/src/kernels/int_forward_op/patchpadding_depthwise_kernel3x3_stride1_inplace_CHW.d \
./Core/Src/TinyEngine/src/kernels/int_forward_op/patchpadding_depthwise_kernel3x3_stride2_inplace_CHW.d \
./Core/Src/TinyEngine/src/kernels/int_forward_op/patchpadding_kbuf_convolve_s8_kernel3_inputch3_stride2.d \
./Core/Src/TinyEngine/src/kernels/int_forward_op/stable_softmax.d \
./Core/Src/TinyEngine/src/kernels/int_forward_op/upsample_byte.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/TinyEngine/src/kernels/int_forward_op/%.o Core/Src/TinyEngine/src/kernels/int_forward_op/%.su Core/Src/TinyEngine/src/kernels/int_forward_op/%.cyclo: ../Core/Src/TinyEngine/src/kernels/int_forward_op/%.c Core/Src/TinyEngine/src/kernels/int_forward_op/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=c99 -g3 -DDEBUG -DTFT96=1 -DUSE_PWR_LDO_SUPPLY -DUSE_HAL_DRIVER -DSTM32H723xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I"/home/busoye_tm/Documents/tinyml/intrusion_detection_system/08-DCMI2LCD_tinyengine/Bsp/Camera" -I"/home/busoye_tm/Documents/tinyml/intrusion_detection_system/08-DCMI2LCD_tinyengine/Bsp/ST7735" -I"/home/busoye_tm/Documents/tinyml/intrusion_detection_system/08-DCMI2LCD_tinyengine/Bsp" -I"/home/busoye_tm/Documents/tinyml/intrusion_detection_system/08-DCMI2LCD_tinyengine/Core/Src/TinyEngine" -I"/home/busoye_tm/Documents/tinyml/intrusion_detection_system/08-DCMI2LCD_tinyengine/Core/Src/TinyEngine/include" -I"/home/busoye_tm/Documents/tinyml/intrusion_detection_system/08-DCMI2LCD_tinyengine/Drivers/CMSIS/arm_cmsis" -I"/home/busoye_tm/Documents/tinyml/intrusion_detection_system/08-DCMI2LCD_tinyengine/Core/Src/TinyEngine/codegen/Include" -O0 -ffunction-sections -fdata-sections -fpermissive -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-TinyEngine-2f-src-2f-kernels-2f-int_forward_op

clean-Core-2f-Src-2f-TinyEngine-2f-src-2f-kernels-2f-int_forward_op:
	-$(RM) ./Core/Src/TinyEngine/src/kernels/int_forward_op/add.cyclo ./Core/Src/TinyEngine/src/kernels/int_forward_op/add.d ./Core/Src/TinyEngine/src/kernels/int_forward_op/add.o ./Core/Src/TinyEngine/src/kernels/int_forward_op/add.su ./Core/Src/TinyEngine/src/kernels/int_forward_op/arm_convolve_s8_4col.cyclo ./Core/Src/TinyEngine/src/kernels/int_forward_op/arm_convolve_s8_4col.d ./Core/Src/TinyEngine/src/kernels/int_forward_op/arm_convolve_s8_4col.o ./Core/Src/TinyEngine/src/kernels/int_forward_op/arm_convolve_s8_4col.su ./Core/Src/TinyEngine/src/kernels/int_forward_op/arm_nn_mat_mult_kernel3_input3_s8_s16.cyclo ./Core/Src/TinyEngine/src/kernels/int_forward_op/arm_nn_mat_mult_kernel3_input3_s8_s16.d ./Core/Src/TinyEngine/src/kernels/int_forward_op/arm_nn_mat_mult_kernel3_input3_s8_s16.o ./Core/Src/TinyEngine/src/kernels/int_forward_op/arm_nn_mat_mult_kernel3_input3_s8_s16.su ./Core/Src/TinyEngine/src/kernels/int_forward_op/arm_nn_mat_mult_kernel_s8_s16_reordered_8mul.cyclo ./Core/Src/TinyEngine/src/kernels/int_forward_op/arm_nn_mat_mult_kernel_s8_s16_reordered_8mul.d ./Core/Src/TinyEngine/src/kernels/int_forward_op/arm_nn_mat_mult_kernel_s8_s16_reordered_8mul.o ./Core/Src/TinyEngine/src/kernels/int_forward_op/arm_nn_mat_mult_kernel_s8_s16_reordered_8mul.su ./Core/Src/TinyEngine/src/kernels/int_forward_op/arm_nn_mat_mult_kernel_s8_s16_reordered_oddch.cyclo ./Core/Src/TinyEngine/src/kernels/int_forward_op/arm_nn_mat_mult_kernel_s8_s16_reordered_oddch.d ./Core/Src/TinyEngine/src/kernels/int_forward_op/arm_nn_mat_mult_kernel_s8_s16_reordered_oddch.o ./Core/Src/TinyEngine/src/kernels/int_forward_op/arm_nn_mat_mult_kernel_s8_s16_reordered_oddch.su ./Core/Src/TinyEngine/src/kernels/int_forward_op/avgpooling.cyclo ./Core/Src/TinyEngine/src/kernels/int_forward_op/avgpooling.d ./Core/Src/TinyEngine/src/kernels/int_forward_op/avgpooling.o ./Core/Src/TinyEngine/src/kernels/int_forward_op/avgpooling.su ./Core/Src/TinyEngine/src/kernels/int_forward_op/concat_ch.cyclo ./Core/Src/TinyEngine/src/kernels/int_forward_op/concat_ch.d ./Core/Src/TinyEngine/src/kernels/int_forward_op/concat_ch.o ./Core/Src/TinyEngine/src/kernels/int_forward_op/concat_ch.su ./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8.cyclo ./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8.d ./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8.o ./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8.su ./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8_SRAM.cyclo ./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8_SRAM.d ./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8_SRAM.o ./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8_SRAM.su ./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8_ch16.cyclo ./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8_ch16.d ./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8_ch16.o ./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8_ch16.su ./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8_ch24.cyclo ./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8_ch24.d ./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8_ch24.o ./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8_ch24.su ./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8_ch48.cyclo ./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8_ch48.d ./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8_ch48.o ./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8_ch48.su ./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8_ch8.cyclo ./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8_ch8.d ./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8_ch8.o ./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8_ch8.su ./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8_kbuf.cyclo ./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8_kbuf.d ./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8_kbuf.o ./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8_kbuf.su ./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8_oddch.cyclo ./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8_oddch.d ./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8_oddch.o ./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8_oddch.su ./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8_skip_pad.cyclo ./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8_skip_pad.d ./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8_skip_pad.o ./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_1x1_s8_skip_pad.su ./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_s8_kernel2x3_inputch3_stride2_pad1.cyclo ./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_s8_kernel2x3_inputch3_stride2_pad1.d ./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_s8_kernel2x3_inputch3_stride2_pad1.o ./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_s8_kernel2x3_inputch3_stride2_pad1.su ./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_s8_kernel3_inputch3_stride2_pad1.cyclo ./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_s8_kernel3_inputch3_stride2_pad1.d ./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_s8_kernel3_inputch3_stride2_pad1.o ./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_s8_kernel3_inputch3_stride2_pad1.su ./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_s8_kernel3_stride1_pad1.cyclo ./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_s8_kernel3_stride1_pad1.d ./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_s8_kernel3_stride1_pad1.o ./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_s8_kernel3_stride1_pad1.su
	-$(RM) ./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_s8_kernel3x2_inputch3_stride2_pad1.cyclo ./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_s8_kernel3x2_inputch3_stride2_pad1.d ./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_s8_kernel3x2_inputch3_stride2_pad1.o ./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_s8_kernel3x2_inputch3_stride2_pad1.su ./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_u8_kernel3_inputch3_stride1_pad1.cyclo ./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_u8_kernel3_inputch3_stride1_pad1.d ./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_u8_kernel3_inputch3_stride1_pad1.o ./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_u8_kernel3_inputch3_stride1_pad1.su ./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_u8_kernel3_inputch3_stride2_pad1.cyclo ./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_u8_kernel3_inputch3_stride2_pad1.d ./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_u8_kernel3_inputch3_stride2_pad1.o ./Core/Src/TinyEngine/src/kernels/int_forward_op/convolve_u8_kernel3_inputch3_stride2_pad1.su ./Core/Src/TinyEngine/src/kernels/int_forward_op/element_mult.cyclo ./Core/Src/TinyEngine/src/kernels/int_forward_op/element_mult.d ./Core/Src/TinyEngine/src/kernels/int_forward_op/element_mult.o ./Core/Src/TinyEngine/src/kernels/int_forward_op/element_mult.su ./Core/Src/TinyEngine/src/kernels/int_forward_op/fully_connected.cyclo ./Core/Src/TinyEngine/src/kernels/int_forward_op/fully_connected.d ./Core/Src/TinyEngine/src/kernels/int_forward_op/fully_connected.o ./Core/Src/TinyEngine/src/kernels/int_forward_op/fully_connected.su ./Core/Src/TinyEngine/src/kernels/int_forward_op/mat_mul_fp.cyclo ./Core/Src/TinyEngine/src/kernels/int_forward_op/mat_mul_fp.d ./Core/Src/TinyEngine/src/kernels/int_forward_op/mat_mul_fp.o ./Core/Src/TinyEngine/src/kernels/int_forward_op/mat_mul_fp.su ./Core/Src/TinyEngine/src/kernels/int_forward_op/mat_mult_kernels.cyclo ./Core/Src/TinyEngine/src/kernels/int_forward_op/mat_mult_kernels.d ./Core/Src/TinyEngine/src/kernels/int_forward_op/mat_mult_kernels.o ./Core/Src/TinyEngine/src/kernels/int_forward_op/mat_mult_kernels.su ./Core/Src/TinyEngine/src/kernels/int_forward_op/maxpooling.cyclo ./Core/Src/TinyEngine/src/kernels/int_forward_op/maxpooling.d ./Core/Src/TinyEngine/src/kernels/int_forward_op/maxpooling.o ./Core/Src/TinyEngine/src/kernels/int_forward_op/maxpooling.su ./Core/Src/TinyEngine/src/kernels/int_forward_op/patchpadding_convolve_s8_kernel3_inputch3_stride2.cyclo ./Core/Src/TinyEngine/src/kernels/int_forward_op/patchpadding_convolve_s8_kernel3_inputch3_stride2.d ./Core/Src/TinyEngine/src/kernels/int_forward_op/patchpadding_convolve_s8_kernel3_inputch3_stride2.o ./Core/Src/TinyEngine/src/kernels/int_forward_op/patchpadding_convolve_s8_kernel3_inputch3_stride2.su ./Core/Src/TinyEngine/src/kernels/int_forward_op/patchpadding_depthwise_kernel3x3_stride1_inplace_CHW.cyclo ./Core/Src/TinyEngine/src/kernels/int_forward_op/patchpadding_depthwise_kernel3x3_stride1_inplace_CHW.d ./Core/Src/TinyEngine/src/kernels/int_forward_op/patchpadding_depthwise_kernel3x3_stride1_inplace_CHW.o ./Core/Src/TinyEngine/src/kernels/int_forward_op/patchpadding_depthwise_kernel3x3_stride1_inplace_CHW.su ./Core/Src/TinyEngine/src/kernels/int_forward_op/patchpadding_depthwise_kernel3x3_stride2_inplace_CHW.cyclo ./Core/Src/TinyEngine/src/kernels/int_forward_op/patchpadding_depthwise_kernel3x3_stride2_inplace_CHW.d ./Core/Src/TinyEngine/src/kernels/int_forward_op/patchpadding_depthwise_kernel3x3_stride2_inplace_CHW.o ./Core/Src/TinyEngine/src/kernels/int_forward_op/patchpadding_depthwise_kernel3x3_stride2_inplace_CHW.su ./Core/Src/TinyEngine/src/kernels/int_forward_op/patchpadding_kbuf_convolve_s8_kernel3_inputch3_stride2.cyclo ./Core/Src/TinyEngine/src/kernels/int_forward_op/patchpadding_kbuf_convolve_s8_kernel3_inputch3_stride2.d ./Core/Src/TinyEngine/src/kernels/int_forward_op/patchpadding_kbuf_convolve_s8_kernel3_inputch3_stride2.o ./Core/Src/TinyEngine/src/kernels/int_forward_op/patchpadding_kbuf_convolve_s8_kernel3_inputch3_stride2.su ./Core/Src/TinyEngine/src/kernels/int_forward_op/stable_softmax.cyclo ./Core/Src/TinyEngine/src/kernels/int_forward_op/stable_softmax.d ./Core/Src/TinyEngine/src/kernels/int_forward_op/stable_softmax.o ./Core/Src/TinyEngine/src/kernels/int_forward_op/stable_softmax.su ./Core/Src/TinyEngine/src/kernels/int_forward_op/upsample_byte.cyclo ./Core/Src/TinyEngine/src/kernels/int_forward_op/upsample_byte.d ./Core/Src/TinyEngine/src/kernels/int_forward_op/upsample_byte.o ./Core/Src/TinyEngine/src/kernels/int_forward_op/upsample_byte.su

.PHONY: clean-Core-2f-Src-2f-TinyEngine-2f-src-2f-kernels-2f-int_forward_op

