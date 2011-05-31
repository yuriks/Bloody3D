#version 330

in  vec4 ex_Color;
out vec4 out_Color;

void main(void)
{
	// Pass through our original color with full opacity.
	out_Color = ex_Color;
};
