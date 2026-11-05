# Intrusion Detection System

Facial recognition access control that runs entirely on a microcontroller (TinyML):
face detection, liveness check, identity match and a weapon-presence check, with an
admin dashboard for unrecognised visitors.

## Layout

| Path | Contents |
|---|---|
| `firmware/stm32h723/` | Main target: WeAct STM32H723VGT6 + OV7725 M12 camera ([README](firmware/stm32h723/README.md)) |
| `tinyengine_www_pico/` | MIT TinyEngine ported to the Raspberry Pi Pico 2, running MCUNet's person-detection demo |
| `tools/capture.py` | Laptop tool: live preview and labelled dataset capture from the STM32 board |
| `image_extractor.ipynb` | Converts a test image into a C array for the Pico demo |

## Roadmap

1. Camera bring-up and dataset capture on the H723 (`firmware/stm32h723/camera_stream`)
2. Person-detection demo on the H723 with live frames (runtime check)
3. Face detection (BlazeFace, 128x128)
4. Face recognition embeddings (MobileFaceNet-style) with enrolment on microSD
5. Texture-based liveness classifier
6. Binary weapon-presence classifier
