"""Administrator dashboard for the face-recognition door (Flask + SQLite).

The door (STM32H723) makes every access decision on its own. This dashboard only handles the exceptions:
  - unknown faces sent by the door: let the visitor in once, add them permanently, or reject;
  - enrolled people and their access hours; the door-wide opening schedule;
  - a mirror of the door's local access log.

The ESP32 on the door talks to this server (all requests carry the header X-Door-Key):
  POST /api/log     {"seq", "time", "type", "name", "score"}            one access-log record
  POST /api/face    {"id", "time", "score", "closest", "scale", "crop", "emb"}  unknown face (base64 fields)
  POST /api/people  {"people": [{"name","start","end","days"}], "sched": {"on","start","end","days"}}
  GET  /api/poll    -> {"commands": [...lines for the STM32...], "log_seq": n, "time": local unix seconds}
  GET  /api/poll.txt   the same as text lines: "TIME t", "LOGSEQ n", then the commands (what the ESP32 uses)
  /api/log also takes {"records": [ ... ]} (a batch)

Run:  python app.py            (first run creates dashboard.db, admin / admin; change it in Settings)
Then open http://localhost:5000 and put this laptop's address and the door key in the ESP32 sketch.
"""
import base64
import calendar
import datetime as dt
import os
import secrets
import sqlite3
import struct
import time
import zlib
from functools import wraps
from pathlib import Path

from flask import (Flask, Response, abort, flash, g, jsonify, redirect, render_template, request,
                   session, url_for)
from werkzeug.security import check_password_hash, generate_password_hash

HERE = Path(__file__).resolve().parent
DB_PATH = Path(os.environ.get("DOOR_DB", HERE / "dashboard.db"))
ONCE_VALID_S = 120            # "let in once" is offered for this long after the face was seen
ONLINE_S = 10                 # door shown online if the ESP32 polled within this many seconds
DAYS = ["Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"]
EVENT_NAMES = {
    "GRANT": "Access granted", "UNKNOWN": "Unknown face", "SPOOF": "Spoof attempt", "HOURS": "Outside access hours",
    "CLOSED": "Door closed (schedule)", "EXIT": "Exit button", "ADMIT": "Let in by admin", "ENROL": "Enrolled",
    "DELETE": "Removed", "OPEN": "Opened remotely", "MOTION": "Motion", "FORCED": "Door forced", "BOOT": "Door started",
}

app = Flask(__name__)


# ------------------------------------------------------------------ database
SCHEMA = """
CREATE TABLE IF NOT EXISTS admin (username TEXT PRIMARY KEY, pw_hash TEXT NOT NULL);
CREATE TABLE IF NOT EXISTS settings (key TEXT PRIMARY KEY, value TEXT);
CREATE TABLE IF NOT EXISTS log (seq INTEGER PRIMARY KEY, time INTEGER, type TEXT, name TEXT, score REAL, received INTEGER);
CREATE TABLE IF NOT EXISTS faces (id INTEGER PRIMARY KEY, time INTEGER, score REAL, closest TEXT, png BLOB,
    scale REAL, emb BLOB, status TEXT DEFAULT 'pending', decided_at INTEGER, decided_name TEXT, received INTEGER);
CREATE TABLE IF NOT EXISTS commands (id INTEGER PRIMARY KEY AUTOINCREMENT, line TEXT, created INTEGER, delivered INTEGER);
CREATE TABLE IF NOT EXISTS people (name TEXT PRIMARY KEY, start INTEGER, end INTEGER, days INTEGER);
"""


def db():
    if "db" not in g:
        g.db = sqlite3.connect(DB_PATH)
        g.db.row_factory = sqlite3.Row
    return g.db


@app.teardown_appcontext
def close_db(_):
    d = g.pop("db", None)
    if d is not None:
        d.close()


def setting(key, default=None):
    r = db().execute("SELECT value FROM settings WHERE key=?", (key,)).fetchone()
    return r["value"] if r else default


def set_setting(key, value):
    db().execute("INSERT INTO settings(key,value) VALUES(?,?) ON CONFLICT(key) DO UPDATE SET value=excluded.value", (key, str(value)))
    db().commit()


def init_db():
    with app.app_context():
        db().executescript(SCHEMA)
        if not db().execute("SELECT 1 FROM admin").fetchone():
            db().execute("INSERT INTO admin VALUES(?,?)", ("admin", generate_password_hash("admin")))
        if setting("door_key") is None:
            set_setting("door_key", secrets.token_hex(8))
        if setting("secret") is None:
            set_setting("secret", secrets.token_hex(32))
        db().commit()
        app.secret_key = setting("secret")


def now_local():
    """This laptop's local time as 'local unix seconds' (the door shows and checks local time)."""
    t = time.time()
    return int(t + (dt.datetime.fromtimestamp(t) - dt.datetime.utcfromtimestamp(t)).total_seconds())


def fmt_time(t):
    if not t:
        return "time not set"
    return dt.datetime.utcfromtimestamp(t).strftime("%a %d %b %Y, %H:%M:%S")


def fmt_hm(m):
    return f"{m // 60:02d}:{m % 60:02d}"


def parse_hm(s):
    h, m = s.strip().split(":")
    v = int(h) * 60 + int(m)
    if not 0 <= v <= 1440:
        raise ValueError(s)
    return v


def days_mask(form):
    return sum(1 << i for i, d in enumerate(DAYS) if form.get(f"day_{d}"))


def days_text(mask):
    if mask == 0x7F:
        return "Every day"
    if mask == 0x1F:
        return "Mon–Fri"
    return ", ".join(d for i, d in enumerate(DAYS) if mask >> i & 1) or "No days"


def queue_command(line):
    db().execute("INSERT INTO commands(line, created) VALUES(?,?)", (line, int(time.time())))
    db().commit()


app.jinja_env.filters["bit"] = lambda mask, i: (int(mask) >> int(i)) & 1
app.jinja_env.globals.update(fmt_time=fmt_time, fmt_hm=fmt_hm, days_text=days_text, DAYS=DAYS, EVENT_NAMES=EVENT_NAMES)


# ------------------------------------------------------------------ face image: door crop -> PNG
def crop_to_png(crop, size=112):
    """The door sends the aligned face as the model's int8 input (q = x - 128.5); turn it back into an RGB PNG."""
    px = bytes(max(0, min(255, (b - 256 if b > 127 else b) + 129)) for b in crop)
    rows = b"".join(b"\x00" + px[y * size * 3:(y + 1) * size * 3] for y in range(size))
    chunk = lambda tag, data: struct.pack(">I", len(data)) + tag + data + struct.pack(">I", zlib.crc32(tag + data) & 0xFFFFFFFF)
    return (b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", size, size, 8, 2, 0, 0, 0))
            + chunk(b"IDAT", zlib.compress(rows, 9)) + chunk(b"IEND", b""))


# ------------------------------------------------------------------ door API (ESP32)
def door_api(f):
    @wraps(f)
    def w(*a, **k):
        if request.headers.get("X-Door-Key") != setting("door_key"):
            abort(403)
        set_setting("last_seen", int(time.time()))
        return f(*a, **k)
    return w


@app.post("/api/log")
@door_api
def api_log():
    j = request.get_json(force=True)
    for r in j.get("records", [j]):
        name = r.get("name", "")
        db().execute("INSERT OR IGNORE INTO log VALUES(?,?,?,?,?,?)",
                     (int(r["seq"]), int(r.get("time", 0)), r["type"], "" if name == "-" else name,
                      float(r.get("score", 0)), int(time.time())))
    db().commit()
    return jsonify(ok=True)


@app.post("/api/face")
@door_api
def api_face():
    j = request.get_json(force=True)
    crop = base64.b64decode(j["crop"])
    if len(crop) != 112 * 112 * 3:
        abort(400)
    db().execute("INSERT OR IGNORE INTO faces(id,time,score,closest,png,scale,emb,received) VALUES(?,?,?,?,?,?,?,?)",
                 (int(j["id"]), int(j.get("time", 0)), float(j.get("score", 0)), j.get("closest", ""),
                  crop_to_png(crop), float(j["scale"]), base64.b64decode(j["emb"]), int(time.time())))
    db().commit()
    return jsonify(ok=True)


@app.post("/api/people")
@door_api
def api_people():
    j = request.get_json(force=True)
    db().execute("DELETE FROM people")
    for p in j.get("people", []):
        db().execute("INSERT OR REPLACE INTO people VALUES(?,?,?,?)", (p["name"], int(p["start"]), int(p["end"]), int(p["days"])))
    s = j.get("sched")
    if s:
        set_setting("sched", f"{int(s['on'])} {int(s['start'])} {int(s['end'])} {int(s['days'])}")
    db().commit()
    return jsonify(ok=True)


def take_commands():
    rows = db().execute("SELECT id, line FROM commands WHERE delivered IS NULL ORDER BY id").fetchall()
    db().executemany("UPDATE commands SET delivered=? WHERE id=?", [(int(time.time()), r["id"]) for r in rows])
    db().commit()
    return [r["line"] for r in rows]


@app.get("/api/poll.txt")
@door_api
def api_poll_txt():
    """Same as /api/poll as plain text for the ESP32: 'TIME t', 'LOGSEQ n', then one door command per line."""
    seq = db().execute("SELECT COALESCE(MAX(seq), 0) AS s FROM log").fetchone()["s"]
    lines = [f"TIME {now_local()}", f"LOGSEQ {seq}"] + take_commands()
    return Response("\n".join(lines) + "\n", mimetype="text/plain")


@app.get("/api/poll")
@door_api
def api_poll():
    rows = db().execute("SELECT id, line FROM commands WHERE delivered IS NULL ORDER BY id").fetchall()
    db().executemany("UPDATE commands SET delivered=? WHERE id=?", [(int(time.time()), r["id"]) for r in rows])
    db().commit()
    seq = db().execute("SELECT COALESCE(MAX(seq), 0) AS s FROM log").fetchone()["s"]
    return jsonify(commands=[r["line"] for r in rows], log_seq=seq, time=now_local())


# ------------------------------------------------------------------ admin pages
def login_required(f):
    @wraps(f)
    def w(*a, **k):
        if not session.get("user"):
            return redirect(url_for("login", next=request.path))
        return f(*a, **k)
    return w


@app.context_processor
def status():
    if not session.get("user"):
        return {}
    last = int(setting("last_seen", 0) or 0)
    pending = db().execute("SELECT COUNT(*) AS n FROM faces WHERE status='pending'").fetchone()["n"]
    return {"door_online": time.time() - last < ONLINE_S, "door_last_seen": last,
            "door_last_seen_local": last + (now_local() - int(time.time())) if last else 0, "pending_count": pending}


@app.route("/login", methods=["GET", "POST"])
def login():
    if request.method == "POST":
        r = db().execute("SELECT pw_hash FROM admin WHERE username=?", (request.form["username"],)).fetchone()
        if r and check_password_hash(r["pw_hash"], request.form["password"]):
            session["user"] = request.form["username"]
            return redirect(request.args.get("next") or url_for("review"))
        flash("Wrong username or password.")
    return render_template("login.html")


@app.get("/logout")
def logout():
    session.clear()
    return redirect(url_for("login"))


@app.get("/")
@login_required
def review():
    faces = db().execute("SELECT id,time,score,closest,received FROM faces WHERE status='pending' ORDER BY id DESC").fetchall()
    recent = db().execute("SELECT * FROM log ORDER BY seq DESC LIMIT 12").fetchall()
    decided = db().execute("SELECT id,time,status,decided_name,decided_at FROM faces WHERE status!='pending' ORDER BY decided_at DESC LIMIT 6").fetchall()
    return render_template("review.html", faces=faces, recent=recent, decided=decided, now=time.time(), once_s=ONCE_VALID_S)


@app.get("/face/<int:fid>.png")
@login_required
def face_png(fid):
    r = db().execute("SELECT png FROM faces WHERE id=?", (fid,)).fetchone()
    if not r:
        abort(404)
    return Response(r["png"], mimetype="image/png")


def get_face(fid):
    r = db().execute("SELECT * FROM faces WHERE id=?", (fid,)).fetchone()
    if not r:
        abort(404)
    return r


@app.post("/face/<int:fid>/admit")
@login_required
def face_admit(fid):
    f = get_face(fid)
    if time.time() - f["received"] > ONCE_VALID_S:
        flash("Too late to let this visitor in: the offer lasts two minutes. Add them permanently, or reject.")
        return redirect(url_for("review"))
    queue_command(f"ADMIT {fid}")
    db().execute("UPDATE faces SET status='admitted', decided_at=? WHERE id=?", (now_local(), fid))
    db().commit()
    flash("Door opening for this visitor.")
    return redirect(url_for("review"))


@app.post("/face/<int:fid>/enrol")
@login_required
def face_enrol(fid):
    f = get_face(fid)
    name = "".join(c for c in request.form.get("name", "") if c.isalnum() or c in "_-.")[:23]
    try:
        start, end = parse_hm(request.form.get("start", "00:00")), parse_hm(request.form.get("end", "23:59"))
    except ValueError:
        flash("Times must look like 07:30.")
        return redirect(url_for("review"))
    days = days_mask(request.form) or 0x7F
    if not name:
        flash("Type a name (letters, digits, _ - . only).")
        return redirect(url_for("review"))
    queue_command(f"ADDB {name} {start} {end} {days} {f['scale']:.8g} {base64.b64encode(f['emb']).decode()}")
    if time.time() - f["received"] <= ONCE_VALID_S:
        queue_command(f"ADMIT {fid}")
    db().execute("INSERT OR REPLACE INTO people VALUES(?,?,?,?)", (name, start, end, days))
    db().execute("UPDATE faces SET status='enrolled', decided_at=?, decided_name=? WHERE id=?", (now_local(), name, fid))
    db().commit()
    flash(f"{name} added. They will be let in by face from now on ({days_text(days)}, {fmt_hm(start)}–{fmt_hm(end)}).")
    return redirect(url_for("review"))


@app.post("/face/<int:fid>/reject")
@login_required
def face_reject(fid):
    get_face(fid)
    db().execute("UPDATE faces SET status='rejected', decided_at=? WHERE id=?", (now_local(), fid))
    db().commit()
    return redirect(url_for("review"))


@app.get("/people")
@login_required
def people():
    rows = db().execute("SELECT * FROM people ORDER BY name").fetchall()
    return render_template("people.html", people=rows)


@app.post("/people/<name>/window")
@login_required
def people_window(name):
    try:
        start, end = parse_hm(request.form["start"]), parse_hm(request.form["end"])
    except ValueError:
        flash("Times must look like 07:30.")
        return redirect(url_for("people"))
    days = days_mask(request.form)
    queue_command(f"WIN {name} {start} {end} {days}")
    db().execute("UPDATE people SET start=?, end=?, days=? WHERE name=?", (start, end, days, name))
    db().commit()
    flash(f"Access hours for {name} sent to the door.")
    return redirect(url_for("people"))


@app.post("/people/<name>/delete")
@login_required
def people_delete(name):
    queue_command(f"DELETE {name}")
    db().execute("DELETE FROM people WHERE name=?", (name,))
    db().commit()
    flash(f"{name} removed from the door.")
    return redirect(url_for("people"))


@app.get("/log")
@login_required
def log_page():
    q, args = "SELECT * FROM log WHERE 1=1", []
    if request.args.get("type"):
        q += " AND type=?"; args.append(request.args["type"])
    if request.args.get("name"):
        q += " AND name LIKE ?"; args.append(f"%{request.args['name']}%")
    if request.args.get("day"):
        try:
            d0 = calendar.timegm(dt.datetime.strptime(request.args["day"], "%Y-%m-%d").timetuple())
            q += " AND time>=? AND time<?"; args += [d0, d0 + 86400]
        except ValueError:
            pass
    rows = db().execute(q + " ORDER BY seq DESC LIMIT 500", args).fetchall()
    types = [r["type"] for r in db().execute("SELECT DISTINCT type FROM log ORDER BY type")]
    return render_template("log.html", rows=rows, types=types, args=request.args)


@app.get("/log.csv")
@login_required
def log_csv():
    rows = db().execute("SELECT * FROM log ORDER BY seq").fetchall()
    out = "seq,time,event,name,score\n" + "".join(
        f"{r['seq']},{fmt_time(r['time'])},{r['type']},{r['name']},{r['score']:.3f}\n" for r in rows)
    return Response(out, mimetype="text/csv", headers={"Content-Disposition": "attachment; filename=access_log.csv"})


@app.route("/settings", methods=["GET", "POST"])
@login_required
def settings_page():
    if request.method == "POST":
        if request.form.get("form") == "sched":
            try:
                on = 1 if request.form.get("on") else 0
                start, end = parse_hm(request.form["start"]), parse_hm(request.form["end"])
            except ValueError:
                flash("Times must look like 07:30.")
                return redirect(url_for("settings_page"))
            days = days_mask(request.form)
            set_setting("sched", f"{on} {start} {end} {days}")
            queue_command(f"SCHED {on} {start} {end} {days}")
            flash("Door schedule sent to the door.")
        elif request.form.get("form") == "password":
            if len(request.form["new"]) < 6:
                flash("Use at least 6 characters.")
            else:
                db().execute("UPDATE admin SET pw_hash=? WHERE username=?", (generate_password_hash(request.form["new"]), session["user"]))
                db().commit()
                flash("Password changed.")
        elif request.form.get("form") == "open":
            queue_command("OPEN")
            flash("Door opening.")
        return redirect(url_for("settings_page"))
    on, start, end, days = (int(x) for x in setting("sched", "0 420 1140 31").split())
    return render_template("settings.html", sched_on=on, sched_start=start, sched_end=end, sched_days=days,
                           door_key=setting("door_key"))


@app.get("/ui/state")
@login_required
def ui_state():
    seq = db().execute("SELECT COALESCE(MAX(seq),0) AS s FROM log").fetchone()["s"]
    fid = db().execute("SELECT COALESCE(MAX(id),0) AS s FROM faces").fetchone()["s"]
    return jsonify(log=seq, face=fid)


init_db()

if __name__ == "__main__":
    with app.app_context():
        print(f"Dashboard: http://localhost:5000  (first login: admin / admin, then change it in Settings)")
        print(f"Door key for the ESP32 sketch: {setting('door_key')}")
    app.run(host="0.0.0.0", port=int(os.environ.get("PORT", 5000)), debug=False, threaded=True)
