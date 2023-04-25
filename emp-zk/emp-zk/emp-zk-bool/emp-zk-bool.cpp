#include "emp-zk/emp-zk-bool/cheat_record.h"

#ifndef THREADING
vector<string> CheatRecord::message;
#else
__thread vector<string>* CheatRecord::message = nullptr;
#endif

void CheatRecord::put(const string &s) {
#ifndef THREADING
	message.push_back(s);
#else
	if (message == nullptr)
		message = new vector<string>();
	message->push_back(s);
#endif
}

