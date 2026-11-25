/* Door administration support (see admin.h). */
#include "main.h"
#include "admin.h"
#include "door.h"
#include "extflash.h"
#include "hwtest.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

static const char *TYPE_NAME[L_NTYPES] = {"BOOT", "GRANT", "UNKNOWN", "SPOOF", "HOURS", "CLOSED", "EXIT", "ADMIT",
                                          "ENROL", "DELETE", "OPEN", "MOTION", "FORCED"};

/* ---------------------------------------------------------------- clock */
static uint32_t clk_base;          /* local unix seconds at tick 0; 0 = not set */
static uint32_t clk_ms_base;

uint32_t adm_now(void)
{
  if (!clk_base) return 0;
  return clk_base + (HAL_GetTick() - clk_ms_base) / 1000;
}
int adm_minute(void) { uint32_t t = adm_now(); return t ? (int)((t % 86400) / 60) : -1; }
int adm_weekday(void) { uint32_t t = adm_now(); return t ? (int)((t / 86400 + 3) % 7) : -1; }   /* 1 Jan 1970 was a Thursday */

int adm_in_window(int start, int end, int days)
{
  int m = adm_minute(), d = adm_weekday();
  if (m < 0) return 1;                              /* time not set yet: do not lock people out */
  if (start <= end) return ((days >> d) & 1) && m >= start && m <= end;
  /* overnight window (e.g. 22:00-06:00): the part after midnight belongs to the previous day's window */
  if (m >= start) return (days >> d) & 1;
  return (days >> ((d + 6) % 7)) & 1 && m <= end;
}

static void set_time(uint32_t t)
{
  clk_ms_base = HAL_GetTick();
  clk_base = t;
}

/* ---------------------------------------------------------------- link state */
static uint32_t link_t;            /* tick of the last LINK UP */
int adm_link_up(void) { return link_t && HAL_GetTick() - link_t < 30000; }

/* ---------------------------------------------------------------- access log: ring of 32-byte records */
typedef struct { uint32_t seq, time; uint8_t type, rsv; int16_t score_milli; char name[20]; } log_rec_t;
#define LOG_N    ((2 * XF_BLOCK) / sizeof(log_rec_t))      /* 4096 */
#define LOG_PER_BLOCK (XF_BLOCK / sizeof(log_rec_t))
static uint32_t next_seq = 1;

static const log_rec_t *log_at(uint32_t seq) { return (const log_rec_t *)XF_MAP(XF_LOG_OFF + (seq % LOG_N) * sizeof(log_rec_t)); }

static void send_log_line(const log_rec_t *r, int src)
{
  char s[80];
  snprintf(s, sizeof s, "LOG %lu %lu %s %s %.3f\n", (unsigned long)r->seq, (unsigned long)r->time,
           r->type < L_NTYPES ? TYPE_NAME[r->type] : "?", r->name[0] ? r->name : "-", r->score_milli / 1000.0f);
  if (src == DOOR_SRC_UART) hw_uart_puts(s); else door_reply(src, s);
}

uint32_t adm_log(int type, const char *name, float score)
{
  log_rec_t r;
  memset(&r, 0, sizeof r);
  r.seq = next_seq++;
  r.time = adm_now();
  r.type = (uint8_t)type;
  r.score_milli = (int16_t)(score * 1000);
  if (name) strncpy(r.name, name, sizeof r.name - 1);
  if (xf_ok() && !xf_writing()) {
    uint32_t idx = r.seq % LOG_N, off = XF_LOG_OFF + idx * sizeof r;
    if (!xf_write_begin()) {
      if (idx % LOG_PER_BLOCK == 0) xf_erase_blocks(off, XF_BLOCK);     /* entering a block: drop its 2048 oldest */
      xf_program(off, (const uint8_t *)&r, sizeof r);
    }
    xf_write_end_data();
  }
  send_log_line(&r, DOOR_SRC_UART);                             /* the ESP32 forwards it when it can; LOGSINCE fills any gap */
  return r.seq;
}

static void log_scan(void)
{
  uint32_t best = 0;
  for (uint32_t i = 0; i < LOG_N; i++) {
    const log_rec_t *r = (const log_rec_t *)XF_MAP(XF_LOG_OFF + i * sizeof(log_rec_t));
    if (r->seq != 0xFFFFFFFFu && r->seq % LOG_N == i && r->seq > best) best = r->seq;
  }
  next_seq = best + 1;
}

static void log_since(uint32_t after, uint32_t max, int src)
{
  uint32_t first = after + 1, n = 0;
  if (next_seq > LOG_N && first < next_seq - LOG_N + LOG_PER_BLOCK) first = next_seq - LOG_N + LOG_PER_BLOCK;  /* oldest kept */
  for (uint32_t s = first; s < next_seq && n < max; s++) {
    const log_rec_t *r = log_at(s);
    if (r->seq == s) { send_log_line(r, src); n++; }
  }
  char m[24]; snprintf(m, sizeof m, "LOGEND %lu\n", (unsigned long)n);
  if (src == DOOR_SRC_UART) hw_uart_puts(m); else door_reply(src, m);
}

/* ---------------------------------------------------------------- unknown-face queue (16 slots of 64 KB) */
#define Q_SLOTS 16
#define Q_MAGIC 0x45434146u                         /* "FACE" */
#define CROP_BYTES (112 * 112 * 3)
typedef struct { uint32_t magic, id, time; float score, scale; char closest[24]; uint32_t rsv[5]; uint32_t sent; } q_head_t;  /* 64 B */
#define Q_PAYLOAD (CROP_BYTES + 4 + 512)

static const q_head_t *q_at(int k) { return (const q_head_t *)XF_MAP(XF_QUEUE_OFF + k * XF_BLOCK); }
static uint32_t waiting_id, waiting_t, retry_t;
static struct { uint32_t id, tick; } recent[8];
static int recent_i;

int adm_faces_waiting(void)
{
  int n = 0;
  for (int k = 0; k < Q_SLOTS; k++) if (q_at(k)->magic == Q_MAGIC && q_at(k)->sent == 0xFFFFFFFFu) n++;
  return n;
}

int adm_face_recent(uint32_t id, uint32_t max_age_ms)
{
  for (int i = 0; i < 8; i++) if (recent[i].id == id && HAL_GetTick() - recent[i].tick < max_age_ms) return 1;
  return 0;
}

void adm_face(uint32_t id, float score, const char *closest, const int8_t *crop112, float scale, const int8_t *q512)
{
  recent[recent_i].id = id; recent[recent_i].tick = HAL_GetTick(); recent_i = (recent_i + 1) % 8;
  if (!xf_ok()) return;
  /* a free slot (erased, or already delivered); otherwise overwrite the oldest one still waiting */
  int k, best = -1;
  for (k = 0; k < Q_SLOTS && best < 0; k++) if (q_at(k)->magic == 0xFFFFFFFFu) best = k;
  for (k = 0; k < Q_SLOTS && best < 0; k++) if (q_at(k)->magic != Q_MAGIC || q_at(k)->sent != 0xFFFFFFFFu) best = k;
  if (best < 0) {
    uint32_t oldest = 0xFFFFFFFFu;
    for (k = 0; k < Q_SLOTS; k++) if (q_at(k)->id < oldest) { oldest = q_at(k)->id; best = k; }
  }
  q_head_t h;
  memset(&h, 0xFF, sizeof h);
  h.magic = Q_MAGIC; h.id = id; h.time = adm_now(); h.score = score; h.scale = scale;
  memset(h.closest, 0, sizeof h.closest);
  if (closest) strncpy(h.closest, closest, sizeof h.closest - 1);
  uint32_t off = XF_QUEUE_OFF + best * XF_BLOCK;
  int erased = q_at(best)->magic == 0xFFFFFFFFu;
  if (!xf_write_begin()) {
    if (!erased) xf_erase_blocks(off, XF_BLOCK);
    xf_program(off + sizeof h, (const uint8_t *)crop112, CROP_BYTES);
    xf_program(off + sizeof h + CROP_BYTES, (const uint8_t *)&scale, 4);
    xf_program(off + sizeof h + CROP_BYTES + 4, (const uint8_t *)q512, 512);
    xf_program(off, (const uint8_t *)&h, sizeof h);    /* header last: a slot is valid only when complete */
  }
  xf_write_end_data();
}

static void send_slot(int k)
{
  const q_head_t *h = q_at(k);
  const uint8_t *p = (const uint8_t *)h + sizeof(q_head_t);
  char s[120];
  snprintf(s, sizeof s, "FACE %lu %lu %.3f %s %u %08lx\n", (unsigned long)h->id, (unsigned long)h->time, h->score,
           h->closest[0] ? h->closest : "-", (unsigned)Q_PAYLOAD, (unsigned long)crc32_buf(0, p, Q_PAYLOAD));
  hw_uart_puts(s);
  hw_uart_write(p, Q_PAYLOAD);
  waiting_id = h->id; waiting_t = HAL_GetTick();
}

static void mark_sent(uint32_t id)
{
  for (int k = 0; k < Q_SLOTS; k++) {
    const q_head_t *h = q_at(k);
    if (h->magic == Q_MAGIC && h->id == id && h->sent == 0xFFFFFFFFu) {
      uint32_t zero = 0, off = XF_QUEUE_OFF + k * XF_BLOCK + offsetof(q_head_t, sent);
      if (!xf_write_begin()) xf_program(off, (const uint8_t *)&zero, 4);    /* 1 -> 0 bits: no erase needed */
      xf_write_end_data();
    }
  }
}

void adm_poll(void)
{
  uint32_t now = HAL_GetTick();
  if (waiting_id && now - waiting_t > 10000) { waiting_id = 0; retry_t = now; }   /* no answer: try again later */
  if (waiting_id || !adm_link_up() || !xf_ok() || xf_writing() || now - retry_t < 5000) return;
  int best = -1; uint32_t lo = 0xFFFFFFFFu;
  for (int k = 0; k < Q_SLOTS; k++) {
    const q_head_t *h = q_at(k);
    if (h->magic == Q_MAGIC && h->sent == 0xFFFFFFFFu && h->id < lo) { lo = h->id; best = k; }
  }
  if (best >= 0) send_slot(best);
}

/* ---------------------------------------------------------------- commands from the ESP32 (or USB) */
int adm_command(const char *u, const char *arg, int src)
{
  (void)src;
  if (!strncmp(u, "TIME", 4)) {
    if (*arg) set_time((uint32_t)strtoul(arg, 0, 10));
    char s[48]; int m = adm_minute();
    snprintf(s, sizeof s, "OK time %02d:%02d day %d\n", m < 0 ? 0 : m / 60, m < 0 ? 0 : m % 60, adm_weekday());
    if (src == DOOR_SRC_USB) door_reply(src, s);
    return 1;
  }
  if (!strcmp(u, "LINK UP")) { link_t = HAL_GetTick(); if (!link_t) link_t = 1; return 1; }
  if (!strcmp(u, "LINK DOWN")) { link_t = 0; waiting_id = 0; return 1; }
  if (!strncmp(u, "FACEACK ", 8)) {
    uint32_t id = (uint32_t)strtoul(arg, 0, 10);
    mark_sent(id);
    if (id == waiting_id) waiting_id = 0;
    return 1;
  }
  if (!strncmp(u, "LOGSINCE", 8)) {
    char *e; uint32_t after = (uint32_t)strtoul(arg, &e, 10), max = (uint32_t)strtoul(e, 0, 10);
    log_since(after, max ? max : 40, src);
    return 1;
  }
  return 0;
}

void adm_init(void)
{
  if (xf_ok()) log_scan();
}

/* ---------------------------------------------------------------- base64 (embeddings sent by the dashboard) */
void adm_base64_decode(const char *in, uint8_t *out, int max, int *n)
{
  uint32_t acc = 0; int bits = 0; *n = 0;
  for (; *in && *in != ' '; in++) {
    int v;
    char c = *in;
    if (c >= 'A' && c <= 'Z') v = c - 'A';
    else if (c >= 'a' && c <= 'z') v = c - 'a' + 26;
    else if (c >= '0' && c <= '9') v = c - '0' + 52;
    else if (c == '+') v = 62;
    else if (c == '/') v = 63;
    else continue;                                  /* '=' padding and anything else */
    acc = (acc << 6) | (uint32_t)v; bits += 6;
    if (bits >= 8) { bits -= 8; if (*n < max) out[(*n)++] = (uint8_t)(acc >> bits); }
  }
}
