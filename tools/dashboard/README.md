# Door Admin dashboard

Flask + SQLite web app for the administrator of the face-recognition door.

```
pip install flask
python app.py              # http://localhost:5000, first login admin / admin
```

It prints the **door key**; put it, the laptop's IP address and the WiFi details in
`firmware/esp32/door_bridge/door_bridge.ino`, then flash that sketch to the ESP32 DevKit.

Pages: **Review** (unknown faces: let in once / add permanently / reject), **People** (access hours, remove),
**Access log** (mirror of the door's own log, filters, CSV), **Settings** (door opening hours, password, open the door).

Without the hardware: `python fake_door.py --key <door key>` runs a door on the PC with the real face models
(needs `pip install onnxruntime opencv-python numpy`). Type a photo's path to put that face at the door: an
enrolled face inside its hours opens the door by itself; an unknown face goes to Pending Review, and after
"Add permanently" the same face is let in automatically. `enrol <name> photo.jpg` enrols from photos.
