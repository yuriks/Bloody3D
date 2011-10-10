#pragma once

#include "Heatwave.hpp"

#include <cmath>

#include <xmmintrin.h>
#include <pmmintrin.h>

namespace math {

template <int N>
struct vec {
	static_assert(N >= 2 && N <= 4, "N must be between 2 and 4");

	__m128 xmm;

	HW_FORCE_INLINE explicit vec(__m128 data) : xmm(data) {}

	HW_FORCE_INLINE explicit vec(float x);

	HW_FORCE_INLINE vec(float x, float y, float z, float w) : xmm(_mm_set_ps(x, y, z,   w  )) { static_assert(N == 4); }
	HW_FORCE_INLINE vec(float x, float y, float z)          : xmm(_mm_set_ps(x, y, z,   0.f)) { static_assert(N == 3); }
	HW_FORCE_INLINE vec(float x, float y)                   : xmm(_mm_set_ps(x, y, 0.f, 0.f)) { static_assert(N == 2); }

	HW_FORCE_INLINE vec spreadX() const { return vec(_mm_shuffle_ps(xmm, xmm, _MM_SHUFFLE(0, 0, 0, 0))); }
	HW_FORCE_INLINE vec spreadY() const { return vec(_mm_shuffle_ps(xmm, xmm, _MM_SHUFFLE(1, 1, 1, 1))); }
	HW_FORCE_INLINE vec spreadZ() const { static_assert(N >= 3); return vec(_mm_shuffle_ps(xmm, xmm, _MM_SHUFFLE(2, 2, 2, 2))); }
	HW_FORCE_INLINE vec spreadW() const { static_assert(N >= 4); return vec(_mm_shuffle_ps(xmm, xmm, _MM_SHUFFLE(3, 3, 3, 3))); }

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
HW_FORCE_INLINE vec<4>::vec(float x) : xmm(_mm_set_ps1(x)) {}
template <>
HW_FORCE_INLINE vec<3>::vec(float x) : xmm(_mm_set_ps(x, x, x, 0.f)) {}
template <>
HW_FORCE_INLINE vec<2>::vec(float x) : xmm(_mm_set_ps(x, x, 0.f, 0.f)) {}

#define OP_TEMPLATE(OP, OPX) \
	template <int N> \
	HW_FORCE_INLINE vec<N>& operator OP##= (vec<N>& a, const vec<N>& b) { \
		a.xmm = _mm_##OPX##_ps(a.xmm, b.xmm); \
		return a; \
	} \
	\
	template <int N> \
	HW_FORCE_INLINE vec<N> operator OP (const vec<N>& a, const vec<N>& b) { \
		return vec<N>(_mm_##OPX##_ps(a.xmm, b.xmm)); \
	} \
	\
	template <int N> \
	HW_FORCE_INLINE vec<N> operator OP##= (vec<N>& a, float b) { \
		a.xmm = _mm_##OPX##_ps(a.xmm, EXPAND_VAL(b)); \
		return a; \
	} \
	\
	template <int N> \
	HW_FORCE_INLINE vec<N> operator OP (const vec<N>& a, float b) { \
		return vec<N>(_mm_##OPX##_ps(a.xmm, EXPAND_VAL(b))); \
	} \
	\
	template <int N> \
	HW_FORCE_INLINE vec<N> operator OP (float b, const vec<N>& a) { \
		return a OP b; \
	}

#define EXPAND_VAL(x) vec<N>(x).xmm

OP_TEMPLATE(+, add)
OP_TEMPLATE(-, sub)

#undef EXPAND_VAL
#define EXPAND_VAL(x) _mm_set_ps1(x)

OP_TEMPLATE(*, mul)
OP_TEMPLATE(/, div)

#undef EXPAND_VAL
#undef OP_TEMPLATE

typedef vec<4> vec4;
typedef vec<3> vec3;
typedef vec<2> vec2;

// Depends on unused elements being set to 0.f
template <int N>
HW_FORCE_INLINE float dot(const vec<N>& a, const vec<N>& b) {
	__m128 x = _mm_mul_ps(a.xmm, b.xmm);

	x = _mm_hadd_ps(x, x);
	x = _mm_hadd_ps(x, x);

	float tmp;
	_mm_store_ss(&tmp, x);
	return tmp;
}

// Depends on unused elements being set to 0.f
template <int N>
HW_FORCE_INLINE float length(const vec<N>& a) {
	__m128 x = _mm_mul_ps(a.xmm, a.xmm);

	x = _mm_hadd_ps(x, x);
	x = _mm_hadd_ps(x, x);

	x = _mm_sqrt_ss(x);

	float tmp;
	_mm_store_ss(&tmp, x);
	return tmp;
}

template <int N>
HW_FORCE_INLINE float invLength(const vec<N>& a) {
	__m128 x = _mm_mul_ps(a.xmm, a.xmm);

	x = _mm_hadd_ps(x, x);
	x = _mm_hadd_ps(x, x);

	x = _mm_rsqrt_ss(x);

	float tmp;
	_mm_store_ss(&tmp, x);
	return tmp;
}

template <int N>
HW_FORCE_INLINE vec<N> normalized(const vec<N>& a) {
	__m128 x = _mm_mul_ps(a.xmm, a.xmm);
	x = _mm_hadd_ps(x, x);
	x = _mm_hadd_ps(x, x);
	x = _mm_rsqrt_ps(x);

	x = _mm_mul_ps(a.xmm, x);

	float tmp;
	_mm_store_ss(&tmp, x);
}

template <int N>
HW_FORCE_INLINE vec<N> inverse(const vec<N>& a) {
	__m128 x = _mm_rcp_ps(a.xmm);
	if (N != 4) {
		// Clear unused components to 0
		x = _mm_castsi128_ps(_mm_slli_si128(_mm_srli_si128(_mm_castps_si128(x), (4-N)*4), (4-N)*4));
	}
	return vec<N>(x);
}

HW_FORCE_INLINE vec3 cross(const vec3& a, const vec3& b) {
	__m128 tmp_a = _mm_mul_ps(_mm_shuffle_ps(a.xmm, a.xmm, _MM_SHUFFLE(3, 0, 2, 1)), _mm_shuffle_ps(b.xmm, b.xmm, _MM_SHUFFLE(3, 1, 0, 2)));
	__m128 tmp_b = _mm_mul_ps(_mm_shuffle_ps(a.xmm, a.xmm, _MM_SHUFFLE(3, 1, 0, 2)), _mm_shuffle_ps(b.xmm, b.xmm, _MM_SHUFFLE(3, 0, 2, 1)));
	return vec3(_mm_sub_ps(tmp_a, tmp_b));
}

}
