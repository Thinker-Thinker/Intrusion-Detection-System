/* Door I/O: ESP32 DevKit drives the lock and signals for the PC that runs the face pipeline
 * (tools/door_pc). Connected to the laptop by USB (serial 115200). It decides nothing about faces.
 *
 * Commands from the PC (one per line):   PING, OPEN <ms>, LOCK, LED G|R|OFF, BEEP <ms>, ALARM <ms>, SERVO <deg>, TEST
 * Events to the PC:  EVT EXIT (exit button: the door opens by itself), EVT MOTION, EVT DOOR OPEN / EVT DOOR CLOSED
 *
 * Wiring (all grounds joined; servo and ESP32 on the 5 V rail, 1000 uF across the servo):
 *   servo signal GPIO18 | green LED GPIO25 (330 R) | red LED GPIO26 (330 R) | buzzer via S8050 base (1 k) GPIO27
 *   PIR out GPIO34 | exit button GPIO32 to GND | reed switch GPIO33 to GND */
#define SERVO_PIN 18
#define LED_G 25
#define LED_R 26
#define BUZZ 27
#define PIR_PIN 34
#define EXIT_PIN 32
#define REED_PIN 33
#define CLOSED_DEG 0
#define OPEN_DEG 90

uint32_t openUntil = 0, beepUntil = 0, alarmUntil = 0, ledUntil = 0;
String line;

void servoBegin() {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcAttach(SERVO_PIN, 50, 16);
#else
  ledcSetup(0, 50, 16);
  ledcAttachPin(SERVO_PIN, 0);
#endif
}
void servoDeg(int deg) {
  deg = constrain(deg, 0, 180);
  uint32_t duty = (uint32_t)(500 + deg * 2000 / 180) * 65535UL / 20000UL;
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcWrite(SERVO_PIN, duty);
#else
  ledcWrite(0, duty);
#endif
}
void leds(bool g, bool r) { digitalWrite(LED_G, g); digitalWrite(LED_R, r); }
void beep(uint32_t ms) { alarmUntil = 0; digitalWrite(BUZZ, HIGH); beepUntil = millis() + ms; }
void openDoor(uint32_t ms) { servoDeg(OPEN_DEG); leds(true, false); openUntil = millis() + ms; }

void handle(String c) {
  c.trim();
  String u = c; u.toUpperCase();
  int sp = u.indexOf(' ');
  long arg = sp > 0 ? u.substring(sp + 1).toInt() : 0;
  if (u == "PING") Serial.println("PONG io");
  else if (u.startsWith("OPEN")) { openDoor(arg > 0 ? arg : 5000); Serial.println("OK open"); }
  else if (u == "LOCK") { openUntil = 0; servoDeg(CLOSED_DEG); leds(false, false); Serial.println("OK lock"); }
  else if (u == "LED G") { leds(true, false); ledUntil = millis() + 2000; Serial.println("OK"); }
  else if (u == "LED R") { leds(false, true); ledUntil = millis() + 2000; Serial.println("OK"); }
  else if (u == "LED OFF") { leds(false, false); Serial.println("OK"); }
  else if (u.startsWith("BEEP")) { beep(arg > 0 ? arg : 150); Serial.println("OK"); }
  else if (u.startsWith("ALARM")) { alarmUntil = millis() + (arg > 0 ? arg : 3000); leds(false, true); ledUntil = alarmUntil; Serial.println("OK"); }
  else if (u.startsWith("SERVO")) { servoDeg(arg); Serial.println("OK servo"); }
  else if (u == "TEST") {
    Serial.println("TEST green, red, buzzer, servo");
    leds(true, false); delay(600); leds(false, true); delay(600); leds(false, false);
    digitalWrite(BUZZ, HIGH); delay(200); digitalWrite(BUZZ, LOW);
    servoDeg(OPEN_DEG); delay(800); servoDeg(CLOSED_DEG);
    Serial.println("TEST done");
  }
  else if (u.length()) Serial.println("?? " + c);
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_G, OUTPUT); pinMode(LED_R, OUTPUT); pinMode(BUZZ, OUTPUT);
  pinMode(PIR_PIN, INPUT); pinMode(EXIT_PIN, INPUT_PULLUP); pinMode(REED_PIN, INPUT_PULLUP);
  leds(false, false); digitalWrite(BUZZ, LOW);
  servoBegin(); servoDeg(CLOSED_DEG);
  Serial.println("READY door_io");
}

void loop() {
  while (Serial.available()) {
    char ch = Serial.read();
    if (ch == '\n') { handle(line); line = ""; }
    else if (ch != '\r' && line.length() < 60) line += ch;
  }
  uint32_t now = millis();
  if (openUntil && (int32_t)(now - openUntil) >= 0) { openUntil = 0; servoDeg(CLOSED_DEG); leds(false, false); }
  if (beepUntil && (int32_t)(now - beepUntil) >= 0) { beepUntil = 0; digitalWrite(BUZZ, LOW); }
  if (alarmUntil) {
    if ((int32_t)(now - alarmUntil) >= 0) { alarmUntil = 0; digitalWrite(BUZZ, LOW); }
    else digitalWrite(BUZZ, (now / 125) & 1);
  }
  if (ledUntil && (int32_t)(now - ledUntil) >= 0 && !openUntil) { ledUntil = 0; leds(false, false); }

  static int btnLast = HIGH; static uint32_t btnT = 0;
  int b = digitalRead(EXIT_PIN);
  if (b != btnLast && now - btnT > 50) { btnT = now; btnLast = b; if (b == LOW) { openDoor(5000); beep(60); Serial.println("EVT EXIT"); } }
  static int pirLast = LOW; static uint32_t pirT = 0;
  int p = digitalRead(PIR_PIN);
  if (p && !pirLast && now - pirT > 10000) { pirT = now; Serial.println("EVT MOTION"); }
  pirLast = p;
  static int reedLast = -1;
  int r = digitalRead(REED_PIN);
  if (r != reedLast) { reedLast = r; Serial.println(r ? "EVT DOOR OPEN" : "EVT DOOR CLOSED"); }
}
