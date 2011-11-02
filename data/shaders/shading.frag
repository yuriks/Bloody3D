#version 330 core

#include "common.glsl"

in vec2 vf_TexCoord;

layout(location = 0) out vec4 out_Color;

#define TEX_DEPTH 0
#define TEX_DIFFUSE 1
#define TEX_NORMAL 2

void main() {
	vec3 normal = decodeNormal(texture(tex[TEX_NORMAL], vf_TexCoord).rg);
	//vec3 normal = texture(tex[TEX_NORMAL], vf_TexCoord).rgb;
	float ratio = dot(normal, vec3(0, 0, -1));
	out_Color = vec4(texture(tex[TEX_DIFFUSE], vf_TexCoord).rgb * ratio, 1.0);
	//out_Color = vec4(normal * 0.5 + 0.5, 1.0);
}
