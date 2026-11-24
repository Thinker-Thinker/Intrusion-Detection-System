# Door unit firmware (STM32H723 + OV7725)

Everything runs on the STM32: SCRFD face detector → 5-point alignment → liveness CNN (photo/screen) →
MobileFaceNet embedding (3-frame average) → cosine match against the enrolled people → servo / LEDs /
buzzer / LCD, events to the ESP32 (USART1) and USB.

| Model | Where the weights live | Arena |
|---|---|---|
| SCRFD-500M detector, 160×128 int8 | internal flash | 160 KB (AXI) |
| Liveness CNN, 64×64 | internal flash | 48 KB (AXI) |
| Recognition (MobileFaceNet-0.5), 112×112 | external W25Q64 (OCTOSPI, memory-mapped) | 294 KB (AXI) |

## Build
`make` → `build/door_unit.bin` (internal flash) and `build/rec_weights.bin` (external flash).

## First-time setup
1. Flash `door_unit.bin` with STM32CubeProgrammer (USB DFU: hold BOOT0, tap NRST; address 0x08000000).
2. Reset. The LCD says "No face model".
3. `python tools/door.py flash-model` (uploads rec_weights.bin over USB, ~10 s). LCD: "Model OK".
4. `python tools/door.py enrol <name>` for each person (10 samples while it beeps).
5. `python tools/door.py monitor --debug` to watch decisions and timings.

Settings are in `Core/Src/door.c` (threshold 0.42, minimum face 48 px, door open 5 s, servo 0°/90°).

## Checks done on the PC
* `models/pcsim`: the same C (vision.c, face_models.c, TinyEngine, generated models) built for x86:
  98.8% faces found, recognition EER 11.2%, 0/338 spoof frames let through, 0/169 empty scenes.
* `models/emu`: this ARM build (`make emu`) run in an emulator (Unicorn, Cortex-M7): detector boxes,
  liveness and recognition bit-identical to the PC build on identical input crops.
