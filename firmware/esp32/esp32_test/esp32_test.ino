/*
  ESP32 hardware test for the door unit (Rev B wiring)
  ----------------------------------------------------
  Arduino IDE: Board = "ESP32 Dev Module", upload, then open Serial Monitor at 115200 baud
  with "Newline" line ending. Type a letter and press Enter:

    i  info: chip, flash, free memory, MAC address
    w  Wi-Fi scan (shows nearby networks and signal strength)
    l  UART2 loopback: put a jumper wire from GPIO16 to GPIO17 first (STM32 wires removed)
    p  ping the STM32 over UART2 (GPIO16 = RX2 <- STM32 PA9, GPIO17 = TX2 -> STM32 PA10)
    s  servo sweep on GPIO18 (power test only: servo signal wire to GPIO18), x = stop
    b  blink the on-board LED (GPIO2) 5 times

  Anything the STM32 sends on UART2 is printed as "[STM32] ...".
  Works with ESP32 Arduino core 2.x and 3.x.
*/
#include <WiFi.h>

#define RX2_PIN   16   // from STM32 PA9 (TX)
#define TX2_PIN   17   // to   STM32 PA10 (RX)
#define SERVO_PIN 18   // test only; in the final design the STM32 (PB0) drives the servo
#define LED_PIN   2    // on-board blue LED on most ESP32 DevKit boards
#define UART2_BAUD 115200

HardwareSerial Link(2);
bool sweeping = false;

// ---------- servo PWM (50 Hz, 16-bit) ----------
void servoBegin() {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcAttach(SERVO_PIN, 50, 16);
#else
  ledcSetup(0, 50, 16);
  ledcAttachPin(SERVO_PIN, 0);
#endif
}
void servoWriteUs(int us) {               // 500..2500 us pulse in a 20 ms period
  uint32_t duty = (uint32_t)us * 65535UL / 20000UL;
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcWrite(SERVO_PIN, duty);
#else
  ledcWrite(0, duty);
#endif
}

// ---------- tests ----------
void info() {
  Serial.println("\n--- ESP32 info ---");
  Serial.printf("Chip: %s rev %d, %d cores @ %d MHz\n", ESP.getChipModel(), ESP.getChipRevision(),
                ESP.getChipCores(), ESP.getCpuFreqMHz());
  Serial.printf("Flash: %u KB   Free heap: %u KB\n", ESP.getFlashChipSize() / 1024, ESP.getFreeHeap() / 1024);
  Serial.printf("PSRAM: %s\n", psramFound() ? "YES - warning: GPIO16/17 may be used by PSRAM on WROVER modules, use other UART pins" : "no (GPIO16/17 are free)");
  Serial.print("MAC: "); Serial.println(WiFi.macAddress());
}

void wifiScan() {
  Serial.println("\n--- Wi-Fi scan (about 3 s) ---");
  WiFi.mode(WIFI_STA); WiFi.disconnect();
  int n = WiFi.scanNetworks();
  if (n <= 0) { Serial.println("No networks found. Check the antenna end of the board is not covered."); return; }
  for (int i = 0; i < n && i < 15; i++)
    Serial.printf("%2d  %-28s  %4d dBm  ch %d\n", i + 1, WiFi.SSID(i).c_str(), WiFi.RSSI(i), WiFi.channel(i));
  Serial.println("PASS: Wi-Fi radio works (-30 to -70 dBm is good, below -80 is weak).");
  WiFi.scanDelete();
}

void loopback() {
  Serial.println("\n--- UART2 loopback (jumper GPIO16 <-> GPIO17) ---");
  while (Link.available()) Link.read();
  const char *msg = "LOOPBACK-123";
  Link.print(msg);
  String got; unsigned long t = millis();
  while (millis() - t < 300) { while (Link.available()) got += (char)Link.read(); }
  if (got == msg) Serial.println("PASS: UART2 sends and receives correctly.");
  else Serial.printf("FAIL: sent \"%s\", got \"%s\". Check the jumper is on GPIO16 and GPIO17.\n", msg, got.c_str());
}

void pingStm32() {
  Serial.println("\n--- Ping STM32 ---");
  while (Link.available()) Link.read();
  Link.print("PING\n");
  String got; unsigned long t = millis();
  while (millis() - t < 1000) {
    while (Link.available()) { char c = Link.read(); if (c == '\n') goto done; if (c != '\r') got += c; }
  }
done:
  if (got.length()) Serial.printf("PASS: STM32 replied \"%s\"\n", got.c_str());
  else Serial.println("No reply in 1 s. Normal until the STM32 test firmware is flashed. Otherwise check: "
                      "PA9->GPIO16, PA10<-GPIO17 (TX to RX), grounds joined, both at 115200.");
}

void blink() {
  for (int i = 0; i < 5; i++) { digitalWrite(LED_PIN, HIGH); delay(200); digitalWrite(LED_PIN, LOW); delay(200); }
  Serial.println("Blinked 5 times (if no LED, your board may have none on GPIO2; that is fine).");
}

void help() {
  Serial.println("\nCommands: i=info  w=wifi scan  l=UART loopback  p=ping STM32  s=servo sweep  x=stop  b=blink");
}

void setup() {
  Serial.begin(115200);
  delay(500);
  pinMode(LED_PIN, OUTPUT);
  Link.begin(UART2_BAUD, SERIAL_8N1, RX2_PIN, TX2_PIN);
  servoBegin(); servoWriteUs(1500);
  Serial.println("\n=== ESP32 door-unit test ===");
  info();
  help();
}

void loop() {
  if (Serial.available()) {
    char c = Serial.read();
    switch (c) {
      case 'i': info(); break;
      case 'w': wifiScan(); break;
      case 'l': loopback(); break;
      case 'p': pingStm32(); break;
      case 's': sweeping = true; Serial.println("Servo sweeping on GPIO18. Watch the 5 V rail; type x to stop."); break;
      case 'x': sweeping = false; servoWriteUs(1500); Serial.println("Servo stopped at centre."); break;
      case 'b': blink(); break;
      case '\n': case '\r': break;
      default: help();
    }
  }

  // print anything the STM32 sends
  static String line;
  while (Link.available()) {
    char c = Link.read();
    if (c == '\n') { Serial.print("[STM32] "); Serial.println(line); line = ""; }
    else if (c != '\r' && line.length() < 200) line += c;
  }

  // servo sweep: 0 -> 180 -> 0 degrees, about 2 s each way
  static unsigned long last = 0; static int us = 500, step = 20;
  if (sweeping && millis() - last > 20) {
    last = millis();
    us += step; if (us >= 2500 || us <= 500) step = -step;
    servoWriteUs(us);
  }

  // heartbeat on the LED
  static unsigned long hb = 0;
  if (!sweeping && millis() - hb > 1000) { hb = millis(); digitalWrite(LED_PIN, !digitalRead(LED_PIN)); }
}
