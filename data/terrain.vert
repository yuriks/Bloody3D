#version 330 core
layout(std140, row_major) uniform;

struct mat3x4_rows {
	vec4 rows[3];
};

uniform UniformBlock {
	mat4 projection_mat;
	mat3x4_rows view_model_mat;
} u;

in vec3 in_Position;

out vec4 vf_Color;

vec4 transform(mat3x4_rows m, vec4 v) {
	return vec4(dot(m.rows[0], v), dot(m.rows[1], v), dot(m.rows[2], v), v.w);
}

void main(void) {
	vec4 pos = transform(u.view_model_mat, vec4(in_Position, 1.0));
	gl_Position = u.projection_mat * pos;
	vf_Color = vec4(vec3(in_Position.xyz * 0.5 + 0.5) * (1.0 - pos.z * 0.1), 1.0);
};
