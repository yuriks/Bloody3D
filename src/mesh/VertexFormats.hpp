#pragma once

#include "math/Vector.hpp"

namespace vertex_fmt {

struct Pos3f {
	math::vec3 pos;
};

struct Pos3f_Norm3f_Tex2f {
	math::vec3 pos;
	math::vec3 norm;
	float tex_coord[2];
};

enum VertexFormat {
	FMT_NONE,
	FMT_POS3F,
	FMT_POS3F_NORM3F_TEX2F
};

} // namespaec vertex_formats