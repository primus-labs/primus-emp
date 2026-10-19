#include "emp-ot/emp-ot.h"
#include <cstdio>
#include <fstream>
#include <iterator>
#include <random>
#include <stdexcept>
#include <vector>

using namespace emp;

#if !(__EMSCRIPTEN__ || THREADING)
namespace {

// Cached setup only exchanges whether both parties have a preprocessing file.
// Model the cached peer so persistence can be tested without a network socket.
class CachedPeerIO : public IOChannel<CachedPeerIO> {
public:
	void send_data_internal(const void *data, size_t size) {
		if (size != sizeof(bool) || !*static_cast<const bool *>(data))
			throw std::runtime_error("Expected the cached-data presence handshake");
	}

	void recv_data_internal(void *data, size_t size) {
		if (size != sizeof(bool))
			throw std::runtime_error("Unexpected communication during cached setup");
		*static_cast<bool *>(data) = true;
	}

	void flush() {}
};

// Keep checks active in Release builds, where assert() is disabled.
bool check(bool condition, const char *message) {
	if (!condition)
		std::cerr << message << std::endl;
	return condition;
}

template<typename T>
void append(std::vector<unsigned char> &bytes, const T &value) {
	const unsigned char *first = reinterpret_cast<const unsigned char *>(&value);
	bytes.insert(bytes.end(), first, first + sizeof(value));
}

class FixtureFile {
public:
	std::string path;

	explicit FixtureFile(int party) {
		std::random_device random;
		path = "ferret-persistence-" + std::to_string(party) + "-"
			+ std::to_string(random()) + "-" + std::to_string(random()) + ".bin";
	}

	~FixtureFile() {
		std::remove(path.c_str());
	}

	void write(const std::vector<unsigned char> &bytes) const {
		std::ofstream output(path, std::ios::binary);
		output.write(reinterpret_cast<const char *>(bytes.data()), bytes.size());
		output.close();
		if (!output)
			throw std::runtime_error("Cannot write the Ferret persistence fixture");
	}

	std::vector<unsigned char> read() const {
		std::ifstream input(path, std::ios::binary);
		if (!input)
			throw std::runtime_error("Ferret did not persist its preprocessing data");
		return std::vector<unsigned char>(std::istreambuf_iterator<char>(input),
			std::istreambuf_iterator<char>());
	}
};

bool test_persistence(int party) {
	CachedPeerIO io;
	CachedPeerIO *ios[] = {&io};
	const PrimalLPNParameter param = ferret_b10;
	const int64_t party64 = party;
	const block delta = party == ALICE
		? makeBlock(0x123456789abcdef0LL, 0x13579bdf2468ace1LL) : zero_block;
	std::vector<block> preprocessing(param.n_pre);
	for (int64_t i = 0; i < param.n_pre; ++i)
		preprocessing[i] = makeBlock(i + party, i * 3 + party);
	const unsigned char *payload =
		reinterpret_cast<const unsigned char *>(preprocessing.data());
	const size_t payload_size = preprocessing.size() * sizeof(block);

	// Seed both parties with canonical cached data. This exercises persistence
	// through the public constructor/destructor without running expensive base OT.
	std::vector<unsigned char> expected_file;
	append(expected_file, party64);
	if (party == ALICE)
		append(expected_file, delta);
	append(expected_file, param.n);
	append(expected_file, param.t);
	append(expected_file, param.k);
	expected_file.insert(expected_file.end(), payload, payload + payload_size);
	FixtureFile fixture(party);
	fixture.write(expected_file);

	bool ok = true;
	{
		FerretCOT<CachedPeerIO> ferret(party, 1, ios, false, true, param, fixture.path);
		CHECK_INITIALIZE_EXCEPTION();
		ok = check(cmpBlock(&ferret.Delta, &delta, 1),
			"Cached preprocessing restored the wrong Delta") && ok;
	}

	const std::vector<unsigned char> saved = fixture.read();
	int64_t saved_party = 0;
	if (saved.size() >= sizeof(saved_party))
		memcpy(&saved_party, saved.data(), sizeof(saved_party));
	ok = check(saved.size() >= sizeof(saved_party) && saved_party == party64,
		"Persisted party must be a canonical eight-byte integer") && ok;
	ok = check(saved == expected_file,
		"Persisted preprocessing changed its layout, Delta, or payload") && ok;

	std::vector<unsigned char> expected_state;
	append(expected_state, party64);
	append(expected_state, param.n);
	append(expected_state, param.t);
	append(expected_state, param.k);
	append(expected_state, delta);
	expected_state.insert(expected_state.end(), payload, payload + payload_size);
	{
		FerretCOT<CachedPeerIO> restored(party, 1, ios, false, true, param, fixture.path);
		CHECK_INITIALIZE_EXCEPTION();
		std::vector<unsigned char> state(restored.state_size());
		restored.assemble_state(state.data(), state.size());
		ok = check(state == expected_state,
			"Reloading persisted preprocessing changed the restored state") && ok;
	}

	return ok;
}

} // namespace
#endif

int main() {
#if __EMSCRIPTEN__ || THREADING
	std::cout << "Skipping Ferret persistence: file caching is disabled" << std::endl;
	return 77;
#else
	try {
		bool ok = true;
		for (int party : {ALICE, BOB}) {
			const bool passed = test_persistence(party);
			ok = passed && ok;
			if (passed)
				std::cout << "Ferret persistence passed for party " << party << std::endl;
		}
		return ok ? 0 : 1;
	} catch (const std::exception &error) {
		std::cerr << error.what() << std::endl;
		return 1;
	}
#endif
}
