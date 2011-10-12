template<>
struct Vec<N> {
	__m128 xmm;

	HW_FORCE_INLINE Vec() {}
	HW_FORCE_INLINE explicit Vec(__m128 data) : xmm(data) {}
	HW_FORCE_INLINE explicit Vec(const float* data) : xmm(_mm_load_ps(data)) {}

#if N == 4
	HW_FORCE_INLINE Vec(float x) : xmm(_mm_set_ps1(x)) {}
#elif N == 3
	HW_FORCE_INLINE Vec(float x) : xmm(_mm_set_ps(x, x, x, 0.f)) {}
#elif N == 2
	HW_FORCE_INLINE Vec(float x) : xmm(_mm_set_ps(x, x, 0.f, 0.f)) {}
#endif

#if N == 4
	HW_FORCE_INLINE Vec(float x, float y, float z, float w) : xmm(_mm_set_ps(x, y, z,   w  )) {}
#elif N == 3
	HW_FORCE_INLINE Vec(float x, float y, float z)          : xmm(_mm_set_ps(x, y, z,   0.f)) {}
#elif N == 2
	HW_FORCE_INLINE Vec(float x, float y)                   : xmm(_mm_set_ps(x, y, 0.f, 0.f)) {}
#endif

	// Pad with 0's
	template <unsigned int M>
	HW_FORCE_INLINE explicit Vec(const Vec<M>& m) : xmm(m.xmm) { static_assert(M <= N, "Invalid constructor for this size."); }

#if N == 3
	HW_FORCE_INLINE explicit Vec(const Vec<4>& v) : xmm(v.xmm) { setVal(3, 0.f); }
#endif

	HW_FORCE_INLINE Vec spreadX() const { return Vec(_mm_shuffle_ps(xmm, xmm, _MM_SHUFFLE(0, 0, 0, 0))); }
	HW_FORCE_INLINE Vec spreadY() const { return Vec(_mm_shuffle_ps(xmm, xmm, _MM_SHUFFLE(1, 1, 1, 1))); }
#if N >= 3
	HW_FORCE_INLINE Vec spreadZ() const { return Vec(_mm_shuffle_ps(xmm, xmm, _MM_SHUFFLE(2, 2, 2, 2))); }
#if N >= 4
	HW_FORCE_INLINE Vec spreadW() const { return Vec(_mm_shuffle_ps(xmm, xmm, _MM_SHUFFLE(3, 3, 3, 3))); }
#endif
#endif

	HW_FORCE_INLINE float getX() const { float tmp; _mm_store_ss(&tmp, xmm); return tmp; }
	HW_FORCE_INLINE float getY() const { return spreadY().getX(); }
#if N >= 3
	HW_FORCE_INLINE float getZ() const { return spreadZ().getX(); }
#if N >= 4
	HW_FORCE_INLINE float getW() const { return spreadW().getX(); }
#endif
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
	HW_FORCE_INLINE void setX(float val) { setVal(0, val); }
	HW_FORCE_INLINE void setY(float val) { setVal(1, val); }
#if N >= 3
	HW_FORCE_INLINE void setZ(float val) { setVal(2, val); }
#if N >= 4
	HW_FORCE_INLINE void setW(float val) { setVal(3, val); }
#endif
#endif
};
