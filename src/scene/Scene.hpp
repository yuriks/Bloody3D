#pragma once

#include "gl/Framebuffer.hpp"
#include "gl/Texture.hpp"
#include "math/vec.hpp"
#include "math/Quaternion.hpp"
#include "mesh/GPUMesh.hpp"
#include "mesh/TextureManager.hpp"
#include "mesh/Material.hpp"
#include "util/AlignedVector.hpp"
#include "scene/RenderContext.hpp"

namespace scene {

struct Scene;

struct MeshInstance {
	math::vec4 pos_scale;
	math::Quaternion rot;
};

struct MeshInstanceHandle {
	u16 mesh_id;
	u16 instance_id;

	MeshInstance& resolve(Scene& scene);
};

struct Light {
	math::vec3 pos;
	math::vec3 color;
};

struct DirectionalLight {
	math::vec3 direction;
	math::vec3 color;
};

struct Camera {
	math::vec3 pos;
	math::Quaternion rot;
	float fov;
	float clip_near;
	float clip_far;
};

struct GBufferSet {
	gl::Framebuffer fbo;
	gl::Texture depth_tex;
	gl::Texture diffuse_tex;
	gl::Texture normal_tex;

	void initialize(int width, int height);
};

struct ShadingBufferSet {
	gl::Framebuffer fbo;
	gl::Texture accum_tex;

	void initialize(int width, int height, gl::Texture& depth_tex);
};

struct Scene {
	TextureManager tex_manager;
	std::vector<Material> material_list;
	std::vector<GPUMesh> gpu_meshes;
	std::vector<util::AlignedVector<MeshInstance>> mesh_instances;
	util::AlignedVector<Light> lights;

	int addMaterial(Material&& mat);
	int addMesh(GPUMesh&& mesh);
	MeshInstanceHandle newInstance(int mesh_id);
};

inline MeshInstance& MeshInstanceHandle::resolve(Scene& scene) {
	return scene.mesh_instances[mesh_id][instance_id];
}

void bindGBufferTextures(GBufferSet& gbuffer);

void renderGeometry(
	const Scene& scene,
	const Camera& camera,
	GBufferSet& buffers,
	RenderContext& render_context,
	const SystemUniformBlock& sys_uniforms);
void shadeDirectionalLights(
	const std::vector<DirectionalLight>& lights,
	const Material& light_material,
	RenderContext& render_context,
	const SystemUniformBlock& sys_uniforms);

} // namespace scene
