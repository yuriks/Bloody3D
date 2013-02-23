#pragma once

#include <cassert>
#include <cmath>
#include <ostream>

namespace math {

template<unsigned int N, typename T = float>
struct vec {
	T data[N == 3 ? 4 : N];

	inline T& operator [](unsigned int i) {
		return data[i];
	}

	inline const T& operator [](unsigned int i) const {
		return data[i];
	}

	inline bool operator ==(const vec o) const {
		bool eq = true;
		for (unsigned int i = 0; i < N; ++i) {
			eq = eq && data[i] == o[i];
		}
		return eq;
	}

	inline bool operator !=(const vec o) const {
		return !(*this == o);
	}

#define DEFINE_OP(op) \
	inline vec& operator op##=(const vec o) { \
		for (unsigned int i = 0; i < N; ++i) { \
			data[i] op##= o[i]; \
		} \
		return *this; \
	} \
	inline vec operator op(const vec o) const { \
		vec r = *this; \
		return r op##= o; \
	}

	DEFINE_OP(+)
	DEFINE_OP(-)
	DEFINE_OP(*)
	DEFINE_OP(/)

#undef DEFINE_OP

	inline vec& operator *=(const T a) {
		for (unsigned int i = 0; i < N; ++i) {
			data[i] *= a;
		}
		return *this;
	}

	inline vec operator *(const T a) const {
		vec r = *this;
		return r *= a;
	}

	inline vec operator -() const {
		vec r = *this;
		for (unsigned int i = 0; i < N; ++i) {
			r[i] = -r[i];
		}
		return r;
	}

	template<typename T2>
	inline vec<N,T2> typecast() const {
		vec<N, T2> r;
		for (unsigned int i = 0; i < N; ++i) {
			r[i] = T2((*this)[i]);
		}
		return r;
	}
};

template<unsigned int N, typename T>
inline vec<N,T> operator *(const T a, const vec<N,T> v) {
	return v * a;
}

// Convenience typedefs for the most popular kinds of vector.
typedef vec<2> vec2;
typedef vec<3> vec3;
typedef vec<4> vec4;

// Convenience functions for constructing vectors inline.
template<typename T>
inline vec<2,T> mvec2(T x, T y) {
	vec<2,T> v = {x, y};
	return v;
}

template<typename T>
inline vec<3,T> mvec3(T x, T y, T z) {
	vec<3,T> v = {x, y, z};
	return v;
}

template<typename T>
inline vec<3,T> mvec3(T s) {
	vec<3,T> v = {s, s, s};
	return v;
}

template<typename T>
inline vec<3,T> mvec3(const vec<4,T>& a) {
	vec<3,T> v = {a[0], a[1], a[2]};
	return v;
}

template<typename T>
inline vec<4,T> mvec4(T x, T y, T z, T w) {
	vec<4,T> v = {x, y, z, w};
	return v;
}

template<typename T>
inline vec<4,T> mvec4(const vec<3,T>& a, T b) {
	vec<4,T> v = {a[0], a[1], a[2], b};
	return v;
}

// Convenient constants for vectors
static const vec3 vec3_x = {1, 0, 0};
static const vec3 vec3_y = {0, 1, 0};
static const vec3 vec3_z = {0, 0, 1};
static const vec3 vec3_1 = {1, 1, 1};
static const vec3 vec3_0 = {0, 0, 0};

static const vec3 right = vec3_x;
static const vec3 up = vec3_y;
static const vec3 forward = vec3_z;

// Computes dot-product of vectors a and b.
template<unsigned int N, typename T>
inline T dot(const vec<N,T> a, const vec<N,T> b) {
	T r = 0;
	for (unsigned int i = 0; i < N; ++i) {
		r += a[i] * b[i];
	}
	return r;
}

// Computes cross-product of vectors a and b.
template<typename T>
inline vec<3,T> cross(const vec<3,T> a, const vec<3,T> b) {
	vec<3,T> r = {
		a[1]*b[2] - a[2]*b[1],
		a[2]*b[0] - a[0]*b[2],
		a[0]*b[1] - a[1]*b[0]
	};
	return r;
}

// Computes euclidean length of vector v.
template<unsigned int N, typename T>
inline T length(const vec<N,T> v) {
	return std::sqrt(dot(v, v));
}

// Normalizes vector v.
template<unsigned int N, typename T>
inline vec<N,T> normalized(const vec<N,T> v) {
	return v * (T(1) / length(v));
}

// Returns a vector with the reciprocals of the elements of v.
template<unsigned int N, typename T>
inline vec<N,T> inverse(const vec<N,T> v) {
	vec<N,T> rv;
	for (unsigned int i = 0; i < N; ++i) {
		rv[i] = T(1) / v[i];
	}
	return rv;
}

// Prints vector v to stream in the format "<x y z>".
template<unsigned int N, typename T>
std::ostream& operator <<(std::ostream& s, const vec<N,T> v) {
	s << '<';
	for (unsigned int i = 0; i < N-1; ++i) {
		s << v[i] << ' ';
	}
	return s << v[N-1] << '>';
}

} // namespace math
