#include "scene/PostProcessing.hpp"

namespace scene {

void tonemap(const ShadingBufferSet& shading_buffers, const Material& material, const RenderContext& render_context) {
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glClear(GL_COLOR_BUFFER_BIT);

	material.shader_program.use();

	render_context.material_ubo.bind(GL_UNIFORM_BUFFER);
	glBufferData(GL_UNIFORM_BUFFER, material.options_size, 0, GL_STREAM_DRAW);

	glActiveTexture(GL_TEXTURE0);
	shading_buffers.accum_tex.bind(GL_TEXTURE_2D);

	glDrawArrays(GL_TRIANGLES, 0, 3);
}

} // namespace scene
