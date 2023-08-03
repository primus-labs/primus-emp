
#include "emp-tool/utils/block.h"
#include <iostream>
#include <map>
#include <openssl/aes.h>
using namespace std;
using namespace emp;

#define USE_BSAES_SCHEMA 0

#if USE_BSAES_SCHEMA
extern "C" {
#include "emp-tool/utils/_bsaes.h"
}
#endif

#if __EMSCRIPTEN__
// the following counter for performance debug
int64_t _mm_aesenc_si128_counter = 0;
int64_t _mm_aesenclast_si128_counter = 0;
int64_t _mm_aesdec_si128_counter = 0;
int64_t _mm_aesdeclast_si128_counter = 0;
int64_t _mm_clmulepi64_si128_counter = 0;
map<int64_t, int64_t> map_aesencrypt_counter;

void _aes_counter_add(int64_t &counter) { counter++; }
void _aes_counter_print(bool reset) {
#define __print_c(c) cout << #c << ":" << c << endl;
  __print_c(_mm_aesenc_si128_counter);
  __print_c(_mm_aesenclast_si128_counter);
  __print_c(_mm_aesdec_si128_counter);
  __print_c(_mm_aesdeclast_si128_counter);
  __print_c(_mm_clmulepi64_si128_counter);
#undef __print_c
  for (auto &a : map_aesencrypt_counter) {
    cout << "_aesencrypt_counter:" << a.first << " -> " << a.second << endl;
  }
  if (reset) {
    _mm_aesenc_si128_counter = 0;
    _mm_aesenclast_si128_counter = 0;
    _mm_aesdec_si128_counter = 0;
    _mm_aesdeclast_si128_counter = 0;
    _mm_clmulepi64_si128_counter = 0;
    map_aesencrypt_counter.clear();
  }
}
#endif

// for emp::AES_KEY conflict with openssl::AES_KEY
typedef struct {
  block rd_key[11];
  unsigned int rounds;
} EMP_AES_KEY;

#ifdef __x86_64__
__attribute__((target("aes,sse2")))
#elif __EMSCRIPTEN__
__attribute__((target("simd128")))
#endif
void
__AES_ecb_encrypt_blks(unsigned char *_blks, unsigned int nblks, const unsigned char *_key)
{
  if (nblks == 0)
    return;

  block *blks = (block *)_blks;
  EMP_AES_KEY *key = (EMP_AES_KEY *)_key;
  {
    AES_KEY aeskey;
    aeskey.rounds = key->rounds;

#if __EMSCRIPTEN__
    // wasm layout
    // emp: 00010203 04050607 08090a0b 0c0d0e0f
    // ssl: 03020100 07060504 0b0a0908 0f0e0d0c
    unsigned char *pekey = (unsigned char *)key->rd_key;
    unsigned char *pokey = (unsigned char *)aeskey.rd_key;
    for (int i = 0; i < 22; i++) // 11*16 = 22*8
    {
      pokey[0] = pekey[3];
      pokey[1] = pekey[2];
      pokey[2] = pekey[1];
      pokey[3] = pekey[0];
      pokey[4] = pekey[7];
      pokey[5] = pekey[6];
      pokey[6] = pekey[5];
      pokey[7] = pekey[4];

      pekey += 8;
      pokey += 8;
    }
#else // __x86_64__
    memcpy((char *)aeskey.rd_key, (char *)key->rd_key, sizeof(key->rd_key));
#endif

    for (unsigned int i = 0; i < nblks; i++) {
      AES_ecb_encrypt((unsigned char *)blks + AES_BLOCK_SIZE * i,
                      (unsigned char *)blks + AES_BLOCK_SIZE * i, &aeskey,
                      AES_ENCRYPT);
    }
  }
}

#ifdef __x86_64__
__attribute__((target("aes,sse2")))
#elif __EMSCRIPTEN__
__attribute__((target("simd128")))
#endif
void
_AES_ecb_encrypt_blks(unsigned char *_blks, unsigned int nblks, const unsigned char *_key)
{
#if !USE_BSAES_SCHEMA
  __AES_ecb_encrypt_blks(_blks, nblks, _key);
  return;
#else
  block *blks = (block *)_blks;
  EMP_AES_KEY *key = (EMP_AES_KEY *)_key;
  if (nblks >= 16) {
    __m128i roundKey[11];
    memcpy((char *)roundKey, (char *)key->rd_key, sizeof(key->rd_key));
    BSAES8State rk[11];
    for (int i = 0; i < 11; i++) {
      (rk + i)->x0 = roundKey[i];
      (rk + i)->x1 = roundKey[i];
      (rk + i)->x2 = roundKey[i];
      (rk + i)->x3 = roundKey[i];
      (rk + i)->x4 = roundKey[i];
      (rk + i)->x5 = roundKey[i];
      (rk + i)->x6 = roundKey[i];
      (rk + i)->x7 = roundKey[i];
      bit_slice_1x128_with_u32x4((uint8_t *)(rk + i), rk + i);
    }

    int N = nblks / 8;
    BSAES8State state;
    BSAES8State out;
    int i = 0;
    for (; i < N; i++) {
      bit_slice_1x128_with_u32x4((uint8_t *)(blks + i * 8), &state);
      encrypt_core(&state, rk, &out);
      un_bit_slice_1x128_with_u32x4(&out, (uint8_t *)(blks + i * 8));
    }

    int remain = nblks % 8;
    if (remain >= 4) {
      uint8_t in[128] = {0};
      uint8_t ou[128] = {0};
      memcpy(in, (uint8_t *)(blks + i * 8), remain * 16);
      bit_slice_1x128_with_u32x4(in, &state);
      encrypt_core(&state, rk, &out);
      un_bit_slice_1x128_with_u32x4(&out, ou);
      memcpy((uint8_t *)(blks + i * 8), ou, remain * 16);
    } else {
      __AES_ecb_encrypt_blks((unsigned char *)(blks + i * 8), remain, _key);
    }
  } else {
    __AES_ecb_encrypt_blks(_blks, nblks, _key);
  }
#endif
}
