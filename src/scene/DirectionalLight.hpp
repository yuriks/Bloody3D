#pragma once

#include "math/vec.hpp"
#include "math/mat.hpp"
#include "Transform.hpp"
#include "util/ObjectPool.hpp"
#include <vector>

struct Material;

namespace scene {

struct RenderContext;
struct SystemUniformBlock;

struct DirectionalLight {
	Handle transform;
	math::vec3 color;
};

struct GPUDirectionalLight {
	math::vec3 direction;
	math::vec3 color;
};

void transformLights(
	const std::vector<DirectionalLight>& in_lights,
	std::vector<GPUDirectionalLight>& out_lights,
	const math::mat4& world2view_mat,
	const ObjectPool<Transform>& transforms,
	const math::mat4* model2world_mats);

void shadeLights(
	const std::vector<GPUDirectionalLight>& lights,
	const Material& light_material,
	const RenderContext& render_context,
	const SystemUniformBlock& sys_uniforms);

} // namespace scene
