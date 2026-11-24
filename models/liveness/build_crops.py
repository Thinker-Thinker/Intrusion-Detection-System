#!/usr/bin/env python3
"""
Cut a loose face crop (1.6 x the detector box, like the board will) from every live and spoof frame.

    python build_crops.py <dataset_dir> <out_dir> --models <dir containing models/buffalo_s>

<dataset_dir> holds live_face/<person>/*.png, spoof_print/*.png, spoof_screen/*.png (capture-tool PNGs,
320x240, names start with the capture time). Output: <out_dir>/crops/<class>/<file>.png (128x128) and
<out_dir>/crops.csv with class, person, session, face width, detector score.
Detection uses InsightFace SCRFD-500M (buffalo_s, non-commercial research licence); data preparation only.
"""
import argparse, csv, glob, os
from datetime import datetime
import numpy as np, cv2

ap = argparse.ArgumentParser()
ap.add_argument("dataset"); ap.add_argument("out")
ap.add_argument("--models", default=os.path.dirname(os.path.abspath(__file__)))
ap.add_argument("--merge", nargs="*", default=["Gideon 2=Gideon"])
ap.add_argument("--min-face", type=int, default=24)
ap.add_argument("--min-score", type=float, default=0.5)
ap.add_argument("--session-gap", type=float, default=60.0, help="seconds between frames that start a new session")
a = ap.parse_args()

from insightface.app import FaceAnalysis
app = FaceAnalysis(name="buffalo_s", root=a.models, allowed_modules=["detection"], providers=["CPUExecutionProvider"])
app.prepare(ctx_id=-1, det_size=(320, 320), det_thresh=0.4)
merge = dict(m.split("=", 1) for m in a.merge)

def loose_crop(img, box, size=128, scale=1.6):
    x1, y1, x2, y2 = box; cx, cy = (x1 + x2) / 2, (y1 + y2) / 2
    s = max(x2 - x1, y2 - y1) * scale
    M = np.float32([[size / s, 0, size / 2 - cx * size / s], [0, size / s, size / 2 - cy * size / s]])
    return cv2.warpAffine(img, M, (size, size), borderMode=cv2.BORDER_REPLICATE)

def stamp(name):
    try: return datetime.strptime(name[:22], "%Y%m%d_%H%M%S_%f").timestamp()
    except ValueError: return 0.0

items = []
for d in sorted(glob.glob(f"{a.dataset}/live_face/*/")):
    person = merge.get(os.path.basename(d.rstrip("/")), os.path.basename(d.rstrip("/")))
    items += [("live", person, f) for f in sorted(glob.glob(f"{d}*.png"))]
for cls in ("spoof_print", "spoof_screen"):
    items += [(cls, "", f) for f in sorted(glob.glob(f"{a.dataset}/{cls}/*.png"))]

os.makedirs(f"{a.out}/crops", exist_ok=True)
rows, last_t, session = [], {}, {}
for cls, person, f in sorted(items, key=lambda x: (x[0], x[1], os.path.basename(x[2]))):
    key = (cls, person); t = stamp(os.path.basename(f))
    if key not in last_t or t - last_t[key] > a.session_gap: session[key] = session.get(key, -1) + 1
    last_t[key] = t
    img = cv2.imread(f); faces = app.get(img) if img is not None else []
    if not faces: continue
    m = max(faces, key=lambda z: (z.bbox[2] - z.bbox[0]) * (z.bbox[3] - z.bbox[1]))
    w = float(m.bbox[2] - m.bbox[0])
    if w < a.min_face or m.det_score < a.min_score: continue
    sub = f"{cls}/{person}" if person else cls
    os.makedirs(f"{a.out}/crops/{sub}", exist_ok=True)
    out = f"crops/{sub}/{os.path.basename(f)}"
    cv2.imwrite(f"{a.out}/{out}", loose_crop(img, m.bbox))
    rows.append({"file": out, "label": 0 if cls == "live" else 1, "class": cls, "person": person,
                 "session": f"{cls}:{person}:{session[key]}", "face_w": round(w), "score": round(float(m.det_score), 3)})
with open(f"{a.out}/crops.csv", "w", newline="") as fh:
    wr = csv.DictWriter(fh, fieldnames=list(rows[0])); wr.writeheader(); wr.writerows(rows)
print(len(rows), "crops from", len(items), "frames")
