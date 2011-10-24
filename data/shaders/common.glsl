layout(std140, row_major) uniform;

struct mat3x4_rows {
	vec4 rows[3];
};

uniform SystemUniforms {
	mat4 projection_mat;
	mat3x4_rows view_model_mat;
} u;

uniform MaterialUniforms {
	float dummy;
} mtl;

uniform sampler2D tex[4];

vec4 transform(mat3x4_rows m, vec4 v) {
	return vec4(dot(m.rows[0], v), dot(m.rows[1], v), dot(m.rows[2], v), v.w);
}

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
