#pragma once

#include "math/vec.hpp"
#include "math/Quaternion.hpp"
#include "math/mat.hpp"
#include "util/ObjectPool.hpp"

namespace scene {

struct Transform {
	math::vec3 pos;
	float scale;
	math::Quaternion rot;

	Handle parent;

	Transform() : pos(math::vec3_0), scale(1.0f), rot() { }
};

math::mat4 calcTransformMtx(const Transform& t);
math::mat4 calcInvTransformMtx(const Transform& t);

void calculateModel2WorldMatrices(
	const ObjectPool<Transform>& transforms,
	math::mat4* out, math::mat4* out_inverse);

} // namespace scene
