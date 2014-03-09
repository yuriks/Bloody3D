#include "TypeRegistry.hpp"
#include "SceneReader.hpp"
#include "FieldAstDeserializer.hpp"
#include "util/StringHash.hpp"

#include "scene/Scene.hpp"
#include "util/Handle.hpp"

#define DECLARE_TYPE(type, str) {fnv_hash(str), str, &objectDeserializator<type>},
#define DECLARE_TYPE_COMPILED(type, str) {fnv_hash(str), str, &compiledObjectReader<type, type##Template>},

TypeEntry type_registry[] = {
	DECLARE_TYPE(::Transform, "Transform")
	DECLARE_TYPE(::DirectionalLight, "DirectionalLight")
	DECLARE_TYPE(::OmniLight, "OmniLight")
	DECLARE_TYPE(::SpotLight, "SpotLight")
	DECLARE_TYPE(::Camera, "Camera")
	DECLARE_TYPE(::MeshInstance, "MeshInstance")
	DECLARE_TYPE(::MaterialOptions, "MaterialParameters")
	DECLARE_TYPE_COMPILED(::Texture, "Texture")
	DECLARE_TYPE_COMPILED(::GPUMesh, "GPUMesh")
};
TypeEntry* type_registry_end = std::end(type_registry);

#undef DECLARE_TYPE
