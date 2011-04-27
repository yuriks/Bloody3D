#ifndef BLOODY3D_SSE2_MATRIX_HPP
#define BLOODY3D_SSE2_MATRIX_HPP

#include <xmmintrin.h>

namespace math {

template <>
struct mat<4, 4>
{
	inline void operator+=(const mat& m)
	{
		xmm_data[0] = _mm_add_ps(xmm_data[0], m.xmm_data[0]);
		xmm_data[1] = _mm_add_ps(xmm_data[1], m.xmm_data[1]);
		xmm_data[2] = _mm_add_ps(xmm_data[2], m.xmm_data[2]);
		xmm_data[3] = _mm_add_ps(xmm_data[3], m.xmm_data[3]);
	}

	inline void operator-=(const mat& m)
	{
		xmm_data[0] = _mm_sub_ps(xmm_data[0], m.xmm_data[0]);
		xmm_data[1] = _mm_sub_ps(xmm_data[1], m.xmm_data[1]);
		xmm_data[2] = _mm_sub_ps(xmm_data[2], m.xmm_data[2]);
		xmm_data[3] = _mm_sub_ps(xmm_data[3], m.xmm_data[3]);
	}

	inline void operator*=(float f)
	{
		__m128 src = _mm_set1_ps(f);

		xmm_data[0] = _mm_mul_ps(xmm_data[0], src);
		xmm_data[1] = _mm_mul_ps(xmm_data[1], src);
		xmm_data[2] = _mm_mul_ps(xmm_data[2], src);
		xmm_data[3] = _mm_mul_ps(xmm_data[3], src);
	}

	inline void operator/=(float f)
	{
		__m128 src = _mm_set1_ps(f);

		xmm_data[0] = _mm_div_ps(xmm_data[0], src);
		xmm_data[1] = _mm_div_ps(xmm_data[1], src);
		xmm_data[2] = _mm_div_ps(xmm_data[2], src);
		xmm_data[3] = _mm_div_ps(xmm_data[3], src);
	}

	inline void clear(float f)
	{
		__m128 src = _mm_set1_ps(f);

		xmm_data[0] = src;
		xmm_data[1] = src;
		xmm_data[2] = src;
		xmm_data[3] = src;
	}

	inline float& operator()(unsigned int r, unsigned int c)
	{
		return data[r + 4*c];
	}

	inline const float& operator()(unsigned int r, unsigned int c) const
	{
		return data[r + 4*c];
	}

	union
	{
		float data[4*4];
		__m128 xmm_data[4];
	};

	static const bool ROW_MAJOR = false;
};

template <>
struct mat<4, 1>
{
	inline void operator+=(const mat& m)
	{
		xmm_data = _mm_add_ps(xmm_data, m.xmm_data);
	}

	inline void operator-=(const mat& m)
	{
		xmm_data = _mm_sub_ps(xmm_data, m.xmm_data);
	}

	inline void operator*=(float f)
	{
		__m128 src = _mm_set1_ps(f);
		xmm_data = _mm_mul_ps(xmm_data, src);
	}

	inline void operator/=(float f)
	{
		__m128 src = _mm_set1_ps(f);
		xmm_data = _mm_div_ps(xmm_data, src);
	}

	inline void clear(float f)
	{
		__m128 src = _mm_set1_ps(f);
		xmm_data = src;
	}

	inline float& operator()(unsigned int r, unsigned int /* c */)
	{
		return data[r];
	}

	inline const float& operator()(unsigned int r, unsigned int /* c */) const
	{
		return data[r];
	}

	inline float& operator[](unsigned int r)
	{
		return data[r];
	}

	inline const float& operator[](unsigned int r) const
	{
		return data[r];
	}

	union
	{
		float data[4];
		__m128 xmm_data;
	};

	static const bool ROW_MAJOR = false;
};

template <>
struct mat<3, 1>
{
	inline void operator+=(const mat& m)
	{
		xmm_data = _mm_add_ps(xmm_data, m.xmm_data);
	}

	inline void operator-=(const mat& m)
	{
		xmm_data = _mm_sub_ps(xmm_data, m.xmm_data);
	}

	inline void operator*=(float f)
	{
		__m128 src = _mm_set1_ps(f);
		xmm_data = _mm_mul_ps(xmm_data, src);
	}

	inline void operator/=(float f)
	{
		__m128 src = _mm_set1_ps(f);
		xmm_data = _mm_div_ps(xmm_data, src);
	}

	inline void clear(float f)
	{
		__m128 src = _mm_set1_ps(f);
		xmm_data = src;
	}

	inline float& operator()(unsigned int r, unsigned int /* c */)
	{
		return data[r];
	}

	inline const float& operator()(unsigned int r, unsigned int /* c */) const
	{
		return data[r];
	}

	inline float& operator[](unsigned int r)
	{
		return data[r];
	}

	inline const float& operator[](unsigned int r) const
	{
		return data[r];
	}

	union
	{
		float data[4];
		__m128 xmm_data;
	};

	static const bool ROW_MAJOR = false;
};

/*
mat4 operator*(const mat4& m1, const mat4& m2)
{
	mat4 mr;

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
*/

} // namespace math

#endif // BLOODY3D_SSE2_MATRIX_HPP
