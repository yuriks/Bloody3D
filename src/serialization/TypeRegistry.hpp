#pragma once

#include "Heatwave.hpp"
#include "util/Handle.hpp"
#include "util/ObjectPool.hpp"
#include "scene/Scene.hpp"

#include "scene/Transform.hpp"

struct FieldAstDeserializer;

struct TypeEntry {
	u32 name_hash;
	const char* name;
	Handle (*astDeserializator)(FieldAstDeserializer&);
};

extern TypeEntry type_registry[];
extern TypeEntry* type_registry_end;

#define DECLARE_POOL(type, pool) \
	inline ObjectPool<type>& reflectGetPool(Scene& scene, type*) { \
		return pool; \
	}

DECLARE_POOL(::Transform, scene.transforms)
DECLARE_POOL(::DirectionalLight, scene.lights_dir)
DECLARE_POOL(::OmniLight, scene.lights_omni)
DECLARE_POOL(::SpotLight, scene.lights_spot)
DECLARE_POOL(::Camera, scene.cameras)
DECLARE_POOL(::MeshInstance, scene.mesh_instances)
DECLARE_POOL(::MaterialOptions, scene.material_options)
DECLARE_POOL(::Texture, scene.engine->textures)
DECLARE_POOL(::GPUMesh, scene.engine->gpu_meshes)

#undef DECLARE_POOL
