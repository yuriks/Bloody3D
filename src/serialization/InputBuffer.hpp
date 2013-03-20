#pragma once

#include <array>
#include <fstream>

namespace serialization {

struct InputBuffer {
	std::ifstream* f;

	std::array<char, 1024> buffer;
	size_t cur_i;
	size_t end_i;

	int cur_line;

	bool in_quote;
	bool eof;
	bool error;

	InputBuffer(std::ifstream* f)
		: f(f), cur_i(0), end_i(0), cur_line(1),
		in_quote(false), eof(false), error(false)
	{}

	void refill();
};

}
