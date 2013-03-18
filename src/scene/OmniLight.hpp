#pragma once

#include "math/vec.hpp"
#include "Transform.hpp"
#include <vector>

struct Material;

namespace scene {

struct RenderContext;
struct SystemUniformBlock;

struct OmniLight {
	Handle transform;
	math::vec3 color;
};

struct GPUOmniLight {
	math::vec3 pos;
	math::vec3 color;
};

void transformOmniLights(
	const std::vector<OmniLight>& in_lights,
	std::vector<GPUOmniLight>& out_lights,
	const math::mat4& world2view_mat,
	const ObjectPool<Transform>& transforms,
	const math::mat4* model2world_mats);

void shadeOmniLights(
	const std::vector<GPUOmniLight>& lights,
	const Material& light_material,
	RenderContext& render_context,
	const SystemUniformBlock& sys_uniforms);

} // namespace scene
