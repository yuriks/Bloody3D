#ifndef BLOODY3D_SSE2_MATRIX_HPP
#define BLOODY3D_SSE2_MATRIX_HPP

#include <xmmintrin.h>

namespace math {

template <>
struct mat<4, 4>
{
	inline void operator+=(const mat& m)
	{
		xmm_data[0] = _mm_add_ps(xmm_data[0], m.xmm_data[0]);
		xmm_data[1] = _mm_add_ps(xmm_data[1], m.xmm_data[1]);
		xmm_data[2] = _mm_add_ps(xmm_data[2], m.xmm_data[2]);
		xmm_data[3] = _mm_add_ps(xmm_data[3], m.xmm_data[3]);
	}

	inline void operator-=(const mat& m)
	{
		xmm_data[0] = _mm_sub_ps(xmm_data[0], m.xmm_data[0]);
		xmm_data[1] = _mm_sub_ps(xmm_data[1], m.xmm_data[1]);
		xmm_data[2] = _mm_sub_ps(xmm_data[2], m.xmm_data[2]);
		xmm_data[3] = _mm_sub_ps(xmm_data[3], m.xmm_data[3]);
	}

	inline void operator*=(float f)
	{
		__m128 src = _mm_set1_ps(f);

		xmm_data[0] = _mm_mul_ps(xmm_data[0], src);
		xmm_data[1] = _mm_mul_ps(xmm_data[1], src);
		xmm_data[2] = _mm_mul_ps(xmm_data[2], src);
		xmm_data[3] = _mm_mul_ps(xmm_data[3], src);
	}

	inline void operator/=(float f)
	{
		__m128 src = _mm_set1_ps(f);

		xmm_data[0] = _mm_div_ps(xmm_data[0], src);
		xmm_data[1] = _mm_div_ps(xmm_data[1], src);
		xmm_data[2] = _mm_div_ps(xmm_data[2], src);
		xmm_data[3] = _mm_div_ps(xmm_data[3], src);
	}

	inline void clear(float f)
	{
		__m128 src = _mm_set1_ps(f);

		xmm_data[0] = src;
		xmm_data[1] = src;
		xmm_data[2] = src;
		xmm_data[3] = src;
	}

	inline float& operator()(unsigned int r, unsigned int c)
	{
		return data[r + 4*c];
	}

	inline const float& operator()(unsigned int r, unsigned int c) const
	{
		return data[r + 4*c];
	}

	union
	{
		float data[4*4];
		__m128 xmm_data[4];
	};

	static const bool ROW_MAJOR = false;
};

#define N 4
#include "vec_def.hpp"
#undef N
#define N 3
#include "vec_def.hpp"
#undef N

inline mat4 operator*(const mat4& m1, const mat4& m2)
{
	mat4 mr;

	__m128 a0, a1, a2, a3;
	__m128 b, r;

	a0 = _mm_load_ps(m1.data);
	a1 = _mm_load_ps(m1.data+4);
	a2 = _mm_load_ps(m1.data+8);
	a3 = _mm_load_ps(m1.data+12);

#define do_column(i) \
	b = _mm_set1_ps(m2(0, i)); \
	r = _mm_mul_ps(a0, b); \
	\
	b = _mm_set1_ps(m2(1, i)); \
	r = _mm_add_ps(r, _mm_mul_ps(a1, b)); \
	\
	b = _mm_set1_ps(m2(2, i)); \
	r = _mm_add_ps(r, _mm_mul_ps(a2, b)); \
	\
	b = _mm_set1_ps(m2(3, i)); \
	r = _mm_add_ps(r, _mm_mul_ps(a3, b)); \
	\
	_mm_store_ps(mr.data+4*i, r)

	do_column(0);
	do_column(1);
	do_column(2);
	do_column(3);
#undef do_column

	return mr;
}

} // namespace math

#endif // BLOODY3D_SSE2_MATRIX_HPP
