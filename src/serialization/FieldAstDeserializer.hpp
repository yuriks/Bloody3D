#pragma once

#include "math/vec.hpp"
#include "util/Handle.hpp"

namespace math {
	struct Quaternion;
}

namespace serialization {

struct AstField;
struct AstSequence;
struct SceneAstDeserializer;

template <typename T>
struct AstFlagSetter {
	const AstSequence *const seq_head;
	T *const target;

	AstFlagSetter(const AstSequence* seq, T* target)
		: seq_head(seq), target(target)
	{}

	AstFlagSetter flag(T value, const char* name, u32 name_hash);
};

struct FieldAstDeserializer {
	const AstField* fields;
	SceneAstDeserializer* scene_reader;

	template <typename T>
	AstFlagSetter<T> flags(T& v, const char* name, u32 name_hash);

	void operator ()(float& v, const char* name, u32 name_hash);
	template <unsigned int N> void operator ()(math::vec<N>& v, const char* name, u32 name_hash);
	void operator ()(math::Quaternion& v, const char* name, u32 name_hash);
	void operator ()(Handle& v, const char* name, u32 name_hash);
	void operator ()(std::string& v, const char* name, u32 name_hash);
};

template <typename T>
Handle objectDeserializator(FieldAstDeserializer& reflector) {
	T obj;
	obj.reflect(reflector);
	ObjectPool<T>& pool = reflectGetPool(*reflector.scene_reader->scene, static_cast<T*>(nullptr));
	return pool.insert(std::move(obj));
}

}
