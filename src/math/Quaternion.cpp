#include "Quaternion.hpp"

namespace math {

mat4 matrixFromQuaternion(const Quaternion& q) {
	const float x = q.val.getX();
	const float y = q.val.getY();
	const float z = q.val.getZ();
	const float w = q.val.getW();

	mat4 m;
	m.rows[0] = vec4(1.f - 2.f*y*y - 2.f*z*z,       2.f*x*y - 2.f*z*w,       2.f*x*z + 2.f*y*w, 0.f);
	m.rows[1] = vec4(      2.f*x*y + 2.f*z*w, 1.f - 2.f*x*x - 2.f*z*z,       2.f*y*z - 2.f*x*w, 0.f);
	m.rows[2] = vec4(      2.f*x*z - 2.f*y*w,       2.f*y*z + 2.f*x*w, 1.f - 2.f*x*x - 2.f*y*y, 0.f);
	m.rows[3] = vec4(                    0.f,                     0.f,                     0.f, 1.f);

	return m;
}

} // namespace math
