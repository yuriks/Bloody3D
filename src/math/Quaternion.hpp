#pragma once

#include "Vector.hpp"
#include "Matrix.hpp"

#include <cmath>

namespace math {

struct Quaternion {
	vec4 val;

	Quaternion() : val(0.f, 0.f, 0.f, 1.f) {}

	explicit Quaternion(const vec4& v) : val(v) {}

	Quaternion(const vec3& axis, float angle) {
		vec4 a = vec4(axis * std::sin(angle/2.f));
		a.setW(std::cos(angle/2.f));
		val = a;
	}
};

inline Quaternion operator*(const Quaternion& p, const Quaternion& q) {
	const vec3 pv = vec3(p.val);
	const vec3 qv = vec3(q.val);
	vec4 val = vec4((vec3(p.val.spreadW().xmm) * qv) + (vec3(q.val.spreadW().xmm) * pv) + cross(pv, qv));
	val.setW(p.val.getW() * q.val.getW() - dot(pv, qv));
	return Quaternion(val);
}

inline Quaternion conjugate(const Quaternion& q) {
	return Quaternion(vec4(_mm_xor_ps(q.val.xmm, _mm_setr_ps(-0.f, -0.f, -0.f, 0.f))));
}

inline vec3 rotate(const Quaternion& q, const vec3& v) {
	return vec3((q * Quaternion(vec4(v)) * conjugate(q)).val);
}

mat4 matrixFromQuaternion(const Quaternion& q);

} // namespace math
