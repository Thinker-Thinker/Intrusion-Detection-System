/* The three TinyEngine models (detector, liveness, recognition) behind one small API.
 * They share one 320 KB AXI SRAM pool (axi_pool) with the camera frame and run one at a time:
 *   [0 .. 153600)            camera frame (RGB565 QVGA)
 *   [153600 .. 317440)       detector activations (runs while the frame is still needed)
 *   [160000 .. 209920)       aligned 112x112 and liveness 64x64 crops (made after detection; outside
 *                            the liveness arena; copied into the recognition input before it runs)
 *   [160000 .. 185600)       LCD preview (only while the frame is drawn, before detection)
 *   [0 .. 301056)            recognition / liveness activations (run after the crops are made)
 * Plain C (no HAL) so it is also built and tested on a PC with -DPCSIM. */
#ifndef FACE_MODELS_H
#define FACE_MODELS_H
#include <stdint.h>
#include "vision.h"

#define FM_EMB 512
/* output quantisation of the int8 models (from the .tflite files used to generate Models/) */
#define FM_REC_OUT_ZP   (-15)
#define FM_LIV_OUT_SCALE 0.09741662442684174f
#define FM_LIV_OUT_ZP   15

extern signed char axi_pool[];
#define FM_FRAME   ((uint8_t *)axi_pool)
#define FM_CROP112 ((int8_t *)axi_pool + 160000)
#define FM_CROP64  ((int8_t *)axi_pool + 160000 + 112 * 112 * 3)
#define FM_PREVIEW ((uint16_t *)(axi_pool + 160000))

/* Detector on a 320x240 RGB565 frame; returns faces (frame pixels), best score first. */
int fm_detect(const uint8_t *frame, vis_face_t *faces, int max_faces);
/* Probability (0..1) that a 64x64 liveness crop is a printed photo or a screen. */
float fm_spoof_prob(const int8_t *crop64);
/* L2-normalised 512-number face embedding of a 112x112 aligned crop. */
void fm_embed(const int8_t *crop112, float *emb);
#endif
