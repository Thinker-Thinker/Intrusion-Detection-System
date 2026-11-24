/* Run the firmware's face pipeline, compiled for the STM32 (arm-none-eabi-gcc, same flags), inside
 * an ARM emulator (models/emu/run_emu.py). The host writes a 320x240 RGB565 frame to FM_FRAME and the
 * results are read back from emu_out: n_faces, score, box[4], kps[10], p_spoof, emb[512]. */
#include <string.h>
#include "face_models.h"
volatile float emu_out[1 + 1 + 4 + 10 + 1 + FM_EMB];
volatile int8_t emu_crop[112 * 112 * 3 + 64 * 64 * 3];   /* the two crops, for comparisons */
void emu_run(void)
{
  vis_face_t f[VIS_MAX_FACES];
  int n = fm_detect(FM_FRAME, f, VIS_MAX_FACES), b = vis_largest(f, n);
  float rec[1 + 1 + 4 + 10 + 1 + FM_EMB] = {0};
  rec[0] = (float)n;
  if (b >= 0) {
    vis_align112(FM_FRAME, f[b].kps, FM_CROP112);
    vis_loose64(FM_FRAME, f[b].box, FM_CROP64);
    memcpy((void *)emu_crop, FM_CROP112, 112 * 112 * 3 + 64 * 64 * 3);
    rec[1] = f[b].score; memcpy(&rec[2], f[b].box, 16); memcpy(&rec[6], f[b].kps, 40);
    rec[16] = fm_spoof_prob(FM_CROP64);
    fm_embed(FM_CROP112, &rec[17]);
  }
  for (unsigned i = 0; i < sizeof rec / 4; i++) emu_out[i] = rec[i];
#ifndef PCSIM
  __asm volatile("bkpt #0");
  for (;;) {}
#endif
}
/* recognition only, on a crop the host wrote to FM_CROP112 */
void emu_embed(void)
{
  float e[FM_EMB];
  fm_embed(FM_CROP112, e);
  for (int i = 0; i < FM_EMB; i++) emu_out[17 + i] = e[i];
#ifndef PCSIM
  __asm volatile("bkpt #0");
  for (;;) {}
#endif
}
#ifdef PCSIM
#include <stdio.h>
int main(void)
{
  static uint8_t buf[320 * 240 * 2];
  while (fread(buf, 1, sizeof buf, stdin) == sizeof buf) {
    memcpy(FM_FRAME, buf, sizeof buf); memset((void *)emu_crop, 0, sizeof emu_crop);
    emu_run();
    fwrite((const void *)emu_out, 1, sizeof emu_out, stdout); fwrite((const void *)emu_crop, 1, sizeof emu_crop, stdout);
  }
  return 0;
}
#endif
