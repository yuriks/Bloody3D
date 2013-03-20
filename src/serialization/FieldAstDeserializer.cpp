#include "FieldAstDeserializer.hpp"
#include "Heatwave.hpp"
#include "util/StringHash.hpp"
#include "ODLAst.hpp"
#include "math/Quaternion.hpp"
#include "SceneReader.hpp"

namespace serialization {

static bool strEqual(const char* a_begin, const char* a_end, const char* b) {
	for (; a_begin != a_end && *b != '\0'; ++a_begin, ++b) {
		if (*a_begin != *b)
			return false;
	}
	return true;
}

static float numberAsFloat(const AstNumber& number) {
	if (number.type == AstNumber::FLOAT) {
		return number.float_value;
	} else {
		return static_cast<float>(number.int_value);
	}
}

const AstField* searchField(const AstField* fields, const char* str, u32 hash) {
	const AstField* field = fields;

	while (field != nullptr) {
		const AstFieldName& name = field->name;
		if (name.hash == hash && strEqual(name.begin, name.end, str)) {
			return field;
		}
		field = field->next;
	}
	return nullptr;
}

void FieldAstDeserializer::operator ()(float& v, const char* name, u32 name_hash) {
	const AstField* field = searchField(fields, name, name_hash);
	if (field == nullptr)
		return;

	const AstExpression& number = field->expression;
	assert(number.type == AstExpression::NUMBER);
	v = numberAsFloat(*number.number);
}

template <unsigned int N>
void FieldAstDeserializer::operator ()(math::vec<N>& v, const char* name, u32 name_hash) {
	const AstField* field = searchField(fields, name, name_hash);
	if (field == nullptr)
		return;

	const AstExpression& tuple = field->expression;
	assert(tuple.type == AstExpression::TUPLE);

	const AstSequence* e = tuple.sequence;
	for (size_t i = 0; i < N; ++i) {
		assert(e != nullptr);
		assert(e->type == AstExpression::NUMBER);
		v[i] = numberAsFloat(*e->number);
		e = e->next;
	}
	assert(e == nullptr);
}

template void FieldAstDeserializer::operator ()<3>(math::vec<3>& v, const char* name, u32 name_hash);

void FieldAstDeserializer::operator ()(math::Quaternion& v, const char* name, u32 name_hash) {
	const AstField* field = searchField(fields, name, name_hash);
	if (field == nullptr)
		return;

	const AstExpression& tuple = field->expression;
	assert(tuple.type == AstExpression::TUPLE);

	const AstSequence* e = tuple.sequence;
	for (size_t i = 0; i < 4; ++i) {
		assert(e != nullptr);
		assert(e->type == AstExpression::NUMBER);
		(i == 3 ? v.w : v.v[i]) = numberAsFloat(*e->number);
		e = e->next;
	}
	assert(e == nullptr);
}

void FieldAstDeserializer::operator ()(Handle& v, const char* name, u32 name_hash) {
	const AstField* field = searchField(fields, name, name_hash);
	if (field == nullptr)
		return;

	const AstExpression& expr = field->expression;
	if (expr.type == AstExpression::INSTANCE) {
		v = scene_reader->deserializeObject(*expr.instance);
	} else if (expr.type == AstExpression::HANDLE_NAME) {
		const AstHandleName& hname = *expr.handle_name;
		if (hname.begin == nullptr) {
			v = Handle();
		} else {
			v = scene_reader->handle_map.at(std::string(hname.begin, hname.end));
		}
	} else {
		assert(false);
	}
}

}
