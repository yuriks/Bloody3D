#ifndef BLOODY3D_MATRIX
#define BLOODY3D_MATRIX

template <int N>
struct mat
{
	void operator+=(const mat& m)
	{
		for (int i = 0; i < N*N; ++i) {
			data[i] += m.data[i];
		}
	}

	void operator*=(float f) {
		for (int i = 0; i < N*N; ++i) {
			data[i] *= f;
		}
	}

	float data[N*N];
};

#endif // BLOODY3D_MATRIX
