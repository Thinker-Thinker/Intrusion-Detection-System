/* Plain-C stand-ins for the Cortex-M DSP intrinsics TinyEngine/CMSIS-NN use, so the generated
 * model code can run on a PC and be checked against TFLite. Semantics follow the ARM ARM. */
#ifndef ARM_SHIM_H
#define ARM_SHIM_H
#include <stdint.h>
#define __STATIC_FORCEINLINE static inline __attribute__((always_inline))
#define __STATIC_INLINE static inline
#define __ALIGNED(x) __attribute__((aligned(x)))
#define __PACKED __attribute__((packed))
#define __RESTRICT __restrict
static inline uint32_t __ROR(uint32_t v, uint32_t s) { s &= 31; return s ? (v >> s) | (v << (32 - s)) : v; }
static inline int32_t __SMLAD(uint32_t x, uint32_t y, int32_t acc) {
  return acc + (int32_t)(int16_t)(x & 0xFFFF) * (int16_t)(y & 0xFFFF) + (int32_t)(int16_t)(x >> 16) * (int16_t)(y >> 16); }
static inline uint32_t __SXTB16(uint32_t x) {
  return ((uint32_t)(uint16_t)(int16_t)(int8_t)(x & 0xFF)) | ((uint32_t)(uint16_t)(int16_t)(int8_t)((x >> 16) & 0xFF) << 16); }
static inline uint32_t __UXTB16(uint32_t x) { return (x & 0xFF) | (x & 0xFF0000); }
static inline uint32_t __PKHBT(uint32_t a, uint32_t b, uint32_t s) { return (a & 0xFFFF) | ((b << s) & 0xFFFF0000u); }
static inline uint32_t __PKHTB(uint32_t a, uint32_t b, uint32_t s) { return (a & 0xFFFF0000u) | (((s ? (uint32_t)((int32_t)b >> s) : b)) & 0xFFFF); }
static inline uint32_t __SADD16(uint32_t a, uint32_t b) {
  return (uint32_t)(uint16_t)((int16_t)(a & 0xFFFF) + (int16_t)(b & 0xFFFF)) | ((uint32_t)(uint16_t)((int16_t)(a >> 16) + (int16_t)(b >> 16)) << 16); }
static inline int32_t __SSAT(int32_t v, uint32_t bits) { int32_t mx = (1 << (bits - 1)) - 1, mn = -(1 << (bits - 1)); return v > mx ? mx : v < mn ? mn : v; }
static inline uint32_t __USAT(int32_t v, uint32_t bits) { int32_t mx = (1 << bits) - 1; return v > mx ? mx : v < 0 ? 0 : v; }
static inline uint8_t __CLZ(uint32_t v) { return v ? __builtin_clz(v) : 32; }
static inline int32_t __QADD(int32_t a, int32_t b) { int64_t r = (int64_t)a + b; return r > INT32_MAX ? INT32_MAX : r < INT32_MIN ? INT32_MIN : (int32_t)r; }
static inline int32_t __QSUB(int32_t a, int32_t b) { int64_t r = (int64_t)a - b; return r > INT32_MAX ? INT32_MAX : r < INT32_MIN ? INT32_MIN : (int32_t)r; }
static inline uint32_t __SXTB16_RORn(uint32_t x, uint32_t r) { return __SXTB16(__ROR(x, r)); }
static inline uint32_t __QADD16(uint32_t a, uint32_t b) { int32_t l = (int16_t)(a & 0xFFFF) + (int16_t)(b & 0xFFFF), h = (int16_t)(a >> 16) + (int16_t)(b >> 16);
  l = l > 32767 ? 32767 : l < -32768 ? -32768 : l; h = h > 32767 ? 32767 : h < -32768 ? -32768 : h; return (uint32_t)(uint16_t)l | ((uint32_t)(uint16_t)h << 16); }
static inline int32_t __SMUAD(uint32_t x, uint32_t y) { return __SMLAD(x, y, 0); }
static inline int64_t __SMLALD(uint32_t x, uint32_t y, int64_t acc) { return acc + (int32_t)(int16_t)(x & 0xFFFF) * (int16_t)(y & 0xFFFF) + (int32_t)(int16_t)(x >> 16) * (int16_t)(y >> 16); }
static inline uint32_t __SXTAB16(uint32_t a, uint32_t b) { return __SADD16(a, __SXTB16(b)); }
static inline uint32_t __SXTAB16_RORn(uint32_t a, uint32_t b, uint32_t r) { return __SXTAB16(a, __ROR(b, r)); }
#endif
