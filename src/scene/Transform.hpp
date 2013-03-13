#pragma once

#include "math/vec.hpp"
#include "math/Quaternion.hpp"
#include "math/mat.hpp"

namespace scene {

struct Transform {
	math::vec3 pos;
	float scale;
	math::Quaternion rot;

	Transform() : pos(math::vec3_0), scale(1.0f), rot() { }
};

math::mat4 calcTransformMtx(const Transform& t);
math::mat4 calcInvTransformMtx(const Transform& t);

} // namespace scene
