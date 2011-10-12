#pragma once

#include "Vector.hpp"

namespace math {

// R = rows, C = columns
// row-major storage
template <unsigned int R, unsigned int C>
struct Mat {
	typedef Vec<C> vec;
	vec rows[R];

	HW_FORCE_INLINE Mat() {}

	HW_FORCE_INLINE explicit Mat(float x) {
		for (unsigned int i = 0; i < R; ++i)
			rows[i] = vec(x);
	}

	HW_FORCE_INLINE explicit Mat(const vec& a, const vec& b, const vec& c, const vec& d) {
		static_assert(C == 4);
		rows[0] = a;
		rows[1] = b;
		rows[2] = c;
		rows[3] = d;
	}

	HW_FORCE_INLINE explicit Mat(const vec& a, const vec& b, const vec& c) {
		static_assert(C == 3);
		rows[0] = a;
		rows[1] = b;
		rows[2] = c;
	}

	HW_FORCE_INLINE explicit Mat(const vec& a, const vec& b) {
		static_assert(C == 2);
		rows[0] = a;
		rows[1] = b;
	}
};

#define OP_TEMPLATE(OP) \
	template <unsigned int R, unsigned int C> \
	HW_FORCE_INLINE Mat<R, C>& operator OP##= (Mat<R, C>& a, const Mat<R, C>& b) { \
		for (unsigned int i = 0; i < R; ++i) \
			a.rows[i] OP##= b.rows[i];	 \
		return a; \
	} \
	\
	template <unsigned int R, unsigned int C> \
	HW_FORCE_INLINE Mat<R, C> operator OP (const Mat<R, C>& a, const Mat<R, C>& b) { \
		Mat<R, C> m; \
		for (unsigned int i = 0; i < R; ++i) \
			m.rows[i] = a.rows[i] OP b.rows[i];	 \
		return m; \
	} \
	\
	template <unsigned int R, unsigned int C> \
	HW_FORCE_INLINE Mat<R, C>& operator OP##= (Mat<R, C>& a, float b) { \
		const Vec<C> v(b); \
		for (unsigned int i = 0; i < R; ++i) \
			a.rows[i] OP##= v; \
		return a; \
	} \
	\
	template <unsigned int R, unsigned int C> \
	HW_FORCE_INLINE Mat<R, C> operator OP (const Mat<R, C>& a, float b) { \
		const Vec<C> v(b); \
		Mat<R, C> m; \
		for (unsigned int i = 0; i < R; ++i) \
			m.rows[i] = a.rows[i] OP v; \
		return m; \
	} \
	\
	template <unsigned int R, unsigned int C> \
	HW_FORCE_INLINE Mat<R, C> operator OP (float b, const Mat<R, C>& a) { \
		return a OP b; \
	} \

OP_TEMPLATE(+)
OP_TEMPLATE(-)
OP_TEMPLATE(/)
#undef OP_TEMPLATE

template <unsigned int R, unsigned int C>
HW_FORCE_INLINE Mat<R, C>& elementMultInPlace(Mat<R, C>& a, const Mat<R, C>& b) {
	for (unsigned int i = 0; i < R; ++i)
		a.rows[i] *= b.rows[i];
	return a;
}

template <unsigned int R, unsigned int C>
HW_FORCE_INLINE Mat<R, C> elementMult(const Mat<R, C>& a, const Mat<R, C>& b) {
	Mat<R, C> m;
	for (unsigned int i = 0; i < R; ++i)
		m.rows[i] = a.rows[i] * b.rows[i];
	return m;
}

template <unsigned int R, unsigned int C>
HW_FORCE_INLINE Mat<R, C>& operator *= (Mat<R, C>& a, float b) {
	const Vec<C> v(b);
	for (unsigned int i = 0; i < R; ++i)
		a.rows[i] *= v;
	return a;
}

template <unsigned int R, unsigned int C>
HW_FORCE_INLINE Mat<R, C> operator * (const Mat<R, C>& a, float b) {
	const Vec<C> v(b);
	Mat<R, C> m;
	for (unsigned int i = 0; i < R; ++i)
		m.rows[i] = a.rows[i] * v;
	return m;
}

template <unsigned int R, unsigned int C>
HW_FORCE_INLINE Mat<R, C> operator * (float b, const Mat<R, C>& a) {
	return a * b;
}

typedef Mat<4, 4> mat4;
typedef Mat<3, 3> mat3;
typedef Mat<2, 2> mat2;
typedef Mat<3, 4> mat3x4;

inline mat4 operator * (const mat4& a, const mat4& b)
{
	mat4 mr;

	const vec4 b0 = b.rows[0];
	const vec4 b1 = b.rows[1];
	const vec4 b2 = b.rows[2];
	const vec4 b3 = b.rows[3];

	for (unsigned int i = 0; i < 4; ++i) {
		const vec4 row = a.rows[i];

		vec4 result;
		result  = b0 * row.spreadX();
		result += b1 * row.spreadY();
		result += b2 * row.spreadZ();
		result += b3 * row.spreadW();
		mr.rows[i] = result;
	}

	return mr;
}

inline mat3 operator * (const mat3& a, const mat3& b)
{
	mat3 mr;

	const vec3 b0 = b.rows[0];
	const vec3 b1 = b.rows[1];
	const vec3 b2 = b.rows[2];

	for (unsigned int i = 0; i < 3; ++i) {
		const vec3 row = a.rows[i];

		vec3 result;
		result  = b0 * row.spreadX();
		result += b1 * row.spreadY();
		result += b2 * row.spreadZ();
		mr.rows[i] = result;
	}

	return mr;
}

inline mat2 operator * (const mat2& a, const mat2& b)
{
	mat2 mr;

	const vec2 b0 = b.rows[0];
	const vec2 b1 = b.rows[1];

	for (unsigned int i = 0; i < 2; ++i) {
		const vec2 row = a.rows[i];

		vec2 result;
		result  = b0 * row.spreadX();
		result += b1 * row.spreadY();
		mr.rows[i] = result;
	}

	return mr;
}

inline mat3x4 concatTransform(const mat3x4& a, const mat3x4& b)
{
	mat3x4 mr;

	const vec4 b0 = b.rows[0];
	const vec4 b1 = b.rows[1];
	const vec4 b2 = b.rows[2];

	for (unsigned int i = 0; i < 3; ++i) {
		const vec4 row = a.rows[i];

		vec4 result;
		result  = b0 * row.spreadX();
		result += b1 * row.spreadY();
		result += b2 * row.spreadZ();
		result += vec4(0.f, 0.f, 0.f, 1.f) * row;
		mr.rows[i] = result;
	}

	return mr;
}

// TODO: Write batch versions that transpose
// This is not specially efficient, storage has been optimized for GPU
inline vec4 operator * (const mat4& m, const vec4& v) {
	const vec4 x = spreadDot(m.rows[0], v);
	const vec4 y = spreadDot(m.rows[1], v);
	const vec4 z = spreadDot(m.rows[2], v);
	const vec4 w = spreadDot(m.rows[3], v);

	__m128 xxyy = _mm_shuffle_ps(x.xmm, y.xmm, _MM_SHUFFLE(0, 0, 0, 0));
	__m128 zzww = _mm_shuffle_ps(z.xmm, w.xmm, _MM_SHUFFLE(0, 0, 0, 0));
	__m128 xyzw = _mm_shuffle_ps(xxyy, zzww, _MM_SHUFFLE(2, 0, 2, 0));

	return vec4(xyzw);
}

// This is not specially efficient, storage has been optimized for GPU
inline vec4 transform(const mat3x4& m, const vec4& v) {
	const vec4 x = spreadDot(m.rows[0], v);
	const vec4 y = spreadDot(m.rows[1], v);
	const vec4 z = spreadDot(m.rows[2], v);
	const vec4 w = v.spreadW();

	__m128 xxyy = _mm_shuffle_ps(x.xmm, y.xmm, _MM_SHUFFLE(0, 0, 0, 0));
	__m128 zzww = _mm_shuffle_ps(z.xmm, w.xmm, _MM_SHUFFLE(0, 0, 0, 0));
	__m128 xyzw = _mm_shuffle_ps(xxyy, zzww, _MM_SHUFFLE(2, 0, 2, 0));

	return vec4(xyzw);
}

inline vec3 transform(const mat3x4& m, const vec3& v) {
	vec4 v1 = v1;
	v1.setW(1.f);
	const vec4 x = spreadDot(m.rows[0], v1);
	const vec4 y = spreadDot(m.rows[1], v1);
	const vec4 z = spreadDot(m.rows[2], v1);

	__m128 xxyy = _mm_shuffle_ps(x.xmm, y.xmm, _MM_SHUFFLE(0, 0, 0, 0));
	//__m128 zz00 = _mm_shuffle_ps(z.xmm, _mm_setzero_ps(), _MM_SHUFFLE(0, 0, 0, 0));
	__m128 zz00 = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(z.xmm), 8));
	__m128 xyz0 = _mm_shuffle_ps(xxyy, zz00, _MM_SHUFFLE(2, 0, 2, 0));

	return vec3(xyz0);
}

} // namespace math
