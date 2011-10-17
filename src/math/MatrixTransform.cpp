#include "MatrixTransform.hpp"

#include <cmath>

namespace math {

namespace mat_transform {

HW_ALIGN_VAR_SSE const float mat_identity[] = {
	1.f, 0.f, 0.f, 0.f,
	0.f, 1.f, 0.f, 0.f,
	0.f, 0.f, 1.f, 0.f,
	0.f, 0.f, 0.f, 1.f
};

mat3x4 rotate(const vec3& axis, float angle)
{
	mat3x4 m;

	// Taken from Graphics Gems I (Section IX.6)

	const float s = std::sin(angle);
	const float c = std::cos(angle);
	const float t = 1.f - c;
	const float x = axis.getX();
	const float y = axis.getY();
	const float z = axis.getZ();

	// A lot of common sub-expression optimization can be
	// done here, but I'll leave that to the compiler.
	m.rows[0] = vec4(t*x*x + c,   t*x*y + s*z, t*x*z - s*y, 0.f);
	m.rows[1] = vec4(t*y*x - s*z, t*y*y + c,   t*y*z + s*x, 0.f);
	m.rows[2] = vec4(t*z*x + s*y, t*z*y - s*x, t*z*z + c,   0.f);

	return m;
}

#if 0

mat4 rotate(const vec3& axis, float angle)
{
	// Taken from Graphics Gems I (Section IX.6)

	const __m128 xyz = axis.xmm_data;

	const __m128 all_x = _mm_shuffle_ps(xyz, xyz, _MM_SHUFFLE(3, 0, 0, 0));
	const __m128 all_y = _mm_shuffle_ps(xyz, xyz, _MM_SHUFFLE(3, 1, 1, 1));
	const __m128 all_z = _mm_shuffle_ps(xyz, xyz, _MM_SHUFFLE(3, 2, 2, 2));

	const __m128 sin = _mm_set_ss(std::sinf(angle));
	const __m128 cos = _mm_set_ss(std::cosf(angle));
	const __m128 t = _mm_sub_ss(_mm_set_ss(1.f), cos);

	const __m128 all_s = _mm_shuffle_ps(sin, sin, _MM_SHUFFLE(0, 0, 0, 0));
	const __m128 all_t = _mm_shuffle_ps(t, t, _MM_SHUFFLE(0, 0, 0, 0));

	const __m128 t_xyz = _mm_mul_ps(all_t, xyz);
	const __m128 s_xyz = _mm_mul_ps(all_s, xyz);

	mat4 m;

	__m128 neg_mask = _mm_set_ps(0.f, 1.f, -1.f, 1.f);
	__m128 a = _mm_mul_ps(t_xyz, all_x);
	__m128 tmp = _mm_shuffle_ps(s_xyz, cos, _MM_SHUFFLE(3, 0, 1, 2));
	tmp = _mm_shuffle_ps(tmp, tmp, _MM_SHUFFLE(3, 1, 0, 2));
	m.xmm_data[0] = _mm_add_ps(a, _mm_mul_ps(tmp, neg_mask));
	//m(0, 0) = tx*x + cos;
	//m(1, 0) = ty*x - sz;
	//m(2, 0) = tz*x + sy;

	neg_mask = _mm_shuffle_ps(neg_mask, neg_mask, _MM_SHUFFLE(3, 1, 0, 0));
	a = _mm_mul_ps(t_xyz, all_y);
	tmp = _mm_shuffle_ps(s_xyz, cos, _MM_SHUFFLE(3, 0, 0, 2));
	tmp = _mm_shuffle_ps(tmp, tmp, _MM_SHUFFLE(3, 1, 2, 0));
	m.xmm_data[1] = _mm_add_ps(a, _mm_mul_ps(tmp, neg_mask));
	//m(0, 1) = tx*y + sz;
	//m(1, 1) = ty*y + c;
	//m(2, 1) = tz*y - sx;

	neg_mask = _mm_shuffle_ps(neg_mask, neg_mask, _MM_SHUFFLE(3, 0, 0, 2));
	a = _mm_mul_ps(t_xyz, all_z);
	tmp = _mm_shuffle_ps(s_xyz, cos, _MM_SHUFFLE(3, 0, 0, 1));
	m.xmm_data[2] = _mm_add_ps(a, _mm_mul_ps(tmp, neg_mask));
	//m(0, 2) = tx*z - sy;
	//m(1, 2) = ty*z + sx;
	//m(2, 2) = tz*z + c;

	neg_mask = _mm_shuffle_ps(neg_mask, neg_mask, _MM_SHUFFLE(1, 3, 3, 3));
	m.xmm_data[3] = neg_mask;

	return m;
}

#endif

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
	float height = z_far * std::tanf(vfov);

	return frustrum_proj(height * aspect, height, z_near, z_far);
}

mat3x4 look_at(const vec3& up, const vec3& camera, const vec3& target)
{
	vec3 z_axis = normalized(target - camera);
	vec3 x_axis = cross(up, z_axis);
	vec3 y_axis = cross(z_axis, x_axis);

	mat3x4 m;

	m.rows[0] = vec4(x_axis);
	m.rows[1] = vec4(y_axis);
	m.rows[2] = vec4(z_axis);

	return concatTransform(mat3x4(vec4(x_axis), vec4(y_axis), vec4(z_axis)), mat_transform::translate3x4(camera * -1.f));
}

} // namespace mat_transform

} // namespace math
