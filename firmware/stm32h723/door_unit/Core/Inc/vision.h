/* Image side of the face pipeline: plain C, no HAL, so the same code is tested on a PC
 * (models/pcsim/run_pipeline.c) before it runs on the STM32.
 *
 * Frame: 320x240 RGB565 as the DCMI stores it (high byte first), exactly what tools/capture.py saves. */
#ifndef VISION_H
#define VISION_H
#include <stdint.h>

#define VIS_FRAME_W 320
#define VIS_FRAME_H 240
#define VIS_DET_W   160          /* detector input: frame / 2, padded to 160x128 */
#define VIS_DET_H   128
#define VIS_MAX_FACES 8

typedef struct { int h, w, c; const int8_t *data; float scale; int zero_point; } vis_head_t;

typedef struct {
  float score;
  float box[4];        /* x1, y1, x2, y2 in frame pixels */
  float kps[5][2];     /* left eye, right eye, nose, left mouth, right mouth (frame pixels) */
} vis_face_t;

/* 320x240 RGB565 -> 160x128x3 int8 detector input (2x2 mean, rows 120..127 zero padding). */
void vis_det_input(const uint8_t *frame, int8_t *dst);

/* Decode SCRFD's 9 heads (3 strides x score/box/keypoints, any order). Returns faces sorted by score,
 * after non-maximum suppression, scaled to frame pixels. */
int vis_det_decode(const vis_head_t *heads, int n_heads, float score_thresh, vis_face_t *out, int max_out);

/* Largest face by box area, or -1. */
int vis_largest(const vis_face_t *f, int n);

/* ArcFace-aligned 112x112x3 int8 crop (similarity transform from the 5 keypoints, bilinear, border 0).
 * Same geometry as insightface norm_crop, which made the training and test crops. */
void vis_align112(const uint8_t *frame, const float kps[5][2], int8_t *dst);

/* Liveness crop: square 1.6x the face box, 128x128 bilinear (edges replicated), 2x2 mean -> 64x64x3 int8. */
void vis_loose64(const uint8_t *frame, const float box[4], int8_t *dst);

/* uint8 0..255 -> model int8 input (scale 1/127.5 or 1/128, zero point -1: q = round(x - 127.5) - 1). */
static inline int8_t vis_q(int v)
{
  int k = v - 128;                         /* x - 127.5 = k + 0.5, rounded half to even like numpy */
  int q = ((k & 1) ? k + 1 : k) - 1;
  return (int8_t)(q < -128 ? -128 : q > 127 ? 127 : q);
}
#endif
