/* Fakes for the PC test of the door logic: clock, flash (with real NOR semantics), UART, USB, models. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "extflash.h"
#include "face_models.h"
#include "hwtest.h"

uint32_t fake_ms;
uint32_t HAL_GetTick(void) { return fake_ms; }

/* ---- external flash: 8 MB, program can only clear bits, erase sets 64 KB to 0xFF */
uint8_t xf_sim[8 << 20];
static int writing, valid = 1;
int xf_ok(void) { return 1; }
int xf_rec_valid(void) { return valid && !writing; }
uint32_t xf_rec_size(void) { return 408512; }
uint32_t xf_jedec_id(void) { return 0x852017; }
int xf_writing(void) { return writing; }
int xf_write_begin(void) { if (writing) { printf("BUG nested write\n"); exit(1); } writing = 1; return 0; }
int xf_erase_blocks(uint32_t off, uint32_t len) { for (uint32_t a = off & ~0xFFFFu; a < off + len; a += 0x10000) memset(xf_sim + a, 0xFF, 0x10000); return 0; }
int xf_program(uint32_t off, const uint8_t *d, uint32_t n) { if (!writing) { printf("BUG program outside write\n"); exit(1); } for (uint32_t i = 0; i < n; i++) xf_sim[off + i] &= d[i]; return 0; }
int xf_write_end(void) { writing = 0; return 0; }
int xf_write_end_data(void) { writing = 0; return 0; }
uint32_t crc32_buf(uint32_t crc, const uint8_t *p, uint32_t n)
{ crc = ~crc; while (n--) { crc ^= *p++; for (int k = 0; k < 8; k++) crc = (crc >> 1) ^ (0xEDB88320u & -(crc & 1)); } return ~crc; }

/* ---- UART to the ESP32: captured */
char uart_out[1 << 20]; size_t uart_len; size_t uart_bin;
void hw_uart_puts(const char *s) { size_t n = strlen(s); if (uart_len + n < sizeof uart_out) { memcpy(uart_out + uart_len, s, n); uart_len += n; uart_out[uart_len] = 0; } }
void hw_uart_write(const uint8_t *p, uint32_t n) { (void)p; uart_bin += n; }
int servo_deg; int led_g, led_r;
void hw_servo_deg(int d) { servo_deg = d; }
void hw_led(int g, int r) { led_g = g; led_r = r; }
void hw_beep(uint32_t ms) { (void)ms; }
void hw_alarm(uint32_t ms) { (void)ms; }
int hw_pir(void) { return 0; }
int hw_exit_pressed(void) { return 0; }
int hw_door_open(void) { return 0; }
void hw_test_outputs(void) {}

/* ---- USB: replies printed */
int CDC_HostConnected(void) { return 1; }
int CDC_SendBlocking(const uint8_t *p, uint32_t n, uint32_t t) { (void)t; fwrite(p, 1, n, stdout); return 0; }
int CDC_ReadByte(void) { return -1; }
int cam_command(const char *u, char *out, int n) { (void)u; (void)out; (void)n; return 0; }

/* ---- models: the test sets what the "camera" sees */
signed char axi_pool[327680];
int face_present = 1, face_w = 80; float spoof = 0.0f; float cur_emb[FM_EMB];
int fm_detect(const uint8_t *frame, vis_face_t *f, int max)
{ (void)frame; (void)max; if (!face_present) return 0; memset(f, 0, sizeof *f); f->score = 0.9f; f->box[0] = 100; f->box[1] = 60; f->box[2] = 100 + face_w; f->box[3] = 60 + face_w; return 1; }
int vis_largest(const vis_face_t *f, int n) { (void)f; return n ? 0 : -1; }
void vis_align112(const uint8_t *fr, const float k[5][2], int8_t *d) { (void)fr; (void)k; memset(d, 7, 112 * 112 * 3); }
void vis_loose64(const uint8_t *fr, const float b[4], int8_t *d) { (void)fr; (void)b; memset(d, 0, 64 * 64 * 3); }
float fm_spoof_prob(const int8_t *c) { (void)c; return spoof; }
void fm_embed(const int8_t *c, float *e) { (void)c; memcpy(e, cur_emb, sizeof cur_emb); }
