"""A door on the PC, to use the dashboard without the hardware.

It runs the door's own face models (tools/door_pc/engine.py: SCRFD detector, MobileFaceNet, liveness) on
photos, with the same rules as the STM32 firmware:

  * the face matches an enrolled person (cosine 0.42 or more) inside their access hours -> the door opens
    by itself, no administrator needed (GRANT in the log)
  * a match outside their hours -> denied (HOURS)
  * no match -> denied, and the face goes to the dashboard's Pending Review page (UNKNOWN). "Let in once"
    opens the door; "Add permanently" enrols that face, so the same person is let in automatically next time.

    python fake_door.py --key <door key>                               # then type commands, see below
    python fake_door.py --key <door key> --face photo.jpg              # someone with this face at the door
    python fake_door.py --key <door key> --enrol Thinker --face a.jpg --face b.jpg   # enrol from photos

Commands while it runs:
    photo.jpg                    someone with this face comes to the door
    enrol <name> a.jpg [b.jpg]   enrol a person from one or more photos (as "door.py enrol" does over USB)
    list                         enrolled people and their hours
    delete <name>                remove a person
    exit                         the exit button
    quit

The enrolled people are kept in fake_door_people.json next to this file (the real door keeps them in its
flash). Needs: pip install onnxruntime opencv-python numpy
"""
import argparse, base64, json, sys, threading, time, urllib.error, urllib.request
from pathlib import Path

import cv2
import numpy as np

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE.parent / "door_pc"))
from engine import Engine  # noqa: E402

THRESH, MIN_FACE_PX, ADMIT_VALID_S = 0.42, 48, 150
STORE = HERE / "fake_door_people.json"
DAYS = ["Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"]

ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
ap.add_argument("--key", required=True, help="the door key shown by the dashboard (Settings page)")
ap.add_argument("--server", default="http://localhost:5000")
ap.add_argument("--face", action="append", default=[], help="photo of a face; may be given more than once")
ap.add_argument("--enrol", metavar="NAME", help="enrol NAME from the --face photos instead of presenting them")
ap.add_argument("--liveness", type=float, default=0, metavar="P",
                help="deny as a spoof when the liveness model says spoof >= P (off by default: the model was "
                     "trained on the OV7725 camera, not on photo files)")
ap.add_argument("--polls", type=int, default=0, help="stop after this many polls (0 = keep running)")
a = ap.parse_args()

lock = threading.Lock()
state = json.loads(STORE.read_text()) if STORE.exists() else {"people": [], "sched": [0, 420, 1140, 31]}
pending = state.setdefault("pending", {})   # face id -> time sent, for "Let in once"
clock = {"t": None}     # dashboard's local time (unix seconds in local time), as the ESP32 forwards it
seq = {"n": 0}
eng = Engine()


# ------------------------------------------------------------------ dashboard
def call(path, body=None):
    req = urllib.request.Request(a.server + path, data=None if body is None else json.dumps(body).encode(),
                                 headers={"X-Door-Key": a.key, "Content-Type": "application/json"})
    try:
        return json.loads(urllib.request.urlopen(req, timeout=8).read())
    except urllib.error.HTTPError as e:
        sys.exit(f"dashboard refused {path}: HTTP {e.code}" + (" (wrong --key?)" if e.code == 403 else ""))


def now():
    return clock["t"] if clock["t"] is not None else int(time.time())


def log(kind, name="", score=0.0):
    with lock:
        seq["n"] += 1
        n = seq["n"]
    call("/api/log", {"seq": n, "time": now(), "type": kind, "name": name, "score": round(float(score), 3)})
    return n


def save_and_report():
    STORE.write_text(json.dumps(state))
    on, s, e, d = state["sched"]
    call("/api/people", {"people": [{"name": p["name"], "start": p["start"], "end": p["end"], "days": p["days"]}
                                    for p in state["people"]],
                         "sched": {"on": on, "start": s, "end": e, "days": d}})


# ------------------------------------------------------------------ time windows (same rules as admin.c)
def in_window(start, end, days):
    t = now()
    m, d = (t % 86400) // 60, (t // 86400 + 3) % 7          # weekday: Monday = 0
    if start <= end:
        return bool((days >> d) & 1) and start <= m <= end
    if m >= start:
        return bool((days >> d) & 1)
    return bool((days >> ((d + 6) % 7)) & 1) and m <= end


def hm(m):
    return f"{m // 60:02d}:{m % 60:02d}"


def days_txt(d):
    return "every day" if d == 127 else ",".join(x for i, x in enumerate(DAYS) if d >> i & 1) or "no days"


# ------------------------------------------------------------------ the face pipeline
def frame_from(path):
    img = cv2.imread(str(path))
    if img is None:
        print(f"  cannot read {path}")
        return None
    h, w = img.shape[:2]                                   # crop to 4:3 and scale to 320 x 240, like the camera
    if w / h > 4 / 3:
        nw = int(h * 4 / 3); img = img[:, (w - nw) // 2:(w - nw) // 2 + nw]
    else:
        nh = int(w * 3 / 4); img = img[(h - nh) // 2:(h - nh) // 2 + nh]
    return cv2.cvtColor(cv2.resize(img, (320, 240), interpolation=cv2.INTER_AREA), cv2.COLOR_BGR2RGB)


def face_of(path):
    """(unit embedding, aligned 112x112 crop, spoof probability) of the largest face, or None."""
    rgb = frame_from(path)
    if rgb is None:
        return None
    faces = eng.detect(rgb)
    if not faces:
        print(f"  no face found in {Path(path).name}")
        return None
    _, box, kps = max(faces, key=lambda f: (f[1][2] - f[1][0]) * (f[1][3] - f[1][1]))
    if box[2] - box[0] < MIN_FACE_PX:
        print(f"  the face in {Path(path).name} is too small ({box[2] - box[0]:.0f} px; the door needs {MIN_FACE_PX})")
        return None
    crop = eng.align112(rgb, kps)
    return eng.embed(crop), crop, eng.spoof_prob(eng.loose64(rgb, box))


def emb_of(p):
    q = np.frombuffer(base64.b64decode(p["q"]), np.int8).astype(np.float32)
    return q / (np.linalg.norm(q) + 1e-9)


def quantise(e):
    m = float(np.abs(e).max()) + 1e-9
    return m / 127.0, np.rint(e * 127.0 / m).astype(np.int8)


def best_match(e):
    best, who = -1.0, None
    for p in state["people"]:
        s = float(emb_of(p) @ e)
        if s > best:
            best, who = s, p
    return best, who


def present(path):
    on, s, e, d = state["sched"]
    if on and not in_window(s, e, d):
        print("  door is CLOSED (opening hours set on the dashboard); the face is not checked")
        return
    r = face_of(path)
    if r is None:
        return
    emb, crop, spoof = r
    if a.liveness and spoof >= a.liveness:
        n = log("SPOOF", "", spoof)
        print(f"  SPOOF (p={spoof:.2f}): denied, alarm sounds  [log #{n}]")
        return
    score, who = best_match(emb)
    if who and score >= THRESH:
        if not in_window(who["start"], who["end"], who["days"]):
            n = log("HOURS", who["name"], score)
            print(f"  {who['name']} ({score:.2f}) is outside their hours "
                  f"({hm(who['start'])}-{hm(who['end'])}, {days_txt(who['days'])}): denied  [log #{n}]")
        else:
            n = log("GRANT", who["name"], score)
            print(f"  >>> DOOR OPENS: welcome {who['name']} (similarity {score:.2f}); no administrator needed  [log #{n}]")
        return
    n = log("UNKNOWN", "", max(score, 0))
    scale, q = quantise(emb)
    crop_q = np.clip(np.round(crop.astype(np.float32) - 127.5) - 1, -128, 127).astype(np.int8)
    call("/api/face", {"id": n, "time": now(), "score": round(max(score, 0), 3), "closest": who["name"] if who else "",
                       "scale": scale, "crop": base64.b64encode(crop_q.tobytes()).decode(),
                       "emb": base64.b64encode(q.tobytes()).decode()})
    pending[str(n)] = time.time(); STORE.write_text(json.dumps(state))
    closest = f"; closest is {who['name']} at {score:.2f}" if who else ""
    print(f"  UNKNOWN face: denied{closest}. Sent to the dashboard's Pending Review page  [face #{n}]")


def enrol(name, photos):
    embs = [r[0] for r in (face_of(p) for p in photos) if r is not None]
    if not embs:
        print("  not enrolled: no usable face in those photos")
        return
    e = np.mean(embs, 0); e /= np.linalg.norm(e)
    scale, q = quantise(e)
    with lock:
        old = next((p for p in state["people"] if p["name"] == name), None)
        rec = {"name": name, "start": old["start"] if old else 0, "end": old["end"] if old else 1439,
               "days": old["days"] if old else 127, "scale": scale, "q": base64.b64encode(q.tobytes()).decode()}
        state["people"] = [p for p in state["people"] if p["name"] != name] + [rec]
    log("ENROL", name)
    save_and_report()
    print(f"  enrolled {name} from {len(embs)} photo(s)")


# ------------------------------------------------------------------ commands from the administrator
def handle(line):
    w = line.split()
    cmd = w[0] if w else ""
    if cmd == "ADMIT":
        fid = int(w[1])
        if time.time() - pending.get(str(fid), 0) > ADMIT_VALID_S:
            print(f"\n  admin: let in face #{fid}, but the visitor has left (offer expired)")
        else:
            log("ADMIT", f"visitor#{fid}")
            print(f"\n  >>> DOOR OPENS: the administrator let in visitor #{fid}")
    elif cmd == "ADDB":                                  # ADDB <name> <start> <end> <days> <scale> <base64 int8 x512>
        name, s, e, d = w[1], int(w[2]), int(w[3]), int(w[4])
        with lock:
            state["people"] = [p for p in state["people"] if p["name"] != name] + \
                              [{"name": name, "start": s, "end": e, "days": d, "scale": float(w[5]), "q": w[6]}]
        log("ENROL", name)
        save_and_report()
        print(f"\n  admin: {name} enrolled from the reviewed face ({hm(s)}-{hm(e)}, {days_txt(d)}); "
              f"the same face is now let in automatically")
    elif cmd == "WIN":
        name, s, e, d = w[1], int(w[2]), int(w[3]), int(w[4])
        for p in state["people"]:
            if p["name"] == name:
                p.update(start=s, end=e, days=d)
        save_and_report()
        print(f"\n  admin: {name}'s hours are now {hm(s)}-{hm(e)}, {days_txt(d)}")
    elif cmd == "DELETE":
        state["people"] = [p for p in state["people"] if p["name"] != w[1]]
        log("DELETE", w[1]); save_and_report()
        print(f"\n  admin: {w[1]} removed")
    elif cmd == "SCHED":
        state["sched"] = [int(x) for x in w[1:5]]; save_and_report()
        on, s, e, d = state["sched"]
        print(f"\n  admin: opening hours {'off' if not on else f'{hm(s)}-{hm(e)}, {days_txt(d)}'}")
    elif cmd == "OPEN":
        log("OPEN")
        print("\n  >>> DOOR OPENS: opened remotely by the administrator")
    elif cmd == "PEOPLE":
        save_and_report()
    elif cmd:
        print(f"\n  door received: {line[:80]}")


def poll_once():
    r = call("/api/poll")
    clock["t"] = r["time"]
    with lock:
        seq["n"] = max(seq["n"], r["log_seq"])
    for c in r["commands"]:
        handle(c)


def poller():
    n = 0
    while not a.polls or n < a.polls:
        try:
            poll_once()
        except SystemExit as e:
            print(e); return
        except Exception as e:                              # dashboard restarting etc.
            print(f"\n  (dashboard not reachable: {e})")
        n += 1
        time.sleep(1)


# ------------------------------------------------------------------ main
poll_once()
log("BOOT")
save_and_report()
print(f"Door running against {a.server}: {len(state['people'])} enrolled "
      f"({', '.join(p['name'] for p in state['people']) or 'nobody yet'}).")
if a.enrol:
    enrol(a.enrol, a.face)
else:
    for f in a.face:
        print(f"{Path(f).name}:"); present(f)
t = threading.Thread(target=poller, daemon=True); t.start()
if not sys.stdin.isatty():
    t.join(); sys.exit()
print("Type a photo path to put that face at the door; also: enrol <name> <photos>, list, delete <name>, exit, quit")
while True:
    try:
        line = input("door> ").strip().strip('"')
    except (EOFError, KeyboardInterrupt):
        break
    if not line:
        continue
    w = line.split()
    if w[0] == "quit":
        break
    elif w[0] == "list":
        for p in state["people"]:
            print(f"  {p['name']}: {hm(p['start'])}-{hm(p['end'])}, {days_txt(p['days'])}")
        print("  (nobody enrolled)" if not state["people"] else "", end="")
    elif w[0] == "enrol" and len(w) >= 3:
        enrol(w[1], w[2:])
    elif w[0] == "delete" and len(w) == 2:
        handle(f"DELETE {w[1]}")
    elif w[0] == "exit":
        log("EXIT"); print("  >>> DOOR OPENS: exit button")
    else:
        present(line)
