/* Glue for the three TinyEngine models (see face_models.h). */
#include "face_models.h"
#include <math.h>
#include <string.h>
#include "det_heads.h"

#ifdef PCSIM
#define SEC_AXI
#define SEC_D2
#define SEC_D3
#else
#define SEC_AXI __attribute__((section(".axi_sram"), aligned(32)))
#define SEC_D2  __attribute__((section(".sram_d2"), aligned(32)))
#define SEC_D3  __attribute__((section(".sram_d3"), aligned(32)))
#endif

/* 320 KB pool: camera frame + every model's activations (they never run at the same time) */
SEC_AXI signed char axi_pool[327680];
/* TinyEngine im2col / kernel scratch, shared by all models (largest: detector 10824 B / 23040 B) */
SEC_D3 int16_t te_sbuf[10824 / 2 + 16];
SEC_D2 int16_t te_kbuf[23040 / 2 + 16];

signed char *det_getInput(void); void det_invoke(float *); signed char *det_te_model_arena(void);
signed char *liv_getInput(void); signed char *liv_getOutput(void); void liv_invoke(float *);
signed char *rec_getInput(void); signed char *rec_getOutput(void); void rec_invoke(float *);

int fm_detect(const uint8_t *frame, vis_face_t *faces, int max_faces)
{
  vis_det_input(frame, (int8_t *)det_getInput());
  det_invoke(0);
  signed char *buffer0 = det_te_model_arena();
  te_head_t heads[TE_N_HEADS] = TE_HEADS(buffer0);
  return vis_det_decode((const vis_head_t *)heads, TE_N_HEADS, 0.5f, faces, max_faces);
}

float fm_spoof_prob(const int8_t *crop64)
{
  memcpy(liv_getInput(), crop64, 64 * 64 * 3);
  liv_invoke(0);
  const signed char *o = liv_getOutput();
  float live = (o[0] - FM_LIV_OUT_ZP) * FM_LIV_OUT_SCALE, spoof = (o[1] - FM_LIV_OUT_ZP) * FM_LIV_OUT_SCALE;
  return 1.0f / (1.0f + expf(live - spoof));
}

void fm_embed(const int8_t *crop112, float *emb)
{
  memcpy(rec_getInput(), crop112, 112 * 112 * 3);
  rec_invoke(0);
  const signed char *o = rec_getOutput();
  float n = 0;
  for (int i = 0; i < FM_EMB; i++) { emb[i] = (float)(o[i] - FM_REC_OUT_ZP); n += emb[i] * emb[i]; }
  n = 1.0f / sqrtf(n + 1e-12f);
  for (int i = 0; i < FM_EMB; i++) emb[i] *= n;
}
