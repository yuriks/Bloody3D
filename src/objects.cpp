#include "objects.hpp"
#include "util/StringHash.hpp"
#include "serialization/FieldAstDeserializer.hpp"

#include "scene/Transform.hpp"
#include "scene/DirectionalLight.hpp"
#include "scene/OmniLight.hpp"
#include "scene/SpotLight.hpp"
#include "scene/Scene.hpp"
#include "mesh/Material.hpp"

TypeDatabase type_database;

namespace {
	template <typename T, size_t N>
	void define_type(const char (&name)[N]) {
		TypeEntry entry;
		entry.name = name;
		entry.name_hash = fnv_hash(name);
		entry.ast_deserializator = &objectDeserializator<T>;
		type_database.registerType(entry);
	}

	template <typename T, typename TTemplate, size_t N>
	void define_type_compiled(const char (&name)[N]) {
		TypeEntry entry;
		entry.name = name;
		entry.name_hash = fnv_hash(name);
		entry.ast_deserializator = &compiledObjectReader<T, TTemplate>;
		type_database.registerType(entry);
	}
}

void register_types() {
	define_type<Transform>("Transform");
	define_type<DirectionalLight>("DirectionalLight");
	define_type<OmniLight>("OmniLight");
	define_type<SpotLight>("SpotLight");
	define_type<Camera>("Camera");
	define_type<MeshInstance>("MeshInstance");
	define_type<MaterialOptions>("MaterialParameters");

	define_type_compiled<Texture, TextureTemplate>("Texture");
	define_type_compiled<GPUMesh, GPUMeshTemplate>("GPUMesh");
}
