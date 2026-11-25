/* PC test of door.c + admin.c (the real files) against fakes: decisions, access windows, schedule, log,
 * unknown-face queue, admin commands, persistence across a restart. */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "door.h"
#include "admin.h"
#include "face_models.h"

extern uint32_t fake_ms; extern char uart_out[]; extern size_t uart_len, uart_bin;
extern int face_present, face_w, servo_deg, led_g, led_r; extern float spoof, cur_emb[FM_EMB];
extern uint8_t xf_sim[];
static int fails;
#define CHECK(c, msg) do { if (c) printf("  ok   %s\n", msg); else { printf("  FAIL %s\n", msg); fails++; } } while (0)

static void vec(float *e, int seed) { srand(seed); float s = 0; for (int i = 0; i < FM_EMB; i++) { e[i] = (float)rand() / RAND_MAX - 0.5f; s += e[i] * e[i]; } s = 1 / sqrtf(s); for (int i = 0; i < FM_EMB; i++) e[i] *= s; }
static void cmd(const char *c, int src) { char b[2048]; strncpy(b, c, sizeof b - 1); b[sizeof b - 1] = 0; door_command(b, src); }
static void frames(int n) { for (int i = 0; i < n; i++) { fake_ms += 800; door_process(0); door_poll(); } }
static void wait_ms(uint32_t ms) { for (uint32_t t = 0; t < ms; t += 100) { fake_ms += 100; door_poll(); } }
static void clear_uart(void) { uart_len = 0; uart_out[0] = 0; uart_bin = 0; }
static const char *b64(const int8_t *d, int n)
{
  static char o[1024]; const char *T = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"; int k = 0;
  for (int i = 0; i < n; i += 3) {
    uint32_t v = (uint8_t)d[i] << 16 | (i + 1 < n ? (uint8_t)d[i + 1] << 8 : 0) | (i + 2 < n ? (uint8_t)d[i + 2] : 0);
    o[k++] = T[v >> 18 & 63]; o[k++] = T[v >> 12 & 63]; o[k++] = i + 1 < n ? T[v >> 6 & 63] : '='; o[k++] = i + 2 < n ? T[v & 63] : '=';
  }
  o[k] = 0; return o;
}

int main(void)
{
  memset(xf_sim, 0xFF, 8 << 20);
  fake_ms = 1000;
  door_init();
  CHECK(strstr(uart_out, "LOG 1 0 BOOT"), "boot is logged as record 1, time unknown");

  /* 1. enrol Thinker from a known embedding via ADDB (as the dashboard would) */
  float a[FM_EMB]; vec(a, 1);
  int8_t q[FM_EMB]; float m = 0; for (int i = 0; i < FM_EMB; i++) m = fmaxf(m, fabsf(a[i]));
  for (int i = 0; i < FM_EMB; i++) q[i] = (int8_t)lrintf(a[i] * 127 / m);
  char line[1200]; snprintf(line, sizeof line, "ADDB Thinker 0 1439 127 %.8g %s", m / 127, b64(q, FM_EMB));
  clear_uart(); cmd(line, DOOR_SRC_UART);
  CHECK(strstr(uart_out, "PERSON Thinker 0 1439 127") && strstr(uart_out, "PEOPLEEND"), "ADDB stores the person and reports the list");
  CHECK(strstr(uart_out, "ENROL Thinker"), "ENROL logged");

  /* 2. Thinker at the door, time not set: granted (windows not enforced without a clock) */
  memcpy(cur_emb, a, sizeof a); clear_uart(); frames(3);
  CHECK(servo_deg == 90 && strstr(uart_out, "GRANT Thinker"), "enrolled face granted, door opens");
  wait_ms(6000); CHECK(servo_deg == 0, "door relocks after 5 s");

  /* 3. set the clock: Wednesday 25 Nov 2026 10:00 local; give Thinker Mon-Fri 08:00-17:00 */
  cmd("TIME 1795600800", DOOR_SRC_UART);
  CHECK(adm_weekday() == 2 && adm_minute() == 600, "clock: Wednesday 10:00");
  clear_uart(); cmd("WIN Thinker 480 1020 31", DOOR_SRC_UART);
  frames(3); CHECK(servo_deg == 90, "inside the window: granted"); wait_ms(6000);
  cmd("WIN Thinker 480 540 31", DOOR_SRC_UART);                 /* 08:00-09:00 only */
  clear_uart(); frames(3);
  CHECK(servo_deg == 0 && strstr(uart_out, "HOURS Thinker"), "outside the window: denied and logged HOURS");
  wait_ms(3000);
  cmd("WIN Thinker 600 1439 64", DOOR_SRC_UART);                /* Sunday only */
  clear_uart(); frames(3); CHECK(servo_deg == 0, "wrong day: denied"); wait_ms(3000);
  cmd("WIN Thinker 1320 360 4", DOOR_SRC_UART);                 /* Wed 22:00 - Thu 06:00 */
  cmd("TIME 1795652400", DOOR_SRC_UART);                        /* Thu 00:20 */
  clear_uart(); frames(3); CHECK(servo_deg == 90, "overnight window, after midnight: granted"); wait_ms(6000);
  cmd("WIN Thinker 0 1439 127", DOOR_SRC_UART);
  cmd("TIME 1795600800", DOOR_SRC_UART);

  /* 4. unknown face: denied, logged, face queued and sent when the link is up */
  float b[FM_EMB]; vec(b, 2); memcpy(cur_emb, b, sizeof b);
  clear_uart(); frames(3);
  CHECK(servo_deg == 0 && strstr(uart_out, "UNKNOWN"), "unknown face denied and logged");
  CHECK(adm_faces_waiting() == 1, "unknown face stored in the queue");
  CHECK(uart_bin == 0, "nothing sent while the link is down");
  wait_ms(3000);
  clear_uart(); cmd("LINK UP", DOOR_SRC_UART); wait_ms(6000);
  char *f = strstr(uart_out, "FACE ");
  CHECK(f && uart_bin == 112 * 112 * 3 + 4 + 512, "queued face sent over the link (38148 bytes)");
  unsigned long fid = f ? strtoul(f + 5, 0, 10) : 0;
  snprintf(line, sizeof line, "FACEACK %lu", fid); cmd(line, DOOR_SRC_UART);
  CHECK(adm_faces_waiting() == 0, "FACEACK marks it delivered");
  clear_uart(); wait_ms(12000); CHECK(uart_bin == 0, "delivered face not sent again");

  /* 5. admin: let in once, then add permanently using the stored embedding */
  snprintf(line, sizeof line, "ADMIT %lu", fid); clear_uart(); cmd(line, DOOR_SRC_UART);
  CHECK(servo_deg == 90 && strstr(uart_out, "ADMIT visitor#"), "ADMIT opens the door once");
  wait_ms(6000);
  fake_ms += 200000; snprintf(line, sizeof line, "ADMIT %lu", fid); cmd(line, DOOR_SRC_UART);
  CHECK(servo_deg == 0, "ADMIT after 2.5 minutes is refused");
  /* the face record holds scale + q: rebuild ADDB like the dashboard does from what the door sent */
  float mb = 0; for (int i = 0; i < FM_EMB; i++) mb = fmaxf(mb, fabsf(b[i]));
  int8_t qb[FM_EMB]; for (int i = 0; i < FM_EMB; i++) qb[i] = (int8_t)lrintf(b[i] * 127 / mb);
  snprintf(line, sizeof line, "ADDB Visitor_Ade 480 1080 127 %.8g %s", mb / 127, b64(qb, FM_EMB)); cmd(line, DOOR_SRC_UART);
  clear_uart(); frames(3); CHECK(servo_deg == 90 && strstr(uart_out, "GRANT Visitor_Ade"), "added visitor now granted by face");
  wait_ms(6000);

  /* 6. spoof: alarm + log, no grant */
  spoof = 0.9f; clear_uart(); frames(3); CHECK(servo_deg == 0 && strstr(uart_out, "SPOOF"), "spoof denied and logged"); spoof = 0; wait_ms(4000);

  /* 7. door schedule: closed now */
  cmd("SCHED 1 420 540 31", DOOR_SRC_UART);   /* 07:00-09:00 */
  memcpy(cur_emb, a, sizeof a); clear_uart(); frames(3);
  CHECK(servo_deg == 0 && !strstr(uart_out, "GRANT"), "outside opening hours nobody is let in");
  cmd("SCHED 0 420 540 31", DOOR_SRC_UART);

  /* 8. log replay and persistence across a restart */
  clear_uart(); cmd("LOGSINCE 0 100", DOOR_SRC_UART);
  int nlog = 0; for (char *p = uart_out; (p = strstr(p, "LOG ")); p++) nlog++;
  CHECK(strstr(uart_out, "LOGEND") && nlog >= 10, "LOGSINCE replays the log");
  clear_uart(); cmd("LOGSINCE 5 3", DOOR_SRC_UART);
  CHECK(strstr(uart_out, "LOG 6 ") && strstr(uart_out, "LOG 8 ") && !strstr(uart_out, "LOG 9 ") && strstr(uart_out, "LOGEND 3"), "LOGSINCE paging");
  clear_uart(); door_init();
  CHECK(strstr(uart_out, "PERSON Thinker") && strstr(uart_out, "PERSON Visitor_Ade 480 1080"), "people and windows survive a restart");
  char want[32]; snprintf(want, sizeof want, "LOG %d ", nlog + 1);
  CHECK(strstr(uart_out, want) && strstr(uart_out, "BOOT"), "log numbering continues after a restart");

  /* 9. log ring wraps past 4096 records */
  for (int i = 0; i < 5000; i++) adm_log(L_MOTION, "", 0);
  clear_uart(); door_init();
  char *p = strstr(uart_out, "LOG "); unsigned long s1 = p ? strtoul(p + 4, 0, 10) : 0;
  CHECK(s1 > 5000, "after wrapping, numbering still continues");
  clear_uart(); cmd("LOGSINCE 0 5", DOOR_SRC_UART); p = strstr(uart_out, "LOG ");
  CHECK(p && strtoul(p + 4, 0, 10) > s1 - 4096, "oldest records dropped, newest kept");

  printf(fails ? "\n%d FAILED\n" : "\nall passed\n", fails);
  return fails != 0;
}
