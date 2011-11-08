#version 330 core

#include "common.glsl"

in vec2 vf_TexCoord;

layout(location = 0) out vec3 out_Color;

void main() {
	// TODO: No actual tonemapping yet.
	out_Color = texture(tex[0], vf_TexCoord).rgb;
}
