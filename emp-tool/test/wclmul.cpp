#include "emp-tool/emp-tool.h"
#include <iostream>
using namespace std;
using namespace emp;

int main()
{
#if !(__x86_64__ || __EMSCRIPTEN__)
// have not test
#endif

	{
		cout << "correct check:" << endl;

		PRG prg;
		prg.reseed(&zero_block);

		for (int i = 0; i < 8; i++)
		{
			block blks[2];
			prg.random_block(blks, 2);
			block a = blks[0];
			block b = blks[1];
			block c[4];
			c[0] = _mm_clmulepi64_si128(a, b, 0x00);
			c[1] = _mm_clmulepi64_si128(a, b, 0x01);
			c[2] = _mm_clmulepi64_si128(a, b, 0x10);
			c[3] = _mm_clmulepi64_si128(a, b, 0x11);

			cout << i << " 0x00 => " << c[0] << endl;
			cout << i << " 0x01 => " << c[1] << endl;
			cout << i << " 0x10 => " << c[2] << endl;
			cout << i << " 0x11 => " << c[3] << endl;
			cout << endl;
		}
	}

	return 0;
}