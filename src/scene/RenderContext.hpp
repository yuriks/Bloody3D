#pragma once

#include "gl/VertexArrayObject.hpp"
#include "gl/BufferObject.hpp"
#include "math/mat.hpp"

struct SystemUniformBlock {
	mat4 projection_mat;
	mat4 view_model_mat;
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
