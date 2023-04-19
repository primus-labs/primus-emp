#include "emp-zk/emp-zk-bool/cheat_record.h"

#ifndef THREADING
vector<string>* CheatRecord::message = nullptr;
#else
__thread vector<string>* CheatRecord::message = nullptr;
#endif
void CheatRecord::put(const string &s) {
	message->push_back(s);
}

