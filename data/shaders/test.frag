#version 330 core

#include "common.glsl"

in VertFragData vf;

layout(location = 0) out vec4 out_Color;
layout(location = 1) out vec4 out_Normal;

void main(void) {
	float ratio = dot(vf.normal, vec3(0.f, 0.f, -1.f));
	out_Color = vec4(texture(tex[0], vf.tex_coord).rgb * ratio, 1.0);
	out_Normal = vec4(vf.normal, 0.0);
};
