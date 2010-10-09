#ifndef BLOODY3D_MATRIX
#define BLOODY3D_MATRIX

template <int N, int M = N>
struct mat
{
	void operator+=(const mat& m)
	{
		for (int i = 0; i < N*M; ++i) {
			data[i] += m.data[i];
		}
	}

	void operator*=(float f) {
		for (int i = 0; i < N*M; ++i) {
			data[i] *= f;
		}
	}

	float data[N*M];
};

#endif // BLOODY3D_MATRIX
