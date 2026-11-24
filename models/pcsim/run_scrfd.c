/* PC harness: read N raw int8 160x128x3 frames from stdin, run the TinyEngine SCRFD code, write the 9 heads. */
#include <stdio.h>
#include <string.h>
#include "scrfd_heads.h"
signed char *getInput(void); void invoke(float *labels);
int main(void) {
  static signed char frame[160 * 128 * 3];
  while (fread(frame, 1, sizeof frame, stdin) == sizeof frame) {
    memcpy(getInput(), frame, sizeof frame);
    invoke(0);
    signed char *buffer0 = scrfd_arena();
    scrfd_head_t heads[9] = SCRFD_HEADS(buffer0);
    for (int i = 0; i < 9; i++) fwrite(heads[i].data, 1, heads[i].h * heads[i].w * heads[i].c, stdout);
  }
  return 0;
}
