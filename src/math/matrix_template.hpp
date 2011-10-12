template <unsigned int C>
struct Mat<R, C> {
	Vec<C> rows[R];

	HW_FORCE_INLINE Mat() {}

	HW_FORCE_INLINE explicit Mat(float x) {
		for (unsigned int i = 0; i < R; ++i)
			rows[i] = Vec<C>(x);
	}

	HW_FORCE_INLINE explicit Mat(const float* data) {
		for (unsigned int i = 0; i < R; ++i)
			rows[i] = Vec<C>(data+(i*4));
	}

	HW_FORCE_INLINE explicit Mat(const Vec<C>& a, const Vec<C>& b
#if R >= 3
			,const Vec<C>& c
#if R >= 4
			,const Vec<C>& d
#endif
#endif
		) {
		rows[0] = a;
		rows[1] = b;
#if R >= 3
		rows[2] = c;
#if R >= 4
		rows[3] = d;
#endif
#endif
	}

	HW_FORCE_INLINE float* data() {
		return reinterpret_cast<float*>(&rows[0]);
	}
};
