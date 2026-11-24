"""
TinyEngine code generation with the fixes this project needs (works for detector, recognition, liveness).

    python tinyengine_codegen.py model_int8.tflite --tinyengine <clone> --out gen_dir [--patches 4 --split 5] [--pin-unconsumed]

Fixes applied on top of stock TinyEngine (see firmware/stm32h723/tinyengine_extra/):
  1. requantisation rounds to nearest (apply_rounding.py) - stock code truncates and drifts layer by layer;
  2. an int8 depthwise layer runs in place, so if its input is also read by another layer, a copy is made first;
  3. 3x3 stride-2 convs with != 3 input channels go to convolve_s8_kernel3_stride2_pad1_fpreq_any
     (stock picks a 3-input-channel-only kernel);
  4. (in the TinyEngine clone) upsample_byte gets (width, height) - the stock generator swapped them.
--pin-unconsumed keeps every graph output alive and writes heads.h (needed for SCRFD's 9 outputs).
"""
import argparse, os, re, subprocess, sys
from pathlib import Path

ap = argparse.ArgumentParser()
ap.add_argument("tflite"); ap.add_argument("--tinyengine", required=True); ap.add_argument("--out", required=True)
ap.add_argument("--patches", type=int, default=0); ap.add_argument("--split", type=int, default=0)
ap.add_argument("--pin-unconsumed", action="store_true")
ap.add_argument("--tf-same", action="store_true", help="Keras model: stride-2 layers pad bottom/right (TF same), not top/left")
a = ap.parse_args()
tflite = str(Path(a.tflite).resolve()); sys.path.insert(0, str(Path(a.tinyengine).resolve()))
out = Path(a.out).resolve(); out.mkdir(parents=True, exist_ok=True); os.chdir(out)
HERE = Path(__file__).resolve().parent

from code_generator.TfliteConvertor import TfliteConvertor
from code_generator.GeneralMemoryScheduler import GeneralMemoryScheduler
from code_generator.CodeGenerator import CodeGenerator
from code_generator.TTEParser import outputInfo
from code_generator.operators.upsample import upSample
from code_generator.InputResizer import PatchResizer
from code_generator.PatchBasedUtil import getPatchParams

tc = TfliteConvertor(tflite); tc.parseOperatorInfo(); L = tc.layer

# fix 2: copy depthwise inputs that other layers still need
i = 0
while i < len(L):
    l = L[i]
    if l.params["op"] == "DEPTHWISE_CONV_2D" and l.params.get("input_dtype", "int8") == "int8":
        g = str(l.input_tensors[0].graph_idx)
        others = [m for m in L if m is not l and any(str(t.graph_idx) == g for t in m.input_tensors)]
        if others:
            p = l.params; cid = f"{g}_copy"
            cp = upSample({"input_idx": g, "output_idx": cid, "factor": 1, "input_dim": 3, "output_dim": 3,
                           "input_h": p["input_h"], "input_w": p["input_w"], "input_c": p["input_c"],
                           "output_h": p["input_h"], "output_w": p["input_w"], "output_c": p["input_c"]})
            for m in others:
                for t in m.input_tensors:
                    if str(t.graph_idx) == g: t.graph_idx = cid
                for k in ("input_idx", "input2_idx", "input1_idx"):
                    if str(m.params.get(k)) == g: m.params[k] = cid
            L.insert(i, cp); i += 1
            print(f"copy before depthwise: {p['input_h']}x{p['input_w']}x{p['input_c']} (read again by {len(others)})")
    i += 1

pins, heads = [], []
if a.pin_unconsumed:
    consumed = {str(t.graph_idx) for l in L for t in l.input_tensors}
    heads = [l for l in L if str(l.output_tensors[0].graph_idx) not in consumed]
    pins = [outputInfo(f"pinned_out_{h.output_tensors[0].graph_idx}", str(h.output_tensors[0].graph_idx),
                       h.output_tensors[0].len(), "int8") for h in heads]

pp = None
if a.patches:
    pp = getPatchParams(L, a.split, a.patches)
    PatchResizer(L).patchResize(pp["layer_cnt"], pp["grain_rf"], pp["grain_rf_height"])
ms = GeneralMemoryScheduler(L, False, False, outputTables=pins, inplace=True, mem_visual_path=str(out / "lifecycle.png"),
                            VisaulizeTrainable=False)
ms.USE_INPLACE = True; ms.allocateMemory()
CodeGenerator(memsche=ms, inplace=True, unsigned_input=False, patch_params=pp, FP_output=False, profile_mode=False,
              fp_requantize=True, tflite_op=False, dummy_address=False, outputTables=pins).codeGeneration()

# fix 3: route multi-channel 3x3 stride-2 convs to the general kernel
gm = out / "codegen/Source/genModel.c"; src = gm.read_text()
src = re.sub(r"convolve_s8_kernel3_inputch3_stride2_pad1_fpreq\((&buffer0\[\d+\]),(\d+),(\d+),(\d+),",
             lambda m: m.group(0) if m.group(4) == "3" else
             f"convolve_s8_kernel3_stride2_pad1_fpreq_any({m.group(1)},{m.group(2)},{m.group(3)},{m.group(4)},", src)
if a.tf_same:
    # fix 5: Keras "same" padding for stride-2 layers (bottom/right) instead of TinyEngine's top/left
    src = src.replace("convolve_s8_kernel3_inputch3_stride2_pad1_fpreq(", "convolve_s8_kernel3_stride2_tfsame_fpreq(")
    src = src.replace("convolve_s8_kernel3_stride2_pad1_fpreq_any(", "convolve_s8_kernel3_stride2_tfsame_fpreq(")
    dw = out / "codegen/Source/depthwise_kernel3x3_stride2_inplace_CHW_fpreq.c"
    if dw.exists():
        d = dw.read_text().replace("depthwise_kernel3x3_stride2_inplace_CHW_fpreq", "depthwise_kernel3x3_stride2_inplace_CHW_fpreq_tfsame").replace("depthwise_kernel3x3_stride2_inplace_kernel_CHW_fpreq", "depthwise_kernel3x3_stride2_inplace_kernel_CHW_fpreq_tfsame")
        # start the 3x3 windows one padded row and column later: original rows 2i..2i+2 instead of 2i-1..2i+1
        d = d.replace("inplace_out, output_offset,output_activation_min, output_activation_max,cols_8b_start, input_x, input_ch);",
                      "inplace_out, output_offset,output_activation_min, output_activation_max,cols_8b_start + input_x + 3, input_x, input_ch);")
        assert "cols_8b_start + input_x + 3" in d, "depthwise template changed"
        (out / "codegen/Source/depthwise_kernel3x3_stride2_inplace_CHW_fpreq_tfsame.c").write_text(d); dw.unlink()
        src = src.replace("depthwise_kernel3x3_stride2_inplace_CHW_fpreq(", "depthwise_kernel3x3_stride2_inplace_CHW_fpreq_tfsame(")
# fix 6: TinyEngine drops a 2-D MEAN (global average pool); insert global_avgpool_s8 before the layer that
# reads the pooled 1x1xC tensor, reading the previous layer's HxWxC output.
import tensorflow as tf
itp = tf.lite.Interpreter(tflite); itp.allocate_tensors(); td = itp.get_tensor_details()
for o in itp._get_ops_details():
    if o["op_name"] != "MEAN": continue
    ti, to = td[o["inputs"][0]], td[o["outputs"][0]]
    _, H, W, C = ti["shape"]; (isc, izp), (osc, ozp) = ti["quantization"], to["quantization"]
    lines = src.split("\n"); outl = []; prev_out = None; done = 0
    for ln in lines:
        m = re.match(r"^\s*[a-zA-Z_0-9]+\((.*)\);\s*$", ln)
        if m and "buffer0" in ln:
            args = re.findall(r"(&buffer0\[\d+\]),(\d+),(\d+),(\d+)", m.group(1))
            if args and args[0][1:] == ("1", "1", str(C)) and prev_out and prev_out[1:] == (str(W), str(H), str(C)):
                outl.append(f"global_avgpool_s8((const int8_t *){prev_out[0]},{W},{H},{C},(int8_t *){args[0][0]},{izp},{isc:.9g}f,{ozp},{osc:.9g}f);")
                done += 1
            if args: prev_out = args[-1]
        outl.append(ln)
    assert done >= 1, "could not place the global average pool"
    src = "\n".join(outl).replace("void invoke(float* labels){", "void global_avgpool_s8(const int8_t*,int,int,int,int8_t*,int,float,int,float);\nvoid invoke(float* labels){", 1)
    print(f"global average pool inserted ({H}x{W}x{C}) at {done} call site(s)")
src += "\nsigned char *te_model_arena(void) { return buffer0; }\n"
gm.write_text(src)
# fix 1: rounding in the generated kernels
subprocess.run([sys.executable, str(HERE.parent / "firmware/stm32h723/tinyengine_extra/apply_rounding.py"),
                str(out / "codegen/Source")], check=True, stdout=subprocess.DEVNULL)

if heads:
    rows = []
    for h in heads:
        p = h.params
        rows.append((p["output_h"], p["output_w"], p["output_c"], h._getBufferstr(p["output_buf_add"], p["output_buf_add_offset"]),
                     float(p["output_scale"]), int(p["output_zero_point"])))
    with open(out / "codegen/Include/heads.h", "w") as f:
        f.write("/* Generated: every model output kept alive in the TinyEngine arena (HWC int8). */\n#pragma once\n#include <stdint.h>\n")
        f.write("typedef struct { int h, w, c; const int8_t *data; float scale; int zero_point; } te_head_t;\n")
        f.write(f"#define TE_N_HEADS {len(rows)}\n#define TE_HEADS(buffer0) {{ \\\n")
        for hh, ww, cc, addr, sc, zp in sorted(rows, key=lambda r: (-r[0], r[2])):
            f.write(f"    {{{hh}, {ww}, {cc}, (const int8_t *){addr}, {sc:.9g}f, {zp}}}, \\\n")
        f.write("}\n")
peak = int(re.search(r"#define PEAK_MEM (\d+)", (out / "codegen/Include/genModel.h").read_text()).group(1))
print(f"layers {len(L)} | arena incl. scratch {peak/1024:.1f} KB")
