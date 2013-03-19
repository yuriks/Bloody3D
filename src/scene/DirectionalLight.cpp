#include "DirectionalLight.hpp"

#include "Scene.hpp"
#include "GL/gl3w.h"

namespace scene {

void transformLights(
	const std::vector<DirectionalLight>& in_lights,
	std::vector<GPUDirectionalLight>& out_lights,
	const math::mat4& world2view_mat,
	const ObjectPool<Transform>& transforms,
	const math::mat4* model2world_mats)
{
	out_lights.resize(in_lights.size());
	for (size_t i = 0; i < in_lights.size(); ++i) {
		const math::mat4 model2view_mat = world2view_mat * model2world_mats[transforms.getPoolIndex(in_lights[i].transform)];
		out_lights[i].direction = -math::mvec3(model2view_mat * math::mvec4(math::vec3_z, 0.0f));
		out_lights[i].color = in_lights[i].color;
	}
}

// Shades a collection of directional lights to the shading buffer. Lights are in view-space.
void shadeLights(
	const std::vector<GPUDirectionalLight>& lights,
	const Material& light_material,
	const RenderContext& render_context,
	const SystemUniformBlock& sys_uniforms)
{
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	light_material.shader_program.use();

	render_context.system_ubo.bind(GL_UNIFORM_BUFFER);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(SystemUniformBlock), &sys_uniforms, GL_STREAM_DRAW);

	glBufferData(GL_ARRAY_BUFFER, sizeof(GPUDirectionalLight) * lights.size(), lights.data(), GL_STREAM_DRAW);
	glDrawArraysInstanced(GL_TRIANGLES, 0, 3, lights.size());
	glBindVertexArray(0);
}

} // namespace scene
