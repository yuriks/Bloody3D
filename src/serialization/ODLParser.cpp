#include "ODLParse.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include "util/StringHash.hpp"

#define TRY(x) if (!(x)) return false
#define REQUIRE(x, msg) if (!(x)) { ast.fail(in, msg); return false; }

void skipWs(InputBuffer& in) {
	do {
		while (in.cur_i < in.end_i) {
			char c = in.buffer[in.cur_i];
			if (!(c == ' ' || c == '\t' || c == '\n' || c == '\r')) {
				in.refill();
				return;
			}

			if (c == '\n') ++in.cur_line;
			++in.cur_i;
		}
		in.refill();
	} while (in.cur_i != in.end_i);
}

bool parseRange(InputBuffer& in, char l_min, char l_max) {
	char c = in.buffer[in.cur_i];
	if (c >= l_min && c <= l_max) {
		in.cur_i++;
		return true;
	} else {
		return false;
	}
}

bool parseLiteral(InputBuffer& in, char l) {
	return parseRange(in, l, l);
}

bool parseLiteral(InputBuffer& in, const char* s) {
	size_t i = in.cur_i;
	while (*s != '\0') {
		if (in.buffer[i++] != *s) {
			return false;
		}
	}

	in.cur_i = i;
	return true;
}

template <typename AstT, typename AstT2>
void parseSequence(InputBuffer& in, ParseAst& ast, AstT*& first_ptr, bool (*parseFunc)(InputBuffer&, ParseAst&, AstT2&)) {
	first_ptr = nullptr;
	AstT* prev_node = nullptr;
	AstT cur_node;
	while (parseFunc(in, ast, cur_node)) {
		AstT* new_node = ast.alloc.allocate<AstT>();
		*new_node = cur_node;

		if (prev_node == nullptr) {
			first_ptr = new_node;
		} else {
			prev_node->next = new_node;
		}
		prev_node = new_node;
	}
	if (prev_node != nullptr) {
		prev_node->next = nullptr;
	}
}

bool parseTypeName(InputBuffer& in, ParseAst& ast, AstTypeName& node);
bool parseHandleName(InputBuffer& in, ParseAst& ast, AstHandleName& node);
bool parseFieldName(InputBuffer& in, ParseAst& ast, AstFieldName& node);

bool parseInstance(InputBuffer& in, ParseAst& ast, AstInstance& node);
bool parseField(InputBuffer& in, ParseAst& ast, AstField& node);
bool parseExpression(InputBuffer& in, ParseAst& ast, AstExpression& node);

bool parseString(InputBuffer& in, ParseAst& ast, AstString& node);
bool parseList(InputBuffer& in, ParseAst& ast, AstSequence*& node_ptr);
bool parseTuple(InputBuffer& in, ParseAst& ast, AstSequence*& node_ptr);
bool parseNumber(InputBuffer& in, ParseAst& ast, AstNumber& node);

void parseRoot(InputBuffer& in, ParseAst& ast) {
	skipWs(in);
	parseSequence(in, ast, ast.instances, parseInstance);
}

bool parseTypeName(InputBuffer& in, ParseAst& ast, AstTypeName& node) {
	const char* str_begin = &in.buffer[in.cur_i];
	TRY(parseRange(in, 'A', 'Z'));
	while (parseRange(in, 'a', 'z') || parseRange(in, 'A', 'Z'));
	const char* str_end = &in.buffer[in.cur_i];

	node.begin = ast.alloc.allocate<char>(str_end - str_begin);
	node.end = std::copy(str_begin, str_end, node.begin);

	skipWs(in);

	return true;
}

bool parseHandleName(InputBuffer& in, ParseAst& ast, AstHandleName& node) {
	TRY(parseLiteral(in, '@'));
	if (parseLiteral(in, "null")) {
		node.begin = node.end = nullptr;
	} else {
		node.create = parseLiteral(in, '+');

		const char* str_begin = &in.buffer[in.cur_i];
		REQUIRE(parseRange(in, 'a', 'z'), "Expected a-z.");
		while (parseRange(in, 'a', 'z') || parseRange(in, '0', '9') || parseLiteral(in, '_'));
		const char* str_end = &in.buffer[in.cur_i];

		node.begin = ast.alloc.allocate<char>(str_end - str_begin);
		node.end = std::copy(str_begin, str_end, node.begin);
	}
	skipWs(in);

	return true;
}

bool parseFieldName(InputBuffer& in, ParseAst& ast, AstFieldName& node) {
	const char* str_begin = &in.buffer[in.cur_i];
	TRY(parseRange(in, 'a', 'z'));
	while (parseRange(in, 'a', 'z') || parseRange(in, '0', '9') || parseLiteral(in, '_'));
	const char* str_end = &in.buffer[in.cur_i];

	node.begin = ast.alloc.allocate<char>(str_end - str_begin);
	node.end = std::copy(str_begin, str_end, node.begin);
	node.hash = fnv_hash_runtime(node.begin, node.end);

	skipWs(in);

	return true;
}

bool parseInstance(InputBuffer& in, ParseAst& ast, AstInstance& node) {
	TRY(parseTypeName(in, ast, node.type_name));
	if (!parseHandleName(in, ast, node.handle_name)) {
		node.handle_name.begin = node.handle_name.end = nullptr;
	}
	REQUIRE(parseLiteral(in, '{'), "Expected {.");
	skipWs(in);
	parseSequence(in, ast, node.fields, parseField);
	REQUIRE(parseLiteral(in, '}'), "Expected }.");
	skipWs(in);

	return true;
}

bool parseField(InputBuffer& in, ParseAst& ast, AstField& node) {
	TRY(parseFieldName(in, ast, node.name));
	REQUIRE(parseLiteral(in, ':'), "Expected :.");
	skipWs(in);
	REQUIRE(parseExpression(in, ast, node.expression), "Expected expression.");

	return true;
}

bool parseExpression(InputBuffer& in, ParseAst& ast, AstExpression& node) {
	union {
		AstInstance node_instance;
		AstHandleName node_handle_name;
		AstString node_string;
		AstSequence* node_sequence;
		AstNumber node_number;
		AstFieldName node_field_name;
	};

#define PARSECASE(e, cls, f) \
		if (parse##cls(in, ast, node_##f)) { \
			node.type = AstExpression::e; \
			node.f = ast.alloc.allocate<Ast##cls>(); \
			*node.f = node_##f; \
		} else
#define PARSECASE_SEQ(e, cls) \
		if (parse##cls(in, ast, node_sequence)) { \
			node.type = AstExpression::e; \
			node.sequence = node_sequence; \
		} else

	PARSECASE(INSTANCE, Instance, instance)
	PARSECASE(HANDLE_NAME, HandleName, handle_name)
	PARSECASE(STRING, String, string)
	PARSECASE_SEQ(LIST, List)
	PARSECASE_SEQ(TUPLE, Tuple)
	PARSECASE(NUMBER, Number, number)
	PARSECASE(FIELD_NAME, FieldName, field_name) {
		return false;
	}

#undef PARSECASE
#undef PARSECASE_SEQ

	return true;
}

bool parseString(InputBuffer& in, ParseAst& ast, AstString& node) {
	TRY(parseLiteral(in, '"'));

	in.in_quote = true;
	in.refill();

	const char* str_begin = &in.buffer[in.cur_i];
	while (in.cur_i < in.end_i && in.buffer[in.cur_i] != '"') {
		++in.cur_i;
	}
	if (in.cur_i == in.end_i) {
		std::cerr << "ERROR: Overlong string while parsing:\n";
		std::cerr.write(str_begin, &in.buffer[in.cur_i] - str_begin);
		std::cerr << std::endl;
		in.error = true;
		return false;
	}
	const char* str_end = &in.buffer[in.cur_i++];

	node.begin = ast.alloc.allocate<char>(str_end - str_begin);
	node.end = std::copy(str_begin, str_end, node.begin);

	skipWs(in);
	return true;
}

bool parseList(InputBuffer& in, ParseAst& ast, AstSequence*& node_ptr) {
	TRY(parseLiteral(in, '['));
	skipWs(in);
	parseSequence(in, ast, node_ptr, parseExpression);
	REQUIRE(parseLiteral(in, ']'), "Expected ].");
	skipWs(in);

	return true;
}

bool parseTuple(InputBuffer& in, ParseAst& ast, AstSequence*& node_ptr) {
	TRY(parseLiteral(in, '('));
	skipWs(in);
	parseSequence(in, ast, node_ptr, parseExpression);
	REQUIRE(parseLiteral(in, ')'), "Expected ).");
	skipWs(in);

	return true;
}

bool parseNumber(InputBuffer& in, ParseAst& ast, AstNumber& node) {
	char c = in.buffer[in.cur_i];
	if (c != '-' && (c < '0' || c > '9')) return false;

	const char* str_begin = &in.buffer[in.cur_i];
	node.type = AstNumber::INT;

	parseLiteral(in, '-');
	if (!(parseLiteral(in, '0'))) {
		REQUIRE(parseRange(in, '1', '9'), "Expected 1-9.");
		while (parseRange(in, '0', '9'));
	}

	if (parseLiteral(in, '.')) {
		node.type = AstNumber::FLOAT;
		REQUIRE(parseRange(in, '0', '9'), "Expected 0-9.");
		while (parseRange(in, '0', '9'));
	}

	if (parseLiteral(in, 'e') || parseLiteral(in, 'E')) {
		node.type = AstNumber::FLOAT;
		parseLiteral(in, '+') || parseLiteral(in, '-');
		REQUIRE(parseRange(in, '0', '9'), "Expected 0-9.");
		while (parseRange(in, '0', '9'));
	}

	const char* str_end = &in.buffer[in.cur_i];
	if (node.type == AstNumber::INT) {
		node.int_value = std::stoi(std::string(str_begin, str_end));
	} else {
		node.float_value = std::stof(std::string(str_begin, str_end));
	}

	skipWs(in);

	return true;
}
