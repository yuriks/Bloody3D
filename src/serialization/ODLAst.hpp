#pragma once

#include "Heatwave.hpp"

struct AstTypeName {
	char* begin;
	char* end;
};

struct AstHandleName {
	char* begin;
	char* end;
	bool create;
};

struct AstFieldName {
	char* begin;
	char* end;
	u32 hash;
};

struct AstString {
	char* begin;
	char* end;
};

struct AstNumber {
	enum Type {
		INT, FLOAT
	};

	Type type;

	union {
		s32 int_value;
		float float_value;
	};
};

struct AstInstance;
struct AstSequence;

struct AstExpression {
	enum Type {
		INSTANCE, HANDLE_NAME, STRING, LIST, TUPLE, NUMBER, FIELD_NAME
	};

	Type type;

	union {
		AstInstance* instance;
		AstHandleName* handle_name;
		AstString* string;
		AstSequence* sequence;
		AstNumber* number;
		AstFieldName* field_name;
	};
};

struct AstSequence : AstExpression {
	AstSequence* next;
};

struct AstField {
	AstFieldName name;
	AstExpression expression;
	AstField* next;
};

struct AstInstance {
	AstTypeName type_name;
	AstHandleName handle_name;
	AstField* fields;

	AstInstance* next;
};
