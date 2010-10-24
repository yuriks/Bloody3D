#ifndef BLOODY3D_VECTOR
#define BLOODY3D_VECTOR

#include "Matrix.h"

typedef mat<4, 1> vec4;
typedef mat<3, 1> vec3;
typedef mat<2, 1> vec2;

namespace vec
{

template <unsigned int N>
float dot(const mat<N, 1>& v1, const mat<N, 1>& v2)
{
	float r = 0;

	for (unsigned int i = 0; i < N; ++i) {
		r += v1[i] * v2[i];
	}

	return r;
}

inline vec3 cross(const vec3& a, const vec3& b)
{
	vec3 r = {a[1]*b[2] - a[2]*b[1], a[2]*b[0] - a[0]*b[2], a[0]*b[1] - a[1]*b[0]};
	return r;
}

template <unsigned int N>
inline mat<N-1, 1> euclidean(const mat<N, 1>& v)
{
	mat<N-1, 1> r;

	for (unsigned int i = 0; i < N-1; ++i) {
		r[i] = v[i] / v[N-1];
	}

	return r;
}

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

template <unsigned int N>
float length(const mat<N, 1>& v)
{
	float l = 0;

	for (unsigned int i = 0; i < N; ++i) {
		l += v[i] * v[i];
	}

	return sqrt(l);
}

template <unsigned int N>
mat<N, 1> unit(const mat<N, 1>& v)
{
	return v * (1.f / length(v));
}

};

#endif // BLOODY3D_VECTOR
