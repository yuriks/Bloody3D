#pragma once

#include "gl/Framebuffer.hpp"
#include "gl/Texture.hpp"
#include "math/vec.hpp"
#include "math/Quaternion.hpp"
#include "mesh/GPUMesh.hpp"
#include "mesh/Material.hpp"
#include "util/AlignedVector.hpp"
#include "scene/RenderContext.hpp"
#include "scene/Transform.hpp"
#include "Engine.hpp"
#include "util/ObjectPool.hpp"

namespace scene {

struct Scene;

struct MeshInstance {
	Transform t;
	Handle mesh_id;
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
	const Engine* engine;

	ObjectPool<MeshInstance> mesh_instances;

	Scene(const Engine* engine)
		: engine(engine)
	{}

	Handle newInstance(Handle mesh_id);
};

void bindGBufferTextures(GBufferSet& gbuffer);

void renderGeometry(
	const Scene& scene,
	const math::mat4& world2view_mat,
	GBufferSet& buffers,
	RenderContext& render_context,
	const SystemUniformBlock& sys_uniforms);

} // namespace scene
