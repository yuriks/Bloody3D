#pragma once

#include <cassert>
#include <cmath>
#include "vec.hpp"

template<unsigned int R, unsigned int C = R, typename T = float>
struct mat {
	// Rows
	vec<C, T> data[R];

	// Basic element acessors
	inline T& operator ()(unsigned int i, unsigned int j) {
		return data[i][j];
	}

	inline const T& operator ()(unsigned int i, unsigned int j) const {
		return data[i][j];
	}

	inline bool operator ==(const mat& o) const {
		bool eq = true;
		for (unsigned int i = 0; i < R; ++i) {
			eq = eq && data[i] == o.data[i];
		}
		return eq;
	}

	inline bool operator !=(const mat& o) const {
		return !(*this == o);
	}

	// Component-wise matrix arithmetic
#define DEFINE_OP(op) \
	inline mat& operator op##=(const mat& o) { \
		for (unsigned int i = 0; i < R; ++i) { \
			data[i] op##= o.data[i]; \
		} \
		return *this; \
	} \
	inline mat operator op(const mat& o) const { \
		mat r = *this; \
		return r op##= o; \
	}

	DEFINE_OP(+)
	DEFINE_OP(-)

#undef DEFINE_OP

	// Scalar multiplication
	inline mat& operator *=(const T a) {
		for (unsigned int i = 0; i < R; ++i) {
			data[i] *= a;
		}
		return *this;
	}

	inline mat operator *(const T a) const {
		mat r = *this;
		return r *= a;
	}

	inline mat operator -() const {
		mat r = *this;
		for (unsigned int i = 0; i < R; ++i) {
			r.data[i] = -r.data[i];
		}
		return r;
	}

	template<typename T2>
	inline mat<R,C,T2> typecast() const {
		mat<R,C,T2> r;
		for (unsigned int i = 0; i < R; ++i) {
			r.data[i] = data[i].typecast<T2>();
		}
		return r;
	}
};

template<unsigned int R, unsigned int C, typename T>
inline mat<R,C,T> operator *(const T a, const mat<R,C,T>& m) {
	return m * a;
}

// Convenience typedefs for the most common kinds of matrices.
typedef mat<2> mat2;
typedef mat<3> mat3;
typedef mat<4> mat4;

// Convenient constants for matrices.
static const mat2 mat2_identity = {{
	{1, 0},
	{0, 1}
}};
static const mat3 mat3_identity = {{
	{1, 0, 0},
	{0, 1, 0},
	{0, 0, 1}
}};
static const mat4 mat4_identity = {{
	{1, 0, 0, 0},
	{0, 1, 0, 0},
	{0, 0, 1, 0},
	{0, 0, 0, 1}
}};

// Matrix-matrix multiplication
template<unsigned int M, unsigned int N, unsigned int P, typename T>
mat<M,P,T> operator *(const mat<M,N,T>& a, const mat<N,P,T>& b) {
	mat<M,P> vr;

	for (unsigned int m = 0; m < M; ++m) {
		for (unsigned int p = 0; p < P; ++p) {
			T s = 0;
			for (unsigned int n = 0; n < N; ++n) {
				s += a(m,n) * b(n,p);
			}
			vr(m,p) = s;
		}
	}

	return vr;
}

// Matrix-column vector multiplication
template<unsigned int R, unsigned int C, typename T>
vec<R,T> operator *(const mat<R,C,T>& m, const vec<C,T> v) {
	vec<R,T> vr;

	for (unsigned int r = 0; r < R; ++r) {
		T s = 0;
		for (unsigned int c = 0; c < C; ++c) {
			s += m(r,c) * v[c];
		}
		vr[r] = s;
	}

	return vr;
}

//Row vector-matrix multiplication
template<unsigned int R, unsigned int C, typename T>
vec<C,T> operator *(const vec<R,T> v, const mat<R,C,T>& m) {
	vec<C,T> vr;

	for (unsigned int c = 0; c < C; ++c) {
		T s = 0;
		for (unsigned int r = 0; r < R; ++r) {
			s += m(r,c) * v[r];
		}
		vr[c] = s;
	}

	return vr;
}

// Column vector version
template<unsigned int N, typename T>
inline vec<N,T> vec_from_mat(const mat<N,1,T>& m) {
	vec<N> v;
	
	for (unsigned int i = 0; i < N; ++i) {
		v[i] = m(i, 0);
	}

	return v;
}

// Row vector version
template<unsigned int N, typename T>
inline vec<N,T> vec_from_mat(const mat<1,N,T>& m) {
	return m.data[0];
}

template<unsigned int R, unsigned int C, typename T>
mat<C,R,T> transpose(const mat<R,C,T>& m) {
	mat<C,R,T> t;

	for (unsigned int i = 0; i < R; ++i) {
		for (unsigned int j = 0; j < C; ++j) {
			t(j, i) = m(i, j);
		}
	}

	return t;
}

template<unsigned int DN, unsigned int SN, typename T>
mat<DN,DN,T> pad(const mat<SN,SN,T>& m) {
	mat<DN,DN,T> rm;

	for (unsigned int i = 0; i < DN; ++i) {
		for (unsigned int j = 0; j < DN; ++j) {
			if (i < SN && j < SN) {
				rm(i,j) = m(i,j);
			} else {
				rm(i,j) = i == j ? T(1) : T(0);
			}
		}
	}

	return rm;
}

// Prints matrix m to stream.
template<unsigned int R, unsigned int C, typename T>
std::ostream& operator <<(std::ostream& s, const mat<R,C,T>& m) {
	for (unsigned int i = 0; i < R; ++i) {
		s << '|';
		for (unsigned int j = 0; j < C-1; ++j) {
			s << m(i, j) << ' ';
		}
		s << m(i, C-1) << '|';
		if (i < R-1)
			s << '\n';
	}

	return s;
}
