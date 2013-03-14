#pragma once

#include "math/vec.hpp"
#include "Transform.hpp"
#include <vector>

struct Material;

namespace scene {

struct RenderContext;
struct SystemUniformBlock;

struct DirectionalLight {
	Transform t;
	math::vec3 color;
};

struct GPUDirectionalLight {
	math::vec3 direction;
	math::vec3 color;
};

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
