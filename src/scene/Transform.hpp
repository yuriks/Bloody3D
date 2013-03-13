#pragma once

#include "math/vec.hpp"
#include "math/Quaternion.hpp"

namespace scene {

struct Transform {
	math::vec3 pos;
	float scale;
	math::Quaternion rot;

	Transform() : pos(math::vec3_0), scale(1.0f), rot() { }
};

} // namespace scene
