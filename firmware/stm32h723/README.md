# STM32H723 firmware

Target: **WeAct STM32H723VGT6 core board** (Cortex-M7 @ 550 MHz, 564 KB RAM, 1 MB flash,
8 MB OSPI flash, 0.96" ST7735 LCD) with the **OV7725 M12** camera on the 24-pin DVP connector.

| Folder | What it does | Status |
|---|---|---|
| `camera_stream/` | Captures 320x240 RGB565 frames, shows a preview on the LCD, streams frames to a laptop over USB | Step 1: builds, needs testing on hardware |

## camera_stream

Built on WeAct's own `08-DCMI2LCD` example for this board (camera and LCD drivers,
pin map, 550 MHz clock), with these changes:

- capture at **QVGA 320x240** (the working resolution for every model) in snapshot mode
- frame buffer placed in **AXI SRAM** (the DCMI's DMA cannot reach DTCM) with a D-cache
  invalidate after each frame
- **USB CDC virtual COM port** on the USB-C socket, sending each frame with a 16-byte header
- LCD preview: 2x downscale, centre 160x80 crop, FPS counter

Memory after build: about 95 KB flash, 150 KB AXI SRAM (the frame), 57 KB DTCM.

### Build

Option A, STM32CubeIDE:
1. File > New > Project... > C/C++ > **Makefile Project with Existing Code**.
2. Existing Code Location: this `camera_stream` folder. Toolchain: **MCU ARM GCC**. Finish.
3. Build (hammer icon). Output: `build/camera_stream.elf`, `.hex`, `.bin`.

Option B, command line: `make` (needs `arm-none-eabi-gcc` on PATH).

Do not regenerate code from `WeAct_DCMI2LCD_reference.ioc`: it is WeAct's original
configuration and does not contain the USB device, so CubeMX would remove it.

### Flash

USB DFU (no ST-Link needed):
1. Hold **BOOT0**, tap **NRST**, release BOOT0. The board appears as "DFU in FS Mode".
2. In STM32CubeProgrammer choose **USB**, connect, open `build/camera_stream.bin`,
   address `0x08000000`, Download.
3. Tap NRST to run.

With an ST-Link on the SWD header, use Run/Debug in CubeIDE or CubeProgrammer's ST-LINK mode.

### Check it works

1. The LCD shows the WeAct logo, then `Cam id:0x7721` (or 0x7722/0x7723). If it says
   `Camera Not Found`, reseat the camera ribbon.
2. A live preview with an FPS counter follows.
3. On the laptop: `pip install -r tools/requirements.txt` then `python tools/capture.py`.
   Windows shows the board as a COM port (driver is built in on Windows 10/11).

### USB protocol

Host sends single ASCII bytes: `C` stream continuously, `P` pause, `S` one frame,
`I` info line. Each frame is a 20-byte little-endian header
(`"FRM2"`, width u16, height u16, format u16 = 1 for RGB565, seq u16, length u32,
CRC-32 u32 of the payload) followed by `width*height*2` bytes of RGB565, high byte first.
The PC tool drops any frame whose CRC does not match and shows the count as "damaged".
USB full speed gives roughly 2-4 frames per second at QVGA, enough for collecting data.
