#include "FieldAstDeserializer.hpp"
#include "Heatwave.hpp"
#include "util/StringHash.hpp"
#include "ODLAst.hpp"
#include "math/Quaternion.hpp"
#include "SceneReader.hpp"

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

template <typename T>
AstFlagSetter<T> AstFlagSetter<T>::flag(T value, const char* name, u32 name_hash) {
	const AstSequence* seq = seq_head;
	while (seq != nullptr) {
		assert(seq->type == AstExpression::FIELD_NAME);
		const AstFieldName& fname = *seq->field_name;
		if (fname.hash == name_hash && strEqual(fname.begin, fname.end, name)) {
			*target |= value;
		}
		seq = seq->next;
	}

	return *this;
}

template AstFlagSetter<u8> AstFlagSetter<u8>::flag(u8 value, const char* name, u32 name_hash);

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

template <typename T>
AstFlagSetter<T> FieldAstDeserializer::flags(T& v, const char* name, u32 name_hash) {
	const AstField* field = searchField(fields, name, name_hash);
	if (field == nullptr)
		return AstFlagSetter<T>(nullptr, nullptr);

	const AstExpression& sequence = field->expression;
	assert(sequence.type == AstExpression::LIST);
	return AstFlagSetter<T>(sequence.sequence, &v);
}

template AstFlagSetter<u8> FieldAstDeserializer::flags<u8>(u8& v, const char* name, u32 name_hash);

void FieldAstDeserializer::operator ()(float& v, const char* name, u32 name_hash) {
	const AstField* field = searchField(fields, name, name_hash);
	if (field == nullptr)
		return;

	const AstExpression& number = field->expression;
	assert(number.type == AstExpression::NUMBER);
	v = numberAsFloat(*number.number);
}

template <unsigned int N>
void FieldAstDeserializer::operator ()(vec<N>& v, const char* name, u32 name_hash) {
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

template void FieldAstDeserializer::operator ()<3>(vec<3>& v, const char* name, u32 name_hash);

void FieldAstDeserializer::operator ()(Quaternion& v, const char* name, u32 name_hash) {
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

void FieldAstDeserializer::operator ()(std::string& v, const char* name, u32 name_hash) {
	const AstField* field = searchField(fields, name, name_hash);
	if (field == nullptr)
		return;

	const AstExpression& str = field->expression;
	assert(str.type == AstExpression::STRING);
	v.assign(str.string->begin, str.string->end);
}
