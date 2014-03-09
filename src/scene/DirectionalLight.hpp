#pragma once

#include "math/vec.hpp"
#include "math/mat.hpp"
#include "Transform.hpp"
#include "util/ObjectPool.hpp"
#include "Engine.hpp"
#include "util/StringHash.hpp"
#include <vector>

struct Material;

struct RenderContext;
struct SystemUniformBlock;

struct DirectionalLight {
	Handle transform;
	vec3 color;

	template <typename T>
	void reflect(T& f) {
		f(transform, HASHSTR("transform"));
		f(color,     HASHSTR("color"));
	}
};

struct GPUDirectionalLight {
	vec3 direction;
	vec3 color;
};

void setupDirLightVao(LightInfo& info);

void transformLights(
	const std::vector<DirectionalLight>& in_lights,
	std::vector<GPUDirectionalLight>& out_lights,
	const mat4& world2view_mat,
	const ObjectPool<Transform>& transforms,
	const mat4* model2world_mats);

void shadeLights(
	const std::vector<GPUDirectionalLight>& lights,
	const Material& light_material,
	const RenderContext& render_context,
	const SystemUniformBlock& sys_uniforms);
