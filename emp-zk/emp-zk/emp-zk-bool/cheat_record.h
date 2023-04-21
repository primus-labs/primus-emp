#ifndef EMP_ZK_CHEAT_RECORD_H__
#define EMP_ZK_CHEAT_RECORD_H__
#include <vector>
#include <string>
using std::vector;
using std::string;

class CheatRecord { public:
#ifndef THREADING
	static vector<string> *message;
#else
	static __thread vector<string> *message;
#endif
	static void reset() {
		message = new vector<string>();
		message->clear();
	}
	static void put(const string &s);
	static bool cheated() {
		return !message->empty();
	}
};
#endif
