#version 330 core

uniform mat4 in_ViewModelMat;

in vec3 in_Position;

out vec4 vf_Color;

void main(void) {
	vec4 pos = in_ViewModelMat * vec4(in_Position, 1.0);
	gl_Position = pos;
	vf_Color = vec4(vec3(in_Position.xyz * 0.5 + 0.5) * (-gl_Position.z * 0.5 + 0.5), 1.0);
};
