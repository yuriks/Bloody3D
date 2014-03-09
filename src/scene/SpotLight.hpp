#pragma once

#include "math/vec.hpp"
#include "Transform.hpp"
#include "Engine.hpp"
#include "util/StringHash.hpp"
#include <vector>

struct Material;

struct RenderContext;
struct SystemUniformBlock;

struct SpotLight {
	Handle transform;
	vec3 color;
	float exponent;

	template <typename T>
	void reflect(T& f) {
		f(transform, HASHSTR("transform"));
		f(color,     HASHSTR("color"));
		f(exponent,  HASHSTR("exponent"));
	}
};

struct GPUSpotLight {
	vec3 pos;
	vec4 dir_exp; // xyz: Direction, w: Exponent
	vec3 color;
};

void setupSpotLightVao(LightInfo& info);

void transformLights(
	const std::vector<SpotLight>& in_lights,
	std::vector<GPUSpotLight>& out_lights,
	const mat4& world2view_mat,
	const ObjectPool<Transform>& transforms,
	const mat4* model2world_mats);

void shadeLights(
	const std::vector<GPUSpotLight>& lights,
	const Material& light_material,
	const RenderContext& render_context,
	const SystemUniformBlock& sys_uniforms);
