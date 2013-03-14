#version 330 core

#include "common.glsl"
#include "fullscreen_triangle.glsl"

layout(location = 0) in vec3 in_light_dir;
layout(location = 1) in vec3 in_light_color;

flat out GPUDirectionalLight vf_light;

void main() {
	doFullscreenTriangle();
	vf_light.dir = in_light_dir;
	vf_light.color = in_light_color;
}
