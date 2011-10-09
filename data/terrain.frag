#version 330 core

layout(std140) uniform;

struct Light
{
	vec3 direction;
	vec3 color;
};

uniform LightBlock
{
	Light lights;
} u_LightBlock;

in vec3 vf_Position;
in vec3 vf_Normal;
in vec4 vf_Color;

out vec4 out_Color;

void main(void)
{
	vec4 diffuse_col = vf_Color;

	vec3 norm_normal = normalize(vf_Normal);

	//////////////////
	// Ambient term //
	//////////////////
	vec3 ambient = vec3(0.1) * diffuse_col.rgb;

	//////////////////
	// Diffuse term //
	//////////////////
	vec3 norm_light_dir = -u_LightBlock.lights.direction;
	float n_dot_l = max(dot(norm_normal, norm_light_dir), 0.0);

	///////////////////
	// Specular term //
	///////////////////
	vec3 h = normalize(norm_light_dir + vec3(0.0, 0.0, -1.0));
	float n_dot_h = max(dot(norm_normal, h), 0.0);
	vec3 specular = diffuse_col.aaa * pow(n_dot_h, 60.0);

	vec3 col = ambient + (diffuse_col.rgb + specular) * (u_LightBlock.lights.color * n_dot_l);

	out_Color = vec4(col, diffuse_col.a);
};
