#ifndef BLOODY3D_MATRIXTRANSFORM
#define BLOODY3D_MATRIXTRANSFORM

#include "Matrix.h"

namespace mat_transform
{

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

}

#endif // BLOODY3D_MATRIXTRANSFORM
