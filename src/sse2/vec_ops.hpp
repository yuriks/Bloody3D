namespace vec {

inline float dot(const vecN& v1, const vecN& v2)
{
	__m128 x;

	x = _mm_mul_ps(v1.xmm_data, v2.xmm_data);

#ifdef USE_SSE3
	x = _mm_hadd_ps(x, x);
	x = _mm_hadd_ps(x, x);
#else
	{
		__m128 y;
		y = _mm_shuffle_ps(x, x, _MM_SHUFFLE(0, 0, 3, 2));
		x = _mm_add_ps(x, y);
		y = _mm_shuffle_ps(x, x, _MM_SHUFFLE(0, 0, 0, 1));
		x = _mm_add_ss(x, y);
	}
#endif

	float r;
	_mm_store_ss(&r, x);
	return r;
}

inline float length(const vecN& v)
{
	/* MSVC is retarded
	__m128 x;
	
	x = _mm_set_ss(dot(v, v));
	x = _mm_sqrt_ss(x);

	float r;
	_mm_store_ss(&r, x);
	return r;
	*/

	__m128 x = v.xmm_data;
	x = _mm_mul_ps(x, x);

#ifdef USE_SSE3
	x = _mm_hadd_ps(x, x);
	x = _mm_hadd_ps(x, x);
#else
	{
		__m128 y;
		y = _mm_shuffle_ps(x, x, _MM_SHUFFLE(0, 0, 3, 2));
		x = _mm_add_ps(x, y);
		y = _mm_shuffle_ps(x, x, _MM_SHUFFLE(0, 0, 0, 1));
		x = _mm_add_ss(x, y);
	}
#endif

	x = _mm_sqrt_ss(x);

	float r;
	_mm_store_ss(&r, x);
	return r;
}

} // namespace vec
