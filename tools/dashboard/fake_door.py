"""Pretend to be the door's ESP32, to try the dashboard without the hardware.

    python fake_door.py --key <door key> [--server http://localhost:5000] [--face some_face.png]

Sends a few log events and one unknown face, then polls for the admin's decisions and prints the
command lines the real door would receive (ADMIT, ADDB, WIN, DELETE, SCHED, OPEN)."""
import argparse, base64, json, time, urllib.request
import numpy as np

ap = argparse.ArgumentParser()
ap.add_argument("--server", default="http://localhost:5000")
ap.add_argument("--key", required=True)
ap.add_argument("--face", help="photo of a face (any size); a grey test pattern if left out")
ap.add_argument("--polls", type=int, default=0, help="stop after this many polls (0 = run until Ctrl+C)")
a = ap.parse_args()

def call(path, body=None):
    req = urllib.request.Request(a.server + path, data=None if body is None else json.dumps(body).encode(),
                                 headers={"X-Door-Key": a.key, "Content-Type": "application/json"})
    return json.loads(urllib.request.urlopen(req, timeout=5).read())

now = call("/api/poll")["time"]
start = call("/api/poll")["log_seq"] + 1
events = [("BOOT", "", 0), ("GRANT", "Thinker", 0.71), ("UNKNOWN", "closest=Thinker", 0.18), ("EXIT", "", 0)]
for i, (t, n, s) in enumerate(events):
    call("/api/log", {"seq": start + i, "time": now - 60 + 10 * i, "type": t, "name": n, "score": s})
if a.face:
    import cv2
    img = cv2.cvtColor(cv2.resize(cv2.imread(a.face), (112, 112)), cv2.COLOR_BGR2RGB)
else:
    img = np.tile(np.linspace(60, 200, 112).astype(np.uint8)[None, :, None], (112, 1, 3))
crop = np.clip(img.astype(int) - 129, -128, 127).astype(np.int8).tobytes()
emb = np.random.default_rng(1).integers(-127, 128, 512).astype(np.int8).tobytes()
call("/api/face", {"id": start + 2, "time": now - 40, "score": 0.18, "closest": "Thinker", "scale": 0.0012,
                   "crop": base64.b64encode(crop).decode(), "emb": base64.b64encode(emb).decode()})
call("/api/people", {"people": [{"name": "Thinker", "start": 0, "end": 1440, "days": 127}],
                     "sched": {"on": 0, "start": 420, "end": 1140, "days": 31}})
print("Sent 4 events, 1 unknown face and the people list. Polling for commands...")
n = 0
while not a.polls or n < a.polls:
    r = call("/api/poll"); n += 1
    for c in r["commands"]:
        print("door would receive:", c[:90] + ("..." if len(c) > 90 else ""))
    time.sleep(1)
