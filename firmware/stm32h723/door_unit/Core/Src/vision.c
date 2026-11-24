/* Image side of the face pipeline (see vision.h). Plain C: also built and tested on a PC. */
#include "vision.h"
#include <math.h>
#include <string.h>

/* RGB565 (high byte first) -> 8-bit RGB, same arithmetic as tools/capture.py */
static inline void px(const uint8_t *frame, int x, int y, int *r, int *g, int *b)
{
  const uint8_t *p = frame + 2 * (y * VIS_FRAME_W + x);
  uint32_t v = ((uint32_t)p[0] << 8) | p[1];
  *r = (int)(((v >> 11) & 0x1F) * 255 / 31);
  *g = (int)(((v >> 5) & 0x3F) * 255 / 63);
  *b = (int)((v & 0x1F) * 255 / 31);
}

void vis_det_input(const uint8_t *frame, int8_t *dst)
{
  for (int y = 0; y < VIS_DET_H; y++)
    for (int x = 0; x < VIS_DET_W; x++, dst += 3) {
      if (y >= VIS_FRAME_H / 2) { dst[0] = dst[1] = dst[2] = vis_q(0); continue; }
      int sr = 0, sg = 0, sb = 0, r, g, b;
      for (int dy = 0; dy < 2; dy++)
        for (int dx = 0; dx < 2; dx++) { px(frame, 2 * x + dx, 2 * y + dy, &r, &g, &b); sr += r; sg += g; sb += b; }
      dst[0] = vis_q((sr + 2) >> 2); dst[1] = vis_q((sg + 2) >> 2); dst[2] = vis_q((sb + 2) >> 2);
    }
}

static float iou(const float *a, const float *b)
{
  float ix = fminf(a[2], b[2]) - fmaxf(a[0], b[0]), iy = fminf(a[3], b[3]) - fmaxf(a[1], b[1]);
  if (ix <= 0 || iy <= 0) return 0;
  float i = ix * iy, u = (a[2] - a[0]) * (a[3] - a[1]) + (b[2] - b[0]) * (b[3] - b[1]) - i;
  return u > 0 ? i / u : 0;
}

int vis_det_decode(const vis_head_t *heads, int n_heads, float score_thresh, vis_face_t *out, int max_out)
{
  enum { MAXC = 64 };
  vis_face_t cand[MAXC]; int nc = 0;
  const float to_frame = (float)VIS_FRAME_W / VIS_DET_W;          /* detector pixels -> frame pixels */
  for (int s = 0; s < n_heads; s++) {
    const vis_head_t *sc = &heads[s];
    if (sc->c != 2) continue;
    const vis_head_t *bb = 0, *kp = 0;
    for (int k = 0; k < n_heads; k++)
      if (heads[k].h == sc->h) { if (heads[k].c == 8) bb = &heads[k]; if (heads[k].c == 20) kp = &heads[k]; }
    if (!bb || !kp) continue;
    const int stride = VIS_DET_H / sc->h;
    for (int y = 0; y < sc->h; y++)
      for (int x = 0; x < sc->w; x++)
        for (int a = 0; a < 2; a++) {
          float logit = (sc->data[(y * sc->w + x) * 2 + a] - sc->zero_point) * sc->scale;
          float prob = 1.0f / (1.0f + expf(-logit));
          if (prob < score_thresh || nc >= MAXC) continue;
          const int8_t *b = bb->data + (y * bb->w + x) * 8 + 4 * a;
          const int8_t *k = kp->data + (y * kp->w + x) * 20 + 10 * a;
          float cx = (float)(x * stride), cy = (float)(y * stride);
          vis_face_t *f = &cand[nc++];
          f->score = prob;
          f->box[0] = (cx - (b[0] - bb->zero_point) * bb->scale * stride) * to_frame;
          f->box[1] = (cy - (b[1] - bb->zero_point) * bb->scale * stride) * to_frame;
          f->box[2] = (cx + (b[2] - bb->zero_point) * bb->scale * stride) * to_frame;
          f->box[3] = (cy + (b[3] - bb->zero_point) * bb->scale * stride) * to_frame;
          for (int i = 0; i < 5; i++) {
            f->kps[i][0] = (cx + (k[2 * i] - kp->zero_point) * kp->scale * stride) * to_frame;
            f->kps[i][1] = (cy + (k[2 * i + 1] - kp->zero_point) * kp->scale * stride) * to_frame;
          }
        }
  }
  /* sort by score (insertion sort, few candidates), then NMS */
  for (int i = 1; i < nc; i++) { vis_face_t t = cand[i]; int j = i - 1; while (j >= 0 && cand[j].score < t.score) { cand[j + 1] = cand[j]; j--; } cand[j + 1] = t; }
  int n = 0;
  for (int i = 0; i < nc && n < max_out; i++) {
    int keep = 1;
    for (int j = 0; j < n; j++) if (iou(cand[i].box, out[j].box) >= 0.4f) { keep = 0; break; }
    if (keep) out[n++] = cand[i];
  }
  return n;
}

int vis_largest(const vis_face_t *f, int n)
{
  int best = -1; float area = 0;
  for (int i = 0; i < n; i++) {
    float a = (f[i].box[2] - f[i].box[0]) * (f[i].box[3] - f[i].box[1]);
    if (a > area) { area = a; best = i; }
  }
  return best;
}

/* bilinear sample at (fx, fy); outside pixels = border (0) or clamped (replicate) */
static void sample(const uint8_t *frame, float fx, float fy, int replicate, int *r, int *g, int *b)
{
  int x0 = (int)floorf(fx), y0 = (int)floorf(fy);
  float ax = fx - x0, ay = fy - y0;
  float acc[3] = {0, 0, 0};
  for (int dy = 0; dy < 2; dy++)
    for (int dx = 0; dx < 2; dx++) {
      int xx = x0 + dx, yy = y0 + dy;
      float w = (dx ? ax : 1 - ax) * (dy ? ay : 1 - ay);
      if (xx < 0 || yy < 0 || xx >= VIS_FRAME_W || yy >= VIS_FRAME_H) {
        if (!replicate) continue;
        xx = xx < 0 ? 0 : xx >= VIS_FRAME_W ? VIS_FRAME_W - 1 : xx;
        yy = yy < 0 ? 0 : yy >= VIS_FRAME_H ? VIS_FRAME_H - 1 : yy;
      }
      int pr, pg, pb; px(frame, xx, yy, &pr, &pg, &pb);
      acc[0] += w * pr; acc[1] += w * pg; acc[2] += w * pb;
    }
  *r = (int)(acc[0] + 0.5f); *g = (int)(acc[1] + 0.5f); *b = (int)(acc[2] + 0.5f);
}

/* insightface arcface_dst: where the 5 keypoints sit in a 112x112 aligned face */
static const float ARC_DST[5][2] = {{38.2946f, 51.6963f}, {73.5318f, 51.5014f}, {56.0252f, 71.7366f},
                                    {41.5493f, 92.3655f}, {70.7299f, 92.2041f}};

void vis_align112(const uint8_t *frame, const float kps[5][2], int8_t *dst)
{
  /* least-squares similarity transform kps -> ARC_DST: [u v] = [[c -s][s c]] [x y] + t */
  float mx = 0, my = 0, ux = 0, uy = 0;
  for (int i = 0; i < 5; i++) { mx += kps[i][0]; my += kps[i][1]; ux += ARC_DST[i][0]; uy += ARC_DST[i][1]; }
  mx /= 5; my /= 5; ux /= 5; uy /= 5;
  float sa = 0, sb = 0, var = 0;
  for (int i = 0; i < 5; i++) {
    float px_ = kps[i][0] - mx, py_ = kps[i][1] - my, qx = ARC_DST[i][0] - ux, qy = ARC_DST[i][1] - uy;
    sa += px_ * qx + py_ * qy; sb += px_ * qy - py_ * qx; var += px_ * px_ + py_ * py_;
  }
  float c = sa / var, s = sb / var, tx = ux - (c * mx - s * my), ty = uy - (s * mx + c * my);
  float d = c * c + s * s;                                     /* inverse: [x y] = R^T ([u v] - t) / d */
  for (int v = 0; v < 112; v++)
    for (int u = 0; u < 112; u++, dst += 3) {
      float du = u - tx, dv = v - ty;
      float fx = (c * du + s * dv) / d, fy = (-s * du + c * dv) / d;
      int r, g, b; sample(frame, fx, fy, 0, &r, &g, &b);
      dst[0] = vis_q(r); dst[1] = vis_q(g); dst[2] = vis_q(b);
    }
}

void vis_loose64(const uint8_t *frame, const float box[4], int8_t *dst)
{
  float cx = (box[0] + box[2]) / 2, cy = (box[1] + box[3]) / 2;
  float side = fmaxf(box[2] - box[0], box[3] - box[1]) * 1.6f, k = side / 128.0f;   /* frame px per crop px */
  for (int y = 0; y < 64; y++)
    for (int x = 0; x < 64; x++, dst += 3) {
      int sr = 0, sg = 0, sb = 0;
      for (int dy = 0; dy < 2; dy++)
        for (int dx = 0; dx < 2; dx++) {
          int u = 2 * x + dx, v = 2 * y + dy, r, g, b;
          sample(frame, cx + (u - 64) * k, cy + (v - 64) * k, 1, &r, &g, &b);
          sr += r; sg += g; sb += b;
        }
      dst[0] = vis_q((sr + 2) >> 2); dst[1] = vis_q((sg + 2) >> 2); dst[2] = vis_q((sb + 2) >> 2);
    }
}
