#ifndef BLOODY3D_VECTOR
#define BLOODY3D_VECTOR

template <unsigned int N>
struct vec
{
	inline void operator+=(const vec& v)
	{
		for (unsigned int i = 0; i < N; ++i) {
			data[i] += v.data[i];
		}
	}

	inline void operator*=(float f)
	{
		for (unsigned int i = 0; i < N; ++i) {
			data[i] *= f;
		}
	}

	inline float& operator[](unsigned int d)
	{
		return data[d];
	}

	inline const float& operator[](unsigned int d) const
	{
		return data[d];
	}

	float data[N];
};

template <unsigned int N>
float dot(const vec<N>& v1, const vec<N>& v2)
{
	float r = 0;

	for (unsigned int i = 0; i < N; ++i) {
		r += v1[i] * v2[i];
	}

	return r;
}

vec<3> cross(const vec<3>& a, const vec<3>& b)
{
	vec<3> r = {a[1]*b[2] - a[2]*b[1], a[2]*b[0] - a[0]*b[2], a[0]*b[1] - a[1]*b[0]};
	return r;
}

template <unsigned int N>
inline vec<N> operator+(const vec<N>& a, const vec<N>& b)
{
	vec<N> r = a;
	r += b;
	return r;
}

template <unsigned int N>
inline vec<N> operator*(const vec<N>& v, float s)
{
	vec<N> r = v;
	r *= s;
	return r;
}

#endif // BLOODY3D_VECTOR
