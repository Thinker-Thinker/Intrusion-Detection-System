"""PC tool for the door unit (STM32 USB virtual COM port).

    python tools/door.py info                      # model / flash / people / timings
    python tools/door.py flash-model               # upload build/rec_weights.bin to the external flash (once)
    python tools/door.py enrol Thinker             # stand in front of the camera; 10 samples (~5 s)
    python tools/door.py list
    python tools/door.py delete Thinker            # or: delete ALL
    python tools/door.py thresh 0.42               # recognition threshold (cosine); no value = show it
    python tools/door.py open                      # open the door remotely
    python tools/door.py monitor [--debug]         # print events (GRANT/UNKNOWN/SPOOF...) and per-frame details
    python tools/door.py snap photo.png            # save what the camera sees
    python tools/door.py send "ANY COMMAND"        # raw command, prints the replies for 2 s

Add --port COM7 if the board is not found automatically.
"""
import argparse
import struct
import sys
import time
import zlib
from pathlib import Path

try:
    import serial
    import serial.tools.list_ports
except ImportError:
    sys.exit("pyserial missing: pip install pyserial")

ST_VID = 0x0483
ROOT = Path(__file__).resolve().parent.parent
DEFAULT_WEIGHTS = ROOT / "firmware/stm32h723/door_unit/build/rec_weights.bin"
CHUNK = 4096


def find_port():
    ports = [p for p in serial.tools.list_ports.comports() if p.vid == ST_VID]
    if not ports:
        names = ", ".join(p.device for p in serial.tools.list_ports.comports()) or "none"
        sys.exit(f"Board not found (ports: {names}). Plug in the USB-C cable or pass --port.")
    return ports[0].device


class Board:
    def __init__(self, port):
        self.ser = serial.Serial(port, 115200, timeout=0.2)
        self.ser.dtr = True
        self.buf = bytearray()
        time.sleep(0.2)
        self.ser.reset_input_buffer()

    def send(self, line):
        self.ser.write((line.strip() + "\n").encode())

    def lines(self, seconds):
        """Yield text lines for up to `seconds` (EVT/DBG lines included)."""
        end = time.time() + seconds
        while time.time() < end:
            data = self.ser.read(max(1, self.ser.in_waiting))
            if data:
                self.buf += data
            while b"\n" in self.buf:
                i = self.buf.index(b"\n")
                raw = bytes(self.buf[:i]); del self.buf[:i + 1]
                yield raw.decode("ascii", "replace").strip()

    def wait_for(self, prefixes, seconds, echo=True):
        for ln in self.lines(seconds):
            if not ln:
                continue
            if ln.startswith(tuple(prefixes)):
                return ln
            if echo:
                print("  board:", ln)
        return None

    def command(self, line, prefixes, seconds=3.0):
        self.send(line)
        r = self.wait_for(prefixes, seconds)
        if r is None:
            sys.exit(f"No reply to {line!r}. Is the door unit firmware running?")
        return r


def cmd_flash_model(b, args):
    data = Path(args.file).read_bytes()
    crc = zlib.crc32(data) & 0xFFFFFFFF
    print(f"Uploading {args.file} ({len(data)} bytes, CRC {crc:08x})")
    r = b.command(f"FLASHREC {len(data)} {crc:08x}", ("READY", "ERR"), seconds=90)
    if not r.startswith("READY"):
        sys.exit(r)
    off, t0 = 0, time.time()
    while off < len(data):
        part = data[off:off + CHUNK]
        for attempt in range(3):
            b.ser.write(f"DATA {off} {len(part)} {zlib.crc32(part) & 0xFFFFFFFF:08x}\n".encode() + part)
            r = b.wait_for(("OK", "ERR"), 10)
            if r and r.startswith("OK"):
                break
            print("  retry:", r)
        else:
            sys.exit(f"Upload failed at {off}: {r}")
        off += len(part)
        print(f"\r  {100 * off // len(data):3d}%  ({off // 1024} KB, {time.time() - t0:.0f} s)", end="", flush=True)
    print()
    r = b.command("FLASHEND", ("REC OK", "ERR"), seconds=30)
    print(r)
    if not r.startswith("REC OK"):
        sys.exit(1)
    print("Face recognition model installed. Next: python tools/door.py enrol <name>")


def cmd_enrol(b, args):
    cmd = f"ENROL {args.name}" + (f" {args.samples}" if args.samples else "")
    r = b.command(cmd, ("OK", "ERR"))
    print(r)
    if r.startswith("ERR"):
        sys.exit(1)
    print("Look at the camera from about 50 cm; turn your head slightly left/right while it beeps.")
    r = b.wait_for(("OK enrolled", "ERR"), 60)
    print(r or "No answer (timeout)")


def cmd_monitor(b, args):
    if args.debug:
        b.send("DEBUG 1")
    print("Listening (Ctrl+C to stop)...")
    try:
        while True:
            for ln in b.lines(1.0):
                if ln:
                    print(time.strftime("%H:%M:%S"), ln, flush=True)
    except KeyboardInterrupt:
        pass
    finally:
        if args.debug:
            b.send("DEBUG 0")


def cmd_snap(b, args):
    import numpy as np
    import cv2
    b.send("SNAP")
    hdr = struct.Struct("<4sHHHHII")
    end, buf = time.time() + 5, bytearray()
    while time.time() < end:
        buf += b.ser.read(max(4096, b.ser.in_waiting))
        i = buf.find(b"FRM2")
        if i >= 0 and len(buf) >= i + hdr.size:
            _, w, h, fmt, seq, length, crc = hdr.unpack_from(buf, i)
            if len(buf) >= i + hdr.size + length:
                px = bytes(buf[i + hdr.size:i + hdr.size + length])
                if zlib.crc32(px) & 0xFFFFFFFF != crc:
                    sys.exit("frame damaged, try again")
                v = np.frombuffer(px, ">u2").reshape(h, w).astype(np.uint32)
                img = np.dstack([(v & 0x1F) * 255 // 31, ((v >> 5) & 0x3F) * 255 // 63, ((v >> 11) & 0x1F) * 255 // 31]).astype(np.uint8)
                cv2.imwrite(args.out, img)
                print("saved", args.out)
                return
    sys.exit("no frame received")


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--port")
    sub = ap.add_subparsers(dest="cmd", required=True)
    sub.add_parser("info")
    p = sub.add_parser("flash-model"); p.add_argument("file", nargs="?", default=str(DEFAULT_WEIGHTS))
    p = sub.add_parser("enrol"); p.add_argument("name"); p.add_argument("--samples", type=int)
    sub.add_parser("list")
    p = sub.add_parser("delete"); p.add_argument("name")
    p = sub.add_parser("thresh"); p.add_argument("value", nargs="?")
    sub.add_parser("open")
    p = sub.add_parser("monitor"); p.add_argument("--debug", action="store_true")
    p = sub.add_parser("snap"); p.add_argument("out", nargs="?", default="snap.png")
    p = sub.add_parser("send"); p.add_argument("line")
    args = ap.parse_args()

    b = Board(args.port or find_port())
    if args.cmd == "info":
        print(b.command("INFO", ("INFO",)))
        print(b.wait_for(("CAM",), 1) or "")
    elif args.cmd == "flash-model":
        cmd_flash_model(b, args)
    elif args.cmd == "enrol":
        cmd_enrol(b, args)
    elif args.cmd == "list":
        r = b.command("LIST", ("PEOPLE",))
        print(r)
        for _ in range(int(r.split()[1])):
            print(" ", b.wait_for(("PERSON",), 2))
    elif args.cmd == "delete":
        print(b.command(f"DELETE {args.name}", ("OK", "ERR")))
    elif args.cmd == "thresh":
        print(b.command("THRESH" + (f" {args.value}" if args.value else ""), ("OK", "ERR")))
    elif args.cmd == "open":
        print(b.command("OPEN", ("OK", "ERR")))
    elif args.cmd == "monitor":
        cmd_monitor(b, args)
    elif args.cmd == "snap":
        cmd_snap(b, args)
    elif args.cmd == "send":
        b.send(args.line)
        for ln in b.lines(2):
            if ln:
                print(ln)


if __name__ == "__main__":
    main()
