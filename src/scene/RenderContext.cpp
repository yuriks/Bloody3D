#include "RenderContext.hpp"

namespace scene {

RenderContext::RenderContext(int width, int height)
	: screen_width(width), screen_height(height),
	aspect_ratio((float)width / (float)height)
{
	system_ubo.bind(GL_UNIFORM_BUFFER);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(scene::SystemUniformBlock), 0, GL_STREAM_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, system_ubo);

	material_ubo.bind(GL_UNIFORM_BUFFER);
	glBufferData(GL_UNIFORM_BUFFER, 4, 0, GL_STREAM_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, material_ubo);

	// Initialize unit cube
	{
		static const signed char cube_vertices[8*3] = {
			-1,  1, -1, // 0
			 1,  1, -1, // 1
			-1, -1, -1, // 2
			 1, -1, -1, // 3
			-1,  1,  1, // 4
			 1,  1,  1, // 5
			-1, -1,  1, // 6
			 1, -1,  1  // 7
		};
		static const unsigned char cube_indices[6*2*3] = { // Fuck yeah grease pencil
			1, 5, 3,  5, 7, 3, // +X
			4, 0, 6,  0, 2, 6, // -X
			4, 5, 0,  5, 1, 0, // +Y
			2, 3, 6,  3, 7, 6, // -Y
			5, 4, 7,  4, 6, 7, // +Z
			0, 1, 2,  1, 3, 2  // -Z
		};

		cube_vao.bind();
		cube_vbo.bind(GL_ARRAY_BUFFER);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_BYTE, GL_FALSE, 3, 0);
		glEnableVertexAttribArray(0);

		cube_ibo.bind(GL_ELEMENT_ARRAY_BUFFER);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_indices), cube_indices, GL_STATIC_DRAW);
	}
}

} // namespace scene
