#pragma once

#include "scene/Scene.hpp"
#include "scene/RenderContext.hpp"
#include "mesh/Material.hpp"
#include <vector>
#include "Engine.hpp"
#include "math/mat.hpp"
#include "scene/Scene.hpp"
#include "scene/RenderContext.hpp"

void tonemap(const ShadingBufferSet& shading_buffers, const Material& material, const RenderContext& render_context);

struct ShadeLightSetParams {
	const mat4* world2view_mat;
	const mat4* model2world_mats;
	const Engine* engine;
	const Scene* scene;
	const SystemUniformBlock* sys_uniforms;
};

template <typename L, typename GPUL>
void shadeLightSet(const std::vector<L>& lights, const LightInfo& light_info, const ShadeLightSetParams& p) {
	std::vector<GPUL> gpu_dirlights;
	transformLights(
		lights, gpu_dirlights,
		*p.world2view_mat, p.scene->transforms, p.model2world_mats);
	light_info.vao.bind();
	light_info.vbo.bind(GL_ARRAY_BUFFER);
	shadeLights(gpu_dirlights, *p.engine->materials[light_info.material], p.engine->render_context, *p.sys_uniforms);
}
