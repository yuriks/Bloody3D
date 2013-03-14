#version 330 core

#include "common.glsl"
#include "fullscreen_triangle.glsl"

layout(location = 0) in vec3 in_light_pos;
layout(location = 1) in vec3 in_light_color;

flat out GPUOmniLight vf_light;

void main() {
	doFullscreenTriangle();
	vf_light.pos = in_light_pos;
	vf_light.color = in_light_color;
}
