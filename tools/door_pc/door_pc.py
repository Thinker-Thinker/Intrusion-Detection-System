"""Door unit, PC version: an ESP32-CAM (or any webcam) sends the picture, this laptop runs the face
pipeline (same models and rules as the STM32 firmware), and an ESP32 on USB drives the lock, LEDs and buzzer.

    python door_pc.py enrol Thinker   --cam http://192.168.4.1           # 10 samples, look at the camera
    python door_pc.py run              --cam http://192.168.4.1 --io COM5 # the door
    python door_pc.py list
    python door_pc.py delete Thinker

--cam  ESP32-CAM address (the /stream URL is added), or a webcam number such as 0
--io   COM port of the ESP32 DevKit running door_io.ino (leave out to run without the lock)
--liveness 0.5   turn the photo/screen check on at that threshold (off by default: it was trained on the
                 OV7725 camera and needs retraining for a new camera)
Keys in the window: q = quit, o = open the door, s = save a snapshot.
Everything is logged to data/access_log.csv; unknown faces are saved in data/unknown/.
"""
import argparse
import csv
import json
import sys
import threading
import time
import urllib.request
from pathlib import Path

import cv2
import numpy as np

from engine import Engine

DATA = Path(__file__).resolve().parent / "data"
MIN_FACE_PX, ENROL_MIN_PX = 48, 60
AVG_FRAMES, OPEN_MS, DENY_HOLD, SPOOF_FRAMES = 3, 5000, 2.0, 3
UNKNOWN_ALERTS, ALERT_WINDOW = 3, 60.0
GREEN, RED, YELLOW, WHITE, CYAN = (80, 200, 80), (60, 60, 230), (40, 200, 230), (235, 235, 235), (230, 200, 60)


# ------------------------------------------------------------------ camera
class Camera:
    """Latest frame from an MJPEG stream (ESP32-CAM) or a webcam, read on a background thread."""
    def __init__(self, src, flip=False):
        self.src, self.flip, self.frame, self.t, self.ok = src, flip, None, 0.0, True
        threading.Thread(target=self._webcam if src.isdigit() else self._mjpeg, daemon=True).start()

    def _store(self, img):
        if img is None:
            return
        if self.flip:
            img = cv2.rotate(img, cv2.ROTATE_180)
        h, w = img.shape[:2]
        if abs(w / h - 4 / 3) > 0.02:                     # crop to 4:3, then 320 x 240 like the board
            nw = int(h * 4 / 3)
            if nw <= w:
                img = img[:, (w - nw) // 2:(w - nw) // 2 + nw]
            else:
                nh = int(w * 3 / 4); img = img[(h - nh) // 2:(h - nh) // 2 + nh]
        self.frame = cv2.resize(img, (320, 240), interpolation=cv2.INTER_AREA)
        self.t = time.time()

    def _webcam(self):
        cap = cv2.VideoCapture(int(self.src))
        while True:
            ok, img = cap.read()
            if ok:
                self._store(img)
            else:
                time.sleep(0.05)

    def _mjpeg(self):
        url = self.src.rstrip("/")
        if not url.startswith("http"):
            url = "http://" + url
        if not url.endswith("/stream"):
            url += "/stream"
        while True:
            try:
                buf = b""
                with urllib.request.urlopen(url, timeout=5) as r:
                    while True:
                        chunk = r.read(4096)
                        if not chunk:
                            break
                        buf += chunk
                        a = buf.find(b"\xff\xd8")
                        b = buf.find(b"\xff\xd9", a + 2)
                        if a >= 0 and b >= 0:
                            jpg, buf = buf[a:b + 2], buf[b + 2:]
                            self._store(cv2.imdecode(np.frombuffer(jpg, np.uint8), cv2.IMREAD_COLOR))
                        if len(buf) > 1_000_000:
                            buf = b""
            except Exception as e:
                print(f"camera: {e}; retrying ({url})", flush=True)
                time.sleep(1)


# ------------------------------------------------------------------ lock / LEDs / buzzer
class DoorIO:
    def __init__(self, port):
        self.ser = None
        self.events = []
        if not port:
            print("No --io port: running without the lock (decisions are shown and logged only).")
            return
        import serial
        self.ser = serial.Serial(port, 115200, timeout=0)
        time.sleep(1.5)                                    # the ESP32 resets when the port opens
        self.buf = b""

    def send(self, cmd):
        if self.ser:
            self.ser.write((cmd + "\n").encode())

    def poll(self):
        if not self.ser:
            return []
        self.buf += self.ser.read(1024)
        out = []
        while b"\n" in self.buf:
            ln, self.buf = self.buf.split(b"\n", 1)
            ln = ln.decode(errors="replace").strip()
            if ln.startswith("EVT"):
                out.append(ln[4:])
        return out


# ------------------------------------------------------------------ enrolled people
def load_people():
    p = DATA / "people.json"
    if not p.exists():
        return {}
    return {k: np.array(v, np.float32) for k, v in json.load(open(p)).items()}


def save_people(people):
    DATA.mkdir(exist_ok=True)
    json.dump({k: [round(float(x), 6) for x in v] for k, v in people.items()}, open(DATA / "people.json", "w"))


def log(event, name="", score=""):
    DATA.mkdir(exist_ok=True)
    f = DATA / "access_log.csv"
    new = not f.exists()
    with open(f, "a", newline="") as fh:
        w = csv.writer(fh)
        if new:
            w.writerow(["time", "event", "name", "score"])
        w.writerow([time.strftime("%Y-%m-%d %H:%M:%S"), event, name, score])
    print(time.strftime("%H:%M:%S"), event, name, score, flush=True)


# ------------------------------------------------------------------ drawing
def draw(frame, face, color, msg, info):
    big = cv2.resize(frame, (640, 480), interpolation=cv2.INTER_LINEAR)
    if face is not None:
        x1, y1, x2, y2 = (face[1] * 2).astype(int)
        cv2.rectangle(big, (x1, y1), (x2, y2), color, 2)
    cv2.rectangle(big, (0, 440), (640, 480), (25, 25, 25), -1)
    cv2.putText(big, msg, (12, 468), cv2.FONT_HERSHEY_SIMPLEX, 0.8, color, 2, cv2.LINE_AA)
    cv2.putText(big, info, (12, 24), cv2.FONT_HERSHEY_SIMPLEX, 0.55, WHITE, 1, cv2.LINE_AA)
    return big


def largest(faces):
    return max(faces, key=lambda c: (c[1][2] - c[1][0]) * (c[1][3] - c[1][1])) if faces else None


# ------------------------------------------------------------------ modes
def wait_camera(cam):
    t0 = time.time()
    while cam.frame is None:
        if time.time() - t0 > 20:
            sys.exit(f"No picture from {cam.src}. Is the laptop on the camera's WiFi, and is the address right?")
        time.sleep(0.1)


def enrol(args, eng, cam):
    people = load_people()
    wait_camera(cam)
    samples, last, t0 = [], 0.0, time.time()
    print(f"Enrolling {args.name}: look at the camera from about 50 cm and keep still ({args.samples} samples).")
    while len(samples) < args.samples:
        frame = cam.frame.copy(); rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        f = largest(eng.detect(rgb))
        msg, color = "Look at the camera", WHITE
        if f is not None and f[1][2] - f[1][0] < ENROL_MIN_PX:
            msg, color = f"Come closer ({int(f[1][2] - f[1][0])} px, need {ENROL_MIN_PX})", YELLOW
        elif f is not None and time.time() - last > 0.3:
            e = eng.embed(eng.align112(rgb, f[2]))
            if len(samples) >= 3:
                m = np.mean(samples, 0); c = float(e @ m / np.linalg.norm(m))
                if c < 0.3:
                    msg, color = f"Hold still (frame unlike the others: {c:.2f})", YELLOW
                    e = None
            if e is not None:
                samples.append(e); last = time.time()
        if f is not None and color == WHITE:
            msg, color = f"Enrolling {args.name}: {len(samples)}/{args.samples}", CYAN
        cv2.imshow("Door - enrolment", draw(frame, f, color, msg, "q = cancel"))
        if cv2.waitKey(1) & 0xFF == ord("q") or time.time() - t0 > 90:
            sys.exit("Enrolment cancelled")
    m = np.mean(samples, 0); people[args.name] = m / np.linalg.norm(m)
    save_people(people); log("ENROLLED", args.name)
    print(f"OK enrolled {args.name} ({len(people)} people)")


def run(args, eng, cam, io):
    people = load_people()
    if not people:
        print("Nobody is enrolled yet: python door_pc.py enrol <name> --cam ...")
    names = list(people); T = np.stack([people[n] for n in names]) if names else np.zeros((0, 512), np.float32)
    wait_camera(cam)
    emb_sum, n_sum, spoof_run, last_box, last_t = np.zeros(512, np.float32), 0, 0, None, 0.0
    hold_until, open_until, unknown_t = 0.0, 0.0, []
    msg, color, fps_t, fps_n, fps = "Ready", WHITE, time.time(), 0, 0.0
    (DATA / "unknown").mkdir(parents=True, exist_ok=True)
    while True:
        for ev in io.poll():
            log(ev.replace(" ", "_"))
            if ev == "EXIT":
                open_until = time.time() + OPEN_MS / 1000; msg, color = "Exit - door open", GREEN
        frame = cam.frame.copy(); rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB); now = time.time()
        f = largest(eng.detect(rgb))
        busy = now < hold_until or now < open_until
        if f is not None:
            cx, cy, w = (f[1][0] + f[1][2]) / 2, (f[1][1] + f[1][3]) / 2, f[1][2] - f[1][0]
            if last_box is None or now - last_t > 1 or abs(cx - last_box[0]) > 0.6 * last_box[2] or abs(cy - last_box[1]) > 0.6 * last_box[2]:
                emb_sum[:], n_sum, spoof_run = 0, 0, 0
            last_box, last_t = (cx, cy, w), now
        box_color = WHITE
        if busy:
            pass
        elif f is None:
            msg, color = ("Look at the camera" if names else "No one enrolled"), WHITE
            if now - last_t > 1:
                emb_sum[:], n_sum = 0, 0
        elif f[1][2] - f[1][0] < MIN_FACE_PX:
            msg, color = "Come closer", YELLOW; emb_sum[:], n_sum = 0, 0
        else:
            p_spoof = eng.spoof_prob(eng.loose64(rgb, f[1])) if args.liveness > 0 else 0.0
            if args.liveness > 0 and p_spoof >= args.liveness:
                box_color = RED; emb_sum[:], n_sum = 0, 0; spoof_run += 1
                msg, color = "Checking...", YELLOW
                if spoof_run >= SPOOF_FRAMES:
                    msg, color = "SPOOF - photo/screen", RED
                    io.send("ALARM 3000"); log("SPOOF", "", f"{p_spoof:.2f}")
                    cv2.imwrite(str(DATA / "unknown" / time.strftime("spoof_%Y%m%d_%H%M%S.jpg")), frame)
                    hold_until, spoof_run = now + 3, 0
            elif not names:
                msg, color = "Live face - enrol someone", YELLOW
            else:
                spoof_run = 0
                emb_sum += eng.embed(eng.align112(rgb, f[2])); n_sum += 1
                msg, color = "Hold still...", WHITE
                if n_sum >= AVG_FRAMES:
                    avg = emb_sum / np.linalg.norm(emb_sum); s = T @ avg; k = int(np.argmax(s)); score = float(s[k])
                    emb_sum[:], n_sum = 0, 0
                    if score >= args.thresh:
                        msg, color = f"Welcome {names[k]}", GREEN
                        io.send(f"OPEN {OPEN_MS}"); io.send("BEEP 120"); log("GRANT", names[k], f"{score:.3f}")
                        open_until, unknown_t = now + OPEN_MS / 1000, []
                    else:
                        msg, color = "Access denied", RED
                        io.send("LED R"); io.send("BEEP 400"); log("UNKNOWN", f"closest={names[k]}", f"{score:.3f}")
                        cv2.imwrite(str(DATA / "unknown" / time.strftime("unknown_%Y%m%d_%H%M%S.jpg")), frame)
                        hold_until = now + DENY_HOLD
                        unknown_t = [t for t in unknown_t if now - t < ALERT_WINDOW] + [now]
                        if len(unknown_t) >= UNKNOWN_ALERTS:
                            io.send("ALARM 3000"); log("ALERT", f"{len(unknown_t)} unknown faces in 1 min"); unknown_t = []
        if box_color == WHITE and color in (GREEN, RED, YELLOW):
            box_color = color
        fps_n += 1
        if now - fps_t >= 1:
            fps, fps_n, fps_t = fps_n / (now - fps_t), 0, now
        info = f"{fps:.1f} fps | {len(names)} enrolled | thresh {args.thresh:.2f} | liveness {'off' if args.liveness <= 0 else args.liveness}"
        cv2.imshow("Door", draw(frame, f, box_color, msg, info))
        k = cv2.waitKey(1) & 0xFF
        if k == ord("q"):
            break
        if k == ord("o"):
            io.send(f"OPEN {OPEN_MS}"); log("REMOTE_OPEN"); open_until = now + OPEN_MS / 1000; msg, color = "Opened from the PC", GREEN
        if k == ord("s"):
            p = DATA / time.strftime("snap_%Y%m%d_%H%M%S.png"); cv2.imwrite(str(p), frame); print("saved", p)


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("mode", choices=["run", "enrol", "list", "delete"])
    ap.add_argument("name", nargs="?")
    ap.add_argument("--cam", default="http://192.168.4.1")
    ap.add_argument("--io")
    ap.add_argument("--thresh", type=float, default=0.42)
    ap.add_argument("--liveness", type=float, default=0.0)
    ap.add_argument("--samples", type=int, default=10)
    ap.add_argument("--flip", action="store_true", help="camera mounted upside down")
    args = ap.parse_args()
    if args.mode == "list":
        p = load_people(); print(f"{len(p)} enrolled:", ", ".join(p) or "-"); return
    if args.mode == "delete":
        p = load_people(); p.pop(args.name, None); save_people(p); print("OK deleted", args.name); return
    if args.mode == "enrol" and not args.name:
        sys.exit("usage: python door_pc.py enrol <name> --cam ...")
    eng = Engine()
    cam = Camera(args.cam, args.flip)
    if args.mode == "enrol":
        enrol(args, eng, cam)
    else:
        run(args, eng, cam, DoorIO(args.io))


if __name__ == "__main__":
    main()
