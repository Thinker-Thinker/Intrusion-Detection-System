/* Door logic (see door.h). */
#include "main.h"
#include "door.h"
#include "face_models.h"
#include "extflash.h"
#include "hwtest.h"
#include "usbd_cdc_if.h"
#include "selftest_data.h"
#include "admin.h"
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---------------------------------------------------------------- settings */
#define MIN_FACE_PX      48       /* face box width below this: "Come closer" (tested rule) */
#define ENROL_MIN_PX     60
#define SPOOF_P          0.5f     /* liveness: probability of photo/screen above this = spoof frame */
#define SPOOF_FRAMES     3
#define AVG_FRAMES       3        /* embeddings averaged before a decision */
#define DEFAULT_THRESH   0.42f    /* cosine; FAR ~1% on the test set (0.419 through this pipeline) */
#define OPEN_MS          5000
#define OPEN_DEG         90
#define CLOSED_DEG       0
#define DENY_HOLD_MS     2000
#define SPOOF_HOLD_MS    3000
#define ADMIT_VALID_MS   150000   /* "let in once" from the dashboard is honoured this long after the face */
#define ENROL_SAMPLES    10
#define ENROL_GAP_MS     300
#define ENROL_TIMEOUT_MS 60000
#define ENROL_SAME_COS   0.30f  /* a sample this far from the samples so far is someone else (or a bad frame) */
#define MAX_PEOPLE       32
#define NAME_LEN         24

/* ---------------------------------------------------------------- enrolled people */
/* access window: minutes after midnight (both ends included) and a day mask, bit 0 = Monday */
typedef struct { char name[NAME_LEN]; float scale; int8_t q[FM_EMB]; uint16_t start, end; uint8_t days, rsv[3]; } person_t;  /* 548 B */
typedef struct { char name[NAME_LEN]; float scale; int8_t q[FM_EMB]; } person_v1_t;                                          /* 540 B */
/* header of the people block; "sched" holds the door-wide schedule: on(1) start(11) end(11) days(7) */
typedef struct { uint32_t magic, count, crc, sched; } tpl_header_t;
#define TPL_MAGIC    0x324C5054u   /* "TPL2" */
#define TPL_MAGIC_V1 0x314C5054u   /* "TPL1": no access windows (read and converted) */
#define ALL_DAY_END  1439
#define ALL_DAYS     0x7F

static person_t people[MAX_PEOPLE];
static float person_k[MAX_PEOPLE];   /* 1 / |q| */
static int n_people;
static int sched_on, sched_start = 420, sched_end = 1140, sched_days = 0x1F;   /* door-wide opening hours */
static int8_t last_crop[112 * 112 * 3];        /* aligned face of the latest frame, kept for the administrator */

/* ---------------------------------------------------------------- state */
static float thresh = DEFAULT_THRESH, spoof_p = SPOOF_P;   /* SPOOF <p> changes it; 1 = liveness off */
static int debug_src, reed_alarm, snap_req, stream_on, selftest_src;

static float emb[FM_EMB], emb_sum[FM_EMB];
static int n_sum, spoof_run;
static float last_cx, last_cy, last_w;
static uint32_t last_face_t;

static uint32_t open_until, hold_until;

static char enrol_name[NAME_LEN];
static int enrol_goal, enrol_n, enrol_src;
static uint32_t enrol_t0, enrol_last;
static float enrol_sum[FM_EMB];

static char lcd_msg[32] = "Starting";
static uint16_t lcd_color = 0xFFFF, box_color;
static int box[4];
static uint32_t t_det, t_liv, t_rec, frame_ms, last_frame_t;

/* upload state */
static uint32_t up_len, up_crc, up_got, up_run_crc, up_last_t;
static int uploading;
static uint8_t chunk[4096];
static uint32_t chunk_need, chunk_got, chunk_off, chunk_crc;

#define C_WHITE  0xFFFF
#define C_GREEN  0x07E0
#define C_RED    0xF800
#define C_YELLOW 0xFFE0
#define C_CYAN   0x07FF

/* ---------------------------------------------------------------- output */
void door_reply(int src, const char *s)
{
  if (src == DOOR_SRC_UART) hw_uart_puts(s);
  else if (CDC_HostConnected()) CDC_SendBlocking((const uint8_t *)s, (uint32_t)strlen(s), 100);
}

static void event(const char *fmt, ...)
{
  char s[96];
  int k = snprintf(s, sizeof s, "EVT ");
  va_list ap; va_start(ap, fmt); vsnprintf(s + k, sizeof s - k - 1, fmt, ap); va_end(ap);
  strcat(s, "\n");
  door_reply(DOOR_SRC_UART, s);
  door_reply(DOOR_SRC_USB, s);
}

static void show(const char *msg, uint16_t color)
{
  strncpy(lcd_msg, msg, sizeof lcd_msg - 1);
  lcd_color = color;
}

/* ---------------------------------------------------------------- templates in external flash */
static void people_load(void)
{
  n_people = 0;
  if (!xf_ok()) return;
  const tpl_header_t *h = (const tpl_header_t *)XF_MAP(XF_TPL_OFF);
  const uint8_t *data = (const uint8_t *)(h + 1);
  if (h->count > MAX_PEOPLE) return;
  if (h->magic == TPL_MAGIC) {
    if (crc32_buf(0, data, h->count * sizeof(person_t)) != h->crc) return;
    memcpy(people, data, h->count * sizeof(person_t));
    if (h->sched != 0xFFFFFFFFu) {
      sched_on = h->sched >> 31; sched_start = (h->sched >> 20) & 0x7FF; sched_end = (h->sched >> 9) & 0x7FF; sched_days = (h->sched >> 2) & 0x7F;
    }
  } else if (h->magic == TPL_MAGIC_V1) {
    if (crc32_buf(0, data, h->count * sizeof(person_v1_t)) != h->crc) return;
    const person_v1_t *o = (const person_v1_t *)data;
    for (uint32_t p = 0; p < h->count; p++) {
      memcpy(people[p].name, o[p].name, NAME_LEN); people[p].scale = o[p].scale; memcpy(people[p].q, o[p].q, FM_EMB);
      people[p].start = 0; people[p].end = ALL_DAY_END; people[p].days = ALL_DAYS;
    }
  } else return;
  n_people = (int)h->count;
  for (int p = 0; p < n_people; p++) {
    float s = 0;
    for (int i = 0; i < FM_EMB; i++) s += (float)people[p].q[i] * people[p].q[i];
    person_k[p] = 1.0f / sqrtf(s + 1e-9f);
    people[p].name[NAME_LEN - 1] = 0;
  }
}

static int people_save(void)
{
  uint32_t sched = ((uint32_t)(sched_on & 1) << 31) | ((uint32_t)(sched_start & 0x7FF) << 20) |
                   ((uint32_t)(sched_end & 0x7FF) << 9) | ((uint32_t)(sched_days & 0x7F) << 2);
  tpl_header_t h = { TPL_MAGIC, (uint32_t)n_people, crc32_buf(0, (const uint8_t *)people, n_people * sizeof(person_t)), sched };
  int r = xf_write_begin();
  if (!r) r = xf_erase_blocks(XF_TPL_OFF, XF_BLOCK);
  if (!r) r = xf_program(XF_TPL_OFF, (const uint8_t *)&h, sizeof h);
  if (!r && n_people) r = xf_program(XF_TPL_OFF + sizeof h, (const uint8_t *)people, n_people * sizeof(person_t));
  if (xf_write_end_data()) r = -1;
  return r;
}

/* the people list and schedule, to the ESP32 (which forwards it to the dashboard) or to the PC */
static void send_people(int src)
{
  char s[80];
  for (int p = 0; p < n_people; p++) {
    snprintf(s, sizeof s, "PERSON %s %u %u %u\n", people[p].name, people[p].start, people[p].end, people[p].days);
    door_reply(src, s);
  }
  snprintf(s, sizeof s, "SCHED %d %d %d %d\nPEOPLEEND\n", sched_on, sched_start, sched_end, sched_days);
  door_reply(src, s);
}

static int door_closed_now(void)
{
  return sched_on && !adm_in_window(sched_start, sched_end, sched_days);
}

static int find_person(const char *name)
{
  for (int p = 0; p < n_people; p++) if (!strcmp(people[p].name, name)) return p;
  return -1;
}

static void set_person(int p, const char *name, const float *e)   /* e: unit vector */
{
  float m = 1e-9f;
  for (int i = 0; i < FM_EMB; i++) m = fmaxf(m, fabsf(e[i]));
  person_t *t = &people[p];
  memset(t->name, 0, NAME_LEN);
  strncpy(t->name, name, NAME_LEN - 1);
  t->scale = m / 127.0f;
  t->start = 0; t->end = ALL_DAY_END; t->days = ALL_DAYS;
  float s = 0;
  for (int i = 0; i < FM_EMB; i++) { int v = (int)lrintf(e[i] / t->scale); t->q[i] = (int8_t)v; s += (float)v * v; }
  person_k[p] = 1.0f / sqrtf(s + 1e-9f);
}

static float best_match(const float *e, int *who)
{
  float best = -1; *who = -1;
  for (int p = 0; p < n_people; p++) {
    float d = 0;
    for (int i = 0; i < FM_EMB; i++) d += e[i] * people[p].q[i];
    d *= person_k[p];
    if (d > best) { best = d; *who = p; }
  }
  return best;
}

static void normalize(float *v)
{
  float s = 0;
  for (int i = 0; i < FM_EMB; i++) s += v[i] * v[i];
  s = 1.0f / sqrtf(s + 1e-12f);
  for (int i = 0; i < FM_EMB; i++) v[i] *= s;
}

/* ---------------------------------------------------------------- door actions */
static void open_door(void)
{
  hw_servo_deg(OPEN_DEG);
  open_until = HAL_GetTick() + OPEN_MS;
}

static void reset_track(void) { n_sum = 0; spoof_run = 0; memset(emb_sum, 0, sizeof emb_sum); }

void door_init(void)
{
  adm_init();
  people_load();
  adm_log(L_BOOT, "", 0);
  send_people(DOOR_SRC_UART);
  hw_servo_deg(CLOSED_DEG);
  hw_led(0, 0);
  show(xf_rec_valid() ? "Ready" : "No face model", xf_rec_valid() ? C_WHITE : C_YELLOW);
}

int door_busy(void) { return uploading; }

int door_want_frame(void)
{
  if (snap_req) { snap_req = 0; return 1; }
  return stream_on;
}

const char *door_lcd_text(uint16_t *color) { *color = lcd_color; return lcd_msg; }

void door_face_box(int *x, int *y, int *w, int *h, uint16_t *color)
{
  *x = box[0]; *y = box[1]; *w = box[2]; *h = box[3]; *color = box_color;
}

/* ---------------------------------------------------------------- per frame */
static void enrol_step(const vis_face_t *f, float p_spoof)
{
  uint32_t now = HAL_GetTick();
  char s[48];
  if (now - enrol_t0 > ENROL_TIMEOUT_MS) {
    snprintf(s, sizeof s, "ERR enrol timeout (%d/%d samples)\n", enrol_n, enrol_goal);
    door_reply(enrol_src, s); enrol_goal = 0; show("Enrol failed", C_RED); return;
  }
  /* why a frame was skipped: shown on the LCD and sent to the PC every 2 s */
  static uint32_t why_t;
  char why[48] = "";
  if (!f) { show("Enrol: look here", C_CYAN); snprintf(why, sizeof why, "no face"); }
  else if (f->box[2] - f->box[0] < ENROL_MIN_PX) {
    show("Enrol: come closer", C_CYAN);
    snprintf(why, sizeof why, "face %d px wide, need %d: come closer", (int)(f->box[2] - f->box[0]), ENROL_MIN_PX);
  }
  else if (now - enrol_last < ENROL_GAP_MS) return;
  else {
    fm_embed(FM_CROP112, emb);
    if (enrol_n >= 3) {                       /* same person as the samples so far? */
      float d = 0, n = 0;
      for (int i = 0; i < FM_EMB; i++) { d += emb[i] * enrol_sum[i]; n += enrol_sum[i] * enrol_sum[i]; }
      float c = d / sqrtf(n + 1e-12f);
      if (c < ENROL_SAME_COS) {
        show("Enrol: hold still", C_YELLOW);
        snprintf(why, sizeof why, "frame unlike the others (%.2f): face the camera", c);
      }
    }
  }
  if (why[0]) {
    if (now - why_t > 2000) {
      char m[80]; why_t = now;
      snprintf(m, sizeof m, "ENROL %d/%d waiting: %s\n", enrol_n, enrol_goal, why);
      door_reply(enrol_src, m);
    }
    return;
  }
  for (int i = 0; i < FM_EMB; i++) enrol_sum[i] += emb[i];
  enrol_n++; enrol_last = now;
  hw_beep(30);
  snprintf(s, sizeof s, "Enrol %d/%d", enrol_n, enrol_goal); show(s, C_CYAN);
  if (enrol_n < enrol_goal) return;

  normalize(enrol_sum);
  int p = find_person(enrol_name);
  int existed = p >= 0;
  person_t keep = existed ? people[p] : people[0];
  if (p < 0) p = n_people < MAX_PEOPLE ? n_people++ : -1;
  if (p < 0) { door_reply(enrol_src, "ERR full\n"); enrol_goal = 0; return; }
  set_person(p, enrol_name, enrol_sum);
  if (existed) { people[p].start = keep.start; people[p].end = keep.end; people[p].days = keep.days; }
  int r = people_save();
  snprintf(s, sizeof s, r ? "ERR save failed\n" : "OK enrolled %s\n", enrol_name);
  door_reply(enrol_src, s);
  if (!r) { event("ENROLLED %s", enrol_name); adm_log(L_ENROL, enrol_name, 0); send_people(DOOR_SRC_UART); }
  show(r ? "Save failed" : "Enrolled", r ? C_RED : C_GREEN);
  hw_beep(r ? 600 : 150);
  enrol_goal = 0;
  hold_until = HAL_GetTick() + 1500;
}

/* Liveness + recognition on a built-in crop, compared with the PC's answer. Runs from door_process(),
 * when the camera DMA is idle (the models use the frame's RAM). */
static void run_selftest(int src)
{
  char s[160];
  uint32_t t0 = HAL_GetTick();
  memcpy(FM_CROP64, st_crop64, sizeof st_crop64);
  float ps = fm_spoof_prob(FM_CROP64);
  uint32_t t1 = HAL_GetTick();
  float c = -2;
  if (xf_rec_valid()) {
    memcpy(FM_CROP112, st_crop112, sizeof st_crop112);
    fm_embed(FM_CROP112, emb);
    c = 0;
    for (int i = 0; i < FM_EMB; i++) c += emb[i] * st_emb[i];
  }
  uint32_t t2 = HAL_GetTick();
  snprintf(s, sizeof s, "SELFTEST rec_cos=%.5f (want 1.0000) spoof=%.4f (want %.4f) liv=%lums rec=%lums %s\n",
           c, ps, ST_SPOOF, (unsigned long)(t1 - t0), (unsigned long)(t2 - t1),
           (c > 0.999f && fabsf(ps - ST_SPOOF) < 0.01f) ? "PASS" : "FAIL");
  door_reply(src, s);
}

void door_process(uint8_t *frame)
{
  if (selftest_src) { int src = selftest_src; selftest_src = 0; run_selftest(src); return; }
  uint32_t t0 = HAL_GetTick(), now = t0;
  frame_ms = now - last_frame_t; last_frame_t = now;
  if (!enrol_goal && !open_until && door_closed_now()) {   /* outside opening hours: the face models are not run */
    char m[32]; snprintf(m, sizeof m, "Closed - opens %02d:%02d", sched_start / 60, sched_start % 60);
    show(m, C_YELLOW); box[2] = 0; reset_track();
    return;
  }
  t_liv = t_rec = 0;

  vis_face_t faces[VIS_MAX_FACES];
  int n = fm_detect(frame, faces, VIS_MAX_FACES), b = vis_largest(faces, n);
  t_det = HAL_GetTick() - t0;
  const vis_face_t *f = b >= 0 ? &faces[b] : 0;
  box[2] = 0;
  if (f) {
    box[0] = (int)f->box[0]; box[1] = (int)f->box[1];
    box[2] = (int)(f->box[2] - f->box[0]); box[3] = (int)(f->box[3] - f->box[1]);
    box_color = C_WHITE;
  }

  /* same face as the last frames? (else start the average again) */
  if (f) {
    float cx = (f->box[0] + f->box[2]) / 2, cy = (f->box[1] + f->box[3]) / 2, w = f->box[2] - f->box[0];
    if (now - last_face_t > 1000 || fabsf(cx - last_cx) > 0.6f * last_w || fabsf(cy - last_cy) > 0.6f * last_w)
      reset_track();
    last_cx = cx; last_cy = cy; last_w = w; last_face_t = now;
  } else if (now - last_face_t > 1000) reset_track();

  float p_spoof = -1, score = -1; int who = -1;
  int busy_hold = (int32_t)(hold_until - now) > 0 || open_until;
  int big = f && (f->box[2] - f->box[0]) >= MIN_FACE_PX;

  if (f && big && (enrol_goal || !busy_hold)) {
    vis_align112(frame, f->kps, FM_CROP112);       /* crops first: the models below overwrite the frame */
    vis_loose64(frame, f->box, FM_CROP64);
    uint32_t t1 = HAL_GetTick();
    p_spoof = fm_spoof_prob(FM_CROP64);
    t_liv = HAL_GetTick() - t1;
  }

  if (enrol_goal) {
    enrol_step(f && big ? f : 0, p_spoof < 0 ? 0 : p_spoof);
    if (f && !big) show("Enrol: come closer", C_CYAN);
    goto done;
  }
  if (busy_hold) goto done;                         /* door open / showing a result: keep the message */

  if (!f) { show(n_people ? "Look at the camera" : "No one enrolled", C_WHITE); goto done; }
  if (!big) { show("Come closer", C_YELLOW); box_color = C_YELLOW; reset_track(); goto done; }

  if (p_spoof >= spoof_p) {
    box_color = C_RED; n_sum = 0; memset(emb_sum, 0, sizeof emb_sum);
    if (++spoof_run >= SPOOF_FRAMES) {
      show("SPOOF - photo/screen", C_RED);
      hw_led(0, 1); hw_alarm(SPOOF_HOLD_MS);
      event("SPOOF %.2f", p_spoof);
      adm_log(L_SPOOF, "", p_spoof);
      hold_until = now + SPOOF_HOLD_MS; spoof_run = 0;
    } else show("Checking...", C_YELLOW);
    goto done;
  }
  spoof_run = 0;
  if (!xf_rec_valid()) { show("Live face (no model)", C_YELLOW); goto done; }
  if (!n_people) { show("Live face - enrol someone", C_YELLOW); goto done; }

  {
    uint32_t t1 = HAL_GetTick();
    memcpy(last_crop, FM_CROP112, sizeof last_crop);   /* the model below overwrites the crop's RAM */
    fm_embed(FM_CROP112, emb);
    t_rec = HAL_GetTick() - t1;
  }
  for (int i = 0; i < FM_EMB; i++) emb_sum[i] += emb[i];
  if (++n_sum < AVG_FRAMES) { show("Hold still...", C_WHITE); goto done; }

  {
    float avg[FM_EMB];
    memcpy(avg, emb_sum, sizeof avg); normalize(avg);
    score = best_match(avg, &who);
    reset_track();
    if (who >= 0 && score >= thresh && !adm_in_window(people[who].start, people[who].end, people[who].days)) {
      show("Outside access hours", C_YELLOW); box_color = C_YELLOW;
      hw_led(0, 1); hw_beep(400);
      hold_until = now + DENY_HOLD_MS;
      event("HOURS %s %.3f", people[who].name, score);
      adm_log(L_HOURS, people[who].name, score);
    } else if (who >= 0 && score >= thresh) {
      char s[40]; snprintf(s, sizeof s, "Welcome %s", people[who].name);
      show(s, C_GREEN); box_color = C_GREEN;
      hw_led(1, 0); hw_beep(120); open_door();
      event("GRANT %s %.3f", people[who].name, score);
      adm_log(L_GRANT, people[who].name, score);
    } else {
      show("Access denied - admin told", C_RED); box_color = C_RED;
      hw_led(0, 1); hw_beep(400);
      hold_until = now + DENY_HOLD_MS;
      event("UNKNOWN %.3f%s%s", score, who >= 0 ? " closest=" : "", who >= 0 ? people[who].name : "");
      uint32_t id = adm_log(L_UNKNOWN, "", score);
      /* the administrator gets the face and its embedding (quantised like a stored person) */
      float m = 1e-9f;
      for (int i = 0; i < FM_EMB; i++) m = fmaxf(m, fabsf(avg[i]));
      static int8_t q[FM_EMB];
      for (int i = 0; i < FM_EMB; i++) q[i] = (int8_t)lrintf(avg[i] * 127.0f / m);
      adm_face(id, score, who >= 0 ? people[who].name : "", last_crop, m / 127.0f, q);
    }
  }

done:
  if (debug_src) {
    char s[160];
    snprintf(s, sizeof s, "DBG faces=%d w=%d spoof=%.2f n=%d score=%.3f who=%s det=%lums liv=%lums rec=%lums frame=%lums\n",
             n, box[2], p_spoof, n_sum, score, who >= 0 ? people[who].name : "-",
             (unsigned long)t_det, (unsigned long)t_liv, (unsigned long)t_rec, (unsigned long)frame_ms);
    door_reply(debug_src, s);
  }
}

/* ---------------------------------------------------------------- polling (timers, inputs, USB) */
static void upload_abort(const char *why)
{
  if (!uploading) return;
  uploading = 0; chunk_need = 0;
  xf_write_end();
  char s[64]; snprintf(s, sizeof s, "ERR upload aborted: %s\n", why); door_reply(DOOR_SRC_USB, s);
  show("Upload failed", C_RED);
}

static void chunk_done(void)
{
  char s[48];
  chunk_need = 0;
  up_last_t = HAL_GetTick();
  if (crc32_buf(0, chunk, chunk_got) != chunk_crc) { snprintf(s, sizeof s, "ERR crc %lu\n", (unsigned long)chunk_off); door_reply(DOOR_SRC_USB, s); return; }
  if (xf_program(chunk_off, chunk, chunk_got)) { upload_abort("flash program"); return; }
  up_run_crc = crc32_buf(up_run_crc, chunk, chunk_got);
  up_got += chunk_got;
  snprintf(s, sizeof s, "OK %lu\n", (unsigned long)up_got);
  door_reply(DOOR_SRC_USB, s);
  snprintf(s, sizeof s, "Model %lu%%", (unsigned long)(100ULL * up_got / up_len)); show(s, C_CYAN);
}

void door_poll(void)
{
  static char line[96];
  static int n;
  int c;
  while ((c = CDC_ReadByte()) >= 0) {
    if (chunk_need) {
      chunk[chunk_got++] = (uint8_t)c;
      if (chunk_got == chunk_need) chunk_done();
      continue;
    }
    if (c == '\r') continue;
    if (c == '\n') { line[n] = 0; n = 0; door_command(line, DOOR_SRC_USB); }
    else if (n < (int)sizeof line - 1) line[n++] = (char)c;
  }
  uint32_t now = HAL_GetTick();
  if (uploading && now - up_last_t > 15000) upload_abort("timeout");
  if (!uploading) adm_poll();

  if (open_until && (int32_t)(now - open_until) >= 0) {
    open_until = 0; hw_servo_deg(CLOSED_DEG); hw_led(0, 0);
    hold_until = now + 500;
  }
  if (hold_until && (int32_t)(now - hold_until) >= 0 && !open_until) { hold_until = 0; hw_led(0, 0); }

  /* exit button (debounced) */
  static uint32_t btn_t; static int btn_state;
  int b = hw_exit_pressed();
  if (b != btn_state && now - btn_t > 50) {
    btn_state = b; btn_t = now;
    if (b) { open_door(); hw_led(1, 0); hw_beep(60); show("Exit - door open", C_GREEN); event("EXIT"); adm_log(L_EXIT, "", 0); }
  }
  /* PIR: report motion, at most every 10 s */
  static uint32_t pir_t; static int pir_last;
  int pir = hw_pir();
  if (pir && !pir_last && now - pir_t > 10000) { pir_t = now; event("MOTION"); adm_log(L_MOTION, "", 0); }
  pir_last = pir;
  /* door switch: opened while locked (only when REED 1: the switch is optional) */
  static uint32_t closed_since; static int forced_sent;
  if (open_until) closed_since = now;
  if (reed_alarm && hw_door_open() && !open_until && now - closed_since > 2000) {
    if (!forced_sent) { forced_sent = 1; hw_alarm(5000); hw_led(0, 1); event("FORCED"); adm_log(L_FORCED, "", 0); show("DOOR FORCED", C_RED); }
  } else if (!hw_door_open()) forced_sent = 0;
}

/* ---------------------------------------------------------------- commands */
static void help(int src)
{
  door_reply(src, "Commands: INFO PEOPLE TIME <unix> WIN <name> <start> <end> <days> SCHED <on> <start> <end> <days> SELFTEST TEST SERVO <deg> BEEP LIST ENROL <name> [n] CANCEL DELETE <name>|ALL THRESH <x> SPOOF <p> OPEN DEBUG 0|1 "
             "REED 0|1 SNAP STREAM 0|1 AE +|- AUTO 0|1 (and PING STATUS TEST SERVO BEEP LED on the ESP32 link)\n");
}

void door_command(char *line, int src)
{
  char u[96], s[160];
  int i = 0;
  while (*line == ' ') line++;
  for (; line[i] && i < (int)sizeof u - 1; i++) u[i] = (line[i] >= 'a' && line[i] <= 'z') ? line[i] - 32 : line[i];
  u[i] = 0;
  const char *arg = strchr(line, ' ');
  arg = arg ? arg + 1 : "";
  while (*arg == ' ') arg++;
  if (!u[0]) return;
  if (uploading && strncmp(u, "DATA ", 5) && strcmp(u, "FLASHEND") && strcmp(u, "PING")) {
    door_reply(src, "ERR busy: model upload running\n"); return;
  }

  if (!strcmp(u, "HELP")) help(src);
  else if (!strcmp(u, "PING")) door_reply(src, "PONG door\n");
  else if (!strcmp(u, "INFO")) {
    snprintf(s, sizeof s, "INFO flash=%s id=0x%06lx model=%s (%lu bytes) people=%d thresh=%.3f det=%lums liv=%lums rec=%lums frame=%lums\n",
             xf_ok() ? "ok" : "FAIL", (unsigned long)xf_jedec_id(), xf_rec_valid() ? "ok" : "MISSING",
             (unsigned long)xf_rec_size(), n_people, thresh,
             (unsigned long)t_det, (unsigned long)t_liv, (unsigned long)t_rec, (unsigned long)frame_ms);
    door_reply(src, s);
    if (cam_command("INFO", s, sizeof s)) door_reply(src, s);
  }
  else if (!strcmp(u, "LIST")) {
    snprintf(s, sizeof s, "PEOPLE %d\n", n_people); door_reply(src, s);
    for (int p = 0; p < n_people; p++) {
      snprintf(s, sizeof s, "PERSON %d %s %02u:%02u-%02u:%02u days=0x%02x\n", p + 1, people[p].name, people[p].start / 60,
               people[p].start % 60, people[p].end / 60, people[p].end % 60, people[p].days);
      door_reply(src, s);
    }
  }
  else if (!strncmp(u, "ENROL ", 6) || !strncmp(u, "ENROLL ", 7)) {
    char name[NAME_LEN]; int k = 0;
    for (const char *p = arg; *p && *p != ' ' && k < NAME_LEN - 1; p++)
      if ((*p >= 'A' && *p <= 'Z') || (*p >= 'a' && *p <= 'z') || (*p >= '0' && *p <= '9') || *p == '_' || *p == '-' || *p == '.')
        name[k++] = *p;
    name[k] = 0;
    const char *ns = strchr(arg, ' ');
    int goal = ns ? atoi(ns + 1) : ENROL_SAMPLES;
    if (goal < 3) goal = 3;
    if (goal > 30) goal = 30;
    if (!k) door_reply(src, "ERR usage: ENROL <name> [samples]\n");
    else if (!xf_rec_valid()) door_reply(src, "ERR face model not in flash: run tools/door.py flash-model first\n");
    else if (find_person(name) < 0 && n_people >= MAX_PEOPLE) door_reply(src, "ERR full (32 people)\n");
    else {
      strcpy(enrol_name, name); enrol_goal = goal; enrol_n = 0; enrol_src = src;
      enrol_t0 = enrol_last = HAL_GetTick(); memset(enrol_sum, 0, sizeof enrol_sum);
      open_until = 0; hold_until = 0; hw_servo_deg(CLOSED_DEG); hw_led(0, 0);
      snprintf(s, sizeof s, "OK enrolling %s: look at the camera, move your head a little (%d samples)\n", name, goal);
      door_reply(src, s);
    }
  }
  else if (!strcmp(u, "TEST")) { hw_test_outputs(); door_reply(src, "OK testing: green LED, red LED, buzzer, servo 0-90-0 (about 4 s)\n"); }
  else if (!strncmp(u, "SERVO ", 6)) { hw_servo_deg(atoi(arg)); door_reply(src, "OK servo\n"); }
  else if (!strcmp(u, "BEEP")) { hw_beep(200); door_reply(src, "OK beep\n"); }
  else if (!strcmp(u, "SELFTEST")) { selftest_src = src; door_reply(src, "OK self-test runs on the next camera frame\n"); }
  else if (!strcmp(u, "CANCEL")) { enrol_goal = 0; door_reply(src, "OK\n"); show("Ready", C_WHITE); }
  else if (!strncmp(u, "DELETE ", 7)) {
    if (!strcmp(u + 7, "ALL")) n_people = 0;
    else {
      int p = find_person(arg);
      if (p < 0) { door_reply(src, "ERR no such person\n"); return; }
      for (; p < n_people - 1; p++) { people[p] = people[p + 1]; person_k[p] = person_k[p + 1]; }
      n_people--;
    }
    int r = people_save();
    door_reply(src, r ? "ERR save failed\n" : "OK deleted\n");
    if (!r) { adm_log(L_DELETE, arg, 0); send_people(DOOR_SRC_UART); }
  }
  else if (!strncmp(u, "SPOOF", 5)) {
    if (*arg) { float t = strtof(arg, 0); if (t > 0) spoof_p = t; }
    snprintf(s, sizeof s, spoof_p >= 1 ? "OK liveness check off\n" : "OK spoof threshold=%.2f\n", spoof_p); door_reply(src, s);
  }
  else if (!strncmp(u, "THRESH", 6)) {
    if (*arg) { float t = strtof(arg, 0); if (t > 0 && t < 1) thresh = t; }
    snprintf(s, sizeof s, "OK thresh=%.3f\n", thresh); door_reply(src, s);
  }
  else if (!strcmp(u, "OPEN")) { open_door(); hw_led(1, 0); show("Opened remotely", C_GREEN); event("REMOTE_OPEN"); adm_log(L_OPEN, "", 0); door_reply(src, "OK\n"); }
  /* ---- administrator (dashboard via the ESP32, or the PC) */
  else if (!strcmp(u, "PEOPLE")) send_people(src);
  else if (!strncmp(u, "ADMIT ", 6)) {
    uint32_t id = (uint32_t)strtoul(arg, 0, 10);
    if (!adm_face_recent(id, ADMIT_VALID_MS)) { door_reply(src, "ERR admit expired\n"); return; }
    open_door(); hw_led(1, 0); hw_beep(120); show("Welcome - admin let in", C_GREEN);
    char nm[20]; snprintf(nm, sizeof nm, "visitor#%lu", (unsigned long)id);
    event("ADMIT %s", nm); adm_log(L_ADMIT, nm, 0); door_reply(src, "OK admitted\n");
  }
  else if (!strncmp(u, "ADDB ", 5) || !strncmp(u, "WIN ", 4)) {
    /* ADDB <name> <start> <end> <days> <scale> <base64 512 int8>  |  WIN <name> <start> <end> <days> */
    char name[NAME_LEN] = ""; int st = 0, en = ALL_DAY_END, dy = ALL_DAYS, k = 0; float sc = 0;
    const char *p = arg;
    while (*p && *p != ' ' && k < NAME_LEN - 1) name[k++] = *p++;
    name[k] = 0;
    char *e; st = (int)strtol(p, &e, 10); en = (int)strtol(e, &e, 10); dy = (int)strtol(e, &e, 10);
    if (!k || st < 0 || st > 1440 || en < 0 || en > 1440 || dy < 0 || dy > 0x7F) { door_reply(src, "ERR usage\n"); return; }
    int idx = find_person(name);
    if (u[0] == 'W') {
      if (idx < 0) { door_reply(src, "ERR no such person\n"); return; }
    } else {
      sc = strtof(e, &e);
      while (*e == ' ') e++;
      static uint8_t qb[FM_EMB + 4]; int nq = 0;
      adm_base64_decode(e, qb, sizeof qb, &nq);
      if (nq != FM_EMB || sc <= 0) { door_reply(src, "ERR bad embedding\n"); return; }
      if (idx < 0) { if (n_people >= MAX_PEOPLE) { door_reply(src, "ERR full\n"); return; } idx = n_people++; }
      person_t *t = &people[idx];
      memset(t->name, 0, NAME_LEN); strncpy(t->name, name, NAME_LEN - 1);
      t->scale = sc; memcpy(t->q, qb, FM_EMB);
      float ss = 0; for (int i = 0; i < FM_EMB; i++) ss += (float)t->q[i] * t->q[i];
      person_k[idx] = 1.0f / sqrtf(ss + 1e-9f);
    }
    people[idx].start = (uint16_t)(st > ALL_DAY_END ? ALL_DAY_END : st);
    people[idx].end = (uint16_t)(en > ALL_DAY_END ? ALL_DAY_END : en);
    people[idx].days = (uint8_t)dy;
    int r = people_save();
    door_reply(src, r ? "ERR save failed\n" : "OK\n");
    if (!r) { if (u[0] == 'A') { event("ENROLLED %s", name); adm_log(L_ENROL, name, 0); } send_people(DOOR_SRC_UART); }
  }
  else if (!strncmp(u, "SCHED", 5)) {
    if (*arg) {
      char *e; int on = (int)strtol(arg, &e, 10), st = (int)strtol(e, &e, 10), en = (int)strtol(e, &e, 10), dy = (int)strtol(e, &e, 10);
      if (st < 0 || st > 1440 || en < 0 || en > 1440 || dy < 0 || dy > 0x7F) { door_reply(src, "ERR usage: SCHED on start end days\n"); return; }
      sched_on = on != 0; sched_start = st > ALL_DAY_END ? ALL_DAY_END : st; sched_end = en > ALL_DAY_END ? ALL_DAY_END : en; sched_days = dy;
      people_save();
    }
    send_people(src);
  }
  else if (adm_command(u, arg, src)) {}
  else if (!strncmp(u, "DEBUG", 5)) { debug_src = atoi(arg) ? src : 0; door_reply(src, "OK\n"); }
  else if (!strncmp(u, "REED", 4)) { reed_alarm = atoi(arg) != 0; door_reply(src, reed_alarm ? "OK door switch alarm on\n" : "OK door switch alarm off\n"); }
  else if (!strcmp(u, "SNAP") && src == DOOR_SRC_USB) snap_req = 1;
  else if (!strncmp(u, "STREAM", 6) && src == DOOR_SRC_USB) stream_on = atoi(arg) != 0;
  /* ---- model upload (USB) */
  else if (!strncmp(u, "FLASHREC ", 9) && src == DOOR_SRC_USB) {
    unsigned long len = 0, crc = 0;
    sscanf(arg, "%lu %lx", &len, &crc);
    if (len != xf_rec_size()) { snprintf(s, sizeof s, "ERR size %lu: this firmware expects %lu\n", len, (unsigned long)xf_rec_size()); door_reply(src, s); return; }
    enrol_goal = 0;
    show("Erasing flash...", C_CYAN);
    if (xf_write_begin() || xf_erase_blocks(0, len) || xf_erase_blocks(XF_HDR_OFF, XF_BLOCK)) {
      xf_write_end(); door_reply(src, "ERR erase\n"); return;
    }
    uploading = 1; up_len = len; up_crc = crc; up_got = 0; up_run_crc = 0; up_last_t = HAL_GetTick();
    door_reply(src, "READY\n");
  }
  else if (!strncmp(u, "DATA ", 5) && src == DOOR_SRC_USB) {
    unsigned long off = 0, cnt = 0, crc = 0;
    sscanf(arg, "%lu %lu %lx", &off, &cnt, &crc);
    if (!uploading || off != up_got || cnt == 0 || cnt > sizeof chunk || off + cnt > up_len) {
      snprintf(s, sizeof s, "ERR bad DATA (expect offset %lu)\n", (unsigned long)up_got); door_reply(src, s);
      /* the raw bytes that follow are still read (as a chunk) so the stream stays in step */
    }
    chunk_off = off; chunk_need = cnt > sizeof chunk ? sizeof chunk : cnt; chunk_got = 0; chunk_crc = crc;
    if (!uploading || off != up_got) chunk_crc = ~crc;          /* force a CRC error: nothing is written */
    up_last_t = HAL_GetTick();
  }
  else if (!strcmp(u, "FLASHEND") && src == DOOR_SRC_USB) {
    if (!uploading) { door_reply(src, "ERR no upload\n"); return; }
    if (up_got != up_len || up_run_crc != up_crc) { upload_abort("length or CRC mismatch"); return; }
    xf_header_t h = { XF_MAGIC_REC, up_len, up_crc, 0 };
    int r = xf_program(XF_HDR_OFF, (const uint8_t *)&h, sizeof h);
    uploading = 0;
    xf_write_end();                                             /* re-maps and re-checks the CRC */
    if (r || !xf_rec_valid()) { door_reply(src, "ERR verify failed\n"); show("Model verify failed", C_RED); }
    else { door_reply(src, "REC OK\n"); show("Model OK", C_GREEN); hw_beep(150); }
  }
  else if (cam_command(u, s, sizeof s)) door_reply(src, s);
  else { snprintf(s, sizeof s, "?? %s (send HELP)\n", line); door_reply(src, s); }
}
