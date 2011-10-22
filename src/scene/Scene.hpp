#pragma once

#include "gl/Framebuffer.h"
#include "gl/Texture.hpp"
#include "math/Vector.hpp"
#include "math/Quaternion.hpp"
#include "mesh/GPUMesh.hpp"
#include "mesh/TextureManager.hpp"
#include "mesh/Material.hpp"
#include "util/AlignedVector.hpp"
#include "scene/RenderContext.hpp"

namespace scene {

struct MeshInstance {
	math::vec4 pos_scale;
	math::Quaternion rot;
};

struct Light {
	math::vec3 pos;
	math::vec3 color;
};

struct Camera {
	math::vec3 pos;
	math::Quaternion rot;
	float fov;
	float clip_near;
	float clip_far;
};

struct RenderBufferSet {
	gl::Framebuffer fbo;
	gl::Texture depth_tex;
	gl::Texture diffuse_tex;
	gl::Texture normal_tex;

	void initialize(int width, int height);
};

struct Scene {
	TextureManager tex_manager;
	std::vector<Material> material_list;
	std::vector<GPUMesh> gpu_meshes;
	std::vector<util::AlignedVector<MeshInstance>> mesh_instances;
	util::AlignedVector<Light> lights;

	int addMaterial(Material&& mat);
	int addMesh(GPUMesh&& mesh);
	MeshInstance& newInstance(int mesh_id);
};

void renderGeometry(const Scene& scene, const Camera& camera, RenderBufferSet& buffers, RenderContext& render_context);
void shadeBuffers(const util::AlignedVector<Light>& lights, const Material& shading_material, RenderBufferSet& buffers, GLuint destination_fbo, RenderContext& render_context);

} // namespace scene
