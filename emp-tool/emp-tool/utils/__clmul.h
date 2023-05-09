#ifndef __CLMUL_H
#define __CLMUL_H

/*
https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html#text=_mm_clmulepi64_si128&ig_expand=827

IF (imm8[0] == 0)
  TEMP1 := a[63:0]
ELSE
  TEMP1 := a[127:64]
FI
IF (imm8[4] == 0)
  TEMP2 := b[63:0]
ELSE
  TEMP2 := b[127:64]
FI
FOR i := 0 to 63
  TEMP[i] := (TEMP1[0] and TEMP2[i])
  FOR j := 1 to i
    TEMP[i] := TEMP[i] XOR (TEMP1[j] AND TEMP2[i-j])
  ENDFOR
  dst[i] := TEMP[i]
ENDFOR
FOR i := 64 to 127
  TEMP[i] := 0
  FOR j := (i - 63) to 63
    TEMP[i] := TEMP[i] XOR (TEMP1[j] AND TEMP2[i-j])
  ENDFOR
  dst[i] := TEMP[i]
ENDFOR
dst[127] := 0
*/

#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

#ifdef use_qclmulq_v1
// static int ___mm_clmulepi64_si128_helper(uint8_t dst[16], uint8_t TEMP1[8], uint8_t TEMP2[8])
static int ___mm_clmulepi64_si128_helper(uint8_t *dst, uint8_t *TEMP1, uint8_t *TEMP2)
{
  // from low to high
  // printf("%x %x\n", TEMP1[7], TEMP2[7]);
  // printf("%x %x\n", TEMP1[0], TEMP2[0]);
  uint8_t TEMP[16] = {0};

  /*
  FOR i := 0 to 63
    TEMP[i] := (TEMP1[0] and TEMP2[i])
    FOR j := 1 to i
      TEMP[i] := TEMP[i] XOR (TEMP1[j] AND TEMP2[i-j])
    ENDFOR
    dst[i] := TEMP[i]
  ENDFOR
  */
  for (int i = 0; i < 64; i++)
  {
    // (c & 1101 1111) | ((a . b) & 0010 0000)
    // (c & mask_) | ((a OP b) & mask)
    uint8_t mask = 1;
    mask <<= (i & 7);
    uint8_t mask_ = (~mask);

    // TEMP[i] = (TEMP1[0] & TEMP2[i]);
    uint8_t c = TEMP[i >> 3]; // TEMP[i]
    uint8_t a = TEMP1[0];     // TEMP1[0]
    a >>= 0;
    uint8_t b = TEMP2[i >> 3]; // TEMP2[i]
    b >>= (i & 7);
    c = (c & mask_) | (((a & b) & 1) << (i & 7));
    TEMP[i >> 3] = c;

    for (int j = 1; j <= i; j++)
    {
      // TEMP[i] = TEMP[i] ^ (TEMP1[j] & TEMP2[i - j]);
      // ==>
      // TEMPJ = TEMP1[j] & TEMP2[i - j];
      // TEMP[i] = TEMP[i] ^ TEMPJ;
      uint8_t d = 0;             // TEMPJ
      uint8_t a = TEMP1[j >> 3]; // TEMP1[j]
      a >>= (j & 7);
      uint8_t b = TEMP2[(i - j) >> 3]; // TEMP2[i - j]
      b >>= ((i - j) & 7);
      d = a & b & 1;

      uint8_t c = TEMP[i >> 3]; // TEMP[i]
      uint8_t e = (c >> (i & 7));
      c = (c & mask_) | (((e ^ d) & 1) << (i & 7));
      TEMP[i >> 3] = c;
    }
    {
      // dst[i] = TEMP[i];
      uint8_t d = dst[i >> 3];  // dst[i];
      uint8_t c = TEMP[i >> 3]; // TEMP[i]
      d = (d & mask_) | (c & mask);
      dst[i >> 3] = d;
    }
  }

  /*
  FOR i := 64 to 127
    TEMP[i] := 0
    FOR j := (i - 63) to 63
      TEMP[i] := TEMP[i] XOR (TEMP1[j] AND TEMP2[i-j])
    ENDFOR
    dst[i] := TEMP[i]
  ENDFOR
  */
  for (int i = 64; i < 128; i++)
  {
    // (c & 1101 1111) | ((a . b) & 0010 0000)
    // (c & mask_) | ((a OP b) & mask)
    uint8_t mask = 1;
    mask <<= (i & 7);
    uint8_t mask_ = (~mask);

    // TEMP[i] = 0;
    uint8_t c = TEMP[i >> 3]; // TEMP[i]
    c = (c & mask_);
    TEMP[i >> 3] = c;

    for (int j = (i - 63); j <= 63; j++)
    {
      // (same as 1)
      // TEMP[i] = TEMP[i] ^ (TEMP1[j] & TEMP2[i - j]);
      // ==>
      // TEMPJ = TEMP1[j] & TEMP2[i - j];
      // TEMP[i] = TEMP[i] ^ TEMPJ;
      uint8_t d = 0;             // TEMPJ
      uint8_t a = TEMP1[j >> 3]; // TEMP1[j]
      a >>= (j & 7);
      uint8_t b = TEMP2[(i - j) >> 3]; // TEMP2[i - j]
      b >>= ((i - j) & 7);
      d = a & b & 1;

      uint8_t c = TEMP[i >> 3]; // TEMP[i]
      uint8_t e = (c >> (i & 7));
      c = (c & mask_) | (((e ^ d) & 1) << (i & 7));
      TEMP[i >> 3] = c;
    }
    {
      // (same as 1)
      // dst[i] = TEMP[i];
      uint8_t d = dst[i >> 3];  // dst[i];
      uint8_t c = TEMP[i >> 3]; // TEMP[i]
      d = (d & mask_) | (c & mask);
      dst[i >> 3] = d;
    }
  }

#if 0 // disable this, and works fine, why ??
  {
    // dst[127] = 0;
    uint8_t d = dst[127>> 3]; // dst[127];
    d = (d & 0xfe);          
    dst[127>> 3] = d;
  }
#endif

  return 0;
}

static void ___mm_clmulepi64_si128(uint8_t *c, uint8_t *a, uint8_t *b, int imm8)
{
  uint8_t *temp1 = NULL, *temp2 = NULL;
  switch (imm8)
  {
  case 0x00:
    temp1 = a;
    temp2 = b;
    break;
  case 0x01:
    temp1 = a + 8;
    temp2 = b;
    break;
  case 0x10:
    temp1 = a;
    temp2 = b + 8;
    break;
  case 0x11:
    temp1 = a + 8;
    temp2 = b + 8;
    break;

  default:
    break;
  }

  if (temp1 == NULL)
  {
    abort();
  }
  else
  {
    ___mm_clmulepi64_si128_helper(c, temp1, temp2);
  }
}
#else

static uint64_t __clmul_u64(uint64_t x, uint64_t y)
{
  uint64_t x0, x1, x2, x3;
  uint64_t y0, y1, y2, y3;
  uint64_t z0, z1, z2, z3;

  x0 = x & UINT64_C(0x1111111111111111);
  x1 = x & UINT64_C(0x2222222222222222);
  x2 = x & UINT64_C(0x4444444444444444);
  x3 = x & UINT64_C(0x8888888888888888);
  y0 = y & UINT64_C(0x1111111111111111);
  y1 = y & UINT64_C(0x2222222222222222);
  y2 = y & UINT64_C(0x4444444444444444);
  y3 = y & UINT64_C(0x8888888888888888);

  z0 = (x0 * y0) ^ (x1 * y3) ^ (x2 * y2) ^ (x3 * y1);
  z1 = (x0 * y1) ^ (x1 * y0) ^ (x2 * y3) ^ (x3 * y2);
  z2 = (x0 * y2) ^ (x1 * y1) ^ (x2 * y0) ^ (x3 * y3);
  z3 = (x0 * y3) ^ (x1 * y2) ^ (x2 * y1) ^ (x3 * y0);

  z0 &= UINT64_C(0x1111111111111111);
  z1 &= UINT64_C(0x2222222222222222);
  z2 &= UINT64_C(0x4444444444444444);
  z3 &= UINT64_C(0x8888888888888888);

  return z0 | z1 | z2 | z3;
}

static uint64_t __bitreverse_u64(uint64_t v)
{
  // clang-format off
    v = ((v >>  1) & UINT64_C(0x5555555555555555)) | ((v & UINT64_C(0x5555555555555555)) <<  1);
    v = ((v >>  2) & UINT64_C(0x3333333333333333)) | ((v & UINT64_C(0x3333333333333333)) <<  2);
    v = ((v >>  4) & UINT64_C(0x0F0F0F0F0F0F0F0F)) | ((v & UINT64_C(0x0F0F0F0F0F0F0F0F)) <<  4);
    v = ((v >>  8) & UINT64_C(0x00FF00FF00FF00FF)) | ((v & UINT64_C(0x00FF00FF00FF00FF)) <<  8);
    v = ((v >> 16) & UINT64_C(0x0000FFFF0000FFFF)) | ((v & UINT64_C(0x0000FFFF0000FFFF)) << 16);
  // clang-format on
  return (v >> 32) | (v << 32);
}

// static void __mm_clmulepi64_si128_u64(uint64_t c[2], uint64_t a[2], uint64_t b[2], int imm8)
static void __mm_clmulepi64_si128_u64(uint64_t *c, uint64_t *a, uint64_t *b, int imm8)
{
  c[0] = __clmul_u64(a[imm8 & 1], b[(imm8 >> 4) & 1]);

  uint64_t ta = __bitreverse_u64(a[imm8 & 1]);
  uint64_t tb = __bitreverse_u64(b[(imm8 >> 4) & 1]);
  c[1] = __bitreverse_u64(__clmul_u64(ta, tb)) >> 1;
}

static void ___mm_clmulepi64_si128(uint8_t *c, uint8_t *a, uint8_t *b, int imm8)
{
  __mm_clmulepi64_si128_u64((uint64_t *)c, (uint64_t *)a, (uint64_t *)b, imm8);
}
#endif

#endif
