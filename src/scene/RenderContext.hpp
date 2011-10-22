#pragma once

#include "gl/BufferObject.hpp"
#include "math/Matrix.hpp"

namespace scene {

struct SystemUniformBlock {
	math::mat4 projection_mat;
	math::mat3x4 view_model_mat;
};

struct RenderContext {
	RenderContext(int width, int height);

	gl::BufferObject system_ubo;
	gl::BufferObject material_ubo;

	int screen_width;
	int screen_height;
	float aspect_ratio;
};

} // namespace scene
