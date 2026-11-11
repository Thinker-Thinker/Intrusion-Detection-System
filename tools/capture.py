#!/usr/bin/env python3
"""
Live preview + labelled dataset capture from the STM32H723 camera firmware.

The board streams 320x240 RGB565 frames over its USB virtual COM port
(firmware/stm32h723/camera_stream). This tool shows them and saves PNGs into
dataset/<label>/ so every training image comes from the camera actually used
at the door.

Usage:
    pip install -r tools/requirements.txt
    python tools/capture.py                 # auto-detects the board
    python tools/capture.py --port COM5     # or name the port

Keys (click the preview window first):
    1-9      choose the label shown at the top of the window
    SPACE    save the current frame under that label
    r        start/stop recording (saves every frame, ~2-4 per second)
    [ / ]    camera brightness target down / up (shown as "bright:" in the bar)
    b        swap RGB565 byte order (press if colours look wrong)
    q / ESC  quit
"""
import argparse
import re
import struct
import zlib
import sys
import time
from datetime import datetime
from pathlib import Path

import numpy as np

try:
    import serial
    import serial.tools.list_ports
except ImportError:
    sys.exit("pyserial missing: pip install -r tools/requirements.txt")

HEADER = struct.Struct("<4sHHHHI")     # FRM1: magic, width, height, format, seq, length
HEADER2 = struct.Struct("<4sHHHHII")   # FRM2: ... + crc32 of the payload
MAGIC = b"FRM"
ST_VID, ST_CDC_PID = 0x0483, 0x5740

DEFAULT_LABELS = [
    "live_face",       # real person in front of the camera
    "spoof_print",     # printed photo of an enrolled person
    "spoof_screen",    # phone/laptop screen showing a face or video
    "knife",           # person holding a knife (kitchen knife is fine)
    "gun_replica",     # toy or replica firearm only
    "no_weapon",       # person with phone, bag, umbrella, bottle, empty hands
    "empty_scene",     # doorway with nobody there
]


def rgb565_to_bgr(payload: bytes, w: int, h: int, big_endian: bool = True) -> np.ndarray:
    """Convert raw RGB565 bytes to an OpenCV BGR image (uint8, h x w x 3)."""
    dtype = ">u2" if big_endian else "<u2"
    px = np.frombuffer(payload, dtype=dtype).reshape(h, w).astype(np.uint32)
    r = ((px >> 11) & 0x1F) * 255 // 31
    g = ((px >> 5) & 0x3F) * 255 // 63
    b = (px & 0x1F) * 255 // 31
    return np.dstack([b, g, r]).astype(np.uint8)


class FrameReader:
    """Finds 'FRM1'/'FRM2' headers in the byte stream and returns whole frames.
    FRM2 frames carry a CRC-32; damaged ones are counted in .bad and skipped."""

    def __init__(self, ser):
        self.ser = ser
        self.buf = bytearray()
        self.bad = 0
        self.info = {}  # latest INFO fields from the board, e.g. {"ae": "0x78"}

    def _fill(self, n):
        while len(self.buf) < n:
            chunk = self.ser.read(max(4096, self.ser.in_waiting or 0))
            if not chunk:
                return False
            self.buf += chunk
        return True

    def read_frame(self):
        """Return (header_tuple, payload) or None on timeout."""
        while True:
            if not self._fill(HEADER.size):
                return None
            i = self.buf.find(MAGIC)
            if i < 0:
                # keep the last 3 bytes in case the magic is split across reads
                del self.buf[:-3]
                continue
            if i:
                # Bytes before a header are either the board's INFO line or the
                # tail of a frame we joined mid-way (binary). Print only INFO lines.
                for line in re.findall(rb"INFO [ -~]{1,120}", bytes(self.buf[:i])):
                    text = line.decode("ascii")
                    print("[board]", text)
                    self.info.update(dict(kv.split("=", 1) for kv in text.split()[1:] if "=" in kv))
                del self.buf[:i]
            if not self._fill(HEADER2.size):
                return None
            version = self.buf[3:4]
            if version == b"2":
                hdr = HEADER2
                magic, w, h, fmt, seq, length, crc = hdr.unpack_from(self.buf)
            elif version == b"1":
                hdr = HEADER
                magic, w, h, fmt, seq, length = hdr.unpack_from(self.buf)
                crc = None
            else:
                del self.buf[:3]  # "FRM" inside pixel data, keep looking
                continue
            if length != w * h * 2 or w == 0 or h == 0:
                del self.buf[:4]  # false match, resync
                continue
            if not self._fill(hdr.size + length):
                return None
            payload = bytes(self.buf[hdr.size:hdr.size + length])
            if crc is not None and zlib.crc32(payload) != crc:
                # Damaged in transit: skip only this header, so a following
                # frame's header inside the bad payload is still found.
                self.bad += 1
                del self.buf[:4]
                continue
            del self.buf[:hdr.size + length]
            return (w, h, fmt, seq, length), payload


def find_port():
    for p in serial.tools.list_ports.comports():
        if p.vid == ST_VID and p.pid == ST_CDC_PID:
            return p.device
    return None


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--port", help="serial port, e.g. COM5 or /dev/ttyACM0 (auto-detected if omitted)")
    ap.add_argument("--out", default="dataset", help="output folder (default: dataset)")
    ap.add_argument("--labels", nargs="+", default=DEFAULT_LABELS, help="label names for keys 1-9")
    ap.add_argument("--scale", type=int, default=2, help="preview zoom factor")
    ap.add_argument("--little-endian", action="store_true", help="start with little-endian RGB565")
    args = ap.parse_args()

    import cv2  # imported here so --help works without OpenCV

    port = args.port or find_port()
    if not port:
        sys.exit("Board not found. Plug it in over USB-C, or pass --port.")
    ser = serial.Serial(port, 115200, timeout=2)
    ser.dtr = True  # firmware only streams while the port is open (DTR set)
    time.sleep(0.2)
    ser.reset_input_buffer()
    ser.write(b"I")   # board replies with an INFO line
    ser.write(b"C")   # continuous streaming

    reader = FrameReader(ser)
    out = Path(args.out)
    labels = args.labels[:9]
    label_i, recording, big_endian = 0, False, not args.little_endian
    saved = {l: len(list((out / l).glob("*.png"))) for l in labels}
    t_last, fps = time.time(), 0.0

    print(f"Streaming from {port}. Press 1-{len(labels)} to pick a label, SPACE to save, q to quit.")
    try:
        while True:
            got = reader.read_frame()
            if got is None:
                print("No frame for 2 s, asking again...")
                ser.write(b"C")
                continue
            (w, h, fmt, seq, length), payload = got
            img = rgb565_to_bgr(payload, w, h, big_endian)
            now = time.time()
            fps = 0.8 * fps + 0.2 / max(now - t_last, 1e-3)
            t_last = now

            label = labels[label_i]
            save_now = recording
            view = cv2.resize(img, (w * args.scale, h * args.scale), interpolation=cv2.INTER_NEAREST)
            bar = f"[{label_i + 1}] {label}  saved:{saved[label]}  {fps:.1f} fps"
            if "ae" in reader.info:
                bar += f"  bright:{reader.info['ae']} gain:{reader.info.get('gain', '?')}"
            if reader.bad:
                bar += f"  damaged:{reader.bad}"
            if recording:
                bar += "  REC"
            cv2.rectangle(view, (0, 0), (view.shape[1], 26), (0, 0, 0), -1)
            cv2.putText(view, bar, (6, 18), cv2.FONT_HERSHEY_SIMPLEX, 0.55,
                        (0, 0, 255) if recording else (255, 255, 255), 1, cv2.LINE_AA)
            cv2.imshow("H723 camera", view)

            key = cv2.waitKey(1) & 0xFF
            if key in (ord("q"), 27):
                break
            if ord("1") <= key <= ord("9") and key - ord("1") < len(labels):
                label_i = key - ord("1")
            elif key == ord(" "):
                save_now = True
            elif key == ord("r"):
                recording = not recording
            elif key in (ord("["), ord("]")):
                ser.write(bytes([key]))  # board answers with an INFO line
            elif key == ord("b"):
                big_endian = not big_endian
                print("byte order:", "big-endian" if big_endian else "little-endian")

            if save_now:
                d = out / label
                d.mkdir(parents=True, exist_ok=True)
                name = datetime.now().strftime("%Y%m%d_%H%M%S_%f") + f"_{seq:05d}.png"
                cv2.imwrite(str(d / name), img)
                saved[label] += 1
    finally:
        ser.write(b"P")
        ser.close()
        cv2.destroyAllWindows()
        print("Saved per label:", saved)


if __name__ == "__main__":
    main()
