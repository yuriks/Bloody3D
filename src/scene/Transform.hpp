#pragma once

#include "math/vec.hpp"
#include "math/Quaternion.hpp"
#include "math/mat.hpp"
#include "util/ObjectPool.hpp"
#include "util/StringHash.hpp"

struct Transform {
	vec3 pos;
	float scale;
	Quaternion rot;

	Handle parent;

	Transform() : pos(vec3_0), scale(1.0f), rot() { }

	template <typename T>
	void reflect(T& f) {
		f(pos,    HASHSTR("pos"));
		f(scale,  HASHSTR("scale"));
		f(rot,    HASHSTR("rot"));
		f(parent, HASHSTR("parent"));
	}
};

mat4 calcTransformMtx(const Transform& t);
mat4 calcInvTransformMtx(const Transform& t);

void calculateModel2WorldMatrices(
	const ObjectPool<Transform>& transforms,
	mat4* out, mat4* out_inverse);
