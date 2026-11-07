# Face recognition: MobileFaceNet on TinyEngine

Feasibility check (7 Nov 2026): can MobileFaceNet (Chen et al., 2018) run on the
STM32H723 through MIT TinyEngine? **Yes, with three changes.** Weights here are
random: these numbers cover memory and compute only, not accuracy.

## Changes needed for TinyEngine

1. **PReLU → ReLU.** TinyEngine has no PReLU layer. The model must be (re)trained with ReLU.
2. **GDConv → global average pooling.** TinyEngine assumes "same" padding for every
   depthwise layer, so MobileFaceNet's 7x7 "valid" GDConv would be computed wrongly
   *without any error*. GAP (a MEAN op) is supported.
3. **Static batch of 1** (`Input(batch_size=1)` + `Reshape`), otherwise TFLite adds
   SHAPE/PACK ops that TinyEngine rejects.

TinyEngine's code generator also needs `tinyengine_numpy2.patch` to run with NumPy 2
(integer overflow in its bias pre-computation; results unchanged).

## Results (int8, TinyEngine, linked for Cortex-M7)

| Variant | Params | MACs | Peak RAM (arena) | Flash (weights+code) |
|---|---|---|---|---|
| 112px, width 1.0, plain | 0.99 M | 221 M | 588 KB ✗ | ~1.06 MB ✗ |
| 112px, width 1.0, patch 4x4 split 5 | 0.99 M | ~244 M | 299 KB (94% of AXI SRAM) | 1.06 MB → **needs external flash** |
| 112px, width 1.0, patch 4x4 split 24 | 0.99 M | ~765 M (3.5x recompute) | 208 KB | 1.06 MB |
| **112px, width 0.5, patch 4x4 split 5** | **0.28 M** | **~66 M** | **150 KB** | **328 KB** ✓ |
| 96px, width 1.0, patch 4x4 split 5 | 0.99 M | ~182 M | 216 KB | ~1.06 MB |

"Patch" = MCUNetV2 patch-based inference (Lin et al., 2021): the first layers run on
overlapping tiles so the large early feature maps never exist in full. Splitting late
cuts memory further but the overlap makes compute grow 3-4x.

**Recommended starting point: 112px, width 0.5, patch-based (4x4, split at layer 5).**
It fits internal flash and leaves the AXI SRAM for the camera frame. If its accuracy is
too low, move to width 1.0 with weights in the 8 MB external flash.

## Next

- Train the width-0.5 ReLU model. Fastest route: knowledge distillation from a
  pre-trained full MobileFaceNet (match its embeddings on unlabelled face crops),
  then fine-tune with ArcFace loss. Report LFW accuracy before/after int8.
- Time it on the board (latency is not estimated here; it will be measured).

## Files

- `mobilefacenet_relu.py`: builds the model and exports int8 TFLite.
- `tinyengine_check.py`: runs TinyEngine codegen, prints layers, MACs, peak RAM. `--patches N --split K` for patch-based.
- `build_check_m7.sh`: compiles + links the generated C for the H723 for real flash/RAM figures.
- `tinyengine_numpy2.patch`: apply inside a tinyengine clone (`git apply`).

Reproduce:
```
python mobilefacenet_relu.py --width 0.5 --gap --out build
python tinyengine_check.py build/mobilefacenet_relu_112_w0.5_gap_int8.tflite --tinyengine ../../tinyengine --out gen/w05 --patches 4 --split 5
./build_check_m7.sh gen/w05 ../../tinyengine ../../firmware/stm32h723/camera_stream/STM32H723VGTX_FLASH.ld
```
