"""Face pipeline of the door unit, in Python (ONNX Runtime), for running the door from a PC.

Same models and the same steps as the STM32 firmware (firmware/stm32h723/door_unit/Core/Src/vision.c):
SCRFD detector on a 160x128 input -> largest face -> 5-point ArcFace alignment to 112x112 -> embedding;
loose 1.6x crop to 64x64 -> liveness. The models are the int8 TFLite models converted to ONNX.
"""
import json
from pathlib import Path

import cv2
import numpy as np
import onnxruntime as ort

HERE = Path(__file__).resolve().parent / "models"
ARC_DST = np.array([[38.2946, 51.6963], [73.5318, 51.5014], [56.0252, 71.7366],
                    [41.5493, 92.3655], [70.7299, 92.2041]], np.float32)
FRAME_W, FRAME_H, DET_W, DET_H = 320, 240, 160, 128


def q_input(rgb_u8):
    """uint8 0..255 -> model int8 (q = round_half_even(x - 127.5) - 1), as vis_q() in the firmware."""
    return np.clip(np.round(rgb_u8.astype(np.float32) - 127.5) - 1, -128, 127).astype(np.int8)


class Engine:
    def __init__(self, threads=2):
        o = ort.SessionOptions(); o.intra_op_num_threads = threads
        self.q = json.load(open(HERE / "quant.json"))
        self.det = ort.InferenceSession(str(HERE / "det.onnx"), o, providers=["CPUExecutionProvider"])
        self.rec = ort.InferenceSession(str(HERE / "rec_r4.onnx"), o, providers=["CPUExecutionProvider"])
        self.liv = ort.InferenceSession(str(HERE / "liv.onnx"), o, providers=["CPUExecutionProvider"])
        self.det_out = [x.name for x in self.det.get_outputs()]

    # ---------------------------------------------------------------- detection
    def detect(self, rgb, thresh=0.5, nms=0.4):
        """rgb: 240x320x3 uint8. Returns list of (score, box[4], kps[5,2]) in frame pixels, best first."""
        small = rgb.reshape(FRAME_H // 2, 2, FRAME_W // 2, 2, 3).astype(np.int32).sum((1, 3))
        small = ((small + 2) >> 2).astype(np.uint8)
        x = np.full((DET_H, DET_W, 3), q_input(np.zeros(1, np.uint8))[0], np.int8)
        x[:FRAME_H // 2] = q_input(small)
        outs = self.det.run(None, {self.det.get_inputs()[0].name: x[None]})
        heads = {}
        for name, o in zip(self.det_out, outs):
            shape, scale, zp = self.q["det"]["outputs"][name]
            heads.setdefault(o.shape[1], {})[o.shape[3]] = (o[0].astype(np.float32) - zp) * scale
        cands = []
        k = FRAME_W / DET_W
        for h, d in heads.items():
            stride = DET_H // h
            sc, bb, kp = d[2], d[8], d[20]
            prob = 1 / (1 + np.exp(-sc))
            for y, xx, a in zip(*np.where(prob >= thresh)):
                cx, cy = xx * stride, y * stride
                b = bb[y, xx, 4 * a:4 * a + 4] * stride
                box = np.array([cx - b[0], cy - b[1], cx + b[2], cy + b[3]]) * k
                kps = (np.array([cx, cy]) + kp[y, xx, 10 * a:10 * a + 10].reshape(5, 2) * stride) * k
                cands.append((float(prob[y, xx, a]), box, kps))
        cands.sort(key=lambda c: -c[0])
        keep = []
        for c in cands:
            if all(iou(c[1], d[1]) < nms for d in keep):
                keep.append(c)
        return keep

    # ---------------------------------------------------------------- crops
    @staticmethod
    def align112(rgb, kps):
        m = similarity(np.asarray(kps, np.float32), ARC_DST)
        return cv2.warpAffine(rgb, m, (112, 112), flags=cv2.INTER_LINEAR, borderValue=0)

    @staticmethod
    def loose64(rgb, box):
        cx, cy = (box[0] + box[2]) / 2, (box[1] + box[3]) / 2
        side = max(box[2] - box[0], box[3] - box[1]) * 1.6
        kk = 128.0 / side
        m = np.array([[kk, 0, 64 - cx * kk], [0, kk, 64 - cy * kk]], np.float32)
        big = cv2.warpAffine(rgb, m, (128, 128), flags=cv2.INTER_LINEAR, borderMode=cv2.BORDER_REPLICATE)
        return big.reshape(64, 2, 64, 2, 3).astype(np.int32).sum((1, 3)).__add__(2).__rshift__(2).astype(np.uint8)

    # ---------------------------------------------------------------- models
    def spoof_prob(self, crop64):
        o = self.liv.run(None, {self.liv.get_inputs()[0].name: q_input(crop64)[None]})[0][0].astype(np.float32)
        _, scale, zp = next(iter(self.q["liv"]["outputs"].values()))
        live, spoof = (o - zp) * scale
        return float(1 / (1 + np.exp(live - spoof)))

    def embed(self, crop112):
        o = self.rec.run(None, {self.rec.get_inputs()[0].name: q_input(crop112)[None]})[0][0].astype(np.float32)
        _, _, zp = next(iter(self.q["rec"]["outputs"].values()))
        e = o - zp
        return e / (np.linalg.norm(e) + 1e-12)


def iou(a, b):
    ix, iy = min(a[2], b[2]) - max(a[0], b[0]), min(a[3], b[3]) - max(a[1], b[1])
    if ix <= 0 or iy <= 0:
        return 0.0
    i = ix * iy
    return i / ((a[2] - a[0]) * (a[3] - a[1]) + (b[2] - b[0]) * (b[3] - b[1]) - i)


def similarity(src, dst):
    """Least-squares similarity transform src -> dst (2x3), as in vis_align112()."""
    ms, md = src.mean(0), dst.mean(0)
    p, q = src - ms, dst - md
    var = (p ** 2).sum()
    a = (p[:, 0] * q[:, 0] + p[:, 1] * q[:, 1]).sum() / var
    b = (p[:, 0] * q[:, 1] - p[:, 1] * q[:, 0]).sum() / var
    t = md - np.array([a * ms[0] - b * ms[1], b * ms[0] + a * ms[1]])
    return np.array([[a, -b, t[0]], [b, a, t[1]]], np.float32)
