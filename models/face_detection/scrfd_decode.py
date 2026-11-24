"""Decode SCRFD-500M raw head outputs (the C firmware ports this exactly)."""
import numpy as np

STRIDES = (8, 16, 32)

def prep(frame_rgb, W=160, H=128):
    """320x240 RGB -> 160x120, zero-padded at the bottom to 160x128 (uint8). Returns image and scale."""
    import cv2
    h0, w0 = frame_rgb.shape[:2]
    s = W / w0
    r = cv2.resize(frame_rgb, (W, round(h0 * s)), interpolation=cv2.INTER_AREA)
    c = np.zeros((H, W, 3), np.uint8); c[:r.shape[0]] = r
    return c, s

def decode(outs, W=160, H=128, thresh=0.5, nms=0.4):
    """outs: dict stride -> (score[h,w,2], bbox[h,w,8], kps[h,w,20]) as float. Returns list of (score, box[4], kps[5,2])."""
    dets = []
    for st in STRIDES:
        sc, bb, kp = outs[st]
        h, w = sc.shape[:2]
        prob = 1 / (1 + np.exp(-sc))
        ys, xs, a = np.where(prob >= thresh)
        for y, x, k in zip(ys, xs, a):
            cx, cy = x * st, y * st
            l, t, r, b = bb[y, x, 4 * k:4 * k + 4] * st
            pts = kp[y, x, 10 * k:10 * k + 10].reshape(5, 2) * st + (cx, cy)
            dets.append((float(prob[y, x, k]), np.array([cx - l, cy - t, cx + r, cy + b]), pts))
    dets.sort(key=lambda d: -d[0])
    keep = []
    for d in dets:
        if all(iou(d[1], k[1]) < nms for k in keep): keep.append(d)
    return keep

def iou(a, b):
    ix = max(0, min(a[2], b[2]) - max(a[0], b[0])); iy = max(0, min(a[3], b[3]) - max(a[1], b[1]))
    i = ix * iy; u = (a[2] - a[0]) * (a[3] - a[1]) + (b[2] - b[0]) * (b[3] - b[1]) - i
    return i / u if u > 0 else 0
