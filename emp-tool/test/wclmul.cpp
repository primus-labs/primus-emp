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
		// test _mm_clmulepi64_si128
		block a = makeBlock(0x0f0e0d0c0b0a0908, 0x0706050403020100);
		block b = makeBlock(0xffeeddccbbaa9988, 0x7766554433221100);
		block c;

		uint64_t *pc = (uint64_t *)&c;
#if (__x86_64__ || __EMSCRIPTEN__)
		cout << std::hex;
		c = _mm_clmulepi64_si128(a, b, 0x00);
		cout << "0x00 => " << std::setw(16) << std::setfill('0') << pc[1] << " " << pc[0] << endl;
		c = _mm_clmulepi64_si128(a, b, 0x01);
		cout << "0x01 => " << std::setw(16) << std::setfill('0') << pc[1] << " " << pc[0] << endl;
		c = _mm_clmulepi64_si128(a, b, 0x10);
		cout << "0x10 => " << std::setw(16) << std::setfill('0') << pc[1] << " " << pc[0] << endl;
		c = _mm_clmulepi64_si128(a, b, 0x11);
		cout << "0x11 => " << std::setw(16) << std::setfill('0') << pc[1] << " " << pc[0] << endl;
		cout << std::dec;
#endif
	}

	return 0;
}