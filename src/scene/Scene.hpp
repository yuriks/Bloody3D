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
#include "scene/DirectionalLight.hpp"
#include "scene/OmniLight.hpp"
#include "scene/SpotLight.hpp"
#include "Engine.hpp"
#include "util/ObjectPool.hpp"

namespace scene {

struct Scene;

struct MeshInstance {
	Handle transform;
	Handle mesh_id;

	MeshInstance(Handle transform, Handle mesh)
		: transform(transform), mesh_id(mesh)
	{}
};

struct Camera {
	Handle transform;
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

	ObjectPool<Transform> transforms;
	ObjectPool<MeshInstance> mesh_instances;
	ObjectPool<Camera> cameras;
	Handle active_camera;

	ObjectPool<DirectionalLight> lights_dir;
	ObjectPool<OmniLight> lights_omni;
	ObjectPool<SpotLight> lights_spot;

	Scene(const Engine* engine)
		: engine(engine)
	{}
};

void bindGBufferTextures(GBufferSet& gbuffer);

void renderGeometry(
	const Scene& scene,
	const math::mat4& world2view_mat,
	const math::mat4* model2world_mats,
	GBufferSet& buffers,
	const RenderContext& render_context,
	const SystemUniformBlock& sys_uniforms);

} // namespace scene
