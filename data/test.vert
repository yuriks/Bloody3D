#version 330

uniform mat4 in_ViewModelMat;
uniform mat4 in_ProjMat;

in vec3 in_Position;
in vec3 in_Normal;
in vec2 in_TexCoord;

out vec3 ex_Position;
out vec3 ex_Normal;
out vec4 ex_Color;
out vec2 ex_TexCoord;

void main(void)
{
	vec4 pos = in_ViewModelMat * vec4(in_Position.xyz, 1.0);
	gl_Position = in_ProjMat * pos;
	ex_Position = pos.xyz;

	ex_Color = vec4(1.0, 1.0, 1.0, 1.0);
	ex_Normal = (in_ViewModelMat * vec4(in_Normal, 0.0)).xyz;
	ex_TexCoord = in_TexCoord;
};
