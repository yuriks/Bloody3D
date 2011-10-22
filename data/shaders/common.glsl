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
	vec2 enc = normalize(n.xy) * sqrt(-n.z);
	return enc;
}

vec3 decodeNormal(vec2 enc) {
	vec3 n;
	n.z = -dot(enc, enc);
	n.xy = normalize(enc) * sqrt(1 - n.z*n.z);
	return n;
}

struct VertFragData {
	vec3 normal;
	vec2 tex_coord;
};
