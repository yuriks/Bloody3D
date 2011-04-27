#ifndef BLOODY3D_SSE2_VECTOR_HPP
#define BLOODY3D_SSE2_VECTOR_HPP

#include <xmmintrin.h>
#include <iostream>

namespace math {

inline vec4 operator*(const mat4& m, const vec4& v)
{
	__m128 src_vec, src_val;
	__m128 dst, tmp;

	src_vec = v.xmm_data;

	src_val = _mm_shuffle_ps(src_vec, src_vec, _MM_SHUFFLE(0,0,0,0));
	dst = _mm_mul_ps(m.xmm_data[0], src_val);

	src_val = _mm_shuffle_ps(src_vec, src_vec, _MM_SHUFFLE(1,1,1,1));
	tmp = _mm_mul_ps(m.xmm_data[1], src_val);
	dst = _mm_add_ps(dst, tmp);

	src_val = _mm_shuffle_ps(src_vec, src_vec, _MM_SHUFFLE(2,2,2,2));
	tmp = _mm_mul_ps(m.xmm_data[2], src_val);
	dst = _mm_add_ps(dst, tmp);

	src_val = _mm_shuffle_ps(src_vec, src_vec, _MM_SHUFFLE(3,3,3,3));
	tmp = _mm_mul_ps(m.xmm_data[3], src_val);
	dst = _mm_add_ps(dst, tmp);

	vec4 vr;
	vr.xmm_data = dst;

	return vr;
}

} // namespace math

#endif // BLOODY3D_SSE2_VECTOR_HPP
