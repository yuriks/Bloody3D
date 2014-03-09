#include "Quaternion.hpp"

mat3 matrixFromQuaternion(const Quaternion& q) {
	const float x = q.v[0];
	const float y = q.v[1];
	const float z = q.v[2];
	const float w = q.w;

	mat3 m = {{
		{1 - 2*y*y - 2*z*z,     2*x*y - 2*z*w,     2*x*z + 2*y*w},
		{    2*x*y + 2*z*w, 1 - 2*x*x - 2*z*z,     2*y*z - 2*x*w},
		{    2*x*z - 2*y*w,     2*y*z + 2*x*w, 1 - 2*x*x - 2*y*y}
	}};

	return m;
}

// Returns the shortest rotation between unit vectors a and b.
// TODO: Doesn't handle a == -b case.
Quaternion shortestArc(const vec3& a, const vec3& b) {
	// From http://stackoverflow.com/a/1171995
	return normalized(Quaternion(cross(a, b), 1 + dot(a, b), 0));
}
