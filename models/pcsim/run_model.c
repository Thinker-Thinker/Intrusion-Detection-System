/* PC harness: read frames of IN bytes from stdin, run the TinyEngine model, write OUT bytes of getOutput(). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
signed char *getInput(void); signed char *getOutput(void); void invoke(float *labels);
int main(int argc, char **argv) {
  int in = atoi(argv[1]), out = atoi(argv[2]); signed char *buf = malloc(in);
  while (fread(buf, 1, in, stdin) == (size_t)in) { memcpy(getInput(), buf, in); invoke(0); fwrite(getOutput(), 1, out, stdout); }
  return 0;
}
