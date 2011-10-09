#version 330 core

uniform mat4 in_ViewModelMat;
uniform mat4 in_ProjMat;
uniform sampler1D u_ColorLookup;
uniform sampler2D u_Heightmap;
uniform int u_LodLevel;

in vec2 in_Position;
in vec2 in_TexCoord;

out vec3 vf_Position;
out vec3 vf_Normal;
out vec4 vf_Color;
//out vec2 ex_TexCoord;

vec2 norm(vec2 v)
{
	return normalize(vec2(v.y, -v.x));
}

void main(void)
{
	const ivec3 fetch_offset = ivec3(-1, 0, 1);
	const float h_scale = 16.0;

	float scale = float(1 << u_LodLevel);
	float size = 0.1 * scale;

	float h = textureLodOffset(u_Heightmap, in_TexCoord * scale, u_LodLevel, fetch_offset.yy).r;

	float hr = textureLodOffset(u_Heightmap, in_TexCoord * scale, u_LodLevel, fetch_offset.zy).r;
	float ht = textureLodOffset(u_Heightmap, in_TexCoord * scale, u_LodLevel, fetch_offset.yz).r;

	vec3 a = normalize(vec3(size, (hr - h)*h_scale, 0.0));
	vec3 b = normalize(vec3(0.0, (ht - h)*h_scale, size));

	vec3 average = normalize(cross(b, a));

	vf_Normal = (in_ViewModelMat * vec4(average, 0.0)).xyz;

	vec4 pos = in_ViewModelMat * vec4(in_Position.x * scale, h * h_scale, in_Position.y * scale, 1.0);
	gl_Position = in_ProjMat * pos;
	vf_Position = pos.xyz;

	vf_Color = texture(u_ColorLookup, h);
};
