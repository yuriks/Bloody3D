#include "Matrix.h"
#include "Vector.h"
#include <iostream>

int main(int argc, char *argv[])
{
	vec4 a = {4, 3, 2, 1};
	mat4 b = {1, 0, 0, 2,
	          0, 1, 0, 2,
	          0, 0, 1, 0,
	          0, 0, 0, 1};

	vec4 r = b * a;
	for (int i = 0; i < 4; ++i)
		std::cout << r[i] << " ";

	std::cin.get();

	return 0;
}