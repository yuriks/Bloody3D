template <>
struct mat<N, 1>
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
};
