#ifndef BLOODY3D_MATRIX
#define BLOODY3D_MATRIX

template <unsigned int M, unsigned int N = M>
struct mat
{
	inline void operator+=(const mat& m)
	{
		for (unsigned int i = 0; i < M*N; ++i) {
			data[i] += m.data[i];
		}
	}

	inline void operator*=(float f)
	{
		for (unsigned int i = 0; i < M*N; ++i) {
			data[i] *= f;
		}
	}

	inline void clear(float f)
	{
		for (unsigned int i = 0; i < M*N; ++i) {
			data[i] = f;
		}
	}

	inline float& operator()(unsigned int r, unsigned int c)
	{
		return data[N*r + c];
	}

	inline const float& operator()(unsigned int r, unsigned int c) const
	{
		return data[N*r + c];
	}

	inline float& operator[](unsigned int r)
	{
		static_assert (N == 1, "operator[] for vectors only!");
		return data[r];
	}

	inline const float& operator[](unsigned int r) const
	{
		static_assert (N == 1, "operator[] for vectors only!");
		return data[r];
	}

	float data[M*N];
};

typedef mat<4> mat4;
typedef mat<3> mat3;
typedef mat<2> mat2;

template <unsigned int M, unsigned int N, unsigned int P>
mat<M, P> operator*(const mat<M, N>& m1, const mat<N, P>& m2)
{
	mat<M, P> mr;

	for (unsigned int i = 0; i < M; ++i) {
		for (unsigned int j = 0; j < P; ++j) {
			mr(i, j) = 0;
			for (unsigned int k = 0; k < N; ++k) {
				mr(i, j) += m1(i, k) * m2(k, j);
			}
		}
	}

	return mr;
}

template <unsigned int M, unsigned int N>
inline mat<M, N> operator+(const mat<M, N>& m1, const mat<M, N>& m2)
{
	mat<M, N> mr = m1;
	mr += m2;
	return mr;
}

template <unsigned int M, unsigned int N>
inline mat<M, N> operator*(const mat<M, N>& m, float s)
{
	mat<M, N> mr = m;
	mr *= s;
	return mr;
}

#endif // BLOODY3D_MATRIX
