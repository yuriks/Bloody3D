#pragma once

#include "texture/TextureManager.hpp"
#include "util/ObjectPool.hpp"
#include "mesh/Material.hpp"
#include "mesh/GPUMesh.hpp"

struct Engine {
	TextureManager texture_manager;
	ObjectPool<Material> materials;
	ObjectPool<GPUMesh> gpu_meshes;
};
