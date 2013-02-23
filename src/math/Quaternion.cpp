#include "Quaternion.hpp"

namespace math {

mat4 matrixFromQuaternion(const Quaternion& q) {
	const float x = q.v[0];
	const float y = q.v[1];
	const float z = q.v[2];
	const float w = q.w;

	mat4 m = {{
		{1 - 2*y*y - 2*z*z,     2*x*y - 2*z*w,     2*x*z + 2*y*w, 0},
		{    2*x*y + 2*z*w, 1 - 2*x*x - 2*z*z,     2*y*z - 2*x*w, 0},
		{    2*x*z - 2*y*w,     2*y*z + 2*x*w, 1 - 2*x*x - 2*y*y, 0},
		{                0,                 0,                 0, 1}
	}};

	return m;
}

} // namespace math
