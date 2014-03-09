#include "OmniLight.hpp"

#include "Scene.hpp"
#include "GL/gl3w.h"

void setupOmniLightVao(LightInfo& info) {
	info.vao.bind();

	info.vbo.bind(GL_ARRAY_BUFFER);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
		sizeof(GPUOmniLight), (void*)offsetof(GPUOmniLight, pos));
	glEnableVertexAttribArray(0);
	glVertexAttribDivisor(0, 1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE,
		sizeof(GPUOmniLight), (void*)offsetof(GPUOmniLight, color));
	glEnableVertexAttribArray(1);
	glVertexAttribDivisor(1, 1);
}

void transformLights(
	const std::vector<OmniLight>& in_lights,
	std::vector<GPUOmniLight>& out_lights,
	const mat4& world2view_mat,
	const ObjectPool<Transform>& transforms,
	const mat4* model2world_mats)
{
	out_lights.resize(in_lights.size());
	for (size_t i = 0; i < in_lights.size(); ++i) {
		const mat4 model2view_mat = world2view_mat * model2world_mats[transforms.getPoolIndex(in_lights[i].transform)];
		out_lights[i].pos = mvec3(model2view_mat * mvec4(vec3_0, 1.0f));
		out_lights[i].color = in_lights[i].color;
	}
}

// Shades a collection of omnilights to the shading buffer. Lights are in view-space.
void shadeLights(
	const std::vector<GPUOmniLight>& lights,
	const Material& light_material,
	const RenderContext& render_context,
	const SystemUniformBlock& sys_uniforms)
{
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	light_material.shader_program.use();

	render_context.system_ubo.bind(GL_UNIFORM_BUFFER);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(SystemUniformBlock), &sys_uniforms, GL_STREAM_DRAW);

	glBufferData(GL_ARRAY_BUFFER, sizeof(GPUOmniLight) * lights.size(), lights.data(), GL_STREAM_DRAW);
	glDrawArraysInstanced(GL_TRIANGLES, 0, 3, lights.size());
	glBindVertexArray(0);
}
