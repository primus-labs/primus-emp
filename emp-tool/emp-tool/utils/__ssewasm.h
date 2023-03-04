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


static __inline__ __m128i _mm_aesenc_si128(__m128i __V, __m128i __R)
{
  unsigned char *in = (unsigned char *)&__V;
  unsigned char out[33] = {0};
  u64 *key = (u64 *)&__R;
  _my_mm_aesenc_si128(in, out, key);
  __m128i tmp_out = *((__m128i *)out);
  return tmp_out;
}
static __inline__ __m128i _mm_aesenclast_si128(__m128i __V, __m128i __R)
{
  unsigned char *in = (unsigned char *)&__V;
  unsigned char out[33] = {0};
  u64 *key = (u64 *)&__R;
  _my_mm_aesenclast_si128(in, out, key);
  __m128i tmp_out = *((__m128i *)out);
  return tmp_out;
}
static __inline__ __m128i _mm_aesdec_si128(__m128i __V, __m128i __R)
{
  unsigned char *in = (unsigned char *)&__V;
  unsigned char out[33] = {0};
  u64 *key = (u64 *)&__R;
  _my_mm_aesdec_si128(in, out, key);
  __m128i tmp_out = *((__m128i *)out);
  return tmp_out;
}
static __inline__ __m128i _mm_aesdeclast_si128(__m128i __V, __m128i __R)
{
  unsigned char *in = (unsigned char *)&__V;
  unsigned char out[33] = {0};
  u64 *key = (u64 *)&__R;
  _my_mm_aesdeclast_si128(in, out, key);
  __m128i tmp_out = *((__m128i *)out);
  return tmp_out;
}
static __inline__ __m128i _mm_aesimc_si128(__m128i __V)
{
  __m128i in = __V;
  u64 *out = (u64 *)&in;
  _my_mm_aesimc_si128(out);
  __m128i tmp_out = *((__m128i *)out);
  return tmp_out;
}

// note: not _mm_aeskeygenassist_si128
static __inline__ void _mm_aeskeygenassist(__m128i __V, __m128i *__R, int nr, int nk)
{
  // __V: userKey
  // __R: rd_key
  unsigned char *userKey = (unsigned char *)&__V;
  u64 *rk = (u64 *)__R;
  _my_KeyExpansion(userKey, rk, nr, nk);
}

static __inline__ __m128i _mm_clmulepi64_si128(__m128i __X, __m128i __Y, const int __I)
{
  __m128i tmp_out;
  _my_mm_clmulepi64_si128((uint8_t *)&tmp_out, (uint8_t *)&__X, (uint8_t *)&__Y, __I);
  return tmp_out;
}

#endif
