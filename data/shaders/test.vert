#version 330 core

#include "common.glsl"

layout(location = 0) in vec3 in_Position;

out vec4 vf_Color;

void main(void) {
	vec4 pos = transform(u.view_model_mat, vec4(in_Position, 1.0));
	gl_Position = u.projection_mat * pos;
	vf_Color = vec4(vec3(in_Position.xyz * 0.5 + 0.5) * (1.0 - pos.z * 0.1), 1.0);
};
