
#include "emp-tool/utils/block.h"
#include <openssl/aes.h>

#include <iostream>
using namespace std;
using namespace emp;

// for emp::AES_KEY conflict with openssl::AES_KEY
typedef struct
{
    block rd_key[11];
    unsigned int rounds;
} EMP_AES_KEY;

#ifdef __x86_64__
__attribute__((target("aes,sse2")))
#elif __EMSCRIPTEN__
__attribute__((target("simd128")))
#endif
// void _AES_ecb_encrypt_blks(block *blks, unsigned int nblks, const EMP_AES_KEY *key) {
void
_AES_ecb_encrypt_blks(unsigned char *_blks, unsigned int nblks, const unsigned char *_key)
{
    block *blks = (block *)_blks;
    EMP_AES_KEY *key = (EMP_AES_KEY *)_key;
    {
        AES_KEY aeskey;
        aeskey.rounds = key->rounds;

#if __EMSCRIPTEN__
        // todo: optimized
        unsigned char *pekey = (unsigned char *)key->rd_key;
        unsigned char *pokey = (unsigned char *)aeskey.rd_key;
        for (int i = 0; i < 176 / 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                int ko = i * 4 + 4 - j - 1;
                int ke = i * 4 + j;
                pokey[ko] = pekey[ke];
            }
        }
#else // __x86_64__
        memcpy((char *)aeskey.rd_key, (char *)key->rd_key, sizeof(key->rd_key));
#endif

        for (unsigned int i = 0; i < nblks; i++)
        {
            AES_ecb_encrypt((unsigned char *)blks + AES_BLOCK_SIZE * i, (unsigned char *)blks + AES_BLOCK_SIZE * i, &aeskey, AES_ENCRYPT);
        }
    }
}
