#ifndef BLOODY3D_MATRIXTRANSFORM
#define BLOODY3D_MATRIXTRANSFORM

#include "Matrix.hpp"
#include "Vector.hpp"

namespace math {

namespace mat_transform {

template <unsigned int N>
const mat<N>& identity()
{
	static mat<N> ident_mat;
	static bool filled = false;

	if (!filled) {
		filled = true;

		ident_mat.clear(0.f);
		for (unsigned int i = 0; i < N; ++i) {
			ident_mat(i, i) = 1.f;
		}
	}

	return ident_mat;
}

template <unsigned int N>
const mat<N+1> translate(const mat<N,1>& v)
{
	mat<N+1> m = identity<N+1>();

	for (unsigned int i = 0; i < N; ++i) {
		m(i, N) = v[i];
	}

	return m;
}

template <unsigned int N>
const mat<N+1> scale(const mat<N,1>& v)
{
	mat<N+1> m = identity<N+1>();

	for (unsigned int i = 0; i < N; ++i) {
		m(i, i) = v[i];
	}

	return m;
}

mat4 rotate(const vec3& axis, float angle);

} // namespace mat_transform

} // namespace math

#endif // BLOODY3D_MATRIXTRANSFORM
