/* PC harness for the detector: run TinyEngine SCRFD, write the 9 heads (order of heads.h). */
#include <stdio.h>
#include <string.h>
#include "heads.h"
signed char *getInput(void); void invoke(float *labels); signed char *te_model_arena(void);
int main(void) {
  static signed char frame[160 * 128 * 3];
  while (fread(frame, 1, sizeof frame, stdin) == sizeof frame) {
    memcpy(getInput(), frame, sizeof frame); invoke(0);
    signed char *buffer0 = te_model_arena(); te_head_t heads[TE_N_HEADS] = TE_HEADS(buffer0);
    for (int i = 0; i < TE_N_HEADS; i++) fwrite(heads[i].data, 1, heads[i].h * heads[i].w * heads[i].c, stdout);
  }
  return 0;
}
