# Liveness (real face vs printed photo / phone or laptop screen)

Runs on the STM32H723 after face detection, on a loose crop (1.6x the detector box) resized to 64x64 RGB.
Output: two logits (live, spoof).

| File | What |
|---|---|
| `build_crops.py` | Cuts the loose face crops from `dataset/live_face`, `spoof_print`, `spoof_screen` (needs InsightFace buffalo_s) |
| `train_liveness.py` | Trains and tests (held-out people and spoof sessions); `--all` trains the final model on everything |
| `liveness_int8.tflite` | Final model, trained on all 3,171 crops (int8, static batch 1) |
| `liveness_results_3splits.csv` | Test results on three different held-out splits |

On the board (TinyEngine): 3.1 M MACs, 48 KB peak RAM, about 30 KB of weights, 62 KB TFLite file.

Training data: 2,155 live crops (30 people), 559 print and 457 screen crops (16 and 10 capture sessions).
Every spoof crop is randomly shrunk to a live-face size during training, because spoofs were captured with
much bigger faces (median 107 px vs 58 px); the "face size alone" baseline in the results shows how much
size by itself would give away.
