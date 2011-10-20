#version 330 core

#include "common.glsl"

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec2 in_TexCoord;

out VertFragData vf;

void main(void) {
	vec4 pos = transform(u.view_model_mat, vec4(in_Position, 1.0));
	vf.normal = transform(u.view_model_mat, vec4(in_Normal, 0.0)).xyz;
	vf.tex_coord = in_TexCoord;
	gl_Position = u.projection_mat * pos;
};
