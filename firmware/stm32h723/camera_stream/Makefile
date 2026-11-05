# Camera streaming firmware for the WeAct STM32H723VGT6 core board + OV7725.
# Build:  make            (needs arm-none-eabi-gcc on PATH; STM32CubeIDE ships one)
# Output: build/camera_stream.elf / .hex / .bin

TARGET   = camera_stream
BUILD    = build
PREFIX  ?= arm-none-eabi-
CC       = $(PREFIX)gcc
AS       = $(PREFIX)gcc -x assembler-with-cpp
CP       = $(PREFIX)objcopy
SZ       = $(PREFIX)size

C_SOURCES = \
  $(wildcard Core/Src/*.c) \
  $(wildcard Bsp/*.c) \
  $(wildcard Bsp/Camera/*.c) \
  $(wildcard Bsp/ST7735/*.c) \
  $(wildcard USB_DEVICE/App/*.c) \
  $(wildcard USB_DEVICE/Target/*.c) \
  $(wildcard Middlewares/ST/STM32_USB_Device_Library/Core/Src/*.c) \
  $(wildcard Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Src/*.c) \
  $(wildcard Drivers/STM32H7xx_HAL_Driver/Src/*.c)

ASM_SOURCES = startup_stm32h723xx.s

C_DEFS = -DUSE_HAL_DRIVER -DSTM32H723xx -DTFT96

C_INCLUDES = \
  -ICore/Inc -IBsp -IBsp/Camera -IBsp/ST7735 \
  -IUSB_DEVICE/App -IUSB_DEVICE/Target \
  -IMiddlewares/ST/STM32_USB_Device_Library/Core/Inc \
  -IMiddlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc \
  -IDrivers/STM32H7xx_HAL_Driver/Inc -IDrivers/STM32H7xx_HAL_Driver/Inc/Legacy \
  -IDrivers/CMSIS/Device/ST/STM32H7xx/Include -IDrivers/CMSIS/Include

MCU    = -mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard
OPT   ?= -O2
CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -g3 -Wall -fdata-sections -ffunction-sections -std=gnu11 -MMD -MP
ASFLAGS = $(MCU) -g3
LDSCRIPT = STM32H723VGTX_FLASH.ld
LDFLAGS = $(MCU) -T$(LDSCRIPT) -specs=nano.specs -specs=nosys.specs -u _printf_float \
          -Wl,-Map=$(BUILD)/$(TARGET).map,--cref -Wl,--gc-sections -Wl,--print-memory-usage \
          -lc -lm -lnosys

OBJECTS  = $(addprefix $(BUILD)/,$(notdir $(C_SOURCES:.c=.o)))
OBJECTS += $(addprefix $(BUILD)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

all: $(BUILD)/$(TARGET).elf $(BUILD)/$(TARGET).hex $(BUILD)/$(TARGET).bin

$(BUILD)/%.o: %.c Makefile | $(BUILD)
	$(CC) -c $(CFLAGS) $< -o $@

$(BUILD)/%.o: %.s Makefile | $(BUILD)
	$(AS) -c $(ASFLAGS) $< -o $@

$(BUILD)/$(TARGET).elf: $(OBJECTS) Makefile $(LDSCRIPT)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD)/%.hex: $(BUILD)/%.elf
	$(CP) -O ihex $< $@

$(BUILD)/%.bin: $(BUILD)/%.elf
	$(CP) -O binary -S $< $@

$(BUILD):
	mkdir -p $@

clean:
	rm -rf $(BUILD)

-include $(wildcard $(BUILD)/*.d)

.PHONY: all clean
