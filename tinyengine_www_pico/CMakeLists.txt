# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.3.1)
set(toolchainVersion 15_2_Rel1)
set(picotoolVersion 2.3.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico2 CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(tinyengine_www C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1
set(TINYENGINE_ROOT ${CMAKE_CURRENT_LIST_DIR})


set(TINYENGINE_SOURCES
  ${TINYENGINE_ROOT}/TinyEngine/codegen/Source/genModel.c
  ${TINYENGINE_ROOT}/TinyEngine/codegen/Source/depthwise_kernel3x3_stride1_inplace_CHW_fpreq.c
  ${TINYENGINE_ROOT}/TinyEngine/codegen/Source/depthwise_kernel3x3_stride2_inplace_CHW_fpreq.c
  ${TINYENGINE_ROOT}/TinyEngine/codegen/Source/depthwise_kernel5x5_stride1_inplace_CHW_fpreq.c
  ${TINYENGINE_ROOT}/TinyEngine/codegen/Source/depthwise_kernel5x5_stride2_inplace_CHW_fpreq.c
  ${TINYENGINE_ROOT}/TinyEngine/codegen/Source/depthwise_kernel7x7_stride1_inplace_CHW_fpreq.c
  ${TINYENGINE_ROOT}/TinyEngine/codegen/Source/depthwise_kernel7x7_stride2_inplace_CHW_fpreq.c
  ${TINYENGINE_ROOT}/TinyEngine/src/kernels/fp_requantize_op/add_fpreq.c
  ${TINYENGINE_ROOT}/TinyEngine/src/kernels/fp_requantize_op/convolve_1x1_s8_fpreq.c
  ${TINYENGINE_ROOT}/TinyEngine/src/kernels/fp_requantize_op/convolve_1x1_s8_ch8_fpreq.c
  ${TINYENGINE_ROOT}/TinyEngine/src/kernels/fp_requantize_op/convolve_1x1_s8_ch16_fpreq.c
  ${TINYENGINE_ROOT}/TinyEngine/src/kernels/fp_requantize_op/convolve_1x1_s8_ch24_fpreq.c
  ${TINYENGINE_ROOT}/TinyEngine/src/kernels/fp_requantize_op/convolve_1x1_s8_ch48_fpreq.c
  ${TINYENGINE_ROOT}/TinyEngine/src/kernels/fp_requantize_op/convolve_s8_kernel3_inputch3_stride2_pad1_fpreq.c
  ${TINYENGINE_ROOT}/TinyEngine/src/kernels/fp_requantize_op/mat_mul_kernels_fpreq.c
  ${TINYENGINE_ROOT}/TinyEngine/src/kernels/int_forward_op/avgpooling.c
  ${TINYENGINE_ROOT}/TinyEngine/src/kernels/int_forward_op/maxpooling.c
  ${TINYENGINE_ROOT}/TinyEngine/src/kernels/int_forward_op/upsample_byte.c
)

add_executable(tinyengine_www main.cpp ${TINYENGINE_SOURCES})


target_include_directories(tinyengine_www PRIVATE
  ${TINYENGINE_ROOT}/TinyEngine/include
  ${TINYENGINE_ROOT}/codegen/Include
  ${TINYENGINE_ROOT}/TinyEngine/codegen/Include

  ${TINYENGINE_ROOT}/TinyEngine/third_party/CMSIS/CMSIS/Core/Include
  ${TINYENGINE_ROOT}/TinyEngine/third_party/CMSIS/CMSIS/DSP/Include
  ${TINYENGINE_ROOT}/TinyEngine/third_party/CMSIS/CMSIS/NN/Include
)


# pull in common dependencies
target_link_libraries(tinyengine_www pico_stdlib m cmsis_core)

# create map/bin/hex file etc.
pico_add_extra_outputs(tinyengine_www)
pico_enable_stdio_usb(tinyengine_www 1)

# add url via pico_set_program_url
