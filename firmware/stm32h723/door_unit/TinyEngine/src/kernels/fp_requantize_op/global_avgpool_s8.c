/* Global average pooling (Keras GlobalAveragePooling2D / TFLite MEAN over H,W) for TinyEngine-generated code.
 * TinyEngine's converter only handles MEAN as two 1-D means and silently drops a 2-D one, so the code
 * generator inserts this call. out = round((mean(x) - in_zp) * in_scale / out_scale) + out_zp. HWC int8. */
#include <stdint.h>
void global_avgpool_s8(const int8_t *in, int w, int h, int c, int8_t *out,
                       int in_zp, float in_scale, int out_zp, float out_scale)
{
  const int n = w * h; const float k = in_scale / (out_scale * (float)n);
  for (int ch = 0; ch < c; ch++) {
    int32_t s = 0;
    for (int i = 0; i < n; i++) s += in[i * c + ch];
    float r = (float)(s - in_zp * n) * k;
    int32_t v = (int32_t)(r >= 0.0f ? r + 0.5f : r - 0.5f) + out_zp;
    out[ch] = (int8_t)(v < -128 ? -128 : v > 127 ? 127 : v);
  }
}
