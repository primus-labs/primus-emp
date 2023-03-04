
/*
 * 1. according to crypto/aes/aes_core.c
 * 2. add my_ for solving conflicts
*/

#ifndef __AES_CORE_H
#define __AES_CORE_H

#include <assert.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

# define AES_MAXNR 14
# define AES_BLOCK_SIZE 16

/* This should be a hidden type, but EVP requires that the size be known */
struct my_aes_key_st {
# ifdef AES_LONG
    unsigned long rd_key[4 * (AES_MAXNR + 1)];
# else
    unsigned int rd_key[4 * (AES_MAXNR + 1)];
# endif
    int rounds;
};
typedef struct my_aes_key_st MY_AES_KEY;


typedef uint32_t u32;
typedef uint64_t u64;


# if (defined(_WIN32) || defined(_WIN64)) && !defined(__MINGW32__)
#  define U64(C) C##UI64
# elif defined(__arch64__)
#  define U64(C) C##UL
# else
#  define U64(C) C##ULL
# endif

typedef union {
    unsigned char b[8];
    u32 w[2];
    u64 d;
} uni;

/*
 * Compute w := (w * x) mod (x^8 + x^4 + x^3 + x^1 + 1)
 * Therefore the name "xtime".
 */
static void XtimeWord(u32 *w)
{
    u32 a, b;

    a = *w;
    b = a & 0x80808080u;
    a ^= b;
    b -= b >> 7;
    b &= 0x1B1B1B1Bu;
    b ^= a << 1;
    *w = b;
}

static void XtimeLong(u64 *w)
{
    u64 a, b;

    a = *w;
    b = a & U64(0x8080808080808080);
    a ^= b;
    b -= b >> 7;
    b &= U64(0x1B1B1B1B1B1B1B1B);
    b ^= a << 1;
    *w = b;
}

/*
 * This computes w := S * w ^ -1 + c, where c = {01100011}.
 * Instead of using GF(2^8) mod (x^8+x^4+x^3+x+1} we do the inversion
 * in GF(GF(GF(2^2)^2)^2) mod (X^2+X+8)
 * and GF(GF(2^2)^2) mod (X^2+X+2)
 * and GF(2^2) mod (X^2+X+1)
 * The first part of the algorithm below transfers the coordinates
 * {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80} =>
 * {1,Y,Y^2,Y^3,Y^4,Y^5,Y^6,Y^7} with Y=0x41:
 * {0x01,0x41,0x66,0x6c,0x56,0x9a,0x58,0xc4}
 * The last part undoes the coordinate transfer and the final affine
 * transformation S:
 * b[i] = b[i] + b[(i+4)%8] + b[(i+5)%8] + b[(i+6)%8] + b[(i+7)%8] + c[i]
 * in one step.
 * The multiplication in GF(2^2^2^2) is done in ordinary coords:
 * A = (a0*1 + a1*x^4)
 * B = (b0*1 + b1*x^4)
 * AB = ((a0*b0 + 8*a1*b1)*1 + (a1*b0 + (a0+a1)*b1)*x^4)
 * When A = (a0,a1) is given we want to solve AB = 1:
 * (a) 1 = a0*b0 + 8*a1*b1
 * (b) 0 = a1*b0 + (a0+a1)*b1
 * => multiply (a) by a1 and (b) by a0
 * (c) a1 = a1*a0*b0 + (8*a1*a1)*b1
 * (d) 0 = a1*a0*b0 + (a0*a0+a1*a0)*b1
 * => add (c) + (d)
 * (e) a1 = (a0*a0 + a1*a0 + 8*a1*a1)*b1
 * => therefore
 * b1 = (a0*a0 + a1*a0 + 8*a1*a1)^-1 * a1
 * => and adding (a1*b0) to (b) we get
 * (f) a1*b0 = (a0+a1)*b1
 * => therefore
 * b0 = (a0*a0 + a1*a0 + 8*a1*a1)^-1 * (a0+a1)
 * Note this formula also works for the case
 * (a0+a1)*a0 + 8*a1*a1 = 0
 * if the inverse element for 0^-1 is mapped to 0.
 * Repeat the same for GF(2^2^2) and GF(2^2).
 * We get the following algorithm:
 * inv8(a0,a1):
 *   x0 = a0^a1
 *   [y0,y1] = mul4([x0,a1],[a0,a1]); (*)
 *   y1 = mul4(8,y1);
 *   t = inv4(y0^y1);
 *   [b0,b1] = mul4([x0,a1],[t,t]); (*)
 *   return [b0,b1];
 * The non-linear multiplies (*) can be done in parallel at no extra cost.
 */
static void SubWord(u32 *w)
{
    u32 x, y, a1, a2, a3, a4, a5, a6;

    x = *w;
    y = ((x & 0xFEFEFEFEu) >> 1) | ((x & 0x01010101u) << 7);
    x &= 0xDDDDDDDDu;
    x ^= y & 0x57575757u;
    y = ((y & 0xFEFEFEFEu) >> 1) | ((y & 0x01010101u) << 7);
    x ^= y & 0x1C1C1C1Cu;
    y = ((y & 0xFEFEFEFEu) >> 1) | ((y & 0x01010101u) << 7);
    x ^= y & 0x4A4A4A4Au;
    y = ((y & 0xFEFEFEFEu) >> 1) | ((y & 0x01010101u) << 7);
    x ^= y & 0x42424242u;
    y = ((y & 0xFEFEFEFEu) >> 1) | ((y & 0x01010101u) << 7);
    x ^= y & 0x64646464u;
    y = ((y & 0xFEFEFEFEu) >> 1) | ((y & 0x01010101u) << 7);
    x ^= y & 0xE0E0E0E0u;
    a1 = x;
    a1 ^= (x & 0xF0F0F0F0u) >> 4;
    a2 = ((x & 0xCCCCCCCCu) >> 2) | ((x & 0x33333333u) << 2);
    a3 = x & a1;
    a3 ^= (a3 & 0xAAAAAAAAu) >> 1;
    a3 ^= (((x << 1) & a1) ^ ((a1 << 1) & x)) & 0xAAAAAAAAu;
    a4 = a2 & a1;
    a4 ^= (a4 & 0xAAAAAAAAu) >> 1;
    a4 ^= (((a2 << 1) & a1) ^ ((a1 << 1) & a2)) & 0xAAAAAAAAu;
    a5 = (a3 & 0xCCCCCCCCu) >> 2;
    a3 ^= ((a4 << 2) ^ a4) & 0xCCCCCCCCu;
    a4 = a5 & 0x22222222u;
    a4 |= a4 >> 1;
    a4 ^= (a5 << 1) & 0x22222222u;
    a3 ^= a4;
    a5 = a3 & 0xA0A0A0A0u;
    a5 |= a5 >> 1;
    a5 ^= (a3 << 1) & 0xA0A0A0A0u;
    a4 = a5 & 0xC0C0C0C0u;
    a6 = a4 >> 2;
    a4 ^= (a5 << 2) & 0xC0C0C0C0u;
    a5 = a6 & 0x20202020u;
    a5 |= a5 >> 1;
    a5 ^= (a6 << 1) & 0x20202020u;
    a4 |= a5;
    a3 ^= a4 >> 4;
    a3 &= 0x0F0F0F0Fu;
    a2 = a3;
    a2 ^= (a3 & 0x0C0C0C0Cu) >> 2;
    a4 = a3 & a2;
    a4 ^= (a4 & 0x0A0A0A0A0Au) >> 1;
    a4 ^= (((a3 << 1) & a2) ^ ((a2 << 1) & a3)) & 0x0A0A0A0Au;
    a5 = a4 & 0x08080808u;
    a5 |= a5 >> 1;
    a5 ^= (a4 << 1) & 0x08080808u;
    a4 ^= a5 >> 2;
    a4 &= 0x03030303u;
    a4 ^= (a4 & 0x02020202u) >> 1;
    a4 |= a4 << 2;
    a3 = a2 & a4;
    a3 ^= (a3 & 0x0A0A0A0Au) >> 1;
    a3 ^= (((a2 << 1) & a4) ^ ((a4 << 1) & a2)) & 0x0A0A0A0Au;
    a3 |= a3 << 4;
    a2 = ((a1 & 0xCCCCCCCCu) >> 2) | ((a1 & 0x33333333u) << 2);
    x = a1 & a3;
    x ^= (x & 0xAAAAAAAAu) >> 1;
    x ^= (((a1 << 1) & a3) ^ ((a3 << 1) & a1)) & 0xAAAAAAAAu;
    a4 = a2 & a3;
    a4 ^= (a4 & 0xAAAAAAAAu) >> 1;
    a4 ^= (((a2 << 1) & a3) ^ ((a3 << 1) & a2)) & 0xAAAAAAAAu;
    a5 = (x & 0xCCCCCCCCu) >> 2;
    x ^= ((a4 << 2) ^ a4) & 0xCCCCCCCCu;
    a4 = a5 & 0x22222222u;
    a4 |= a4 >> 1;
    a4 ^= (a5 << 1) & 0x22222222u;
    x ^= a4;
    y = ((x & 0xFEFEFEFEu) >> 1) | ((x & 0x01010101u) << 7);
    x &= 0x39393939u;
    x ^= y & 0x3F3F3F3Fu;
    y = ((y & 0xFCFCFCFCu) >> 2) | ((y & 0x03030303u) << 6);
    x ^= y & 0x97979797u;
    y = ((y & 0xFEFEFEFEu) >> 1) | ((y & 0x01010101u) << 7);
    x ^= y & 0x9B9B9B9Bu;
    y = ((y & 0xFEFEFEFEu) >> 1) | ((y & 0x01010101u) << 7);
    x ^= y & 0x3C3C3C3Cu;
    y = ((y & 0xFEFEFEFEu) >> 1) | ((y & 0x01010101u) << 7);
    x ^= y & 0xDDDDDDDDu;
    y = ((y & 0xFEFEFEFEu) >> 1) | ((y & 0x01010101u) << 7);
    x ^= y & 0x72727272u;
    x ^= 0x63636363u;
    *w = x;
}

static void SubLong(u64 *w)
{
    u64 x, y, a1, a2, a3, a4, a5, a6;

    x = *w;
    y = ((x & U64(0xFEFEFEFEFEFEFEFE)) >> 1) | ((x & U64(0x0101010101010101)) << 7);
    x &= U64(0xDDDDDDDDDDDDDDDD);
    x ^= y & U64(0x5757575757575757);
    y = ((y & U64(0xFEFEFEFEFEFEFEFE)) >> 1) | ((y & U64(0x0101010101010101)) << 7);
    x ^= y & U64(0x1C1C1C1C1C1C1C1C);
    y = ((y & U64(0xFEFEFEFEFEFEFEFE)) >> 1) | ((y & U64(0x0101010101010101)) << 7);
    x ^= y & U64(0x4A4A4A4A4A4A4A4A);
    y = ((y & U64(0xFEFEFEFEFEFEFEFE)) >> 1) | ((y & U64(0x0101010101010101)) << 7);
    x ^= y & U64(0x4242424242424242);
    y = ((y & U64(0xFEFEFEFEFEFEFEFE)) >> 1) | ((y & U64(0x0101010101010101)) << 7);
    x ^= y & U64(0x6464646464646464);
    y = ((y & U64(0xFEFEFEFEFEFEFEFE)) >> 1) | ((y & U64(0x0101010101010101)) << 7);
    x ^= y & U64(0xE0E0E0E0E0E0E0E0);
    a1 = x;
    a1 ^= (x & U64(0xF0F0F0F0F0F0F0F0)) >> 4;
    a2 = ((x & U64(0xCCCCCCCCCCCCCCCC)) >> 2) | ((x & U64(0x3333333333333333)) << 2);
    a3 = x & a1;
    a3 ^= (a3 & U64(0xAAAAAAAAAAAAAAAA)) >> 1;
    a3 ^= (((x << 1) & a1) ^ ((a1 << 1) & x)) & U64(0xAAAAAAAAAAAAAAAA);
    a4 = a2 & a1;
    a4 ^= (a4 & U64(0xAAAAAAAAAAAAAAAA)) >> 1;
    a4 ^= (((a2 << 1) & a1) ^ ((a1 << 1) & a2)) & U64(0xAAAAAAAAAAAAAAAA);
    a5 = (a3 & U64(0xCCCCCCCCCCCCCCCC)) >> 2;
    a3 ^= ((a4 << 2) ^ a4) & U64(0xCCCCCCCCCCCCCCCC);
    a4 = a5 & U64(0x2222222222222222);
    a4 |= a4 >> 1;
    a4 ^= (a5 << 1) & U64(0x2222222222222222);
    a3 ^= a4;
    a5 = a3 & U64(0xA0A0A0A0A0A0A0A0);
    a5 |= a5 >> 1;
    a5 ^= (a3 << 1) & U64(0xA0A0A0A0A0A0A0A0);
    a4 = a5 & U64(0xC0C0C0C0C0C0C0C0);
    a6 = a4 >> 2;
    a4 ^= (a5 << 2) & U64(0xC0C0C0C0C0C0C0C0);
    a5 = a6 & U64(0x2020202020202020);
    a5 |= a5 >> 1;
    a5 ^= (a6 << 1) & U64(0x2020202020202020);
    a4 |= a5;
    a3 ^= a4 >> 4;
    a3 &= U64(0x0F0F0F0F0F0F0F0F);
    a2 = a3;
    a2 ^= (a3 & U64(0x0C0C0C0C0C0C0C0C)) >> 2;
    a4 = a3 & a2;
    a4 ^= (a4 & U64(0x0A0A0A0A0A0A0A0A)) >> 1;
    a4 ^= (((a3 << 1) & a2) ^ ((a2 << 1) & a3)) & U64(0x0A0A0A0A0A0A0A0A);
    a5 = a4 & U64(0x0808080808080808);
    a5 |= a5 >> 1;
    a5 ^= (a4 << 1) & U64(0x0808080808080808);
    a4 ^= a5 >> 2;
    a4 &= U64(0x0303030303030303);
    a4 ^= (a4 & U64(0x0202020202020202)) >> 1;
    a4 |= a4 << 2;
    a3 = a2 & a4;
    a3 ^= (a3 & U64(0x0A0A0A0A0A0A0A0A)) >> 1;
    a3 ^= (((a2 << 1) & a4) ^ ((a4 << 1) & a2)) & U64(0x0A0A0A0A0A0A0A0A);
    a3 |= a3 << 4;
    a2 = ((a1 & U64(0xCCCCCCCCCCCCCCCC)) >> 2) | ((a1 & U64(0x3333333333333333)) << 2);
    x = a1 & a3;
    x ^= (x & U64(0xAAAAAAAAAAAAAAAA)) >> 1;
    x ^= (((a1 << 1) & a3) ^ ((a3 << 1) & a1)) & U64(0xAAAAAAAAAAAAAAAA);
    a4 = a2 & a3;
    a4 ^= (a4 & U64(0xAAAAAAAAAAAAAAAA)) >> 1;
    a4 ^= (((a2 << 1) & a3) ^ ((a3 << 1) & a2)) & U64(0xAAAAAAAAAAAAAAAA);
    a5 = (x & U64(0xCCCCCCCCCCCCCCCC)) >> 2;
    x ^= ((a4 << 2) ^ a4) & U64(0xCCCCCCCCCCCCCCCC);
    a4 = a5 & U64(0x2222222222222222);
    a4 |= a4 >> 1;
    a4 ^= (a5 << 1) & U64(0x2222222222222222);
    x ^= a4;
    y = ((x & U64(0xFEFEFEFEFEFEFEFE)) >> 1) | ((x & U64(0x0101010101010101)) << 7);
    x &= U64(0x3939393939393939);
    x ^= y & U64(0x3F3F3F3F3F3F3F3F);
    y = ((y & U64(0xFCFCFCFCFCFCFCFC)) >> 2) | ((y & U64(0x0303030303030303)) << 6);
    x ^= y & U64(0x9797979797979797);
    y = ((y & U64(0xFEFEFEFEFEFEFEFE)) >> 1) | ((y & U64(0x0101010101010101)) << 7);
    x ^= y & U64(0x9B9B9B9B9B9B9B9B);
    y = ((y & U64(0xFEFEFEFEFEFEFEFE)) >> 1) | ((y & U64(0x0101010101010101)) << 7);
    x ^= y & U64(0x3C3C3C3C3C3C3C3C);
    y = ((y & U64(0xFEFEFEFEFEFEFEFE)) >> 1) | ((y & U64(0x0101010101010101)) << 7);
    x ^= y & U64(0xDDDDDDDDDDDDDDDD);
    y = ((y & U64(0xFEFEFEFEFEFEFEFE)) >> 1) | ((y & U64(0x0101010101010101)) << 7);
    x ^= y & U64(0x7272727272727272);
    x ^= U64(0x6363636363636363);
    *w = x;
}

/*
 * This computes w := (S^-1 * (w + c))^-1
 */
static void InvSubLong(u64 *w)
{
    u64 x, y, a1, a2, a3, a4, a5, a6;

    x = *w;
    x ^= U64(0x6363636363636363);
    y = ((x & U64(0xFEFEFEFEFEFEFEFE)) >> 1) | ((x & U64(0x0101010101010101)) << 7);
    x &= U64(0xFDFDFDFDFDFDFDFD);
    x ^= y & U64(0x5E5E5E5E5E5E5E5E);
    y = ((y & U64(0xFEFEFEFEFEFEFEFE)) >> 1) | ((y & U64(0x0101010101010101)) << 7);
    x ^= y & U64(0xF3F3F3F3F3F3F3F3);
    y = ((y & U64(0xFEFEFEFEFEFEFEFE)) >> 1) | ((y & U64(0x0101010101010101)) << 7);
    x ^= y & U64(0xF5F5F5F5F5F5F5F5);
    y = ((y & U64(0xFEFEFEFEFEFEFEFE)) >> 1) | ((y & U64(0x0101010101010101)) << 7);
    x ^= y & U64(0x7878787878787878);
    y = ((y & U64(0xFEFEFEFEFEFEFEFE)) >> 1) | ((y & U64(0x0101010101010101)) << 7);
    x ^= y & U64(0x7777777777777777);
    y = ((y & U64(0xFEFEFEFEFEFEFEFE)) >> 1) | ((y & U64(0x0101010101010101)) << 7);
    x ^= y & U64(0x1515151515151515);
    y = ((y & U64(0xFEFEFEFEFEFEFEFE)) >> 1) | ((y & U64(0x0101010101010101)) << 7);
    x ^= y & U64(0xA5A5A5A5A5A5A5A5);
    a1 = x;
    a1 ^= (x & U64(0xF0F0F0F0F0F0F0F0)) >> 4;
    a2 = ((x & U64(0xCCCCCCCCCCCCCCCC)) >> 2) | ((x & U64(0x3333333333333333)) << 2);
    a3 = x & a1;
    a3 ^= (a3 & U64(0xAAAAAAAAAAAAAAAA)) >> 1;
    a3 ^= (((x << 1) & a1) ^ ((a1 << 1) & x)) & U64(0xAAAAAAAAAAAAAAAA);
    a4 = a2 & a1;
    a4 ^= (a4 & U64(0xAAAAAAAAAAAAAAAA)) >> 1;
    a4 ^= (((a2 << 1) & a1) ^ ((a1 << 1) & a2)) & U64(0xAAAAAAAAAAAAAAAA);
    a5 = (a3 & U64(0xCCCCCCCCCCCCCCCC)) >> 2;
    a3 ^= ((a4 << 2) ^ a4) & U64(0xCCCCCCCCCCCCCCCC);
    a4 = a5 & U64(0x2222222222222222);
    a4 |= a4 >> 1;
    a4 ^= (a5 << 1) & U64(0x2222222222222222);
    a3 ^= a4;
    a5 = a3 & U64(0xA0A0A0A0A0A0A0A0);
    a5 |= a5 >> 1;
    a5 ^= (a3 << 1) & U64(0xA0A0A0A0A0A0A0A0);
    a4 = a5 & U64(0xC0C0C0C0C0C0C0C0);
    a6 = a4 >> 2;
    a4 ^= (a5 << 2) & U64(0xC0C0C0C0C0C0C0C0);
    a5 = a6 & U64(0x2020202020202020);
    a5 |= a5 >> 1;
    a5 ^= (a6 << 1) & U64(0x2020202020202020);
    a4 |= a5;
    a3 ^= a4 >> 4;
    a3 &= U64(0x0F0F0F0F0F0F0F0F);
    a2 = a3;
    a2 ^= (a3 & U64(0x0C0C0C0C0C0C0C0C)) >> 2;
    a4 = a3 & a2;
    a4 ^= (a4 & U64(0x0A0A0A0A0A0A0A0A)) >> 1;
    a4 ^= (((a3 << 1) & a2) ^ ((a2 << 1) & a3)) & U64(0x0A0A0A0A0A0A0A0A);
    a5 = a4 & U64(0x0808080808080808);
    a5 |= a5 >> 1;
    a5 ^= (a4 << 1) & U64(0x0808080808080808);
    a4 ^= a5 >> 2;
    a4 &= U64(0x0303030303030303);
    a4 ^= (a4 & U64(0x0202020202020202)) >> 1;
    a4 |= a4 << 2;
    a3 = a2 & a4;
    a3 ^= (a3 & U64(0x0A0A0A0A0A0A0A0A)) >> 1;
    a3 ^= (((a2 << 1) & a4) ^ ((a4 << 1) & a2)) & U64(0x0A0A0A0A0A0A0A0A);
    a3 |= a3 << 4;
    a2 = ((a1 & U64(0xCCCCCCCCCCCCCCCC)) >> 2) | ((a1 & U64(0x3333333333333333)) << 2);
    x = a1 & a3;
    x ^= (x & U64(0xAAAAAAAAAAAAAAAA)) >> 1;
    x ^= (((a1 << 1) & a3) ^ ((a3 << 1) & a1)) & U64(0xAAAAAAAAAAAAAAAA);
    a4 = a2 & a3;
    a4 ^= (a4 & U64(0xAAAAAAAAAAAAAAAA)) >> 1;
    a4 ^= (((a2 << 1) & a3) ^ ((a3 << 1) & a2)) & U64(0xAAAAAAAAAAAAAAAA);
    a5 = (x & U64(0xCCCCCCCCCCCCCCCC)) >> 2;
    x ^= ((a4 << 2) ^ a4) & U64(0xCCCCCCCCCCCCCCCC);
    a4 = a5 & U64(0x2222222222222222);
    a4 |= a4 >> 1;
    a4 ^= (a5 << 1) & U64(0x2222222222222222);
    x ^= a4;
    y = ((x & U64(0xFEFEFEFEFEFEFEFE)) >> 1) | ((x & U64(0x0101010101010101)) << 7);
    x &= U64(0xB5B5B5B5B5B5B5B5);
    x ^= y & U64(0x4040404040404040);
    y = ((y & U64(0xFEFEFEFEFEFEFEFE)) >> 1) | ((y & U64(0x0101010101010101)) << 7);
    x ^= y & U64(0x8080808080808080);
    y = ((y & U64(0xFEFEFEFEFEFEFEFE)) >> 1) | ((y & U64(0x0101010101010101)) << 7);
    x ^= y & U64(0x1616161616161616);
    y = ((y & U64(0xFEFEFEFEFEFEFEFE)) >> 1) | ((y & U64(0x0101010101010101)) << 7);
    x ^= y & U64(0xEBEBEBEBEBEBEBEB);
    y = ((y & U64(0xFEFEFEFEFEFEFEFE)) >> 1) | ((y & U64(0x0101010101010101)) << 7);
    x ^= y & U64(0x9797979797979797);
    y = ((y & U64(0xFEFEFEFEFEFEFEFE)) >> 1) | ((y & U64(0x0101010101010101)) << 7);
    x ^= y & U64(0xFBFBFBFBFBFBFBFB);
    y = ((y & U64(0xFEFEFEFEFEFEFEFE)) >> 1) | ((y & U64(0x0101010101010101)) << 7);
    x ^= y & U64(0x7D7D7D7D7D7D7D7D);
    *w = x;
}

static void ShiftRows(u64 *state)
{
    unsigned char s[4];
    unsigned char *s0;
    int r;

    s0 = (unsigned char *)state;
    for (r = 0; r < 4; r++) {
        s[0] = s0[0*4 + r];
        s[1] = s0[1*4 + r];
        s[2] = s0[2*4 + r];
        s[3] = s0[3*4 + r];
        s0[0*4 + r] = s[(r+0) % 4];
        s0[1*4 + r] = s[(r+1) % 4];
        s0[2*4 + r] = s[(r+2) % 4];
        s0[3*4 + r] = s[(r+3) % 4];
    }
}

static void InvShiftRows(u64 *state)
{
    unsigned char s[4];
    unsigned char *s0;
    int r;

    s0 = (unsigned char *)state;
    for (r = 0; r < 4; r++) {
        s[0] = s0[0*4 + r];
        s[1] = s0[1*4 + r];
        s[2] = s0[2*4 + r];
        s[3] = s0[3*4 + r];
        s0[0*4 + r] = s[(4-r) % 4];
        s0[1*4 + r] = s[(5-r) % 4];
        s0[2*4 + r] = s[(6-r) % 4];
        s0[3*4 + r] = s[(7-r) % 4];
    }
}

static void MixColumns(u64 *state)
{
    uni s1;
    uni s;
    int c;

    for (c = 0; c < 2; c++) {
        s1.d = state[c];
        s.d = s1.d;
        s.d ^= ((s.d & U64(0xFFFF0000FFFF0000)) >> 16)
               | ((s.d & U64(0x0000FFFF0000FFFF)) << 16);
        s.d ^= ((s.d & U64(0xFF00FF00FF00FF00)) >> 8)
               | ((s.d & U64(0x00FF00FF00FF00FF)) << 8);
        s.d ^= s1.d;
        XtimeLong(&s1.d);
        s.d ^= s1.d;
        s.b[0] ^= s1.b[1];
        s.b[1] ^= s1.b[2];
        s.b[2] ^= s1.b[3];
        s.b[3] ^= s1.b[0];
        s.b[4] ^= s1.b[5];
        s.b[5] ^= s1.b[6];
        s.b[6] ^= s1.b[7];
        s.b[7] ^= s1.b[4];
        state[c] = s.d;
    }
}

static void InvMixColumns(u64 *state)
{
    uni s1;
    uni s;
    int c;

    for (c = 0; c < 2; c++) {
        s1.d = state[c];
        s.d = s1.d;
        s.d ^= ((s.d & U64(0xFFFF0000FFFF0000)) >> 16)
               | ((s.d & U64(0x0000FFFF0000FFFF)) << 16);
        s.d ^= ((s.d & U64(0xFF00FF00FF00FF00)) >> 8)
               | ((s.d & U64(0x00FF00FF00FF00FF)) << 8);
        s.d ^= s1.d;
        XtimeLong(&s1.d);
        s.d ^= s1.d;
        s.b[0] ^= s1.b[1];
        s.b[1] ^= s1.b[2];
        s.b[2] ^= s1.b[3];
        s.b[3] ^= s1.b[0];
        s.b[4] ^= s1.b[5];
        s.b[5] ^= s1.b[6];
        s.b[6] ^= s1.b[7];
        s.b[7] ^= s1.b[4];
        XtimeLong(&s1.d);
        s1.d ^= ((s1.d & U64(0xFFFF0000FFFF0000)) >> 16)
                | ((s1.d & U64(0x0000FFFF0000FFFF)) << 16);
        s.d ^= s1.d;
        XtimeLong(&s1.d);
        s1.d ^= ((s1.d & U64(0xFF00FF00FF00FF00)) >> 8)
                | ((s1.d & U64(0x00FF00FF00FF00FF)) << 8);
        s.d ^= s1.d;
        state[c] = s.d;
    }
}

static void _my_mm_aesimc_si128(u64 *state)
{
    InvMixColumns(state);
}

static void AddRoundKey(u64 *state, const u64 *w)
{
    state[0] ^= w[0];
    state[1] ^= w[1];
}

static void Cipher(const unsigned char *in, unsigned char *out,
                   const u64 *w, int nr)
{
    u64 state[2];
    int i;

    memcpy(state, in, 16);

    AddRoundKey(state, w);

    for (i = 1; i < nr; i++) {
        SubLong(&state[0]);
        SubLong(&state[1]);
        ShiftRows(state);
        MixColumns(state);
        AddRoundKey(state, w + i*2);
    }

    SubLong(&state[0]);
    SubLong(&state[1]);
    ShiftRows(state);
    AddRoundKey(state, w + nr*2);

    memcpy(out, state, 16);
}

static void _my_mm_aesenc_si128_o(const unsigned char *in, unsigned char *out,
                   const u64 *key)
{
    // u64 state[2];
    u64* state = (u64*)out;
    
    memcpy(state, in, 16);

    SubLong(&state[0]);
    SubLong(&state[1]);
    ShiftRows(state);
    MixColumns(state);
    AddRoundKey(state, key);

    // memcpy(out, state, 16);
}

static void _my_mm_aesenc_si128(const unsigned char *in, unsigned char *out,
                   const u64 *key)
{
    u64 state[2];
    memcpy(state, in, 16);

    SubLong(&state[0]);
    SubLong(&state[1]);
    ShiftRows(state);
    MixColumns(state);
    AddRoundKey(state, key);

    memcpy(out, state, 16);
}

static void _my_mm_aesenclast_si128(const unsigned char *in, unsigned char *out,
                   const u64 *key)
{
    u64 state[2];

    memcpy(state, in, 16);

    SubLong(&state[0]);
    SubLong(&state[1]);
    ShiftRows(state);
    AddRoundKey(state, key);

    memcpy(out, state, 16);
}

static void InvCipher(const unsigned char *in, unsigned char *out,
                      const u64 *w, int nr)

{
  
    // for (int k=nr; k>=0; k--){
    //   const uint64_t *data = w + k*2;
    //   cout << "+first keyxxxx:" << k << " " << std::hex << std::setw(16) << std::setfill('0')
    //        << data[1] << " " << std::setw(16) << std::setfill('0') << data[0]
    //        << std::dec << std::setw(0) << endl;
    // }

    u64 state[2];
    int i;


    // {
    //   const uint64_t *data = w + nr*2;
    //   cout << "-key:" << std::hex << std::setw(16) << std::setfill('0')
    //        << data[1] << " " << std::setw(16) << std::setfill('0') << data[0]
    //        << std::dec << std::setw(0) << endl;
    // }

    // {
    //   cout << endl;
    //   uint64_t *data = (uint64_t *)in;
    //   cout << "-in:" << std::hex << std::setw(16) << std::setfill('0')
    //        << data[1] << " " << std::setw(16) << std::setfill('0') << data[0]
    //        << std::dec << std::setw(0) << endl;
    // }

    memcpy(state, in, 16);

    AddRoundKey(state, w + nr*2);
    // {
    //   cout << "-out:" << std::hex << std::setw(16) << std::setfill('0')
    //        << state[1] << " " << std::setw(16) << std::setfill('0') << state[0]
    //        << std::dec << std::setw(0) << endl;
    // }


    for (i = nr - 1; i > 0; i--) {
        // {
        //   const uint64_t *data = w + i*2;
        //   cout << "\n[= key["<<i<<"]:" << std::hex << std::setw(16) << std::setfill('0')
        //       << data[1] << " " << std::setw(16) << std::setfill('0') << data[0]
        //       << std::dec << std::setw(0) << endl;
        // }
        // {
        //   cout << "[= in:" << std::hex << std::setw(16) << std::setfill('0')
        //       << state[1] << " " << std::setw(16) << std::setfill('0') << state[0]
        //       << std::dec << std::setw(0) << endl;
        // }

        InvShiftRows(state);
        InvSubLong(&state[0]);
        InvSubLong(&state[1]);
        AddRoundKey(state, w + i*2);
        InvMixColumns(state);
        // {
        //   cout << "-out:" << std::hex << std::setw(16) << std::setfill('0')
        //       << state[1] << " " << std::setw(16) << std::setfill('0') << state[0]
        //       << std::dec << std::setw(0) << endl;
        // }
    }

    InvShiftRows(state);
    InvSubLong(&state[0]);
    InvSubLong(&state[1]);
    AddRoundKey(state, w);

    memcpy(out, state, 16);
}


static void _my_mm_aesdec_si128(const unsigned char *in, unsigned char *out,
                   const u64 *key)

{
    u64 state[2];

    memcpy(state, in, 16);

    // {
    //   const uint64_t *data = key;
    //   cout << "[= key:" << std::hex << std::setw(16) << std::setfill('0')
    //        << data[1] << " " << std::setw(16) << std::setfill('0') << data[0]
    //        << std::dec << std::setw(0) << endl;
    // }
    // {
    //   cout << "[= in:" << std::hex << std::setw(16) << std::setfill('0')
    //        << state[1] << " " << std::setw(16) << std::setfill('0') << state[0]
    //        << std::dec << std::setw(0) << endl;
    // }


    InvShiftRows(state);
    InvSubLong(&state[0]);
    InvSubLong(&state[1]);
    InvMixColumns(state);
    AddRoundKey(state, key);

    memcpy(out, state, 16);
    
    // {
    //   uint64_t *data = (uint64_t *)out;
    //   cout << "[= out:" << std::hex << std::setw(16) << std::setfill('0')
    //        << data[1] << " " << std::setw(16) << std::setfill('0') << data[0]
    //        << std::dec << std::setw(0) << endl;
    // }
}

static void _my_mm_aesdeclast_si128(const unsigned char *in, unsigned char *out,
                   const u64 *key)

{
    u64 state[2];

    memcpy(state, in, 16);

    InvShiftRows(state);
    InvSubLong(&state[0]);
    InvSubLong(&state[1]);
    AddRoundKey(state, key);

    memcpy(out, state, 16);
}

static void RotWord(u32 *x)
{
    unsigned char *w0;
    unsigned char tmp;

    w0 = (unsigned char *)x;
    tmp = w0[0];
    w0[0] = w0[1];
    w0[1] = w0[2];
    w0[2] = w0[3];
    w0[3] = tmp;
}

static void KeyExpansion(const unsigned char *key, u64 *w,
                         int nr, int nk)
{
    u32 rcon;
    uni prev;
    u32 temp;
    int i, n;

    memcpy(w, key, nk*4);
    memcpy(&rcon, "\1\0\0\0", 4);
    n = nk/2;
    prev.d = w[n-1];
    for (i = n; i < (nr+1)*2; i++) {
        temp = prev.w[1];
        if (i % n == 0) {
            RotWord(&temp);
            SubWord(&temp);
            temp ^= rcon;
            XtimeWord(&rcon);
        } else if (nk > 6 && i % n == 2) {
            SubWord(&temp);
        }
        prev.d = w[i-n];
        prev.w[0] ^= temp;
        prev.w[1] ^= prev.w[0];
        w[i] = prev.d;
    }
}

/**
 * Expand the cipher key into the encryption key schedule.
 */
static int my_AES_set_encrypt_key(const unsigned char *userKey, const int bits,
                        MY_AES_KEY *key)
{
    u64 *rk;

    if (!userKey || !key)
        return -1;
    if (bits != 128 && bits != 192 && bits != 256)
        return -2;

    rk = (u64*)key->rd_key;

    if (bits == 128)
        key->rounds = 10;
    else if (bits == 192)
        key->rounds = 12;
    else
        key->rounds = 14;

    KeyExpansion(userKey, rk, key->rounds, bits/32);
    return 0;
}


// userKey: userKey
// rk: (u64*)key->rd_key
// nr: rounds
// nk: bits/32
static void _my_KeyExpansion(const unsigned char *userKey, u64 *rk, int nr, int nk)
{
    KeyExpansion(userKey, rk, nr, nk);
}


/**
 * Expand the cipher key into the decryption key schedule.
 */
static int my_AES_set_decrypt_key(const unsigned char *userKey, const int bits,
                        MY_AES_KEY *key)
{
    return my_AES_set_encrypt_key(userKey, bits, key);
}

/*
 * Encrypt a single block
 * in and out can overlap
 */
static void my_AES_encrypt(const unsigned char *in, unsigned char *out,
                 const MY_AES_KEY *key)
{
    const u64 *rk;

    assert(in && out && key);
    rk = (u64*)key->rd_key;

    Cipher(in, out, rk, key->rounds);
}

/*
 * Decrypt a single block
 * in and out can overlap
 */
static void my_AES_decrypt(const unsigned char *in, unsigned char *out,
                 const MY_AES_KEY *key)
{
    const u64 *rk;

    assert(in && out && key);
    rk = (u64*)key->rd_key;

    InvCipher(in, out, rk, key->rounds);
}


#endif
