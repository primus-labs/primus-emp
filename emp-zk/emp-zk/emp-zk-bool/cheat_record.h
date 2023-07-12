#ifndef EMP_ZK_CHEAT_RECORD_H__
#define EMP_ZK_CHEAT_RECORD_H__
#include <vector>
#include <string>
using std::vector;
using std::string;

class CheatRecord { public:
#ifndef THREADING
	static vector<string> message;
#else
	static __thread vector<string> *message;
#endif
	static void reset() {
#ifndef THREADING
		message.clear();
#else
		if (message == nullptr)
		    message = new vector<string>();
		message->clear();
#endif
	}
	static void put(const string &s);
	static bool cheated() {
#ifndef THREADING
		return !message.empty();
#else
		if(message==nullptr) return false;

		bool res = !message->empty();
		delete message;
		message = nullptr;

		return res;
#endif
	}
};
#endif
