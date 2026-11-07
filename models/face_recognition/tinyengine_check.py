"""
Feasibility check: run an int8 .tflite model through MIT TinyEngine's code
generator and report the layers, MACs, peak SRAM and weight size.

    python tinyengine_check.py path/to/model_int8.tflite --tinyengine ../../te --out gen/

Writes the generated C (codegen/Include, codegen/Source) under --out.
"""
import argparse
import os
import sys
from pathlib import Path

ap = argparse.ArgumentParser()
ap.add_argument("tflite")
ap.add_argument("--tinyengine", required=True, help="path to a clone of mit-han-lab/tinyengine")
ap.add_argument("--out", default="gen")
ap.add_argument("--patches", type=int, default=0, help="MCUNetV2 patch-based inference: n x n patches (0 = off)")
ap.add_argument("--split", type=int, default=0, help="layer index where patch-based stage ends")
a = ap.parse_args()

tflite = str(Path(a.tflite).resolve())
sys.path.insert(0, str(Path(a.tinyengine).resolve()))
out = Path(a.out).resolve(); out.mkdir(parents=True, exist_ok=True)
os.chdir(out)

from code_generator.TfliteConvertor import TfliteConvertor
from code_generator.GeneralMemoryScheduler import GeneralMemoryScheduler
from code_generator.CodeGenerator import CodeGenerator
from code_generator.InputResizer import PatchResizer
from code_generator.PatchBasedUtil import getPatchParams

tf_convertor = TfliteConvertor(tflite)
tf_convertor.parseOperatorInfo()
layers = tf_convertor.layer

total_macs, weight_bytes = 0, 0
print(f"{'#':>3} {'op':<22} {'in (HxWxC)':<16} {'out (HxWxC)':<16} {'k':>4} {'MACs':>12}")
for i, l in enumerate(layers):
    p = l.params
    op = p.get("op", type(l).__name__)
    ins = f"{p.get('input_h','')}x{p.get('input_w','')}x{p.get('input_c','')}"
    outs = f"{p.get('output_h','')}x{p.get('output_w','')}x{p.get('output_c','')}"
    k = f"{p.get('kernel_h','')}" if p.get("kernel_h") else ""
    try:
        macs = l.get_macs()
    except Exception:
        macs = 0
    try:
        weight_bytes += l.get_weights_size()
    except Exception:
        pass
    total_macs += macs
    print(f"{i:>3} {op:<22} {ins:<16} {outs:<16} {k:>4} {macs:>12,}")

patch_params = None
if a.patches:
    patch_params = getPatchParams(layers, a.split, a.patches)
    PatchResizer(layers).patchResize(patch_params["layer_cnt"], patch_params["grain_rf"], patch_params["grain_rf_height"])

memory_scheduler = GeneralMemoryScheduler(layers, False, False, outputTables=[], inplace=True,
                                          mem_visual_path=str(out / "lifecycle.png"), VisaulizeTrainable=False)
memory_scheduler.USE_INPLACE = True
memory_scheduler.allocateMemory()

code_generator = CodeGenerator(memsche=memory_scheduler, inplace=memory_scheduler.USE_INPLACE,
                               unsigned_input=False, patch_params=patch_params, FP_output=False,
                               profile_mode=False, fp_requantize=not a.patches, tflite_op=False, dummy_address=False,
                               outputTables=[])
code_generator.codeGeneration()

peak = memory_scheduler.buffers["input_output"]
print(f"\nlayers: {len(layers)}   MACs: {total_macs/1e6:.1f} M")
print(f"peak activation SRAM (TinyEngine arena): {peak/1024:.1f} KB")
print(f"weights+bias (approx): {weight_bytes/1024:.1f} KB")
