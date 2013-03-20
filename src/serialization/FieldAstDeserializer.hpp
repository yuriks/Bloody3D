#pragma once

#include "math/vec.hpp"
#include "util/Handle.hpp"

namespace math {
	struct Quaternion;
}

namespace serialization {

struct AstField;
struct SceneAstDeserializer;

struct FieldAstDeserializer {
	const AstField* fields;
	SceneAstDeserializer* scene_reader;

	const AstField* searchField(const char* str);

	void operator ()(float& v, const char* name);
	template <unsigned int N> void operator ()(math::vec<N>& v, const char* name);
	void operator ()(math::Quaternion& v, const char* name);
	void operator ()(Handle& v, const char* name);
};

template <typename T>
Handle objectDeserializator(FieldAstDeserializer& reflector) {
	T obj;
	obj.reflect(reflector);
	ObjectPool<T>& pool = reflectGetPool(*reflector.scene_reader->scene, static_cast<T*>(nullptr));
	return pool.insert(std::move(obj));
}

}
