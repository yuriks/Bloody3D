#ifndef BLOODY3D_SSE2_VECTOR_HPP
#define BLOODY3D_SSE2_VECTOR_HPP

#include <xmmintrin.h>
#ifdef USE_SSE3
#include <pmmintrin.h>
#endif

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

namespace vec {

inline vec3 cross(const vec3& a, const vec3& b)
{
	__m128 va = a.xmm_data;
	__m128 vb = b.xmm_data;

	va = _mm_shuffle_ps(va, va, _MM_SHUFFLE(3, 0, 2, 1));
	vb = _mm_shuffle_ps(vb, vb, _MM_SHUFFLE(3, 1, 0, 2));

	__m128 vtmp = va;
	vtmp = _mm_mul_ps(vtmp, vb);

	va = _mm_shuffle_ps(va, va, _MM_SHUFFLE(3, 0, 2, 1));
	vb = _mm_shuffle_ps(vb, vb, _MM_SHUFFLE(3, 1, 0, 2));

	va = _mm_mul_ps(va, vb);

	vtmp = _mm_sub_ps(vtmp, va);

	vec3 r;
	r.xmm_data = vtmp;
	return r;
}

inline vec3 euclidean(const vec4& v)
{
	__m128 vv = v.xmm_data;
	__m128 w;

	w = _mm_shuffle_ps(vv, vv, _MM_SHUFFLE(3, 3, 3, 3));
	vv = _mm_div_ps(vv, w);

	vec3 r;
	r.xmm_data = vv;
	return r;
}

/*
template <unsigned int N>
inline mat<N+1, 1> homogeneous(const mat<N, 1>& v)
{
	mat<N+1, 1> r;

	for (unsigned int i = 0; i < N; ++i) {
		r[i] = v[i];
	}
	r[N] = 1.f;

	return r;
}
*/

} // namespace vec

#define N 4
#define vecN vec4
#include "vec_ops.hpp"
#undef N
#undef vecN
#define N 3
#define vecN vec3
#include "vec_ops.hpp"
#undef N
#undef vecN

} // namespace math

#endif // BLOODY3D_SSE2_VECTOR_HPP
