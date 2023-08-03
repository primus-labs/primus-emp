#ifndef BSAES_H
#define BSAES_H

#include <stdint.h>
#include <stdlib.h>
#include <tmmintrin.h>
#include <emmintrin.h>
#include "emp-tool/utils/_stable.h"

extern const __m128i bit0; // Used to extract the 0 bit data.
extern const __m128i bit1; // Used to extract the 1 bit data.
extern const __m128i bit2; // Used to extract the 2 bit data.
extern const __m128i bit3; // Used to extract the 3 bit data.
extern const __m128i bit4; // Used to extract the 4 bit data.
extern const __m128i bit5; // Used to extract the 5 bit data.
extern const __m128i bit6; // Used to extract the 6 bit data.
extern const __m128i bit7; // Used to extract the 7 bit data.

extern const __m128i flp; // Used to negate a 128 bit data.

extern const __m128i shift_row_mask; // Used to implement shift row operation for a 128-bit block data.

extern const __m128i convert_row_major_mask; // Used to convert a column major 128-bit data to row major.

// Used to implement ror 1 byte for 128-bit data using shuffle_eip32.
#define ror1_mask 0b00111001
// Used to implement ror 2 bytes for 128-bit data using shuffle_eip32.
#define ror2_mask 0b01001110

typedef struct BSAES2State
{
    __m128i x0;
    __m128i x1;
} BSAES2State;

typedef struct BSAES4State
{
    __m128i x0;
    __m128i x1;
    __m128i x2;
    __m128i x3;
} BSAES4State;

typedef struct BSAES8State
{
    __m128i x0;
    __m128i x1;
    __m128i x2;
    __m128i x3;
    __m128i x4;
    __m128i x5;
    __m128i x6;
    __m128i x7;
} BSAES8State;

// ============================ Basic 128-bit SIMD operation ==========================
#define and128(a, b) _mm_and_si128((a), (b))
#define xor128(a, b) _mm_xor_si128((a), (b))
#define or128(a, b) _mm_or_si128((a), (b))
#define shl128(a, n)                       \
    ({                                     \
        __m128i v1, v2;                    \
        v1 = _mm_slli_epi64((a), (n));     \
        v2 = _mm_slli_si128((a), 8);       \
        v2 = _mm_srli_epi64(v2, 64 - (n)); \
        v1 = _mm_or_si128(v1, v2);         \
        v1;                                \
    })
#define shr128(a, n)                       \
    ({                                     \
        __m128i v1, v2;                    \
        v1 = _mm_srli_epi64((a), (n));     \
        v2 = _mm_srli_si128((a), 8);       \
        v2 = _mm_slli_epi64(v2, 64 - (n)); \
        v1 = _mm_or_si128(v1, v2);         \
        v1;                                \
    })

#define or8x128(x0, x1, x2, x3, x4, x5, x6, x7) \
    or128(or128(or128((x0), (x1)), or128((x2), (x3))), or128(or128((x4), (x5)), or128((x6), (x7))))

#define xor3_128(x0, x1, x2) xor128(xor128((x0), (x1)), (x2))
#define xor4_128(x0, x1, x2, x3) xor128(xor128((x0), (x1)), xor128((x2), (x3)))
#define xor5_128(x0, x1, x2, x3, x4) xor128(xor128((x0), (x1)), xor3_128((x2), (x3), (x4)))
#define xor6_128(x0, x1, x2, x3, x4, x5) xor128(xor3_128((x0), (x1), (x2)), xor3_128((x3), (x4), (x5)))

#define shift_row(a) _mm_shuffle_epi8((a), shift_row_mask)
#define ror1(a) _mm_shuffle_epi32((a), ror1_mask)
#define ror2(a) _mm_shuffle_epi32((a), ror2_mask)
// ======================================================================================

// =================================== Basic GF Operation ===============================
#define BS2StateGFMUL(a, b, res)                                                  \
    do                                                                            \
    {                                                                             \
        __m128i tmp = and128(xor128((a)->x1, (a)->x0), xor128((b)->x0, (b)->x1)); \
        (res)->x0 = xor128(and128((a)->x0, (b)->x0), tmp);                        \
        (res)->x1 = xor128(and128((a)->x1, (b)->x1), tmp);                        \
    } while (0)

#define BS2StateGFXOR(a, b, res)              \
    do                                        \
    {                                         \
        (res)->x0 = xor128((a)->x0, (b)->x0); \
        (res)->x1 = xor128((a)->x1, (b)->x1); \
    } while (0)

#define BS2StateGFSCL_N(a)                  \
    do                                      \
    {                                       \
        __m128i tmp = (a)->x0;              \
        (a)->x0 = xor128((a)->x0, (a)->x1); \
        (a)->x1 = tmp;                      \
    } while (0)

#define BS2StateGFSCL_N2(a)             \
    do                                  \
    {                                   \
        __m128i tmp = (a)->x0;          \
        (a)->x0 = (a)->x1;              \
        (a)->x1 = xor128((a)->x1, tmp); \
    } while (0)

#define BS2StateGFINV(a)       \
    do                         \
    {                          \
        __m128i tmp = (a)->x0; \
        (a)->x0 = (a)->x1;     \
        (a)->x1 = tmp;         \
    } while (0)

#define BS4StateGFXOR(a, b, res)              \
    do                                        \
    {                                         \
        (res)->x0 = xor128((a)->x0, (b)->x0); \
        (res)->x1 = xor128((a)->x1, (b)->x1); \
        (res)->x2 = xor128((a)->x2, (b)->x2); \
        (res)->x3 = xor128((a)->x3, (b)->x3); \
    } while (0)

#define BS4StateGFSQ_SCL(a)          \
    do                               \
    {                                \
        BSAES2State aa, bb;          \
        BSAES2State p;               \
        aa.x0 = (a)->x2;             \
        aa.x1 = (a)->x3;             \
        bb.x0 = (a)->x0;             \
        bb.x1 = (a)->x1;             \
        BS2StateGFXOR(&aa, &bb, &p); \
        __m128i tmp = p.x1;          \
        p.x1 = p.x0;                 \
        p.x0 = tmp;                  \
        tmp = bb.x1;                 \
        bb.x1 = bb.x0;               \
        bb.x0 = tmp;                 \
        BS2StateGFSCL_N2(&bb);       \
        (a)->x0 = bb.x0;             \
        (a)->x1 = bb.x1;             \
        (a)->x2 = p.x0;              \
        (a)->x3 = p.x1;              \
    } while (0)

#define BS8StateXOR_X63(a)              \
    do                                  \
    {                                   \
        (a)->x5 = xor128((a)->x5, flp); \
        (a)->x6 = xor128((a)->x6, flp); \
        (a)->x0 = xor128((a)->x0, flp); \
        (a)->x1 = xor128((a)->x1, flp); \
    } while (0);

#define read_row_major(data)                                    \
    _mm_setr_epi8((data)[0], (data)[4], (data)[8], (data)[12],  \
                  (data)[1], (data)[5], (data)[9], (data)[13],  \
                  (data)[2], (data)[6], (data)[10], (data)[14], \
                  (data)[3], (data)[7], (data)[11], (data)[15])

#define write_row_major(data, out)                                 \
    do                                                             \
    {                                                              \
        (data) = _mm_shuffle_epi8((data), convert_row_major_mask); \
        _mm_stream_si128((__m128i *)(out), (data));                \
    } while (0)

#define AddRoundKey(state, rk)                       \
    do                                               \
    {                                                \
        (state)->x0 = xor128((state)->x0, (rk)->x0); \
        (state)->x1 = xor128((state)->x1, (rk)->x1); \
        (state)->x2 = xor128((state)->x2, (rk)->x2); \
        (state)->x3 = xor128((state)->x3, (rk)->x3); \
        (state)->x4 = xor128((state)->x4, (rk)->x4); \
        (state)->x5 = xor128((state)->x5, (rk)->x5); \
        (state)->x6 = xor128((state)->x6, (rk)->x6); \
        (state)->x7 = xor128((state)->x7, (rk)->x7); \
    } while (0)

// =================================================================================

// =================================== EXPORT Method ===============================
void bit_slice_1x128_with_u32x4(unsigned char *data, BSAES8State *result);
void un_bit_slice_1x128_with_u32x4(BSAES8State *state, unsigned char *out);
void aes_key_expansion_sse2(uint8_t *key, __m128i *roundKey);
// void encrypt_core(BSAES8State *state, __m128i *rk, BSAES8State *out);
void encrypt_core(BSAES8State *state, BSAES8State *rk, BSAES8State *out);
// =================================================================================

#endif