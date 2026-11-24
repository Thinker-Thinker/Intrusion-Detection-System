# Door unit firmware for the WeAct STM32H723VGT6 core board + OV7725: face detection, liveness and
# recognition on the chip (TinyEngine), servo/LEDs/buzzer, ESP32 link.
# Build:  make            (needs arm-none-eabi-gcc on PATH; STM32CubeIDE ships one)
# Output: build/door_unit.bin / .hex   -> internal flash (STM32CubeProgrammer)
#         build/rec_weights.bin        -> external flash, uploaded with: python tools/door.py flash-model

TARGET   = door_unit
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
  $(wildcard Drivers/STM32H7xx_HAL_Driver/Src/*.c) \
  $(wildcard Bsp/W25Qxx/*.c) \
  $(wildcard Models/*.c) $(wildcard Models/kernels/*.c) \
  $(shell find TinyEngine/src TinyEngine/CMSIS/NN/Source -name '*.c')

ASM_SOURCES = startup_stm32h723xx.s

C_DEFS = -DUSE_HAL_DRIVER -DSTM32H723xx -DTFT96 -DARM_MATH_DSP -DARM_MATH_LOOPUNROLL

C_INCLUDES = \
  -ICore/Inc -IBsp -IBsp/Camera -IBsp/ST7735 -IBsp/W25Qxx -IModels \
  -ITinyEngine/include -ITinyEngine/CMSIS/NN/Include -ITinyEngine/CMSIS/DSP/Include \
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

# TinyEngine calls several kernels without prototypes; give the generated models and kernels the real ones
$(BUILD)/obj/Models/%.o $(BUILD)/obj/TinyEngine/%.o: CFLAGS += -include te_protos.h

# objects mirror the source tree (several TinyEngine files share a name)
OBJECTS  = $(addprefix $(BUILD)/obj/,$(C_SOURCES:.c=.o))
OBJECTS += $(addprefix $(BUILD)/obj/,$(ASM_SOURCES:.s=.o))

all: $(BUILD)/$(TARGET).elf $(BUILD)/$(TARGET).hex $(BUILD)/$(TARGET).bin $(BUILD)/rec_weights.bin

$(BUILD)/obj/%.o: %.c Makefile
	@mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) $< -o $@

$(BUILD)/obj/%.o: %.s Makefile
	@mkdir -p $(dir $@)
	$(AS) -c $(ASFLAGS) $< -o $@

$(BUILD)/$(TARGET).elf: $(OBJECTS) Makefile $(LDSCRIPT)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD)/$(TARGET).hex: $(BUILD)/$(TARGET).elf
	$(CP) -O ihex -R .extflash $< $@

$(BUILD)/$(TARGET).bin: $(BUILD)/$(TARGET).elf
	$(CP) -O binary -S -R .extflash $< $@

$(BUILD)/rec_weights.bin: $(BUILD)/$(TARGET).elf
	$(CP) -O binary -j .extflash $< $@

$(BUILD):
	mkdir -p $@

clean:
	rm -rf $(BUILD)

-include $(shell find $(BUILD) -name '*.d' 2>/dev/null)

.PHONY: all clean

# ---- test build: the face pipeline alone, run in an ARM emulator on the PC (models/emu/run_emu.py)
EMU_OBJS = $(filter $(BUILD)/obj/Models/% $(BUILD)/obj/TinyEngine/% $(BUILD)/obj/Core/Src/vision.o $(BUILD)/obj/Core/Src/face_models.o,$(OBJECTS))
emu: $(BUILD)/emu.elf
$(BUILD)/emu.elf: $(EMU_OBJS) ../../../models/emu/emu_main.c
	$(CC) -c $(CFLAGS) ../../../models/emu/emu_main.c -o $(BUILD)/obj/emu_main.o
	$(CC) $(BUILD)/obj/emu_main.o $(EMU_OBJS) $(MCU) -T$(LDSCRIPT) -nostartfiles -e emu_run -Wl,--undefined=emu_embed -specs=nano.specs -specs=nosys.specs -Wl,--gc-sections -lc -lm -o $@
.PHONY: emu
