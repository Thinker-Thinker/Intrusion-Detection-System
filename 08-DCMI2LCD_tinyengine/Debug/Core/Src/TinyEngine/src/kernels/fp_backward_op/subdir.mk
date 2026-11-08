################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/TinyEngine/src/kernels/fp_backward_op/add_fp.c \
../Core/Src/TinyEngine/src/kernels/fp_backward_op/div_fp.c \
../Core/Src/TinyEngine/src/kernels/fp_backward_op/group_conv_fp_kernel4_stride1_pad0.c \
../Core/Src/TinyEngine/src/kernels/fp_backward_op/group_conv_fp_kernel8_stride1_pad0.c \
../Core/Src/TinyEngine/src/kernels/fp_backward_op/group_pointwise_conv_fp.c \
../Core/Src/TinyEngine/src/kernels/fp_backward_op/less_fp.c \
../Core/Src/TinyEngine/src/kernels/fp_backward_op/log_softmax_fp.c \
../Core/Src/TinyEngine/src/kernels/fp_backward_op/mul_fp.c \
../Core/Src/TinyEngine/src/kernels/fp_backward_op/negative_fp.c \
../Core/Src/TinyEngine/src/kernels/fp_backward_op/nll_loss_fp.c \
../Core/Src/TinyEngine/src/kernels/fp_backward_op/pointwise_conv_fp.c \
../Core/Src/TinyEngine/src/kernels/fp_backward_op/strided_slice_3Dto3D_fp.c \
../Core/Src/TinyEngine/src/kernels/fp_backward_op/strided_slice_4Dto4D_fp.c \
../Core/Src/TinyEngine/src/kernels/fp_backward_op/sub_fp.c \
../Core/Src/TinyEngine/src/kernels/fp_backward_op/sum_2D_fp.c \
../Core/Src/TinyEngine/src/kernels/fp_backward_op/sum_3D_fp.c \
../Core/Src/TinyEngine/src/kernels/fp_backward_op/sum_4D_exclude_fp.c \
../Core/Src/TinyEngine/src/kernels/fp_backward_op/transpose_depthwise_conv_fp_kernel3_stride1_inpad1_outpad0.c \
../Core/Src/TinyEngine/src/kernels/fp_backward_op/transpose_depthwise_conv_fp_kernel3_stride2_inpad1_outpad1.c \
../Core/Src/TinyEngine/src/kernels/fp_backward_op/transpose_depthwise_conv_fp_kernel5_stride1_inpad2_outpad0.c \
../Core/Src/TinyEngine/src/kernels/fp_backward_op/transpose_depthwise_conv_fp_kernel5_stride2_inpad2_outpad1.c \
../Core/Src/TinyEngine/src/kernels/fp_backward_op/transpose_depthwise_conv_fp_kernel7_stride1_inpad3_outpad0.c \
../Core/Src/TinyEngine/src/kernels/fp_backward_op/transpose_depthwise_conv_fp_kernel7_stride2_inpad3_outpad1.c \
../Core/Src/TinyEngine/src/kernels/fp_backward_op/tte_exp_fp.c \
../Core/Src/TinyEngine/src/kernels/fp_backward_op/where_fp.c 

OBJS += \
./Core/Src/TinyEngine/src/kernels/fp_backward_op/add_fp.o \
./Core/Src/TinyEngine/src/kernels/fp_backward_op/div_fp.o \
./Core/Src/TinyEngine/src/kernels/fp_backward_op/group_conv_fp_kernel4_stride1_pad0.o \
./Core/Src/TinyEngine/src/kernels/fp_backward_op/group_conv_fp_kernel8_stride1_pad0.o \
./Core/Src/TinyEngine/src/kernels/fp_backward_op/group_pointwise_conv_fp.o \
./Core/Src/TinyEngine/src/kernels/fp_backward_op/less_fp.o \
./Core/Src/TinyEngine/src/kernels/fp_backward_op/log_softmax_fp.o \
./Core/Src/TinyEngine/src/kernels/fp_backward_op/mul_fp.o \
./Core/Src/TinyEngine/src/kernels/fp_backward_op/negative_fp.o \
./Core/Src/TinyEngine/src/kernels/fp_backward_op/nll_loss_fp.o \
./Core/Src/TinyEngine/src/kernels/fp_backward_op/pointwise_conv_fp.o \
./Core/Src/TinyEngine/src/kernels/fp_backward_op/strided_slice_3Dto3D_fp.o \
./Core/Src/TinyEngine/src/kernels/fp_backward_op/strided_slice_4Dto4D_fp.o \
./Core/Src/TinyEngine/src/kernels/fp_backward_op/sub_fp.o \
./Core/Src/TinyEngine/src/kernels/fp_backward_op/sum_2D_fp.o \
./Core/Src/TinyEngine/src/kernels/fp_backward_op/sum_3D_fp.o \
./Core/Src/TinyEngine/src/kernels/fp_backward_op/sum_4D_exclude_fp.o \
./Core/Src/TinyEngine/src/kernels/fp_backward_op/transpose_depthwise_conv_fp_kernel3_stride1_inpad1_outpad0.o \
./Core/Src/TinyEngine/src/kernels/fp_backward_op/transpose_depthwise_conv_fp_kernel3_stride2_inpad1_outpad1.o \
./Core/Src/TinyEngine/src/kernels/fp_backward_op/transpose_depthwise_conv_fp_kernel5_stride1_inpad2_outpad0.o \
./Core/Src/TinyEngine/src/kernels/fp_backward_op/transpose_depthwise_conv_fp_kernel5_stride2_inpad2_outpad1.o \
./Core/Src/TinyEngine/src/kernels/fp_backward_op/transpose_depthwise_conv_fp_kernel7_stride1_inpad3_outpad0.o \
./Core/Src/TinyEngine/src/kernels/fp_backward_op/transpose_depthwise_conv_fp_kernel7_stride2_inpad3_outpad1.o \
./Core/Src/TinyEngine/src/kernels/fp_backward_op/tte_exp_fp.o \
./Core/Src/TinyEngine/src/kernels/fp_backward_op/where_fp.o 

C_DEPS += \
./Core/Src/TinyEngine/src/kernels/fp_backward_op/add_fp.d \
./Core/Src/TinyEngine/src/kernels/fp_backward_op/div_fp.d \
./Core/Src/TinyEngine/src/kernels/fp_backward_op/group_conv_fp_kernel4_stride1_pad0.d \
./Core/Src/TinyEngine/src/kernels/fp_backward_op/group_conv_fp_kernel8_stride1_pad0.d \
./Core/Src/TinyEngine/src/kernels/fp_backward_op/group_pointwise_conv_fp.d \
./Core/Src/TinyEngine/src/kernels/fp_backward_op/less_fp.d \
./Core/Src/TinyEngine/src/kernels/fp_backward_op/log_softmax_fp.d \
./Core/Src/TinyEngine/src/kernels/fp_backward_op/mul_fp.d \
./Core/Src/TinyEngine/src/kernels/fp_backward_op/negative_fp.d \
./Core/Src/TinyEngine/src/kernels/fp_backward_op/nll_loss_fp.d \
./Core/Src/TinyEngine/src/kernels/fp_backward_op/pointwise_conv_fp.d \
./Core/Src/TinyEngine/src/kernels/fp_backward_op/strided_slice_3Dto3D_fp.d \
./Core/Src/TinyEngine/src/kernels/fp_backward_op/strided_slice_4Dto4D_fp.d \
./Core/Src/TinyEngine/src/kernels/fp_backward_op/sub_fp.d \
./Core/Src/TinyEngine/src/kernels/fp_backward_op/sum_2D_fp.d \
./Core/Src/TinyEngine/src/kernels/fp_backward_op/sum_3D_fp.d \
./Core/Src/TinyEngine/src/kernels/fp_backward_op/sum_4D_exclude_fp.d \
./Core/Src/TinyEngine/src/kernels/fp_backward_op/transpose_depthwise_conv_fp_kernel3_stride1_inpad1_outpad0.d \
./Core/Src/TinyEngine/src/kernels/fp_backward_op/transpose_depthwise_conv_fp_kernel3_stride2_inpad1_outpad1.d \
./Core/Src/TinyEngine/src/kernels/fp_backward_op/transpose_depthwise_conv_fp_kernel5_stride1_inpad2_outpad0.d \
./Core/Src/TinyEngine/src/kernels/fp_backward_op/transpose_depthwise_conv_fp_kernel5_stride2_inpad2_outpad1.d \
./Core/Src/TinyEngine/src/kernels/fp_backward_op/transpose_depthwise_conv_fp_kernel7_stride1_inpad3_outpad0.d \
./Core/Src/TinyEngine/src/kernels/fp_backward_op/transpose_depthwise_conv_fp_kernel7_stride2_inpad3_outpad1.d \
./Core/Src/TinyEngine/src/kernels/fp_backward_op/tte_exp_fp.d \
./Core/Src/TinyEngine/src/kernels/fp_backward_op/where_fp.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/TinyEngine/src/kernels/fp_backward_op/%.o Core/Src/TinyEngine/src/kernels/fp_backward_op/%.su Core/Src/TinyEngine/src/kernels/fp_backward_op/%.cyclo: ../Core/Src/TinyEngine/src/kernels/fp_backward_op/%.c Core/Src/TinyEngine/src/kernels/fp_backward_op/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=c99 -g3 -DDEBUG -DTFT96=1 -DUSE_PWR_LDO_SUPPLY -DUSE_HAL_DRIVER -DSTM32H723xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I"/home/busoye_tm/Documents/tinyml/intrusion_detection_system/08-DCMI2LCD_tinyengine/Bsp/Camera" -I"/home/busoye_tm/Documents/tinyml/intrusion_detection_system/08-DCMI2LCD_tinyengine/Bsp/ST7735" -I"/home/busoye_tm/Documents/tinyml/intrusion_detection_system/08-DCMI2LCD_tinyengine/Bsp" -I"/home/busoye_tm/Documents/tinyml/intrusion_detection_system/08-DCMI2LCD_tinyengine/Core/Src/TinyEngine" -I"/home/busoye_tm/Documents/tinyml/intrusion_detection_system/08-DCMI2LCD_tinyengine/Core/Src/TinyEngine/include" -I"/home/busoye_tm/Documents/tinyml/intrusion_detection_system/08-DCMI2LCD_tinyengine/Drivers/CMSIS/arm_cmsis" -I"/home/busoye_tm/Documents/tinyml/intrusion_detection_system/08-DCMI2LCD_tinyengine/Core/Src/TinyEngine/codegen/Include" -O0 -ffunction-sections -fdata-sections -fpermissive -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-TinyEngine-2f-src-2f-kernels-2f-fp_backward_op

clean-Core-2f-Src-2f-TinyEngine-2f-src-2f-kernels-2f-fp_backward_op:
	-$(RM) ./Core/Src/TinyEngine/src/kernels/fp_backward_op/add_fp.cyclo ./Core/Src/TinyEngine/src/kernels/fp_backward_op/add_fp.d ./Core/Src/TinyEngine/src/kernels/fp_backward_op/add_fp.o ./Core/Src/TinyEngine/src/kernels/fp_backward_op/add_fp.su ./Core/Src/TinyEngine/src/kernels/fp_backward_op/div_fp.cyclo ./Core/Src/TinyEngine/src/kernels/fp_backward_op/div_fp.d ./Core/Src/TinyEngine/src/kernels/fp_backward_op/div_fp.o ./Core/Src/TinyEngine/src/kernels/fp_backward_op/div_fp.su ./Core/Src/TinyEngine/src/kernels/fp_backward_op/group_conv_fp_kernel4_stride1_pad0.cyclo ./Core/Src/TinyEngine/src/kernels/fp_backward_op/group_conv_fp_kernel4_stride1_pad0.d ./Core/Src/TinyEngine/src/kernels/fp_backward_op/group_conv_fp_kernel4_stride1_pad0.o ./Core/Src/TinyEngine/src/kernels/fp_backward_op/group_conv_fp_kernel4_stride1_pad0.su ./Core/Src/TinyEngine/src/kernels/fp_backward_op/group_conv_fp_kernel8_stride1_pad0.cyclo ./Core/Src/TinyEngine/src/kernels/fp_backward_op/group_conv_fp_kernel8_stride1_pad0.d ./Core/Src/TinyEngine/src/kernels/fp_backward_op/group_conv_fp_kernel8_stride1_pad0.o ./Core/Src/TinyEngine/src/kernels/fp_backward_op/group_conv_fp_kernel8_stride1_pad0.su ./Core/Src/TinyEngine/src/kernels/fp_backward_op/group_pointwise_conv_fp.cyclo ./Core/Src/TinyEngine/src/kernels/fp_backward_op/group_pointwise_conv_fp.d ./Core/Src/TinyEngine/src/kernels/fp_backward_op/group_pointwise_conv_fp.o ./Core/Src/TinyEngine/src/kernels/fp_backward_op/group_pointwise_conv_fp.su ./Core/Src/TinyEngine/src/kernels/fp_backward_op/less_fp.cyclo ./Core/Src/TinyEngine/src/kernels/fp_backward_op/less_fp.d ./Core/Src/TinyEngine/src/kernels/fp_backward_op/less_fp.o ./Core/Src/TinyEngine/src/kernels/fp_backward_op/less_fp.su ./Core/Src/TinyEngine/src/kernels/fp_backward_op/log_softmax_fp.cyclo ./Core/Src/TinyEngine/src/kernels/fp_backward_op/log_softmax_fp.d ./Core/Src/TinyEngine/src/kernels/fp_backward_op/log_softmax_fp.o ./Core/Src/TinyEngine/src/kernels/fp_backward_op/log_softmax_fp.su ./Core/Src/TinyEngine/src/kernels/fp_backward_op/mul_fp.cyclo ./Core/Src/TinyEngine/src/kernels/fp_backward_op/mul_fp.d ./Core/Src/TinyEngine/src/kernels/fp_backward_op/mul_fp.o ./Core/Src/TinyEngine/src/kernels/fp_backward_op/mul_fp.su ./Core/Src/TinyEngine/src/kernels/fp_backward_op/negative_fp.cyclo ./Core/Src/TinyEngine/src/kernels/fp_backward_op/negative_fp.d ./Core/Src/TinyEngine/src/kernels/fp_backward_op/negative_fp.o ./Core/Src/TinyEngine/src/kernels/fp_backward_op/negative_fp.su ./Core/Src/TinyEngine/src/kernels/fp_backward_op/nll_loss_fp.cyclo ./Core/Src/TinyEngine/src/kernels/fp_backward_op/nll_loss_fp.d ./Core/Src/TinyEngine/src/kernels/fp_backward_op/nll_loss_fp.o ./Core/Src/TinyEngine/src/kernels/fp_backward_op/nll_loss_fp.su ./Core/Src/TinyEngine/src/kernels/fp_backward_op/pointwise_conv_fp.cyclo ./Core/Src/TinyEngine/src/kernels/fp_backward_op/pointwise_conv_fp.d ./Core/Src/TinyEngine/src/kernels/fp_backward_op/pointwise_conv_fp.o ./Core/Src/TinyEngine/src/kernels/fp_backward_op/pointwise_conv_fp.su ./Core/Src/TinyEngine/src/kernels/fp_backward_op/strided_slice_3Dto3D_fp.cyclo ./Core/Src/TinyEngine/src/kernels/fp_backward_op/strided_slice_3Dto3D_fp.d ./Core/Src/TinyEngine/src/kernels/fp_backward_op/strided_slice_3Dto3D_fp.o ./Core/Src/TinyEngine/src/kernels/fp_backward_op/strided_slice_3Dto3D_fp.su ./Core/Src/TinyEngine/src/kernels/fp_backward_op/strided_slice_4Dto4D_fp.cyclo ./Core/Src/TinyEngine/src/kernels/fp_backward_op/strided_slice_4Dto4D_fp.d ./Core/Src/TinyEngine/src/kernels/fp_backward_op/strided_slice_4Dto4D_fp.o ./Core/Src/TinyEngine/src/kernels/fp_backward_op/strided_slice_4Dto4D_fp.su ./Core/Src/TinyEngine/src/kernels/fp_backward_op/sub_fp.cyclo ./Core/Src/TinyEngine/src/kernels/fp_backward_op/sub_fp.d ./Core/Src/TinyEngine/src/kernels/fp_backward_op/sub_fp.o ./Core/Src/TinyEngine/src/kernels/fp_backward_op/sub_fp.su ./Core/Src/TinyEngine/src/kernels/fp_backward_op/sum_2D_fp.cyclo ./Core/Src/TinyEngine/src/kernels/fp_backward_op/sum_2D_fp.d ./Core/Src/TinyEngine/src/kernels/fp_backward_op/sum_2D_fp.o ./Core/Src/TinyEngine/src/kernels/fp_backward_op/sum_2D_fp.su ./Core/Src/TinyEngine/src/kernels/fp_backward_op/sum_3D_fp.cyclo ./Core/Src/TinyEngine/src/kernels/fp_backward_op/sum_3D_fp.d ./Core/Src/TinyEngine/src/kernels/fp_backward_op/sum_3D_fp.o ./Core/Src/TinyEngine/src/kernels/fp_backward_op/sum_3D_fp.su ./Core/Src/TinyEngine/src/kernels/fp_backward_op/sum_4D_exclude_fp.cyclo ./Core/Src/TinyEngine/src/kernels/fp_backward_op/sum_4D_exclude_fp.d ./Core/Src/TinyEngine/src/kernels/fp_backward_op/sum_4D_exclude_fp.o ./Core/Src/TinyEngine/src/kernels/fp_backward_op/sum_4D_exclude_fp.su ./Core/Src/TinyEngine/src/kernels/fp_backward_op/transpose_depthwise_conv_fp_kernel3_stride1_inpad1_outpad0.cyclo ./Core/Src/TinyEngine/src/kernels/fp_backward_op/transpose_depthwise_conv_fp_kernel3_stride1_inpad1_outpad0.d ./Core/Src/TinyEngine/src/kernels/fp_backward_op/transpose_depthwise_conv_fp_kernel3_stride1_inpad1_outpad0.o ./Core/Src/TinyEngine/src/kernels/fp_backward_op/transpose_depthwise_conv_fp_kernel3_stride1_inpad1_outpad0.su ./Core/Src/TinyEngine/src/kernels/fp_backward_op/transpose_depthwise_conv_fp_kernel3_stride2_inpad1_outpad1.cyclo ./Core/Src/TinyEngine/src/kernels/fp_backward_op/transpose_depthwise_conv_fp_kernel3_stride2_inpad1_outpad1.d ./Core/Src/TinyEngine/src/kernels/fp_backward_op/transpose_depthwise_conv_fp_kernel3_stride2_inpad1_outpad1.o ./Core/Src/TinyEngine/src/kernels/fp_backward_op/transpose_depthwise_conv_fp_kernel3_stride2_inpad1_outpad1.su ./Core/Src/TinyEngine/src/kernels/fp_backward_op/transpose_depthwise_conv_fp_kernel5_stride1_inpad2_outpad0.cyclo ./Core/Src/TinyEngine/src/kernels/fp_backward_op/transpose_depthwise_conv_fp_kernel5_stride1_inpad2_outpad0.d ./Core/Src/TinyEngine/src/kernels/fp_backward_op/transpose_depthwise_conv_fp_kernel5_stride1_inpad2_outpad0.o ./Core/Src/TinyEngine/src/kernels/fp_backward_op/transpose_depthwise_conv_fp_kernel5_stride1_inpad2_outpad0.su
	-$(RM) ./Core/Src/TinyEngine/src/kernels/fp_backward_op/transpose_depthwise_conv_fp_kernel5_stride2_inpad2_outpad1.cyclo ./Core/Src/TinyEngine/src/kernels/fp_backward_op/transpose_depthwise_conv_fp_kernel5_stride2_inpad2_outpad1.d ./Core/Src/TinyEngine/src/kernels/fp_backward_op/transpose_depthwise_conv_fp_kernel5_stride2_inpad2_outpad1.o ./Core/Src/TinyEngine/src/kernels/fp_backward_op/transpose_depthwise_conv_fp_kernel5_stride2_inpad2_outpad1.su ./Core/Src/TinyEngine/src/kernels/fp_backward_op/transpose_depthwise_conv_fp_kernel7_stride1_inpad3_outpad0.cyclo ./Core/Src/TinyEngine/src/kernels/fp_backward_op/transpose_depthwise_conv_fp_kernel7_stride1_inpad3_outpad0.d ./Core/Src/TinyEngine/src/kernels/fp_backward_op/transpose_depthwise_conv_fp_kernel7_stride1_inpad3_outpad0.o ./Core/Src/TinyEngine/src/kernels/fp_backward_op/transpose_depthwise_conv_fp_kernel7_stride1_inpad3_outpad0.su ./Core/Src/TinyEngine/src/kernels/fp_backward_op/transpose_depthwise_conv_fp_kernel7_stride2_inpad3_outpad1.cyclo ./Core/Src/TinyEngine/src/kernels/fp_backward_op/transpose_depthwise_conv_fp_kernel7_stride2_inpad3_outpad1.d ./Core/Src/TinyEngine/src/kernels/fp_backward_op/transpose_depthwise_conv_fp_kernel7_stride2_inpad3_outpad1.o ./Core/Src/TinyEngine/src/kernels/fp_backward_op/transpose_depthwise_conv_fp_kernel7_stride2_inpad3_outpad1.su ./Core/Src/TinyEngine/src/kernels/fp_backward_op/tte_exp_fp.cyclo ./Core/Src/TinyEngine/src/kernels/fp_backward_op/tte_exp_fp.d ./Core/Src/TinyEngine/src/kernels/fp_backward_op/tte_exp_fp.o ./Core/Src/TinyEngine/src/kernels/fp_backward_op/tte_exp_fp.su ./Core/Src/TinyEngine/src/kernels/fp_backward_op/where_fp.cyclo ./Core/Src/TinyEngine/src/kernels/fp_backward_op/where_fp.d ./Core/Src/TinyEngine/src/kernels/fp_backward_op/where_fp.o ./Core/Src/TinyEngine/src/kernels/fp_backward_op/where_fp.su

.PHONY: clean-Core-2f-Src-2f-TinyEngine-2f-src-2f-kernels-2f-fp_backward_op

