#pragma once

#include "math/vec.hpp"

namespace vertex_fmt {

struct Pos3f {
	vec3 pos;
};

struct Pos3f_Norm3f_Tex2f {
	vec3 pos;
	vec3 norm;
	float tex_coord[2];
};

// DON'T CHANGE VALUES
enum VertexFormat {
	FMT_NONE = 0,
	FMT_POS3F = 1,
	FMT_POS3F_NORM3F_TEX2F = 2
};

} // namespaec vertex_formats
