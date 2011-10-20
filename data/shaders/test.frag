#version 330 core

#include "common.glsl"

in vec4 vf_Color;

layout(location = 0) out vec4 out_Color;

void main(void) {
	out_Color = vf_Color;
};
