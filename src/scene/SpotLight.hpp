#pragma once

#include "math/vec.hpp"
#include "Transform.hpp"
#include <vector>

struct Material;

namespace scene {

struct RenderContext;
struct SystemUniformBlock;

struct SpotLight {
	Transform t;
	math::vec3 color;
	float exponent;
};

struct GPUSpotLight {
	math::vec3 pos;
	math::vec4 dir_exp; // xyz: Direction, w: Exponent
	math::vec3 color;
};

void transformSpotLights(
	const std::vector<SpotLight>& in_lights,
	std::vector<GPUSpotLight>& out_lights,
	const math::mat4& world2view_mat);

void shadeSpotLights(
	const std::vector<GPUSpotLight>& lights,
	const Material& light_material,
	RenderContext& render_context,
	const SystemUniformBlock& sys_uniforms);

} // namespace scene
