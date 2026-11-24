/* PC test of the firmware's face pipeline (vision.c + face_models.c + the three TinyEngine models,
 * built exactly as for the STM32 except -DPCSIM). Reads 320x240 RGB565 (high byte first) frames from
 * stdin; per frame writes: n_faces(int32), score, box[4], kps[10], p_spoof, emb[512] (floats; zeros if no face). */
#include <stdio.h>
#include <string.h>
#include "face_models.h"
int main(void)
{
  static uint8_t buf[320 * 240 * 2];
  static int8_t crop112[112 * 112 * 3], crop64[64 * 64 * 3];
  while (fread(buf, 1, sizeof buf, stdin) == sizeof buf) {
    memcpy(FM_FRAME, buf, sizeof buf);                    /* the frame lives in the shared pool, as on the board */
    vis_face_t f[VIS_MAX_FACES];
    int n = fm_detect(FM_FRAME, f, VIS_MAX_FACES), b = vis_largest(f, n);
    float rec[1 + 4 + 10 + 1 + FM_EMB] = {0};
    if (b >= 0) {
      vis_align112(FM_FRAME, f[b].kps, crop112);           /* crops first: the next models overwrite the frame */
      vis_loose64(FM_FRAME, f[b].box, crop64);
      rec[0] = f[b].score; memcpy(&rec[1], f[b].box, 16); memcpy(&rec[5], f[b].kps, 40);
      rec[15] = fm_spoof_prob(crop64);
      fm_embed(crop112, &rec[16]);
    }
    int32_t nn = n; fwrite(&nn, 4, 1, stdout); fwrite(rec, 4, sizeof rec / 4, stdout);
  }
  return 0;
}
