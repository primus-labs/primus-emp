#include "emp-tool/utils/_bsaes.h"
#include <string.h>
#include <stdio.h>

// When compiling the code with emcc, it seems not support 64-bit imm data.
// We need to seperate the 128-bit const data into 4 32-bit data when difining it.
// __EMSCRIPTEN__ is defined by emcc compiler.
#ifdef __EMSCRIPTEN__
const __m128i bit0 __attribute__((aligned(0x10))) =
    {0x01010101, 0x01010101, 0x01010101, 0x01010101};
const __m128i bit1 __attribute__((aligned(0x10))) =
    {0x02020202, 0x02020202, 0x02020202, 0x02020202};
const __m128i bit2 __attribute__((aligned(0x10))) =
    {0x04040404, 0x04040404, 0x04040404, 0x04040404};
const __m128i bit3 __attribute__((aligned(0x10))) =
    {0x08080808, 0x08080808, 0x08080808, 0x08080808};
const __m128i bit4 __attribute__((aligned(0x10))) =
    {0x10101010, 0x10101010, 0x10101010, 0x10101010};
const __m128i bit5 __attribute__((aligned(0x10))) =
    {0x20202020, 0x20202020, 0x20202020, 0x20202020};
const __m128i bit6 __attribute__((aligned(0x10))) =
    {0x40404040, 0x40404040, 0x40404040, 0x40404040};
const __m128i bit7 __attribute__((aligned(0x10))) =
    {0x80808080, 0x80808080, 0x80808080, 0x80808080};

const __m128i flp __attribute__((aligned(0x10))) =
    {0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff};

const __m128i shift_row_mask __attribute__((aligned(0x10))) =
    {0x03020100, 0x04070605, 0x09080b0a, 0x0e0d0c0f};

const __m128i convert_row_major_mask __attribute__((aligned(0x10))) =
    {0x0c080400, 0x0d090501, 0x0e0a0602, 0x0f0b0703};
#else
const __m128i bit0 __attribute__((aligned(0x10))) =
    {0x0101010101010101, 0x0101010101010101};
const __m128i bit1 __attribute__((aligned(0x10))) =
    {0x0202020202020202, 0x0202020202020202};
const __m128i bit2 __attribute__((aligned(0x10))) =
    {0x0404040404040404, 0x0404040404040404};
const __m128i bit3 __attribute__((aligned(0x10))) =
    {0x0808080808080808, 0x0808080808080808};
const __m128i bit4 __attribute__((aligned(0x10))) =
    {0x1010101010101010, 0x1010101010101010};
const __m128i bit5 __attribute__((aligned(0x10))) =
    {0x2020202020202020, 0x2020202020202020};
const __m128i bit6 __attribute__((aligned(0x10))) =
    {0x4040404040404040, 0x4040404040404040};
const __m128i bit7 __attribute__((aligned(0x10))) =
    {0x8080808080808080, 0x8080808080808080};

const __m128i flp __attribute__((aligned(0x10))) =
    {0xffffffffffffffff, 0xffffffffffffffff};

const __m128i shift_row_mask __attribute__((aligned(0x10))) =
    {0x0407060503020100, 0x0e0d0c0f09080b0a};

const __m128i convert_row_major_mask __attribute__((aligned(0x10))) =
    {0x0d0905010c080400, 0x0f0b07030e0a0602};
#endif

// Bit Slice a 128 byte array of eight 16 byte blocks. Each block is in column major order.
void bit_slice_1x128_with_u32x4(unsigned char *data, BSAES8State *result)
{

    __m128i t0 = read_row_major(data);
    __m128i t1 = read_row_major(data + 16);
    __m128i t2 = read_row_major(data + 32);
    __m128i t3 = read_row_major(data + 48);
    __m128i t4 = read_row_major(data + 64);
    __m128i t5 = read_row_major(data + 80);
    __m128i t6 = read_row_major(data + 96);
    __m128i t7 = read_row_major(data + 112);

    __m128i x0 = or8x128(
        and128(t0, bit0),
        and128(shl128(t1, 1), bit1),
        and128(shl128(t2, 2), bit2),
        and128(shl128(t3, 3), bit3),
        and128(shl128(t4, 4), bit4),
        and128(shl128(t5, 5), bit5),
        and128(shl128(t6, 6), bit6),
        and128(shl128(t7, 7), bit7));

    __m128i x1 = or8x128(
        and128(shr128(t0, 1), bit0),
        and128(t1, bit1),
        and128(shl128(t2, 1), bit2),
        and128(shl128(t3, 2), bit3),
        and128(shl128(t4, 3), bit4),
        and128(shl128(t5, 4), bit5),
        and128(shl128(t6, 5), bit6),
        and128(shl128(t7, 6), bit7));

    __m128i x2 = or8x128(
        and128(shr128(t0, 2), bit0),
        and128(shr128(t1, 1), bit1),
        and128(t2, bit2),
        and128(shl128(t3, 1), bit3),
        and128(shl128(t4, 2), bit4),
        and128(shl128(t5, 3), bit5),
        and128(shl128(t6, 4), bit6),
        and128(shl128(t7, 5), bit7));

    __m128i x3 = or8x128(
        and128(shr128(t0, 3), bit0),
        and128(shr128(t1, 2), bit1),
        and128(shr128(t2, 1), bit2),
        and128(t3, bit3),
        and128(shl128(t4, 1), bit4),
        and128(shl128(t5, 2), bit5),
        and128(shl128(t6, 3), bit6),
        and128(shl128(t7, 4), bit7));

    __m128i x4 = or8x128(
        and128(shr128(t0, 4), bit0),
        and128(shr128(t1, 3), bit1),
        and128(shr128(t2, 2), bit2),
        and128(shr128(t3, 1), bit3),
        and128(t4, bit4),
        and128(shl128(t5, 1), bit5),
        and128(shl128(t6, 2), bit6),
        and128(shl128(t7, 3), bit7));

    __m128i x5 = or8x128(
        and128(shr128(t0, 5), bit0),
        and128(shr128(t1, 4), bit1),
        and128(shr128(t2, 3), bit2),
        and128(shr128(t3, 2), bit3),
        and128(shr128(t4, 1), bit4),
        and128(t5, bit5),
        and128(shl128(t6, 1), bit6),
        and128(shl128(t7, 2), bit7));

    __m128i x6 = or8x128(
        and128(shr128(t0, 6), bit0),
        and128(shr128(t1, 5), bit1),
        and128(shr128(t2, 4), bit2),
        and128(shr128(t3, 3), bit3),
        and128(shr128(t4, 2), bit4),
        and128(shr128(t5, 1), bit5),
        and128(t6, bit6),
        and128(shl128(t7, 1), bit7));

    __m128i x7 = or8x128(
        and128(shr128(t0, 7), bit0),
        and128(shr128(t1, 6), bit1),
        and128(shr128(t2, 5), bit2),
        and128(shr128(t3, 4), bit3),
        and128(shr128(t4, 3), bit4),
        and128(shr128(t5, 2), bit5),
        and128(shr128(t6, 1), bit6),
        and128(t7, bit7));

    result->x0 = x0;
    result->x1 = x1;
    result->x2 = x2;
    result->x3 = x3;
    result->x4 = x4;
    result->x5 = x5;
    result->x6 = x6;
    result->x7 = x7;
}

// Un bit slice into a 128 byte buffer.
void un_bit_slice_1x128_with_u32x4(BSAES8State *state, unsigned char *out)
{

    __m128i t0 = state->x0;
    __m128i t1 = state->x1;
    __m128i t2 = state->x2;
    __m128i t3 = state->x3;
    __m128i t4 = state->x4;
    __m128i t5 = state->x5;
    __m128i t6 = state->x6;
    __m128i t7 = state->x7;

    // decode the individual blocks, in row-major order
    __m128i x0 = or8x128(
        and128(t0, bit0),
        and128(shl128(t1, 1), bit1),
        and128(shl128(t2, 2), bit2),
        and128(shl128(t3, 3), bit3),
        and128(shl128(t4, 4), bit4),
        and128(shl128(t5, 5), bit5),
        and128(shl128(t6, 6), bit6),
        and128(shl128(t7, 7), bit7));

    __m128i x1 = or8x128(
        and128(shr128(t0, 1), bit0),
        and128(t1, bit1),
        and128(shl128(t2, 1), bit2),
        and128(shl128(t3, 2), bit3),
        and128(shl128(t4, 3), bit4),
        and128(shl128(t5, 4), bit5),
        and128(shl128(t6, 5), bit6),
        and128(shl128(t7, 6), bit7));

    __m128i x2 = or8x128(
        and128(shr128(t0, 2), bit0),
        and128(shr128(t1, 1), bit1),
        and128(t2, bit2),
        and128(shl128(t3, 1), bit3),
        and128(shl128(t4, 2), bit4),
        and128(shl128(t5, 3), bit5),
        and128(shl128(t6, 4), bit6),
        and128(shl128(t7, 5), bit7));

    __m128i x3 = or8x128(
        and128(shr128(t0, 3), bit0),
        and128(shr128(t1, 2), bit1),
        and128(shr128(t2, 1), bit2),
        and128(t3, bit3),
        and128(shl128(t4, 1), bit4),
        and128(shl128(t5, 2), bit5),
        and128(shl128(t6, 3), bit6),
        and128(shl128(t7, 4), bit7));

    __m128i x4 = or8x128(
        and128(shr128(t0, 4), bit0),
        and128(shr128(t1, 3), bit1),
        and128(shr128(t2, 2), bit2),
        and128(shr128(t3, 1), bit3),
        and128(t4, bit4),
        and128(shl128(t5, 1), bit5),
        and128(shl128(t6, 2), bit6),
        and128(shl128(t7, 3), bit7));

    __m128i x5 = or8x128(
        and128(shr128(t0, 5), bit0),
        and128(shr128(t1, 4), bit1),
        and128(shr128(t2, 3), bit2),
        and128(shr128(t3, 2), bit3),
        and128(shr128(t4, 1), bit4),
        and128(t5, bit5),
        and128(shl128(t6, 1), bit6),
        and128(shl128(t7, 2), bit7));

    __m128i x6 = or8x128(
        and128(shr128(t0, 6), bit0),
        and128(shr128(t1, 5), bit1),
        and128(shr128(t2, 4), bit2),
        and128(shr128(t3, 3), bit3),
        and128(shr128(t4, 2), bit4),
        and128(shr128(t5, 1), bit5),
        and128(t6, bit6),
        and128(shl128(t7, 1), bit7));

    __m128i x7 = or8x128(
        and128(shr128(t0, 7), bit0),
        and128(shr128(t1, 6), bit1),
        and128(shr128(t2, 5), bit2),
        and128(shr128(t3, 4), bit3),
        and128(shr128(t4, 3), bit4),
        and128(shr128(t5, 2), bit5),
        and128(shr128(t6, 1), bit6),
        and128(t7, bit7));

    write_row_major(x0, out);
    write_row_major(x1, out + 16);
    write_row_major(x2, out + 32);
    write_row_major(x3, out + 48);
    write_row_major(x4, out + 64);
    write_row_major(x5, out + 80);
    write_row_major(x6, out + 96);
    write_row_major(x7, out + 112);
}

static inline void change_basis_a2x(BSAES8State *state)
{
    __m128i t06 = xor128(state->x6, state->x0);
    __m128i t056 = xor128(state->x5, t06);
    __m128i t0156 = xor128(t056, state->x1);
    __m128i t13 = xor128(state->x1, state->x3);

    BSAES8State tmp;
    tmp.x0 = xor3_128(state->x2, t06, t13);
    tmp.x1 = t056;
    tmp.x2 = state->x0;
    tmp.x3 = xor4_128(state->x0, state->x4, state->x7, t13);
    tmp.x4 = xor128(state->x7, t056);
    tmp.x5 = t0156;
    tmp.x6 = xor128(state->x4, t056);
    tmp.x7 = xor3_128(state->x2, state->x7, t0156);

    state->x0 = tmp.x0;
    state->x1 = tmp.x1;
    state->x2 = tmp.x2;
    state->x3 = tmp.x3;
    state->x4 = tmp.x4;
    state->x5 = tmp.x5;
    state->x6 = tmp.x6;
    state->x7 = tmp.x7;
}

static inline void change_basis_x2s(BSAES8State *state)
{
    __m128i t46 = xor128(state->x4, state->x6);
    __m128i t35 = xor128(state->x3, state->x5);
    __m128i t06 = xor128(state->x0, state->x6);
    __m128i t357 = xor128(t35, state->x7);

    BSAES8State tmp;
    tmp.x0 = xor128(state->x1, t46);
    tmp.x1 = xor3_128(state->x1, state->x4, state->x5);
    tmp.x2 = xor3_128(state->x2, t35, t06);
    tmp.x3 = xor128(t46, t357);
    tmp.x4 = t357;
    tmp.x5 = t06;
    tmp.x6 = xor128(state->x3, state->x7);
    tmp.x7 = t35;

    state->x0 = tmp.x0;
    state->x1 = tmp.x1;
    state->x2 = tmp.x2;
    state->x3 = tmp.x3;
    state->x4 = tmp.x4;
    state->x5 = tmp.x5;
    state->x6 = tmp.x6;
    state->x7 = tmp.x7;
}

static inline void BS4StateGFMUL(BSAES4State *a, BSAES4State *b, BSAES4State *res)
{
    BSAES2State e, f;
    BSAES2State aa, bb, cc, dd;
    BSAES2State p, q;
    aa.x0 = (a)->x2;
    aa.x1 = (a)->x3;
    bb.x0 = (a)->x0;
    bb.x1 = (a)->x1;
    cc.x0 = (b)->x2;
    cc.x1 = (b)->x3;
    dd.x0 = (b)->x0;
    dd.x1 = (b)->x1;

    BS2StateGFXOR(&cc, &dd, &f);
    BS2StateGFXOR(&aa, &bb, &e);
    BS2StateGFMUL(&e, &f, &f);
    BS2StateGFSCL_N(&f);
    BS2StateGFMUL(&aa, &cc, &p);
    BS2StateGFXOR(&p, &f, &p);
    BS2StateGFMUL(&bb, &dd, &q);
    BS2StateGFXOR(&q, &f, &q);

    res->x0 = q.x0;
    res->x1 = q.x1;
    res->x2 = p.x0;
    res->x3 = p.x1;
}

static inline void BS4StateGFINV(BSAES4State *a)
{
    BSAES2State aa, bb, cc, dd;
    BSAES2State p, q;
    __m128i tmp;

    aa.x0 = a->x2;
    aa.x1 = a->x3;
    bb.x0 = a->x0;
    bb.x1 = a->x1;

    BS2StateGFXOR(&aa, &bb, &cc);
    tmp = cc.x1;
    cc.x1 = cc.x0;
    cc.x0 = tmp;
    BS2StateGFSCL_N(&cc);
    BS2StateGFMUL(&aa, &bb, &dd);
    BS2StateGFXOR(&cc, &dd, &dd);
    BS2StateGFINV(&dd);
    BS2StateGFMUL(&dd, &bb, &p);
    BS2StateGFMUL(&dd, &aa, &q);

    a->x0 = q.x0;
    a->x1 = q.x1;
    a->x2 = p.x0;
    a->x3 = p.x1;
}

static inline void BS8StateGFINV(BSAES8State *a)
{
    BSAES4State aa, bb, cc, dd;
    BSAES4State p, q;

    aa.x0 = (a)->x4;
    aa.x1 = (a)->x5;
    aa.x2 = (a)->x6;
    aa.x3 = (a)->x7;
    bb.x0 = (a)->x0;
    bb.x1 = (a)->x1;
    bb.x2 = (a)->x2;
    bb.x3 = (a)->x3;

    BS4StateGFXOR(&aa, &bb, &cc);
    BS4StateGFSQ_SCL(&cc);
    BS4StateGFMUL(&aa, &bb, &dd);
    BS4StateGFXOR(&cc, &dd, &dd);
    BS4StateGFINV(&dd);
    BS4StateGFMUL(&dd, &bb, &p);
    BS4StateGFMUL(&dd, &aa, &q);

    a->x0 = q.x0;
    a->x1 = q.x1;
    a->x2 = q.x2;
    a->x3 = q.x3;
    a->x4 = p.x0;
    a->x5 = p.x1;
    a->x6 = p.x2;
    a->x7 = p.x3;
}

// static inline BSAES8State *SubBytes(BSAES8State *state) {
static inline void SubBytes(BSAES8State *state)
{
    change_basis_a2x(state);
    BS8StateGFINV(state);
    change_basis_x2s(state);
    BS8StateXOR_X63(state);
}

// static inline BSAES8State *ShiftRows(BSAES8State *state) {
static inline void ShiftRows(BSAES8State *state)
{
    state->x0 = shift_row(state->x0);
    state->x1 = shift_row(state->x1);
    state->x2 = shift_row(state->x2);
    state->x3 = shift_row(state->x3);
    state->x4 = shift_row(state->x4);
    state->x5 = shift_row(state->x5);
    state->x6 = shift_row(state->x6);
    state->x7 = shift_row(state->x7);
}

static inline void MixColumns(BSAES8State *state)
{
    __m128i x0 = state->x0;
    __m128i x1 = state->x1;
    __m128i x2 = state->x2;
    __m128i x3 = state->x3;
    __m128i x4 = state->x4;
    __m128i x5 = state->x5;
    __m128i x6 = state->x6;
    __m128i x7 = state->x7;

    __m128i ror1_x7 = ror1(x7);
    __m128i ror1_x6 = ror1(x6);
    __m128i ror1_x5 = ror1(x5);
    __m128i ror1_x4 = ror1(x4);
    __m128i ror1_x3 = ror1(x3);
    __m128i ror1_x2 = ror1(x2);
    __m128i ror1_x1 = ror1(x1);
    __m128i ror1_x0 = ror1(x0);

    state->x0 = xor4_128(x7, ror1_x7, ror1_x0, ror2(xor128(x0, ror1_x0)));
    state->x1 = xor6_128(x0, ror1_x0, x7, ror1_x7, ror1_x1, ror2(xor128(x1, ror1_x1)));
    state->x2 = xor4_128(x1, ror1_x1, ror1_x2, ror2(xor128(x2, ror1_x2)));
    state->x3 = xor6_128(x2, ror1_x2, x7, ror1_x7, ror1_x3, ror2(xor128(x3, ror1_x3)));
    state->x4 = xor6_128(x3, ror1_x3, x7, ror1_x7, ror1_x4, ror2(xor128(x4, ror1_x4)));
    state->x5 = xor4_128(x4, ror1_x4, ror1_x5, ror2(xor128(x5, ror1_x5)));
    state->x6 = xor4_128(x5, ror1_x5, ror1_x6, ror2(xor128(x6, ror1_x6)));
    state->x7 = xor4_128(x6, ror1_x6, ror1_x7, ror2(xor128(x7, ror1_x7)));
}

void encrypt_core(BSAES8State *state, BSAES8State *rk, BSAES8State *out)
{
    memcpy(out, state, sizeof(BSAES8State));

    AddRoundKey(out, rk);

    for (int i = 1; i <= 9; i++)
    {
        SubBytes(out);
        ShiftRows(out);
        MixColumns(out);
        AddRoundKey(out, rk + i);
    }

    SubBytes(out);
    ShiftRows(out);
    AddRoundKey(out, rk + 10);
}

void aes_key_expansion_sse2(uint8_t *key, __m128i *roundKey)
{
    uint32_t w[44] = {0};
    uint32_t t[4] = {0};

    memcpy(w, key, 4);
    memcpy(w + 1, key + 4, 4);
    memcpy(w + 2, key + 8, 4);
    memcpy(w + 3, key + 12, 4);

    roundKey[0] = _mm_set_epi32(w[0], w[1], w[2], w[3]);
    for (register int i = 4; i < 44; i++)
    {
        if (!(i & 3))
        { // if i%4==0
            // Note: suppose w[i - 1] is aa bb cc dd (from lowest bit to highest bit, i.e., aa is the lowest byte， the origin value is 0xddccbbaa),
            //       then t[i] = g(w[i-1], rc[i]) should be s[bb]^rc[i] s[cc] s[dd] s[aa]
            w[i] = ((S[(w[i - 1] >> 8) & 0xff] ^ RC[i / 4 - 1]) |
                    S[(w[i - 1] >> 16) & 0xff] << 8 |
                    S[w[i - 1] >> 24] << 16 |
                    S[w[i - 1] & 0xff] << 24) ^
                   w[i - 4];
        }
        else
        {
            w[i] = w[i - 1] ^ w[i - 4];
            if (i & 1 & (i >> 1)) // if i%4==3
                roundKey[i >> 2] = _mm_set_epi32(w[i - 3], w[i - 2], w[i - 1], w[i]);
        }
    }

    // reverse(because of SSE2's style of storing)
    for (int i = 0; i < 11; i++)
    {
        uint32_t *s = (uint32_t *)&roundKey[i];
        for (int j = 0; j < 4; j++)
            t[j] = s[3 - j];
        roundKey[i] = _mm_loadu_si128((__m128i *)t);
    }

    return;
}
