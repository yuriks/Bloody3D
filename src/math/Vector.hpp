#pragma once

#include "Heatwave.hpp"

#include <cmath>

#include <xmmintrin.h>
#include <pmmintrin.h>

namespace math {

#define N 4
#include "vec_template.hpp"
#undef N

#define N 3
#include "vec_template.hpp"
#undef N

#define N 2
#include "vec_template.hpp"
#undef N

namespace vec {
	HW_FORCE_INLINE vec3 cross(const vec3& a, const vec3& b) {
		__m128 tmp_a = _mm_mul_ps(_mm_shuffle_ps(a.xmm, a.xmm, _MM_SHUFFLE(3, 0, 2, 1)), _mm_shuffle_ps(b.xmm, b.xmm, _MM_SHUFFLE(3, 1, 0, 2)));
		__m128 tmp_b = _mm_mul_ps(_mm_shuffle_ps(a.xmm, a.xmm, _MM_SHUFFLE(3, 1, 0, 2)), _mm_shuffle_ps(b.xmm, b.xmm, _MM_SHUFFLE(3, 0, 2, 1)));
		return vec3(_mm_sub_ps(tmp_a, tmp_b));
	}
}

}