#include "Matrix.h"
#include "Vector.h"
#include "MatrixTransform.h"
#include <iostream>

int main(int argc, char *argv[])
{
	vec4 a = {0, 0, 0, 1};

	mat4 b = mat_transform::identity<4>();

	vec3 t = {1, 2, 3};
	mat4 c = mat_transform::scale(t) * mat_transform::translate(t);
	b = b * c;

	vec4 r = b * a;

	for (int i = 0; i < 4; ++i) {
		std::cout << r[i] << " ";
	}
	std::cout << std::endl;

	std::cout << std::endl;

	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			std::cout << b(i, j) << " ";
		}
		std::cout << std::endl;
	}

	std::cin.get();

	return 0;
}