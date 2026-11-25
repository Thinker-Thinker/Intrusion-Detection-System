/* Door camera: ESP32-CAM (AI-Thinker) streams 320x240 JPEG frames over WiFi to the PC that runs the
 * face pipeline (tools/door_pc). No inference here.
 *
 *   http://<ip>/stream    MJPEG stream (what the PC reads)
 *   http://<ip>/capture   one JPEG
 *   http://<ip>/flash?on=1  flash LED on (0 = off)
 *
 * WiFi: joins WIFI_SSID if it is set and reachable within 15 s, otherwise makes its own network
 * "DoorCam" (password doorcam123) at 192.168.4.1. The address is printed on the serial monitor (115200).
 * Arduino IDE: board "AI Thinker ESP32-CAM", PSRAM enabled. Power it from a solid 5 V (1 A or more). */
#include "esp_camera.h"
#include <WiFi.h>
#include "esp_http_server.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

const char *WIFI_SSID = "";            // your hotspot name, or leave "" to use the DoorCam network
const char *WIFI_PASS = "";
const char *AP_SSID = "DoorCam";
const char *AP_PASS = "doorcam123";

// AI-Thinker ESP32-CAM pins
#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27
#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22
#define FLASH_LED 4

static const char *BOUNDARY = "frame";

static esp_err_t capture_handler(httpd_req_t *req) {
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) { httpd_resp_send_500(req); return ESP_FAIL; }
  httpd_resp_set_type(req, "image/jpeg");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  esp_err_t r = httpd_resp_send(req, (const char *)fb->buf, fb->len);
  esp_camera_fb_return(fb);
  return r;
}

static esp_err_t stream_handler(httpd_req_t *req) {
  char part[80];
  httpd_resp_set_type(req, "multipart/x-mixed-replace;boundary=frame");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  while (true) {
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) { delay(10); continue; }
    size_t n = snprintf(part, sizeof part, "\r\n--%s\r\nContent-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n", BOUNDARY, (unsigned)fb->len);
    esp_err_t r = httpd_resp_send_chunk(req, part, n);
    if (r == ESP_OK) r = httpd_resp_send_chunk(req, (const char *)fb->buf, fb->len);
    esp_camera_fb_return(fb);
    if (r != ESP_OK) break;                       // the PC closed the connection
  }
  return ESP_OK;
}

static esp_err_t flash_handler(httpd_req_t *req) {
  char q[32] = "", v[4] = "0";
  httpd_req_get_url_query_str(req, q, sizeof q);
  httpd_query_key_value(q, "on", v, sizeof v);
  digitalWrite(FLASH_LED, v[0] == '1' ? HIGH : LOW);
  return httpd_resp_sendstr(req, "OK");
}

static void start_server() {
  httpd_config_t cfg = HTTPD_DEFAULT_CONFIG();
  cfg.server_port = 80;
  httpd_handle_t h = NULL;
  if (httpd_start(&h, &cfg) != ESP_OK) { Serial.println("HTTP server failed"); return; }
  httpd_uri_t a = {"/capture", HTTP_GET, capture_handler, NULL};
  httpd_uri_t b = {"/stream", HTTP_GET, stream_handler, NULL};
  httpd_uri_t c = {"/flash", HTTP_GET, flash_handler, NULL};
  httpd_register_uri_handler(h, &a);
  httpd_register_uri_handler(h, &b);
  httpd_register_uri_handler(h, &c);
}

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);   // the WiFi start-up current dip otherwise resets the board
  Serial.begin(115200);
  pinMode(FLASH_LED, OUTPUT); digitalWrite(FLASH_LED, LOW);

  camera_config_t c = {};
  c.ledc_channel = LEDC_CHANNEL_0; c.ledc_timer = LEDC_TIMER_0;
  c.pin_d0 = Y2_GPIO_NUM; c.pin_d1 = Y3_GPIO_NUM; c.pin_d2 = Y4_GPIO_NUM; c.pin_d3 = Y5_GPIO_NUM;
  c.pin_d4 = Y6_GPIO_NUM; c.pin_d5 = Y7_GPIO_NUM; c.pin_d6 = Y8_GPIO_NUM; c.pin_d7 = Y9_GPIO_NUM;
  c.pin_xclk = XCLK_GPIO_NUM; c.pin_pclk = PCLK_GPIO_NUM; c.pin_vsync = VSYNC_GPIO_NUM; c.pin_href = HREF_GPIO_NUM;
  c.pin_sccb_sda = SIOD_GPIO_NUM; c.pin_sccb_scl = SIOC_GPIO_NUM;
  c.pin_pwdn = PWDN_GPIO_NUM; c.pin_reset = RESET_GPIO_NUM;
  c.xclk_freq_hz = 20000000; c.pixel_format = PIXFORMAT_JPEG;
  c.frame_size = FRAMESIZE_QVGA;               // 320 x 240, the size the face pipeline works at
  c.jpeg_quality = 10;
  c.fb_count = psramFound() ? 2 : 1;
  c.fb_location = psramFound() ? CAMERA_FB_IN_PSRAM : CAMERA_FB_IN_DRAM;
  c.grab_mode = CAMERA_GRAB_LATEST;            // always the newest frame, no lag
  if (esp_camera_init(&c) != ESP_OK) { Serial.println("Camera init failed: check the ribbon cable and the 5 V supply"); delay(3000); ESP.restart(); }

  bool joined = false;
  if (strlen(WIFI_SSID)) {
    WiFi.mode(WIFI_STA); WiFi.begin(WIFI_SSID, WIFI_PASS);
    Serial.printf("Joining %s", WIFI_SSID);
    for (int i = 0; i < 30 && WiFi.status() != WL_CONNECTED; i++) { delay(500); Serial.print('.'); }
    joined = WiFi.status() == WL_CONNECTED;
    Serial.println();
  }
  if (joined) Serial.printf("Camera ready: http://%s/stream\n", WiFi.localIP().toString().c_str());
  else {
    WiFi.mode(WIFI_AP); WiFi.softAP(AP_SSID, AP_PASS);
    Serial.printf("Camera ready: join WiFi \"%s\" (password %s), then http://%s/stream\n", AP_SSID, AP_PASS, WiFi.softAPIP().toString().c_str());
  }
  WiFi.setSleep(false);
  start_server();
}

void loop() {
  static uint32_t t = 0;
  if (millis() - t > 10000) {                  // repeat the address for whoever opens the monitor late
    t = millis();
    Serial.printf("stream: http://%s/stream\n", (WiFi.getMode() == WIFI_AP ? WiFi.softAPIP() : WiFi.localIP()).toString().c_str());
  }
  delay(100);
}
