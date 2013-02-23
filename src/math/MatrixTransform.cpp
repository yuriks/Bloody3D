#include "MatrixTransform.hpp"

#include "misc.hpp"
#include <cmath>

namespace math {

namespace mat_transform {

HW_ALIGN_VAR_SSE const float mat_identity[] = {
	1.f, 0.f, 0.f, 0.f,
	0.f, 1.f, 0.f, 0.f,
	0.f, 0.f, 1.f, 0.f,
	0.f, 0.f, 0.f, 1.f
};

mat4 rotate(const vec3& axis, float angle)
{
	mat4 m;

	// Taken from Graphics Gems I (Section IX.6)

	const float s = std::sin(angle);
	const float c = std::cos(angle);
	const float t = 1.f - c;
	const float x = axis.getX();
	const float y = axis.getY();
	const float z = axis.getZ();

	// A lot of common sub-expression optimization can be
	// done here, but I'll leave that to the compiler.
	m.rows[0] = vec4(t*x*x + c,   t*x*y - s*z, t*x*z + s*y, 0.f);
	m.rows[1] = vec4(t*y*x + s*z, t*y*y + c,   t*y*z - s*x, 0.f);
	m.rows[2] = vec4(t*z*x - s*y, t*z*y + s*x, t*z*z + c,   0.f);
	m.rows[3] = vec4(0.f,         0.f,         0.f,         1.f);

	return m;
}

mat4 orthographic_proj(float left, float right, float bottom, float top, float z_near, float z_far)
{
	vec3 t = vec3(left + right, bottom + top, z_near + z_far) / -2.f;
	vec3 s = inverse(vec3(right - left, top - bottom, z_far - z_near)) * 2.f;
	return scale(s) * translate(t);
}

mat4 frustrum_proj(float half_width, float half_height, float z_near, float z_far)
{
	mat4 m;

	// x' * z = (F / W)*x;
	// y' * z = (F / H)*y;
	// z' * z = A*z + B*1;
	// A = (F + N)/(F - N);
	// B = (2 * F * N)/(N - F);

	const float z_over_w = z_far / half_width;
	const float z_over_h = z_far / half_height;
	const float a = (z_far + z_near) / (z_far - z_near);
	const float b = (2.f * z_far * z_near)/(z_near - z_far);

	m.rows[0] = vec4(z_over_w,   0.f,    0.f, 0.f);
	m.rows[1] = vec4(  0.f,    z_over_h, 0.f, 0.f);
	m.rows[2] = vec4(  0.f,      0.f,     a,   b );
	m.rows[3] = vec4(  0.f,      0.f,    1.f, 0.f);

	return m;
}

mat4 perspective_proj(float vfov, float aspect, float z_near, float z_far)
{
	float height = z_far * std::tanf(vfov * (math::pi / 180.f));

	return frustrum_proj(height * aspect, height, z_near, z_far);
}

mat4 look_at(const vec3& up, const vec3& camera, const vec3& target)
{
	vec3 z_axis = normalized(target - camera);
	vec3 x_axis = cross(up, z_axis);
	vec3 y_axis = cross(z_axis, x_axis);

	mat4 m;

	m.rows[0] = vec4(x_axis);
	m.rows[1] = vec4(y_axis);
	m.rows[2] = vec4(z_axis);
	m.rows[3] = vec4(0.f, 0.f, 0.f, 1.f);

	return m * mat_transform::translate(camera * -1.f);
}

} // namespace mat_transform

} // namespace math
