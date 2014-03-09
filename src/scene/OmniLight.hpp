#pragma once

#include "math/vec.hpp"
#include "Transform.hpp"
#include "Engine.hpp"
#include "util/StringHash.hpp"
#include <vector>

struct Material;

struct RenderContext;
struct SystemUniformBlock;

struct OmniLight {
	Handle transform;
	vec3 color;

	template <typename T>
	void reflect(T& f) {
		f(transform, HASHSTR("transform"));
		f(color,     HASHSTR("color"));
	}
};

struct GPUOmniLight {
	vec3 pos;
	vec3 color;
};

void setupOmniLightVao(LightInfo& info);

void transformLights(
	const std::vector<OmniLight>& in_lights,
	std::vector<GPUOmniLight>& out_lights,
	const mat4& world2view_mat,
	const ObjectPool<Transform>& transforms,
	const mat4* model2world_mats);

void shadeLights(
	const std::vector<GPUOmniLight>& lights,
	const Material& light_material,
	const RenderContext& render_context,
	const SystemUniformBlock& sys_uniforms);
