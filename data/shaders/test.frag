#version 330 core

#include "common.glsl"

in VertFragData vf;

layout(location = 0) out vec4 out_Color;
layout(location = 1) out vec2 out_Normal;

mat3 cotangent_frame(vec3 N, vec3 p, vec2 uv) {
	// Code verbatim from http://www.thetenthplanet.de/archives/1180
	// "Followup: Normal Mapping Without Precomputed Tangents", Christian Schüler

	// get edge vectors of the pixel triangle
	vec3 dp1 = dFdx(p);
	vec3 dp2 = dFdy(p);
	vec2 duv1 = dFdx(uv);
	vec2 duv2 = dFdy(uv);

	// solve the linear system
	vec3 dp2perp = cross(dp2, N);
	vec3 dp1perp = cross(N, dp1);
	vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
	vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;

	// construct a scale-invariant frame
	float invmax = inversesqrt(max(dot(T,T), dot(B,B)));
	return mat3(T * invmax, B * invmax, N);
}

void main(void) {
	out_Color = vec4(texture(tex[0], vf.tex_coord).rgb, 1.0);

	vec3 map_normal = texture(tex[1], vf.tex_coord).xyz * 255./127. - 128./127.;
	mat3 TBN = cotangent_frame(normalize(vf.normal), -vf.position, vf.tex_coord);
	out_Normal = encodeNormal(TBN * map_normal);
}
