#version 330 core

#include "common.glsl"
#include "lighting.glsl"

flat in GPUDirectionalLight vf_light;

vec3 surface_radiance(vec3 normal, vec3 light_dir, vec3 view_dir) {
	vec3 surface_reflectance = brdf(normal, light_dir, view_dir);
	float cos_theta = max(0, dot(normal, light_dir));

	return cos_theta * surface_reflectance * vf_light.color;
}

void main() {
	vec3 normal = decodeNormal(texture(tex[TEX_NORMAL], vf_TexCoord).rg);
	vec3 position = calcViewspacePosition();

	out_Color = vec4(surface_radiance(normal, vf_light.dir, -normalize(position)), 1.0);
}
