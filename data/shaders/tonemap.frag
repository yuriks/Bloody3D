#version 330 core

#include "common.glsl"

in vec2 vf_TexCoord;

layout(location = 0) out vec3 out_Color;

float srgb(float c) {
	if (c < 0.0031308) {
		return 12.92 * c;
	} else {
		return 1.055 * pow(c, 0.41666) - 0.055;
	}
}

vec3 convSrgbColor(vec3 col) {
	return vec3(srgb(col.r), srgb(col.g), srgb(col.b));
}

void main() {
	// TODO: No actual tonemapping yet.
	out_Color = convSrgbColor(texture(tex[0], vf_TexCoord).rgb);
}
