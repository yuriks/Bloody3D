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
#include "util/StringHash.hpp"
#include <unordered_map>
#include <string>

struct Scene;

struct MeshInstance {
	Handle transform;
	Handle mesh_id;
	Handle material_opts;

	MeshInstance() {}

	MeshInstance(Handle transform, Handle mesh)
		: transform(transform), mesh_id(mesh)
	{}

	template <typename T>
	void reflect(T& f) {
		f(transform,     HASHSTR("transform"));
		f(mesh_id,       HASHSTR("mesh_id"));
		f(material_opts, HASHSTR("material_params"));
	}
};

struct Camera {
	Handle transform;
	float fov;
	float clip_near;
	float clip_far;

	template <typename T>
	void reflect(T& f) {
		f(transform, HASHSTR("transform"));
		f(fov,       HASHSTR("fov"));
		f(clip_near, HASHSTR("clip_near"));
		f(clip_far,  HASHSTR("clip_far"));
	}
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
	Engine* engine;
	std::unordered_map<std::string, Handle> named_handles;

	ObjectPool<Transform> transforms;
	ObjectPool<MeshInstance> mesh_instances;
	ObjectPool<MaterialOptions> material_options;
	ObjectPool<Camera> cameras;
	Handle active_camera;

	ObjectPool<DirectionalLight> lights_dir;
	ObjectPool<OmniLight> lights_omni;
	ObjectPool<SpotLight> lights_spot;

	Scene(Engine* engine)
		: engine(engine)
	{}
};

void bindGBufferTextures(GBufferSet& gbuffer);

void renderGeometry(
	const Scene& scene,
	const mat4& world2view_mat,
	const mat4* model2world_mats,
	GBufferSet& buffers,
	const RenderContext& render_context,
	const SystemUniformBlock& sys_uniforms);
