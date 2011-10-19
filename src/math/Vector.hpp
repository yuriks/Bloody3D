#pragma once

#include "Heatwave.hpp"

#include <cmath>

#include <xmmintrin.h>
#include <pmmintrin.h>

namespace math {

template <unsigned int N>
struct Vec;

#define N 4
#include "vector_template.hpp"
#undef N
#define N 3
#include "vector_template.hpp"
#undef N
#define N 2
#include "vector_template.hpp"
#undef N

#define OP_TEMPLATE(OP, OPX) \
	template <unsigned int N> \
	HW_FORCE_INLINE Vec<N>& operator OP##= (Vec<N>& a, const Vec<N>& b) { \
		a.xmm = _mm_##OPX##_ps(a.xmm, b.xmm); \
		return a; \
	} \
	\
	template <unsigned int N> \
	HW_FORCE_INLINE Vec<N> operator OP (const Vec<N>& a, const Vec<N>& b) { \
		return Vec<N>(_mm_##OPX##_ps(a.xmm, b.xmm)); \
	} \
	\
	template <unsigned int N> \
	HW_FORCE_INLINE Vec<N> operator OP##= (Vec<N>& a, float b) { \
		a.xmm = _mm_##OPX##_ps(a.xmm, EXPAND_VAL(b)); \
		return a; \
	} \
	\
	template <unsigned int N> \
	HW_FORCE_INLINE Vec<N> operator OP (const Vec<N>& a, float b) { \
		return Vec<N>(_mm_##OPX##_ps(a.xmm, EXPAND_VAL(b))); \
	} \
	\
	template <unsigned int N> \
	HW_FORCE_INLINE Vec<N> operator OP (float b, const Vec<N>& a) { \
		return a OP b; \
	}

#define EXPAND_VAL(x) Vec<N>(x).xmm

OP_TEMPLATE(+, add)
OP_TEMPLATE(-, sub)

#undef EXPAND_VAL
#define EXPAND_VAL(x) _mm_set_ps1(x)

OP_TEMPLATE(*, mul)
OP_TEMPLATE(/, div)

#undef EXPAND_VAL
#undef OP_TEMPLATE

typedef Vec<4> vec4;
typedef Vec<3> vec3;
typedef Vec<2> vec2;

static const vec3 right(1.f, 0.f, 0.f);
static const vec3 up(0.f, 1.f, 0.f);
static const vec3 forward(0.f, 0.f, 1.f);

// Depends on unused elements being set to 0.f
template <unsigned int N>
HW_FORCE_INLINE float dot(const Vec<N>& a, const Vec<N>& b) {
	__m128 x = _mm_mul_ps(a.xmm, b.xmm);

	x = _mm_hadd_ps(x, x);
	x = _mm_hadd_ps(x, x);

	float tmp;
	_mm_store_ss(&tmp, x);
	return tmp;
}

// Depends on unused elements being set to 0.f
template <unsigned int N>
HW_FORCE_INLINE Vec<N> spreadDot(const Vec<N>& a, const Vec<N>& b) {
	__m128 x = _mm_mul_ps(a.xmm, b.xmm);

	x = _mm_hadd_ps(x, x);
	x = _mm_hadd_ps(x, x);

	return Vec<N>(x);
}

// Depends on unused elements being set to 0.f
template <unsigned int N>
HW_FORCE_INLINE float length(const Vec<N>& a) {
	__m128 x = _mm_mul_ps(a.xmm, a.xmm);

	x = _mm_hadd_ps(x, x);
	x = _mm_hadd_ps(x, x);

	x = _mm_sqrt_ss(x);

	float tmp;
	_mm_store_ss(&tmp, x);
	return tmp;
}

template <unsigned int N>
HW_FORCE_INLINE float invLength(const Vec<N>& a) {
	__m128 x = _mm_mul_ps(a.xmm, a.xmm);

	x = _mm_hadd_ps(x, x);
	x = _mm_hadd_ps(x, x);

	x = _mm_rsqrt_ss(x);

	float tmp;
	_mm_store_ss(&tmp, x);
	return tmp;
}

template <unsigned int N>
HW_FORCE_INLINE Vec<N> normalized(const Vec<N>& a) {
	__m128 x = _mm_mul_ps(a.xmm, a.xmm);
	x = _mm_hadd_ps(x, x);
	x = _mm_hadd_ps(x, x);
	x = _mm_rsqrt_ps(x);

	x = _mm_mul_ps(a.xmm, x);

	return Vec<N>(x);
}

template <unsigned int N>
HW_FORCE_INLINE Vec<N> inverse(const Vec<N>& a) {
	__m128 x = _mm_rcp_ps(a.xmm);
	if (N != 4) {
		// Clear unused components to 0
		x = _mm_castsi128_ps(_mm_slli_si128(_mm_srli_si128(_mm_castps_si128(x), (4-N)*4), (4-N)*4));
	}
	return Vec<N>(x);
}

HW_FORCE_INLINE vec3 cross(const vec3& a, const vec3& b) {
	__m128 tmp_a = _mm_mul_ps(_mm_shuffle_ps(a.xmm, a.xmm, _MM_SHUFFLE(3, 0, 2, 1)), _mm_shuffle_ps(b.xmm, b.xmm, _MM_SHUFFLE(3, 1, 0, 2)));
	__m128 tmp_b = _mm_mul_ps(_mm_shuffle_ps(a.xmm, a.xmm, _MM_SHUFFLE(3, 1, 0, 2)), _mm_shuffle_ps(b.xmm, b.xmm, _MM_SHUFFLE(3, 0, 2, 1)));
	return vec3(_mm_sub_ps(tmp_a, tmp_b));
}

}
