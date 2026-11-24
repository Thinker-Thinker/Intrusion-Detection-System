/* 3x3 stride-2 int8 convolution with TensorFlow/Keras "same" padding (bottom/right only), any input channels.
 * Keras models need this: TinyEngine assumes PyTorch-style padding (top/left). Arguments as the TinyEngine kernel. */
#include "arm_math.h"
#include "tinyengine_function.h"

tinyengine_status convolve_s8_kernel3_stride2_tfsame_fpreq(
    const q7_t *input, const uint16_t input_x, const uint16_t input_y, const uint16_t input_ch,
    const q7_t *kernel, const int32_t *bias, const float *scales, const int32_t output_offset,
    const int32_t input_offset, const int32_t output_activation_min, const int32_t output_activation_max,
    q7_t *output, const uint16_t output_x, const uint16_t output_y, const uint16_t output_ch,
    q15_t *runtime_buf, q15_t *kbuf, q7_t pad_value)
{
  (void)runtime_buf; (void)kbuf;
  const int32_t pad_in = (int32_t)pad_value + input_offset;
  for (int oy = 0; oy < output_y; oy++)
    for (int ox = 0; ox < output_x; ox++)
      for (int oc = 0; oc < output_ch; oc++) {
        int32_t acc = bias[oc];
        const q7_t *w = kernel + oc * 9 * input_ch;
        for (int ky = 0; ky < 3; ky++) {
          int iy = oy * 2 + ky;          /* TF "same", stride 2: pad bottom/right only */
          for (int kx = 0; kx < 3; kx++, w += input_ch) {
            int ix = ox * 2 + kx;
            if (iy < 0 || iy >= input_y || ix < 0 || ix >= input_x) {
              int32_t ws = 0;
              for (int ic = 0; ic < input_ch; ic++) ws += w[ic];
              acc += ws * pad_in;
            } else {
              const q7_t *x = input + (iy * input_x + ix) * input_ch;
              for (int ic = 0; ic < input_ch; ic++) acc += ((int32_t)x[ic] + input_offset) * w[ic];
            }
          }
        }
        float r = (float)acc * scales[oc];
        int32_t v = (int32_t)(r >= 0.0f ? r + 0.5f : r - 0.5f) + output_offset;
        v = v < output_activation_min ? output_activation_min : v > output_activation_max ? output_activation_max : v;
        output[(oy * output_x + ox) * output_ch + oc] = (q7_t)v;
      }
  return STATE_SUCCESS;
}
