#pragma once

#include "gl/VertexArrayObject.hpp"
#include "gl/BufferObject.hpp"
#include "math/mat.hpp"

namespace scene {

struct SystemUniformBlock {
	math::mat4 projection_mat;
	math::mat4 view_model_mat;
};

struct RenderContext {
	RenderContext();
	void setScreenSize(int width, int height);

	gl::BufferObject system_ubo;
	gl::BufferObject material_ubo;

	int screen_width;
	int screen_height;
	float aspect_ratio;
};

} // namespace scene
