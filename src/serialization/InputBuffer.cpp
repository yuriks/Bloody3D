#include "InputBuffer.hpp"

#include <array>
#include <fstream>
#include <cassert>
#include <iostream>

namespace serialization {

static bool checkWhitespace(InputBuffer& buf, size_t start) {
	// Check for whitespace in buffer
	for (size_t i = start; i < buf.end_i; ++i) {
		char c = buf.buffer[i];
		if (buf.in_quote) {
			if (c == '"')
				buf.in_quote = false;
		} else if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
			// We have enough data to last another parse, no need to refill
			return true;
		}
	}

	return false;
}

void InputBuffer::refill() {
	if (checkWhitespace(*this, cur_i))
		return;

	// Shift buffer contents down if needed
	if (cur_i != 0) {
		size_t i = 0;
		while (cur_i != end_i) {
			buffer[i++] = buffer[cur_i++];
		}
		cur_i = 0;
		end_i = i;
	}

	// Fill rest of buffer with read data
	while (true) {
		f->read(&buffer[end_i], buffer.size() - end_i);
		const size_t check_start = end_i;
		end_i += static_cast<size_t>(f->gcount());

		if (f->eof()) {
			eof = true;
			break;
		} else if (f->fail()) {
			error = true;
			break;
		}

		if (checkWhitespace(*this, check_start))
			break;

		if (end_i - cur_i == buffer.size()) {
			// Buffer is full an no whitespace found:
			// We got stuck on a too long token
			std::cerr << "ERROR: Overlong token while parsing:\n";
			std::cerr.write(buffer.data(), buffer.size());
			std::cerr << std::endl;
			error = true;
			break;
		}
	}
}

}
