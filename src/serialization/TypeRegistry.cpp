#include "TypeRegistry.hpp"
#include "SceneReader.hpp"
#include "FieldAstDeserializer.hpp"
#include "util/StringHash.hpp"

#include "scene/Scene.hpp"
#include "util/Handle.hpp"

namespace serialization {

#define DECLARE_TYPE(type, str) {util::fnv_hash(str), str, &objectDeserializator<type>},

TypeEntry type_registry[] = {
	DECLARE_TYPE(::scene::Transform, "Transform")
};
TypeEntry* type_registry_end = std::end(type_registry);

}
