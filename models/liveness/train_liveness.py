#!/usr/bin/env python3
"""
Train and test the liveness (real face vs printed photo / screen) model for the STM32H723.

    python train_liveness.py <out_dir from build_crops.py> [--size 64] [--epochs 40]

Test set = 7 live people and about a quarter of the print/screen sessions that are never used in training,
so the score reflects new people and new spoof sessions. Reports APCER (spoofs let through), BPCER (real
people rejected), EER, results by face size, and a "face size alone" baseline to show the model is not
just measuring how big the face is. Exports an int8 TFLite model (static batch 1, TinyEngine ops only).
"""
import argparse, json, os, random
import numpy as np, pandas as pd, cv2, tensorflow as tf
from tensorflow.keras import layers, Model

ap = argparse.ArgumentParser()
ap.add_argument("data"); ap.add_argument("--size", type=int, default=64); ap.add_argument("--epochs", type=int, default=40)
ap.add_argument("--seed", type=int, default=0)
ap.add_argument("--all", action="store_true", help="final model: train on every photo, no test")
a = ap.parse_args()
random.seed(a.seed); np.random.seed(a.seed); tf.random.set_seed(a.seed)
S = a.size
d = pd.read_csv(f"{a.data}/crops.csv")

# ---------------- split: held-out live people and spoof sessions ----------------
people = sorted(d[d.label == 0].person.unique())
rng = random.Random(a.seed); test_people = set(rng.sample(people, 7))
test_sessions = set()
for cls in ("spoof_print", "spoof_screen"):
    ss = d[d["class"] == cls].groupby("session").size().sort_values(ascending=False).index.tolist()
    test_sessions |= set(ss[(1 + a.seed) % 4::4])        # every 4th session by size; offset changes with --seed
d["test"] = (d.person.isin(test_people) | d.session.isin(test_sessions)) & (not a.all)
tr, te = d[~d.test].reset_index(drop=True), d[d.test].reset_index(drop=True)
print("train:", tr["class"].value_counts().to_dict(), "| test:", te["class"].value_counts().to_dict())
print("test people:", sorted(test_people))

live_w = tr[tr.label == 0].face_w.values               # face sizes seen at the door, for the size augmentation

def load(f): return cv2.cvtColor(cv2.imread(f"{a.data}/{f}"), cv2.COLOR_BGR2RGB)

def augment(img, w):
    # 1) make every face look like a door-camera face size: shrink to a width drawn from the live faces
    if random.random() < 0.85:
        w2 = min(w, float(np.random.choice(live_w)) * random.uniform(0.85, 1.15))
        f = w2 / w
        if f < 0.98:
            small = cv2.resize(img, (max(8, int(128 * f)), max(8, int(128 * f))), interpolation=cv2.INTER_AREA)
            img = cv2.resize(small, (128, 128), interpolation=cv2.INTER_LINEAR)
    # 2) framing jitter, mirror, light
    c = random.randint(104, 128); x = random.randint(0, 128 - c); y = random.randint(0, 128 - c)
    img = img[y:y + c, x:x + c]
    if random.random() < 0.5: img = img[:, ::-1]
    img = img.astype(np.float32) * random.uniform(0.7, 1.3) + random.uniform(-30, 30)
    if random.random() < 0.3: img = cv2.GaussianBlur(img, (3, 3), random.uniform(0.3, 1.0))
    return cv2.resize(np.clip(img, 0, 255).astype(np.uint8), (S, S), interpolation=cv2.INTER_AREA)

def plain(img): return cv2.resize(img, (S, S), interpolation=cv2.INTER_AREA)
pre = lambda x: (x.astype(np.float32) - 127.5) / 127.5

tr_img = [load(f) for f in tr.file]; te_img = [load(f) for f in te.file]
Xte = pre(np.stack([plain(i) for i in te_img])) if len(te) else np.zeros((0, S, S, 3), np.float32); yte = te.label.values

# ---------------- model: small, TinyEngine-friendly (conv, depthwise, ReLU, GAP, 1x1 conv) ----------------
def block(x, c, s, n):
    x = layers.DepthwiseConv2D(3, s, padding="same", use_bias=False, name=f"{n}_dw")(x)
    x = layers.BatchNormalization(name=f"{n}_dwbn")(x); x = layers.ReLU(name=f"{n}_dwr")(x)
    x = layers.Conv2D(c, 1, use_bias=False, name=f"{n}_pw")(x)
    x = layers.BatchNormalization(name=f"{n}_pwbn")(x); return layers.ReLU(name=f"{n}_pwr")(x)
def net(batch=None):
    i = layers.Input((S, S, 3), batch_size=batch, name="crop")
    x = layers.Conv2D(16, 3, 2, padding="same", use_bias=False, name="c0")(i)
    x = layers.BatchNormalization(name="c0bn")(x); x = layers.ReLU(name="c0r")(x)
    for k, (c, s) in enumerate([(32, 1), (48, 2), (48, 1), (64, 2), (64, 1), (96, 2), (96, 1)]):
        x = block(x, c, s, f"b{k}")
    x = layers.GlobalAveragePooling2D(keepdims=True, name="gap")(x)
    x = layers.Dropout(0.2, name="drop")(x)
    x = layers.Conv2D(2, 1, name="logits")(x)
    return Model(i, layers.Reshape((2,), name="out")(x), name="liveness")
m = net(); print(f"{m.count_params():,} parameters")

class Seq(tf.keras.utils.Sequence):
    def __init__(self): super().__init__(); self.idx = np.arange(len(tr))
    def __len__(self): return len(self.idx) // 64
    def on_epoch_end(self): np.random.shuffle(self.idx)
    def __getitem__(self, b):
        j = self.idx[b * 64:(b + 1) * 64]
        return pre(np.stack([augment(tr_img[k], tr.face_w[k]) for k in j])), tr.label.values[j]
cw = {0: len(tr) / (2 * (tr.label == 0).sum()), 1: len(tr) / (2 * (tr.label == 1).sum())}
steps = (len(tr) // 64) * a.epochs
m.compile(tf.keras.optimizers.Adam(tf.keras.optimizers.schedules.CosineDecay(3e-3, steps, alpha=0.02)),
          tf.keras.losses.SparseCategoricalCrossentropy(from_logits=True), metrics=["accuracy"])
seq = Seq(); seq.on_epoch_end()
m.fit(seq, epochs=a.epochs, class_weight=cw, verbose=2)

# ---------------- test ----------------
def spoof_prob(logits): e = np.exp(logits - logits.max(1, keepdims=True)); return e[:, 1] / e.sum(1)
def report(p, name):
    live, spoof = p[yte == 0], p[yte == 1]
    th = np.linspace(0, 1, 1001)
    apcer = np.array([(spoof < t).mean() for t in th]); bpcer = np.array([(live >= t).mean() for t in th])
    k = np.argmin(np.abs(apcer - bpcer))
    r = {"model": name, "EER_%": round(100 * (apcer[k] + bpcer[k]) / 2, 2), "thr_EER": round(float(th[k]), 3),
         "APCER@0.5_%": round(100 * (spoof < 0.5).mean(), 2), "BPCER@0.5_%": round(100 * (live >= 0.5).mean(), 2)}
    for cls in ("spoof_print", "spoof_screen"):
        r[f"APCER@0.5_{cls}_%"] = round(100 * (p[(te["class"] == cls).values] < 0.5).mean(), 2)
    for lo, hi in ((0, 48), (48, 1000)):
        lm = (yte == 0) & (te.face_w.values >= lo) & (te.face_w.values < hi)
        sm = (yte == 1) & (te.face_w.values >= lo) & (te.face_w.values < hi)
        if lm.any(): r[f"BPCER@0.5_face{lo}-{hi}px_%"] = round(100 * (p[lm] >= 0.5).mean(), 2)
        if sm.any(): r[f"APCER@0.5_face{lo}-{hi}px_%"] = round(100 * (p[sm] < 0.5).mean(), 2)
    return r

res = [report(spoof_prob(m.predict(Xte, verbose=0)), "float32")] if len(te) else []
# baseline: decide from face size alone (bigger face = spoof)
if len(te):
    wz = te.face_w.values.astype(float); res.append(report((wz - wz.min()) / (wz.max() - wz.min()), "face size alone (baseline)"))
    del res[-1]["APCER@0.5_%"], res[-1]["BPCER@0.5_%"]

# ---------------- int8 export (static batch 1) ----------------
dep = net(batch=1); dep.set_weights(m.get_weights())
rep_imgs = random.sample(tr_img, 300)
def rep():
    for im in rep_imgs: yield [pre(plain(im))[None]]
cv = tf.lite.TFLiteConverter.from_keras_model(dep)
cv.optimizations = [tf.lite.Optimize.DEFAULT]; cv.representative_dataset = rep
cv.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS_INT8]
cv.inference_input_type = tf.int8; cv.inference_output_type = tf.int8
tfl = cv.convert(); open(f"{a.data}/liveness_int8.tflite", "wb").write(tfl)
itp = tf.lite.Interpreter(model_content=tfl); itp.allocate_tensors()
ii, oo = itp.get_input_details()[0], itp.get_output_details()[0]
out = []
for x in Xte[:0] if not len(te) else Xte:
    s, z = ii["quantization"]; itp.set_tensor(ii["index"], np.clip(np.round(x / s + z), -128, 127).astype(np.int8)[None]); itp.invoke()
    s2, z2 = oo["quantization"]; out.append((itp.get_tensor(oo["index"]).astype(np.float32) - z2) * s2)
if len(te): res.append(report(spoof_prob(np.concatenate(out)), "int8 (on-board)"))
m.save_weights(f"{a.data}/liveness.weights.h5")
if res:
    pd.DataFrame(res).to_csv(f"{a.data}/liveness_results.csv", index=False); print(pd.DataFrame(res).T.to_string())
print("int8 file", len(tfl) // 1024, "KB")
