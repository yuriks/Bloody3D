in vec2 vf_TexCoord;
in vec2 vf_ViewspacePos;

layout(location = 0) out vec4 out_Color;

vec3 calcViewspacePosition() {
	float proj_a = u.projection_mat[2][2];
	float proj_b = u.projection_mat[3][2];
	float projected_z = texture(tex[TEX_DEPTH], vf_TexCoord).r;
	float view_z = proj_b / (projected_z - proj_a) * 0.5f;
	return vec3(vf_ViewspacePos * view_z, view_z);
}

vec3 brdf(vec3 normal, vec3 light_dir, vec3 view_dir) {
	vec3 diffuse_term = texture(tex[TEX_DIFFUSE], vf_TexCoord).rgb / C_PI;

	vec3 spec_color = 0.02f * vec3(1, 1, 1);
	float spec_m = 1000;
	vec3 h = normalize(light_dir + view_dir);
	vec3 specular_term = (spec_m + 8)/(8*C_PI) * pow(dot(normal, h), spec_m) * spec_color;

	return diffuse_term + specular_term;
}
