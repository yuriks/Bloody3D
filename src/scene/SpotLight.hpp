#pragma once

#include "math/vec.hpp"
#include "Transform.hpp"
#include "Engine.hpp"
#include "util/StringHash.hpp"
#include <vector>

struct Material;

namespace scene {

struct RenderContext;
struct SystemUniformBlock;

struct SpotLight {
	Handle transform;
	math::vec3 color;
	float exponent;

	template <typename T>
	void reflect(T& f) {
		f(transform, HASHSTR("transform"));
		f(color,     HASHSTR("color"));
		f(exponent,  HASHSTR("exponent"));
	}
};

struct GPUSpotLight {
	math::vec3 pos;
	math::vec4 dir_exp; // xyz: Direction, w: Exponent
	math::vec3 color;
};

void setupSpotLightVao(LightInfo& info);

void transformLights(
	const std::vector<SpotLight>& in_lights,
	std::vector<GPUSpotLight>& out_lights,
	const math::mat4& world2view_mat,
	const ObjectPool<Transform>& transforms,
	const math::mat4* model2world_mats);

void shadeLights(
	const std::vector<GPUSpotLight>& lights,
	const Material& light_material,
	const RenderContext& render_context,
	const SystemUniformBlock& sys_uniforms);

} // namespace scene
