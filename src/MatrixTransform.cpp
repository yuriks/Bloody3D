#include "MatrixTransform.hpp"

#include <cmath>

namespace math {

namespace mat_transform {

mat4 rotate(const vec3& axis, float angle)
{
	mat4 m = identity<4>();

	// Taken from Graphics Gems I (Section IX.6)

	const float s = std::sin(angle);
	const float c = std::cos(angle);
	const float t = 1 - c;
	const float x = axis[0];
	const float y = axis[1];
	const float z = axis[2];

	// A lot of common sub-expression optimization can be
	// done here, but I'll leave that to the compiler.
	m(0, 0) = t*x*x + c;
	m(0, 1) = t*x*y + s*z;
	m(0, 2) = t*x*z - s*y;

	m(1, 0) = t*y*x - s*z;
	m(1, 1) = t*y*y + c;
	m(1, 2) = t*y*z + s*x;

	m(2, 0) = t*z*x + s*y;
	m(2, 1) = t*z*y - s*x;
	m(2, 2) = t*z*z + c;

	return m;
}

} // namespace mat_transform

} // namespace math
