/* Door bridge: ESP32 DevKit between the STM32 door unit (UART) and the administrator dashboard (WiFi/HTTP).
 * It makes no access decision: the STM32 keeps working when this board or the network is down.
 *
 *   STM32 -> here -> dashboard:  LOG lines (access log), FACE records (unknown faces), PERSON/SCHED lists
 *   dashboard -> here -> STM32:  ADMIT, ADDB, WIN, DELETE, SCHED, OPEN, plus TIME and LOGSINCE from here
 * See firmware/stm32h723/door_unit/Core/Inc/admin.h for the line protocol.
 *
 * Wiring (as before): GPIO16 (RX2) <- STM32 PA9, GPIO17 (TX2) -> STM32 PA10, GND <-> GND. 921600 baud.
 * Set WIFI_SSID / WIFI_PASS, SERVER (the laptop running tools/dashboard/app.py) and DOOR_KEY (shown by
 * the dashboard at start-up and in Settings). Arduino IDE: board "ESP32 Dev Module". */
#include <WiFi.h>
#include <HTTPClient.h>
#include "base64.h"

const char *WIFI_SSID = "YourHotspot";
const char *WIFI_PASS = "password";
const char *SERVER = "http://192.168.43.10:5000";      // the laptop's address on that WiFi, port 5000
const char *DOOR_KEY = "paste-the-door-key-here";

#define RX2_PIN 16
#define TX2_PIN 17
#define LINK_BAUD 921600
#define FACE_BYTES (112 * 112 * 3 + 4 + 512)

HardwareSerial Link(2);
static uint8_t *face_buf;
static String log_batch, people_json, sched_json;
static int log_count;
static uint32_t log_t, last_poll, last_time_sent, last_logsince, last_link_msg;
static long server_seq = -1;
static uint32_t last_reconnect;
static bool link_ok = false, logsince_busy = false, people_asked = false;
static int fails = 0;

// ------------------------------------------------------------------ helpers
static uint32_t crc32(const uint8_t *p, size_t n) {
  uint32_t c = 0xFFFFFFFF;
  while (n--) { c ^= *p++; for (int k = 0; k < 8; k++) c = (c >> 1) ^ (0xEDB88320 & (0 - (c & 1))); }
  return ~c;
}
static void to_door(const String &s) { Link.print(s); Link.print('\n'); delay(5); }

static int post(const char *path, const String &body, String *reply = nullptr) {
  if (WiFi.status() != WL_CONNECTED) return -1;
  HTTPClient http;
  http.begin(String(SERVER) + path);
  http.setTimeout(8000);
  http.addHeader("X-Door-Key", DOOR_KEY);
  http.addHeader("Content-Type", "application/json");
  int code = http.POST(body);
  if (reply && code > 0) *reply = http.getString();
  http.end();
  return code;
}

static String jstr(const String &s) {             // JSON string with quotes escaped
  String o = "\"";
  for (char c : s) { if (c == '"' || c == '\\') o += '\\'; o += c; }
  return o + "\"";
}

// ------------------------------------------------------------------ from the STM32
static void flush_logs() {
  if (!log_count) return;
  int code = post("/api/log", "{\"records\":[" + log_batch + "]}");
  if (code == 200) { log_batch = ""; log_count = 0; }
  else if (log_count > 200) { log_batch = ""; log_count = 0; }   // the door keeps its own log; LOGSINCE refills it
}

static void on_log(const String &l) {        // LOG <seq> <time> <TYPE> <name|-> <score>
  char type[16], name[32]; unsigned long seq, t; float score;
  if (sscanf(l.c_str(), "LOG %lu %lu %15s %31s %f", &seq, &t, type, name, &score) != 5) return;
  if (log_count) log_batch += ",";
  log_batch += "{\"seq\":" + String(seq) + ",\"time\":" + String(t) + ",\"type\":\"" + type + "\",\"name\":" +
               jstr(name) + ",\"score\":" + String(score, 3) + "}";
  log_count++;
  if (!log_t) log_t = millis();
}

static void on_face(const String &l) {       // FACE <id> <time> <score> <closest|-> <len> <crc>
  unsigned long id, t, len, crc; float score; char closest[32];
  if (sscanf(l.c_str(), "FACE %lu %lu %f %31s %lu %lx", &id, &t, &score, closest, &len, &crc) != 6 || len != FACE_BYTES) return;
  size_t got = Link.readBytes(face_buf, len);
  if (got != len || crc32(face_buf, len) != crc) { Serial.printf("face %lu damaged (%u bytes)\n", id, (unsigned)got); return; }
  float scale; memcpy(&scale, face_buf + 112 * 112 * 3, 4);
  String body = "{\"id\":" + String(id) + ",\"time\":" + String(t) + ",\"score\":" + String(score, 3) +
                ",\"closest\":" + jstr(strcmp(closest, "-") ? closest : "") + ",\"scale\":" + String(scale, 9) +
                ",\"crop\":\"" + base64::encode(face_buf, 112 * 112 * 3) + "\",\"emb\":\"" +
                base64::encode(face_buf + 112 * 112 * 3 + 4, 512) + "\"}";
  int code = post("/api/face", body);
  Serial.printf("face %lu -> dashboard: HTTP %d\n", id, code);
  if (code == 200) to_door("FACEACK " + String(id));
}

static void on_person(const String &l) {     // PERSON <name> <start> <end> <days>
  char name[32]; int s, e, d;
  if (sscanf(l.c_str(), "PERSON %31s %d %d %d", name, &s, &e, &d) != 4) return;
  if (people_json.length()) people_json += ",";
  people_json += "{\"name\":" + jstr(name) + ",\"start\":" + s + ",\"end\":" + e + ",\"days\":" + d + "}";
}

static void on_line(const String &l) {
  if (l.startsWith("LOG ")) on_log(l);
  else if (l.startsWith("LOGEND")) { logsince_busy = false; flush_logs(); }
  else if (l.startsWith("FACE ")) on_face(l);
  else if (l.startsWith("PERSON ")) on_person(l);
  else if (l.startsWith("SCHED ")) {
    int on, s, e, d;
    if (sscanf(l.c_str(), "SCHED %d %d %d %d", &on, &s, &e, &d) == 4)
      sched_json = "{\"on\":" + String(on) + ",\"start\":" + s + ",\"end\":" + e + ",\"days\":" + d + "}";
  }
  else if (l.startsWith("PEOPLEEND")) {
    int code = post("/api/people", "{\"people\":[" + people_json + "]" + (sched_json.length() ? ",\"sched\":" + sched_json : "") + "}");
    Serial.printf("people list -> dashboard: HTTP %d\n", code);
    people_json = ""; sched_json = "";
  }
  else Serial.println("door: " + l);           // EVT, OK, ERR ... shown on the USB serial monitor
}

static void read_door() {
  static String line;
  while (Link.available()) {
    char c = Link.read();
    if (c == '\n') {
      String l = line; line = ""; l.trim();
      if (l.length()) on_line(l);              // a FACE line reads its binary payload right here
    } else if (c != '\r' && line.length() < 200) line += c;
  }
}

// ------------------------------------------------------------------ to the dashboard and back
static void poll_dashboard() {
  if (WiFi.status() != WL_CONNECTED) { fails++; return; }
  HTTPClient http;
  http.begin(String(SERVER) + "/api/poll.txt");
  http.setTimeout(4000);
  http.addHeader("X-Door-Key", DOOR_KEY);
  int code = http.GET();
  String body = code == 200 ? http.getString() : "";
  http.end();
  if (code != 200) { fails++; if (code == 403) Serial.println("dashboard says the DOOR_KEY is wrong"); return; }
  fails = 0;
  int i = 0;
  while (i < (int)body.length()) {
    int j = body.indexOf('\n', i); if (j < 0) j = body.length();
    String l = body.substring(i, j); l.trim(); i = j + 1;
    if (!l.length()) continue;
    if (l.startsWith("TIME ")) {
      if (millis() - last_time_sent > 30000 || !last_time_sent) { to_door(l); last_time_sent = millis(); }
    } else if (l.startsWith("LOGSEQ ")) server_seq = l.substring(7).toInt();
    else { Serial.println("admin -> door: " + l.substring(0, 60)); to_door(l); }
  }
}

void setup() {
  Serial.begin(115200);
  Link.setRxBufferSize(48 * 1024);              // a whole FACE record fits while WiFi is busy
  Link.begin(LINK_BAUD, SERIAL_8N1, RX2_PIN, TX2_PIN);
  Link.setTimeout(3000);
  face_buf = (uint8_t *)malloc(FACE_BYTES);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.printf("\nDoor bridge: joining %s, dashboard %s\n", WIFI_SSID, SERVER);
}

void loop() {
  read_door();
  uint32_t now = millis();
  if (log_count && (log_count >= 40 || now - log_t > 1000)) { flush_logs(); log_t = 0; }

  if (now - last_poll > 1000) {
    last_poll = now;
    poll_dashboard();
    bool up = fails == 0;
    if (up != link_ok || now - last_link_msg > 10000) {
      to_door(up ? "LINK UP" : "LINK DOWN"); last_link_msg = now;
      if (up && !link_ok) { Serial.printf("dashboard reachable (%s)\n", WiFi.localIP().toString().c_str()); people_asked = false; }
      link_ok = up;
    }
    if (link_ok && !people_asked) { to_door("PEOPLE"); people_asked = true; }
    /* copy any log records the dashboard does not have yet, 40 at a time */
    if (link_ok && server_seq >= 0 && (!logsince_busy || now - last_logsince > 5000) && now - last_logsince > 2000) {
      to_door("LOGSINCE " + String(server_seq) + " 40");
      logsince_busy = true; last_logsince = now;
    }
  }
  if (WiFi.status() != WL_CONNECTED && now - last_reconnect > 15000) { last_reconnect = now; WiFi.reconnect(); }
}
