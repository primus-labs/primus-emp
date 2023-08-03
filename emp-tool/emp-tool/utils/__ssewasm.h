#ifndef __SSEWASM_H
#define __SSEWASM_H

#include "__aesenc.h"
#include "__clmul.h"

#include "wasm_simd128.h"
#include <immintrin.h> // -mavx
// #include <xmmintrin.h> // -msse
// #include <emmintrin.h> // -msse2
// #include <pmmintrin.h> // -msse3
// #include <tmmintrin.h> // -mssse3
// #include <smmintrin.h> // -msse4.1
// #include <nmmintrin.h> // -msse4.2

#define make_mm_aes_op(__op)                                                   \
  static __inline__ __m128i __attribute__((always_inline))                     \
  __op(__m128i __V, __m128i __R) {                                             \
    __m128i __O;                                                               \
    __##__op((uint8_t *)&__V, (uint8_t *)&__O, (u64 *)&__R);                   \
    aes_counter_add(__op##_counter);                                           \
    return __O;                                                                \
  }

make_mm_aes_op(_mm_aesenc_si128);
make_mm_aes_op(_mm_aesenclast_si128);
make_mm_aes_op(_mm_aesdec_si128);
make_mm_aes_op(_mm_aesdeclast_si128);

static __inline__ __m128i _mm_aesimc_si128(__m128i __V) {
  __m128i out = __V;
  ___mm_aesimc_si128((u64 *)&out);
  return out;
}

// note: not _mm_aeskeygenassist_si128
static __inline__ void _mm_aeskeygenassist(__m128i __V, __m128i *__R, int nr,
                                           int nk) {
  // __V: userKey
  // __R: rd_key
  ___KeyExpansion((unsigned char *)&__V, (u64 *)__R, nr, nk);
}

static __inline__ __m128i _mm_clmulepi64_si128(__m128i __X, __m128i __Y,
                                               const int __I) {
  __m128i out;
  ___mm_clmulepi64_si128((uint8_t *)&out, (uint8_t *)&__X, (uint8_t *)&__Y,
                         __I);
  aes_counter_add(_mm_clmulepi64_si128_counter);
  return out;
}

#endif
