#include "TypeRegistry.hpp"
#include "SceneReader.hpp"
#include "FieldAstDeserializer.hpp"
#include "util/StringHash.hpp"

#include "scene/Scene.hpp"
#include "util/Handle.hpp"

namespace serialization {

#define DECLARE_TYPE(type, str) {util::fnv_hash(str), str, &objectDeserializator<type>},
#define DECLARE_TYPE_COMPILED(type, str) {util::fnv_hash(str), str, &compiledObjectReader<type, type##Template>},

TypeEntry type_registry[] = {
	DECLARE_TYPE(::scene::Transform, "Transform")
	DECLARE_TYPE(::scene::DirectionalLight, "DirectionalLight")
	DECLARE_TYPE(::scene::OmniLight, "OmniLight")
	DECLARE_TYPE(::scene::SpotLight, "SpotLight")
	DECLARE_TYPE(::scene::Camera, "Camera")
	DECLARE_TYPE(::scene::MeshInstance, "MeshInstance")
	DECLARE_TYPE(::MaterialOptions, "MaterialParameters")
	DECLARE_TYPE_COMPILED(::Texture, "Texture")
	DECLARE_TYPE_COMPILED(::GPUMesh, "GPUMesh")
};
TypeEntry* type_registry_end = std::end(type_registry);

#undef DECLARE_TYPE

}
