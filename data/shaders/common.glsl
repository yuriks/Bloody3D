layout(std140, row_major) uniform;

uniform SystemUniforms {
	mat4 projection_mat;
	mat4 view_model_mat;
} u;

uniform MaterialUniforms {
	float dummy;
} mtl;

uniform sampler2D tex[4];

vec2 encodeNormal(vec3 n) {
	return n.xy * inversesqrt(8 - 8 * n.z) + 0.5;
}

vec3 decodeNormal(vec2 renc) {
	vec2 enc = 2 * renc - 1;
	float len2 = dot(enc, enc);
	return vec3(enc * sqrt(4 - 4 * len2), 2 * len2 - 1);
}

struct VertFragData {
	vec3 normal;
	vec2 tex_coord;
};
