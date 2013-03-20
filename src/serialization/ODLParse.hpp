#pragma once

#include "InputBuffer.hpp"
#include "memory/LinearAllocator.hpp"
#include "ODLAst.hpp"

namespace serialization {

struct ParseAst {
	std::array<u8, 8*1024> alloc_buf;
	memory::LinearAllocator alloc;

	const char* fail_message;
	int fail_line;
	char fail_char;
	AstInstance* instances;

	ParseAst()
		: alloc(alloc_buf.data(), alloc_buf.size()),
		fail_message(nullptr)
	{}

	void fail(InputBuffer& in, const char* msg) {
		if (fail_message == nullptr) {
			fail_message = msg;
			fail_line = in.cur_line;
			fail_char = in.buffer[in.cur_i];
		}
	}
};

void parseRoot(InputBuffer& in, ParseAst& ast);

}
