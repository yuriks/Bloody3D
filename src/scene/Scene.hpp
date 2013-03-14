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
#include "scene/Transform.hpp"

namespace scene {

struct Scene;

struct MeshInstance {
	Transform t;
};

struct MeshInstanceHandle {
	u16 mesh_id;
	u16 instance_id;

	MeshInstance& resolve(Scene& scene);
};

struct Light {
	Transform t;
	math::vec3 color;
};

struct DirectionalLight {
	Transform t;
	math::vec3 color;
};

struct GPUDirectionalLight {
	math::vec3 direction;
	math::vec3 color;
};

struct Camera {
	Transform t;
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
	const math::mat4& world2view_mat,
	GBufferSet& buffers,
	RenderContext& render_context,
	const SystemUniformBlock& sys_uniforms);

void transformDirectionalLights(
	const std::vector<DirectionalLight>& in_lights,
	std::vector<GPUDirectionalLight>& out_lights,
	const math::mat4& world2view_mat);
void shadeDirectionalLights(
	const std::vector<GPUDirectionalLight>& lights,
	const Material& light_material,
	RenderContext& render_context,
	const SystemUniformBlock& sys_uniforms);

} // namespace scene
