#pragma once

#include "texture/TextureManager.hpp"
#include "util/ObjectPool.hpp"
#include "mesh/Material.hpp"

struct Engine {
	TextureManager texture_manager;
	ObjectPool<Material> materials;
};
