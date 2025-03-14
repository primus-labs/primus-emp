#ifndef EMP_LPN_F2K_H__
#define EMP_LPN_F2K_H__

#include "emp-tool/emp-tool.h"
using namespace emp;

//Implementation of local linear code on F_2^k
//Performance highly dependent on the CPU cache size
template<typename IO, int d = 10>
class LpnF2 { public:
	int party;
	int64_t n;
	ThreadPool * pool;
	IO *io;
	int threads, k, mask;
	block seed;
	LpnF2 (int party, int64_t n, int k, ThreadPool * pool, IO *io, int threads) {
		this->party = party;
		this->k = k;
		this->n = n;
		this->pool = pool;
		this->io = io;
		this->threads = threads;
		mask = 1;
		while(mask < k) {
			mask <<=1;
			mask = mask | 0x1;
		}
	}

	void __compute4(block * nn, const block * kk, int64_t i, PRP * prp) {
		block tmp[d];
		for(int m = 0; m < d; ++m)
			tmp[m] = makeBlock(i, m);
		AES_ecb_encrypt_blks(tmp, d, &prp->aes);
		uint32_t* r = (uint32_t*)(tmp);
		for(int m = 0; m < 4; ++m)
			for (int j = 0; j < d; ++j) {
				int index = (*r) & mask;
				++r;
				index = index >= k? index-k:index;
				nn[i+m] = nn[i+m] ^ kk[index];
			}
	}

	void __compute1(block * nn, const block * kk, int64_t i, PRP*prp) {
                const auto nr_blocks = d/4 + (d % 4 != 0);
                block tmp[nr_blocks];
		for(int m = 0; m < nr_blocks; ++m)
			tmp[m] = makeBlock(i, m);
		prp->permute_block(tmp, nr_blocks);
		uint32_t* r = (uint32_t*)(tmp);
		for (int j = 0; j < d; ++j)
			nn[i] = nn[i] ^ kk[r[j]%k];
	}

	void task(block * nn, const block * kk, int64_t start, int64_t end) {
		PRP prp(seed);
		int64_t j = start;
		for(; j < end-4; j+=4)
			__compute4(nn, kk, j, &prp);
		for(; j < end; ++j)
			__compute1(nn, kk, j, &prp);
	}

	void compute(block * nn, const block * kk) {
		vector<std::future<void>> fut;
		int64_t width = n/threads;
		seed = seed_gen();
		for(int i = 0; i < threads; ++i) {
			int64_t start = i * width;
			int64_t end = min((i+1)* width, n);
			if (i == threads - 1) {
				end = n;
			}
			fut.push_back(pool->enqueue(FunctionWrapper([this, nn, kk, start, end]() {
				task(nn, kk, start, end);
			}, pool)));
		}

		for (auto &f: fut) f.get();

		CHECK_THREAD_POOL_EXCEPTION(pool);

	}

	block seed_gen() {
		block seed;
		if(party == ALICE) {
			PRG prg;
			prg.random_block(&seed, 1);
			io->send_data(&seed, sizeof(block));
		} else {
			io->recv_data(&seed, sizeof(block));
		}io->flush();
		return seed;
	}
	void bench(block * nn, const block * kk) {
		vector<std::future<void>> fut;
		int64_t width = n/threads;
		for(int i = 0; i < threads; ++i) {
			int64_t start = i * width;
			int64_t end = min((i+1)* width, n);
			if (i == threads - 1) {
				end = n;
			}
			fut.push_back(pool->enqueue(FunctionWrapper([this, nn, kk, start, end]() {
				task(nn, kk, start, end);
			}, pool)));
		}

		for (auto &f: fut) f.get();

		CHECK_THREAD_POOL_EXCEPTION(pool);

	}

};
#endif
