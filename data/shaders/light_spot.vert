#version 330 core

#include "common.glsl"
#include "fullscreen_triangle.glsl"

layout(location = 0) in vec3 in_light_pos;
layout(location = 1) in vec4 in_light_dir_exp;
layout(location = 2) in vec3 in_light_color;

flat out GPUSpotLight vf_light;

void main() {
	doFullscreenTriangle();
	vf_light.pos = in_light_pos;
	vf_light.dir_exp = in_light_dir_exp;
	vf_light.color = in_light_color;
}
