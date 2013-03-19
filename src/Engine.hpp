#pragma once

#include "texture/TextureManager.hpp"
#include "util/ObjectPool.hpp"
#include "mesh/Material.hpp"
#include "mesh/GPUMesh.hpp"
#include "scene/RenderContext.hpp"

struct LightInfo {
	Handle material;
	gl::VertexArrayObject vao;
	gl::BufferObject vbo;
};

struct Engine {
	TextureManager texture_manager;
	ObjectPool<Material> materials;
	ObjectPool<GPUMesh> gpu_meshes;

	scene::RenderContext render_context;

	Handle tonemap_material;

	LightInfo dirlight;
	LightInfo omnilight;
	LightInfo spotlight;
	gl::VertexArrayObject null_vao;
};

void loadEngineMaterials(Engine& engine);
