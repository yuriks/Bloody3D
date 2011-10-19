#pragma once

#include "Vector.hpp"

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

Quaternion operator*(const Quaternion& p, const Quaternion& q) {
	const vec3 pv = vec3(p.val);
	const vec3 qv = vec3(q.val);
	vec4 val = vec4((vec3(p.val.spreadW().xmm) * qv) + (vec3(q.val.spreadW().xmm) * pv) + cross(pv, qv));
	val.setW(p.val.getW() * q.val.getW() - dot(pv, qv));
	return Quaternion(val);
}

Quaternion conjugate(const Quaternion& q) {
	return Quaternion(vec4(_mm_xor_ps(q.val.xmm, _mm_setr_ps(-0.f, -0.f, -0.f, 0.f))));
}

vec3 rotate(const Quaternion& q, const vec3& v) {
	return vec3((q * Quaternion(vec4(v)) * conjugate(q)).val);
}

mat3x4 matrixFromQuaternion(const Quaternion& q) {
	const float x = q.val.getX();
	const float y = q.val.getY();
	const float z = q.val.getZ();
	const float w = q.val.getW();

	mat3x4 m;
	m.rows[0] = vec4(1.f - 2.f*y*y - 2.f*z*z,       2.f*x*y - 2.f*z*w,       2.f*x*z + 2.f*y*w, 0.f);
	m.rows[1] = vec4(      2.f*x*y + 2.f*z*w, 1.f - 2.f*x*x - 2.f*z*z,       2.f*y*z - 2.f*x*w, 0.f);
	m.rows[2] = vec4(      2.f*x*z - 2.f*y*w,       2.f*y*z + 2.f*x*w, 1.f - 2.f*x*x - 2.f*y*y, 0.f);

	return m;
}

} // namespace math
