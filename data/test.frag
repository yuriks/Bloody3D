#version 330

layout(std140) uniform;

struct Light
{
	vec3 position;
	vec3 color;
};

const int MAX_LIGHTS = 4;
uniform LightBlock
{
	Light lights[MAX_LIGHTS];
} u_LightBlock;

uniform sampler2D u_DiffuseMap;
uniform sampler2D u_NormalMap;

in vec3 ex_Position;
in vec3 ex_Normal;
in vec4 ex_Color;
in vec2 ex_TexCoord;

out vec4 out_Color;

void main(void)
{
	vec4 diffuse_col = texture(u_DiffuseMap, ex_TexCoord) * ex_Color;

	vec3 norm_normal = normalize(ex_Normal);

	vec3 col = vec3(0.1) * diffuse_col.rgb;

	for (int i = 0; i < MAX_LIGHTS; ++i) {
		if (u_LightBlock.lights[i].color != vec3(0.0))
		{
			vec3 norm_light_dir = normalize(u_LightBlock.lights[i].position - ex_Position);

			float n_dot_l = max(dot(norm_light_dir, norm_normal), 0.0);
			vec3 diffuse = diffuse_col.rgb * n_dot_l * u_LightBlock.lights[i].color;

			vec3 r = reflect(-norm_light_dir, norm_normal);
			float r_dot_v = dot(r, vec3(0.0, 0.0, -1.0));
			vec3 specular = pow(max(r_dot_v, 0.0), 30.0) * vec3(0.6);

			col += diffuse + specular;
		}
	}

	out_Color = vec4(col, diffuse_col.a);
};
