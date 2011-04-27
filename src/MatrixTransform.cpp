#include "MatrixTransform.hpp"

#include <cmath>

namespace math {

namespace mat_transform {

#ifndef USE_SSE2

mat4 rotate(const vec3& axis, float angle)
{
	mat4 m = identity<4>();

	// Taken from Graphics Gems I (Section IX.6)

	const float s = std::sin(angle);
	const float c = std::cos(angle);
	const float t = 1.f - c;
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

#else

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

} // namespace mat_transform

} // namespace math
