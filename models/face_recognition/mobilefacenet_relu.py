"""
MobileFaceNet (Chen et al., 2018) with PReLU replaced by ReLU so that every
layer is supported by MIT TinyEngine's code generator.

Architecture follows Table 1 of the paper (112x112 input, 128-d embedding):
  conv3x3 s2 64 -> dwconv3x3 64 -> bottlenecks (t=2/4) -> conv1x1 512
  -> linear GDConv7x7 -> linear conv1x1 128 -> BN -> embedding

`width` scales all channel counts; `input_size` may be lowered (e.g. 96) to cut
memory and MACs. With input_size != 112 the GDConv kernel follows the final
feature-map size.

Usage:
    python mobilefacenet_relu.py --out build/        # builds + int8-quantises
    python mobilefacenet_relu.py --size 96 --width 0.5
"""
import argparse
from pathlib import Path

import numpy as np
import tensorflow as tf
from tensorflow.keras import layers, Model

BN_EPS = 1e-5


def conv_bn(x, filters, k, s, act=True, name=None):
    x = layers.Conv2D(filters, k, s, padding="same", use_bias=False, name=f"{name}_conv")(x)
    x = layers.BatchNormalization(epsilon=BN_EPS, name=f"{name}_bn")(x)
    return layers.ReLU(name=f"{name}_relu")(x) if act else x


def dw_bn(x, k, s, act=True, name=None, padding="same"):
    x = layers.DepthwiseConv2D(k, s, padding=padding, use_bias=False, name=f"{name}_dw")(x)
    x = layers.BatchNormalization(epsilon=BN_EPS, name=f"{name}_bn")(x)
    return layers.ReLU(name=f"{name}_relu")(x) if act else x


def bottleneck(x, out_c, t, s, name):
    in_c = x.shape[-1]
    y = conv_bn(x, in_c * t, 1, 1, name=f"{name}_expand")
    y = dw_bn(y, 3, s, name=f"{name}_dwise")
    y = conv_bn(y, out_c, 1, 1, act=False, name=f"{name}_project")
    if s == 1 and in_c == out_c:
        y = layers.Add(name=f"{name}_add")([x, y])
    return y


def mobilefacenet(input_size=112, width=1.0, emb=128, gdconv=True):
    c = lambda n: max(8, int(round(n * width / 8)) * 8)
    inp = layers.Input((input_size, input_size, 3), batch_size=1, name="face")  # static batch: no dynamic-shape ops
    x = conv_bn(inp, c(64), 3, 2, name="conv1")
    x = dw_bn(x, 3, 1, name="dwconv1")
    # (t, c, n, s) from the paper
    cfg = [(2, 64, 5, 2), (4, 128, 1, 2), (2, 128, 6, 1), (4, 128, 1, 2), (2, 128, 2, 1)]
    i = 0
    for t, ch, n, s in cfg:
        for j in range(n):
            x = bottleneck(x, c(ch), t, s if j == 0 else 1, name=f"bneck{i}")
            i += 1
    x = conv_bn(x, c(512), 1, 1, name="conv_last")
    k = x.shape[1]
    if gdconv:
        # Global depthwise conv: kxk depthwise, valid padding -> 1x1 (linear)
        x = dw_bn(x, k, 1, act=False, name="gdconv", padding="valid")
    else:
        x = layers.GlobalAveragePooling2D(keepdims=True, name="gap")(x)
    x = layers.Conv2D(emb, 1, use_bias=False, name="emb_conv")(x)
    x = layers.BatchNormalization(epsilon=BN_EPS, name="emb_bn")(x)
    x = layers.Reshape((emb,), name="embedding")(x)  # fixed shape: no SHAPE/PACK ops
    return Model(inp, x, name=f"mobilefacenet_relu_{input_size}_w{width}")


def to_int8_tflite(model, rep_images):
    """Full-integer quantisation (int8 in, int8 out) as TinyEngine expects."""
    def rep():
        for im in rep_images:
            yield [im[None].astype(np.float32)]
    conv = tf.lite.TFLiteConverter.from_keras_model(model)
    conv.optimizations = [tf.lite.Optimize.DEFAULT]
    conv.representative_dataset = rep
    conv.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS_INT8]
    conv.inference_input_type = tf.int8
    conv.inference_output_type = tf.int8
    return conv.convert()


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--size", type=int, default=112)
    ap.add_argument("--width", type=float, default=1.0)
    ap.add_argument("--gap", action="store_true", help="global average pool instead of GDConv")
    ap.add_argument("--out", default="build")
    a = ap.parse_args()
    m = mobilefacenet(a.size, a.width, gdconv=not a.gap)
    print(m.name, "params:", m.count_params())
    # Random images only check conversion and memory; real calibration uses face crops.
    rep = np.random.uniform(-1, 1, (32, a.size, a.size, 3))
    out = Path(a.out); out.mkdir(parents=True, exist_ok=True)
    p = out / f"{m.name}{'_gap' if a.gap else ''}_int8.tflite"
    p.write_bytes(to_int8_tflite(m, rep))
    print("wrote", p, p.stat().st_size, "bytes")
