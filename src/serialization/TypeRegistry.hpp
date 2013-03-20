#pragma once

#include "Heatwave.hpp"
#include "util/Handle.hpp"
#include "util/ObjectPool.hpp"
#include "scene/Scene.hpp"

#include "scene/Transform.hpp"

namespace serialization {

struct FieldAstDeserializer;

struct TypeEntry {
	u32 name_hash;
	const char* name;
	Handle (*astDeserializator)(FieldAstDeserializer&);
};

extern TypeEntry type_registry[];
extern TypeEntry* type_registry_end;

#define DECLARE_POOL(type, pool) \
	inline ObjectPool<type>& reflectGetPool(scene::Scene& scene, type*) { \
		return pool; \
	}

DECLARE_POOL(::scene::Transform, scene.transforms)

}
