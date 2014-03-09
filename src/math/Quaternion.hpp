#pragma once

#include "vec.hpp"
#include "mat.hpp"

#include <cmath>

struct Quaternion {
	vec3 v;
	float w;

	Quaternion() : v(vec3_0), w(1) {}

	Quaternion(const vec3& v, float w, int) : v(v), w(w) {}

	Quaternion(const vec3& axis, float angle)
		: v(axis * std::sin(angle/2.f)),
		w(std::cos(angle/2.f))
	{}
};

inline Quaternion operator*(const Quaternion& q, const Quaternion& r) {
	// From Real-Time Rendering, 3rd ed., pg. 73
	return Quaternion(
		cross(q.v, r.v) + r.w*q.v + q.w*r.v,
		q.w*r.w - dot(q.v, r.v), 0);
}

inline Quaternion operator*(const Quaternion& q, float s) {
	return Quaternion(q.v * s, q.w * s, 0);
}

inline Quaternion operator*(float s, const Quaternion& q) {
	return q * s;
}

inline Quaternion conjugate(const Quaternion& q) {
	return Quaternion(-q.v, q.w);
}

inline vec3 rotate(const Quaternion& q, const vec3& v) {
	return (q * Quaternion(v, 0.f, 0) * conjugate(q)).v;
}

inline float norm(const Quaternion& q) {
	return std::sqrt(dot(q.v, q.v) + q.w*q.w);
}

inline Quaternion normalized(const Quaternion& q) {
	return q * (1.0f / norm(q));
}

mat3 matrixFromQuaternion(const Quaternion& q);
Quaternion shortestArc(const vec3& a, const vec3& b);
