#version 330 core

#include "common.glsl"

in VertFragData vf;

layout(location = 0) out vec4 out_Color;
layout(location = 1) out vec2 out_Normal;

void main(void) {
	out_Color = vec4(texture(tex[0], vf.tex_coord).rgb, 1.0);
	out_Normal = encodeNormal(normalize(vf.normal));
}
