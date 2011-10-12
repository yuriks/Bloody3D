#pragma once

#include "Heatwave.hpp"

#include <cmath>

#include <xmmintrin.h>
#include <pmmintrin.h>

namespace math {

template <unsigned int N>
struct Vec {
	static_assert(N >= 2 && N <= 4, "N must be between 2 and 4");

	__m128 xmm;

	HW_FORCE_INLINE Vec() {}
	HW_FORCE_INLINE explicit Vec(__m128 data) : xmm(data) {}

	HW_FORCE_INLINE explicit Vec(float x);

	HW_FORCE_INLINE Vec(float x, float y, float z, float w) : xmm(_mm_set_ps(x, y, z,   w  )) { static_assert(N == 4); }
	HW_FORCE_INLINE Vec(float x, float y, float z)          : xmm(_mm_set_ps(x, y, z,   0.f)) { static_assert(N == 3); }
	HW_FORCE_INLINE Vec(float x, float y)                   : xmm(_mm_set_ps(x, y, 0.f, 0.f)) { static_assert(N == 2); }

	HW_FORCE_INLINE Vec spreadX() const { return Vec(_mm_shuffle_ps(xmm, xmm, _MM_SHUFFLE(0, 0, 0, 0))); }
	HW_FORCE_INLINE Vec spreadY() const { return Vec(_mm_shuffle_ps(xmm, xmm, _MM_SHUFFLE(1, 1, 1, 1))); }
	HW_FORCE_INLINE Vec spreadZ() const { static_assert(N >= 3); return Vec(_mm_shuffle_ps(xmm, xmm, _MM_SHUFFLE(2, 2, 2, 2))); }
	HW_FORCE_INLINE Vec spreadW() const { static_assert(N >= 4); return Vec(_mm_shuffle_ps(xmm, xmm, _MM_SHUFFLE(3, 3, 3, 3))); }

	HW_FORCE_INLINE float getX() const { float tmp; _mm_store_ss(&tmp, xmm); return tmp; }
	HW_FORCE_INLINE float getY() const { return spreadY().getX(); }
	HW_FORCE_INLINE float getZ() const { static_assert(N >= 3); return spreadZ().getX(); }
	HW_FORCE_INLINE float getW() const { static_assert(N >= 4); return spreadW().getX(); }

private:
	HW_FORCE_INLINE void setVal(int n, float val) {
		union {
			__m128 v;
			float f[4];
		} u;
		u.v = xmm;
		u.f[n] = val;
		xmm = u.v;
	}

public:
	HW_FORCE_INLINE float setX(float val) { setVal(0, val); }
	HW_FORCE_INLINE float setY(float val) { setVal(1, val); }
	HW_FORCE_INLINE float setZ(float val) { static_assert(N >= 3); setVal(2, val); }
	HW_FORCE_INLINE float setW(float val) { static_assert(N >= 4); setVal(3, val); }
};

template <>
HW_FORCE_INLINE Vec<4>::Vec(float x) : xmm(_mm_set_ps1(x)) {}
template <>
HW_FORCE_INLINE Vec<3>::Vec(float x) : xmm(_mm_set_ps(x, x, x, 0.f)) {}
template <>
HW_FORCE_INLINE Vec<2>::Vec(float x) : xmm(_mm_set_ps(x, x, 0.f, 0.f)) {}

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

	float tmp;
	_mm_store_ss(&tmp, x);
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
