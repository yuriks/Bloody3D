#if N < 2 || N > 4
#error vecN must be between 2 and 4
#endif

#define HACK_(x) vec ## x
#define HACK(x) HACK_(x)
#define vecN HACK(N)

//__declspec(align(4))
struct vecN {
	__m128 xmm;

	HW_FORCE_INLINE explicit vecN(__m128 data) : xmm(data) {}
	HW_FORCE_INLINE explicit vecN(float x) : xmm(_mm_set_ps1(x)) {}
#if N == 4
	HW_FORCE_INLINE vecN(float x, float y, float z, float w = 1.f) : xmm(_mm_set_ps(x, y, z, w)) {}
#elif N == 3
	HW_FORCE_INLINE vecN(float x, float y, float z) : xmm(_mm_set_ps(x, y, z, 0.f)) {}
#elif N == 2
	HW_FORCE_INLINE vecN(float x, float y) : xmm(_mm_set_ps(x, y, 0.f, 0.f)) {}
#endif

	HW_FORCE_INLINE vecN spreadX() const { return vecN(_mm_shuffle_ps(xmm, xmm, _MM_SHUFFLE(0, 0, 0, 0))); }
	HW_FORCE_INLINE vecN spreadY() const { return vecN(_mm_shuffle_ps(xmm, xmm, _MM_SHUFFLE(1, 1, 1, 1))); }
#if N >= 3
	HW_FORCE_INLINE vecN spreadZ() const { return vecN(_mm_shuffle_ps(xmm, xmm, _MM_SHUFFLE(2, 2, 2, 2))); }
#endif
#if N >= 4
	HW_FORCE_INLINE vecN spreadW() const { return vecN(_mm_shuffle_ps(xmm, xmm, _MM_SHUFFLE(3, 3, 3, 3))); }
#endif

	HW_FORCE_INLINE float getX() const { float tmp; _mm_store_ss(&tmp, xmm); return tmp; }
	HW_FORCE_INLINE float getY() const { return spreadY().getX(); }
#if N >= 3
	HW_FORCE_INLINE float getZ() const { return spreadZ().getX(); }
#endif
#if N >= 4
	HW_FORCE_INLINE float getW() const { return spreadW().getX(); }
#endif

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
#if N >= 3
	HW_FORCE_INLINE float setZ(float val) { setVal(2, val); }
#endif
#if N >= 4
	HW_FORCE_INLINE float setW(float val) { setVal(3, val); }
#endif
};

#define OP_TEMPLATE(OP, OPX) \
	HW_FORCE_INLINE vecN& operator OP##= (vecN& a, const vecN& b) { \
		a.xmm = _mm_##OPX##_ps(a.xmm, b.xmm); \
		return a; \
	} \
	\
	HW_FORCE_INLINE vecN operator OP (const vecN& a, const vecN& b) { \
		return vecN(_mm_##OPX##_ps(a.xmm, b.xmm)); \
	} \
	\
	HW_FORCE_INLINE vecN operator OP##= (vecN& a, float b) { \
		a.xmm = _mm_##OPX##_ps(a.xmm, EXPAND_VAL(b)); \
		return a; \
	} \
	\
	HW_FORCE_INLINE vecN operator OP (const vecN& a, float b) { \
		return vecN(_mm_##OPX##_ps(a.xmm, EXPAND_VAL(b))); \
	} \
	\
	HW_FORCE_INLINE vecN operator OP (float b, const vecN& a) { \
		return a OP b; \
	}

#if N == 2
#	define EXPAND_VAL(x) _mm_set_ps(x, x, 0.f, 0.f)
#elif N == 3
#	define EXPAND_VAL(x) _mm_set_ps(x, x, x, 0.f)
#elif N == 4
#	define EXPAND_VAL(x) _mm_set_ps1(x)
#endif

OP_TEMPLATE(+, add)
OP_TEMPLATE(-, sub)

#undef EXPAND_VAL
#define EXPAND_VAL(x) _mm_set_ps1(x)

OP_TEMPLATE(*, mul)
OP_TEMPLATE(/, div)

#undef EXPAND_VAL
#undef OP_TEMPLATE

namespace vec {

// Depends on unused elements being set to 0.f
HW_FORCE_INLINE float dot(const vecN& a, const vecN& b) {
	__m128 x = _mm_mul_ps(a.xmm, b.xmm);

	x = _mm_hadd_ps(x, x);
	x = _mm_hadd_ps(x, x);

	float tmp;
	_mm_store_ss(&tmp, x);
	return tmp;
}

// Depends on unused elements being set to 0.f
HW_FORCE_INLINE float length(const vecN& a) {
	__m128 x = _mm_mul_ps(a.xmm, a.xmm);

	x = _mm_hadd_ps(x, x);
	x = _mm_hadd_ps(x, x);

	x = _mm_sqrt_ss(x);

	float tmp;
	_mm_store_ss(&tmp, x);
	return tmp;
}

HW_FORCE_INLINE float invLength(const vecN& a) {
	__m128 x = _mm_mul_ps(a.xmm, a.xmm);

	x = _mm_hadd_ps(x, x);
	x = _mm_hadd_ps(x, x);

	x = _mm_rsqrt_ss(x);

	float tmp;
	_mm_store_ss(&tmp, x);
	return tmp;
}

HW_FORCE_INLINE vecN normalized(const vecN& a) {
	__m128 x = _mm_mul_ps(a.xmm, a.xmm);
	x = _mm_hadd_ps(x, x);
	x = _mm_hadd_ps(x, x);
	x = _mm_rsqrt_ps(x);

	x = _mm_mul_ps(a.xmm, x);

	float tmp;
	_mm_store_ss(&tmp, x);
}

HW_FORCE_INLINE vecN inverse(const vecN& a) {
	__m128 x = _mm_rcp_ps(a.xmm);
#if N != 4
	// Clear unused components to 0
	x = _mm_castsi128_ps(_mm_slli_si128(_mm_srli_si128(_mm_castps_si128(x), (4-N)*4), (4-N)*4));
#endif
	return vecN(x);
}

}

#undef vecN
#undef HACK
#undef HACK_
