#version 330

// in_Position was bound to attribute index 0 and in_Color was bound to attribute index 1
in  vec3 in_Position;
in  vec3 in_Normal;
uniform mat4 in_Proj;

// We output the ex_Color variable to the next shader in the chain
out vec4 ex_Color;

void main(void) {
    gl_Position = in_Proj * vec4(in_Position.xyz, 1.0);
    ex_Color = vec4(0.5 + in_Normal / 2, 1.0);
};
