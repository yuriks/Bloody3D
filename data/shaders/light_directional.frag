#version 330 core

#include "common.glsl"

uniform MaterialUniforms {
	// Pre-transformed
	vec3 light_dir;
	vec3 light_color;
} mtl;

in vec2 vf_TexCoord;

layout(location = 0) out vec4 out_Color;

vec3 brdf(vec3 normal, vec3 light_dir, vec3 view_dir) {
	vec3 diffuse_term = texture(tex[TEX_DIFFUSE], vf_TexCoord).rgb / C_PI;

	return diffuse_term;
}

vec3 surface_exitance(vec3 normal, vec3 light_dir, vec3 view_dir) {
	vec3 surface_reflectance = brdf(normal, light_dir, view_dir);
	float cos_theta = max(0, dot(normal, light_dir));

	return cos_theta * surface_reflectance * mtl.light_color;
}

void main() {
	vec3 normal = decodeNormal(texture(tex[TEX_NORMAL], vf_TexCoord).rg);

	out_Color = vec4(surface_exitance(normal, -mtl.light_dir, vec3(0, 0, -1)), 1.0);
}
